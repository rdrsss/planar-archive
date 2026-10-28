// Scratch-only native storage checks. No installed or operator databases are used.
#include <catch2/catch_test_macros.hpp>
#include <fcntl.h>
#include <signal.h>
#include <sqlite3.h>
#include <sys/wait.h>
#include <unistd.h>
import std;
import planar.engine.ledger.storage;
import planar.engine.ledger.format;
using namespace planar::engine::ledger;
namespace hist = planar::engine::history;
namespace {
struct scratch {
  std::filesystem::path root;
  scratch() {
    char  pattern[] = "/tmp/planar-storage-XXXXXX";
    auto* made      = ::mkdtemp(pattern);
    REQUIRE(made);
    root = std::filesystem::canonical(made);
  }
  ~scratch() {
    std::filesystem::remove_all(root);
  }
};
std::string bytes(const std::filesystem::path& path) {
  std::ifstream in(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(in), {}};
}
void initialized(const std::filesystem::path& path) {
  REQUIRE(initialize_new(path, "main_ws"));
}
hist::record sample() {
  return hist::file_record{420, "storage sample"};
}
constexpr std::string_view unavailable = "Unavailable record transaction";
} // namespace
static_assert(std::is_nothrow_move_constructible_v<write_transaction>);
static_assert(std::is_nothrow_move_assignable_v<write_transaction>);
static_assert(!std::is_copy_constructible_v<write_transaction>);
static_assert(!std::is_copy_assignable_v<write_transaction>);
static_assert(std::is_nothrow_move_constructible_v<read_snapshot>);
static_assert(std::is_nothrow_move_assignable_v<read_snapshot>);
static_assert(!std::is_copy_constructible_v<read_snapshot>);
static_assert(!std::is_copy_assignable_v<read_snapshot>);
namespace {
enum class child_kind { writer, writer_with_shm, shared_only };
constexpr std::string_view sidecar_refusal = "Ledger sidecar requires separately audited recovery";
using steady                               = std::chrono::steady_clock;
// Child bodies use only the raw SQLite C API, POSIX calls and _exit: no Catch macros, no exceptions, no return to the test.
// POSIX record locks belong to a process, so every competing lock holder is a forked child, never a second connection here.
[[noreturn]] void run_child(const std::string& path, int hold_ms, child_kind kind, int ready_fd) {
  sqlite3* db = nullptr;
  int      rc = sqlite3_open_v2(path.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr);
  if (rc == SQLITE_OK) {
    const char* opening = kind == child_kind::shared_only ? "BEGIN; SELECT count(*) FROM sqlite_master" : "BEGIN IMMEDIATE";
    rc                  = sqlite3_exec(db, opening, nullptr, nullptr, nullptr);
  }
  // The first page write of this transaction creates the rollback journal.
  if (rc == SQLITE_OK && kind != child_kind::shared_only)
    rc = sqlite3_exec(db, "CREATE TABLE c01_hold(x)", nullptr, nullptr, nullptr);
  const char ready = rc == SQLITE_OK ? 'r' : 'f';
  (void)::write(ready_fd, &ready, 1);
  if (rc != SQLITE_OK) {
    sqlite3_close(db);
    ::_exit(1);
  }
  if (kind == child_kind::writer_with_shm) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // A plain -shm file, not -wal: a -wal would switch the connection to WAL before the post-lock check runs.
    const int shm = ::open((path + "-shm").c_str(), O_CREAT | O_WRONLY | O_CLOEXEC, 0600);
    if (shm >= 0)
      ::close(shm);
    std::this_thread::sleep_for(std::chrono::milliseconds(hold_ms - 1000));
  } else {
    std::this_thread::sleep_for(std::chrono::milliseconds(hold_ms));
  }
  // Never COMMIT: the hold leaves the ledger byte-identical.
  sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
  sqlite3_close(db);
  ::_exit(0);
}
// Holds a BEGIN IMMEDIATE (or SHARED-only) transaction in a forked child for hold_ms, or until killed.
class live_writer {
public:
  live_writer(const std::filesystem::path& path, int hold_ms, child_kind kind) : _path(path.string()) {
    int ready[2] = {-1, -1};
    REQUIRE(::pipe(ready) == 0);
    _pid = ::fork();
    REQUIRE(_pid >= 0);
    if (_pid == 0) {
      ::close(ready[0]);
      run_child(_path, hold_ms, kind, ready[1]);
    }
    ::close(ready[1]);
    char       answer = 0;
    const auto n      = ::read(ready[0], &answer, 1);
    ::close(ready[0]);
    _started = n == 1 && answer == 'r';
    if (_started && kind != child_kind::shared_only)
      _started = std::filesystem::exists(_path + "-journal");
  }
  live_writer(const live_writer&)            = delete;
  live_writer& operator=(const live_writer&) = delete;
  ~live_writer() {
    kill_now();
  }
  bool started() const {
    return _started;
  }
  void kill_now() {
    if (_pid <= 0)
      return;
    ::kill(_pid, SIGKILL);
    reap();
  }
  int wait() {
    if (_pid <= 0)
      return -1;
    return reap();
  }

private:
  int reap() {
    int status = 0;
    ::waitpid(_pid, &status, 0);
    _pid = -1;
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
  }
  std::string _path;
  pid_t       _pid     = -1;
  bool        _started = false;
};
// Kills and reaps a forked child on every exit path, including a failed REQUIRE.
struct reaper {
  pid_t pid = -1;
  ~reaper() {
    if (pid > 0) {
      ::kill(pid, SIGKILL);
      int status = 0;
      ::waitpid(pid, &status, 0);
    }
  }
};
// Child body for the probe test: one BEGIN IMMEDIATE without any busy wait. Zero only when RESERVED is free.
int reserved_attempt(const std::string& path) {
  sqlite3* db = nullptr;
  if (sqlite3_open_v2(path.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr) != SQLITE_OK) {
    sqlite3_close(db);
    return 3;
  }
  sqlite3_busy_timeout(db, 0);
  const int rc = sqlite3_exec(db, "BEGIN IMMEDIATE", nullptr, nullptr, nullptr);
  if (rc == SQLITE_OK)
    sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
  sqlite3_close(db);
  return rc == SQLITE_OK ? 0 : 10 + rc;
}
// Runs the public refusal entry point and checks the exact sidecar refusal; returns how long the refusal took.
template <class Attempt> steady::duration expect_sidecar_refusal(std::string_view label, Attempt&& attempt) {
  INFO(label);
  const auto start  = steady::now();
  auto       result = attempt();
  const auto spent  = steady::now() - start;
  REQUIRE_FALSE(result);
  CHECK(result.error().code == format_error_code::recovery_required);
  CHECK(result.error().message == sidecar_refusal);
  return spent;
}
// Reports -shm creation through the public read guard seam: verify() creates the sidecar after the open_read check.
class shm_on_verify final : public storage_read_guard {
public:
  explicit shm_on_verify(const std::filesystem::path& ledger) : _shm(ledger.string() + "-shm") {
  }
  read_gate_status bind(int) override {
    return read_gate_status::ready;
  }
  read_gate_status verify() override {
    const int fd = ::open(_shm.c_str(), O_CREAT | O_WRONLY | O_CLOEXEC, 0600);
    if (fd < 0)
      return read_gate_status::refused;
    ::close(fd);
    return read_gate_status::ready;
  }

private:
  std::string _shm;
};
// Replaces the bound descriptor with a write-only duplicate of the same ledger on the first verify() call, so the
// RESERVED probe's read lock fails with EBADF after the gated open has begun.
class fd_swap_on_verify final : public storage_read_guard {
public:
  explicit fd_swap_on_verify(const std::filesystem::path& ledger) : _ledger(ledger.string()) {
  }
  read_gate_status bind(int descriptor) override {
    _held = descriptor;
    return read_gate_status::ready;
  }
  read_gate_status verify() override {
    if (_swapped)
      return read_gate_status::ready;
    const int wronly = ::open(_ledger.c_str(), O_WRONLY | O_CLOEXEC);
    if (wronly < 0)
      return read_gate_status::refused;
    const int moved = ::dup2(wronly, _held);
    ::close(wronly);
    if (moved < 0)
      return read_gate_status::refused;
    _swapped = true;
    return read_gate_status::ready;
  }

private:
  std::string _ledger;
  int         _held    = -1;
  bool        _swapped = false;
};
// Observes the RESERVED byte from a forked child while the gated open is in flight: verify() runs after the probe has
// finished and before SQLite has released its own locks, so a leaked probe lock is still held by this process.
class reserved_observer final : public storage_read_guard {
public:
  explicit reserved_observer(const std::filesystem::path& ledger) : _ledger(ledger.string()) {
  }
  read_gate_status bind(int) override {
    return read_gate_status::ready;
  }
  read_gate_status verify() override {
    if (_observed)
      return read_gate_status::ready;
    _observed       = true;
    const pid_t pid = ::fork();
    if (pid < 0)
      return read_gate_status::refused;
    if (pid == 0) {
      const int fd = ::open(_ledger.c_str(), O_RDWR | O_CLOEXEC);
      if (fd < 0)
        ::_exit(3);
      struct flock lock{};
      lock.l_type   = F_WRLCK;
      lock.l_whence = SEEK_SET;
      lock.l_start  = 0x40000000 + 1;
      lock.l_len    = 1;
      ::_exit(::fcntl(fd, F_SETLK, &lock) == 0 ? 0 : 10);
    }
    int status = 0;
    if (::waitpid(pid, &status, 0) != pid || !WIFEXITED(status))
      return read_gate_status::refused;
    _child_status = WEXITSTATUS(status);
    return read_gate_status::ready;
  }
  int child_status() const noexcept {
    return _child_status;
  }

private:
  std::string _ledger;
  bool        _observed     = false;
  int         _child_status = -1;
};
} // namespace
TEST_CASE("storage write transaction matches the record writer byte for byte", "[storage]") {
  scratch    f;
  const auto direct = f.root / "direct";
  const auto legacy = f.root / "records";
  initialized(direct);
  initialized(legacy);
  {
    auto tx = begin_write(direct);
    REQUIRE(tx);
    REQUIRE(tx->intern_value(sample()));
    REQUIRE(tx->verify_before_commit(commit_site::record_writer));
    REQUIRE(tx->commit(commit_site::record_writer));
    // The transaction is unavailable after commit.
    auto again = tx->read_head();
    REQUIRE_FALSE(again);
    CHECK(again.error().code == format_error_code::invalid_argument);
    CHECK(again.error().message == unavailable);
  }
  {
    auto writer = begin_records(legacy);
    REQUIRE(writer);
    REQUIRE(writer->intern(sample()));
    REQUIRE(writer->commit());
  }
  auto a = open_existing(direct);
  auto b = open_existing(legacy);
  REQUIRE(a);
  REQUIRE(b);
  auto left  = a->read_record(1);
  auto right = b->read_record(1);
  REQUIRE(left);
  REQUIRE(right);
  CHECK(left->reference.id == right->reference.id);
  CHECK(left->reference.digest == right->reference.digest);
  auto x = planar::engine::ledger::types::encode_stored_record(*left);
  auto y = planar::engine::ledger::types::encode_stored_record(*right);
  REQUIRE(x);
  REQUIRE(y);
  CHECK(*x == *y);
}
TEST_CASE("storage abort and destruction roll back", "[storage]") {
  scratch    f;
  const auto path = f.root / "db";
  initialized(path);
  const auto before = bytes(path);
  {
    auto tx = begin_write(path);
    REQUIRE(tx);
    REQUIRE(tx->intern_value(sample()));
    tx->abort();
    tx->abort();
    CHECK_FALSE(tx->read_head());
  }
  CHECK(bytes(path) == before);
  {
    auto tx = begin_write(path);
    REQUIRE(tx);
    REQUIRE(tx->intern_value(sample()));
  }
  CHECK(bytes(path) == before);
  auto reopened = open_existing(path);
  REQUIRE(reopened);
  CHECK_FALSE(reopened->read_record(1));
}
TEST_CASE("storage failed operation poisons and rolls back the transaction", "[storage]") {
  scratch    f;
  const auto path = f.root / "db";
  initialized(path);
  const auto before = bytes(path);
  {
    auto tx = begin_write(path);
    REQUIRE(tx);
    REQUIRE(tx->intern_value(sample()));
    hist::record_child bogus;
    bogus.name  = "a";
    bogus.id    = 999;
    auto failed = tx->intern_value(hist::directory_record{{bogus}});
    REQUIRE_FALSE(failed);
    CHECK(failed.error().code == format_error_code::invalid_argument);
    CHECK(failed.error().message == "Invalid record child reference");
    auto check = [](const auto& result) {
      REQUIRE_FALSE(result);
      CHECK(result.error().code == format_error_code::invalid_argument);
      CHECK(result.error().message == unavailable);
    };
    check(tx->read_head());
    check(tx->intern_value(sample()));
    check(tx->read_record(1));
    check(tx->verify_before_commit(commit_site::record_writer));
    check(tx->commit(commit_site::record_writer));
  }
  CHECK(bytes(path) == before);
  auto reopened = open_existing(path);
  REQUIRE(reopened);
  CHECK_FALSE(reopened->read_record(1));
}
TEST_CASE("storage entry and precommit identity checks are separate and site specific", "[storage]") {
  struct variant {
    commit_site      site;
    std::string_view message;
  };
  const std::array<variant, 2> variants{variant{commit_site::record_writer, "Ledger identity changed before commit"},
                                        variant{commit_site::acceptance, "Ledger identity changed before acceptance commit"}};
  for (const auto& wanted : variants) {
    scratch    f;
    const auto path  = f.root / "db";
    const auto moved = f.root / "moved";
    initialized(path);
    auto tx = begin_write(path);
    REQUIRE(tx);
    // Entry verification already succeeded; replacing the named file now is observable only at the precommit check.
    std::filesystem::rename(path, moved);
    auto verified = tx->verify_before_commit(wanted.site);
    REQUIRE_FALSE(verified);
    CHECK(verified.error().code == format_error_code::unsafe_path);
    CHECK(verified.error().message == wanted.message);
    auto later = tx->commit(wanted.site);
    REQUIRE_FALSE(later);
    CHECK(later.error().message == unavailable);
  }
}
TEST_CASE("storage begin_write refuses a missing ledger", "[storage]") {
  scratch f;
  auto    tx = begin_write(f.root / "missing");
  REQUIRE_FALSE(tx);
  CHECK(tx.error().code == format_error_code::not_found);
}
TEST_CASE("storage request miss does not poison the transaction", "[storage]") {
  scratch    f;
  const auto path = f.root / "db";
  initialized(path);
  auto tx = begin_write(path);
  REQUIRE(tx);
  auto miss = tx->read_request("absent-request");
  REQUIRE(miss);
  CHECK_FALSE(*miss);
  auto head = tx->read_head();
  REQUIRE(head);
  CHECK_FALSE(head->revision);
  CHECK(head->last_sequence == 1);
  CHECK(tx->intern_value(sample()));
  tx->abort();
}
TEST_CASE("storage begin_write waits out a transient live writer journal", "[storage]") {
  scratch    f;
  const auto path = f.root / "ledger";
  initialized(path);
  // A real child holds BEGIN IMMEDIATE and a page write for about 2.5 s; begin_write waits on the busy handler.
  live_writer w(path, 2500, child_kind::writer);
  REQUIRE(w.started());
  const auto start = steady::now();
  auto       tx    = begin_write(path);
  const auto spent = steady::now() - start;
  REQUIRE(tx);
  CHECK(spent >= std::chrono::milliseconds(1500));
  CHECK(spent < std::chrono::milliseconds(5000));
  CHECK(tx->read_head());
  CHECK(w.wait() == 0);
}
TEST_CASE("storage begin_write still refuses a persistent journal", "[storage]") {
  scratch    f;
  const auto path = f.root / "ledger";
  initialized(path);
  {
    std::ofstream(f.root / "ledger-journal", std::ios::binary) << "x";
  }
  auto tx = begin_write(path);
  REQUIRE_FALSE(tx);
  CHECK(tx.error().code == format_error_code::recovery_required);
}
TEST_CASE("storage begin_write sidecar refusal message is byte exact", "[storage]") {
  scratch    f;
  const auto path = f.root / "ledger";
  initialized(path);
  {
    std::ofstream(f.root / "ledger-wal", std::ios::binary) << "x";
  }
  auto tx = begin_write(path);
  REQUIRE_FALSE(tx);
  CHECK(tx.error().code == format_error_code::recovery_required);
  CHECK(tx.error().message == "Ledger sidecar requires separately audited recovery");
}
TEST_CASE("storage begin_write surfaces a non-ENOENT sidecar stat failure", "[storage]") {
  scratch    f;
  const auto made = f.root / "ledger";
  // NAME_MAX is 255 bytes on macOS and Linux: a 250 byte name makes "<name>-journal" 258 bytes, so the sidecar stat
  // fails with ENAMETOOLONG (not ENOENT) and must surface as the shared io failure.
  const auto path = f.root / std::string(250, 'a');
  initialized(made);
  std::filesystem::rename(made, path);
  auto tx = begin_write(path);
  REQUIRE_FALSE(tx);
  CHECK(tx.error().code == format_error_code::io);
  CHECK(tx.error().message == "Ledger storage operation failed");
}
TEST_CASE("storage open_read and native no_sidecars surface a non-ENOENT sidecar stat failure", "[storage]") {
  scratch    f;
  const auto made = f.root / "ledger";
  // A 250 byte name makes "<name>-journal" exceed NAME_MAX, so each sidecar stat site fails with ENAMETOOLONG.
  const auto path = f.root / std::string(250, 'a');
  initialized(made);
  std::filesystem::rename(made, path);
  for (auto access : {ledger_access::read_only, ledger_access::read_write}) {
    auto snap = read_snapshot::open_read(path, access, {});
    REQUIRE_FALSE(snap);
    CHECK(snap.error().code == format_error_code::io);
    CHECK(snap.error().message == "Ledger storage operation failed");
  }
  auto where = native::locate(path);
  REQUIRE(where);
  auto result = native::no_sidecars(*where);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == format_error_code::io);
  CHECK(result.error().message == "Ledger storage operation failed");
}
TEST_CASE("storage no_sidecars refuses a leftover sidecar without delay", "[storage]") {
  scratch    f;
  const auto path = f.root / "ledger";
  initialized(path);
  {
    std::ofstream(f.root / "ledger-wal", std::ios::binary) << "x";
  }
  auto where = native::locate(path);
  REQUIRE(where);
  const auto start  = steady::now();
  auto       result = native::no_sidecars(*where);
  const auto spent  = steady::now() - start;
  REQUIRE_FALSE(result);
  CHECK(result.error().code == format_error_code::recovery_required);
  CHECK(result.error().message == sidecar_refusal);
  CHECK(spent < std::chrono::milliseconds(250));
}
TEST_CASE("storage no_sidecars ignores writer liveness", "[storage]") {
  scratch    f;
  const auto path = f.root / "ledger";
  initialized(path);
  live_writer w(path, 2500, child_kind::writer);
  REQUIRE(w.started());
  auto where = native::locate(path);
  REQUIRE(where);
  // The plain existence check refuses a journal even while its writer is alive; it never probes liveness.
  auto result = native::no_sidecars(*where);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == format_error_code::recovery_required);
  CHECK(result.error().message == sidecar_refusal);
  w.kill_now();
}
TEST_CASE("storage leftover journal refuses immediately on read and write paths", "[storage]") {
  scratch    f;
  const auto path    = f.root / "ledger";
  const auto journal = f.root / "ledger-journal";
  initialized(path);
  {
    std::ofstream(journal, std::ios::binary) << "x";
  }
  const auto before = bytes(journal);
  const auto read_only =
      expect_sidecar_refusal("open_read read_only", [&] { return read_snapshot::open_read(path, ledger_access::read_only, {}); });
  CHECK(read_only < std::chrono::milliseconds(500));
  const auto read_write = expect_sidecar_refusal("open_read read_write",
                                                 [&] { return read_snapshot::open_read(path, ledger_access::read_write, {}); });
  CHECK(read_write < std::chrono::milliseconds(500));
  const auto writing = expect_sidecar_refusal("begin_write", [&] { return begin_write(path); });
  CHECK(writing < std::chrono::milliseconds(500));
  CHECK(bytes(journal) == before);
}
TEST_CASE("storage live writer journal does not refuse the read path", "[storage]") {
  scratch    f;
  const auto path = f.root / "ledger";
  initialized(path);
  live_writer w(path, 2500, child_kind::writer);
  REQUIRE(w.started());
  auto snap = read_snapshot::open_read(path, ledger_access::read_only, {});
  REQUIRE(snap);
  CHECK(std::filesystem::exists(f.root / "ledger-journal"));
  w.kill_now();
}
TEST_CASE("storage SHARED-only reader does not excuse a leftover journal", "[storage]") {
  scratch    f;
  const auto path = f.root / "ledger";
  initialized(path);
  // The child keeps only a SHARED read transaction: no RESERVED lock and no journal.
  live_writer reader(path, 2500, child_kind::shared_only);
  REQUIRE(reader.started());
  {
    std::ofstream(f.root / "ledger-journal", std::ios::binary) << "x";
  }
  expect_sidecar_refusal("open_read read_only", [&] { return read_snapshot::open_read(path, ledger_access::read_only, {}); });
  expect_sidecar_refusal("open_read read_write", [&] { return read_snapshot::open_read(path, ledger_access::read_write, {}); });
  expect_sidecar_refusal("begin_write", [&] { return begin_write(path); });
  reader.kill_now();
}
TEST_CASE("storage writable open tolerates a live writer journal", "[storage]") {
  scratch    f;
  const auto path = f.root / "ledger";
  initialized(path);
  live_writer w(path, 2500, child_kind::writer);
  REQUIRE(w.started());
  auto snap = read_snapshot::open_read(path, ledger_access::read_write, {});
  REQUIRE(snap);
  CHECK(std::filesystem::exists(f.root / "ledger-journal"));
  w.kill_now();
}
TEST_CASE("storage killed writer leaves a genuine hot journal that still refuses", "[storage]") {
  scratch    f;
  const auto path    = f.root / "ledger";
  const auto journal = f.root / "ledger-journal";
  initialized(path);
  live_writer w(path, 60000, child_kind::writer);
  REQUIRE(w.started());
  w.kill_now();
  REQUIRE(std::filesystem::exists(journal));
  const auto before = bytes(journal);
  expect_sidecar_refusal("open_read read_only", [&] { return read_snapshot::open_read(path, ledger_access::read_only, {}); });
  expect_sidecar_refusal("open_read read_write", [&] { return read_snapshot::open_read(path, ledger_access::read_write, {}); });
  expect_sidecar_refusal("begin_write", [&] { return begin_write(path); });
  CHECK(std::filesystem::exists(journal));
  CHECK(bytes(journal) == before);
}
TEST_CASE("storage live writer does not excuse a wal or shm sidecar", "[storage]") {
  for (const auto suffix : {std::string_view("-wal"), std::string_view("-shm")}) {
    INFO(suffix);
    scratch    f;
    const auto path = f.root / "ledger";
    initialized(path);
    live_writer w(path, 2500, child_kind::writer);
    REQUIRE(w.started());
    {
      std::ofstream(f.root / ("ledger" + std::string(suffix)), std::ios::binary) << "x";
    }
    expect_sidecar_refusal("open_read read_only", [&] { return read_snapshot::open_read(path, ledger_access::read_only, {}); });
    expect_sidecar_refusal("begin_write", [&] { return begin_write(path); });
    w.kill_now();
  }
}
TEST_CASE("storage begin_write refuses a sidecar that appears while waiting for the writer lock", "[storage]") {
  scratch    f;
  const auto path = f.root / "ledger";
  initialized(path);
  // The child creates ledger-shm about 1 s into its 2.5 s hold; begin_write must refuse only after the lock wait.
  live_writer w(path, 2500, child_kind::writer_with_shm);
  REQUIRE(w.started());
  const auto start = steady::now();
  auto       tx    = begin_write(path);
  const auto spent = steady::now() - start;
  REQUIRE_FALSE(tx);
  CHECK(tx.error().code == format_error_code::recovery_required);
  CHECK(tx.error().message == sidecar_refusal);
  CHECK(spent >= std::chrono::milliseconds(1500));
  CHECK(std::filesystem::exists(f.root / "ledger-shm"));
  CHECK_FALSE(std::filesystem::exists(f.root / "ledger-wal"));
  CHECK(w.wait() == 0);
}
TEST_CASE("storage writable open refuses a sidecar created after the read check", "[storage]") {
  scratch    f;
  const auto path = f.root / "ledger";
  initialized(path);
  // verify() creates ledger-shm only after the open_read check at the read path has passed.
  auto guard = std::make_shared<shm_on_verify>(path);
  auto snap  = read_snapshot::open_read(path, ledger_access::read_write, guard);
  REQUIRE_FALSE(snap);
  CHECK(snap.error().code == format_error_code::recovery_required);
  CHECK(snap.error().message == sidecar_refusal);
  CHECK(std::filesystem::exists(f.root / "ledger-shm"));
}
TEST_CASE("storage probe releases the writer lock on the no-journal path", "[storage]") {
  scratch    f;
  const auto path = f.root / "ledger";
  initialized(path);
  // The child is forked before the parent opens anything, then waits for a go byte so its BEGIN IMMEDIATE runs
  // while the parent's read snapshot (retaining the probed descriptor) is still open.
  int gate[2] = {-1, -1};
  REQUIRE(::pipe(gate) == 0);
  const pid_t pid = ::fork();
  REQUIRE(pid >= 0);
  if (pid == 0) {
    ::close(gate[1]);
    char go = 0;
    if (::read(gate[0], &go, 1) != 1)
      ::_exit(2);
    ::_exit(reserved_attempt(path.string()));
  }
  ::close(gate[0]);
  reaper child{pid};
  auto   snap = read_snapshot::open_read(path, ledger_access::read_only, {});
  REQUIRE(snap);
  REQUIRE(::write(gate[1], "g", 1) == 1);
  ::close(gate[1]);
  int status = 0;
  REQUIRE(::waitpid(pid, &status, 0) == pid);
  child.pid = -1;
  REQUIRE(WIFEXITED(status));
  CHECK(WEXITSTATUS(status) == 0);
  CHECK(snap->read_head());
}
TEST_CASE("storage probe lock is gone before the gated open reads", "[storage]") {
  scratch    f;
  const auto path = f.root / "ledger";
  initialized(path);
  auto guard = std::make_shared<reserved_observer>(path);
  auto snap  = read_snapshot::open_read(path, ledger_access::read_only, guard);
  REQUIRE(snap);
  // The child's write lock on the RESERVED byte succeeds only when the probe released its read lock.
  CHECK(guard->child_status() == 0);
}
TEST_CASE("storage probe fcntl failure surfaces as io", "[storage]") {
  scratch    f;
  const auto path = f.root / "ledger";
  initialized(path);
  auto guard = std::make_shared<fd_swap_on_verify>(path);
  auto snap  = read_snapshot::open_read(path, ledger_access::read_write, guard);
  REQUIRE_FALSE(snap);
  CHECK(snap.error().code == format_error_code::io);
  CHECK(snap.error().message == "Ledger storage operation failed");
}
