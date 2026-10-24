// Scratch-only format and publication checks. No installed or operator databases are used.
#include <catch2/catch_test_macros.hpp>
#include <fcntl.h>
#include <signal.h>
#include <sqlite3.h>
#include <sys/wait.h>
#include <unistd.h>
import std;
import planar.engine.history.request_v2;
import planar.engine.ledger.format;
import planar.engine.ledger.sqlite;
import planar.engine.json_codec;
using namespace planar::engine::ledger;
namespace {
struct scratch {
  std::filesystem::path root;
  scratch() {
    char  pattern[] = "/tmp/planar-format-XXXXXX";
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
std::map<std::string, std::string> snapshot(const std::filesystem::path& root) {
  std::map<std::string, std::string> out;
  for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
    const auto name = entry.path().lexically_relative(root).string();
    if (entry.is_symlink())
      out[name] = "link:" + std::filesystem::read_symlink(entry.path()).string();
    else if (entry.is_directory())
      out[name] = "directory";
    else
      out[name] = bytes(entry.path());
  }
  return out;
}
connection raw(const std::filesystem::path& path) {
  auto db = connection::open(path, open_mode::scratch_create);
  REQUIRE(db);
  return std::move(*db);
}
void sql(connection& db, std::string_view text) {
  auto s = db.prepare(text);
  REQUIRE(s);
  REQUIRE(s->step() == step_result::done);
}
std::int64_t scalar(connection& db, std::string_view text) {
  auto s = db.prepare(text);
  REQUIRE(s);
  REQUIRE(s->step() == step_result::row);
  auto n = s->integer(0);
  REQUIRE(n);
  REQUIRE(*n);
  return **n;
}
auto schema_rows(connection& db) {
  std::vector<std::array<std::optional<std::string>, 4>> rows;
  auto query = db.prepare("SELECT name,type,tbl_name,sql FROM sqlite_schema ORDER BY name");
  REQUIRE(query);
  while (true) {
    auto step = query->step();
    REQUIRE(step);
    if (*step == step_result::done)
      break;
    std::array<std::optional<std::string>, 4> row;
    for (int i = 0; i < 4; ++i) {
      auto text = query->text(i);
      REQUIRE(text);
      row[i] = std::move(*text);
    }
    rows.push_back(std::move(row));
  }
  return rows;
}
void initialized(const std::filesystem::path& path) {
  auto result = initialize_new(path, "test_workspace");
  if (!result)
    INFO(result.error().message);
  REQUIRE(result);
}
void refusal_unchanged(const std::filesystem::path& root, const std::filesystem::path& path, format_error_code wanted) {
  const auto before = snapshot(root);
  for (auto access : {ledger_access::read_only, ledger_access::read_write}) {
    auto result = open_existing(path, access);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == wanted);
    CHECK(snapshot(root) == before);
  }
}
} // namespace
TEST_CASE("ledger format initializes exact metadata schema and Glaze event", "[ledger]") {
  scratch f;
  auto    result = initialize_new(f.root / "db", "workspace_1");
  REQUIRE(result);
  CHECK(result->identity.size() == 32);
  CHECK(result->created_at.size() == 24);
  CHECK(std::ranges::all_of(result->identity, [](char c) { return (c >= 'a' && c <= 'f') || (c >= '0' && c <= '9'); }));
  auto db = raw(f.root / "db");
  CHECK(scalar(db, "PRAGMA application_id") == 1347178034);
  CHECK(scalar(db, "PRAGMA user_version") == 1);
  CHECK(scalar(db, "SELECT count(*) FROM pragma_table_list WHERE schema='main' AND strict=1") == 7);
  CHECK(scalar(db, "SELECT count(*) FROM sqlite_schema WHERE type='trigger'") == 13);
  CHECK(scalar(db, "SELECT count(*) FROM sqlite_schema WHERE type='index' AND sql IS NOT NULL") == 1);
  // SQLite parses the independent normative fixture; compare complete metadata/stored SQL, not shared product constants.
  sqlite3* fixture = nullptr;
  REQUIRE(sqlite3_open((f.root / "reference").c_str(), &fixture) == SQLITE_OK);
  auto expected_sql = bytes(PLANAR_LEDGER_SCHEMA);
  REQUIRE_FALSE(expected_sql.empty());
  REQUIRE(sqlite3_exec(fixture, expected_sql.c_str(), nullptr, nullptr, nullptr) == SQLITE_OK);
  REQUIRE(sqlite3_close(fixture) == SQLITE_OK);
  auto reference = raw(f.root / "reference");
  CHECK(schema_rows(db) == schema_rows(reference));

  for (auto table : {"records", "tree_edges", "revisions", "requests"})
    CHECK(scalar(db, std::format("SELECT count(*) FROM {}", table)) == 0);
  CHECK(scalar(db, "SELECT count(*) FROM current_state WHERE singleton=1 AND active_revision_id IS NULL AND last_seq=1") == 1);
  auto q = db.prepare("SELECT identity,workspace_id,created_at FROM ledger_meta");
  REQUIRE(q);
  REQUIRE(q->step() == step_result::row);
  CHECK(*q->text(0) == result->identity);
  CHECK(*q->text(1) == "workspace_1");
  CHECK(*q->text(2) == result->created_at);
  auto e = db.prepare("SELECT actor,occurred_at,payload FROM events");
  REQUIRE(e);
  REQUIRE(e->step() == step_result::row);
  CHECK(*e->text(0) == "planar:init");
  CHECK(*e->text(1) == result->created_at);
  auto payload = e->blob(2);
  REQUIRE(payload);
  REQUIRE(*payload);
  auto decoded = planar::engine::codec_decode(**payload);
  REQUIRE(decoded);
  CHECK((*decoded)->fields.size() == 5);
  CHECK((*decoded)->fields.at("version")->integer == 1);
  CHECK((*decoded)->fields.at("reason")->string == "initialized");
  CHECK((*decoded)->fields.at("references")->fields.empty());
  CHECK((*decoded)->fields.at("before")->fields.at("active_revision")->string == "none");
  CHECK((*decoded)->fields.at("after")->fields.at("active_revision")->string == "none");
}
TEST_CASE("ledger format reopens read only and writable without mutation", "[ledger]") {
  scratch f;
  initialized(f.root / "db");
  const auto before = snapshot(f.root);
  for (auto mode : {ledger_access::read_only, ledger_access::read_write}) {
    auto result = open_existing(f.root / "db", mode);
    if (!result)
      INFO(result.error().message);
    REQUIRE(result);
    CHECK(result->metadata().workspace_id == "test_workspace");
    auto moved = std::move(*result);
    CHECK(moved.metadata().identity.size() == 32);
  }
  CHECK(snapshot(f.root) == before);
}
TEST_CASE("ledger format refuses foreign empty truncated and version files unchanged", "[ledger]") {
  scratch f;
  for (auto name : {"empty", "truncated", "foreign", "future", "old", "wrong_app"}) {
    auto path = f.root / name;
    if (std::string_view(name) == "empty")
      std::ofstream(path).close();
    else if (std::string_view(name) == "truncated")
      std::ofstream(path) << "SQLite format 3";
    else if (std::string_view(name) == "foreign") {
      auto db = raw(path);
      sql(db, "CREATE TABLE unrelated(value TEXT)");
    } else {
      initialized(path);
      auto db = raw(path);
      sql(db, std::string_view(name) == "future" ? "PRAGMA user_version=3"
              : std::string_view(name) == "old"  ? "PRAGMA user_version=0"
                                                 : "PRAGMA application_id=42");
    }
    const auto wanted = (std::string_view(name) == "future" || std::string_view(name) == "old")
                            ? format_error_code::unsupported_version
                            : format_error_code::foreign_ledger;
    refusal_unchanged(f.root, path, wanted);
  }
}
TEST_CASE("ledger format refuses every missing trigger and explicit index unchanged", "[ledger]") {
  scratch f;
  int     ordinal = 0;
  for (auto table : {"ledger_meta", "records", "tree_edges", "revisions", "events", "requests", "current_state"}) {
    for (auto op : {"update", "delete"}) {
      if (std::string_view(table) == "current_state" && std::string_view(op) == "update")
        continue;
      auto path = f.root / std::to_string(ordinal++);
      initialized(path);
      {
        auto db = raw(path);
        sql(db, std::format("DROP TRIGGER {}_no_{}", table, op));
      }
      refusal_unchanged(f.root, path, format_error_code::corrupt);
    }
  }
  auto path = f.root / "index";
  initialized(path);
  {
    auto db = raw(path);
    sql(db, "DROP INDEX records_digest");
  }
  refusal_unchanged(f.root, path, format_error_code::corrupt);
}
TEST_CASE("ledger format rejects altered schema and extra objects unchanged", "[ledger]") {
  scratch f;
  for (auto [name, change] : {std::pair{"extra", "CREATE TABLE extra(x TEXT) STRICT"},
                              {"column", "ALTER TABLE records ADD COLUMN extra INTEGER"},
                              {"index", "CREATE INDEX unrequested ON records(kind)"}}) {
    auto path = f.root / name;
    initialized(path);
    {
      auto db = raw(path);
      sql(db, change);
    }
    refusal_unchanged(f.root, path, format_error_code::corrupt);
  }
}
TEST_CASE("ledger format validates init cross rows and refuses populated history explicitly", "[ledger]") {
  scratch f;
  for (auto [name, change] : {std::pair{"current", "UPDATE current_state SET last_seq=2"},
                              {"missing", "DELETE FROM current_state"},
                              {"record", "INSERT INTO records VALUES(1,1,1,zeroblob(16),x'01')"}}) {
    auto path = f.root / name;
    initialized(path);
    {
      auto db = raw(path);
      sql(db, "PRAGMA foreign_keys=OFF");
      if (std::string_view(name) == "missing") {
        sql(db, "DROP TRIGGER current_state_no_delete");
        sql(db, change);
        sql(db, "CREATE TRIGGER current_state_no_delete BEFORE DELETE ON current_state BEGIN SELECT RAISE(ABORT,'immutable "
                "ledger row'); END");
      } else
        sql(db, change);
    }
    refusal_unchanged(f.root, path, format_error_code::corrupt);
  }
  auto path = f.root / "event";
  initialized(path);
  {
    auto db = raw(path);
    sql(db, "DROP TRIGGER events_no_update");
    sql(db, "UPDATE events SET actor='changed'");
    sql(db, "CREATE TRIGGER events_no_update BEFORE UPDATE ON events BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END");
  }
  refusal_unchanged(f.root, path, format_error_code::corrupt);
}
TEST_CASE("ledger format refuses metadata grammar and timestamp corruption", "[ledger]") {
  scratch f;
  int     i = 0;
  for (auto change :
       {"UPDATE ledger_meta SET identity='ABCDEF0123456789abcdef0123456789'", "UPDATE ledger_meta SET workspace_id='Bad'",
        "UPDATE ledger_meta SET created_at='2026-02-30T12:00:00.000Z'"}) {
    auto path = f.root / std::to_string(i++);
    initialized(path);
    {
      auto db = raw(path);
      sql(db, "DROP TRIGGER ledger_meta_no_update");
      sql(db, change);
      sql(db, "CREATE TRIGGER ledger_meta_no_update BEFORE UPDATE ON ledger_meta BEGIN SELECT RAISE(ABORT,'immutable ledger "
              "row'); END");
    }
    refusal_unchanged(f.root, path, format_error_code::corrupt);
  }
}
TEST_CASE("ledger format all immutable triggers abort meaningful row mutations", "[ledger]") {
  scratch f;
  initialized(f.root / "db");
  auto db = raw(f.root / "db");
  sql(db, "INSERT INTO records VALUES(1,1,1,zeroblob(16),x'01')");
  sql(db, "INSERT INTO records VALUES(2,2,1,zeroblob(16),x'02')");
  sql(db, "INSERT INTO tree_edges VALUES(2,0,'a',1)");
  sql(db, "INSERT INTO revisions VALUES(1,NULL,2,2,2,1,1,'actor','time')");
  sql(db, "INSERT INTO requests VALUES('request',1,1,1,'accepted',x'01')");
  for (auto table : {"ledger_meta", "records", "tree_edges", "revisions", "events", "requests", "current_state"}) {
    for (auto op : {"update", "delete"}) {
      if (std::string_view(table) == "current_state" && std::string_view(op) == "update")
        continue;
      auto statement =
          db.prepare(std::string_view(op) == "delete" ? std::format("DELETE FROM {}", table)
                                                      : std::format("UPDATE {} SET {}={}", table,
                                                                    std::string_view(table) == "tree_edges"    ? "ordinal"
                                                                    : std::string_view(table) == "events"      ? "seq"
                                                                    : std::string_view(table) == "requests"    ? "request_id"
                                                                    : std::string_view(table) == "ledger_meta" ? "singleton"
                                                                                                               : "id",
                                                                    std::string_view(table) == "tree_edges"    ? "ordinal"
                                                                    : std::string_view(table) == "events"      ? "seq"
                                                                    : std::string_view(table) == "requests"    ? "request_id"
                                                                    : std::string_view(table) == "ledger_meta" ? "singleton"
                                                                                                               : "id"));
      REQUIRE(statement);
      auto result = statement->step();
      REQUIRE_FALSE(result);
      CHECK(result.error().message == "immutable ledger row");
    }
  }
}
TEST_CASE("ledger format no replace and no follow initialization preserves all targets", "[ledger]") {
  scratch f;
  std::filesystem::create_directory(f.root / "directory");
  std::ofstream(f.root / "empty").close();
  std::ofstream(f.root / "regular") << "sentinel";
  std::filesystem::create_symlink("regular", f.root / "symlink");
  std::filesystem::create_symlink("absent", f.root / "dangling");
  for (auto name : {"directory", "empty", "regular", "symlink", "dangling"}) {
    const auto before = snapshot(f.root);
    auto       result = initialize_new(f.root / name, "workspace");
    REQUIRE_FALSE(result);
    CHECK(result.error().code == format_error_code::exists);
    CHECK(snapshot(f.root) == before);
  }
  std::filesystem::create_directory_symlink("directory", f.root / "linkdir");
  const auto before = snapshot(f.root);
  for (auto name : {"missing/db", "missing/../db", "linkdir/db", "linkdir/../db", "regular/../db"}) {
    CHECK_FALSE(initialize_new(f.root / name, "workspace"));
    CHECK_FALSE(open_existing(f.root / name));
    CHECK(snapshot(f.root) == before);
  }
  CHECK(initialize_new(f.root / "directory/../db", "workspace"));
}
TEST_CASE("ledger format sidecars require recovery without writable fallback", "[ledger]") {
  scratch f;
  for (auto suffix : {"-journal", "-wal", "-shm"}) {
    auto path = f.root / (std::string("db") + suffix);
    initialized(path);
    std::ofstream(path.string() + suffix) << "retained";
    refusal_unchanged(f.root, path, format_error_code::recovery_required);
  }
}
TEST_CASE("ledger format initialization rejects invalid arguments and persists literal memory filename", "[ledger]") {
  scratch f;
  for (auto id : {"", "Upper", "a-b", "0start"})
    CHECK_FALSE(initialize_new(f.root / "db", id));
  CHECK_FALSE(initialize_new(f.root / "db", std::string(65, 'a')));
  CHECK_FALSE(initialize_new({}, "workspace"));
  CHECK_FALSE(initialize_new(f.root / ".", "workspace"));
  CHECK(std::filesystem::is_empty(f.root));
  initialized(f.root / ":memory:");
  REQUIRE(std::filesystem::file_size(f.root / ":memory:") > 512);
  CHECK(open_existing(f.root / ":memory:"));
}
TEST_CASE("ledger format schema and initialization rollback are one atomic transaction", "[ledger]") {
  scratch f;
  initialized(f.root / "db");
  const auto before = bytes(f.root / "db");
  {
    auto db = raw(f.root / "db");
    auto tx = db.begin();
    REQUIRE(tx);
    sql(db, "CREATE TABLE tentative(x TEXT) STRICT");
    auto duplicate = db.prepare("INSERT INTO ledger_meta VALUES(1,1,'0123456789abcdef0123456789abcdef','other','time')");
    REQUIRE(duplicate);
    CHECK_FALSE(duplicate->step());
    CHECK_FALSE(tx->commit());
  }
  CHECK(bytes(f.root / "db") == before);
  CHECK(open_existing(f.root / "db"));
}

