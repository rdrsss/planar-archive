// Acceptance transaction faults use a temporary ledger and a scoped SQLite VFS.
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <cstdio>
#include <poll.h>
#include <signal.h>
#include <sqlite3.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
import std;
import planar.engine.acceptance.prepared;
import planar.engine.canonical;
import planar.engine.ledger.format;
import planar.engine.ledger.acceptance;
using namespace planar::engine;
using namespace planar::engine::history;
using namespace planar::engine::acceptance;
using namespace planar::engine::ledger;
namespace {
struct temporary {
  std::filesystem::path path;
  int                   format;
  explicit temporary(int selected_format = 1) : format(selected_format) {
    char pattern[] = "/tmp/planar-prepared-XXXXXX";
    path           = std::filesystem::canonical(::mkdtemp(pattern));
    put("workspace/a.lua", std::string("a\0b\r\n", 5));
    put("workspace/b.lua", "second");
    put("registry.json", std::string("r\0z", 3));
    REQUIRE(::chmod((path / "workspace/b.lua").c_str(), 0755) == 0);
  }
  ~temporary() {
    std::filesystem::remove_all(path);
  }
  void put(std::string name, std::string_view bytes) {
    auto file = path / name;
    std::filesystem::create_directories(file.parent_path());
    std::ofstream stream(file, std::ios::binary);
    stream.write(bytes.data(), bytes.size());
  }
  acceptance_capture capture(int registry = 1) {
    native_capture_session session;
    REQUIRE(session.add_root("workspace", path / "workspace"));
    REQUIRE(session.tree("workspace"));
    if (registry == 1) {
      REQUIRE(session.add_input("registry", path / "registry.json"));
      REQUIRE(session.read("registry", "input.json", false));
    } else if (registry == 2) {
      REQUIRE(session.add_root("registry", path / "other"));
      REQUIRE(session.tree("registry"));
    }
    auto result = session.finish_for_acceptance();
    REQUIRE(result);
    return std::move(*result);
  }
};
std::string read_bytes(const std::filesystem::path& file) {
  std::ifstream stream(file, std::ios::binary);
  REQUIRE(stream.good());
  return {std::istreambuf_iterator<char>(stream), {}};
}
std::string hex(const record_digest& digest) {
  std::string out;
  for (auto byte : digest)
    out += std::format("{:02x}", byte);
  return out;
}
struct inputs {
  prepared_provenance     stdlib;
  recorded_interpretation interpretation;
  inputs() {
    const std::array originals{original_file_view{"part.lua", "return {}"}};
    auto             p = prepare_stdlib(originals, "\n", "return {}\n");
    REQUIRE(p);
    stdlib       = std::move(*p);
    auto root    = std::filesystem::path(PLANAR_PREPARED_GOLDENS);
    auto decoded = decode_interpretation(read_bytes(root / "interpretation.canonical"), read_bytes(root / "versions.canonical"));
    REQUIRE(decoded);
    auto p2 = encode_interpretation(decoded->interpretation->fields.at("bundle"), decoded->interpretation->fields.at("semantic"),
                                    hex(stdlib.digest));
    REQUIRE(p2);
    interpretation = std::move(*p2);
  }
  auto prepare(acceptance_capture cap, std::span<const std::string_view> paths) {
    return prepared_acceptance::create(std::move(cap), paths, stdlib.files, interpretation.encoded,
                                       interpretation.versions_encoded);
  }
};
const std::array paths{std::string_view("b.lua"), std::string_view("a.lua")};

} // namespace
namespace {
request_intent intent(temporary& t, std::string_view key, std::optional<std::int64_t> head, std::string_view actor = "tester") {
  auto result = (t.format == 2 ? request_intent::create_v2(key, actor, head, (t.path / "workspace/a.lua").string(),
                                                           (t.path / "registry.json").string())
                               : request_intent::create(key, actor, head, (t.path / "workspace/a.lua").string(),
                                                        (t.path / "registry.json").string()));
  REQUIRE(result);
  return *result;
}
prepared_acceptance prepare(temporary& t, inputs& i) {
  auto result = i.prepare(t.capture(), paths);
  REQUIRE(result);
  return std::move(*result);
}
// Only this serial test executable changes the default VFS. Every scope restores the prior default.
struct fault_vfs {
  struct file {
    sqlite3_file  base;
    sqlite3_file* real;
    fault_vfs*    owner;
    bool          journal;
  };
  static inline fault_vfs* active   = nullptr;
  sqlite3_vfs*             original = sqlite3_vfs_find(nullptr);
  sqlite3_vfs              wrapper  = *original;
  sqlite3_io_methods       methods{};
  bool                     journal    = false;
  bool                     sync       = false;
  bool                     read_fault = false;
  bool                     armed      = false;
  int                      hits       = 0;
  int                      barrier    = -1;
  int                      remaining  = 0;
  fault_vfs() {
    REQUIRE_FALSE(active);
    active           = this;
    methods.iVersion = 1;
    methods.xClose   = [](sqlite3_file* p) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xClose(f.real);
    };
    methods.xRead = [](sqlite3_file* p, void* b, int n, sqlite3_int64 o) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xRead(f.real, b, n, o);
    };
    methods.xWrite = [](sqlite3_file* p, const void* b, int n, sqlite3_int64 o) {
      auto& f = *reinterpret_cast<file*>(p);
      if (f.owner->fire(f, false)) {
        std::println("VFS IOERR_WRITE offset={} bytes={}", o, n);
        return SQLITE_FULL;
      }
      return f.real->pMethods->xWrite(f.real, b, n, o);
    };
    methods.xTruncate = [](sqlite3_file* p, sqlite3_int64 n) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xTruncate(f.real, n);
    };
    methods.xSync = [](sqlite3_file* p, int flags) {
      auto& f = *reinterpret_cast<file*>(p);
      if (f.owner->fire(f, true))
        return SQLITE_IOERR_FSYNC;
      return f.real->pMethods->xSync(f.real, flags);
    };
    methods.xFileSize = [](sqlite3_file* p, sqlite3_int64* n) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xFileSize(f.real, n);
    };
    methods.xLock = [](sqlite3_file* p, int n) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xLock(f.real, n);
    };
    methods.xUnlock = [](sqlite3_file* p, int n) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xUnlock(f.real, n);
    };
    methods.xCheckReservedLock = [](sqlite3_file* p, int* n) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xCheckReservedLock(f.real, n);
    };
    methods.xFileControl = [](sqlite3_file* p, int op, void* arg) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xFileControl(f.real, op, arg);
    };
    methods.xSectorSize = [](sqlite3_file* p) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xSectorSize(f.real);
    };
    methods.xDeviceCharacteristics = [](sqlite3_file* p) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xDeviceCharacteristics(f.real);
    };
    wrapper.zName    = "planar-r12c2-test";
    wrapper.szOsFile = static_cast<int>(sizeof(file) + alignof(std::max_align_t) + original->szOsFile);
    wrapper.xOpen    = [](sqlite3_vfs*, const char* name, sqlite3_file* p, int flags, int* out) {
      auto& f       = *reinterpret_cast<file*>(p);
      auto  address = reinterpret_cast<std::uintptr_t>(p) + sizeof(file);
      address       = (address + alignof(std::max_align_t) - 1) & ~(alignof(std::max_align_t) - 1);
      f.real        = reinterpret_cast<sqlite3_file*>(address);
      f.owner       = active;
      f.journal     = (flags & SQLITE_OPEN_MAIN_JOURNAL) != 0;
      const int rc  = active->original->xOpen(active->original, name, f.real, flags, out);
      if (rc == SQLITE_OK)
        f.base.pMethods = &active->methods;
      return rc;
    };
    REQUIRE(sqlite3_vfs_register(&wrapper, 1) == SQLITE_OK);
  }
  bool fire(file& f, bool is_sync) {
    if (!armed || journal != f.journal || sync != is_sync)
      return false;
    if (remaining > 0) {
      --remaining;
      return false;
    }
    armed = false;
    ++hits;
    std::println("VFS fault journal={} sync={} barrier={}", journal, sync, barrier);
    if (barrier >= 0) {
      ::fflush(nullptr);
      const char ready = 'S';
      if (::write(barrier, &ready, 1) != 1)
        ::_exit(92);
      for (;;)
        ::pause();
    }
    return true;
  }
  ~fault_vfs() {
    sqlite3_vfs_register(original, 1);
    sqlite3_vfs_unregister(&wrapper);
    active = nullptr;
  }
};
struct child {
  pid_t pid           = -1;
  int   read_fd       = -1;
  int   write_fd      = -1;
  int   release_read  = -1;
  int   release_write = -1;
  child() {
    int pipe_fds[2];
    REQUIRE(::pipe(pipe_fds) == 0);
    read_fd  = pipe_fds[0];
    write_fd = pipe_fds[1];
    REQUIRE(::pipe(pipe_fds) == 0);
    release_read  = pipe_fds[0];
    release_write = pipe_fds[1];
    pid           = ::fork();
    REQUIRE(pid >= 0);
  }
  void ready() {
    pollfd p{read_fd, POLLIN, 0};
    REQUIRE(::poll(&p, 1, 10000) == 1);
    char signal;
    REQUIRE(::read(read_fd, &signal, 1) == 1);
    CHECK(signal == 'S');
  }
  void notify() {
    const char signal = 'S';
    if (::write(write_fd, &signal, 1) != 1)
      ::_exit(91);
  }
  void release() {
    const char signal = 'R';
    REQUIRE(::write(release_write, &signal, 1) == 1);
  }
  void await_release() {
    pollfd p{release_read, POLLIN, 0};
    if (::poll(&p, 1, 10000) != 1)
      ::_exit(99);
    char signal;
    if (::read(release_read, &signal, 1) != 1 || signal != 'R')
      ::_exit(100);
  }
  void exit_wait() {
    int   status = 0;
    pid_t waited = 0;
    for (int i = 0; i < 1000 && waited == 0; ++i) {
      waited = ::waitpid(pid, &status, WNOHANG);
      if (waited == 0)
        ::usleep(10000);
    }
    REQUIRE(waited == pid);
    pid = -1;
    std::println("child normal wait status={}", status);
    REQUIRE(WIFEXITED(status));
    REQUIRE(WEXITSTATUS(status) == 0);
  }
  int kill_wait() {
    REQUIRE(::kill(pid, SIGKILL) == 0);
    int status;
    REQUIRE(::waitpid(pid, &status, 0) == pid);
    pid = -1;
    std::println("child wait status={} signal={}", status, WIFSIGNALED(status) ? WTERMSIG(status) : 0);
    REQUIRE(WIFSIGNALED(status));
    REQUIRE(WTERMSIG(status) == SIGKILL);
    return status;
  }
  ~child() {
    if (pid > 0) {
      ::kill(pid, SIGKILL);
      int status;
      ::waitpid(pid, &status, 0);
    }
    ::close(read_fd);
    ::close(write_fd);
    ::close(release_read);
    ::close(release_write);
  }
};

} // namespace
namespace {
std::vector<std::int64_t> row_counts(temporary& t) {
  sqlite3* db = nullptr;
  REQUIRE(sqlite3_open_v2((t.path / "ledger").c_str(), &db, SQLITE_OPEN_READONLY, nullptr) == SQLITE_OK);
  std::vector<std::int64_t> result;
  for (const auto table : {"ledger_meta", "records", "tree_edges", "revisions", "events", "current_state", "requests"}) {
    sqlite3_stmt* statement = nullptr;
    REQUIRE(sqlite3_prepare_v2(db, std::format("SELECT count(*) FROM {}", table).c_str(), -1, &statement, nullptr) == SQLITE_OK);
    REQUIRE(sqlite3_step(statement) == SQLITE_ROW);
    result.push_back(sqlite3_column_int64(statement, 0));
    sqlite3_finalize(statement);
  }
  REQUIRE(sqlite3_close(db) == SQLITE_OK);
  return result;
}
// The verified opener rejects a crash journal; only this explicit scratch recovery may repair it.
bool acceptance_state(temporary& t) {
  auto opened = open_existing(t.path / "ledger");
  if (!opened && opened.error().code == format_error_code::recovery_required) {
    auto recovered = recover_quiescent(t.path / "ledger");
    INFO((recovered ? "recovery succeeded" : recovered.error().message));
    REQUIRE(recovered);
    opened = open_existing(t.path / "ledger");
  }
  REQUIRE(opened);
  auto head = opened->read_head();
  REQUIRE(head);
  auto request = opened->read_request("fault");
  REQUIRE(request);
  const bool committed = request->has_value();
  CHECK(head->revision.has_value() == committed);
  CHECK(head->last_sequence == (committed ? 2 : 1));
  auto revision = opened->read_revision(1);
  REQUIRE(revision);
  CHECK(revision->has_value() == committed);
  auto event = opened->read_event(2);
  REQUIRE(event);
  CHECK(event->has_value() == committed);
  if (committed) {
    auto decoded = decode_load_result((**request).result);
    REQUIRE(decoded);
    CHECK(decoded->revision == 1);
    CHECK(decoded->event == 2);
  }
  return committed;
}
void retry(temporary& t, const request_intent& request, const prepared_acceptance& prepared, bool committed) {
  auto result = complete_acceptance(t.path / "ledger", request, prepared);
  REQUIRE(result);
  CHECK(result->replayed == committed);
  const auto saved = result->result;
  REQUIRE(acceptance_state(t));
  const auto counts = row_counts(t);
  CHECK(counts[3] == 1);
  CHECK(counts[4] == 2);
  CHECK(counts[6] == 1);
  auto again = complete_acceptance(t.path / "ledger", request, prepared);
  REQUIRE(again);
  CHECK(again->replayed);
  CHECK(again->result == saved);
  REQUIRE(acceptance_state(t));
  CHECK(row_counts(t) == counts);
}
} // namespace
TEST_CASE("D8 complete acceptance SQLITE_FULL enumerates journal and database writes with reusable request",
          "[atomic_acceptance][d8]") {
  const int format = GENERATE(1, 2);
  INFO(std::format("format={}", format));
  for (bool journal : {true, false}) {
    int  failures    = 0;
    bool reached_end = false;
    for (int ordinal = 0; ordinal < 128; ++ordinal) {
      INFO(std::format("journal={} ordinal={}", journal, ordinal));
      temporary t(format);
      inputs    i;
      REQUIRE((format == 2 ? initialize_new_v2 : initialize_new)(t.path / "ledger", "quota"));
      auto       candidate = prepare(t, i);
      auto       request   = intent(t, "fault", {});
      const auto before    = row_counts(t);
      fault_vfs  vfs;
      vfs.journal   = journal;
      vfs.remaining = ordinal;
      vfs.armed     = true;
      auto result   = complete_acceptance(t.path / "ledger", request, candidate);
      vfs.armed     = false;
      std::println("D8 SQLITE_FULL format={} journal={} ordinal={} hits={} success={}", format, journal, ordinal, vfs.hits,
                   bool(result));
      if (!vfs.hits) {
        REQUIRE(result);
        reached_end = true;
        break;
      }
      ++failures;
      REQUIRE_FALSE(result);
      CHECK(result.error().code == format_error_code::storage_full);
      REQUIRE_FALSE(acceptance_state(t));
      CHECK(row_counts(t) == before);
      retry(t, request, candidate, false);
    }
    CHECK(failures > 0);
    CHECK(reached_end);
  }
}
TEST_CASE("D8 complete acceptance process death at write sync and returned completion stays atomic", "[atomic_acceptance][d8]") {
  const int format = GENERATE(1, 2);
  INFO(std::format("format={}", format));
  for (int boundary : {0, 1, 2, 3, 4})
    DYNAMIC_SECTION("boundary " << boundary) {
      INFO(std::format("boundary={}", boundary));
      temporary t(format);
      inputs    i;
      REQUIRE((format == 2 ? initialize_new_v2 : initialize_new)(t.path / "ledger", "quota"));
      auto       candidate    = prepare(t, i);
      auto       request      = intent(t, "fault", {});
      const auto initial_main = read_bytes(t.path / "ledger");
      child      process;
      if (process.pid == 0) {
        fault_vfs vfs;
        if (boundary < 4) {
          vfs.journal = boundary == 0 || boundary == 1;
          vfs.sync    = boundary == 1 || boundary == 3;
          vfs.armed   = true;
          vfs.barrier = process.write_fd;
        }
        auto result = complete_acceptance(t.path / "ledger", request, candidate);
        if (!result)
          ::_exit(95);
        process.notify();
        for (;;)
          ::pause();
      }
      process.ready();
      process.kill_wait();
      if (boundary < 2) {
        // Conservative sidecar refusal is the approved contract for a non-hot remnant.
        // No supported automated recovery/retry is claimed for these two boundaries.
        CHECK(read_bytes(t.path / "ledger") == initial_main);
        auto snapshot = [&] {
          std::map<std::string, std::string> files;
          for (const auto& entry : std::filesystem::recursive_directory_iterator(t.path))
            if (entry.is_regular_file())
              files[entry.path().string()] = read_bytes(entry.path());
          return files;
        };
        const auto before = snapshot();
        auto       opened = open_existing(t.path / "ledger");
        REQUIRE_FALSE(opened);
        CHECK(opened.error().code == format_error_code::recovery_required);
        CHECK(snapshot() == before);
        auto recovered = recover_quiescent(t.path / "ledger");
        REQUIRE_FALSE(recovered);
        CHECK(recovered.error().code == format_error_code::corrupt);
        CHECK(recovered.error().message == "Recovery requires a valid hot journal header");
        CHECK(snapshot() == before);
        auto retried = complete_acceptance(t.path / "ledger", request, candidate);
        REQUIRE_FALSE(retried);
        CHECK(retried.error().code == format_error_code::recovery_required);
        CHECK(snapshot() == before);
        std::println("D8 process format={} boundary={} unchanged main/non-hot sidecar; recovery and retry deliberately refused",
                     format, boundary);
      } else {
        const bool committed = acceptance_state(t);
        std::println("D8 process format={} boundary={} committed={}", format, boundary, committed);
        CHECK(committed == (boundary == 4));
        retry(t, request, candidate, committed);
      }
    }
}
