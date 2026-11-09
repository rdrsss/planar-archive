// R17-B runner: R17-2 rules 2 to 8 plus spec validation (DB-1) of the native process runner.
#include <catch2/catch_test_macros.hpp>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
import std;
import planar.engine.process.runner;
using namespace planar::engine::process;
namespace {
using environment_t = std::vector<std::pair<std::string, std::string>>;
struct temporary {
  std::filesystem::path path;
  temporary() {
    char pattern[] = "/tmp/planar-r17b-runner-XXXXXX";
    path           = std::filesystem::canonical(::mkdtemp(pattern));
  }
  ~temporary() {
    std::error_code ignored;
    std::filesystem::remove_all(path, ignored);
  }
  temporary(const temporary&)            = delete;
  temporary& operator=(const temporary&) = delete;
};
// Owns a test descriptor.
struct test_fd {
  int fd;
  explicit test_fd(int value) : fd(value) {
    REQUIRE(fd >= 0);
  }
  ~test_fd() {
    ::close(fd);
  }
  test_fd(const test_fd&)            = delete;
  test_fd& operator=(const test_fd&) = delete;
};
// Isolates the single-threaded descriptor probes from handles inherited from their test launcher. Pre-existing handles
// stay open until restoration; the probe's own subjects are created after capture. The enumeration handle is excluded,
// and cleanup rejects a changed file identity instead of changing a later subject that reused a descriptor number.
class inherited_descriptor_isolation {
  struct saved_descriptor {
    int           fd;
    int           flags;
    struct ::stat identity;
  };
  std::vector<saved_descriptor> _saved;

  std::expected<void, int> rollback(int error) {
    const auto restored = restore();
    return std::unexpected(restored ? error : restored.error());
  }

public:
  inherited_descriptor_isolation() = default;
  ~inherited_descriptor_isolation() {
    (void)restore();
  }
  inherited_descriptor_isolation(const inherited_descriptor_isolation&)            = delete;
  inherited_descriptor_isolation& operator=(const inherited_descriptor_isolation&) = delete;

  std::expected<void, int> isolate(std::span<const int> descriptors) {
    if (!_saved.empty())
      return std::unexpected(EALREADY);
    for (const int fd : descriptors) {
      const int flags = ::fcntl(fd, F_GETFD);
      if (flags < 0)
        return rollback(errno);
      struct ::stat identity{};
      if (::fstat(fd, &identity) != 0)
        return rollback(errno);
      _saved.push_back({fd, flags, identity});
      if (::fcntl(fd, F_SETFD, flags | FD_CLOEXEC) != 0)
        return rollback(errno);
    }
    return {};
  }

  std::expected<void, int> capture() {
    std::vector<int> descriptors;
    {
      std::unique_ptr<DIR, decltype(&::closedir)> observer(::opendir("/dev/fd"), &::closedir);
      if (!observer)
        return std::unexpected(errno);
      const int observer_fd = ::dirfd(observer.get());
      if (observer_fd < 0)
        return std::unexpected(errno);
      while (true) {
        errno             = 0;
        const auto* entry = ::readdir(observer.get());
        if (!entry) {
          if (errno != 0)
            return std::unexpected(errno);
          break;
        }
        const std::string_view name(entry->d_name);
        int                    fd     = -1;
        const auto             parsed = std::from_chars(name.data(), name.data() + name.size(), fd);
        if (parsed.ec == std::errc{} && parsed.ptr == name.data() + name.size() && fd >= 3 && fd != observer_fd)
          descriptors.push_back(fd);
      }
    }
    return isolate(descriptors);
  }