TEST_CASE("ledger format malformed btree and altered trigger refuse unchanged", "[ledger]") {
  scratch f;
  auto    broken = f.root / "broken";
  initialized(broken);
  {
    std::fstream stream(broken, std::ios::binary | std::ios::in | std::ios::out);
    stream.seekp(100);
    stream.put(char(0xff));
  }
  refusal_unchanged(f.root, broken, format_error_code::corrupt);
  auto changed = f.root / "changed";
  initialized(changed);
  {
    auto db = raw(changed);
    sql(db, "DROP TRIGGER events_no_delete");
    sql(db, "CREATE TRIGGER events_no_delete BEFORE DELETE ON events BEGIN SELECT 1; END");
  }
  refusal_unchanged(f.root, changed, format_error_code::corrupt);
  const auto before  = snapshot(f.root);
  auto       missing = open_existing(f.root / "missing", ledger_access::read_write);
  REQUIRE_FALSE(missing);
  CHECK(missing.error().code == format_error_code::not_found);
  CHECK(snapshot(f.root) == before);
}

TEST_CASE("ledger format initialization refuses every existing destination sidecar", "[ledger]") {
  for (auto suffix : {"-journal", "-wal", "-shm"}) {
    for (auto kind : {"regular", "symlink", "directory"}) {
      CAPTURE(suffix, kind);
      scratch    f;
      const auto target  = f.root / "db";
      const auto sidecar = f.root / (std::string("db") + suffix);
      std::ofstream(f.root / "sentinel") << "unrelated bytes";
      if (std::string_view(kind) == "regular")
        std::ofstream(sidecar) << "unrelated sidecar bytes";
      else if (std::string_view(kind) == "symlink")
        std::filesystem::create_symlink("sentinel", sidecar);
      else {
        std::filesystem::create_directory(sidecar);
        std::ofstream(sidecar / "child") << "retained child";
      }
      const auto before = snapshot(f.root);
      auto       result = initialize_new(target, "workspace");
      CHECK_FALSE(result);
      if (!result)
        CHECK(result.error().code == format_error_code::recovery_required);
      CHECK_FALSE(std::filesystem::exists(target));
      CHECK(snapshot(f.root) == before);
    }
  }
}
TEST_CASE("ledger format rejects NUL suffixes and oversized encoded metadata unchanged", "[ledger]") {
  for (auto column : {"identity", "workspace_id", "created_at"}) {
    for (bool nul : {true, false}) {
      CAPTURE(column, nul);
      scratch    f;
      const auto path = f.root / "db";
      initialized(path);
      {
        auto db = raw(path);
        sql(db, "DROP TRIGGER ledger_meta_no_update");
        // Keep identity's normative character CHECK satisfied for both encodings.
        const auto characters = std::string_view(column) == "workspace_id" ? 64
                                : std::string_view(column) == "created_at" ? 24
                                                                           : 32;
        const auto expression = nul ? std::format("{} || char(0) || CAST(zeroblob(2097152) AS TEXT)", column)
                                    : std::format("replace(hex(zeroblob({})), '0', char(233))", characters / 2);
        sql(db, std::format("UPDATE ledger_meta SET {}={}", column, expression));
        sql(db, "CREATE TRIGGER ledger_meta_no_update BEFORE UPDATE ON ledger_meta BEGIN SELECT RAISE(ABORT,'immutable ledger "
                "row'); END");
      }
      refusal_unchanged(f.root, path, format_error_code::corrupt);
    }
  }
}

