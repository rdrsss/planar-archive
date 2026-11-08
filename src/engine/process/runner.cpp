module;
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
module planar.engine.process.runner;
import std;
namespace planar::engine::process {
namespace {
constexpr std::uint64_t max_timeout_ms       = 3'600'000;
constexpr std::uint64_t max_output_limit     = 16'777'216;
constexpr std::uint64_t max_kill_grace_ms    = 60'000;
constexpr int           poll_slice_ms        = 10;
constexpr std::size_t   read_buffer_bytes    = 64 * 1024;
constexpr int           late_drain_max_reads = 64;
using clock                                  = std::chrono::steady_clock;

// Owns one descriptor; closed on every path.
class unique_fd {
public:
  unique_fd() = default;
  explicit unique_fd(int fd) : _fd(fd) {
  }
  unique_fd(unique_fd&& other) noexcept : _fd(std::exchange(other._fd, -1)) {
  }
  unique_fd& operator=(unique_fd&& other) noexcept {
    if (this != &other) {
      reset();
      _fd = std::exchange(other._fd, -1);
    }
    return *this;
  }
  unique_fd(const unique_fd&)            = delete;
  unique_fd& operator=(const unique_fd&) = delete;
  ~unique_fd() {
    reset();
  }
  [[nodiscard]] int get() const noexcept {
    return _fd;
  }
  void reset() noexcept {
    if (_fd >= 0)
      ::close(_fd);
    _fd = -1;
  }

private:
  int _fd = -1;
};

// Owns an initialised posix_spawnattr_t.
class spawn_attributes {
public:
  spawn_attributes()                                   = default;
  spawn_attributes(const spawn_attributes&)            = delete;
  spawn_attributes& operator=(const spawn_attributes&) = delete;
  ~spawn_attributes() {
    if (_initialised)
      ::posix_spawnattr_destroy(&_value);
  }
  int init() {
    const int rc = ::posix_spawnattr_init(&_value);
    _initialised = rc == 0;
    return rc;
  }
  posix_spawnattr_t* get() noexcept {
    return &_value;
  }

private:
  posix_spawnattr_t _value{};
  bool              _initialised = false;
};

// Owns an initialised posix_spawn_file_actions_t.
class spawn_file_actions {
public:
  spawn_file_actions()                                     = default;
  spawn_file_actions(const spawn_file_actions&)            = delete;
  spawn_file_actions& operator=(const spawn_file_actions&) = delete;
  ~spawn_file_actions() {
    if (_initialised)
      ::posix_spawn_file_actions_destroy(&_value);
  }
  int init() {
    const int rc = ::posix_spawn_file_actions_init(&_value);
    _initialised = rc == 0;
    return rc;
  }
  posix_spawn_file_actions_t* get() noexcept {
    return &_value;
  }

private:
  posix_spawn_file_actions_t _value{};
  bool                       _initialised = false;
};

std::unexpected<process_error> failure(process_error_code code, int sys_errno) {
  return std::unexpected(process_error{code, sys_errno});
}

bool has_nul(std::string_view text) {
  return text.find('\0') != std::string_view::npos;
}

// DB-1: bound violations are resource_limit; structural violations are spawn_io with EINVAL.
std::optional<process_error> validate(const spawn_spec& spec) {
  if (spec.timeout_ms < 1 || spec.timeout_ms > max_timeout_ms || spec.max_output_bytes < 1 ||
      spec.max_output_bytes > max_output_limit || spec.retain_bytes > spec.max_output_bytes ||
      spec.kill_grace_ms > max_kill_grace_ms)
    return process_error{process_error_code::resource_limit, 0};
  const process_error structural{process_error_code::spawn_io, EINVAL};
  if (spec.executable.empty() || spec.executable.front() != '/' || has_nul(spec.executable))
    return structural;
  if (spec.argv.empty() || std::ranges::any_of(spec.argv, [](const std::string& arg) { return has_nul(arg); }))
    return structural;
  if (spec.cwd_fd < 0)
    return structural;
  for (std::size_t index = 0; index < spec.environment.size(); ++index) {
    const auto& [name, value] = spec.environment[index];
    if (name.empty() || name.find('=') != std::string::npos || has_nul(name) || has_nul(value))
      return structural;
    if (index > 0 && !(spec.environment[index - 1].first < name))
      return structural;
  }
  return std::nullopt;
}

// Rule 2: both ends close-on-exec before any spawn (DB-7).
std::expected<std::pair<unique_fd, unique_fd>, int> make_pipe() {
  int fds[2] = {-1, -1};
#if defined(__APPLE__)
  if (::pipe(fds) != 0)
    return std::unexpected(errno);
  unique_fd read_end(fds[0]);
  unique_fd write_end(fds[1]);
  if (::fcntl(read_end.get(), F_SETFD, FD_CLOEXEC) != 0 || ::fcntl(write_end.get(), F_SETFD, FD_CLOEXEC) != 0)
    return std::unexpected(errno);
  return std::pair{std::move(read_end), std::move(write_end)};
#elif defined(__linux__)
  if (::pipe2(fds, O_CLOEXEC) != 0)
    return std::unexpected(errno);
  return std::pair{unique_fd(fds[0]), unique_fd(fds[1])};
#else
#error "planar.engine.process.runner supports macOS and Linux only"
#endif
}

int add_chdir(posix_spawn_file_actions_t* actions, int cwd_fd) {
#if defined(__APPLE__)
  // DB-12: the macOS 26 SDK deprecates the _np spelling in favour of posix_spawn_file_actions_addfchdir; the
  // contract names the _np form, which every supported macOS release provides.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
  return ::posix_spawn_file_actions_addfchdir_np(actions, cwd_fd);
#pragma clang diagnostic pop
#elif defined(__linux__)
  return ::posix_spawn_file_actions_addfchdir_np(actions, cwd_fd);
#else
#error "planar.engine.process.runner supports macOS and Linux only"
#endif
}

// Rule 2 attributes: new process group led by the child, empty mask, default disposition for catchable signals.
int configure_attributes(spawn_attributes& attributes) {
  if (const int rc = attributes.init(); rc != 0)
    return rc;
  int flags = POSIX_SPAWN_SETPGROUP | POSIX_SPAWN_SETSIGMASK | POSIX_SPAWN_SETSIGDEF;
#if defined(__APPLE__)
  flags |= POSIX_SPAWN_CLOEXEC_DEFAULT;
#endif
  if (const int rc = ::posix_spawnattr_setflags(attributes.get(), static_cast<short>(flags)); rc != 0)
    return rc;
  if (const int rc = ::posix_spawnattr_setpgroup(attributes.get(), 0); rc != 0)
    return rc;
  sigset_t empty;
  sigemptyset(&empty);
  if (const int rc = ::posix_spawnattr_setsigmask(attributes.get(), &empty); rc != 0)
    return rc;
  sigset_t defaults;
  sigfillset(&defaults);
  sigdelset(&defaults, SIGKILL);
  sigdelset(&defaults, SIGSTOP);
  return ::posix_spawnattr_setsigdefault(attributes.get(), &defaults);
}

int configure_file_actions(spawn_file_actions& actions, int cwd_fd, int devnull, int write_end) {
  if (const int rc = actions.init(); rc != 0)
    return rc;
  if (const int rc = add_chdir(actions.get(), cwd_fd); rc != 0)
    return rc;
  if (const int rc = ::posix_spawn_file_actions_adddup2(actions.get(), devnull, STDIN_FILENO); rc != 0)
    return rc;
  if (const int rc = ::posix_spawn_file_actions_adddup2(actions.get(), write_end, STDOUT_FILENO); rc != 0)
    return rc;
  return ::posix_spawn_file_actions_adddup2(actions.get(), write_end, STDERR_FILENO);
}

bool not_spawned_errno(int code) {
  return code == ENOENT || code == EACCES || code == ENOEXEC || code == ENOTDIR;
}

// Accounts merged output: retains the first retain_bytes, counts up to max_output_bytes + 1.
struct output_sink {
  std::uint64_t retain_bytes;
  std::uint64_t saturation;
  std::string   retained;
  std::uint64_t total = 0;

