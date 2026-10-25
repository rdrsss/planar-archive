/// @file format.cppm
/// @brief Versioned native initialization and opaque verified ledger owners with typed format and filesystem refusals.
export module planar.engine.ledger.format;
import std;
export import planar.engine.ledger.types;
export import planar.engine.ledger.sqlite;
export import planar.engine.ledger.completion;
export import planar.engine.ledger.history_codec;
export import planar.engine.ledger.storage_types;
import planar.engine.ledger.storage;
export namespace planar::engine::ledger {
/// @brief Trusted read guard alias; bind the actual no-follow descriptor before any ledger header access.
using format_read_guard = storage_read_guard;
class record_writer;
/// @brief Opaque thread-confined owner of a verified ledger and pinned read snapshot.
/// @note No SQL or SQLite handle escapes. Cooperative directory ownership is required; hostile swapping is unsupported.
class format_file {
public:
  /// @brief Transfer the opaque owner.
  format_file(format_file&&) noexcept = default;
  /// @brief Transfer the opaque owner, releasing the previous snapshot.
  /// @return This owner.
  format_file& operator=(format_file&&) noexcept = default;
  format_file(const format_file&)                = delete;
  format_file& operator=(const format_file&)     = delete;
  /// @brief Get cached immutable metadata verified at open, including after a failed read.
  /// @return Initialization metadata; caller must not use a moved-from owner.
  const ledger_metadata& metadata() const;
  /// @brief Inspect sticky SQLite gate status without allocation.
  /// @return First gate refusal, or ready for ungated, healthy or moved owners. Cached metadata is not authorization.
  read_gate_status gate_status() const noexcept;
  /// @brief Copy one bounded typed record from this owner's verified snapshot.
  /// @param id Positive signed-64 native ID.
  /// @return Validated record or the original redacted typed failure. Every failure permanently invalidates further reads.
  /// @note Invalid IDs and missing records also invalidate this owner. Moves preserve validity; only an explicit reopen
  /// obtains another verified snapshot. Cached metadata remains accessible. Retained resources release through RAII.
  std::expected<stored_record, format_error> read_record(std::uint64_t id) const;

