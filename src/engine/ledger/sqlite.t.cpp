// Scratch-only behavioral coverage for trusted SQLite primitives, not ledger validation.
#include <catch2/catch_test_macros.hpp>
#include <sqlite3.h>
#include <unistd.h>
import std;
import planar.engine.ledger.sqlite;
using namespace planar::engine::ledger;
namespace {
struct scratch {
  std::filesystem::path root;
  scratch() {
    char  pattern[] = "/tmp/planar-sqlite-XXXXXX";
    auto* made      = ::mkdtemp(pattern);
    REQUIRE(made);
    root = std::filesystem::canonical(made);
  }
  ~scratch() {
    std::filesystem::remove_all(root);
  }
  connection open(std::string_view name = "db", int timeout = 5000) {
    auto db = connection::open(root / name, open_mode::scratch_create, timeout);
    if (!db)
      INFO(db.error().message);
    REQUIRE(db);
    return std::move(*db);
  }
};
// Catch2 runs cases serially within this executable; restore process-global cwd before scratch cleanup.
struct scoped_cwd {
  std::filesystem::path previous = std::filesystem::current_path();
  explicit scoped_cwd(const std::filesystem::path& path) {
    std::filesystem::current_path(path);
  }
  ~scoped_cwd() {
    std::filesystem::current_path(previous);
  }
};
void execute(connection& db, std::string_view sql) {
  auto s = db.prepare(sql);
  REQUIRE(s);
  REQUIRE(s->step() == step_result::done);
}
std::int64_t count(connection& db, std::string_view table = "items") {
  auto query = db.prepare(std::format("SELECT count(*) FROM {}", table));
  REQUIRE(query);
  REQUIRE(query->step() == step_result::row);
  auto value = query->integer(0);
  REQUIRE(value);
  REQUIRE(value->has_value());
  return **value;
}
} // namespace
TEST_CASE("ledger sqlite owns binary text blob empty and null values", "[ledger]") {
  scratch files;
  auto    db = files.open();
  execute(db, "CREATE TABLE items(t TEXT,b BLOB,e TEXT,z BLOB,n)");
  auto insert = db.prepare("INSERT INTO items VALUES(?1,?2,?3,?4,?5)");
  REQUIRE(insert);
  std::string text("a\0b", 3), blob("\xff\0\x80", 3);
  REQUIRE(insert->bind_text(1, text));
  REQUIRE(insert->bind_blob(2, blob));
  REQUIRE(insert->bind_text(3, {}));
  REQUIRE(insert->bind_blob(4, {}));
  REQUIRE(insert->bind_null(5));
  text.assign("changed");
  blob.clear();
  REQUIRE(insert->step() == step_result::done);
  auto select = db.prepare("SELECT t,b,e,z,n FROM items");
  REQUIRE(select);
  CHECK_FALSE(select->text(0));
  REQUIRE(select->step() == step_result::row);
  auto saved = select->text(0);
  REQUIRE(saved);
  CHECK(**saved == std::string("a\0b", 3));
  CHECK(select->blob(1).value().value() == std::string("\xff\0\x80", 3));
  CHECK(select->text(2).value() == std::optional<std::string>(""));
  CHECK(select->blob(3).value() == std::optional<std::string>(""));
  CHECK_FALSE(select->text(4).value().has_value());
  CHECK_FALSE(select->integer(0));
  CHECK_FALSE(select->text(1));
  CHECK_FALSE(select->blob(-1));
  REQUIRE(select->step() == step_result::done);
  REQUIRE(select->reset());
  CHECK(**saved == std::string("a\0b", 3));
  REQUIRE(insert->reset());
  REQUIRE(insert->step() == step_result::done);
  auto cleared = db.prepare("SELECT t IS NULL AND b IS NULL AND e IS NULL AND z IS NULL AND n IS NULL FROM items WHERE rowid=2");
  REQUIRE(cleared);
  REQUIRE(cleared->step() == step_result::row);
  CHECK(cleared->integer(0).value().value() == 1);
}
TEST_CASE("ledger sqlite statements retain moved and destroyed connections", "[ledger]") {
  scratch files;
  auto    stmt = [&] {
    auto first  = files.open();
    auto second = std::move(first);
    CHECK_FALSE(first.prepare("SELECT 1"));
    execute(second, "CREATE TABLE items(n INTEGER)");
    return second.prepare("INSERT INTO items VALUES(7)");
  }();
  REQUIRE(stmt);
  auto moved = std::move(*stmt);
  CHECK_FALSE(stmt->step());
  REQUIRE(moved.step() == step_result::done);
  auto db = files.open();
  CHECK(count(db) == 1);
  auto replacement = db.prepare("SELECT 9");
  REQUIRE(replacement);
  moved = std::move(*replacement);
  REQUIRE(moved.step() == step_result::row);
  CHECK(moved.integer(0).value().value() == 9);
}
TEST_CASE("ledger sqlite transactions commit abandon and survive wrapper destruction", "[ledger]") {
  scratch files;
  auto    db = files.open();
  execute(db, "CREATE TABLE items(n INTEGER)");
  {
    auto tx = db.begin();
    REQUIRE(tx);
    execute(db, "INSERT INTO items VALUES(1)");
  }
  CHECK(count(db) == 0);
  {
    auto tx = db.begin();
    REQUIRE(tx);
    CHECK_FALSE(db.begin());
    execute(db, "INSERT INTO items VALUES(2)");
    auto moved = std::move(*tx);
    CHECK_FALSE(tx->commit());
    REQUIRE(moved.commit());
    CHECK_FALSE(moved.commit());
  }
  CHECK(count(db) == 1);
  auto retained = [&] {
    auto other = files.open();
    auto tx    = other.begin();
    REQUIRE(tx);
    execute(other, "INSERT INTO items VALUES(3)");
    return std::move(*tx);
  }();
  REQUIRE(retained.rollback());
  CHECK(count(db) == 1);
  auto other = files.open("other");
  execute(other, "CREATE TABLE items(n INTEGER)");
  auto replaced = db.begin();
  auto incoming = other.begin();
  REQUIRE(replaced);
  REQUIRE(incoming);
  execute(db, "INSERT INTO items VALUES(4)");
  execute(other, "INSERT INTO items VALUES(5)");
  *replaced = std::move(*incoming);
  REQUIRE(replaced->commit());
  CHECK_FALSE(incoming->commit());
  CHECK(count(db) == 1);
  CHECK(count(other) == 1);
}
TEST_CASE("ledger sqlite extended constraints rollback and permit retry", "[ledger]") {
  scratch files;
  auto    db = files.open();
  execute(db, "CREATE TABLE items(n INTEGER UNIQUE NOT NULL)");
  execute(db, "INSERT INTO items VALUES(1)");
  auto tx = db.begin();
  REQUIRE(tx);
  execute(db, "INSERT INTO items VALUES(2)");
  auto bad = db.prepare("INSERT INTO items VALUES(1)");
  REQUIRE(bad);
  auto failed = bad->step();
  REQUIRE_FALSE(failed);
  CHECK(failed.error().code == SQLITE_CONSTRAINT);
  CHECK(failed.error().extended_code == SQLITE_CONSTRAINT_UNIQUE);
  CHECK_FALSE(tx->commit());
  CHECK(count(db) == 1);
  auto next = db.begin();
  REQUIRE(next);
  execute(db, "INSERT INTO items VALUES(3)");
  // The invalid old guard cannot roll back this new transaction.
  CHECK_FALSE(tx->rollback());
  REQUIRE(next->commit());
  CHECK(count(db) == 2);
}
TEST_CASE("ledger sqlite failed deferred commit rolls back and releases writer", "[ledger]") {
  scratch files;
  auto    db = files.open();
  execute(db, "CREATE TABLE parents(id INTEGER PRIMARY KEY)");
  execute(db, "CREATE TABLE items(n INTEGER REFERENCES parents(id) DEFERRABLE INITIALLY DEFERRED)");
  auto tx = db.begin();
  REQUIRE(tx);
  execute(db, "INSERT INTO items VALUES(42)");
  auto committed = tx->commit();
  REQUIRE_FALSE(committed);
  CHECK(committed.error().code == SQLITE_CONSTRAINT);
  CHECK(committed.error().extended_code == SQLITE_CONSTRAINT_FOREIGNKEY);
  CHECK(count(db) == 0);
  auto other = files.open();
  auto retry = other.begin();
  REQUIRE(retry);
  execute(other, "INSERT INTO parents VALUES(42)");
  execute(other, "INSERT INTO items VALUES(42)");
  REQUIRE(retry->commit());
  CHECK(count(db) == 1);
}
TEST_CASE("ledger sqlite bounded busy refusal releases for retry", "[ledger]") {
  scratch files;
  auto    first = files.open();
  execute(first, "CREATE TABLE items(n)");
  auto second = files.open("db", 80);
  auto held   = first.begin();
  REQUIRE(held);
  execute(first, "INSERT INTO items VALUES(1)");
  const auto start   = std::chrono::steady_clock::now();
  auto       busy    = second.begin();
  const auto elapsed = std::chrono::steady_clock::now() - start;
  REQUIRE_FALSE(busy);
  CHECK(busy.error().code == SQLITE_BUSY);
  CHECK(elapsed >= std::chrono::milliseconds(40));
  CHECK(elapsed < std::chrono::seconds(2));
  REQUIRE(held->rollback());
  auto retry = second.begin();
  REQUIRE(retry);
  execute(second, "INSERT INTO items VALUES(2)");
  REQUIRE(retry->commit());
  CHECK(count(first) == 1);
}
TEST_CASE("ledger sqlite readonly existing mode never creates missing files", "[ledger]") {
  scratch files;
  CHECK_FALSE(connection::open(files.root / "absent", open_mode::read_only));
  CHECK_FALSE(connection::open(files.root / "absent", open_mode::read_write));
  CHECK_FALSE(std::filesystem::exists(files.root / "absent"));
  CHECK_FALSE(connection::open(files.root / "missing" / "db", open_mode::scratch_create));
  CHECK_FALSE(std::filesystem::exists(files.root / "missing"));
  auto db = files.open();
  execute(db, "CREATE TABLE items(n)");
  auto ro = connection::open(files.root / "db", open_mode::read_only);
  REQUIRE(ro);
  CHECK(count(*ro) == 0);
  auto write = ro->prepare("INSERT INTO items VALUES(1)");
  REQUIRE(write);
  auto denied = write->step();
  REQUIRE_FALSE(denied);
  CHECK(denied.error().code == SQLITE_READONLY);
  std::filesystem::create_symlink(files.root / "db", files.root / "link");
  CHECK_FALSE(connection::open(files.root / "link", open_mode::read_only));
  CHECK_FALSE(connection::open(files.root / "db", open_mode::read_only, -1));
  CHECK_FALSE(connection::open(files.root / "db", open_mode::read_only, 5001));
}
TEST_CASE("ledger sqlite bare memory filename existing modes refuse missing files", "[ledger][native_path]") {
  scratch    files;
  scoped_cwd cwd(files.root);
  for (auto mode : {open_mode::read_only, open_mode::read_write}) {
    auto missing = connection::open(":memory:", mode, 0);
    CHECK_FALSE(missing);
    if (!missing)
      CHECK(missing.error().code == SQLITE_CANTOPEN);
    CHECK_FALSE(std::filesystem::exists(":memory:"));
  }
}
TEST_CASE("ledger sqlite bare memory filename persists through native path spellings", "[ledger][native_path]") {
  scratch    files;
  scoped_cwd cwd(files.root);
  {
    auto db = connection::open(":memory:", open_mode::scratch_create, 0);
    REQUIRE(db);
    execute(*db, "CREATE TABLE items(n)");
    execute(*db, "INSERT INTO items VALUES(7)");
  }
  REQUIRE(std::filesystem::is_regular_file(":memory:"));
  for (const auto& path : {std::filesystem::path(":memory:"), std::filesystem::path("./:memory:"), files.root / ":memory:"}) {
    auto db = connection::open(path, open_mode::read_only, 0);
    REQUIRE(db);
    CHECK(count(*db) == 1);
  }
}
TEST_CASE("ledger sqlite URI looking native filenames remain persistent files", "[ledger][native_path]") {
  scratch                     files;
  scoped_cwd                  cwd(files.root);
  const std::filesystem::path name("file:literal?mode=memory&cache=shared");
  {
    auto db = connection::open(name, open_mode::scratch_create, 0);
    REQUIRE(db);
    execute(*db, "CREATE TABLE items(n)");
    execute(*db, "INSERT INTO items VALUES(9)");
  }
  REQUIRE(std::filesystem::is_regular_file(name));
  auto db = connection::open(files.root / name, open_mode::read_write, 0);
  REQUIRE(db);
  CHECK(count(*db) == 1);
  CHECK_FALSE(std::filesystem::exists("literal"));
  CHECK_FALSE(connection::open("", open_mode::scratch_create, 0));
  CHECK_FALSE(connection::open(std::string("bad\0name", 8), open_mode::scratch_create, 0));
  CHECK_FALSE(connection::open("missing/../db", open_mode::scratch_create, 0));
  CHECK_FALSE(connection::open("missing/db", open_mode::scratch_create, 0));
  CHECK_FALSE(std::filesystem::exists("missing"));
  std::filesystem::create_directory("directory");
  std::filesystem::create_directory_symlink("directory", "link");
  CHECK_FALSE(connection::open("link/../db", open_mode::scratch_create, 0));
  CHECK_FALSE(std::filesystem::exists("db"));
  auto traversed = connection::open("directory/../db", open_mode::scratch_create, 0);
  REQUIRE(traversed);
  CHECK(std::filesystem::is_regular_file("db"));
}
TEST_CASE("ledger sqlite unavailable cwd returns typed path failure", "[ledger][native_path]") {
  scratch files;
  std::filesystem::create_directory(files.root / "removed");
  scoped_cwd cwd(files.root / "removed");
  REQUIRE(std::filesystem::remove(files.root / "removed"));
  auto db = connection::open("db", open_mode::scratch_create, 0);
  REQUIRE_FALSE(db);
  CHECK(db.error().code == SQLITE_CANTOPEN);
  CHECK_FALSE(std::filesystem::exists(files.root / "db"));
}
TEST_CASE("ledger sqlite authorizer prevents manual transaction bypass", "[ledger]") {
  scratch files;
  auto    db = files.open();
  execute(db, "CREATE TABLE items(n)");
  for (const auto sql : {"BEGIN", "COMMIT", "ROLLBACK", "SAVEPOINT x", "RELEASE x", "ROLLBACK TO x"}) {
    auto denied = db.prepare(sql);
    REQUIRE_FALSE(denied);
    CHECK(denied.error().code == SQLITE_AUTH);
  }
  auto tx = db.begin();
  REQUIRE(tx);
  execute(db, "INSERT INTO items VALUES(1)");
  CHECK_FALSE(db.prepare("COMMIT"));
  CHECK_FALSE(tx->commit());
  CHECK(count(db) == 0);
  CHECK_FALSE(db.prepare("SELECT 1; SELECT 2"));
  CHECK_FALSE(db.prepare(std::string("SELECT 1\0; SELECT 2", 19)));
  CHECK_FALSE(db.prepare("-- comment only"));
  auto comment = db.prepare("SELECT 1; -- harmless tail");
  REQUIRE(comment);
  REQUIRE(comment->step() == step_result::row);
}
TEST_CASE("ledger sqlite binding failures and preparation errors abandon writes", "[ledger]") {
  scratch files;
  auto    db = files.open();
  execute(db, "CREATE TABLE items(n)");
  {
    auto tx = db.begin();
    REQUIRE(tx);
    execute(db, "INSERT INTO items VALUES(1)");
    auto s = db.prepare("SELECT ?1");
    REQUIRE(s);
    auto failure = s->bind_integer(2, 7);
    REQUIRE_FALSE(failure);
    CHECK(failure.error().code == SQLITE_RANGE);
    CHECK_FALSE(tx->commit());
  }
  CHECK(count(db) == 0);
  {
    auto tx = db.begin();
    REQUIRE(tx);
    execute(db, "INSERT INTO items VALUES(2)");
    auto failure = db.prepare("invalid syntax");
    REQUIRE_FALSE(failure);
    CHECK(failure.error().code == SQLITE_ERROR);
    CHECK_FALSE(tx->commit());
  }
  CHECK(count(db) == 0);
}