  void consume(std::string_view chunk) {
    if (retained.size() < retain_bytes) {
      const auto room = static_cast<std::size_t>(retain_bytes - retained.size());
      retained.append(chunk.substr(0, std::min(room, chunk.size())));
    }
    total = std::min(saturation, total + chunk.size());
  }
};

enum class slice_result { idle, data, eof };

// One poll slice on the read end; EINTR is retried by returning idle (rule 8).
std::expected<slice_result, int> read_slice(int fd, int timeout_ms, std::vector<char>& buffer, output_sink& sink) {
  pollfd    entry{fd, POLLIN, 0};
  const int ready = ::poll(&entry, 1, timeout_ms);
  if (ready < 0)
    return errno == EINTR ? std::expected<slice_result, int>(slice_result::idle) : std::unexpected(errno);
  if (ready == 0)
    return slice_result::idle;
  if ((entry.revents & POLLNVAL) != 0)
    return std::unexpected(EBADF);
  const ::ssize_t count = ::read(fd, buffer.data(), buffer.size());
  if (count < 0)
    return errno == EINTR || errno == EAGAIN ? std::expected<slice_result, int>(slice_result::idle) : std::unexpected(errno);
  if (count == 0)
    return slice_result::eof;
  sink.consume(std::string_view(buffer.data(), static_cast<std::size_t>(count)));
  return slice_result::data;
}

// Best-effort cleanup on an error path (DB-9): group SIGKILL, then a blocking reap of the leader.
void abandon(::pid_t pid, bool reaped) {
  ::kill(-pid, SIGKILL);
  if (reaped)
    return;
  int status = 0;
  while (::waitpid(pid, &status, 0) < 0 && errno == EINTR) {
  }
}

// Signals the whole group led by pid (DB-9). ESRCH means the group is gone. EPERM is also not an error (DB-16):
// macOS reports it for a group whose only members are zombies, e.g. a leader that exited but is not yet reaped.
std::optional<int> signal_group(::pid_t pid, int signal) {
  if (::kill(-pid, signal) == 0 || errno == ESRCH || errno == EPERM)
    return std::nullopt;
  return errno;
}

std::uint64_t elapsed_ms(clock::time_point from, clock::time_point to) {
  return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(to - from).count());
}
} // namespace