namespace {
// The child communicates only after SQLite has synced a real rollback journal and flushed dirty database pages.
void crash_dirty(const std::filesystem::path& path, int record_id = 1) {
  int pipefd[2];
  REQUIRE(::pipe(pipefd) == 0);
  auto pid = ::fork();
  REQUIRE(pid >= 0);
  if (pid == 0) {
    ::close(pipefd[0]);
    sqlite3* db   = nullptr;
    int      code = sqlite3_open(path.c_str(), &db);
    if (code == SQLITE_OK)
      code = sqlite3_exec(db,
                          std::format("PRAGMA cache_size=1; PRAGMA synchronous=FULL; BEGIN IMMEDIATE;"
                                      "INSERT INTO records VALUES({},1,1,zeroblob(16),zeroblob(200000));",
                                      record_id)
                              .c_str(),
                          nullptr, nullptr, nullptr);
    if (code == SQLITE_OK)
      code = sqlite3_db_cacheflush(db);
    char ready = code == SQLITE_OK ? 'Y' : 'N';
    if (::write(pipefd[1], &ready, 1) != 1)
      ::_exit(92);
    for (;;)
      ::pause();
  }
  ::close(pipefd[1]);
  char       ready = 0;
  const auto n     = ::read(pipefd[0], &ready, 1);
  ::close(pipefd[0]);
  const auto killed = ::kill(pid, SIGKILL);
  int        status = 0;
  REQUIRE(::waitpid(pid, &status, 0) == pid);
  REQUIRE(killed == 0);
  REQUIRE(n == 1);
  REQUIRE(ready == 'Y');
  REQUIRE(WIFSIGNALED(status));
  REQUIRE(WTERMSIG(status) == SIGKILL);
  REQUIRE(std::filesystem::file_size(path.string() + "-journal") > 512);
}
void metadata_equal(const ledger_metadata& a, const ledger_metadata& b) {
  CHECK(a.identity == b.identity);
  CHECK(a.workspace_id == b.workspace_id);
  CHECK(a.created_at == b.created_at);
}
void recovery_refused_unchanged(const scratch& f, const std::filesystem::path& path) {
  const auto before = snapshot(f.root);
  auto       result = recover_quiescent(path);
  REQUIRE_FALSE(result);
  CHECK(snapshot(f.root) == before);
}
} // namespace
TEST_CASE("ledger format online backup and restore preserve initialized metadata and state", "[ledger][backup]") {
  scratch f;
  auto    info = initialize_new(f.root / "source", "backup_workspace");
  REQUIRE(info);
  const auto source_bytes = bytes(f.root / "source");
  auto       backup       = backup_new(f.root / "source", f.root / "backup");
  REQUIRE(backup);
  metadata_equal(*backup, *info);
  auto restored = restore_new(f.root / "backup", f.root / "restored");
  REQUIRE(restored);
  metadata_equal(*restored, *info);
  for (auto name : {"source", "backup", "restored"}) {
    auto verified = open_existing(f.root / name, ledger_access::read_write);
    REQUIRE(verified);
    metadata_equal(verified->metadata(), *info);
  }
  CHECK(bytes(f.root / "source") == source_bytes);
  CHECK(snapshot(f.root).size() == 3);
}
TEST_CASE("ledger format backup restore refuse existing destinations and sidecars unchanged", "[ledger][backup]") {
  for (bool restore : {false, true})
    for (auto suffix : {"", "-journal", "-wal", "-shm"})
      for (int kind = 0; kind < 3; ++kind) {
        scratch f;
        initialized(f.root / "source");
        auto entry = f.root / (std::string("target") + suffix);
        if (kind == 0)
          std::ofstream(entry) << "retained bytes";
        else if (kind == 1)
          std::filesystem::create_symlink("source", entry);
        else
          std::filesystem::create_directory(entry);
        const auto before = snapshot(f.root);
        auto       result =
            restore ? restore_new(f.root / "source", f.root / "target") : backup_new(f.root / "source", f.root / "target");
        REQUIRE_FALSE(result);
        CHECK(result.error().code ==
              (std::string_view(suffix).empty() ? format_error_code::exists : format_error_code::recovery_required));
        CHECK(snapshot(f.root) == before);
      }
}
TEST_CASE("ledger format backup restore reject invalid sources before target creation", "[ledger][backup]") {
  for (bool restore : {false, true})
    for (int kind = 0; kind < 7; ++kind) {
      scratch    f;
      const auto path = f.root / "source";
      if (kind == 0)
        std::ofstream(path) << "foreign bytes";
      else {
        initialized(path);
        if (kind == 1) {
          auto db = raw(path);
          sql(db, "PRAGMA user_version=2");
        } else if (kind == 2) {
          auto db = raw(path);
          sql(db, "DROP TRIGGER events_no_delete");
        } else if (kind == 3) {
          auto db = raw(path);
          sql(db, "INSERT INTO records VALUES(1,1,1,zeroblob(16),X'00')");
        } else if (kind == 4)
          std::ofstream(path.string() + "-journal") << "unrecovered journal";
        else if (kind == 5)
          std::filesystem::resize_file(path, 512);
        else {
          std::filesystem::rename(path, f.root / "real");
          std::filesystem::create_symlink("real", path);
        }
      }
      const auto before = snapshot(f.root);
      auto       result = restore ? restore_new(path, f.root / "target") : backup_new(path, f.root / "target");
      REQUIRE_FALSE(result);
      CHECK(snapshot(f.root) == before);
    }
}
TEST_CASE("ledger format explicit recovery rolls back real killed dirty pages", "[ledger][recovery]") {
  scratch    f;
  const auto path = f.root / "database";
  auto       info = initialize_new(path, "recovery_workspace");
  REQUIRE(info);
  const auto original = bytes(path);
  crash_dirty(path);
  CHECK(bytes(path) != original);
  refusal_unchanged(f.root, path, format_error_code::recovery_required);
  auto result = recover_quiescent(path);
  if (!result)
    INFO(result.error().message);
  REQUIRE(result);
  metadata_equal(*result, *info);
  auto verified = open_existing(path);
  REQUIRE(verified);
  metadata_equal(verified->metadata(), *info);
  CHECK(snapshot(f.root).size() == 1);
  auto db = raw(path);
  CHECK(scalar(db, "SELECT count(*) FROM records") == 0);
}
TEST_CASE("ledger format recovery accepts redundant genuine hot journal", "[ledger][recovery]") {
  scratch    f;
  const auto path = f.root / "database";
  auto       info = initialize_new(path, "recovery_workspace");
  REQUIRE(info);
  const auto original = bytes(path);
  crash_dirty(path);
  // Retain the genuine SQLite hot journal but reconstruct the prior main bytes, modeling death before main-page writes.
  std::ofstream(path, std::ios::binary | std::ios::trunc).write(original.data(), original.size());
  auto result = recover_quiescent(path);
  REQUIRE(result);
  metadata_equal(*result, *info);
  REQUIRE(open_existing(path));
  CHECK(snapshot(f.root).size() == 1);
}
TEST_CASE("ledger format recovery refuses super-journal authority unchanged", "[ledger][recovery]") {
  for (const auto kind : {"external", "absent", "symlink", "directory", "zero-length", "huge-length", "truncated-name",
                          "wrong-checksum", "nul-name", "magic-only"}) {
    INFO(kind);
    scratch    f;
    const auto path = f.root / "database";
    initialized(path);
    crash_dirty(path);
    const auto  journal  = path.string() + "-journal";
    const auto  external = f.root / "external-mj000000900";
    std::string name     = external.string();
    if (std::string_view(kind) == "symlink") {
      std::ofstream(f.root / "sentinel") << journal << '\0';
      std::filesystem::create_symlink("sentinel", external);
    } else if (std::string_view(kind) == "directory")
      std::filesystem::create_directory(external);
    else if (std::string_view(kind) != "absent")
      std::ofstream(external, std::ios::binary) << journal << '\0';
    if (std::string_view(kind) == "nul-name")
      name += std::string("\0ignored", 8);
    std::uint32_t length   = static_cast<std::uint32_t>(name.size());
    std::uint32_t checksum = 0;
    for (unsigned char c : name)
      checksum += c;
    if (std::string_view(kind) == "zero-length")
      length = 0;
    else if (std::string_view(kind) == "huge-length")
      length = 0xffffffff;
    else if (std::string_view(kind) == "truncated-name")
      length += 100000;
    else if (std::string_view(kind) == "wrong-checksum")
      ++checksum;
    {
      std::ofstream out(journal, std::ios::binary | std::ios::app);
      auto          u32 = [&](std::uint32_t n) {
        for (int shift = 24; shift >= 0; shift -= 8)
          out.put(static_cast<char>(n >> shift));
      };
      // SQLite's documented trailer: page marker, pathname, BE length/checksum, exact eight-byte magic at EOF.
      if (std::string_view(kind) != "magic-only") {
        u32(0x40000000 / 4096 + 1);
        out.write(name.data(), name.size());
        u32(length);
        u32(checksum);
      }
      const unsigned char magic[] = {0xd9, 0xd5, 0x05, 0xf9, 0x20, 0xa1, 0x63, 0xd7};
      out.write(reinterpret_cast<const char*>(magic), sizeof(magic));
      REQUIRE(out.good());
    }
    const auto before = snapshot(f.root);
    auto       result = recover_quiescent(path);
    CHECK_FALSE(result);
    if (!result)
      CHECK(result.error().code == format_error_code::corrupt);
    // Nonfatal assertions retain the full unsafe success/deletion observation in the red run.
    CHECK(std::filesystem::exists(external) == (std::string_view(kind) != "absent"));
    CHECK(snapshot(f.root) == before);
  }
}
TEST_CASE("ledger format recovery rejects deceptive and malformed pairs unchanged", "[ledger][recovery]") {
  for (int kind = 0; kind < 14; ++kind) {
    INFO(kind);
    scratch    f;
    const auto path = f.root / "database";
    initialized(path);
    if (kind < 4) {
      auto db = raw(path);
      if (kind == 0)
        sql(db, "DROP TRIGGER events_no_delete");
      else if (kind == 1)
        sql(db, "PRAGMA user_version=2");
      else if (kind == 2)
        sql(db, "PRAGMA application_id=1");
      else
        sql(db, "INSERT INTO records VALUES(2,1,1,zeroblob(16),X'00')");
    }
    std::int64_t event_page = 0, page_size = 0;
    {
      auto db    = raw(path);
      page_size  = scalar(db, "PRAGMA page_size");
      event_page = scalar(db, "SELECT rootpage FROM sqlite_schema WHERE name='events'");
    }
    crash_dirty(path);
    if (kind == 4) {
      std::fstream journal(path.string() + "-journal", std::ios::in | std::ios::out | std::ios::binary);
      journal.put('X');
    } else if (kind == 5)
      std::filesystem::resize_file(path.string() + "-journal", 512);
    else if (kind == 6)
      std::ofstream(path.string() + "-wal") << "competing sidecar";
    else if (kind == 7) {
      std::filesystem::rename(path.string() + "-journal", f.root / "real-journal");
      std::filesystem::create_symlink("real-journal", path.string() + "-journal");
    } else if (kind == 8) {
      // Corrupt a page that is not in the rollback journal: scratch integrity/schema must catch it.
      std::fstream main(path, std::ios::in | std::ios::out | std::ios::binary);
      main.seekp((event_page - 1) * page_size);
      main.put('\0');
    } else if (kind >= 9 && kind <= 11) {
      std::fstream journal(path.string() + "-journal", std::ios::in | std::ios::out | std::ios::binary);
      journal.seekp(kind == 9 ? 20 : kind == 10 ? 8 : 24);
      const char invalid[4] = {0, 0, 0, 0};
      journal.write(invalid, 4);
    } else if (kind == 12) {
      std::filesystem::remove(path.string() + "-journal");
      std::filesystem::create_directory(path.string() + "-journal");
    } else if (kind == 13)
      std::filesystem::create_symlink("database", path.string() + "-shm");
    recovery_refused_unchanged(f, path);
  }
}