  std::expected<void, int> restore() noexcept {
    int error = 0;
    for (const auto& saved : std::views::reverse(_saved)) {
      struct ::stat current{};
      if (::fstat(saved.fd, &current) != 0) {
        if (error == 0)
          error = errno;
      } else if (current.st_dev != saved.identity.st_dev || current.st_ino != saved.identity.st_ino ||
                 current.st_mode != saved.identity.st_mode || current.st_rdev != saved.identity.st_rdev) {
        if (error == 0)
          error = ESTALE;
      } else if (::fcntl(saved.fd, F_SETFD, saved.flags) != 0 && error == 0) {
        error = errno;
      }
    }
    _saved.clear();
    if (error != 0)
      return std::unexpected(error);
    return {};
  }
};
// A deliberate ambient descriptor, dynamically allocated inside the child probe's 3..63 observation range.
int open_inheritable_marker() {
  test_fd source(::open("/dev/null", O_RDONLY | O_CLOEXEC));
  return ::fcntl(source.fd, F_DUPFD, 4);
}
int open_directory(const std::filesystem::path& path) {
  return ::open(path.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
}
// A /bin/sh -c spec rooted at the given directory descriptor with generous bounds.
spawn_spec shell(const std::string& script, int cwd_fd, environment_t environment = {}) {
  spawn_spec spec;
  spec.executable       = "/bin/sh";
  spec.argv             = {"sh", "-c", script};
  spec.environment      = std::move(environment);
  spec.cwd_fd           = cwd_fd;
  spec.timeout_ms       = 30'000;
  spec.max_output_bytes = 1'048'576;
  spec.retain_bytes     = 1'048'576;
  spec.kill_grace_ms    = 100;
  return spec;
}
run_result must_run(const spawn_spec& spec) {
  auto result = run(spec);
  if (!result)
    UNSCOPED_INFO("process_error code " << static_cast<int>(result.error().code) << " errno " << result.error().sys_errno);
  REQUIRE(result.has_value());
  return *result;
}
// Wall-clock milliseconds since a steady-clock point.
std::int64_t since_ms(std::chrono::steady_clock::time_point from) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - from).count();
}
// Reads a decimal pid the fixture wrote into the temporary root.
::pid_t read_pid(const std::filesystem::path& file) {
  std::ifstream in(file);
  long          value = 0;
  in >> value;
  REQUIRE(value > 1);
  return static_cast<::pid_t>(value);
}
// True once kill(pid, 0) reports ESRCH, re-checked for up to 5 s so init can reap a reparented zombie.
bool gone_eventually(::pid_t pid) {
  for (int attempt = 0; attempt < 500; ++attempt) {
    if (::kill(pid, 0) != 0 && errno == ESRCH)
      return true;
    ::usleep(10'000);
  }
  return false;
}
// Kills an escaped process at scope exit so a failing check never leaves it running.
struct kill_on_exit {
  ::pid_t pid = 0;
  ~kill_on_exit() {
    if (pid > 1)
      ::kill(pid, SIGKILL);
  }
};
void rejected(const spawn_spec& spec, process_error_code code, int sys_errno) {
  const auto result = run(spec);
  REQUIRE_FALSE(result.has_value());
  CHECK(result.error().code == code);
  CHECK(result.error().sys_errno == sys_errno);
}
} // namespace