TEST_CASE("ledger sqlite online backup supports pinned snapshots and rejects invalid owners", "[ledger][backup]") {
  scratch f;
  auto    source      = connection::open(f.root / "source", open_mode::scratch_create, 0);
  auto    destination = connection::open(f.root / "target", open_mode::scratch_create, 0);
  REQUIRE(source);
  REQUIRE(destination);
  execute(*source, "CREATE TABLE items(n)");
  execute(*source, "INSERT INTO items VALUES(41)");
  auto snapshot = source->begin(false);
  REQUIRE(snapshot);
  CHECK(count(*source) == 1);
  auto copied = source->online_backup_to(*destination);
  REQUIRE(copied);
  CHECK(count(*destination) == 1);
  auto self = source->online_backup_to(*source);
  REQUIRE_FALSE(self);
  CHECK(self.error().code == SQLITE_MISUSE);
  auto tx = destination->begin();
  REQUIRE(tx);
  auto active = source->online_backup_to(*destination);
  REQUIRE_FALSE(active);
  CHECK(active.error().code == SQLITE_MISUSE);
  REQUIRE(tx->rollback());
  auto moved = std::move(*destination);
  CHECK_FALSE(source->online_backup_to(*destination));
  CHECK_FALSE(destination->online_backup_to(moved));
}

