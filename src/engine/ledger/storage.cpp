module;
#include <cerrno>
#include <fcntl.h>
#include <sqlite3.h>
#include <sys/stat.h>
#include <unistd.h>
module planar.engine.ledger.storage;
import std;
import planar.engine.ledger.sqlite;
import planar.engine.json_codec;
import planar.engine.ledger.history_codec;
import planar.engine.history.tree_core;
import planar.engine.history.provenance_codec;
import planar.engine.history.interpretation_codec;
import planar.engine.history.recorded_interpretation;
import planar.engine.canonical;
namespace planar::engine::ledger {
namespace {
constexpr std::string_view             init_actor = "planar:init";
const std::array<std::string_view, 21> schema_sql = {
    R"SQL(CREATE TABLE ledger_meta (
  singleton INTEGER PRIMARY KEY CHECK(singleton=1),
  format INTEGER NOT NULL CHECK(format=1),
  identity TEXT NOT NULL CHECK(length(identity)=32),
  workspace_id TEXT NOT NULL,
  created_at TEXT NOT NULL
) STRICT)SQL",
    R"SQL(CREATE TABLE records (
  id INTEGER PRIMARY KEY CHECK(id>0),
  kind INTEGER NOT NULL CHECK(kind BETWEEN 1 AND 4),
  codec INTEGER NOT NULL CHECK(codec=1),
  digest BLOB NOT NULL CHECK(length(digest)=16),
  encoded BLOB NOT NULL CHECK(length(encoded)<=67108864)
) STRICT)SQL",
    R"SQL(CREATE INDEX records_digest ON records(codec,kind,digest))SQL",
    R"SQL(CREATE TABLE tree_edges (
  parent_id INTEGER NOT NULL REFERENCES records(id),
  ordinal INTEGER NOT NULL CHECK(ordinal>=0),
  name TEXT NOT NULL,
  child_id INTEGER NOT NULL REFERENCES records(id),
  PRIMARY KEY(parent_id,ordinal), UNIQUE(parent_id,name),
  CHECK(parent_id>child_id)
) STRICT)SQL",
    R"SQL(CREATE TABLE revisions (
  id INTEGER PRIMARY KEY CHECK(id>0),
  parent_id INTEGER REFERENCES revisions(id),
  source_id INTEGER NOT NULL REFERENCES records(id),
  registry_id INTEGER NOT NULL REFERENCES records(id),
  stdlib_id INTEGER NOT NULL REFERENCES records(id),
  interpretation_id INTEGER NOT NULL REFERENCES records(id),
  versions_id INTEGER NOT NULL REFERENCES records(id),
  actor TEXT NOT NULL,
  accepted_at TEXT NOT NULL,
  CHECK(parent_id IS NULL OR parent_id<id)
) STRICT)SQL",
    R"SQL(CREATE TABLE events (
  seq INTEGER PRIMARY KEY CHECK(seq>0),
  type TEXT NOT NULL CHECK(type IN ('ledger_initialized','declaration_accepted','load_noop')),
  version INTEGER NOT NULL CHECK(version=1),
  revision_id INTEGER REFERENCES revisions(id),
  request_id TEXT,
  actor TEXT NOT NULL,
  occurred_at TEXT NOT NULL,
  payload BLOB NOT NULL CHECK(length(payload)<=1048576)
) STRICT)SQL",
    R"SQL(CREATE TABLE current_state (
  singleton INTEGER PRIMARY KEY CHECK(singleton=1),
  active_revision_id INTEGER REFERENCES revisions(id),
  last_seq INTEGER NOT NULL REFERENCES events(seq)
) STRICT)SQL",
    R"SQL(CREATE TABLE requests (
  request_id TEXT PRIMARY KEY,
  body_id INTEGER NOT NULL REFERENCES records(id),
  event_seq INTEGER NOT NULL UNIQUE REFERENCES events(seq),
  revision_id INTEGER NOT NULL REFERENCES revisions(id),
  outcome TEXT NOT NULL CHECK(outcome IN ('accepted','noop')),
  result BLOB NOT NULL CHECK(length(result)<=1048576)
) STRICT)SQL",
    R"SQL(CREATE TRIGGER ledger_meta_no_update BEFORE UPDATE ON ledger_meta BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER ledger_meta_no_delete BEFORE DELETE ON ledger_meta BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER records_no_update BEFORE UPDATE ON records BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER records_no_delete BEFORE DELETE ON records BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER tree_edges_no_update BEFORE UPDATE ON tree_edges BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER tree_edges_no_delete BEFORE DELETE ON tree_edges BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER revisions_no_update BEFORE UPDATE ON revisions BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER revisions_no_delete BEFORE DELETE ON revisions BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER events_no_update BEFORE UPDATE ON events BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER events_no_delete BEFORE DELETE ON events BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER requests_no_update BEFORE UPDATE ON requests BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER requests_no_delete BEFORE DELETE ON requests BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL",
    R"SQL(CREATE TRIGGER current_state_no_delete BEFORE DELETE ON current_state BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END)SQL"};