namespace {
using namespace planar::engine::history;
record_child child(std::string name, const record_reference& ref) {
  return {std::move(name), ref.kind, ref.id, ref.digest};
}
void raw_record(connection& db, std::int64_t id, const record& value, std::optional<std::int64_t> kind = {}) {
  auto encoded = encode_record(value);
  auto digest  = digest_record(value);
  REQUIRE(encoded);
  REQUIRE(digest);
  auto q = db.prepare("INSERT INTO records VALUES(?1,?2,1,?3,?4)");
  REQUIRE(q);
  REQUIRE(q->bind_integer(1, id));
  REQUIRE(q->bind_integer(2, kind.value_or(static_cast<std::int64_t>(kind_of(value)))));
  REQUIRE(q->bind_blob(3, {reinterpret_cast<const char*>(digest->data()), digest->size()}));
  REQUIRE(q->bind_blob(4, *encoded));
  REQUIRE(q->step() == step_result::done);
}
record_reference store_file(const std::filesystem::path& path) {
  auto writer = begin_records(path);
  REQUIRE(writer);
  auto ref = writer->intern(file_record{493, std::string("binary\0\r\n", 9)});
  REQUIRE(ref);
  REQUIRE(writer->commit());
  return *ref;
}
} // namespace
TEST_CASE("ledger records intern all kinds with exact reuse and ordered edges", "[ledger][records]") {
  scratch    f;
  const auto path = f.root / "db";
  initialized(path);
  auto writer = begin_records(path);
  REQUIRE(writer);
  const std::array<record, 4>   values{file_record{493, std::string("binary\0\r\n", 9)}, directory_record{},
                                       value_record{"canonical_v1", std::string(1, '\0')},
                                       request_record{"load", "actor", "none", "../workspace", "registry"}};
  std::vector<record_reference> refs;
  for (const auto& value : values) {
    auto ref = writer->intern(value);
    REQUIRE(ref);
    refs.push_back(*ref);
    auto repeated = writer->intern(value);
    REQUIRE(repeated);
    CHECK(repeated->id == ref->id);
    auto read = writer->read_record(ref->id);
    REQUIRE(read);
    CHECK(types::encode_stored_record(*read).value() == encode_record(value).value());
  }
  record parent = directory_record{{child("a", refs[0]), child("b", refs[1])}};
  auto   root   = writer->intern(parent);
  REQUIRE(root);
  CHECK(root->id == 5);
  REQUIRE(writer->commit());
  CHECK_FALSE(writer->commit());
  auto reader = open_existing(path);
  REQUIRE(reader);
  auto read = reader->read_record(root->id);
  REQUIRE(read);
  CHECK(types::encode_stored_record(*read).value() == encode_record(parent).value());
  auto moved = std::move(*reader);
  CHECK(moved.read_record(refs[0].id));
  CHECK_FALSE(reader->read_record(1));
  CHECK_FALSE(moved.read_record(0));
  CHECK_FALSE(moved.read_record(std::numeric_limits<std::uint64_t>::max()));
  CHECK(moved.read_record(99).error().code == format_error_code::invalid_argument);
  auto db = raw(path);
  CHECK(scalar(db, "SELECT count(*) FROM records") == 5);
  CHECK(scalar(db, "SELECT count(*) FROM tree_edges WHERE parent_id=5 AND ordinal=0 AND name='a' AND child_id=1") == 1);
  CHECK(scalar(db, "SELECT count(*) FROM tree_edges WHERE parent_id=5 AND ordinal=1 AND name='b' AND child_id=2") == 1);
}
TEST_CASE("ledger record transaction errors and abandonment roll back every prefix", "[ledger][records]") {
  for (int kind = 0; kind < 7; ++kind) {
    scratch    f;
    const auto path = f.root / "db";
    initialized(path);
    auto       existing = store_file(path);
    const auto before   = snapshot(f.root);
    {
      auto writer = begin_records(path);
      REQUIRE(writer);
      REQUIRE(writer->intern(directory_record{}));
      if (kind < 4) {
        auto bad = child("a", existing);
        if (kind == 0)
          bad.id = 999;
        if (kind == 1)
          bad.kind = record_kind::directory;
        if (kind == 2)
          bad.digest[0] ^= 1;
        if (kind == 3)
          bad.id = std::numeric_limits<std::uint64_t>::max();
        CHECK_FALSE(writer->intern(directory_record{{bad}}));
      } else if (kind == 4)
        CHECK_FALSE(writer->intern(file_record{0, "bad mode"}));
      else if (kind == 5)
        CHECK_FALSE(writer->read_record(999));
      if (kind != 6) {
        CHECK_FALSE(writer->intern(directory_record{}));
        CHECK_FALSE(writer->commit());
      }
    }
    CHECK(snapshot(f.root) == before);
    auto reopened = open_existing(path);
    REQUIRE(reopened);
    CHECK(reopened->read_record(existing.id));
    CHECK_FALSE(reopened->read_record(2));
  }
}
TEST_CASE("ledger record reader remains independent and blocked commit rolls back", "[ledger][records]") {
  scratch f;
  auto    path = f.root / "db";
  initialized(path);
  {
    auto reader = open_existing(path);
    REQUIRE(reader);
    auto writer = begin_records(path);
    REQUIRE(writer);
    REQUIRE(writer->intern(file_record{420, "uncommitted"}));
    CHECK_FALSE(reader->read_record(1));
    auto result = writer->commit();
    REQUIRE_FALSE(result);
    CHECK(result.error().code == format_error_code::busy);
    CHECK_FALSE(writer->commit());
    CHECK_FALSE(reader->read_record(1));
  }
  auto writer = begin_records(path);
  REQUIRE(writer);
  REQUIRE(writer->intern(file_record{420, "committed"}));
  REQUIRE(writer->commit());
  auto reader = open_existing(path);
  REQUIRE(reader);
  CHECK(std::get<file_record>(reader->read_record(1)->value).payload == "committed");
}
TEST_CASE("ledger records reject inconsistent graphs and encoded rows without mutation", "[ledger][records]") {
  for (int kind = 0; kind < 11; ++kind) {
    scratch f;
    auto    path = f.root / "db";
    initialized(path);
    auto file = store_file(path);
    {
      auto db   = raw(path);
      auto edge = child("a", file);
      if (kind == 0)
        edge.id = 99;
      if (kind == 1)
        edge.id = 2;
      if (kind == 2)
        edge.kind = record_kind::directory;
      if (kind == 3)
        edge.digest[0] ^= 1;
      if (kind <= 7) {
        raw_record(db, 2, directory_record{{edge}});
        if (kind != 4) {
          sql(db, "PRAGMA foreign_keys=OFF");
          sql(db, "PRAGMA ignore_check_constraints=ON");
          sql(db, std::format("INSERT INTO tree_edges VALUES(2,{},'{}',{})", kind == 5 ? 1 : 0, kind == 6 ? "b" : "a", edge.id));
        }
        if (kind == 7)
          sql(db, "INSERT INTO tree_edges VALUES(2,1,'b',1)");
      } else if (kind == 8) {
        raw_record(db, 2, file_record{420, "leaf"});
        sql(db, "INSERT INTO tree_edges VALUES(2,0,'a',1)");
      } else if (kind == 9)
        raw_record(db, 2, file_record{420, "leaf"}, 2);
      else
        sql(db, "INSERT INTO records VALUES(2,1,1,zeroblob(16),x'01')");
    }
    refusal_unchanged(f.root, path, format_error_code::corrupt);
    const auto before = snapshot(f.root);
    CHECK_FALSE(begin_records(path));
    CHECK(snapshot(f.root) == before);
  }
}
TEST_CASE("ledger record IDs guard signed overflow and preserve existing rows", "[ledger][records]") {
  scratch f;
  auto    path = f.root / "db";
  initialized(path);
  {
    auto db = raw(path);
    raw_record(db, std::numeric_limits<std::int64_t>::max(), file_record{420, "last"});
  }
  auto before = snapshot(f.root);
  auto writer = begin_records(path);
  REQUIRE(writer);
  auto reused = writer->intern(file_record{420, "last"});
  REQUIRE(reused);
  CHECK(reused->id == std::uint64_t(std::numeric_limits<std::int64_t>::max()));
  CHECK_FALSE(writer->intern(directory_record{}));
  CHECK_FALSE(writer->commit());
  CHECK(snapshot(f.root) == before);
}
TEST_CASE("ledger record only backups restoration and real hot recovery preserve records", "[ledger][records][recovery]") {
  scratch f;
  auto    path = f.root / "db";
  initialized(path);
  auto ref = store_file(path);
  REQUIRE(backup_new(path, f.root / "backup"));
  REQUIRE(restore_new(f.root / "backup", f.root / "restore"));
  for (auto name : {"backup", "restore"}) {
    auto reader = open_existing(f.root / name);
    REQUIRE(reader);
    auto found = reader->read_record(ref.id);
    REQUIRE(found);
    CHECK(found->reference.digest == ref.digest);
  }
  // Existing crash helper inserts ID 1, so reserve that ID's data by using its full-page dirty insertion at ID 2.
  auto recovery_path = f.root / "recovery";
  REQUIRE(backup_new(path, recovery_path));
  // crash_dirty uses a fixed ID; its optional parameter keeps the original foundation scenarios unchanged.
  crash_dirty(recovery_path, 2);
  REQUIRE(recover_quiescent(recovery_path));
  auto recovered = open_existing(recovery_path);
  REQUIRE(recovered);
  CHECK(recovered->read_record(ref.id));
  CHECK_FALSE(recovered->read_record(2));
}
TEST_CASE("ledger refuses orphan revision history after valid record storage", "[ledger][records]") {
  scratch f;
  auto    path = f.root / "db";
  initialized(path);
  store_file(path);
  {
    auto db = raw(path);
    sql(db, "INSERT INTO revisions VALUES(1,NULL,1,1,1,1,1,'actor','2026-01-01T00:00:00.000Z')");
  }
  refusal_unchanged(f.root, path, format_error_code::corrupt);
}
TEST_CASE("ledger records refuse malformed metadata and unknown value codecs before use", "[ledger][records]") {
  for (int kind = 0; kind < 4; ++kind) {
    scratch f;
    auto    path = f.root / "db";
    initialized(path);
    {
      auto db = raw(path);
      if (kind < 3) {
        sql(db, "PRAGMA ignore_check_constraints=ON");
        sql(db, kind == 0   ? "INSERT INTO records VALUES(1,1,2,zeroblob(16),x'01')"
                : kind == 1 ? "INSERT INTO records VALUES(1,1,1,zeroblob(17),x'01')"
                            : "INSERT INTO records VALUES(1,1,1,zeroblob(16),zeroblob(67108865))");
      } else {
        auto encoded  = encode_record(value_record{"canonical_v1", std::string(1, '\0')}).value();
        auto position = encoded.find("canonical_v1");
        REQUIRE(position != std::string::npos);
        encoded[position + 11] = '2';
        auto q                 = db.prepare("INSERT INTO records VALUES(1,3,1,zeroblob(16),?1)");
        REQUIRE(q);
        REQUIRE(q->bind_blob(1, encoded));
        REQUIRE(q->step() == step_result::done);
      }
    }
    refusal_unchanged(f.root, path, kind == 3 ? format_error_code::unsupported_state : format_error_code::corrupt);
  }
}