TEST_CASE("R17-B runner exit 0 retains merged output", "[process]") {
  temporary  root;
  test_fd    cwd(open_directory(root.path));
  const auto result = must_run(shell("printf hello", cwd.fd));
  CHECK(result.how == termination::exited);
  CHECK(result.exit_code == 0);
  CHECK(result.signal == 0);
  CHECK(result.output == "hello");
  CHECK(result.output_bytes == 5);
  CHECK_FALSE(result.truncated);
  CHECK(result.duration_ms < 30'000);
}

TEST_CASE("R17-B runner exit 1 is exited with code 1", "[process]") {
  temporary  root;
  test_fd    cwd(open_directory(root.path));
  const auto result = must_run(shell("exit 1", cwd.fd));
  CHECK(result.how == termination::exited);
  CHECK(result.exit_code == 1);
  CHECK(result.output.empty());
  CHECK(result.output_bytes == 0);
}

TEST_CASE("R17-B runner child killing itself with SIGTERM is signaled", "[process]") {
  temporary  root;
  test_fd    cwd(open_directory(root.path));
  const auto result = must_run(shell("kill -TERM $$; sleep 5", cwd.fd));
  CHECK(result.how == termination::signaled);
  CHECK(result.signal == SIGTERM);
  CHECK(result.exit_code == 0);
}

TEST_CASE("R17-B runner signal disposition is reset to default in the child", "[process]") {
  // The test process ignores SIGTERM; SETSIGDEF must restore the default so the child still dies by it.
  temporary  root;
  test_fd    cwd(open_directory(root.path));
  const auto previous = ::signal(SIGTERM, SIG_IGN);
  const auto result   = run(shell("kill -TERM $$; sleep 5", cwd.fd));
  ::signal(SIGTERM, previous);
  REQUIRE(result.has_value());
  CHECK(result->how == termination::signaled);
  CHECK(result->signal == SIGTERM);
}

TEST_CASE("R17-B runner merges stdout and stderr in write order", "[process]") {
  temporary  root;
  test_fd    cwd(open_directory(root.path));
  const auto result = must_run(shell("printf a; printf b >&2; printf c", cwd.fd));
  CHECK(result.how == termination::exited);
  CHECK(result.output == "abc");
}

TEST_CASE("R17-B runner passes exactly the spec environment", "[process]") {
  temporary root;
  test_fd   cwd(open_directory(root.path));
  REQUIRE(::setenv("HOME", "/planar-r17b-host-home", 1) == 0);
  REQUIRE(::setenv("PLANAR_R17B_HOST_ONLY", "leak", 1) == 0);
  spawn_spec spec;
  spec.executable       = "/usr/bin/env";
  spec.argv             = {"env"};
  spec.environment      = {{"ALPHA", "1"}, {"BETA", "two words"}, {"GAMMA", "x=y"}};
  spec.cwd_fd           = cwd.fd;
  spec.timeout_ms       = 30'000;
  spec.max_output_bytes = 4096;
  spec.retain_bytes     = 4096;
  spec.kill_grace_ms    = 100;
  const auto result     = must_run(spec);
  CHECK(result.how == termination::exited);
  CHECK(result.exit_code == 0);
  CHECK(result.output == "ALPHA=1\nBETA=two words\nGAMMA=x=y\n");
}

TEST_CASE("R17-B runner empty environment gives the child no variables", "[process]") {
  temporary  root;
  test_fd    cwd(open_directory(root.path));
  spawn_spec spec;
  spec.executable       = "/usr/bin/env";
  spec.argv             = {"env"};
  spec.cwd_fd           = cwd.fd;
  spec.timeout_ms       = 30'000;
  spec.max_output_bytes = 4096;
  spec.retain_bytes     = 4096;
  const auto result     = must_run(spec);
  CHECK(result.how == termination::exited);
  CHECK(result.output.empty());
}

TEST_CASE("R17-B runner working directory comes from the descriptor", "[process]") {
  temporary  root;
  temporary  elsewhere;
  const auto target = root.path / "bound";
  std::filesystem::create_directory(target);
  test_fd    cwd(open_directory(target));
  const auto saved = std::filesystem::current_path();
  std::filesystem::current_path(elsewhere.path);
  const auto result = run(shell("pwd -P", cwd.fd));
  std::filesystem::current_path(saved);
  REQUIRE(result.has_value());
  CHECK(result->how == termination::exited);
  CHECK(result->output == std::filesystem::canonical(target).string() + "\n");
}

TEST_CASE("R17-B runner stdin is empty", "[process]") {
  temporary  root;
  test_fd    cwd(open_directory(root.path));
  spawn_spec spec   = shell("", cwd.fd);
  spec.executable   = "/bin/cat";
  spec.argv         = {"cat"};
  const auto result = must_run(spec);
  CHECK(result.how == termination::exited);
  CHECK(result.exit_code == 0);
  CHECK(result.output.empty());
  CHECK(result.output_bytes == 0);
}

TEST_CASE("R17-B runner leaks no descriptors to the child", "[process]") {
  test_fd ambient(open_inheritable_marker());
  REQUIRE(ambient.fd > 3);
  REQUIRE(ambient.fd <= 63);
  const int original_flags = ::fcntl(ambient.fd, F_GETFD);
  REQUIRE(original_flags >= 0);
  REQUIRE((original_flags & FD_CLOEXEC) == 0);
  {
    // A failure after the first flag change must roll that change back immediately.
    inherited_descriptor_isolation partial;
    const std::array               descriptors{ambient.fd, -1};
    const auto                     isolated = partial.isolate(descriptors);
    REQUIRE_FALSE(isolated.has_value());
    CHECK(isolated.error() == EBADF);
    CHECK(::fcntl(ambient.fd, F_GETFD) == original_flags);
  }
  CHECK_THROWS_AS(
      [&] {
        inherited_descriptor_isolation unwinding;
        const std::array               descriptors{ambient.fd};
        REQUIRE(unwinding.isolate(descriptors).has_value());
        throw std::runtime_error("descriptor fixture unwinding");
      }(),
      std::runtime_error);
  CHECK(::fcntl(ambient.fd, F_GETFD) == original_flags);
  inherited_descriptor_isolation isolation;
  const auto                     isolated = isolation.capture();
  INFO("descriptor isolation errno " << (isolated ? 0 : isolated.error()));
  REQUIRE(isolated.has_value());
  CHECK(::fcntl(ambient.fd, F_GETFD) == (original_flags | FD_CLOEXEC));
  temporary  root;
  test_fd    cwd(open_directory(root.path));
  const auto file = root.path / "held";
  std::ofstream(file) << "x";
  test_fd cloexec(::open(file.c_str(), O_RDONLY | O_CLOEXEC));
  CHECK(cloexec.fd <= 63);
#if defined(__APPLE__)
  // POSIX_SPAWN_CLOEXEC_DEFAULT closes even a descriptor opened without O_CLOEXEC.
  test_fd inheritable(::open(file.c_str(), O_RDONLY));
  CHECK(inheritable.fd <= 63);
#endif
  std::string probe = "for n in";
  for (int fd = 3; fd <= 63; ++fd)
    probe += std::format(" {}", fd);
  probe += "; do [ -e /dev/fd/$n ] && echo $n; done; exit 0";
#if defined(__APPLE__)
  const int inheritable_flags = ::fcntl(inheritable.fd, F_GETFD);
  REQUIRE(inheritable_flags >= 0);
  CHECK((inheritable_flags & FD_CLOEXEC) == 0);
#endif
  const auto result = must_run(shell(probe, cwd.fd));
  INFO("leaked descriptors: " << result.output);
  CHECK(result.how == termination::exited);
  CHECK(result.exit_code == 0);
  CHECK(result.output.empty());
  REQUIRE(isolation.restore().has_value());
  CHECK(::fcntl(ambient.fd, F_GETFD) == original_flags);
}

TEST_CASE("R17-B runner probe detects an inherited descriptor", "[process]") {
  // Positive control for the leak probe: a descriptor opened onto fd 9 inside the shell is reported. The probe uses
  // `[ -e /dev/fd/N ]` rather than `(: <&N)`, which bash (macOS /bin/sh) answers with its own saved fd 10 (DB-13).
  test_fd ambient(open_inheritable_marker());
  REQUIRE(ambient.fd > 3);
  REQUIRE(ambient.fd <= 63);
  const int original_flags = ::fcntl(ambient.fd, F_GETFD);
  REQUIRE(original_flags >= 0);
  REQUIRE((original_flags & FD_CLOEXEC) == 0);
  inherited_descriptor_isolation isolation;
  const auto                     isolated = isolation.capture();
  INFO("descriptor isolation errno " << (isolated ? 0 : isolated.error()));
  REQUIRE(isolated.has_value());
  CHECK(::fcntl(ambient.fd, F_GETFD) == (original_flags | FD_CLOEXEC));
  temporary  root;
  test_fd    cwd(open_directory(root.path));
  const auto result = must_run(shell("exec 9</dev/null; for n in 3 9; do [ -e /dev/fd/$n ] && echo $n; done", cwd.fd));
  CHECK(result.output == "9\n");
  REQUIRE(isolation.restore().has_value());
  CHECK(::fcntl(ambient.fd, F_GETFD) == original_flags);
}

TEST_CASE("R17-B runner retains a prefix and marks truncated", "[process]") {
  temporary root;
  test_fd   cwd(open_directory(root.path));
  auto      spec    = shell("printf 0123456789", cwd.fd);
  spec.retain_bytes = 4;
  const auto result = must_run(spec);
  CHECK(result.how == termination::exited);
  CHECK(result.output == "0123");
  CHECK(result.output_bytes == 10);
  CHECK(result.truncated);
}

TEST_CASE("R17-B runner retain 0 retains nothing but counts bytes", "[process]") {
  temporary root;
  test_fd   cwd(open_directory(root.path));
  auto      spec    = shell("printf 0123456789", cwd.fd);
  spec.retain_bytes = 0;
  const auto result = must_run(spec);
  CHECK(result.how == termination::exited);
  CHECK(result.output.empty());
  CHECK(result.output_bytes == 10);
  CHECK(result.truncated);
}

TEST_CASE("R17-B runner reads output larger than the pipe buffer", "[process]") {
  // 300000 bytes exceed any default pipe capacity; the child must not block and every byte is retained in order.
  temporary   root;
  test_fd     cwd(open_directory(root.path));
  const auto  data = root.path / "data";
  std::string expected;
  for (int index = 0; expected.size() < 300'000; ++index)
    expected += std::format("{:08}\n", index);
  std::ofstream(data, std::ios::binary) << expected;
  auto       spec   = shell("/bin/cat data", cwd.fd);
  const auto result = must_run(spec);
  CHECK(result.how == termination::exited);
  CHECK(result.output_bytes == expected.size());
  CHECK(result.output == expected);
  CHECK_FALSE(result.truncated);
}

TEST_CASE("R17-B runner non-executable file is not_spawned with EACCES", "[process]") {
  temporary  root;
  test_fd    cwd(open_directory(root.path));
  const auto tool = root.path / "tool";
  std::ofstream(tool) << "#!/bin/sh\necho ran\n";
  std::filesystem::permissions(tool, std::filesystem::perms::owner_read | std::filesystem::perms::owner_write);
  auto spec         = shell("", cwd.fd);
  spec.executable   = tool.string();
  spec.argv         = {"tool"};
  const auto result = must_run(spec);
  CHECK(result.how == termination::not_spawned);
  CHECK(result.spawn_errno == EACCES);
  CHECK(result.output.empty());
  CHECK(result.output_bytes == 0);
  CHECK(result.duration_ms == 0);
}

TEST_CASE("R17-B runner nonexistent executable is not_spawned with ENOENT", "[process]") {
  temporary root;
  test_fd   cwd(open_directory(root.path));
  auto      spec    = shell("", cwd.fd);
  spec.executable   = (root.path / "absent").string();
  spec.argv         = {"absent"};
  const auto result = must_run(spec);
  CHECK(result.how == termination::not_spawned);
  CHECK(result.spawn_errno == ENOENT);
  CHECK(result.exit_code == 0);
  CHECK(result.signal == 0);
}

TEST_CASE("R17-B runner rejects out-of-bound limits as resource_limit", "[process]") {
  temporary                                           root;
  test_fd                                             cwd(open_directory(root.path));
  const auto                                          base      = shell("exit 0", cwd.fd);
  const std::vector<std::function<void(spawn_spec&)>> mutations = {
      [](spawn_spec& spec) { spec.timeout_ms = 0; },
      [](spawn_spec& spec) { spec.timeout_ms = 3'600'001; },
      [](spawn_spec& spec) {
        spec.max_output_bytes = 0;
        spec.retain_bytes     = 0;
      },
      [](spawn_spec& spec) { spec.max_output_bytes = 16'777'217; },
      [](spawn_spec& spec) {
        spec.max_output_bytes = 10;
        spec.retain_bytes     = 11;
      },
      [](spawn_spec& spec) { spec.kill_grace_ms = 60'001; },
  };
  for (const auto& mutate : mutations) {
    auto spec = base;
    mutate(spec);
    rejected(spec, process_error_code::resource_limit, 0);
  }
  auto edges             = base;
  edges.timeout_ms       = 3'600'000;
  edges.max_output_bytes = 16'777'216;
  edges.retain_bytes     = 16'777'216;
  edges.kill_grace_ms    = 60'000;
  CHECK(must_run(edges).how == termination::exited);
  edges.timeout_ms       = 1'000;
  edges.max_output_bytes = 1;
  edges.retain_bytes     = 0;
  edges.kill_grace_ms    = 0;
  CHECK(must_run(edges).how == termination::exited);
}

TEST_CASE("R17-B runner rejects structural spec violations as spawn_io EINVAL", "[process]") {
  temporary                                           root;
  test_fd                                             cwd(open_directory(root.path));
  const auto                                          base      = shell("exit 0", cwd.fd, {{"A", "1"}, {"B", "2"}});
  const std::vector<std::function<void(spawn_spec&)>> mutations = {
      [](spawn_spec& spec) { spec.executable = "sh"; },
      [](spawn_spec& spec) { spec.executable = ""; },
      [](spawn_spec& spec) { spec.executable = std::string("/bin/sh\0x", 9); },
      [](spawn_spec& spec) { spec.argv.clear(); },
      [](spawn_spec& spec) { spec.argv.push_back(std::string("a\0b", 3)); },
      [](spawn_spec& spec) { spec.cwd_fd = -1; },
      [](spawn_spec& spec) { spec.environment = {{"B", "2"}, {"A", "1"}}; },
      [](spawn_spec& spec) { spec.environment = {{"A", "1"}, {"A", "2"}}; },
      [](spawn_spec& spec) { spec.environment = {{"", "1"}}; },
      [](spawn_spec& spec) { spec.environment = {{"A=B", "1"}}; },
      [](spawn_spec& spec) { spec.environment = {{std::string("A\0B", 3), "1"}}; },
      [](spawn_spec& spec) {
        spec.environment = {{"A", std::string("1\0"
                                              "2",
                                              3)}};
      },
  };
  for (const auto& mutate : mutations) {
    auto spec = base;
    mutate(spec);
    rejected(spec, process_error_code::spawn_io, EINVAL);
  }
  CHECK(must_run(base).how == termination::exited);
}

TEST_CASE("R17-B runner validation precedes spawning", "[process]") {
  // An invalid spec must not run the command: the marker file stays absent.
  temporary root;
  test_fd   cwd(open_directory(root.path));
  auto      spec  = shell(": > marker", cwd.fd);
  spec.timeout_ms = 0;
  rejected(spec, process_error_code::resource_limit, 0);
  CHECK_FALSE(std::filesystem::exists(root.path / "marker"));
  spec.timeout_ms = 30'000;
  CHECK(must_run(spec).how == termination::exited);
  CHECK(std::filesystem::exists(root.path / "marker"));
}

TEST_CASE("R17-B runner timeout with a TERM-ignoring child ends in SIGKILL", "[process]") {
  temporary root;
  test_fd   cwd(open_directory(root.path));
  auto      spec    = shell("trap '' TERM; /bin/sleep 30", cwd.fd);
  spec.timeout_ms   = 200;
  const auto result = must_run(spec);
  CHECK(result.how == termination::timed_out);
  CHECK(result.signal == SIGKILL);
  CHECK(result.exit_code == 0);
  // SIGTERM at 200 ms is ignored, SIGKILL follows after the 100 ms grace; generous slack for a loaded machine.
  CHECK(result.duration_ms >= 300);
  CHECK(result.duration_ms < 3'000);
}

TEST_CASE("R17-B runner timeout of a plain sleep ends in SIGTERM", "[process]") {
  temporary root;
  test_fd   cwd(open_directory(root.path));
  auto      spec    = shell("", cwd.fd);
  spec.executable   = "/bin/sleep";
  spec.argv         = {"sleep", "30"};
  spec.timeout_ms   = 200;
  const auto result = must_run(spec);
  CHECK(result.how == termination::timed_out);
  CHECK(result.signal == SIGTERM);
  CHECK(result.duration_ms >= 200);
  CHECK(result.duration_ms < 3'000);
}

TEST_CASE("R17-B runner leader exiting from a TERM handler keeps timed_out with signal 0", "[process]") {
  // DB-4: the first event decides how; the exit code is still reported.
  temporary root;
  test_fd   cwd(open_directory(root.path));
  auto      spec    = shell("trap 'exit 7' TERM; /bin/sleep 30 & wait", cwd.fd);
  spec.timeout_ms   = 200;
  const auto result = must_run(spec);
  CHECK(result.how == termination::timed_out);
  CHECK(result.signal == 0);
  CHECK(result.exit_code == 7);
}

TEST_CASE("R17-B runner kills a grandchild left in the group", "[process]") {
  temporary root;
  test_fd   cwd(open_directory(root.path));
  // The backgrounded sleep keeps the pipe open; only the group SIGKILL after the leader's reap ends it.
  const auto started = std::chrono::steady_clock::now();
  const auto result  = must_run(shell("/bin/sleep 30 & echo $! > pid.tmp; mv pid.tmp pid; exit 0", cwd.fd));
  CHECK(since_ms(started) < 5'000);
  CHECK(result.how == termination::exited);
  CHECK(result.exit_code == 0);
  const auto   grandchild = read_pid(root.path / "pid");
  kill_on_exit guard{grandchild};
  CHECK(gone_eventually(grandchild));
}

TEST_CASE("R17-B runner setsid escapee is not killed and the drain ends at the grace", "[process]") {
  // Recorded Q15 A limit: the escapee leaves the group, keeps the pipe open and survives run.
  temporary         root;
  test_fd           cwd(open_directory(root.path));
  const std::string launcher = "if [ -x /usr/bin/setsid ]; then L='/usr/bin/setsid'; "
                               "else L='/usr/bin/perl -MPOSIX -e POSIX::setsid();exec(@ARGV)or(exit(127))'; fi; "
                               "$L /bin/sh -c 'echo $$ > pid.tmp; mv pid.tmp pid; exec /bin/sleep 30' & "
                               "while [ ! -s pid ]; do /bin/sleep 0.01; done; exit 0";
  const auto        started  = std::chrono::steady_clock::now();
  const auto        result   = must_run(shell(launcher, cwd.fd));
  const auto        wall     = since_ms(started);
  const auto        escapee  = read_pid(root.path / "pid");
  kill_on_exit      guard{escapee};
  CHECK(result.how == termination::exited);
  CHECK(result.exit_code == 0);
  // run returns about kill_grace_ms (100) after the leader's reap, not when the escapee's sleep ends.
  const auto after_reap = wall - static_cast<std::int64_t>(result.duration_ms);
  CHECK(after_reap >= 90);
  CHECK(after_reap < 3'000);
  CHECK(::kill(escapee, 0) == 0);
}

TEST_CASE("R17-B runner output flood ends in output_limit with a saturated count", "[process]") {
  temporary root;
  test_fd   cwd(open_directory(root.path));
  auto      spec        = shell("", cwd.fd);
  spec.executable       = "/usr/bin/yes";
  spec.argv             = {"yes"};
  spec.max_output_bytes = 4096;
  spec.retain_bytes     = 1000;
  const auto  result    = must_run(spec);
  std::string expected;
  while (expected.size() < 1000)
    expected += "y\n";
  CHECK(result.how == termination::output_limit);
  CHECK(result.signal == SIGTERM);
  CHECK(result.output_bytes == 4097);
  CHECK(result.output == expected);
  CHECK(result.truncated);
  CHECK(result.duration_ms < 3'000);
}

TEST_CASE("R17-B runner output flood ignoring TERM ends in SIGKILL", "[process]") {
  temporary root;
  test_fd   cwd(open_directory(root.path));
  auto      spec        = shell("trap '' TERM; exec /usr/bin/yes", cwd.fd);
  spec.max_output_bytes = 4096;
  spec.retain_bytes     = 4096;
  const auto result     = must_run(spec);
  CHECK(result.how == termination::output_limit);
  CHECK(result.signal == SIGKILL);
  CHECK(result.output_bytes == 4097);
  CHECK(result.output.size() == 4096);
  CHECK(result.truncated);
}

TEST_CASE("R17-B runner output past the cap from a leader that already exited is output_limit", "[process]") {
  // DB-15: the overflow may only be read during the drain; the result is still output_limit.
  temporary  root;
  test_fd    cwd(open_directory(root.path));
  const auto data = root.path / "data";
  std::ofstream(data, std::ios::binary) << std::string(5000, 'z');
  auto spec             = shell("/bin/cat data", cwd.fd);
  spec.max_output_bytes = 4096;
  spec.retain_bytes     = 10;
  const auto result     = must_run(spec);
  CHECK(result.how == termination::output_limit);
  CHECK(result.output_bytes == 4097);
  CHECK(result.output == std::string(10, 'z'));
  CHECK(result.truncated);
}

TEST_CASE("R17-B runner output of exactly max_output_bytes is exited", "[process]") {
  temporary   root;
  test_fd     cwd(open_directory(root.path));
  const auto  data = root.path / "data";
  std::string expected;
  for (int index = 0; expected.size() < 4096; ++index)
    expected += static_cast<char>('a' + index % 26);
  std::ofstream(data, std::ios::binary) << expected;
  auto spec             = shell("/bin/cat data", cwd.fd);
  spec.max_output_bytes = 4096;
  spec.retain_bytes     = 4096;
  const auto result     = must_run(spec);
  CHECK(result.how == termination::exited);
  CHECK(result.exit_code == 0);
  CHECK(result.signal == 0);
  CHECK(result.output_bytes == 4096);
  CHECK(result.output == expected);
  CHECK_FALSE(result.truncated);
}
