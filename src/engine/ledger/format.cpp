module;
#include <cerrno>
#include <fcntl.h>
#include <sqlite3.h>
#include <sys/random.h>
#include <sys/stat.h>
#include <unistd.h>
module planar.engine.ledger.format;
import std;
import planar.engine.ledger.sqlite;
import planar.engine.json_codec;
import planar.engine.ledger.history_codec;
import planar.engine.ledger.storage;
namespace planar::engine::ledger {
namespace {
using namespace native;
constexpr std::string_view init_actor = "planar:init";
using failure                         = std::unexpected<format_error>;
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
bool valid_workspace(std::string_view id) {
  if (id.empty() || id.size() > 64 || id.front() < 'a' || id.front() > 'z')
    return false;
  return std::ranges::all_of(id, [](char c) { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'; });
}
std::expected<std::string, format_error> random_hex() {
  std::array<unsigned char, 16> bytes{};
  if (::getentropy(bytes.data(), bytes.size()) != 0)
    return storage();
  std::string                result;
  constexpr std::string_view digits = "0123456789abcdef";
  for (auto byte : bytes) {
    result += digits[byte >> 4];
    result += digits[byte & 15];
  }
  return result;
}
value_ptr string_value(std::string_view text) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = text;
  return v;
}
value_ptr record_value(std::map<std::string, value_ptr> fields) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::record;
  v->fields = std::move(fields);
  return v;
}
std::expected<std::string, format_error> initial_payload() {
  auto version     = std::make_shared<value>();
  version->kind    = value_kind::integer;
  version->integer = 1;
  auto state       = record_value({{"active_revision", string_value("none")}});
  auto encoded     = codec_encode(record_value({{"version", version},
                                                {"before", state},
                                                {"after", state},
                                                {"reason", string_value("initialized")},
                                                {"references", record_value({})}}));
  if (!encoded)
    return storage();
  return *encoded;
}
} // namespace
format_file::format_file(read_snapshot snapshot) : _snapshot(std::move(snapshot)) {
}
const ledger_metadata& format_file::metadata() const {
  return _snapshot.metadata();
}
read_gate_status format_file::gate_status() const noexcept {
  return _snapshot.gate_status();
}
std::expected<format_file, format_error> open_existing(const std::filesystem::path& path, ledger_access access) {
  auto snapshot = read_snapshot::open_read(path, access, {});
  if (!snapshot)
    return std::unexpected(snapshot.error());
  return format_file(std::move(*snapshot));
}
std::expected<format_file, format_error> open_existing_gated(const std::filesystem::path&       path,
                                                             std::shared_ptr<format_read_guard> guard) {
  if (!guard)
    return refuse(format_error_code::invalid_argument, "A native read guard is required");
  auto snapshot = read_snapshot::open_read(path, ledger_access::read_only, std::move(guard));
  if (!snapshot)
    return std::unexpected(snapshot.error());
  return format_file(std::move(*snapshot));
}
std::expected<stored_record, format_error> format_file::read_record(std::uint64_t id) const {
  return _snapshot.read_record(id);
}
std::expected<history_head, format_error> format_file::read_head() const {
  return _snapshot.read_head();
}
std::expected<std::optional<revision_row>, format_error> format_file::read_revision(std::uint64_t id) const {
  return _snapshot.read_revision(id);
}
std::expected<std::optional<event_row>, format_error> format_file::read_event(std::uint64_t sequence) const {
  return _snapshot.read_event(sequence);
}
std::expected<std::optional<request_row>, format_error> format_file::read_request(std::string_view key) const {
  return _snapshot.read_request(key);
}
std::expected<event_page, format_error> format_file::read_events_after(std::uint64_t cursor, std::size_t limit) const {
  return _snapshot.read_events_after(cursor, limit);
}
std::expected<std::optional<execution_request_row>, format_error>
format_file::read_execution_request(std::string_view key) const {
  return _snapshot.read_execution_request(key);
}
std::expected<execution_state_at, format_error> format_file::read_execution_state_at(std::uint64_t sequence,
                                                                                   std::string_view job) const {
  return _snapshot.read_execution_state_at(sequence, job);
}
record_writer::record_writer(write_transaction transaction) : _transaction(std::move(transaction)) {
}
std::expected<record_writer, format_error> begin_records(const std::filesystem::path& path) {
  auto transaction = begin_write(path);
  if (!transaction)
    return std::unexpected(transaction.error());
  return record_writer(std::move(*transaction));
}
std::expected<stored_record, format_error> record_writer::read_record(std::uint64_t id) {
  if (!_transaction)
    return refuse(format_error_code::invalid_argument, "Unavailable record transaction");
  return _transaction->read_record(id);
}
std::expected<record_reference, format_error> record_writer::intern(const history::record& value) {
  if (!_transaction)
    return refuse(format_error_code::invalid_argument, "Unavailable record transaction");
  return _transaction->intern_value(value);
}
std::expected<record_reference, format_error> record_writer::intern_request_v2(const history::request_v2& value) {
  if (!_transaction)
    return refuse(format_error_code::invalid_argument, "Unavailable record transaction");
  return _transaction->intern_request(types::stored_value{value});
}
void record_writer::abort() noexcept {
  _transaction.reset();
}
std::expected<void, format_error> record_writer::commit() {
  if (!_transaction)
    return refuse(format_error_code::invalid_argument, "Unavailable record transaction");
  // The writer is unavailable after either outcome.
  auto transaction = std::move(*_transaction);
  _transaction.reset();
  auto verified = transaction.verify_before_commit(commit_site::record_writer);
  if (!verified)
    return verified;
  return transaction.commit(commit_site::record_writer);
}
namespace {
struct staging {
  location&                          where;
  std::string                        name;
  descriptor                         directory;
  struct stat                        identity{};
  std::optional<struct stat>         database_identity;
  std::map<std::string, struct stat> captures;
  // The exclusive 0700 directory confines all entries to this initialization under cooperative ownership.
  bool cleanup() {
    struct stat named{}, held{};
    if (::fstat(directory.fd, &held) != 0 || ::fstatat(where.parent.fd, name.c_str(), &named, AT_SYMLINK_NOFOLLOW) != 0 ||
        !same(named, identity) || !same(held, identity) || !S_ISDIR(named.st_mode) || named.st_uid != ::geteuid())
      return false;
    for (auto file : {"database-journal", "database", "captured-main", "captured-journal"}) {
      struct stat entry{};
      if (::fstatat(directory.fd, file, &entry, AT_SYMLINK_NOFOLLOW) != 0) {
        if (errno == ENOENT)
          continue;
        return false;
      }
      if (!S_ISREG(entry.st_mode) || entry.st_uid != ::geteuid() ||
          (std::string_view(file) == "database" && (!database_identity || !same(entry, *database_identity))) ||
          (std::string_view(file).starts_with("captured-") && (!captures.contains(file) || !same(entry, captures.at(file)))) ||
          ::unlinkat(directory.fd, file, 0) != 0)
        return false;
    }
    return ::unlinkat(where.parent.fd, name.c_str(), AT_REMOVEDIR) == 0;
  }
};
std::expected<void, format_error> create_database(const std::filesystem::path& path, const ledger_metadata& info) {
  auto db = connection::open(path, open_mode::scratch_create);
  if (!db)
    return storage();
  auto s = settings(*db);
  if (!s)
    return s;
  // Persistent settings are confined to this exclusively owned new staging database.
  auto journal = db->prepare("PRAGMA journal_mode=DELETE");
  if (!journal || journal->step() != step_result::row || !journal->text(0) ||
      *journal->text(0) != std::optional<std::string>("delete") || journal->step() != step_result::done)
    return storage();
  LEDGER_TRY(execute(*db, "PRAGMA encoding='UTF-8'"));
  auto tx = db->begin();
  if (!tx)
    return storage();
  LEDGER_TRY(execute(*db, "PRAGMA application_id=1347178034"));
  LEDGER_TRY(execute(*db, std::format("PRAGMA user_version={}", static_cast<int>(info.format))));
  for (const auto& sql : selected_schema(info.format))
    LEDGER_TRY(execute(*db, sql));
  auto meta = db->prepare(std::format("INSERT INTO ledger_meta VALUES(1,{},?1,?2,?3)", static_cast<int>(info.format)));
  if (!meta || !meta->bind_text(1, info.identity) || !meta->bind_text(2, info.workspace_id) ||
      !meta->bind_text(3, info.created_at) || meta->step() != step_result::done)
    return storage();
  auto payload = initial_payload();
  if (!payload)
    return std::unexpected(payload.error());
  auto event = db->prepare("INSERT INTO events VALUES(1,'ledger_initialized',1,NULL,NULL,?1,?2,?3)");
  if (!event || !event->bind_text(1, init_actor) || !event->bind_text(2, info.created_at) || !event->bind_blob(3, *payload) ||
      event->step() != step_result::done)
    return storage();
  LEDGER_TRY(execute(*db, "INSERT INTO current_state VALUES(1,NULL,1)"));
  LEDGER_TRY(tx->commit());
  return {};
}
} // namespace
namespace {
// Shared safe publication for initialization and SQLite-created snapshots. The creator closes its connection before return.
std::expected<void, format_error>
publish_new(location& where, const std::function<std::expected<void, format_error>(const std::filesystem::path&)>& create,
            const ledger_metadata& expected) {
  struct stat target{};
  if (::fstatat(where.parent.fd, where.name.c_str(), &target, AT_SYMLINK_NOFOLLOW) == 0)
    return refuse(format_error_code::exists, "Ledger target already exists");
  if (errno != ENOENT)
    return storage();
  auto sidecars = no_sidecars(where);
  if (!sidecars)
    return std::unexpected(sidecars.error());
  auto nonce = random_hex();
  if (!nonce)
    return std::unexpected(nonce.error());
  staging stage{where, ".planar-init-" + *nonce, descriptor(), {}, std::nullopt, {}};
  if (::mkdirat(where.parent.fd, stage.name.c_str(), 0700) != 0)
    return storage();
  stage.directory = descriptor(::openat(where.parent.fd, stage.name.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
  if (stage.directory.fd < 0 || ::fstat(stage.directory.fd, &stage.identity) != 0)
    return refuse(format_error_code::io, "Cannot retain owned staging directory; inspect explicit parent");
  bool published = false;
  auto operation = [&]() -> std::expected<void, format_error> {
    struct stat named_stage{};
    if (::fstatat(where.parent.fd, stage.name.c_str(), &named_stage, AT_SYMLINK_NOFOLLOW) != 0 ||
        !same(named_stage, stage.identity))
      return refuse(format_error_code::unsafe_path, "Owned staging identity changed");
    descriptor  seed(::openat(stage.directory.fd, "database", O_RDWR | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600));
    struct stat seeded{};
    if (seed.fd < 0 || ::fstat(seed.fd, &seeded) != 0)
      return storage();
    stage.database_identity = seeded;
    auto staging_path       = where.path.parent_path() / stage.name / "database";
    auto created            = create(staging_path);
    if (!created)
      return created;
    descriptor  file(::openat(stage.directory.fd, "database", O_RDONLY | O_NOFOLLOW | O_CLOEXEC));
    struct stat file_identity{};
    if (file.fd < 0 || ::fstat(file.fd, &file_identity) != 0 || !S_ISREG(file_identity.st_mode) ||
        !same(file_identity, *stage.database_identity) || ::fsync(file.fd) != 0 || ::fsync(stage.directory.fd) != 0)
      return storage();
    {
      auto check = open_existing(staging_path);
      if (!check)
        return std::unexpected(check.error());
      const auto& actual = check->metadata();
      if (actual.identity != expected.identity || actual.workspace_id != expected.workspace_id ||
          actual.created_at != expected.created_at || actual.format != expected.format)
        return refuse(format_error_code::corrupt, "Staged ledger metadata differs from source");
    }
    struct stat named{}, parent{}, current_parent{};
    descriptor  checked_parent(::open(where.path.parent_path().c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
    if (checked_parent.fd < 0 || ::fstat(checked_parent.fd, &current_parent) != 0 || ::fstat(where.parent.fd, &parent) != 0 ||
        !same(current_parent, parent) || ::fstatat(stage.directory.fd, "database", &named, AT_SYMLINK_NOFOLLOW) != 0 ||
        !same(named, file_identity))
      return refuse(format_error_code::unsafe_path, "Staging or parent identity changed");
    auto sidecars = no_sidecars(where);
    if (!sidecars)
      return std::unexpected(sidecars.error());
    // POSIX linkat publishes a regular file atomically, refusing every existing destination. Never overwrite-rename.
    if (::linkat(stage.directory.fd, "database", where.parent.fd, where.name.c_str(), 0) != 0)
      return errno == EEXIST ? refuse(format_error_code::exists, "Ledger target already exists") : storage();
    published = true;
    if (::fsync(where.parent.fd) != 0)
      return refuse(format_error_code::publication_uncertain, "Ledger may be published; inspect explicit target");
    return {};
  }();
  const bool cleaned = stage.cleanup();
  if (published && (!operation || !cleaned || ::fsync(where.parent.fd) != 0))
    return refuse(format_error_code::publication_uncertain,
                  "Ledger may be published; inspect target and owned staging in explicit parent");
  if (!cleaned)
    return refuse(format_error_code::io, "Owned staging cleanup incomplete; inspect explicit parent");
  if (!operation)
    return std::unexpected(operation.error());
  return {};
}
} // namespace
namespace {
std::expected<ledger_metadata, format_error> initialize_selected(const std::filesystem::path& path, std::string_view workspace_id,
                                                                 ledger_format format) {
  if (!valid_workspace(workspace_id))
    return refuse(format_error_code::invalid_argument, "Invalid declaration workspace identifier");
  auto where = locate(path);
  if (!where)
    return std::unexpected(where.error());
  auto id = random_hex();
  if (!id)
    return std::unexpected(id.error());
  ledger_metadata info{*id, std::string(workspace_id), timestamp(), format};
  auto result = publish_new(*where, [&](const auto& staging_path) { return create_database(staging_path, info); }, info);
  if (!result)
    return std::unexpected(result.error());
  return info;
}
} // namespace
std::expected<ledger_metadata, format_error> initialize_new(const std::filesystem::path& path, std::string_view workspace_id) {
  return initialize_selected(path, workspace_id, ledger_format::v1);
}
std::expected<ledger_metadata, format_error> initialize_new_v2(const std::filesystem::path& path, std::string_view workspace_id) {
  return initialize_selected(path, workspace_id, ledger_format::v2);
}
std::expected<ledger_metadata, format_error> backup_new(const std::filesystem::path& source,
                                                        const std::filesystem::path& target) {
  auto original = open_existing(source);
  if (!original)
    return std::unexpected(original.error());
  auto where = locate(target);
  if (!where)
    return std::unexpected(where.error());
  auto result = publish_new(
      *where,
      [&](const auto& staging_path) -> std::expected<void, format_error> { return original->_snapshot.backup_to(staging_path); },
      original->metadata());
  if (!result)
    return std::unexpected(result.error());
  return original->metadata();
}
std::expected<ledger_metadata, format_error> restore_new(const std::filesystem::path& source,
                                                         const std::filesystem::path& target) {
  return backup_new(source, target);
}

namespace {
std::expected<void, format_error> without_wal(const location& where) {
  for (auto suffix : {"-wal", "-shm"}) {
    struct stat st{};
    if (::fstatat(where.parent.fd, (where.name + suffix).c_str(), &st, AT_SYMLINK_NOFOLLOW) == 0)
      return refuse(format_error_code::recovery_required, "WAL recovery is unsupported");
    if (errno != ENOENT)
      return storage();
  }
  return {};
}
// Fixed-size transfer and comparison buffers bound native copying independently of the database size.
std::expected<void, format_error> copy_exact(int source, int target) {
  struct stat before{}, after{};
  if (::fstat(source, &before) != 0 || !S_ISREG(before.st_mode))
    return storage();
  std::array<char, 65536> buffer{};
  for (off_t offset = 0; offset < before.st_size;) {
    auto count = static_cast<std::size_t>(std::min<off_t>(buffer.size(), before.st_size - offset));
    auto read  = ::pread(source, buffer.data(), count, offset);
    if (read < 0 && errno == EINTR)
      continue;
    if (read <= 0)
      return storage();
    for (ssize_t written = 0; written < read;) {
      auto n = ::pwrite(target, buffer.data() + written, read - written, offset + written);
      if (n < 0 && errno == EINTR)
        continue;
      if (n <= 0)
        return refuse(errno == ENOSPC ? format_error_code::storage_full : format_error_code::io, "Cannot copy recovery input");
      written += n;
    }
    offset += read;
  }
  if (::fstat(source, &after) != 0 || !same(before, after) || before.st_size != after.st_size || ::fsync(target) != 0)
    return storage();
  return {};
}
std::expected<bool, format_error> equal_bytes(int left, int right) {
  struct stat a{}, b{};
  if (::fstat(left, &a) != 0 || ::fstat(right, &b) != 0)
    return storage();
  if (a.st_size != b.st_size)
    return false;
  std::array<char, 65536> x{}, y{};
  for (off_t offset = 0; offset < a.st_size;) {
    auto       count = static_cast<std::size_t>(std::min<off_t>(x.size(), a.st_size - offset));
    const auto n     = ::pread(left, x.data(), count, offset);
    if (n < 0 && errno == EINTR)
      continue;
    const auto m = ::pread(right, y.data(), count, offset);
    if (m < 0 && errno == EINTR)
      continue;
    if (n != static_cast<ssize_t>(count) || m != n)
      return storage();
    if (std::memcmp(x.data(), y.data(), count) != 0)
      return false;
    offset += n;
  }
  return true;
}
struct journal_dimensions {
  std::uint32_t page_size;
  std::uint64_t original_size;
};
std::expected<journal_dimensions, format_error> hot_header(int fd, int main) {
  struct stat                            st{};
  std::array<unsigned char, 28>          bytes{};
  constexpr std::array<unsigned char, 8> magic{0xd9, 0xd5, 0x05, 0xf9, 0x20, 0xa1, 0x63, 0xd7};
  if (::fstat(fd, &st) != 0 || !S_ISREG(st.st_mode))
    return refuse(format_error_code::unsafe_path, "Recovery journal must be a regular file");
  if (st.st_size <= 512 || ::pread(fd, bytes.data(), bytes.size(), 0) != static_cast<ssize_t>(bytes.size()) ||
      !std::equal(magic.begin(), magic.end(), bytes.begin()))
    return refuse(format_error_code::corrupt, "Recovery requires a valid hot journal header");
  auto u32 = [&](std::size_t i) {
    return (std::uint32_t(bytes[i]) << 24) | (std::uint32_t(bytes[i + 1]) << 16) | (std::uint32_t(bytes[i + 2]) << 8) |
           bytes[i + 3];
  };
  auto       power   = [](std::uint32_t n) { return n >= 512 && n <= 65536 && (n & (n - 1)) == 0; };
  const auto records = u32(8), pages = u32(16), sector = u32(20), page = u32(24);
  if (!power(sector) || !power(page) || pages == 0 || records == 0 || records == 0xffffffff ||
      std::uint64_t(sector) + std::uint64_t(records) * (page + 8) > std::uint64_t(st.st_size))
    return refuse(format_error_code::corrupt, "Malformed or unsupported recovery journal dimensions");
  std::array<unsigned char, 2> main_page{};
  if (::pread(main, main_page.data(), main_page.size(), 16) != static_cast<ssize_t>(main_page.size()))
    return storage();
  auto main_size = std::uint32_t(main_page[0]) * 256 + main_page[1];
  if (main_size == 1)
    main_size = 65536;
  if (main_size != page)
    return refuse(format_error_code::corrupt, "Journal page size does not match main file");
  std::array<unsigned char, 8> tail{};
  if (::pread(fd, tail.data(), tail.size(), st.st_size - static_cast<off_t>(tail.size())) != static_cast<ssize_t>(tail.size()))
    return storage();
  if (tail == magic)
    return refuse(format_error_code::corrupt, "Recovery does not support a super-journal footer");
  return journal_dimensions{page, std::uint64_t(pages) * page};
}
// This writable opener is private to explicitly authorized scratch/original recovery, never a fallback from ordinary open.
std::expected<ledger_metadata, format_error> recover_and_verify(const std::filesystem::path& path) {
  auto db = connection::open(path, open_mode::read_write);
  if (!db)
    return storage(db.error());
  auto configured = settings(*db);
  if (!configured)
    return std::unexpected(configured.error());
  auto tx = db->begin(false);
  if (!tx)
    return storage(tx.error());
  return verify(*db);
}
} // namespace
std::expected<ledger_metadata, format_error> recover_quiescent(const std::filesystem::path& path) {
  auto where = locate(path);
  if (!where)
    return std::unexpected(where.error());
  auto journal_where = locate(std::filesystem::path(where->path.string() + "-journal"));
  if (!journal_where)
    return std::unexpected(journal_where.error());
  descriptor main(::openat(where->parent.fd, where->name.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK));
  descriptor journal(
      ::openat(where->parent.fd, (where->name + "-journal").c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK));
  struct stat main_identity{}, journal_identity{};
  if (main.fd < 0 || journal.fd < 0 || ::fstat(main.fd, &main_identity) != 0 || ::fstat(journal.fd, &journal_identity) != 0 ||
      !S_ISREG(main_identity.st_mode) || !S_ISREG(journal_identity.st_mode))
    return refuse(format_error_code::unsafe_path, "Recovery requires regular main and journal files");
  auto checked = header(main.fd);
  if (!checked)
    return std::unexpected(checked.error());
  checked = without_wal(*where);
  if (!checked)
    return std::unexpected(checked.error());
  auto dimensions = hot_header(journal.fd, main.fd);
  if (!dimensions)
    return std::unexpected(dimensions.error());
  if (!identity_matches(*where, main_identity) || !identity_matches(*journal_where, journal_identity))
    return refuse(format_error_code::unsafe_path, "Recovery input identities changed");
  auto nonce = random_hex();
  if (!nonce)
    return std::unexpected(nonce.error());
  staging stage{*where, ".planar-recover-" + *nonce, descriptor(), {}, std::nullopt, {}};
  if (::mkdirat(where->parent.fd, stage.name.c_str(), 0700) != 0)
    return storage();
  stage.directory = descriptor(::openat(where->parent.fd, stage.name.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
  if (stage.directory.fd < 0 || ::fstat(stage.directory.fd, &stage.identity) != 0)
    return refuse(format_error_code::io, "Cannot retain recovery scratch; inspect explicit parent");
  auto operation = [&]() -> std::expected<ledger_metadata, format_error> {
    auto capture = [&](const char* name, int source) -> std::expected<descriptor, format_error> {
      descriptor  file(::openat(stage.directory.fd, name, O_RDWR | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600));
      struct stat identity{};
      if (file.fd < 0 || ::fstat(file.fd, &identity) != 0)
        return storage();
      if (std::string_view(name) == "database")
        stage.database_identity = identity;
      else if (std::string_view(name).starts_with("captured-"))
        stage.captures.emplace(name, identity);
      auto copied = copy_exact(source, file.fd);
      if (!copied)
        return std::unexpected(copied.error());
      return file;
    };
    auto captured_main = capture("captured-main", main.fd);
    if (!captured_main)
      return std::unexpected(captured_main.error());
    auto captured_journal = capture("captured-journal", journal.fd);
    if (!captured_journal)
      return std::unexpected(captured_journal.error());
    auto working = capture("database", captured_main->fd);
    if (!working)
      return std::unexpected(working.error());
    auto working_journal = capture("database-journal", captured_journal->fd);
    if (!working_journal)
      return std::unexpected(working_journal.error());
    const auto scratch_path  = where->path.parent_path() / stage.name / "database";
    auto       scratch_where = locate(scratch_path);
    if (!scratch_where)
      return std::unexpected(scratch_where.error());
    struct stat named_stage{};
    if (::fstatat(where->parent.fd, stage.name.c_str(), &named_stage, AT_SYMLINK_NOFOLLOW) != 0 ||
        !same(named_stage, stage.identity) || !identity_matches(*scratch_where, *stage.database_identity))
      return refuse(format_error_code::unsafe_path, "Recovery scratch identity changed");
    auto recovered = recover_and_verify(scratch_path);
    if (!recovered)
      return std::unexpected(recovered.error());
    struct stat recovered_identity{};
    if (::fstat(working->fd, &recovered_identity) != 0 || std::uint64_t(recovered_identity.st_size) != dimensions->original_size)
      return refuse(format_error_code::corrupt, "Recovered size differs from hot journal");
    checked = no_sidecars(*scratch_where);
    if (!checked)
      return std::unexpected(checked.error());
    auto exact_main    = equal_bytes(main.fd, captured_main->fd);
    auto exact_journal = equal_bytes(journal.fd, captured_journal->fd);
    if (!exact_main || !exact_journal)
      return storage();
    if (!*exact_main || !*exact_journal || !identity_matches(*where, main_identity) ||
        !identity_matches(*journal_where, journal_identity))
      return refuse(format_error_code::unsafe_path, "Recovery input changed after scratch validation");
    checked = without_wal(*where);
    if (!checked)
      return std::unexpected(checked.error());
    // From this point SQLite alone owns recovery of the original. The caller's whole-operation quiescence is essential.
    auto original = recover_and_verify(where->path);
    if (!original)
      return std::unexpected(original.error());
    if (!same_metadata(*original, *recovered) || !identity_matches(*where, main_identity))
      return refuse(format_error_code::corrupt, "Original recovery differs from verified scratch");
    checked = no_sidecars(*where);
    if (!checked)
      return std::unexpected(checked.error());
    if (::fsync(main.fd) != 0 || ::fsync(where->parent.fd) != 0)
      return storage();
    return original;
  }();
  if (!stage.cleanup() || ::fsync(where->parent.fd) != 0)
    return refuse(format_error_code::io, "Recovery scratch cleanup incomplete; inspect explicit parent");
  return operation;
}

#undef LEDGER_TRY
} // namespace planar::engine::ledger