std::expected<run_result, process_error> run(const spawn_spec& spec) {
  if (const auto invalid = validate(spec))
    return std::unexpected(*invalid);

  unique_fd devnull(::open("/dev/null", O_RDONLY | O_CLOEXEC));
  if (devnull.get() < 0)
    return failure(process_error_code::spawn_io, errno);
  auto pipe = make_pipe();
  if (!pipe)
    return failure(process_error_code::pipe_io, pipe.error());
  auto& [read_end, write_end] = *pipe;

  spawn_attributes attributes;
  if (const int rc = configure_attributes(attributes); rc != 0)
    return failure(process_error_code::spawn_io, rc);
  spawn_file_actions actions;
  if (const int rc = configure_file_actions(actions, spec.cwd_fd, devnull.get(), write_end.get()); rc != 0)
    return failure(process_error_code::spawn_io, rc);

  // Rule 3: argv and envp are built from the spec only; nothing is inherited.
  std::vector<std::string> arguments = spec.argv;
  std::vector<char*>       argv;
  argv.reserve(arguments.size() + 1);
  for (auto& argument : arguments)
    argv.push_back(argument.data());
  argv.push_back(nullptr);
  std::vector<std::string> assignments;
  assignments.reserve(spec.environment.size());
  for (const auto& [name, value] : spec.environment)
    assignments.push_back(name + "=" + value);
  std::vector<char*> envp;
  envp.reserve(assignments.size() + 1);
  for (auto& assignment : assignments)
    envp.push_back(assignment.data());
  envp.push_back(nullptr);

  ::pid_t    pid   = -1;
  const auto start = clock::now();
  if (const int rc = ::posix_spawn(&pid, spec.executable.c_str(), actions.get(), attributes.get(), argv.data(), envp.data());
      rc != 0) {
    if (not_spawned_errno(rc)) {
      run_result skipped{};
      skipped.how         = termination::not_spawned; // DB-6: nothing ran, so output and duration stay empty
      skipped.spawn_errno = rc;
      return skipped;
    }
    return failure(process_error_code::spawn_io, rc);
  }
  write_end.reset();
  devnull.reset();

  output_sink       sink{spec.retain_bytes, spec.max_output_bytes + 1, {}, 0};
  std::vector<char> buffer(read_buffer_bytes);
  bool              eof    = false;
  bool              reaped = false;
  int               status = 0;
  clock::time_point reaped_at{};
  const auto        deadline = start + std::chrono::milliseconds(spec.timeout_ms);
  const auto        grace    = std::chrono::milliseconds(spec.kill_grace_ms);
  // DB-4: the first of deadline and output overflow decides how; later events change nothing.
  std::optional<termination> event;
  clock::time_point          term_sent_at{};
  bool                       kill_sent = false;

  // Rules 4 and 5: SIGTERM to the group now, SIGKILL once the grace has passed without a reap.
  const auto begin_kill = [&](termination cause) -> std::optional<int> {
    event        = cause;
    term_sent_at = clock::now();
    return signal_group(pid, SIGTERM);
  };

  // Main loop (DB-3): poll slices of at most 10 ms, leader liveness by waitpid(WNOHANG) after each slice.
  while (!reaped) {
    if (!eof) {
      const auto slice = read_slice(read_end.get(), poll_slice_ms, buffer, sink);
      if (!slice) {
        abandon(pid, false);
        return failure(process_error_code::pipe_io, slice.error());
      }
      eof = *slice == slice_result::eof;
    } else {
      ::poll(nullptr, 0, poll_slice_ms);
    }
    // Rule 5 (DR17-13): reads past the cap are discarded by the sink; the group is killed like a timeout.
    if (!event && sink.total > spec.max_output_bytes) {
      if (const auto code = begin_kill(termination::output_limit)) {
        abandon(pid, false);
        return failure(process_error_code::wait_io, *code);
      }
    }
    const ::pid_t waited = ::waitpid(pid, &status, WNOHANG);
    if (waited == pid) {
      reaped    = true;
      reaped_at = clock::now();
      break;
    }
    if (waited < 0 && errno != EINTR) {
      const int code = errno;
      abandon(pid, false);
      return failure(process_error_code::wait_io, code);
    }
    const auto now = clock::now();
    if (!event && now >= deadline) {
      if (const auto code = begin_kill(termination::timed_out)) {
        abandon(pid, false);
        return failure(process_error_code::wait_io, *code);
      }
    }
    if (event && !kill_sent && clock::now() >= term_sent_at + grace) {
      kill_sent = true;
      if (const auto code = signal_group(pid, SIGKILL)) {
        abandon(pid, false);
        return failure(process_error_code::wait_io, *code);
      }
    }
  }

  // Rule 6: kill what remains of the group, then drain until EOF or for at most kill_grace_ms.
  if (const auto code = signal_group(pid, SIGKILL))
    return failure(process_error_code::wait_io, *code);
  const auto drain_deadline = reaped_at + std::chrono::milliseconds(spec.kill_grace_ms);
  int        late_reads     = 0;
  while (!eof) {
    const auto now  = clock::now();
    const bool late = now >= drain_deadline;
    const int  wait_ms =
        late ? 0
             : static_cast<int>(std::min<std::int64_t>(
                   poll_slice_ms, std::chrono::duration_cast<std::chrono::milliseconds>(drain_deadline - now).count() + 1));
    const auto slice = read_slice(read_end.get(), wait_ms, buffer, sink);
    if (!slice)
      return failure(process_error_code::pipe_io, slice.error());
    eof = *slice == slice_result::eof;
    // DB-11: past the grace, only bytes already buffered are taken, and only for a bounded number of reads.
    if (late && (*slice != slice_result::data || ++late_reads >= late_drain_max_reads))
      break;
  }

  // DB-15: an overflow first seen while draining (the leader wrote past the cap and exited before the main loop
  // read it) still makes the result output_limit unless the deadline came first.
  if (!event && sink.total > spec.max_output_bytes)
    event = termination::output_limit;

  run_result result{};
  if (WIFEXITED(status)) {
    result.how       = termination::exited;
    result.exit_code = WEXITSTATUS(status);
  } else if (WIFSIGNALED(status)) {
    result.how    = termination::signaled;
    result.signal = WTERMSIG(status);
  } else {
    return failure(process_error_code::wait_io, EINVAL);
  }
  // DB-4: a leader that exits normally after the kill keeps timed_out or output_limit with signal 0.
  if (event)
    result.how = *event;
  result.output       = std::move(sink.retained);
  result.output_bytes = sink.total;
  result.truncated    = result.output.size() < result.output_bytes;
  result.duration_ms  = elapsed_ms(start, reaped_at);
  return result;
}
} // namespace planar::engine::process
