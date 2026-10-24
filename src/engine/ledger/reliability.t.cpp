// Storage atomicity and fault checks use symbolic rows, isolated files and a scoped test VFS.
#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <poll.h>
#include <signal.h>
#include <sqlite3.h>
#include <sys/wait.h>
#include <unistd.h>
import std;
import planar.engine.ledger.sqlite;
using namespace planar::engine::ledger;
namespace {
struct scratch {
  std::filesystem::path root;
  scratch() {
    char name[] = "/tmp/planar-reliability-XXXXXX";
    REQUIRE(::mkdtemp(name));
    root = std::filesystem::canonical(name);
  }
  ~scratch() {
    std::filesystem::remove_all(root);
  }
};
connection open(const std::filesystem::path& path, int timeout = 0) {
  auto db = connection::open(path, open_mode::scratch_create, timeout);
  REQUIRE(db);
  return std::move(*db);
}
void sql(connection& db, std::string_view text) {
  auto s = db.prepare(text);
  REQUIRE(s);
  auto result = s->step();
  if (!result)
    INFO(result.error().message);
  REQUIRE(result == step_result::done);
}
std::string state(connection& db) {
  auto query = db.prepare("SELECT id, value FROM atomicity ORDER BY id");
  REQUIRE(query);
  std::string out;
  while (true) {
    auto stepped = query->step();
    REQUIRE(stepped);
    if (*stepped == step_result::done)
      break;
    out += std::format("{}:{};", query->integer(0).value().value(), query->text(1).value().value());
  }
  return out;
}
void seed(const std::filesystem::path& path) {
  auto db = open(path);
  sql(db, "CREATE TABLE atomicity(id INTEGER PRIMARY KEY, value TEXT NOT NULL)");
  sql(db, "INSERT INTO atomicity VALUES(1,'old'),(2,'old')");
}
void change(connection& db) {
  sql(db, "UPDATE atomicity SET value='new'");
  sql(db, "INSERT INTO atomicity VALUES(3,'new')");
}
constexpr auto old_state = "1:old;2:old;";
constexpr auto new_state = "1:new;2:new;3:new;";
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
        return SQLITE_IOERR_WRITE;
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
TEST_CASE("ledger sqlite configured full rolls back complete symbolic storage state", "[ledger][fault]") {
  scratch    f;
  const auto path = f.root / "db";
  seed(path);
  {
    auto db    = open(path);
    auto pages = db.prepare("PRAGMA page_count");
    REQUIRE(pages);
    REQUIRE(pages->step() == step_result::row);
    const auto limit = pages->integer(0).value().value();
    pages            = db.prepare(std::format("PRAGMA max_page_count={}", limit));
    REQUIRE(pages);
    REQUIRE(pages->step() == step_result::row);
    REQUIRE(pages->integer(0).value().value() == limit);
    pages   = db.prepare("SELECT 1");
    auto tx = db.begin();
    REQUIRE(tx);
    change(db);
    auto large = db.prepare("INSERT INTO atomicity VALUES(4,zeroblob(1048576))");
    REQUIRE(large);
    auto failed = large->step();
    REQUIRE_FALSE(failed);
    std::println("configured full primary={} extended={} pages={}", failed.error().code, failed.error().extended_code, limit);
    CHECK(failed.error().code == SQLITE_FULL);
    CHECK_FALSE(tx->commit());
    CHECK(state(db) == old_state);
    {
      auto before_retry = open(path);
      CHECK(state(before_retry) == old_state);
    }
    auto retry = db.begin();
    REQUIRE(retry);
    change(db);
    REQUIRE(retry->commit());
    CHECK(state(db) == new_state);
  }
  auto reopened = open(path);
  CHECK(state(reopened) == new_state);
}
TEST_CASE("ledger sqlite VFS write and sync errors never report commit and permit complete retry", "[ledger][fault]") {
  for (bool journal : {true, false})
    for (bool sync : {false, true}) {
      bool exhausted = false;
      // Enumerate every matching callback position through the first unhit, successful commit.
      for (int ordinal = 0; ordinal < 64; ++ordinal) {
        INFO(std::format("journal={} sync={} ordinal={}", journal, sync, ordinal));
        scratch    f;
        fault_vfs  vfs;
        const auto path = f.root / "db";
        seed(path);
        {
          auto db = open(path);
          auto tx = db.begin();
          REQUIRE(tx);
          change(db);
          vfs.journal              = journal;
          vfs.sync                 = sync;
          vfs.remaining            = ordinal;
          vfs.armed                = true;
          bool confirmed_noncommit = true;
          auto result              = tx->commit(confirmed_noncommit);
          std::println("COMMIT evidence journal={} sync={} ordinal={} success={} confirmed_noncommit={}", journal, sync, ordinal,
                       bool(result), confirmed_noncommit);
          vfs.armed = false;
          if (vfs.hits == 0) {
            REQUIRE(result);
            CHECK_FALSE(confirmed_noncommit);
            std::println("VFS enumeration journal={} sync={} fault_positions={}", journal, sync, ordinal);
            CHECK(state(db) == new_state);
            exhausted = true;
          } else {
            REQUIRE(vfs.hits == 1);
            REQUIRE_FALSE(result);
            CHECK(result.error().code == SQLITE_IOERR);
            CHECK(result.error().extended_code == (sync ? SQLITE_IOERR_FSYNC : SQLITE_IOERR_WRITE));
            CHECK(state(db) == old_state);
            {
              auto before_retry = open(path);
              CHECK(state(before_retry) == old_state);
            }
            auto retry = db.begin();
            REQUIRE(retry);
            change(db);
            REQUIRE(retry->commit());
          }
        }
        auto reopened = open(path);
        CHECK(state(reopened) == new_state);
        if (exhausted)
          break;
      }
      REQUIRE(exhausted);
    }
}
TEST_CASE("ledger sqlite online backup propagates actual destination VFS failure", "[ledger][fault]") {
  for (bool sync : {false, true}) {
    scratch   f;
    fault_vfs vfs;
    seed(f.root / "source");
    auto source = open(f.root / "source");
    auto target = open(f.root / "target");
    vfs.journal = false;
    vfs.sync    = sync;
    vfs.armed   = true;
    auto result = source.online_backup_to(target);
    REQUIRE(vfs.hits == 1);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == SQLITE_IOERR);
    CHECK(result.error().extended_code == (sync ? SQLITE_IOERR_FSYNC : SQLITE_IOERR_WRITE));
    CHECK(state(source) == old_state);
    REQUIRE(source.online_backup_to(target));
    CHECK(state(target) == old_state);
  }
}
TEST_CASE("ledger sqlite process termination brackets actual commit sync and returned commit", "[ledger][process]") {
  for (int boundary : {0, 1, 2, 3}) {
    INFO(std::format("boundary={}", boundary));
    scratch    f;
    const auto path = f.root / "db";
    seed(path);
    child process;
    if (process.pid == 0) {
      fault_vfs vfs;
      auto      db = open(path);
      auto      tx = db.begin();
      if (!tx)
        ::_exit(93);
      change(db);
      if (boundary == 0) {
        process.notify();
        for (;;)
          ::pause();
      }
      if (boundary == 1 || boundary == 2) {
        vfs.journal = boundary == 1;
        vfs.sync    = true;
        vfs.armed   = true;
        vfs.barrier = process.write_fd;
      }
      if (!tx->commit())
        ::_exit(94);
      process.notify();
      for (;;)
        ::pause();
    }
    process.ready();
    process.kill_wait();
    auto       db     = open(path);
    const auto actual = state(db);
    std::println("storage atomicity boundary={} state={}", boundary, actual);
    CHECK(actual == (boundary == 3 ? new_state : old_state));
    auto tx = db.begin();
    REQUIRE(tx);
    REQUIRE(tx->rollback());
  }
}
TEST_CASE("ledger sqlite separate writer process busy bound releases and retries", "[ledger][process]") {
  scratch    f;
  const auto path = f.root / "db";
  seed(path);
  child process;
  if (process.pid == 0) {
    auto db = open(path);
    auto tx = db.begin();
    if (!tx)
      ::_exit(95);
    change(db);
    process.notify();
    process.await_release();
    if (!tx->rollback())
      ::_exit(98);
    ::_exit(0);
  }
  process.ready();
  auto db      = open(path, 80);
  auto start   = std::chrono::steady_clock::now();
  auto busy    = db.begin();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
  REQUIRE_FALSE(busy);
  CHECK(busy.error().code == SQLITE_BUSY);
  std::println("separate writer busy elapsed_ms={} code={}", elapsed, busy.error().code);
  CHECK(elapsed >= 40);
  CHECK(elapsed < 1500);
  process.release();
  process.exit_wait();
  CHECK(state(db) == old_state);
  auto retry = db.begin();
  REQUIRE(retry);
  change(db);
  REQUIRE(retry->commit());
  CHECK(state(db) == new_state);
}
TEST_CASE("ledger sqlite separate reader makes commit busy and complete rollback reusable", "[ledger][process]") {
  scratch    f;
  const auto path = f.root / "db";
  seed(path);
  child process;
  if (process.pid == 0) {
    auto db = open(path);
    auto tx = db.begin(false);
    if (!tx)
      ::_exit(96);
    if (state(db) != old_state)
      ::_exit(97);
    process.notify();
    for (;;)
      ::pause();
  }
  process.ready();
  auto db = open(path, 80);
  auto tx = db.begin();
  REQUIRE(tx);
  change(db);
  const auto start               = std::chrono::steady_clock::now();
  bool       confirmed_noncommit = false;
  auto       failed              = tx->commit(confirmed_noncommit);
  CHECK(confirmed_noncommit);
  const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
  REQUIRE_FALSE(failed);
  CHECK(failed.error().code == SQLITE_BUSY);
  CHECK(elapsed < 1500);
  CHECK(state(db) == old_state);
  process.kill_wait();
  auto retry = db.begin();
  REQUIRE(retry);
  change(db);
  REQUIRE(retry->commit());
  CHECK(state(db) == new_state);
}