TEST_CASE("ledger sqlite commit evidence distinguishes success inactive and deferred rollback", "[ledger]") {
  scratch files;
  auto    db = files.open();
  execute(db, "PRAGMA foreign_keys=ON");
  execute(db, "CREATE TABLE parent(id INTEGER PRIMARY KEY)");
  execute(db, "CREATE TABLE child(id REFERENCES parent(id) DEFERRABLE INITIALLY DEFERRED)");
  auto tx = db.begin();
  REQUIRE(tx);
  execute(db, "INSERT INTO child VALUES(1)");
  bool confirmed_noncommit = false;
  auto failed              = tx->commit(confirmed_noncommit);
  REQUIRE_FALSE(failed);
  CHECK(failed.error().code == SQLITE_CONSTRAINT);
  CHECK(failed.error().extended_code == SQLITE_CONSTRAINT_FOREIGNKEY);
  CHECK(failed.error().message == "FOREIGN KEY constraint failed");
  CHECK(confirmed_noncommit);
  CHECK(count(db, "child") == 0);
  confirmed_noncommit = false;
  CHECK_FALSE(tx->commit(confirmed_noncommit));
  CHECK(confirmed_noncommit);
  auto retry = db.begin();
  REQUIRE(retry);
  execute(db, "INSERT INTO parent VALUES(1)");
  execute(db, "INSERT INTO child VALUES(1)");
  confirmed_noncommit = true;
  REQUIRE(retry->commit(confirmed_noncommit));
  CHECK_FALSE(confirmed_noncommit);
  CHECK(count(db, "child") == 1);
}