  /// @brief Read the current head and last event from this owner's pinned snapshot.
  /// @return Owned projection or a sticky typed failure; cached metadata survives failure.
  std::expected<history_head, format_error> read_head() const;
  /// @brief Read one immutable revision from this snapshot.
  /// @param id Positive signed-64 native ID.
  /// @return Owned row or a successful optional miss. Invalid arguments and actual failures invalidate all reads.
  std::expected<std::optional<revision_row>, format_error> read_revision(std::uint64_t id) const;
  /// @brief Read one exact saved event from this snapshot.
  /// @param sequence Positive signed-64 sequence.
  /// @return Owned row or successful optional miss; failures invalidate all reads, including read_record.
  std::expected<std::optional<event_row>, format_error> read_event(std::uint64_t sequence) const;
  /// @brief Read private replay data, including exact saved request and result bytes.
  /// @param key ASCII request token of one through 128 bytes.
  /// @return Owned row or successful optional miss; failures invalidate all reads.
  std::expected<std::optional<request_row>, format_error> read_request(std::string_view key) const;
  /// @brief Read an ascending exclusive event page without caching history.
  /// @param cursor Zero starts before event one; otherwise at most signed-64 maximum.
  /// @param limit Explicit count from one through 100.
  /// @return Owned bounded page; invalid arguments and actual failures invalidate all reads.
  std::expected<event_page, format_error> read_events_after(std::uint64_t cursor, std::size_t limit) const;

private:
  friend std::expected<format_file, format_error>     open_existing(const std::filesystem::path&, ledger_access);
  friend std::expected<ledger_metadata, format_error> backup_new(const std::filesystem::path&, const std::filesystem::path&);
  friend std::expected<format_file, format_error>     open_existing_gated(const std::filesystem::path&,
                                                                          std::shared_ptr<format_read_guard>);
  explicit format_file(read_snapshot snapshot);
  read_snapshot _snapshot;
};
/// @brief Independently owned, move-only thread-confined record write transaction.
/// @note No raw handles escape. Every failed operation or abandonment rolls back the entire transaction and invalidates it.
class record_writer {
public:
  /// Transfer the transaction; the source becomes unavailable.
  record_writer(record_writer&&) noexcept = default;
  /// Release the previous transaction and transfer ownership.
  /// @return This owner.
  record_writer& operator=(record_writer&&) noexcept = default;
  record_writer(const record_writer&)                = delete;
  record_writer& operator=(const record_writer&)     = delete;
  /// @brief Intern one exact record, checking every digest-bucket candidate and directory reference.
  /// @param value Locally valid typed record; children must already exist in this transaction.
  /// @return Reused or newly allocated reference; no implicit commit occurs.
  std::expected<record_reference, format_error> intern(const history::record& value);
  /// @brief Intern one independently framed codec2 request in a format2 transaction.
  /// @param value Locally valid exact request selector; no acceptance is implied.
  /// @return Reused or allocated reference; refusal invalidates and rolls back the transaction.
  std::expected<record_reference, format_error> intern_request_v2(const history::request_v2& value);
  /// @brief Copy a validated record, including this transaction's uncommitted inserts.
  /// @param id Positive signed-64 native ID.
  /// @return Owned record or typed failure, which also invalidates this writer.
  std::expected<stored_record, format_error> read_record(std::uint64_t id);
  /// @brief Abandon all pending records and permanently invalidate this writer; repeated calls are harmless.
  /// @note Releases transaction resources through RAII without committing or promising rollback-error reporting.
  void abort() noexcept;
  /// @brief Revalidate and commit all records atomically once.
  /// @return Success or typed failure; this writer is unavailable after either outcome.
  std::expected<void, format_error> commit();

private:
  friend std::expected<record_writer, format_error> begin_records(const std::filesystem::path&);
  explicit record_writer(write_transaction transaction);
  std::optional<write_transaction> _transaction;
};
/// @brief Independently verify an explicit ledger and acquire a record write transaction with BEGIN IMMEDIATE.
/// @param path Existing no-follow quiescent ledger, verified read-only before writable access.
/// @return Opaque independent writer after releasing its own read snapshot and revalidating under the write lock.
std::expected<record_writer, format_error> begin_records(const std::filesystem::path& path);
/// @brief Verify an explicit regular ledger without mutation before optionally opening it writable.
/// @param path Existing no-follow native file path; original components must traverse successfully.
/// @param access Desired access; both modes retain a consistent read snapshot.
/// @return Opaque owner or refusal. Supported embedded accepted history is verified across codec1 and codec2 requests.
/// @note Writable access revalidates the selected exact schema under the acquired transaction.
std::expected<format_file, format_error> open_existing(const std::filesystem::path& path,
                                                       ledger_access                access = ledger_access::read_only);
/// @brief Bind original native admission before raw header access, then verify through the same SQLite read guard.
/// @param path Explicit existing no-follow ledger path.
/// @param guard Trusted stable original-evidence adapter; null refuses. Original rich failures remain adapter-owned.
/// @return Read-only verified owner or typed format refusal. No writable transition is available.
std::expected<format_file, format_error> open_existing_gated(const std::filesystem::path&       path,
                                                             std::shared_ptr<format_read_guard> guard);
/// @brief Atomically publish a newly committed ledger with no replacement of any existing target.
/// @param path New filename in an existing real directory; no directories are created except owned staging.
/// @param workspace_id ASCII declaration identifier, [a-z][a-z0-9_]{0,63}.
/// @return Recorded identity/time or typed failure; publication_uncertain requires inspecting the explicit target.
/// @note Initialization actor is the fixed native assertion `planar:init`. No CLI or ambient data discovery occurs.
std::expected<ledger_metadata, format_error> initialize_new(const std::filesystem::path& path, std::string_view workspace_id);
/// @brief Initialize a format2 ledger at a new explicit target using the same atomic publication rules.
/// @param path Absent filename in an existing real directory.
/// @param workspace_id ASCII declaration identifier, [a-z][a-z0-9_]{0,63}.
/// @return Preserved format2 initialization metadata or typed refusal.
/// @note Native format2 embedded acceptance and explicit recovery use the selected exact schema.
/// Existing initialize_new and public init continue to create format1 until the reviewed admission cutover.
std::expected<ledger_metadata, format_error> initialize_new_v2(const std::filesystem::path& path, std::string_view workspace_id);
/// @brief Snapshot a quiescent initialized ledger through SQLite online backup and publish a new verified file.
/// @param source Explicit source file; caller must close writers for the entire operation.
/// @param target Absent explicit filename in an existing real directory, with no journal/WAL/SHM entries.
/// @return Preserved identity/workspace/time or typed refusal; published targets survive uncertain sync/cleanup failures.
std::expected<ledger_metadata, format_error> backup_new(const std::filesystem::path& source, const std::filesystem::path& target);
/// @brief Validate an initialized backup and restore through SQLite online backup into a new target.
/// @param source Explicit quiescent backup; no source mutation or implicit recovery occurs.
/// @param target New explicit target; existing files and sidecars are never replaced.
/// @return Preserved metadata or typed refusal after complete accepted-history validation.
std::expected<ledger_metadata, format_error> restore_new(const std::filesystem::path& source,
                                                         const std::filesystem::path& target);
/// @brief Explicitly recover a quiescent initialized ledger after validating its hot rollback journal in owned scratch.
/// @param path Existing explicit main file with a regular hot DELETE journal, without WAL/SHM.
/// @return Verified recovered metadata or refusal. Ordinary open_existing never invokes this operation.
/// @note Caller must close all writers for the whole operation and own the directory cooperatively. Quiescence is a
/// precondition, not detected or enforced against external SQLite users. SQLite journal checksums are not authentication;
/// scratch validation proves the supported recovered state, not provenance of arbitrary malicious journal bytes.
std::expected<ledger_metadata, format_error> recover_quiescent(const std::filesystem::path& path);
} // namespace planar::engine::ledger