TEST_CASE("ledger reader ordinary failures invalidate through moves and preserve metadata", "[ledger][records]") {
  scratch    f;
  const auto path = f.root / "db";
  initialized(path);
  const auto ref = store_file(path);
  for (const auto id : {std::uint64_t{0}, std::numeric_limits<std::uint64_t>::max(), std::uint64_t{99}}) {
    auto opened = open_existing(path);
    REQUIRE(opened);
    const auto info   = opened->metadata();
    auto       reader = std::move(*opened);
    REQUIRE(reader.read_record(ref.id));
    auto failure = reader.read_record(id);
    REQUIRE_FALSE(failure);
    CHECK(failure.error().code == (id == 99 ? format_error_code::not_found : format_error_code::invalid_argument));
    CHECK(reader.metadata().identity == info.identity);
    auto destination = open_existing(path);
    REQUIRE(destination);
    *destination = std::move(reader);
    auto refused = destination->read_record(ref.id);
    REQUIRE_FALSE(refused);
    CHECK(refused.error().code == format_error_code::invalid_argument);
    CHECK(destination->metadata().identity == info.identity);
    CHECK(destination->metadata().workspace_id == info.workspace_id);
    CHECK(destination->metadata().created_at == info.created_at);
    CHECK_FALSE(reader.read_record(ref.id));
    auto reopened = open_existing(path);
    REQUIRE(reopened);
    CHECK(reopened->read_record(ref.id));
  }
}
TEST_CASE("ledger writer explicit abort is idempotent releases lock and prevents commit", "[ledger]") {
  scratch f;
  initialized(f.root / "db");
  auto writer = begin_records(f.root / "db");
  REQUIRE(writer);
  REQUIRE(writer->intern(planar::engine::history::file_record{420, "pending"}));
  writer->abort();
  writer->abort();
  REQUIRE_FALSE(writer->commit());
  REQUIRE_FALSE(writer->read_record(1));
  REQUIRE_FALSE(writer->intern(planar::engine::history::file_record{}));
  auto next = begin_records(f.root / "db");
  REQUIRE(next);
  auto first = next->intern(planar::engine::history::file_record{420, "committed"});
  REQUIRE(first);
  CHECK(first->id == 1);
  REQUIRE(next->commit());
  next->abort();
}