using failure = std::unexpected<format_error>;
failure refuse(format_error_code code, std::string message, std::optional<types::format_error_detail> detail = {}) {
  return failure(format_error{code, std::move(message), std::move(detail)});
}
failure storage(const sqlite_error& error) {
  if (error.code == SQLITE_BUSY || error.code == SQLITE_LOCKED)
    return refuse(format_error_code::busy, "Ledger storage is busy");
  if (error.code == SQLITE_FULL)
    return refuse(format_error_code::storage_full, "Ledger storage is full");
  if (error.code == SQLITE_CORRUPT || error.code == SQLITE_NOTADB)
    return refuse(format_error_code::corrupt, "Ledger SQLite structure is corrupt");
  return refuse(format_error_code::io, "Ledger storage operation failed");
}
failure storage() {
  return refuse(format_error_code::io, "Ledger storage operation failed");
}
#define LEDGER_TRY(expression)                                                                                                   \
  if (!(expression))                                                                                                             \
  return storage()
} // namespace
namespace native {
bool same_metadata(const ledger_metadata& a, const ledger_metadata& b) {
  return a.identity == b.identity && a.workspace_id == b.workspace_id && a.created_at == b.created_at && a.format == b.format;
}
bool same(const struct stat& a, const struct stat& b) {
  return a.st_dev == b.st_dev && a.st_ino == b.st_ino;
}
std::expected<location, format_error> locate(const std::filesystem::path& path) {
  const auto raw = path.string();
  if (raw.empty() || raw.find('\0') != std::string::npos || path.filename().empty() || path.filename() == "." ||
      path.filename() == "..")
    return refuse(format_error_code::invalid_argument, "An explicit ledger filename is required");
  std::error_code ec;
  auto            absolute = std::filesystem::absolute(path, ec);
  if (ec)
    return refuse(format_error_code::unsafe_path, "Cannot resolve explicit path");
  descriptor dir(::open("/", O_RDONLY | O_DIRECTORY | O_CLOEXEC));
  if (dir.fd < 0)
    return storage();
  // Traverse original components before lexical normalization. In particular missing/.. must fail.
  for (const auto& part : absolute.parent_path().relative_path()) {
    descriptor next(::openat(dir.fd, part.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
    if (next.fd < 0)
      return refuse(format_error_code::unsafe_path, "Parent traversal requires existing real directories");
    dir = std::move(next);
  }
  return location{std::move(dir), absolute.lexically_normal(), absolute.filename().string()};
}
bool identity_matches(const location& where, const struct stat& wanted) {
  struct stat named{}, actual{}, parent{}, path_parent{};
  descriptor  checked(::open(where.path.parent_path().c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
  return checked.fd >= 0 && ::fstat(checked.fd, &path_parent) == 0 && ::fstat(where.parent.fd, &parent) == 0 &&
         same(parent, path_parent) && ::fstatat(where.parent.fd, where.name.c_str(), &named, AT_SYMLINK_NOFOLLOW) == 0 &&
         ::lstat(where.path.c_str(), &actual) == 0 && S_ISREG(named.st_mode) && same(named, wanted) && same(actual, wanted);
}
std::string timestamp() {
  using namespace std::chrono;
  const auto           now = floor<milliseconds>(system_clock::now());
  const auto           day = floor<days>(now);
  const year_month_day ymd(day);
  const hh_mm_ss       tod(now - day);
  return std::format("{:04}-{:02}-{:02}T{:02}:{:02}:{:02}.{:03}Z", int(ymd.year()), unsigned(ymd.month()), unsigned(ymd.day()),
                     tod.hours().count(), tod.minutes().count(), tod.seconds().count(), tod.subseconds().count());
}
std::expected<void, format_error> execute(connection& db, std::string_view sql) {
  auto s = db.prepare(sql);
  if (!s)
    return storage(s.error());
  auto result = s->step();
  if (!result)
    return storage(result.error());
  if (*result != step_result::done)
    return storage();
  return {};
}
std::expected<std::int64_t, format_error> scalar(connection& db, std::string_view sql) {
  auto q = db.prepare(sql);
  if (!q)
    return storage(q.error());
  auto step = q->step();
  if (!step)
    return storage(step.error());
  if (*step != step_result::row)
    return refuse(format_error_code::corrupt, "Ledger query failed");
  auto n = q->integer(0);
  if (!n || !*n)
    return refuse(format_error_code::corrupt, "Invalid ledger scalar");
  step = q->step();
  if (!step)
    return storage(step.error());
  if (*step != step_result::done)
    return refuse(format_error_code::corrupt, "Invalid ledger scalar");
  return **n;
}
std::expected<void, format_error> settings(connection& db, bool configure) {
  if (configure)
    for (auto sql : {"PRAGMA foreign_keys=ON", "PRAGMA synchronous=FULL", "PRAGMA trusted_schema=OFF"}) {
      auto r = execute(db, sql);
      if (!r)
        return r;
    }
  for (auto [sql, wanted] : {std::pair{"PRAGMA foreign_keys", 1}, {"PRAGMA synchronous", 2}, {"PRAGMA trusted_schema", 0}}) {
    auto n = scalar(db, sql);
    if (!n)
      return std::unexpected(n.error());
    if (*n != wanted)
      return refuse(format_error_code::corrupt, "Required SQLite settings unavailable");
  }
  return {};
}
std::array<std::string, 21> selected_schema(ledger_format format) {
  std::array<std::string, 21> selected;
  std::ranges::copy(schema_sql, selected.begin());
  if (format == ledger_format::v2) {
    auto replace = [](std::string& text, std::string_view old_text, std::string_view new_text) {
      text.replace(text.find(old_text), old_text.size(), new_text);
    };
    replace(selected[0], "CHECK(format=1)", "CHECK(format=2)");
    replace(selected[1], "CHECK(codec=1)", "CHECK((kind=4 AND codec IN (1,2)) OR (kind BETWEEN 1 AND 3 AND codec=1))");
  }
  return selected;
}
std::expected<void, format_error> header(int fd) {
  struct stat                    stat{};
  std::array<unsigned char, 100> bytes{};
  if (::fstat(fd, &stat) != 0 || !S_ISREG(stat.st_mode))
    return refuse(format_error_code::unsafe_path, "Ledger must be a regular file");
  if (stat.st_size < 512 || ::pread(fd, bytes.data(), bytes.size(), 0) != static_cast<ssize_t>(bytes.size()))
    return refuse(format_error_code::foreign_ledger, "Empty or truncated ledger header");
  if (std::memcmp(bytes.data(), "SQLite format 3\0", 16) != 0)
    return refuse(format_error_code::foreign_ledger, "Unrecognized ledger header");
  auto u32 = [&](std::size_t i) {
    return (std::uint32_t(bytes[i]) << 24) | (std::uint32_t(bytes[i + 1]) << 16) | (std::uint32_t(bytes[i + 2]) << 8) |
           bytes[i + 3];
  };
  if (u32(68) != 1347178034)
    return refuse(format_error_code::foreign_ledger, "Unrecognized ledger application");
  if (u32(60) != 1 && u32(60) != 2)
    return refuse(format_error_code::unsupported_version, "Unsupported ledger format version");
  if (u32(56) != 1 || bytes[18] != 1 || bytes[19] != 1)
    return refuse(format_error_code::corrupt, "Unsupported ledger encoding or journal mode");
  return {};
}
std::expected<void, format_error> no_sidecars(const location& where) {
  for (auto suffix : {"-journal", "-wal", "-shm"}) {
    struct stat st{};
    if (::fstatat(where.parent.fd, (where.name + suffix).c_str(), &st, AT_SYMLINK_NOFOLLOW) == 0)
      return refuse(format_error_code::recovery_required, "Ledger sidecar requires separately audited recovery");
    if (errno != ENOENT)
      return storage();
  }
  return {};
}
} // namespace native
namespace {
using namespace native;
bool valid_workspace(std::string_view id) {
  if (id.empty() || id.size() > 64 || id.front() < 'a' || id.front() > 'z')
    return false;
  return std::ranges::all_of(id, [](char c) { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'; });
}
bool valid_identity(std::string_view id) {
  return id.size() == 32 && std::ranges::all_of(id, [](char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); });
}
bool valid_time(std::string_view text) {
  if (text.size() != 24 || text[4] != '-' || text[7] != '-' || text[10] != 'T' || text[13] != ':' || text[16] != ':' ||
      text[19] != '.' || text[23] != 'Z')
    return false;
  for (std::size_t i = 0; i < text.size(); ++i)
    if (i != 4 && i != 7 && i != 10 && i != 13 && i != 16 && i != 19 && i != 23 && (text[i] < '0' || text[i] > '9'))
      return false;
  auto number = [&](std::size_t i, std::size_t count) {
    int n = 0;
    for (std::size_t j = i; j < i + count; ++j)
      n = n * 10 + text[j] - '0';
    return n;
  };
  const std::chrono::year_month_day date{std::chrono::year(number(0, 4)), std::chrono::month(number(5, 2)),
                                         std::chrono::day(number(8, 2))};
  return date.ok() && number(11, 2) <= 23 && number(14, 2) <= 59 && number(17, 2) <= 59;
}
struct schema_entry {
  std::string                type, table;
  std::optional<std::string> sql;
};
std::map<std::string, schema_entry> expected_schema(ledger_format format) {
  const auto                            schema = selected_schema(format);
  const std::array<std::string_view, 8> names  = {"ledger_meta", "records", "records_digest", "tree_edges",
                                                  "revisions",   "events",  "current_state",  "requests"};
  std::map<std::string, schema_entry>   expected;
  for (std::size_t i = 0; i < names.size(); ++i)
    expected.emplace(
        names[i], schema_entry{i == 2 ? "index" : "table", i == 2 ? "records" : std::string(names[i]), std::string(schema[i])});
  std::size_t i = 8;
  for (auto table : {"ledger_meta", "records", "tree_edges", "revisions", "events", "requests"})
    for (auto op : {"update", "delete"})
      expected.emplace(std::format("{}_no_{}", table, op), schema_entry{"trigger", table, std::string(schema[i++])});
  expected.emplace("current_state_no_delete", schema_entry{"trigger", "current_state", std::string(schema[i])});
  for (auto table : {"tree_edges", "requests"})
    for (int j = 1; j <= 2; ++j)
      expected.emplace(std::format("sqlite_autoindex_{}_{}", table, j), schema_entry{"index", table, std::nullopt});
  return expected;
}
std::expected<void, format_error> check_schema(connection& db, ledger_format format) {
  auto expected = expected_schema(format);
  auto query =
      db.prepare("SELECT name,type,tbl_name,sql,octet_length(name),octet_length(type),octet_length(tbl_name),octet_length(sql) "
                 "FROM sqlite_schema");
  if (!query)
    return storage(query.error());
  while (true) {
    auto step = query->step();
    if (!step)
      return storage(step.error());
    if (*step == step_result::done)
      break;
    for (int i = 4; i < 8; ++i) {
      auto length = query->integer(i);
      if (!length || (*length && (**length < 0 || **length > 4096)))
        return refuse(format_error_code::corrupt, "Oversized ledger schema entry");
    }
    auto name = query->text(0), type = query->text(1), table = query->text(2), sql = query->text(3);
    if (!name)
      return storage(name.error());
    if (!type)
      return storage(type.error());
    if (!table)
      return storage(table.error());
    if (!sql)
      return storage(sql.error());
    if (!*name)
      return refuse(format_error_code::corrupt, "Malformed ledger schema entry");
    auto found = expected.find(**name);
    if (found == expected.end() || *type != found->second.type || *table != found->second.table || *sql != found->second.sql)
      return refuse(format_error_code::corrupt, "Ledger schema does not match selected format");
    expected.erase(found);
  }
  if (!expected.empty())
    return refuse(format_error_code::corrupt, "Incomplete ledger schema");
  return {};
}
using namespace planar::engine::history;
constexpr std::int64_t max_record_bytes = 67108864;
std::string_view       digest_bytes(const record_digest& digest) {
  return {reinterpret_cast<const char*>(digest.data()), digest.size()};
}
std::expected<stored_record, format_error> load_record(connection& db, std::uint64_t id) {
  if (id == 0 || id > std::uint64_t(std::numeric_limits<std::int64_t>::max()))
    return refuse(format_error_code::invalid_argument, "Invalid native record identifier");
  auto q = db.prepare("SELECT kind,codec,octet_length(digest),octet_length(encoded),digest,encoded FROM records WHERE id=?1");
  if (!q)
    return storage(q.error());
  auto bound = q->bind_integer(1, static_cast<std::int64_t>(id));
  if (!bound)
    return storage(bound.error());
  auto step = q->step();
  if (!step)
    return storage(step.error());
  if (*step == step_result::done)
    return refuse(format_error_code::not_found, "Native record not found");
  auto kind = q->integer(0), codec = q->integer(1), digest_size = q->integer(2), size = q->integer(3);
  if (!kind || !*kind || **kind < 1 || **kind > 4 || !codec || !*codec || (**codec != 1 && **codec != 2) || !digest_size ||
      *digest_size != 16 || !size || !*size || **size < 0 || **size > max_record_bytes)
    return refuse(format_error_code::corrupt, "Invalid record row metadata");
  auto digest = q->blob(4), encoded = q->blob(5);
  if (!digest)
    return storage(digest.error());
  if (!encoded)
    return storage(encoded.error());
  if (!*digest || !*encoded)
    return refuse(format_error_code::corrupt, "Invalid record row bytes");
  if (**codec == 2) {
    auto format = scalar(db, "PRAGMA user_version");
    if (!format)
      return std::unexpected(format.error());
    if (*format != 2 || **kind != 4)
      return refuse(format_error_code::corrupt, "Invalid record codec for selected format");
    auto value = decode_request_v2(**encoded);
    if (!value)
      return refuse(value.error().code == record_error_code::unsupported_codec ? format_error_code::unsupported_state
                                                                               : format_error_code::corrupt,
                    "Unsupported or malformed request encoding");
    auto hash = digest_request_v2(*value);
    if (!hash || digest_bytes(*hash) != **digest)
      return refuse(format_error_code::corrupt, "Request digest disagrees with encoding");
    auto edges = db.prepare("SELECT 1 FROM tree_edges WHERE parent_id=?1 LIMIT 1");
    if (!edges)
      return storage(edges.error());
    auto bound = edges->bind_integer(1, static_cast<std::int64_t>(id));
    if (!bound)
      return storage(bound.error());
    auto step = edges->step();
    if (!step)
      return storage(step.error());
    if (*step != step_result::done)
      return refuse(format_error_code::corrupt, "Request record has directory edges");
    return stored_record{{id, record_kind::request, *hash}, std::move(*value)};
  }
  auto value = decode_record(**encoded);
  if (!value)
    return refuse(value.error().code == record_error_code::unsupported_codec ? format_error_code::unsupported_state
                                                                             : format_error_code::corrupt,
                  "Unsupported or malformed record encoding");
  auto hash = digest_record(*value);
  if (!hash || static_cast<std::int64_t>(kind_of(*value)) != **kind || digest_bytes(*hash) != **digest)
    return refuse(format_error_code::corrupt, "Record kind or digest disagrees with encoding");
  auto edges = db.prepare("SELECT ordinal,octet_length(name),name,child_id FROM tree_edges WHERE parent_id=?1 ORDER BY ordinal");
  if (!edges)
    return storage(edges.error());
  bound = edges->bind_integer(1, static_cast<std::int64_t>(id));
  if (!bound)
    return storage(bound.error());
  auto        directory = std::get_if<directory_record>(&*value);
  std::size_t index     = 0;
  while (true) {
    auto next = edges->step();
    if (!next)
      return storage(next.error());
    if (*next == step_result::done)
      break;
    auto ordinal = edges->integer(0), length = edges->integer(1), child_id = edges->integer(3);
    if (!directory || index >= directory->children.size() || !ordinal || *ordinal != static_cast<std::int64_t>(index) ||
        !length || !*length || **length < 1 || **length > 128 || !child_id || !*child_id || **child_id < 1 ||
        std::uint64_t(**child_id) >= id)
      return refuse(format_error_code::corrupt, "Invalid record edge metadata");
    auto name = edges->text(2);
    if (!name)
      return storage(name.error());
    const auto& child = directory->children[index++];
    if (*name != child.name || std::uint64_t(**child_id) != child.id)
      return refuse(format_error_code::corrupt, "Record edges disagree with encoding");
  }
  if (directory && index != directory->children.size())
    return refuse(format_error_code::corrupt, "Missing record edges");
  if (directory) {
    // References are metadata-only here; the streaming outer pass validates every child's own bytes and edges.
    for (const auto& child : directory->children) {
      if (child.id >= id)
        return refuse(format_error_code::corrupt, "Record child is not older than parent");
      if (child.kind == record_kind::request) {
        auto format = scalar(db, "PRAGMA user_version");
        if (!format)
          return std::unexpected(format.error());
        if (*format == 2)
          return refuse(format_error_code::corrupt, "Request record is not a directory child");
      }
      auto ref = db.prepare("SELECT kind,octet_length(digest),digest FROM records WHERE id=?1");
      if (!ref)
        return storage(ref.error());
      bound = ref->bind_integer(1, static_cast<std::int64_t>(child.id));
      if (!bound)
        return storage(bound.error());
      auto child_step = ref->step();
      if (!child_step)
        return storage(child_step.error());
      if (*child_step != step_result::row)
        return refuse(format_error_code::corrupt, "Missing record child");
      auto child_kind = ref->integer(0), length = ref->integer(1);
      if (!child_kind || *child_kind != static_cast<std::int64_t>(child.kind) || !length || *length != 16)
        return refuse(format_error_code::corrupt, "Invalid record child metadata");
      auto hash_bytes = ref->blob(2);
      if (!hash_bytes)
        return storage(hash_bytes.error());
      if (*hash_bytes != digest_bytes(child.digest))
        return refuse(format_error_code::corrupt, "Record child digest disagrees");
    }
  }
  return stored_record{{id, kind_of(*value), *hash}, std::move(*value)};
}
std::expected<void, format_error> validate_records(connection& db) {
  auto rows = db.prepare("SELECT id FROM records ORDER BY id");
  if (!rows)
    return storage(rows.error());
  while (true) {
    auto step = rows->step();
    if (!step)
      return storage(step.error());
    if (*step == step_result::done)
      return {};
    auto id = rows->integer(0);
    if (!id || !*id || **id < 1)
      return refuse(format_error_code::corrupt, "Invalid record identifier");
    auto value = load_record(db, static_cast<std::uint64_t>(**id));
    if (!value)
      return std::unexpected(value.error());
    auto encoded = types::encode_stored_record(*value);
    if (!encoded)
      return refuse(format_error_code::corrupt, "Invalid stored record");
    // Every bucket member is compared, even when an exact member was already found.
    auto bucket = db.prepare("SELECT octet_length(encoded),encoded FROM records WHERE codec=?3 AND kind=?1 AND digest=?2");
    if (!bucket)
      return storage(bucket.error());
    auto kind_bound = bucket->bind_integer(1, static_cast<std::int64_t>(value->reference.kind));
    if (!kind_bound)
      return storage(kind_bound.error());
    auto digest_bound = bucket->bind_blob(2, digest_bytes(value->reference.digest));
    if (!digest_bound)
      return storage(digest_bound.error());
    auto codec_bound = bucket->bind_integer(3, std::holds_alternative<request_v2>(value->value) ? 2 : 1);
    if (!codec_bound)
      return storage(codec_bound.error());
    while (true) {
      auto next = bucket->step();
      if (!next)
        return storage(next.error());
      if (*next == step_result::done)
        break;
      auto length = bucket->integer(0);
      if (!length || !*length || **length < 0 || **length > max_record_bytes)
        return refuse(format_error_code::corrupt, "Invalid digest bucket size");
      auto bytes = bucket->blob(1);
      if (!bytes)
        return storage(bytes.error());
      if (*bytes != *encoded)
        return refuse(format_error_code::corrupt, "Ambiguous record digest bucket");
    }
  }
}

// Every cursor is forward-only; retained history state is one head and its five references.
failure history_corrupt() {
  return refuse(format_error_code::corrupt, "Accepted history is inconsistent");
}
std::expected<std::string, format_error> bounded_column(statement& row, int index, bool blob, std::int64_t maximum) {
  auto length = row.integer(index);
  if (!length || !*length || **length < 0 || **length > maximum)
    return history_corrupt();
  auto bytes = blob ? row.blob(index + 1) : row.text(index + 1);
  if (!bytes)
    return storage(bytes.error());
  if (!*bytes)
    return history_corrupt();
  return std::move(**bytes);
}
// Column policy failures describe corrupt rows; actual SQLite failures retain storage classification.
failure row_failure(const sqlite_error& error) {
  return error.code == SQLITE_MISUSE ? history_corrupt() : storage(error);
}
std::expected<std::optional<std::int64_t>, format_error> row_integer(statement& row, int column) {
  auto result = row.integer(column);
  if (!result)
    return row_failure(result.error());
  return *result;
}
std::expected<std::string, format_error> row_bytes(statement& row, int column, bool blob, std::int64_t maximum) {
  auto length = row_integer(row, column);
  if (!length)
    return std::unexpected(length.error());
  if (!*length || **length < 0 || **length > maximum)
    return history_corrupt();
  auto bytes = blob ? row.blob(column + 1) : row.text(column + 1);
  if (!bytes)
    return row_failure(bytes.error());
  if (!*bytes)
    return history_corrupt();
  return std::move(**bytes);
}
bool valid_request_key(std::string_view key) {
  return !key.empty() && key.size() <= 128 && std::ranges::all_of(key, [](unsigned char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '.' || c == ':' ||
           c == '-';
  });
}
bool positive_id(std::uint64_t id) {
  return id > 0 && id <= std::uint64_t(std::numeric_limits<std::int64_t>::max());
}
std::expected<history_head, format_error> load_head(connection& db) {
  auto row = db.prepare("SELECT active_revision_id,last_seq FROM current_state WHERE singleton=1");
  if (!row)
    return storage(row.error());
  auto step = row->step();
  if (!step)
    return storage(step.error());
  if (*step != step_result::row)
    return history_corrupt();
  auto head = row_integer(*row, 0), last = row_integer(*row, 1);
  if (!head)
    return std::unexpected(head.error());
  if (!last)
    return std::unexpected(last.error());
  if ((*head && **head <= 0) || !*last || **last <= 0)
    return history_corrupt();
  return history_head{*head, **last};
}
std::expected<revision_row, format_error> decode_revision_row(statement& row) {
  auto id = row_integer(row, 0), parent = row_integer(row, 1);
  if (!id)
    return std::unexpected(id.error());
  if (!parent)
    return std::unexpected(parent.error());
  if (!*id || **id <= 0 || (*parent && (**parent <= 0 || **parent >= **id)))
    return history_corrupt();
  std::array<std::int64_t, 5> refs{};
  for (int i = 0; i < 5; ++i) {
    auto value = row_integer(row, i + 2);
    if (!value)
      return std::unexpected(value.error());
    if (!*value || **value <= 0)
      return history_corrupt();
    refs[i] = **value;
  }
  auto actor = row_bytes(row, 7, false, 256), time = row_bytes(row, 9, false, 24);
  if (!actor)
    return std::unexpected(actor.error());
  if (!time)
    return std::unexpected(time.error());
  if (actor->empty() || !valid_text(*actor) || !valid_time(*time))
    return history_corrupt();
  return revision_row{**id, *parent, {refs[0], refs[1], refs[2], refs[3], refs[4]}, std::move(*actor), std::move(*time)};
}
constexpr std::string_view event_columns =
    "SELECT "
    "seq,octet_length(type),type,version,revision_id,octet_length(request_id),request_id,octet_length(actor),actor,octet_length("
    "occurred_at),occurred_at,octet_length(payload),payload FROM events ";
std::expected<event_row, format_error> decode_event_row(statement& row) {
  auto seq = row_integer(row, 0), version = row_integer(row, 3), revision = row_integer(row, 4),
       request_length = row_integer(row, 5);
  if (!seq)
    return std::unexpected(seq.error());
  if (!version)
    return std::unexpected(version.error());
  if (!revision)
    return std::unexpected(revision.error());
  if (!request_length)
    return std::unexpected(request_length.error());
  if (!*seq || **seq <= 0 || !*version)
    return history_corrupt();
  if (**version != 1)
    return refuse(format_error_code::unsupported_state, "Unsupported event version");
  auto type = row_bytes(row, 1, false, 20), actor = row_bytes(row, 7, false, 256), time = row_bytes(row, 9, false, 24),
       payload = row_bytes(row, 11, true, 1048576);
  if (!type)
    return std::unexpected(type.error());
  if (!actor)
    return std::unexpected(actor.error());
  if (!time)
    return std::unexpected(time.error());
  if (!payload)
    return std::unexpected(payload.error());
  if (actor->empty() || !valid_text(*actor) || !valid_time(*time))
    return history_corrupt();
  std::optional<std::string> request;
  if (*request_length) {
    auto key = row_bytes(row, 5, false, 128);
    if (!key)
      return std::unexpected(key.error());
    if (!valid_request_key(*key))
      return history_corrupt();
    request = std::move(*key);
  }
  auto decoded = decode_history_event(*payload);
  if (!decoded)
    return decoded.error() == history_codec_error::unsupported_version
               ? refuse(format_error_code::unsupported_state, "Unsupported event payload version")
               : history_corrupt();
  if (**seq == 1) {
    if (*type != "ledger_initialized" || *revision || request || decoded->reason != event_reason::initialized)
      return history_corrupt();
  } else if (!*revision || **revision <= 0 || !request || decoded->after != *revision ||
             (*type == "declaration_accepted" ? decoded->reason != event_reason::declaration_accepted
                                              : *type != "load_noop" || decoded->reason != event_reason::identical_declaration))
    return history_corrupt();
  return event_row{**seq,
                   std::move(*type),
                   **version,
                   *revision,
                   std::move(request),
                   std::move(*actor),
                   std::move(*time),
                   std::move(*payload)};
}
std::expected<request_row, format_error> decode_request_row(connection& db, statement& row) {
  auto key = row_bytes(row, 0, false, 128), outcome = row_bytes(row, 5, false, 8), result = row_bytes(row, 7, true, 1048576);
  if (!key)
    return std::unexpected(key.error());
  if (!outcome)
    return std::unexpected(outcome.error());
  if (!result)
    return std::unexpected(result.error());
  auto body = row_integer(row, 2), event = row_integer(row, 3), revision = row_integer(row, 4);
  if (!body)
    return std::unexpected(body.error());
  if (!event)
    return std::unexpected(event.error());
  if (!revision)
    return std::unexpected(revision.error());
  if (!valid_request_key(*key) || !*body || **body <= 0 || !*event || **event <= 0 || !*revision || **revision <= 0 ||
      (*outcome != "accepted" && *outcome != "noop"))
    return history_corrupt();
  auto decoded = decode_load_result(*result);
  if (!decoded)
    return decoded.error() == history_codec_error::unsupported_version
               ? refuse(format_error_code::unsupported_state, "Unsupported result version")
               : history_corrupt();
  auto typed_outcome = *outcome == "accepted" ? load_outcome::accepted : load_outcome::noop;
  if (decoded->request_id != *key || decoded->event != **event || decoded->revision != **revision ||
      decoded->outcome != typed_outcome)
    return history_corrupt();
  auto record = load_record(db, std::uint64_t(**body));
  if (!record)
    return std::unexpected(record.error());
  if (!std::holds_alternative<request_record>(record->value) && !std::holds_alternative<request_v2>(record->value))
    return history_corrupt();
  auto raw = db.prepare("SELECT octet_length(encoded),encoded FROM records WHERE id=?1");
  if (!raw)
    return storage(raw.error());
  auto bound = raw->bind_integer(1, **body);
  if (!bound)
    return storage(bound.error());
  auto step = raw->step();
  if (!step)
    return storage(step.error());
  if (*step != step_result::row)
    return history_corrupt();
  auto encoded = row_bytes(*raw, 0, true, 67108864);
  if (!encoded)
    return std::unexpected(encoded.error());
  return request_row{std::move(*key), record->reference, std::move(*encoded), **event,
                     **revision,      typed_outcome,     std::move(*result)};
}
std::expected<recorded_family, format_error> verify_content(connection& db, const event_references& refs,
                                                            std::string_view workspace) {
  auto reader = [&](std::uint64_t id) { return load_record(db, id); };
  auto tree   = [&](std::int64_t id, tree_profile profile) -> std::expected<std::vector<tree_file>, format_error> {
    auto root = load_record(db, static_cast<std::uint64_t>(id));
    if (!root)
      return std::unexpected(root.error());
    if (root->reference.kind != record_kind::directory)
      return history_corrupt();
    auto result = read_tree_core(root->reference, profile, reader);
    if (!result) {
      if (result.error().storage)
        return std::unexpected(*result.error().storage);
      return history_corrupt();
    }
    return std::move(*result);
  };
  { // Release each complete tree before reconstructing the next independently bounded tree.
    auto source = tree(refs.source, tree_profile::source);
    if (!source)
      return std::unexpected(source.error());
  }
  {
    auto registry = tree(refs.registry, tree_profile::provenance);
    if (!registry)
      return std::unexpected(registry.error());
    if (registry->size() != 1 || registry->front().path != "input.json")
      return history_corrupt();
  }
  auto interpretation = load_record(db, static_cast<std::uint64_t>(refs.interpretation));
  auto versions       = load_record(db, static_cast<std::uint64_t>(refs.versions));
  if (!interpretation)
    return std::unexpected(interpretation.error());
  if (!versions)
    return std::unexpected(versions.error());
  auto iv = std::get_if<value_record>(&interpretation->value), vv = std::get_if<value_record>(&versions->value);
  if (!iv || !vv)
    return history_corrupt();
  auto family = classify_recorded_interpretation(iv->encoded, vv->encoded);
  if (!family)
    return history_corrupt();
  auto files =
      tree(refs.stdlib, *family == recorded_family::effective ? tree_profile::effective_provenance : tree_profile::provenance);
  if (!files)
    return std::unexpected(files.error());
  auto verified = verify_recorded_interpretation(iv->encoded, vv->encoded, *files, workspace);
  if (!verified) {
    if (*family == recorded_family::effective && verified.error().code == recorded_error_code::unsupported_manifest)
      return refuse(format_error_code::unsupported_state, "Unsupported recorded provenance");
    return history_corrupt();
  }
  return verified->family;
}
// Compare exact bounded record bytes, including directory child identities, rather than trusting hashes or IDs.
// validate_records has already rejected every digest bucket with unequal complete encodings.
std::expected<bool, format_error> identical_content_rows(connection& db, const event_references& a, const event_references& b) {
  const std::array<std::int64_t, 5> left{a.source, a.registry, a.stdlib, a.interpretation, a.versions};
  const std::array<std::int64_t, 5> right{b.source, b.registry, b.stdlib, b.interpretation, b.versions};
  for (std::size_t i = 0; i < left.size(); ++i) {
    if (left[i] == right[i])
      continue;
    auto x = load_record(db, static_cast<std::uint64_t>(left[i]));
    if (!x)
      return std::unexpected(x.error());
    auto y = load_record(db, static_cast<std::uint64_t>(right[i]));
    if (!y)
      return std::unexpected(y.error());
    auto encoded_x = types::encode_stored_record(*x), encoded_y = types::encode_stored_record(*y);
    if (!encoded_x || !encoded_y)
      return history_corrupt();
    if (*encoded_x != *encoded_y)
      return false;
  }
  return true;
}
std::expected<void, format_error> verify_history(connection& db, const ledger_metadata& metadata) {
  auto rows = db.prepare("SELECT seq,type,version,revision_id,octet_length(request_id),request_id,"
                         "octet_length(actor),actor,octet_length(occurred_at),occurred_at,octet_length(payload),payload "
                         "FROM events ORDER BY seq");
  if (!rows)
    return storage(rows.error());
  std::int64_t                    seq = 0, accepted = 0, requests = 0;
  std::optional<std::int64_t>     head, parent;
  std::optional<event_references> active;
  std::optional<recorded_family>  active_family;
  while (true) {
    auto step = rows->step();
    if (!step)
      return storage(step.error());
    if (*step == step_result::done)
      break;
    auto sequence = rows->integer(0), version = rows->integer(2), revision = rows->integer(3);
    if (!sequence || !*sequence || seq == std::numeric_limits<std::int64_t>::max() || **sequence != seq + 1 || !version ||
        !*version || !revision)
      return history_corrupt();
    ++seq;
    if (**version != 1)
      return refuse(format_error_code::unsupported_state, "Unsupported event version");
    auto type = rows->text(1);
    if (!type)
      return storage(type.error());
    auto actor = bounded_column(*rows, 6, false, 256), time = bounded_column(*rows, 8, false, 24),
         payload = bounded_column(*rows, 10, true, 1048576);
    if (!type || !*type || !actor || !time || !payload)
      return !actor     ? std::unexpected(actor.error())
             : !time    ? std::unexpected(time.error())
             : !payload ? std::unexpected(payload.error())
                        : history_corrupt();
    if (actor->empty() || !valid_text(*actor) || !valid_time(*time))
      return history_corrupt();
    auto event = decode_history_event(*payload);
    if (!event)
      return event.error() == history_codec_error::unsupported_version
                 ? refuse(format_error_code::unsupported_state, "Unsupported event payload version")
                 : history_corrupt();
    if (seq == 1) {
      auto request_length = rows->integer(4);
      if (**type != "ledger_initialized" || *revision || !request_length || *request_length || *actor != init_actor ||
          *time != metadata.created_at || *event != history_event{{}, {}, event_reason::initialized, {}})
        return history_corrupt();
      continue;
    }
    auto request_id = bounded_column(*rows, 4, false, 128);
    if (!request_id)
      return std::unexpected(request_id.error());
    if (!*revision || event->before != head || event->after != *revision || !event->references)
      return history_corrupt();
    bool is_accepted = **type == "declaration_accepted";
    if (is_accepted) {
      if (event->reason != event_reason::declaration_accepted || (!head && **revision != 1) || (head && **revision <= *head))
        return history_corrupt();
      auto row = db.prepare("SELECT parent_id,source_id,registry_id,stdlib_id,interpretation_id,versions_id,"
                            "octet_length(actor),actor,octet_length(accepted_at),accepted_at FROM revisions WHERE id=?1");
      if (!row)
        return storage(row.error());
      auto bound = row->bind_integer(1, **revision);
      if (!bound)
        return storage(bound.error());
      auto next = row->step();
      if (!next)
        return storage(next.error());
      if (*next != step_result::row)
        return history_corrupt();
      auto prior = row->integer(0);
      if (!prior || *prior != head)
        return history_corrupt();
      std::array<std::int64_t, 5> ids{};
      for (int i = 0; i < 5; ++i) {
        auto id = row->integer(i + 1);
        if (!id || !*id || **id <= 0)
          return history_corrupt();
        ids[i] = **id;
      }
      event_references refs{ids[0], ids[1], ids[2], ids[3], ids[4]};
      auto             revision_actor = bounded_column(*row, 6, false, 256), revision_time = bounded_column(*row, 8, false, 24);
      if (!revision_actor)
        return std::unexpected(revision_actor.error());
      if (!revision_time)
        return std::unexpected(revision_time.error());
      if (*revision_actor != *actor || *revision_time != *time || refs != *event->references)
        return history_corrupt();
      auto content = verify_content(db, refs, metadata.workspace_id);
      if (!content)
        return std::unexpected(content.error());
      if (active) {
        auto identical = identical_content_rows(db, refs, *active);
        if (!identical)
          return std::unexpected(identical.error());
        if (*identical)
          return history_corrupt();
      }
      parent        = head;
      head          = *revision;
      active        = refs;
      active_family = *content;
      ++accepted;
    } else if (**type != "load_noop" || event->reason != event_reason::identical_declaration || !head || event->after != head ||
               event->references != active)
      return history_corrupt();
    auto request = db.prepare("SELECT body_id,event_seq,revision_id,outcome,octet_length(result),result "
                              "FROM requests WHERE request_id=?1");
    if (!request)
      return storage(request.error());
    auto bound = request->bind_text(1, *request_id);
    if (!bound)
      return storage(bound.error());
    auto next = request->step();
    if (!next)
      return storage(next.error());
    if (*next != step_result::row)
      return history_corrupt();
    auto body_id = request->integer(0), event_seq = request->integer(1), request_revision = request->integer(2);
    auto outcome = request->text(3);
    if (!outcome)
      return storage(outcome.error());
    if (!body_id || !*body_id || **body_id <= 0 || !event_seq || *event_seq != seq || !request_revision ||
        *request_revision != head || !outcome || *outcome != (is_accepted ? "accepted" : "noop"))
      return history_corrupt();
    auto result_bytes = bounded_column(*request, 4, true, 1048576);
    if (!result_bytes)
      return std::unexpected(result_bytes.error());
    auto result = decode_load_result(*result_bytes);
    if (!result)
      return result.error() == history_codec_error::unsupported_version
                 ? refuse(format_error_code::unsupported_state, "Unsupported result version")
                 : history_corrupt();
    if (*result != load_result{metadata.identity, *request_id, is_accepted ? load_outcome::accepted : load_outcome::noop, *head,
                               parent, seq})
      return history_corrupt();
    auto body = load_record(db, static_cast<std::uint64_t>(**body_id));
    if (!body)
      return std::unexpected(body.error());
    bool matches = std::visit(
        [&](const auto& typed) {
          using T = std::decay_t<decltype(typed)>;
          if constexpr (std::is_same_v<T, request_record> || std::is_same_v<T, request_v2>)
            return typed.operation == "load" && typed.actor == *actor &&
                   typed.expected_head == (event->before ? std::to_string(*event->before) : "none");
          else
            return false;
        },
        body->value);
    if (!matches)
      return history_corrupt();
    // Selector presence is private intent; its host path never participates in portable provenance equality.
    auto typed            = std::get_if<request_v2>(&body->value);
    auto requested_family = typed && typed->profile_argument ? recorded_family::effective : recorded_family::embedded;
    if (!active_family || *active_family != requested_family)
      return history_corrupt();
    ++requests;
  }
  auto revisions = scalar(db, "SELECT count(*) FROM revisions"), saved = scalar(db, "SELECT count(*) FROM requests");
  if (!revisions)
    return std::unexpected(revisions.error());
  if (!saved)
    return std::unexpected(saved.error());
  if (!seq || *revisions != accepted || *saved != requests)
    return history_corrupt();
  auto current = db.prepare("SELECT singleton,active_revision_id,last_seq FROM current_state");
  if (!current)
    return storage(current.error());
  auto step = current->step();
  if (!step)
    return storage(step.error());
  if (*step != step_result::row)
    return history_corrupt();
  auto singleton = current->integer(0), current_head = current->integer(1), last_seq = current->integer(2);
  if (!singleton || *singleton != 1 || !current_head || *current_head != head || !last_seq || *last_seq != seq)
    return history_corrupt();
  return {};
}
} // namespace
namespace native {
std::expected<ledger_metadata, format_error> verify(connection& db) {
  auto setup = settings(db, false);
  if (!setup)
    return std::unexpected(setup.error());
  auto version = scalar(db, "PRAGMA user_version");
  if (!version)
    return std::unexpected(version.error());
  if (*version != 1 && *version != 2)
    return refuse(format_error_code::unsupported_version, "Unsupported ledger format version");
  const auto format = static_cast<ledger_format>(*version);
  for (auto [sql, wanted] : {std::pair{"PRAGMA application_id", 1347178034}}) {
    auto n = scalar(db, sql);
    if (!n)
      return std::unexpected(n.error());
    if (*n != wanted)
      return refuse(format_error_code::corrupt, "Ledger header changed");
  }
  for (auto [sql, wanted] : {std::pair{"PRAGMA encoding", "UTF-8"}, {"PRAGMA journal_mode", "delete"}}) {
    auto q = db.prepare(sql);
    if (!q)
      return storage(q.error());
    auto step = q->step();
    if (!step)
      return storage(step.error());
    if (*step != step_result::row)
      return refuse(format_error_code::corrupt, "Ledger integrity unavailable");
    auto text = q->text(0);
    if (!text)
      return storage(text.error());
    if (*text != wanted)
      return refuse(format_error_code::corrupt, "Ledger integrity or storage policy failed");
    step = q->step();
    if (!step)
      return storage(step.error());
    if (*step != step_result::done)
      return refuse(format_error_code::corrupt, "Ledger integrity or storage policy failed");
  }
  auto schema = check_schema(db, format);
  if (!schema)
    return std::unexpected(schema.error());
  // Inspect codec negotiation after exact schema validation, before CHECK diagnostics mask unknown versions.
  auto unknown_event = scalar(db, "SELECT EXISTS(SELECT 1 FROM events WHERE version<>1)");
  if (!unknown_event)
    return std::unexpected(unknown_event.error());
  if (*unknown_event)
    return refuse(format_error_code::unsupported_state, "Unsupported event version");
  auto unknown_codec = scalar(db, "SELECT EXISTS(SELECT 1 FROM records WHERE codec NOT IN (1,2))");
  if (!unknown_codec)
    return std::unexpected(unknown_codec.error());
  if (*unknown_codec)
    return refuse(format_error_code::unsupported_state, "Unsupported stored record codec");
  auto integrity = db.prepare("PRAGMA integrity_check");
  if (!integrity)
    return storage(integrity.error());
  auto integrity_step = integrity->step();
  if (!integrity_step)
    return storage(integrity_step.error());
  if (*integrity_step != step_result::row)
    return refuse(format_error_code::corrupt, "Ledger integrity unavailable");
  auto integrity_text = integrity->text(0);
  if (!integrity_text)
    return storage(integrity_text.error());
  if (*integrity_text != "ok")
    return refuse(format_error_code::corrupt, "Ledger integrity failed");
  integrity_step = integrity->step();
  if (!integrity_step)
    return storage(integrity_step.error());
  if (*integrity_step != step_result::done)
    return refuse(format_error_code::corrupt, "Ledger integrity failed");
  auto fk = db.prepare("PRAGMA foreign_key_check");
  if (!fk)
    return storage(fk.error());
  auto fk_step = fk->step();
  if (!fk_step)
    return storage(fk_step.error());
  if (*fk_step != step_result::done)
    return refuse(format_error_code::corrupt, "Ledger foreign keys are inconsistent");
  auto meta_count    = scalar(db, "SELECT count(*) FROM ledger_meta"),
       current_count = scalar(db, "SELECT count(*) FROM current_state");
  if (!meta_count)
    return std::unexpected(meta_count.error());
  if (!current_count)
    return std::unexpected(current_count.error());
  if (*meta_count != 1 || *current_count != 1)
    return refuse(format_error_code::corrupt, "Ledger singleton rows are inconsistent");
  auto q = db.prepare("SELECT singleton,format,identity,workspace_id,created_at,"
                      "octet_length(identity),octet_length(workspace_id),octet_length(created_at) FROM ledger_meta");
  if (!q)
    return storage(q.error());
  auto meta_step = q->step();
  if (!meta_step)
    return storage(meta_step.error());
  if (*meta_step != step_result::row)
    return refuse(format_error_code::corrupt, "Ledger metadata unavailable");
  for (auto [i, wanted] : {std::pair{0, 1}, {1, static_cast<int>(format)}, {5, 32}, {7, 24}}) {
    auto n = q->integer(i);
    if (!n || *n != wanted)
      return refuse(format_error_code::corrupt, "Invalid ledger metadata");
  }
  auto len = q->integer(6);
  if (!len || !*len || **len < 1 || **len > 64)
    return refuse(format_error_code::corrupt, "Invalid workspace identifier length");
  auto id = q->text(2), workspace = q->text(3), time = q->text(4);
  if (!id)
    return storage(id.error());
  if (!workspace)
    return storage(workspace.error());
  if (!time)
    return storage(time.error());
  if (!*id || !*workspace || !*time || !valid_identity(**id) || !valid_workspace(**workspace) || !valid_time(**time))
    return refuse(format_error_code::corrupt, "Invalid ledger metadata values");
  ledger_metadata metadata{**id, **workspace, **time, format};
  auto            records = validate_records(db);
  if (!records)
    return std::unexpected(records.error());
  auto history = verify_history(db, metadata);
  if (!history)
    return std::unexpected(history.error());
  return metadata;
}
} // namespace native
/// @brief Thread-confined descriptor, connection and snapshot ownership, destroyed in reverse dependency order.
struct storage_state {
  descriptor                          file;     ///< Original no-follow file identity retained through connection lifetime.
  location                            where;    ///< Retained parent directory and explicit pathname.
  connection                          db;       ///< Private trusted primitive, never exposed by the format API.
  transaction                         snapshot; ///< Pinned verified read transaction.
  ledger_metadata                     info;     ///< Values copied from that snapshot.
  std::shared_ptr<storage_read_guard> guard;    ///< Same original guard, also checked around cached native reads.
  read_gate_status                    gate_failure = read_gate_status::ready; ///< First format-boundary refusal.
  bool                                readable = true; ///< Sticky reader validity; a failed read may have ended the transaction.
};
namespace {
read_gate_status verify_gate(const std::shared_ptr<storage_read_guard>& guard) noexcept {
  if (!guard)
    return read_gate_status::ready;
  try {
    auto status = guard->verify();
    return status == read_gate_status::ready || status == read_gate_status::refused || status == read_gate_status::resource
               ? status
               : read_gate_status::invalid_status;
  } catch (const std::bad_alloc&) {
    return read_gate_status::resource;
  } catch (const std::length_error&) {
    return read_gate_status::resource;
  } catch (...) {
    return read_gate_status::verifier_exception;
  }
}
bool check_gate(storage_state& state) noexcept {
  if (state.gate_failure == read_gate_status::ready)
    state.gate_failure = state.db.gate_status();
  if (state.gate_failure == read_gate_status::ready)
    state.gate_failure = verify_gate(state.guard);
  return state.gate_failure == read_gate_status::ready;
}
} // namespace
read_snapshot::read_snapshot(std::shared_ptr<storage_state> state) : _state(std::move(state)) {
}
const ledger_metadata& read_snapshot::metadata() const {
  return _state->info;
}
read_gate_status read_snapshot::gate_status() const noexcept {
  return !_state                                           ? read_gate_status::ready
         : _state->gate_failure != read_gate_status::ready ? _state->gate_failure
                                                           : _state->db.gate_status();
}
std::expected<read_snapshot, format_error> read_snapshot::open_read(const std::filesystem::path& path, ledger_access access,
                                                                    std::shared_ptr<storage_read_guard> guard) {
  if (access != ledger_access::read_only && access != ledger_access::read_write)
    return refuse(format_error_code::invalid_argument, "Invalid ledger access mode");
  auto where = locate(path);
  if (!where)
    return std::unexpected(where.error());
  descriptor held(::openat(where->parent.fd, where->name.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK));
  if (held.fd < 0)
    return refuse(errno == ENOENT ? format_error_code::not_found : format_error_code::unsafe_path,
                  "Cannot open explicit regular ledger");
  struct stat identity{};
  if (::fstat(held.fd, &identity) != 0 || !S_ISREG(identity.st_mode))
    return refuse(format_error_code::unsafe_path, "Ledger must be a regular file");
  if (guard) {
    // Bind the actual descriptor before the first raw pread. No adapter exception crosses this boundary.
    try {
      if (guard->bind(held.fd) != read_gate_status::ready)
        return refuse(format_error_code::capture_changed, "Native ledger read binding refused");
    } catch (...) {
      return refuse(format_error_code::capture_changed, "Native ledger read binding refused");
    }
  }
  auto checked = header(held.fd);
  if (!checked)
    return std::unexpected(checked.error());
  auto sidecars = no_sidecars(*where);
  if (!sidecars)
    return std::unexpected(sidecars.error());
  if (!identity_matches(*where, identity))
    return refuse(format_error_code::unsafe_path, "Ledger identity changed");
  auto db = guard ? connection::open_gated(where->path, guard) : connection::open(where->path, open_mode::read_only);
  if (!db)
    return storage();
  auto configured = settings(*db);
  if (!configured)
    return std::unexpected(configured.error());
  auto tx = db->begin(false);
  if (!tx)
    return storage();
  auto info = verify(*db);
  if (!info)
    return std::unexpected(info.error());
  if (!identity_matches(*where, identity))
    return refuse(format_error_code::unsafe_path, "Ledger identity changed during verification");
  if (db->gate_status() != read_gate_status::ready || verify_gate(guard) != read_gate_status::ready)
    return refuse(format_error_code::capture_changed, "Native ledger read gate refused");
  if (access == ledger_access::read_write) {
    // Destroy the read snapshot and connection before the separately verified writable transition.
    LEDGER_TRY(tx->rollback());
    sidecars = no_sidecars(*where);
    if (!sidecars)
      return std::unexpected(sidecars.error());
    if (!identity_matches(*where, identity))
      return refuse(format_error_code::unsafe_path, "Ledger identity changed before writable open");
    auto writable = connection::open(where->path, open_mode::read_write);
    if (!writable)
      return storage();
    db         = std::move(writable);
    configured = settings(*db);
    if (!configured)
      return std::unexpected(configured.error());
    tx = db->begin(false);
    if (!tx)
      return storage();
    auto repeated = verify(*db);
    if (!repeated)
      return std::unexpected(repeated.error());
    if (repeated->identity != info->identity || repeated->workspace_id != info->workspace_id ||
        repeated->created_at != info->created_at || repeated->format != info->format || !identity_matches(*where, identity))
      return refuse(format_error_code::unsafe_path, "Ledger changed during writable transition");
  }
  return read_snapshot(std::make_shared<storage_state>(std::move(held), std::move(*where), std::move(*db), std::move(*tx),
                                                       std::move(*info), std::move(guard)));
}
std::expected<stored_record, format_error> read_snapshot::read_record(std::uint64_t id) const {
  if (!_state || !_state->readable)
    return refuse(format_error_code::invalid_argument, "Unavailable ledger owner");
  if (!check_gate(*_state)) {
    _state->readable = false;
    return refuse(format_error_code::capture_changed, "Native ledger read gate refused");
  }
  auto result = load_record(_state->db, id);
  if (!check_gate(*_state)) {
    _state->readable = false;
    return refuse(format_error_code::capture_changed, "Native ledger read gate refused");
  }
  if (!result)
    _state->readable = false;
  return result;
}
namespace {
template <class Result, class Read>
std::expected<Result, format_error> checked_read(const std::shared_ptr<storage_state>& state, Read read) {
  if (!state || !state->readable)
    return refuse(format_error_code::invalid_argument, "Unavailable ledger owner");
  if (!check_gate(*state)) {
    state->readable = false;
    return refuse(format_error_code::capture_changed, "Native ledger read gate refused");
  }
  auto result = read(state->db);
  if (!check_gate(*state)) {
    state->readable = false;
    return refuse(format_error_code::capture_changed, "Native ledger read gate refused");
  }
  if (!result)
    state->readable = false;
  return result;
}
template <class Result, class Decode>
std::expected<std::optional<Result>, format_error> lookup_row(connection& db, std::string_view sql, std::uint64_t id,
                                                              Decode decode) {
  if (!positive_id(id))
    return refuse(format_error_code::invalid_argument, "A positive signed-64 ID is required");
  auto row = db.prepare(sql);
  if (!row)
    return storage(row.error());
  auto bound = row->bind_integer(1, std::int64_t(id));
  if (!bound)
    return storage(bound.error());
  auto step = row->step();
  if (!step)
    return storage(step.error());
  if (*step == step_result::done)
    return std::nullopt;
  auto result = decode(*row);
  if (!result)
    return std::unexpected(result.error());
  return std::move(*result);
}
} // namespace
namespace {
std::expected<std::optional<request_row>, format_error> lookup_request(connection& db, std::string_view key) {
  if (!valid_request_key(key))
    return refuse(format_error_code::invalid_argument, "A bounded ASCII request token is required");
  auto row = db.prepare("SELECT "
                        "octet_length(request_id),request_id,body_id,event_seq,revision_id,octet_length(outcome),outcome,"
                        "octet_length(result),result FROM requests WHERE request_id=?1");
  if (!row)
    return storage(row.error());
  auto bound = row->bind_text(1, key);
  if (!bound)
    return storage(bound.error());
  auto step = row->step();
  if (!step)
    return storage(step.error());
  if (*step == step_result::done)
    return std::nullopt;
  auto result = decode_request_row(db, *row);
  if (!result)
    return std::unexpected(result.error());
  return std::move(*result);
}
} // namespace
std::expected<history_head, format_error> read_snapshot::read_head() const {
  return checked_read<history_head>(_state, load_head);
}
std::expected<std::optional<revision_row>, format_error> read_snapshot::read_revision(std::uint64_t id) const {
  return checked_read<std::optional<revision_row>>(_state, [&](connection& db) {
    return lookup_row<revision_row>(db,
                                    "SELECT "
                                    "id,parent_id,source_id,registry_id,stdlib_id,interpretation_id,versions_id,octet_length("
                                    "actor),actor,octet_length(accepted_at),accepted_at FROM revisions WHERE id=?1",
                                    id, decode_revision_row);
  });
}
std::expected<std::optional<event_row>, format_error> read_snapshot::read_event(std::uint64_t sequence) const {
  return checked_read<std::optional<event_row>>(_state, [&](connection& db) {
    return lookup_row<event_row>(db, std::string(event_columns) + "WHERE seq=?1", sequence, decode_event_row);
  });
}
std::expected<std::optional<request_row>, format_error> read_snapshot::read_request(std::string_view key) const {
  return checked_read<std::optional<request_row>>(_state, [&](connection& db) { return lookup_request(db, key); });
}
std::expected<event_page, format_error> read_snapshot::read_events_after(std::uint64_t cursor, std::size_t limit) const {
  return checked_read<event_page>(_state, [&](connection& db) -> std::expected<event_page, format_error> {
    if (cursor > std::uint64_t(std::numeric_limits<std::int64_t>::max()) || limit < 1 || limit > 100)
      return refuse(format_error_code::invalid_argument, "An event cursor and limit from one through 100 are required");
    auto head = load_head(db);
    if (!head)
      return std::unexpected(head.error());
    auto rows = db.prepare(std::string(event_columns) + "WHERE seq>?1 ORDER BY seq LIMIT ?2");
    if (!rows)
      return storage(rows.error());
    auto bound = rows->bind_integer(1, std::int64_t(cursor));
    if (!bound)
      return storage(bound.error());
    bound = rows->bind_integer(2, std::int64_t(limit));
    if (!bound)
      return storage(bound.error());
    event_page page;
    while (true) {
      auto step = rows->step();
      if (!step)
        return storage(step.error());
      if (*step == step_result::done)
        break;
      auto event = decode_event_row(*rows);
      if (!event)
        return std::unexpected(event.error());
      page.events.push_back(std::move(*event));
    }
    if (!page.events.empty() && page.events.back().sequence < head->last_sequence)
      page.next_after = page.events.back().sequence;
    return page;
  });
}
std::expected<void, format_error> read_snapshot::backup_to(const std::filesystem::path& destination_path) const {
  if (!_state)
    return refuse(format_error_code::invalid_argument, "Unavailable ledger owner");
  auto destination = connection::open(destination_path, open_mode::read_write);
  if (!destination)
    return storage(destination.error());
  auto copied = _state->db.online_backup_to(*destination);
  if (!copied)
    return storage(copied.error());
  return {};
}
write_transaction::write_transaction(std::shared_ptr<storage_state> state, ledger_metadata metadata)
    : _state(std::move(state)), _metadata(std::move(metadata)) {
}
std::expected<write_transaction, format_error> begin_write(const std::filesystem::path& path) {
  auto opened = read_snapshot::open_read(path, ledger_access::read_write, {});
  if (!opened)
    return std::unexpected(opened.error());
  auto state    = std::move(opened->_state);
  auto released = state->snapshot.rollback();
  if (!released)
    return storage(released.error());
  struct stat identity{};
  if (::fstat(state->file.fd, &identity) != 0 || !identity_matches(state->where, identity))
    return refuse(format_error_code::unsafe_path, "Ledger identity changed before write transaction");
  auto sidecars = no_sidecars(state->where);
  if (!sidecars)
    return std::unexpected(sidecars.error());
  auto tx = state->db.begin();
  if (!tx)
    return storage(tx.error());
  state->snapshot = std::move(*tx);
  auto checked    = verify(state->db);
  if (!checked)
    return std::unexpected(checked.error());
  sidecars = no_sidecars(state->where);
  if (!sidecars)
    return std::unexpected(sidecars.error());
  if (!identity_matches(state->where, identity) || !same_metadata(*checked, state->info))
    return refuse(format_error_code::unsafe_path, "Ledger changed before write transaction");
  auto info = state->info;
  return write_transaction(std::move(state), std::move(info));
}
namespace {
// Shared signed-64 allocator boundary, also used by the exact-source boundary probe.
std::expected<std::int64_t, format_error> next_identifier(std::int64_t maximum) {
  if (maximum == std::numeric_limits<std::int64_t>::max())
    return refuse(format_error_code::unsupported_state, "Native identifiers exhausted");
  return maximum + 1;
}
std::expected<record_reference, format_error> intern_stored(connection& db, const types::stored_value& value) {
  auto result = [&]() -> std::expected<record_reference, format_error> {
    const bool codec2  = std::holds_alternative<request_v2>(value);
    auto       encoded = std::visit(
        [](const auto& item) -> std::expected<std::string, record_error> {
          if constexpr (std::is_same_v<std::decay_t<decltype(item)>, request_v2>)
            return encode_request_v2(item);
          else
            return encode_record(history::record{item});
        },
        value);
    auto digest = std::visit(
        [](const auto& item) -> std::expected<record_digest, record_error> {
          if constexpr (std::is_same_v<std::decay_t<decltype(item)>, request_v2>)
            return digest_request_v2(item);
          else
            return digest_record(history::record{item});
        },
        value);
    if (!encoded || !digest)
      return refuse(format_error_code::invalid_argument, "Invalid record candidate");
    const auto kind = std::visit(
        [](const auto& item) {
          if constexpr (std::is_same_v<std::decay_t<decltype(item)>, request_v2>)
            return record_kind::request;
          else
            return kind_of(history::record{item});
        },
        value);
    auto format = scalar(db, "PRAGMA user_version");
    if (!format)
      return std::unexpected(format.error());
    if (codec2 && *format != 2)
      return refuse(format_error_code::unsupported_version, "Unsupported ledger format version");
    if (auto directory = std::get_if<directory_record>(&value)) {
      for (const auto& child : directory->children) {
        auto actual = load_record(db, child.id);
        if (!actual || actual->reference.kind != child.kind || actual->reference.digest != child.digest ||
            (*format == 2 && child.kind == record_kind::request))
          return refuse(format_error_code::invalid_argument, "Invalid record child reference");
      }
    }
    auto bucket =
        db.prepare("SELECT id,octet_length(encoded),encoded FROM records WHERE codec=?3 AND kind=?1 AND digest=?2 ORDER BY id");
    if (!bucket || !bucket->bind_integer(1, static_cast<std::int64_t>(kind)) || !bucket->bind_blob(2, digest_bytes(*digest)) ||
        !bucket->bind_integer(3, codec2 ? 2 : 1))
      return storage();
    std::optional<std::uint64_t> matched;
    while (true) {
      auto step = bucket->step();
      if (!step)
        return storage(step.error());
      if (*step == step_result::done)
        break;
      auto id = bucket->integer(0), size = bucket->integer(1);
      if (!id || !*id || **id < 1 || !size || !*size || **size < 0 || **size > max_record_bytes)
        return refuse(format_error_code::corrupt, "Invalid digest bucket metadata");
      auto bytes = bucket->blob(2);
      if (!bytes || *bytes != *encoded)
        return refuse(format_error_code::hash_collision, "Unequal record digest collision",
                      types::record_collision_detail{kind, kind == record_kind::request ? std::nullopt : std::optional{*digest}});
      auto existing = load_record(db, static_cast<std::uint64_t>(**id));
      if (!existing)
        return std::unexpected(existing.error());
      if (!matched)
        matched = static_cast<std::uint64_t>(**id);
    }
    if (matched)
      return record_reference{*matched, kind, *digest};
    auto maximum = scalar(db, "SELECT coalesce(max(id),0) FROM records");
    if (!maximum)
      return std::unexpected(maximum.error());
    auto allocated = next_identifier(*maximum);
    if (!allocated)
      return std::unexpected(allocated.error());
    const auto id     = *allocated;
    auto       insert = db.prepare("INSERT INTO records(id,kind,codec,digest,encoded) VALUES(?1,?2,?5,?3,?4)");
    if (!insert || !insert->bind_integer(1, id) || !insert->bind_integer(2, static_cast<std::int64_t>(kind)) ||
        !insert->bind_blob(3, digest_bytes(*digest)) || !insert->bind_blob(4, *encoded) ||
        !insert->bind_integer(5, codec2 ? 2 : 1))
      return storage();
    auto step = insert->step();
    if (!step)
      return storage(step.error());
    if (*step != step_result::done)
      return storage();
    if (auto directory = std::get_if<directory_record>(&value)) {
      auto edge = db.prepare("INSERT INTO tree_edges(parent_id,ordinal,name,child_id) VALUES(?1,?2,?3,?4)");
      if (!edge)
        return storage();
      for (std::size_t i = 0; i < directory->children.size(); ++i) {
        const auto& child = directory->children[i];
        if (!edge->bind_integer(1, id) || !edge->bind_integer(2, static_cast<std::int64_t>(i)) ||
            !edge->bind_text(3, child.name) || !edge->bind_integer(4, static_cast<std::int64_t>(child.id)))
          return storage();
        auto next = edge->step();
        if (!next)
          return storage(next.error());
        if (*next != step_result::done || !edge->reset())
          return storage();
      }
    }
    return record_reference{static_cast<std::uint64_t>(id), kind, *digest};
  }();
  return result;
}
std::expected<record_reference, format_error> intern_record(connection& db, const history::record& value) {
  return std::visit([&](const auto& item) { return intern_stored(db, types::stored_value{item}); }, value);
}
std::expected<record_reference, format_error> intern_tree_records(connection& db, std::span<const history::tree_file> files,
                                                                  history::tree_profile profile) {
  std::vector<history::tree_file_view> views;
  views.reserve(files.size());
  for (const auto& file : files)
    views.push_back({file.path, file.payload, file.mode});
  auto root = history::build_tree_core(views, profile, [&](const auto& value) { return intern_record(db, value); });
  if (!root)
    return root.error().storage ? std::unexpected(*root.error().storage)
                                : refuse(format_error_code::invalid_argument, "Invalid prepared tree");
  return *root;
}
// Every failed operation releases the shared state, rolling back through RAII and invalidating the transaction.
template <class Operation>
auto transact(std::shared_ptr<storage_state>& state, Operation&& operation) -> decltype(operation(*state)) {
  if (!state)
    return refuse(format_error_code::invalid_argument, "Unavailable record transaction");
  auto result = operation(*state);
  if (!result)
    state.reset();
  return result;
}
} // namespace
const ledger_metadata& write_transaction::metadata() const {
  return _metadata;
}
std::expected<std::optional<request_row>, format_error> write_transaction::read_request(std::string_view key) {
  return transact(_state, [&](storage_state& state) { return lookup_request(state.db, key); });
}
std::expected<history_head, format_error> write_transaction::read_head() {
  return transact(_state, [&](storage_state& state) { return load_head(state.db); });
}
std::expected<std::optional<revision_row>, format_error> write_transaction::read_revision(std::uint64_t id) {
  return transact(_state, [&](storage_state& state) {
    return lookup_row<revision_row>(state.db,
                                    "SELECT "
                                    "id,parent_id,source_id,registry_id,stdlib_id,interpretation_id,versions_id,octet_length("
                                    "actor),actor,octet_length(accepted_at),accepted_at FROM revisions WHERE id=?1",
                                    id, decode_revision_row);
  });
}
std::expected<stored_record, format_error> write_transaction::read_record(std::uint64_t id) {
  return transact(_state, [&](storage_state& state) { return load_record(state.db, id); });
}
std::expected<record_reference, format_error> write_transaction::intern_value(const history::record& value) {
  return transact(_state, [&](storage_state& state) { return intern_record(state.db, value); });
}
std::expected<record_reference, format_error> write_transaction::intern_request(const types::stored_value& value) {
  return transact(_state, [&](storage_state& state) { return intern_stored(state.db, value); });
}
std::expected<record_reference, format_error> write_transaction::intern_tree(std::span<const history::tree_file> files,
                                                                             history::tree_profile               profile) {
  return transact(_state, [&](storage_state& state) { return intern_tree_records(state.db, files, profile); });
}
std::expected<bool, format_error> write_transaction::identical_content(const event_references& left,
                                                                       const event_references& right) {
  return transact(_state, [&](storage_state& state) { return identical_content_rows(state.db, left, right); });
}
std::expected<acceptance_rows_output, format_error>
write_transaction::append_acceptance_rows(const acceptance_rows_input& input) {
  return transact(_state, [&](storage_state& state) -> std::expected<acceptance_rows_output, format_error> {
    auto&       db       = state.db;
    const auto& head     = input.head;
    const auto& refs     = input.references;
    const bool  noop     = input.noop;
    auto        sequence = next_identifier(head.last_sequence);
    if (!sequence)
      return std::unexpected(sequence.error());
    std::int64_t revision = input.previous ? input.previous->id : 0;
    auto         parent   = input.previous ? input.previous->parent : std::optional<std::int64_t>{};
    const auto   time     = timestamp();
    if (!noop) {
      auto maximum = scalar(db, "SELECT coalesce(max(id),0) FROM revisions");
      if (!maximum)
        return std::unexpected(maximum.error());
      auto allocated = next_identifier(*maximum);
      if (!allocated)
        return std::unexpected(allocated.error());
      revision = *allocated;
      parent   = head.revision;
      auto row = db.prepare("INSERT INTO revisions VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9)");
      if (!row || !row->bind_integer(1, revision) || !(parent ? row->bind_integer(2, *parent) : row->bind_null(2)) ||
          !row->bind_integer(3, refs.source) || !row->bind_integer(4, refs.registry) || !row->bind_integer(5, refs.stdlib) ||
          !row->bind_integer(6, refs.interpretation) || !row->bind_integer(7, refs.versions) || !row->bind_text(8, input.actor) ||
          !row->bind_text(9, time))
        return storage();
      auto step = row->step();
      if (!step)
        return storage(step.error());
      if (*step != step_result::done)
        return storage();
    }
    const auto outcome = noop ? load_outcome::noop : load_outcome::accepted;
    auto       payload = encode_history_event(
        {head.revision, revision, noop ? event_reason::identical_declaration : event_reason::declaration_accepted, refs});
    auto result = encode_load_result({state.info.identity, std::string(input.request_id), outcome, revision, parent, *sequence});
    if (!payload || !result)
      return refuse(format_error_code::invalid_argument, "Cannot encode acceptance result");
    {
      auto row = db.prepare("INSERT INTO events VALUES(?1,?2,1,?3,?4,?5,?6,?7)");
      if (!row || !row->bind_integer(1, *sequence) || !row->bind_text(2, noop ? "load_noop" : "declaration_accepted") ||
          !row->bind_integer(3, revision) || !row->bind_text(4, input.request_id) || !row->bind_text(5, input.actor) ||
          !row->bind_text(6, time) || !row->bind_blob(7, *payload))
        return storage();
      auto step = row->step();
      if (!step)
        return storage(step.error());
      if (*step != step_result::done)
        return storage();
    }
    {
      auto row = db.prepare("INSERT INTO requests VALUES(?1,?2,?3,?4,?5,?6)");
      if (!row || !row->bind_text(1, input.request_id) || !row->bind_integer(2, std::int64_t(input.body.id)) ||
          !row->bind_integer(3, *sequence) || !row->bind_integer(4, revision) || !row->bind_text(5, noop ? "noop" : "accepted") ||
          !row->bind_blob(6, *result))
        return storage();
      auto step = row->step();
      if (!step)
        return storage(step.error());
      if (*step != step_result::done)
        return storage();
    }
    {
      auto row = db.prepare("UPDATE current_state SET active_revision_id=?1,last_seq=?2 WHERE singleton=1");
      if (!row || !row->bind_integer(1, revision) || !row->bind_integer(2, *sequence))
        return storage();
      auto step = row->step();
      if (!step)
        return storage(step.error());
      if (*step != step_result::done)
        return storage();
    }
    return acceptance_rows_output{std::move(*result), revision, parent, *sequence};
  });
}
std::expected<void, format_error> write_transaction::verify_before_commit(commit_site site) {
  return transact(_state, [&](storage_state& state) -> std::expected<void, format_error> {
    if (site != commit_site::record_writer && site != commit_site::acceptance)
      return refuse(format_error_code::invalid_argument, "Invalid commit site");
    const bool  acceptance = site == commit_site::acceptance;
    struct stat identity{};
    if (::fstat(state.file.fd, &identity) != 0 || !identity_matches(state.where, identity))
      return refuse(format_error_code::unsafe_path,
                    acceptance ? "Ledger identity changed before acceptance commit" : "Ledger identity changed before commit");
    auto checked = verify(state.db);
    if (!checked)
      return std::unexpected(checked.error());
    if (!same_metadata(*checked, state.info))
      return refuse(format_error_code::unsafe_path,
                    acceptance ? "Ledger metadata changed before acceptance commit" : "Ledger metadata changed before commit");
    return {};
  });
}
std::expected<void, format_error> write_transaction::commit([[maybe_unused]] commit_site site) {
  if (!_state)
    return refuse(format_error_code::invalid_argument, "Unavailable record transaction");
  auto state     = std::move(_state);
  auto committed = state->snapshot.commit();
  if (!committed)
    return storage(committed.error());
  return {};
}
std::expected<void, format_error> write_transaction::commit([[maybe_unused]] commit_site site, bool& confirmed_noncommit) {
  if (!_state)
    return refuse(format_error_code::invalid_argument, "Unavailable record transaction");
  auto state     = std::move(_state);
  auto committed = state->snapshot.commit(confirmed_noncommit);
  if (!committed)
    return storage(committed.error());
  return {};
}
void write_transaction::abort() noexcept {
  _state.reset();
}

#undef LEDGER_TRY
} // namespace planar::engine::ledger
