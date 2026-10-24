/// @file format.cppm
/// @brief Versioned native initialization and opaque verified ledger owners with typed format and filesystem refusals.
export module planar.engine.ledger.format;
import std;
export import planar.engine.ledger.types;
export import planar.engine.ledger.sqlite;
export import planar.engine.ledger.completion;
export import planar.engine.ledger.history_codec;
export import planar.engine.acceptance.prepared;
export namespace planar::engine::ledger {
/// Exact shared ledger failure category.
using format_error_code = ::planar::engine::ledger::types::format_error_code;
/// Exact shared redacted ledger failure.
using format_error = ::planar::engine::ledger::types::format_error;
/// @brief Exact supported on-disk schema discriminator.
enum class ledger_format { v1 = 1, v2 = 2 };
/// @brief Verified initialization metadata; identifiers remain strings at JSON boundaries.
struct ledger_metadata {
  std::string   identity;                   ///< Thirty-two lowercase hexadecimal digits from sixteen OS-random bytes.
  std::string   workspace_id;               ///< Declaration identifier grammar.
  std::string   created_at;                 ///< UTC RFC3339 timestamp with milliseconds.
  ledger_format format = ledger_format::v1; ///< Exact verified schema, retained independently of request codecs.
};
/// @brief Requested access after independent read-only verification.
enum class ledger_access { read_only, read_write };
/// Exact shared native immutable record identity.
using record_reference = ::planar::engine::ledger::types::record_reference;
/// Exact shared owned bounded record.
using stored_record = ::planar::engine::ledger::types::stored_record;
/// Owned current projection from the pinned snapshot.
struct history_head {
  std::optional<std::int64_t> revision;      ///< Active revision, absent before the first acceptance.
  std::int64_t                last_sequence; ///< Last committed event sequence.
};
/// Owned immutable revision metadata; all five references are database local.
struct revision_row {
  std::int64_t                               id;          ///< Positive revision identity.
  std::optional<std::int64_t>                parent;      ///< Older revision, absent for initial acceptance.
  ::planar::engine::ledger::event_references references;  ///< Complete captured roots.
  std::string                                actor;       ///< Bounded UTF-8 native assertion.
  std::string                                accepted_at; ///< Contextual UTC time, never a causal clock.
};
/// Owned event row retaining exact saved canonical payload bytes.
struct event_row {
  std::int64_t                sequence;    ///< Positive event sequence.
  std::string                 type;        ///< Fixed event type.
  std::int64_t                version;     ///< Event row version.
  std::optional<std::int64_t> revision;    ///< Absent only at initialization.
  std::optional<std::string>  request_id;  ///< Absent only at initialization.
  std::string                 actor;       ///< Bounded UTF-8 native assertion.
  std::string                 occurred_at; ///< Contextual UTC time.
  std::string                 payload;     ///< Exact canonical event bytes; decode with the pure history codec.
};
/// Owned private replay data. Host argument bytes must never appear in public history output.
struct request_row {
  std::string                                       request_id;     ///< Bounded replay key.
  ::planar::engine::ledger::types::record_reference body_reference; ///< Validated request record identity.
  std::string                            body_encoded;   ///< Exact stored request codec bytes, including private host arguments.
  std::int64_t                           event_sequence; ///< Saved successful operation event.
  std::int64_t                           revision;       ///< Returned revision.
  ::planar::engine::ledger::load_outcome outcome;        ///< Accepted or unchanged result.
  std::string                            result;         ///< Exact saved canonical result bytes.
};
/// One bounded ascending event page from the pinned snapshot.
struct event_page {
  std::vector<event_row>      events;     ///< At most the explicitly requested limit.
  std::optional<std::int64_t> next_after; ///< Last returned sequence only when this snapshot has more events.
};
/// Exact committed response bytes, buffered until commit succeeds.
struct acceptance_result {
  std::string result;   ///< Original canonical saved JSON bytes.
  bool        replayed; ///< True only for an exact previously committed request.
};
/// @brief Complete one prepared acceptance atomically, resolving exact replay before inspecting the candidate.
/// @param path Explicit existing ledger path.
/// @param intent Immutable validated request and expected revision.
/// @param candidate Immutable prepared input retaining native capture receipts.
/// @return Exact saved result after commit, or a redacted refusal with all tentative writes rolled back.
/// @note Thread-confined per call; native receipts are verified immediately before SQLite commit.
std::expected<acceptance_result, format_error>
complete_acceptance(const std::filesystem::path& path, const ::planar::engine::acceptance::request_intent& intent,
                    const ::planar::engine::acceptance::prepared_acceptance& candidate);
/// @brief Complete against the ledger identity retained by a native prelude, retaining authoritative completion evidence.
/// @param path Explicit existing ledger path.
/// @param intent Immutable validated request and expected revision.
/// @param candidate Immutable prepared input; exact replay does not inspect it.
/// @param expected_ledger_identity Native prelude identity, checked before locked request lookup.
/// @param receipt Initially empty thread-confined output; saved bytes and phase survive errors and exceptions.
/// @return Exact saved result or typed refusal; an uncertain receipt never promises rollback.
std::expected<acceptance_result, format_error>
complete_acceptance(const std::filesystem::path& path, const ::planar::engine::acceptance::request_intent& intent,
                    const ::planar::engine::acceptance::prepared_acceptance& candidate, std::string_view expected_ledger_identity,
                    ::planar::engine::ledger::completion_receipt& receipt);
/// @brief Trusted read guard bound to the actual no-follow descriptor before any ledger header access.
/// @note Thread-confined; bind must not read content, run Lua or recursively call SQLite. The same object gates SQLite.
class format_read_guard : public read_guard {
public:
  /// @brief Bind the actual held regular-file descriptor to original native admission evidence.
  /// @param descriptor Borrowed descriptor; valid through the complete format owner lifetime.
  /// @return ready, refused or resource; any other value or exception fails closed.
  virtual read_gate_status bind(int descriptor) = 0;
};
struct format_state;
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
  friend std::expected<record_writer, format_error>   begin_records(const std::filesystem::path&);
  friend std::expected<format_file, format_error>     open_existing(const std::filesystem::path&, ledger_access);
  friend std::expected<ledger_metadata, format_error> backup_new(const std::filesystem::path&, const std::filesystem::path&);
  friend std::expected<format_file, format_error>     open_existing_gated(const std::filesystem::path&,
                                                                          std::shared_ptr<format_read_guard>);
  static std::expected<format_file, format_error>     open_impl(const std::filesystem::path&, ledger_access,
                                                                std::shared_ptr<format_read_guard>);
  explicit format_file(std::shared_ptr<format_state> state);
  std::shared_ptr<format_state> _state;
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
  friend std::expected<acceptance_result, format_error>
  complete_acceptance(const std::filesystem::path&, const ::planar::engine::acceptance::request_intent&,
                      const ::planar::engine::acceptance::prepared_acceptance&);
  friend std::expected<acceptance_result, format_error>
  complete_acceptance(const std::filesystem::path&, const ::planar::engine::acceptance::request_intent&,
                      const ::planar::engine::acceptance::prepared_acceptance&, std::string_view,
                      ::planar::engine::ledger::completion_receipt&);
  static std::expected<acceptance_result, format_error>
  complete_acceptance_impl(const std::filesystem::path&, const ::planar::engine::acceptance::request_intent&,
                           const ::planar::engine::acceptance::prepared_acceptance&, std::optional<std::string_view>,
                           ::planar::engine::ledger::completion_receipt&);
  explicit record_writer(std::shared_ptr<format_state> state);
  std::shared_ptr<format_state> _state;
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