TEST_CASE("ledger format2 exact schema delta and native initialization retain format", "[ledger][format2]") {
  scratch    f;
  const auto v1 = f.root / "v1", v2 = f.root / "v2";
  REQUIRE(initialize_new(v1, "workspace"));
  const auto info = initialize_new_v2(v2, "workspace");
  REQUIRE(info);
  CHECK(info->format == ledger_format::v2);
  auto one = raw(v1), two = raw(v2);
  CHECK(scalar(one, "PRAGMA user_version") == 1);
  CHECK(scalar(two, "PRAGMA user_version") == 2);
  CHECK(scalar(two, "SELECT format FROM ledger_meta") == 2);
  CHECK(scalar(two, "PRAGMA application_id") == scalar(one, "PRAGMA application_id"));
  auto expected = schema_rows(one);
  for (auto& row : expected) {
    REQUIRE(row[0]);
    if (*row[0] == "ledger_meta") {
      REQUIRE(row[3]);
      auto& text = *row[3];
      text.replace(text.find("CHECK(format=1)"), 15, "CHECK(format=2)");
    } else if (*row[0] == "records") {
      REQUIRE(row[3]);
      auto& text = *row[3];
      text.replace(text.find("CHECK(codec=1)"), 14, "CHECK((kind=4 AND codec IN (1,2)) OR (kind BETWEEN 1 AND 3 AND codec=1))");
    }
  }
  CHECK(schema_rows(two) == expected);
  auto opened = open_existing(v2);
  REQUIRE(opened);
  CHECK(opened->metadata().format == ledger_format::v2);
  CHECK(opened->metadata().identity == info->identity);
  auto head = opened->read_head();
  REQUIRE(head);
  CHECK_FALSE(head->revision);
  CHECK(head->last_sequence == 1);
  CHECK(open_existing(v1)->metadata().format == ledger_format::v1);
}
TEST_CASE("ledger format2 refuses mutation before writable access and preserves codec1 reads", "[ledger][format2]") {
  scratch    f;
  const auto path = f.root / "db";
  REQUIRE(initialize_new_v2(path, "workspace"));
  {
    auto db = raw(path);
    raw_record(db, 1, file_record{420, "retained v1 bytes"});
    raw_record(db, 2, request_record{"load", "actor", "none", "/workspace", "/registry"});
  }
  const auto before = snapshot(f.root);
  auto       opened = open_existing(path);
  REQUIRE(opened);
  auto file = opened->read_record(1);
  REQUIRE(file);
  CHECK(std::get<file_record>(file->value).payload == "retained v1 bytes");
  REQUIRE(opened->read_record(2));
  opened = open_existing(path); // Release the old snapshot before native write acquisition.
  {
    auto writable = open_existing(path, ledger_access::read_write);
    REQUIRE(writable);
  }
  {
    auto writer = begin_records(path);
    REQUIRE(writer);
    writer->abort();
  }
  CHECK(snapshot(f.root) == before);
  // Invalid journals still refuse without mutation after the temporary closure is removed.
  std::ofstream(path.string() + "-journal") << "journal sentinel";
  const auto with_journal = snapshot(f.root);
  auto       recovered    = recover_quiescent(path);
  REQUIRE_FALSE(recovered);
  CHECK(recovered.error().code == format_error_code::corrupt);
  CHECK(snapshot(f.root) == with_journal);
}
TEST_CASE("ledger format2 refuses schema header metadata mismatch and codec2 rows unchanged", "[ledger][format2]") {
  for (int selected = 0; selected < 8; ++selected) {
    scratch    f;
    const auto path = f.root / "db";
    if (selected == 7)
      REQUIRE(initialize_new(path, "workspace"));
    else
      REQUIRE(initialize_new_v2(path, "workspace"));
    {
      auto db = raw(path);
      if (selected == 7)
        sql(db, "PRAGMA user_version=2");
      else if (selected == 0)
        sql(db, "PRAGMA user_version=1");
      else if (selected == 1)
        sql(db, "PRAGMA user_version=3");
      else if (selected == 2)
        sql(db, "PRAGMA application_id=42");
      else if (selected == 3) {
        sql(db, "PRAGMA ignore_check_constraints=ON");
        std::string trigger;
        {
          auto query = db.prepare("SELECT sql FROM sqlite_schema WHERE name='ledger_meta_no_update'");
          REQUIRE(query);
          REQUIRE(query->step() == step_result::row);
          auto text = query->text(0);
          REQUIRE(text);
          REQUIRE(*text);
          trigger = **text;
        }
        sql(db, "DROP TRIGGER ledger_meta_no_update");
        sql(db, "UPDATE ledger_meta SET format=1");
        sql(db, trigger);
      } else if (selected == 4)
        sql(db, "DROP INDEX records_digest");
      else if (selected == 5)
        sql(db, "INSERT INTO records VALUES(1,4,2,zeroblob(16),x'504c414e41522d5245515545535402')");
      else {
        sql(db, "PRAGMA writable_schema=ON");
        sql(db, "UPDATE sqlite_schema SET sql=replace(sql,'CHECK(format=2)','CHECK(format=1)') WHERE name='ledger_meta'");
      }
    }
    refusal_unchanged(f.root, path,
                      selected == 1   ? format_error_code::unsupported_version
                      : selected == 2 ? format_error_code::foreign_ledger
                                      : format_error_code::corrupt);
  }
}
TEST_CASE("ledger format2 backup restore preserve format identity and never overwrite", "[ledger][format2][backup]") {
  scratch    f;
  const auto source = f.root / "source", backup = f.root / "backup", restored = f.root / "restored";
  auto       info = initialize_new_v2(source, "workspace");
  REQUIRE(info);
  const auto original = bytes(source);
  REQUIRE(backup_new(source, backup));
  REQUIRE(restore_new(backup, restored));
  for (const auto& path : {source, backup, restored}) {
    auto opened = open_existing(path);
    REQUIRE(opened);
    CHECK(opened->metadata().identity == info->identity);
    CHECK(opened->metadata().created_at == info->created_at);
    CHECK(opened->metadata().workspace_id == info->workspace_id);
    CHECK(opened->metadata().format == ledger_format::v2);
  }
  CHECK(bytes(source) == original);
  for (auto suffix : {"", "-journal", "-wal", "-shm"}) {
    const auto target   = f.root / "target";
    const auto occupied = std::filesystem::path(target.string() + suffix);
    std::ofstream(occupied) << "occupied sentinel";
    const auto before = snapshot(f.root);
    CHECK_FALSE(initialize_new_v2(target, "workspace"));
    CHECK_FALSE(backup_new(source, target));
    CHECK_FALSE(restore_new(backup, target));
    CHECK(snapshot(f.root) == before);
    std::filesystem::remove(occupied);
  }
  const auto before = snapshot(f.root);
  CHECK_FALSE(initialize_new_v2(f.root / "missing/../target", "workspace"));
  CHECK_FALSE(initialize_new_v2(f.root / "target", "INVALID"));
  CHECK(snapshot(f.root) == before);
}

