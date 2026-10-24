/// @file
/// @brief Internal SQLite ownership, bound values and transactions with typed storage failures.
export module planar.engine.ledger.sqlite;
import std;
export namespace planar::engine::ledger {
/// @brief Sticky native gate outcome; ready is the only permitting value.
enum class read_gate_status {
  ready,
  refused,
  resource,
  verifier_exception,
  invalid_status,
  moved_unsupported,
  moved_error,
  moved,
  sidecar
};
/// @brief Trusted native verifier retained through actual SQLite close; confined to the caller thread.
/// @note Adapters own original diagnostics and receipts. Verification must not call Lua or SQLite recursively.
class read_guard {
public:
  /// @brief Release adapter-owned native receipts after the last retained owner.
  virtual ~read_guard() = default;
  /// @brief Check original native admission evidence before content access; may throw and is caught by the gate.
  /// @return ready, refused or resource; other values fail closed as invalid_status.
  virtual read_gate_status verify() = 0;
};
/// @brief Storage failure preserving SQLite primary/extended codes; policy failures use SQLITE_MISUSE.
struct sqlite_error {
  /// @brief Primary SQLite result code.
  int code;
  /// @brief Extended SQLite result code.
  int extended_code;
  /// @brief SQLite or wrapper diagnostic, for trusted callers.
  std::string message;
  /// @brief First native gate refusal, independent of SQLite diagnostic allocation; ready for ungated errors.
  read_gate_status gate_status = read_gate_status::ready;
};
/// @brief Explicit primitive opening modes; scratch_create is only for caller-owned scratch/staging paths.
enum class open_mode { read_only, read_write, scratch_create };
/// @brief Prepared statement progress.
enum class step_result { row, done };
struct sqlite_state;
class statement;
class transaction;
/// @brief Move-only connection; all related objects are confined to one caller thread.
/// @note Statements/transactions retain the connection after this wrapper is destroyed. No format verification occurs.
class connection {
public:
  /// @brief Open an explicit path without URI parsing, path discovery or parent creation.
  /// @param path Nonempty native path without NUL, made absolute without normalization or link resolution before SQLite.
  /// @note The original parent must traverse to an existing directory; no parent directories are created.
  /// @note SQLite NOFOLLOW rejects symbolic links in its resolved pathname; colon-prefixed names remain native files.
  /// @param mode Existing read-only/read-write, or SQLite CREATE for trusted scratch staging (not exclusive publication).
  /// @param busy_ms Busy timeout in milliseconds, from zero through 5000; default 5000.
  /// @return Connection or typed storage failure. This is not a safe ledger opener.
  static std::expected<connection, sqlite_error> open(const std::filesystem::path& path, open_mode mode, int busy_ms = 5000);
  /// @brief Open read-only through a private nondefault VFS gating each main read and mmap fetch.
  /// @param path Explicit native path with the same rules as open.
  /// @param guard Stable trusted native verifier; null is refused. Original diagnostic remains adapter-owned.
  /// @param busy_ms Busy timeout from zero through 5000.
  /// @return Connection or storage error carrying the first typed gate refusal.
  /// @note This primitive supplies no profile admission, header binding or whole-invocation accounting.
  static std::expected<connection, sqlite_error> open_gated(const std::filesystem::path& path, std::shared_ptr<read_guard> guard,
                                                            int busy_ms = 5000);
  /// @brief Inspect first gate status without allocating, including after error construction throws.
  /// @return First typed refusal, or ready for a healthy, ungated or moved-from owner.
  read_gate_status gate_status() const noexcept;
  /// @brief Transfer connection ownership, leaving the source unavailable.
  connection(connection&&) noexcept = default;
  /// @brief Release this wrapper and transfer connection ownership.
  /// @return This connection wrapper.
  connection& operator=(connection&&) noexcept = default;
  connection(const connection&)                = delete;
  connection& operator=(const connection&)     = delete;
  /// @brief Prepare exactly one trusted SQL statement; explicit transaction/savepoint SQL is denied.
  /// @param sql SQL bytes without embedded NUL; parameters must be bound separately.
  /// @return Owned prepared statement or typed failure; failure abandons an active transaction.
  std::expected<statement, sqlite_error> prepare(std::string_view sql);
  /// @brief Copy the pinned main snapshot through SQLite's online backup API.
  /// @param destination Distinct usable connection to an exclusively owned new staging database, without an active transaction.
  /// @return Success only after backup completion and finish; preserves SQLite failure codes.
  /// @note Internal primitive, not format validation/publication. A source read transaction is allowed. No retry loop extends
  /// the connections' configured busy wait (at most 5000 ms). Both owners remain caller-thread confined.
  std::expected<void, sqlite_error> online_backup_to(connection& destination);
  /// @brief Begin a non-nested transaction; write uses BEGIN IMMEDIATE, read uses BEGIN.
  /// @param write Whether to acquire the reserved writer lock immediately.
  /// @return Rollback-on-abandon transaction or typed failure.
  std::expected<transaction, sqlite_error> begin(bool write = true);

private:
  static std::expected<connection, sqlite_error> open_impl(const std::filesystem::path& path, open_mode mode, int busy_ms,
                                                           std::shared_ptr<read_guard> guard);
  explicit connection(std::shared_ptr<sqlite_state> state);
  std::shared_ptr<sqlite_state> _state;
};
/// @brief Move-only prepared statement, retaining its connection and copying bound bytes.
/// @note A step/bind error rolls back the active transaction. Column values are owned copies and require a current row.
class statement {
public:
  /// @brief Transfer statement ownership, leaving the source unavailable.
  /// @param other Source whose ownership is transferred.
  statement(statement&& other) noexcept;
  /// @brief Finalize this statement and transfer ownership.
  /// @param other Source whose ownership is transferred.
  /// @return This owner.
  statement& operator=(statement&& other) noexcept;
  ~statement();
  statement(const statement&)            = delete;
  statement& operator=(const statement&) = delete;
  /// @brief Bind a SQL NULL at a one-based parameter index.
  /// @param index One-based parameter index.
  /// @return Success or typed storage failure.
  std::expected<void, sqlite_error> bind_null(int index);
  /// @brief Bind a signed integer at a one-based parameter index.
  /// @param index One-based parameter index.
  /// @param value Exact signed integer.
  /// @return Success or typed storage failure.
  std::expected<void, sqlite_error> bind_integer(int index, std::int64_t value);
  /// @brief Copy arbitrary text bytes, including NUL; empty text remains text.
  /// @param index One-based parameter index.
  /// @param bytes Exact bytes copied before returning.
  /// @return Success or typed storage failure.
  std::expected<void, sqlite_error> bind_text(int index, std::string_view bytes);
  /// @brief Copy arbitrary BLOB bytes, including NUL; empty BLOB remains BLOB.
  /// @param index One-based parameter index.
  /// @param bytes Exact bytes copied before returning.
  /// @return Success or typed storage failure.
  std::expected<void, sqlite_error> bind_blob(int index, std::string_view bytes);
  /// @brief Advance once; a failure rolls back the active transaction before returning.
  /// @return Result or typed storage failure.
  std::expected<step_result, sqlite_error> step();
  /// @brief Reset execution and clear all bindings; reports the previous step error, if any.
  /// @return Result or typed storage failure.
  std::expected<void, sqlite_error> reset();
  /// @brief Read an integer column, or nullopt for SQL NULL; reject other types/invalid row/index.
  /// @param column Zero-based column index.
  /// @return Owned value, SQL NULL, or typed failure.
  std::expected<std::optional<std::int64_t>, sqlite_error> integer(int column) const;
  /// @brief Copy a text column exactly, or nullopt for SQL NULL; reject other types/invalid row/index.
  /// @param column Zero-based column index.
  /// @return Owned value, SQL NULL, or typed failure.
  std::expected<std::optional<std::string>, sqlite_error> text(int column) const;
  /// @brief Copy a BLOB column exactly, or nullopt for SQL NULL; reject other types/invalid row/index.
  /// @param column Zero-based column index.
  /// @return Owned value, SQL NULL, or typed failure.
  std::expected<std::optional<std::string>, sqlite_error> blob(int column) const;

private:
  friend class connection;
  statement(std::shared_ptr<sqlite_state> state, void* handle);
  std::expected<void, sqlite_error>                       bound(int code);
  std::expected<int, sqlite_error>                        column_type(int column) const;
  std::expected<std::optional<std::string>, sqlite_error> bytes(int column, int type) const;
  std::shared_ptr<sqlite_state>                           _state;
  void*                                                   _handle = nullptr;
  bool                                                    _row    = false;
};
/// @brief Move-only transaction; abandonment and failed commit roll back; nested/manual transactions are refused.
/// @note Any statement failure invalidates this transaction. A failed rollback poisons the connection against reuse.
class transaction {
public:
  /// @brief Transfer the active guard, leaving the source inactive.
  /// @param other Source whose ownership is transferred.
  transaction(transaction&& other) noexcept;
  /// @brief Roll back this guard and transfer another guard.
  /// @param other Source whose ownership is transferred.
  /// @return This owner.
  transaction& operator=(transaction&& other) noexcept;
  ~transaction();
  transaction(const transaction&)            = delete;
  transaction& operator=(const transaction&) = delete;
  /// @brief Commit once; a commit error is returned even when rollback succeeds.
  /// @return Result or typed storage failure.
  std::expected<void, sqlite_error> commit();
  /// @brief Commit with no-allocation noncommit evidence, published before constructing any error.
  /// @param confirmed_noncommit True only when no COMMIT was issued or an active failed COMMIT was rolled back.
  /// @return Original COMMIT failure even after successful rollback; false evidence leaves failure uncertain.
  /// @note Autocommit following failed COMMIT alone does not prove noncommit. Evidence survives error allocation failure.
  std::expected<void, sqlite_error> commit(bool& confirmed_noncommit);
  /// @brief Explicitly roll back once, returning any SQLite failure.
  /// @return Result or typed storage failure.
  std::expected<void, sqlite_error> rollback();

private:
  friend class connection;
  transaction(std::shared_ptr<sqlite_state> state, std::uint64_t generation);
  void                          abandon() noexcept;
  std::shared_ptr<sqlite_state> _state;
  std::uint64_t                 _generation = 0;
};
} // namespace planar::engine::ledger