namespace {
#include "../../../tests/native/v5-02-x05-c4-b1-support/observer.inc"
struct test_read_guard : read_guard {
  int                   calls    = 0;
  int                   throwing = 0;
  read_gate_status      result   = read_gate_status::ready;
  std::function<void()> before;
  read_gate_status      verify() override {
    ++calls;
    if (before)
      before();
    if (throwing == 1)
      throw std::bad_alloc();
    if (throwing == 2)
      throw std::length_error("controlled verifier");
    if (throwing == 3)
      throw 42;
    return result;
  }
};
void gated_seed(scratch& files) {
  auto db = files.open();
  execute(db, "CREATE TABLE items(n BLOB)");
  execute(db, "INSERT INTO items SELECT zeroblob(8192)");
}
int gated_registrations() {
  int n = 0;
  for (auto* v = sqlite3_vfs_find(nullptr); v; v = v->pNext)
    if (std::string_view(v->zName).starts_with("planar-read-"))
      ++n;
  return n;
}
} // namespace
TEST_CASE("ledger gated first header verification precedes real content reads", "[ledger][gated]") {
  scratch files;
  gated_seed(files);
  observed_vfs observer;
  auto         guard = std::make_shared<test_read_guard>();
  guard->result      = read_gate_status::refused;
  auto bad           = connection::open_gated(files.root / "db", guard);
  REQUIRE_FALSE(bad);
  CHECK(bad.error().gate_status == read_gate_status::refused);
  CHECK(guard->calls == 1);
  CHECK(observer.reads == 0);
  CHECK(observer.moves == 0);
  CHECK(observer.closes == 1);
  CHECK(gated_registrations() == 0);
  guard->result = read_gate_status::ready;
  auto good     = connection::open_gated(files.root / "db", guard);
  REQUIRE(good);
  CHECK(observer.reads > 0);
  CHECK(observer.moves > 0);
  CHECK(count(*good) == 1);
  CHECK(gated_registrations() == 1);
  CHECK(sqlite3_vfs_find(nullptr) == &observer.vfs);
}
TEST_CASE("ledger gated verifier failures are typed and never cross SQLite C frames", "[ledger][gated]") {
  scratch files;
  gated_seed(files);
  for (int kind = 0; kind < 6; ++kind) {
    observed_vfs observer;
    auto         guard = std::make_shared<test_read_guard>();
    guard->throwing    = kind < 3 ? kind + 1 : 0;
    guard->result      = kind == 3   ? read_gate_status::resource
                         : kind == 4 ? static_cast<read_gate_status>(999)
                                     : read_gate_status::refused;
    auto db            = connection::open_gated(files.root / "db", guard);
    REQUIRE_FALSE(db);
    CHECK(observer.reads == 0);
    auto expected = kind < 2 || kind == 3 ? read_gate_status::resource
                    : kind == 2           ? read_gate_status::verifier_exception
                    : kind == 4           ? read_gate_status::invalid_status
                                          : read_gate_status::refused;
    CHECK(db.error().gate_status == expected);
    CHECK(gated_registrations() == 0);
  }
  CHECK_FALSE(connection::open_gated(files.root / "db", {}));
}
TEST_CASE("ledger gated underlying HAS_MOVED must succeed unmoved before read", "[ledger][gated]") {
  scratch files;
  gated_seed(files);
  for (int kind = 0; kind < 3; ++kind) {
    observed_vfs observer;
    observer.moved_code  = kind == 0 ? SQLITE_NOTFOUND : kind == 1 ? SQLITE_IOERR : SQLITE_OK;
    observer.moved_value = kind == 2;
    auto guard           = std::make_shared<test_read_guard>();
    auto db              = connection::open_gated(files.root / "db", guard);
    REQUIRE_FALSE(db);
    CHECK(observer.reads == 0);
    CHECK(observer.moves == 1);
    CHECK(guard->calls == 1);
    CHECK(db.error().gate_status == (kind == 0   ? read_gate_status::moved_unsupported
                                     : kind == 1 ? read_gate_status::moved_error
                                                 : read_gate_status::moved));
  }
}
TEST_CASE("ledger gated owners retain VFS and guard through statements and transactions", "[ledger][gated]") {
  scratch files;
  gated_seed(files);
  observed_vfs                   observer;
  std::weak_ptr<test_read_guard> weak;
  auto                           stmt = [&] {
    auto guard = std::make_shared<test_read_guard>();
    weak       = guard;
    auto db    = connection::open_gated(files.root / "db", guard);
    REQUIRE(db);
    auto moved = std::move(*db);
    CHECK_FALSE(db->prepare("SELECT 1"));
    return moved.prepare("SELECT count(*) FROM items");
  }();
  REQUIRE(stmt);
  CHECK_FALSE(weak.expired());
  CHECK(observer.closes == 0);
  REQUIRE(stmt->step() == step_result::row);
  CHECK(stmt->integer(0).value().value() == 1);
  stmt = std::unexpected(sqlite_error{0, 0, {}});
  CHECK(weak.expired());
  CHECK(observer.closes == 1);
  CHECK(gated_registrations() == 0);
  auto tx = [&] {
    auto guard = std::make_shared<test_read_guard>();
    weak       = guard;
    auto db    = connection::open_gated(files.root / "db", guard);
    REQUIRE(db);
    return db->begin(false);
  }();
  REQUIRE(tx);
  CHECK_FALSE(weak.expired());
  CHECK(gated_registrations() == 1);
  REQUIRE(tx->rollback());
  tx = std::unexpected(sqlite_error{0, 0, {}});
  CHECK(weak.expired());
  CHECK(gated_registrations() == 0);
}
TEST_CASE("ledger gated independent owners and sticky late refusal", "[ledger][gated]") {
  scratch files;
  gated_seed(files);
  observed_vfs observer;
  auto         a = std::make_shared<test_read_guard>(), b = std::make_shared<test_read_guard>();
  auto         first = connection::open_gated(files.root / "db", a), second = connection::open_gated(files.root / "db", b);
  REQUIRE(first);
  REQUIRE(second);
  CHECK(gated_registrations() == 2);
  a->result   = read_gate_status::refused;
  auto failed = first->prepare("SELECT n FROM items");
  if (failed) {
    auto stepped = failed->step();
    REQUIRE_FALSE(stepped);
    CHECK(stepped.error().gate_status == read_gate_status::refused);
  } else
    CHECK(failed.error().gate_status == read_gate_status::refused);
  auto calls    = a->calls;
  a->throwing   = 1;
  auto repeated = first->prepare("SELECT n FROM items");
  if (repeated) {
    auto stepped = repeated->step();
    REQUIRE_FALSE(stepped);
    CHECK(stepped.error().gate_status == read_gate_status::refused);
  } else
    CHECK(repeated.error().gate_status == read_gate_status::refused);
  CHECK(a->calls == calls);
  CHECK(count(*second) == 1);
}
TEST_CASE("ledger gated read-only SQL rejects writes and attached alternate database before open", "[ledger][gated]") {
  scratch files;
  gated_seed(files);
  observed_vfs observer;
  auto         db = connection::open_gated(files.root / "db", std::make_shared<test_read_guard>());
  REQUIRE(db);
  auto writes = db->prepare("DELETE FROM items");
  REQUIRE(writes);
  CHECK_FALSE(writes->step());
  int opens = observer.opens, reads = observer.reads;
  CHECK_FALSE(db->prepare(std::format("ATTACH '{}' AS other", (files.root / "alternate").string())));
  CHECK(observer.opens == opens);
  CHECK(observer.reads == reads);
  CHECK_FALSE(std::filesystem::exists(files.root / "alternate"));
  for (auto suffix : {"-journal", "-wal", "-shm"})
    CHECK_FALSE(std::filesystem::exists((files.root / "db").string() + suffix));
}
TEST_CASE("ledger gated partial base open closes before registration release", "[ledger][gated]") {
  scratch files;
  gated_seed(files);
  observed_vfs observer;
  observer.partial_open = true;
  auto guard            = std::make_shared<test_read_guard>();
  auto db               = connection::open_gated(files.root / "db", guard);
  REQUIRE_FALSE(db);
  CHECK(observer.opens == 1);
  CHECK(observer.closes == 1);
  CHECK(observer.reads == 0);
  CHECK(gated_registrations() == 0);
}
TEST_CASE("ledger gated versioned optional methods retain real reads", "[ledger][gated]") {
  scratch files;
  gated_seed(files);
  for (int version : {1, 2, 3}) {
    observed_vfs observer;
    observer.io_version    = version;
    observer.optional_null = true;
    auto db                = connection::open_gated(files.root / "db", std::make_shared<test_read_guard>());
    REQUIRE(db);
    CHECK(count(*db) == 1);
    CHECK(observer.reads > 0);
    CHECK(observer.fetches == 0);
  }
}
TEST_CASE("ledger gated actual pathname replacement refuses before additional delegate", "[ledger][gated]") {
  scratch files;
  gated_seed(files);
  observed_vfs observer;
  auto         db = connection::open_gated(files.root / "db", std::make_shared<test_read_guard>());
  REQUIRE(db);
  std::filesystem::rename(files.root / "db", files.root / "original");
  std::filesystem::copy_file(files.root / "original", files.root / "db");
  int  reads = observer.reads;
  auto q     = db->prepare("SELECT n FROM items");
  if (q)
    CHECK_FALSE(q->step());
  else
    CHECK(q.error().gate_status == read_gate_status::moved);
  CHECK(db->gate_status() == read_gate_status::moved);
  CHECK(observer.reads == reads);
}
TEST_CASE("ledger gated mmap performs actual mapping through native verification", "[ledger][gated]") {
  scratch files;
  gated_seed(files);
  observed_vfs observer;
  auto         db = connection::open_gated(files.root / "db", std::make_shared<test_read_guard>());
  REQUIRE(db);
  auto mmap = db->prepare("PRAGMA mmap_size=1048576");
  REQUIRE(mmap);
  REQUIRE(mmap->step() == step_result::row);
  REQUIRE(count(*db) == 1);
  auto read = db->prepare("SELECT length(n) FROM items");
  REQUIRE(read);
  REQUIRE(read->step() == step_result::row);
  CHECK(read->integer(0).value().value() == 8192);
  CHECK(observer.fetches > 0);
  CHECK(observer.mapped > 0);
}
TEST_CASE("ledger gated healthy reads preserve exact file bytes and permissions", "[ledger][gated]") {
  scratch files;
  gated_seed(files);
  auto bytes = [&] {
    std::ifstream in(files.root / "db", std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), {});
  };
  auto original = bytes();
  auto mode     = std::filesystem::status(files.root / "db").permissions();
  {
    auto db = connection::open_gated(files.root / "db", std::make_shared<test_read_guard>());
    REQUIRE(db);
    CHECK(count(*db) == 1);
    auto tx = db->begin(false);
    REQUIRE(tx);
    CHECK(count(*db) == 1);
    REQUIRE(tx->commit());
  }
  CHECK(bytes() == original);
  CHECK(std::filesystem::status(files.root / "db").permissions() == mode);
  for (auto suffix : {"-journal", "-wal", "-shm"})
    CHECK_FALSE(std::filesystem::exists((files.root / "db").string() + suffix));
}