TEST_CASE("ledger format2 reads native typed embedded request-v2 fixture", "[ledger][format2][request_v2]") {
  scratch f;
  auto    path = f.root / "db";
  REQUIRE(initialize_new_v2(path, "workspace"));
  auto encoded = encode_request_v2(request_v2{"load", "driver:private", "none", "/private/workspace", "/private/registry", {}});
  auto digest  = digest_request_v2(request_v2{"load", "driver:private", "none", "/private/workspace", "/private/registry", {}});
  REQUIRE(encoded);
  REQUIRE(digest);
  {
    auto db = raw(path);
    auto q  = db.prepare("INSERT INTO records VALUES(1,4,2,?1,?2)");
    REQUIRE(q);
    REQUIRE(q->bind_blob(1, {reinterpret_cast<const char*>(digest->data()), digest->size()}));
    REQUIRE(q->bind_blob(2, *encoded));
    REQUIRE(q->step() == step_result::done);
  }
  auto before = bytes(path);
  auto owner  = open_existing(path);
  REQUIRE(owner);
  auto record = owner->read_record(1);
  REQUIRE(record);
  CHECK(record->reference.kind == record_kind::request);
  CHECK(record->reference.digest == *digest);
  CHECK(std::holds_alternative<request_v2>(record->value));
  CHECK(std::get<request_v2>(record->value).workspace_argument == "/private/workspace");
  CHECK(bytes(path) == before);
}

