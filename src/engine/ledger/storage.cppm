/// @file storage.cppm
/// @brief Native verified ledger storage: pinned read snapshots, one opaque locked write transaction and the minimum internal helpers shared with initialization/recovery.
///
/// Entry points are `read_snapshot::open_read`, `begin_write` and the `native` helper namespace. Failures use the
/// shared redacted `format_error` vocabulary. Every owner is thread-confined and move-only. No raw SQLite handle, callback
/// or caller-supplied SQL escapes: `write_transaction` exposes a finite set of typed operations, and any failed operation
/// rolls the whole transaction back and invalidates it. The module imports neither runtime, acceptance nor format.
/// Format3 ledgers open read-only with a reduced shape check (R15-C DD35); their complete verification and every format3
/// write belong to the execution module, and writable format3 access here is refused as an unsupported version.
module;
#include <sys/stat.h>
#include <unistd.h>
export module planar.engine.ledger.storage;
import std;
export import planar.engine.ledger.storage_types;
export import planar.engine.ledger.sqlite;
import planar.engine.history.tree_core;
export namespace planar::engine::ledger {
/// @brief Trusted read guard bound to the actual no-follow descriptor before any ledger header access.
/// @note Thread-confined; bind must not read content, run Lua or recursively call SQLite. The same object gates SQLite.
class storage_read_guard : public read_guard {
public:
  /// @brief Bind the actual held regular-file descriptor to original native admission evidence.
  /// @param descriptor Borrowed descriptor; valid through the complete snapshot lifetime.
  /// @return ready, refused or resource; any other value or exception fails closed.
  virtual read_gate_status bind(int descriptor) = 0;
};
/// @brief Which public operation is committing; selects the exact refusal text only, never policy.
enum class commit_site { record_writer, acceptance };
/// @brief Native facts for one acceptance's revision/event/request/current-state rows, decided by the caller.
struct acceptance_rows_input {
  history_head                head;       ///< Current locked head read from this transaction.
  std::optional<revision_row> previous;   ///< Active revision row when head has a revision.
  event_references            references; ///< Interned complete roots of the candidate.
  bool                        noop;       ///< True when the candidate is identical to the active content.
  std::string_view            request_id; ///< Validated idempotency key.
  std::string_view            actor;      ///< Validated bounded actor.
  record_reference            body;       ///< Interned request body record.
};
/// @brief Rows written by append_acceptance_rows, including the exact canonical result bytes saved in the request row.
struct acceptance_rows_output {
  std::string                 result;   ///< Exact saved canonical result bytes.
  std::int64_t                revision; ///< Returned revision, existing for no-op.
  std::optional<std::int64_t> parent;   ///< Parent of the returned revision.
  std::int64_t                sequence; ///< Newly appended event sequence.
};
struct storage_state;
class write_transaction;
/// @brief Opaque thread-confined owner of a verified ledger and pinned read snapshot.
/// @note No SQL or SQLite handle escapes. Cooperative directory ownership is required; hostile swapping is unsupported.
class read_snapshot {
public:
  /// @brief Transfer the opaque owner.
  read_snapshot(read_snapshot&&) noexcept = default;
  /// @brief Transfer the opaque owner, releasing the previous snapshot.
  /// @return This owner.
  read_snapshot& operator=(read_snapshot&&) noexcept = default;
  read_snapshot(const read_snapshot&)                = delete;
  read_snapshot& operator=(const read_snapshot&)     = delete;
  /// @brief Verify an explicit regular ledger without mutation before optionally opening it writable.
  /// @param path Existing no-follow native file path; original components must traverse successfully.
  /// @param access Desired access; both modes retain a consistent read snapshot.
  /// @param guard Optional trusted original-evidence adapter bound before the first raw read.
  /// @return Opaque owner or refusal.
  static std::expected<read_snapshot, format_error> open_read(const std::filesystem::path& path, ledger_access access,
                                                              std::shared_ptr<storage_read_guard> guard);
  /// @brief Get cached immutable metadata verified at open, including after a failed read.
  /// @return Initialization metadata; caller must not use a moved-from owner.
  const ledger_metadata& metadata() const;
  /// @brief Inspect sticky SQLite gate status without allocation.
  /// @return First gate refusal, or ready for ungated, healthy or moved owners.
  read_gate_status gate_status() const noexcept;
  /// @brief Copy one bounded typed record from this verified snapshot.
  /// @param id Positive signed-64 native ID.
  /// @return Validated record or typed failure; every failure permanently invalidates further reads.
  std::expected<stored_record, format_error> read_record(std::uint64_t id) const;
  /// @brief Read the current head and last event from this pinned snapshot.
  /// @return Owned projection or a sticky typed failure.
  std::expected<history_head, format_error> read_head() const;
  /// @brief Read one immutable revision from this snapshot.
  /// @param id Positive signed-64 native ID.
  /// @return Owned row or a successful optional miss; failures invalidate all reads.
  std::expected<std::optional<revision_row>, format_error> read_revision(std::uint64_t id) const;
  /// @brief Read one exact saved event from this snapshot.
  /// @param sequence Positive signed-64 sequence.
  /// @return Owned row or successful optional miss; failures invalidate all reads.
  std::expected<std::optional<event_row>, format_error> read_event(std::uint64_t sequence) const;
  /// @brief Read private replay data, including exact saved request and result bytes.
  /// @param key ASCII request token of one through 128 bytes.
  /// @return Owned row or successful optional miss; failures invalidate all reads.
  std::expected<std::optional<request_row>, format_error> read_request(std::string_view key) const;
  /// @brief Read an ascending exclusive event page without caching history.
  /// @param cursor Zero starts before event one; otherwise at most signed-64 maximum.
  /// @param limit Explicit count from one through 100.
  /// @return Owned bounded page; failures invalidate all reads.
  std::expected<event_page, format_error> read_events_after(std::uint64_t cursor, std::size_t limit) const;
  /// @brief Read a format3 request row of any execution operation, without its private body.
  /// @param key ASCII request token of one through 128 bytes.
  /// @return Owned row or successful optional miss; invalid_argument on a format1 or format2 snapshot; failures invalidate.
  std::expected<std::optional<::planar::engine::ledger::execution_request_row>, format_error> read_execution_request(std::string_view key) const;
  /// @brief Replay the format3 execution state at one event from immutable rows, without Lua (P2-5 `--at-event`).
  /// @param sequence Positive event sequence; rows with a greater sequence are ignored.
  /// @param job Active job at that event, or empty for none (then no roles or runs are replayed).
  /// @return Latest activation, the job's roles and the active runs; invalid_argument on format1 or format2.
  std::expected<::planar::engine::ledger::execution_state_at, format_error> read_execution_state_at(std::uint64_t sequence, std::string_view job) const;
  /// @brief Copy this pinned snapshot into an already staged destination through SQLite online backup.
  /// @param destination Existing staged file owned by the caller.
  /// @return Success or storage failure mapped to the shared vocabulary.
  std::expected<void, format_error> backup_to(const std::filesystem::path& destination) const;

private:
  friend std::expected<write_transaction, format_error> begin_write(const std::filesystem::path&);
  explicit read_snapshot(std::shared_ptr<storage_state> state);
  std::shared_ptr<storage_state> _state;
};
/// @brief Independently owned, move-only thread-confined locked write transaction.
/// @note No raw handles, callbacks or caller SQL. Every failed operation, abort or abandonment rolls back the entire
/// transaction and invalidates it: later calls return invalid_argument 'Unavailable record transaction'.
class write_transaction {
public:
  /// Transfer the transaction; the source becomes unavailable.
  write_transaction(write_transaction&&) noexcept = default;
  /// Release the previous transaction and transfer ownership.
  /// @return This owner.
  write_transaction& operator=(write_transaction&&) noexcept = default;
  write_transaction(const write_transaction&)                = delete;
  write_transaction& operator=(const write_transaction&)     = delete;
  /// @brief Get the metadata verified at entry; remains readable after invalidation.
  /// @return Initialization metadata, including the selected format.
  const ledger_metadata& metadata() const;
  /// @brief Read one saved request under the write lock, including uncommitted rows.
  /// @param key ASCII request token of one through 128 bytes.
  /// @return Owned row or a successful optional miss (a miss does not invalidate); failures invalidate.
  std::expected<std::optional<request_row>, format_error> read_request(std::string_view key);
  /// @brief Read the locked current head.
  /// @return Owned projection; failure invalidates.
  std::expected<history_head, format_error> read_head();
  /// @brief Read one immutable revision under the write lock.
  /// @param id Positive signed-64 native ID.
  /// @return Owned row or optional miss; failures invalidate.
  std::expected<std::optional<revision_row>, format_error> read_revision(std::uint64_t id);
  /// @brief Copy a validated record, including this transaction's uncommitted inserts.
  /// @param id Positive signed-64 native ID.
  /// @return Owned record; any failure invalidates the transaction.
  std::expected<stored_record, format_error> read_record(std::uint64_t id);
  /// @brief Intern one exact record, checking every digest-bucket candidate and directory reference.
  /// @param value Locally valid typed record; children must already exist in this transaction.
  /// @return Reused or newly allocated reference; no implicit commit occurs.
  std::expected<record_reference, format_error> intern_value(const history::record& value);
  /// @brief Intern one stored request value under either request codec.
  /// @param value Locally valid typed value (codec1 request_record or codec2 request_v2).
  /// @return Reused or allocated reference; refusal invalidates and rolls back the transaction.
  std::expected<record_reference, format_error> intern_request(const types::stored_value& value);
  /// @brief Intern one complete bounded tree.
  /// @param files Complete portable files selected for the tree.
  /// @param profile Fixed independent tree budget.
  /// @return Root directory reference; refusal invalidates the transaction.
  std::expected<record_reference, format_error> intern_tree(std::span<const history::tree_file> files,
                                                            history::tree_profile               profile);
  /// @brief Compare exact bounded record bytes between two complete reference sets.
  /// @param left First complete roots.
  /// @param right Second complete roots.
  /// @return True only when every pair has equal complete encodings; failure invalidates.
  std::expected<bool, format_error> identical_content(const event_references& left, const event_references& right);
  /// @brief Append the revision (unless no-op), event, request and current-state rows for one decided acceptance.
  /// @param input Native facts decided by the caller; storage decides no replay, CAS, family or outcome policy.
  /// @return Saved result bytes and the returned revision/parent/sequence; failure invalidates.
  std::expected<acceptance_rows_output, format_error> append_acceptance_rows(const acceptance_rows_input& input);
  /// @brief Repeat held-file/name identity, full verification and complete metadata equality including format.
  /// @param site Selects the exact refusal text; no other behavior differs.
  /// @return Success, or a refusal that invalidates the transaction.
  std::expected<void, format_error> verify_before_commit(commit_site site);
  /// @brief Commit once; this transaction is unavailable after either outcome. Performs no verification itself.
  /// @param site Public operation committing.
  /// @return Success or storage failure.
  std::expected<void, format_error> commit(commit_site site);
  /// @brief Commit once with no-allocation noncommit evidence.
  /// @param site Public operation committing.
  /// @param confirmed_noncommit True only when no COMMIT was issued or an active failed COMMIT was rolled back.
  /// @return Success or the original storage failure; this transaction is unavailable after either outcome.
  std::expected<void, format_error> commit(commit_site site, bool& confirmed_noncommit);
  /// @brief Abandon all pending writes and permanently invalidate this transaction; repeated calls are harmless.
  void abort() noexcept;

private:
  friend std::expected<write_transaction, format_error> begin_write(const std::filesystem::path&);
  write_transaction(std::shared_ptr<storage_state> state, ledger_metadata metadata);
  std::shared_ptr<storage_state> _state;
  ledger_metadata                _metadata;
};
/// @brief Independently verify an explicit ledger and acquire a write transaction with BEGIN IMMEDIATE.
/// @param path Existing no-follow quiescent ledger, verified read-only before writable access.
/// @return Opaque transaction after releasing its own read snapshot and revalidating under the write lock; this is the
/// entry metadata check, separate from verify_before_commit.
std::expected<write_transaction, format_error> begin_write(const std::filesystem::path& path);
} // namespace planar::engine::ledger
/// Internal helpers shared with format initialization, backup and recovery. Not a public storage API.
export namespace planar::engine::ledger::native {
/// Owned file descriptor closed on destruction; move-only.
struct descriptor {
  /// Raw file descriptor; negative when empty.
  int fd       = -1;
  descriptor() = default;
  /// @brief Take ownership of a descriptor.
  /// @param value Raw descriptor, possibly negative.
  explicit descriptor(int value) : fd(value) {
  }
  /// @brief Transfer ownership.
  /// @param other Source left empty.
  descriptor(descriptor&& other) noexcept : fd(std::exchange(other.fd, -1)) {
  }
  /// @brief Close the previous descriptor and transfer ownership.
  /// @param other Source left empty.
  /// @return This owner.
  descriptor& operator=(descriptor&& other) noexcept {
    if (this != &other) {
      if (fd >= 0)
        ::close(fd);
      fd = std::exchange(other.fd, -1);
    }
    return *this;
  }
  /// Close the held descriptor.
  ~descriptor() {
    if (fd >= 0)
      ::close(fd);
  }
};
/// Retained parent directory, normalized path and filename of an explicit ledger.
struct location {
  descriptor            parent; ///< Held no-follow parent directory.
  std::filesystem::path path;   ///< Absolute lexically normalized explicit path.
  std::string           name;   ///< Final component.
};
/// @brief Compare complete verified metadata, including the selected format.
/// @param a First metadata.
/// @param b Second metadata.
/// @return True when every field is equal.
bool same_metadata(const ledger_metadata& a, const ledger_metadata& b);
/// @brief Compare device/inode identity.
/// @param a First stat.
/// @param b Second stat.
/// @return True when identical.
bool same(const struct stat& a, const struct stat& b);
/// @brief Traverse original parent components without following symlinks.
/// @param path Explicit ledger path.
/// @return Retained location or a typed refusal.
std::expected<location, format_error> locate(const std::filesystem::path& path);
/// @brief Verify that the held parent and filename still name the wanted regular file.
/// @param where Retained location.
/// @param wanted Identity of the held file.
/// @return True only when everything still matches.
bool identity_matches(const location& where, const struct stat& wanted);
/// @brief Current UTC time as a fixed-width millisecond timestamp.
/// @return RFC3339 text.
std::string timestamp();
/// @brief Run one statement to completion.
/// @param db Open connection.
/// @param sql Statement text.
/// @return Success or typed failure.
std::expected<void, format_error> execute(connection& db, std::string_view sql);
/// @brief Run one single-integer query.
/// @param db Open connection.
/// @param sql Query text.
/// @return The integer or typed failure.
std::expected<std::int64_t, format_error> scalar(connection& db, std::string_view sql);
/// @brief Configure and check required connection settings.
/// @param db Open connection.
/// @param configure False checks without setting.
/// @return Success or typed failure.
std::expected<void, format_error> settings(connection& db, bool configure = true);
/// @brief Exact schema statements for one format.
/// @param format Selected format.
/// @return Twenty-one ordered statements.
std::array<std::string, 21> selected_schema(ledger_format format);
/// @brief Validate the raw SQLite header of an open descriptor.
/// @param fd Borrowed descriptor.
/// @return Success or typed refusal.
std::expected<void, format_error> header(int fd);
/// @brief Refuse any journal, WAL or shared-memory sidecar.
/// @param where Retained location.
/// @return Success or typed refusal.
std::expected<void, format_error> no_sidecars(const location& where);
/// @brief Liveness-aware sidecar check for the execution opener, forwarding the same rule the read snapshot applies.
/// @note A -wal or -shm always refuses. A -journal refuses unless a live writer holds the RESERVED byte of the borrowed
/// descriptor, in which case SQLite busy handling decides. The refusal category is recovery_required.
/// @param where Retained location.
/// @param fd Borrowed read-only ledger descriptor, used only for the RESERVED probe and never closed here.
/// @return Success when no sidecar blocks the open, recovery_required for an orphan sidecar, or the shared io failure.
std::expected<void, format_error> check_orphan_sidecars(const location& where, int fd);
/// @brief Fully verify schema, records and accepted history under the current transaction.
/// @param db Open connection inside a transaction.
/// @return Verified metadata or typed failure.
std::expected<ledger_metadata, format_error> verify(connection& db);
} // namespace planar::engine::ledger::native