TEST_CASE("ledger sqlite handled rollback write fault recovers complete state and permits retry", "[ledger][fault]") {
  scratch    f;
  fault_vfs  vfs;
  const auto path = f.root / "db";
  seed(path);
  {
    auto db = open(path);
    sql(db, "UPDATE atomicity SET value=hex(zeroblob(40000))");
    const auto before = state(db);
    sql(db, "PRAGMA cache_size=1");
    auto tx = db.begin();
    REQUIRE(tx);
    sql(db, "UPDATE atomicity SET value=hex(zeroblob(40001))");
    vfs.journal = false;
    vfs.sync    = false;
    vfs.armed   = true;
    // SQLite's SQL ROLLBACK discards the Btree rollback status. Observe safety, not an unavailable return code.
    auto rolled = tx->rollback();
    REQUIRE(vfs.hits == 1);
    std::println("rollback API success after injected main write error={}", bool(rolled));
    REQUIRE(rolled);
    CHECK(bool(state(db) == before));
    {
      auto reopened = open(path);
      CHECK(bool(state(reopened) == before));
    }
    auto retry = db.begin();
    REQUIRE(retry);
    change(db);
    REQUIRE(retry->commit());
    CHECK(state(db) == new_state);
  }
  auto reopened = open(path);
  CHECK(state(reopened) == new_state);
}