TEST_CASE("ledger format2 structurally decodes private profile request without claiming acceptance",
          "[ledger][format2][request_v2]") {
  scratch f;
  auto    path = f.root / "db";
  REQUIRE(initialize_new_v2(path, "workspace"));
  request_v2 body{"load", "driver:private", "none", "/private/workspace", "/private/registry", "/private/profile"};
  auto       encoded = encode_request_v2(body).value();
  auto       digest  = digest_request_v2(body).value();
  {
    auto db = raw(path);
    auto q  = db.prepare("INSERT INTO records VALUES(1,4,2,?1,?2)");
    REQUIRE(q);
    REQUIRE(q->bind_blob(1, {reinterpret_cast<const char*>(digest.data()), digest.size()}));
    REQUIRE(q->bind_blob(2, encoded));
    REQUIRE(q->step() == step_result::done);
  }
  auto owner = open_existing(path);
  REQUIRE(owner);
  auto record = owner->read_record(1);
  REQUIRE(record);
  CHECK(std::get<request_v2>(record->value).profile_argument == body.profile_argument);
  CHECK_FALSE(owner->read_head()->revision);
  CHECK_FALSE(*owner->read_request("never-accepted"));
}
TEST_CASE("ledger format2 native write transaction opens after complete typed integration", "[ledger][format2]") {
  scratch    f;
  const auto path = f.root / "db";
  REQUIRE(initialize_new_v2(path, "workspace"));
  auto writer = begin_records(path);
  REQUIRE(writer);
  REQUIRE(writer->intern(file_record{420, "native format2"}));
  REQUIRE(writer->commit());
}

namespace {
struct binding_guard final : format_read_guard {
  int              descriptor = -1, binds = 0, checks = 0;
  read_gate_status result = read_gate_status::ready;
  bool             throws = false;
  read_gate_status bind(int fd) override {
    ++binds;
    descriptor = fd;
    CHECK(::fcntl(fd, F_GETFD) >= 0);
    if (throws)
      throw std::bad_alloc();
    return result;
  }
  read_gate_status verify() override {
    ++checks;
    CHECK(descriptor >= 0);
    CHECK(::fcntl(descriptor, F_GETFD) >= 0);
    return result;
  }
};
} // namespace
TEST_CASE("C4 B2 format binds actual held descriptor before interpreting invalid header", "[format][c4b2]") {
  scratch f;
  auto    path = f.root / "ledger";
  std::ofstream(path) << "not a SQLite header";
  auto guard    = std::make_shared<binding_guard>();
  guard->result = read_gate_status::refused;
  auto refused  = open_existing_gated(path, guard);
  REQUIRE_FALSE(refused);
  CHECK(refused.error().code == format_error_code::capture_changed);
  CHECK(guard->binds == 1);
  CHECK(guard->checks == 0);
  auto positive = open_existing(path);
  REQUIRE_FALSE(positive);
  CHECK(positive.error().code != format_error_code::capture_changed);
  guard         = std::make_shared<binding_guard>();
  guard->throws = true;
  CHECK_FALSE(open_existing_gated(path, guard));
  CHECK(guard->binds == 1);
  CHECK_FALSE(open_existing_gated(path, {}));
}
TEST_CASE("C4 B2 same bound guard gates format reads with sticky failure and embedded format parity", "[format][c4b2]") {
  for (bool v2 : {false, true}) {
    scratch f;
    auto    path = f.root / "ledger";
    REQUIRE((v2 ? initialize_new_v2(path, "demo") : initialize_new(path, "demo")));
    auto guard = std::make_shared<binding_guard>();
    auto file  = open_existing_gated(path, guard);
    REQUIRE(file);
    CHECK(guard->binds == 1);
    CHECK(guard->checks > 0);
    CHECK(file->metadata().format == (v2 ? ledger_format::v2 : ledger_format::v1));
    REQUIRE(file->read_head());
    guard->result = read_gate_status::refused;
    // Even a cached native read must inspect the original guard before publishing facts.
    auto failed = file->read_record(1);
    REQUIRE_FALSE(failed);
    CHECK(file->gate_status() == read_gate_status::refused);
    auto calls    = guard->checks;
    guard->result = read_gate_status::ready;
    CHECK_FALSE(file->read_head());
    CHECK(guard->checks == calls);
    CHECK(file->gate_status() == read_gate_status::refused);
    CHECK(file->metadata().workspace_id == "demo");
  }
}
