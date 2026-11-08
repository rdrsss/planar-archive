/// @file execution_schema.cppm
/// @brief Format3 execution ledger schema: exact stored-SQL creation, the liveness-aware opener and the schema verifier.
///
/// Entry points are `initialize_format3`, `open_format3` and `verify_format3_schema`. Failures use the closed
/// `execution_error_code` vocabulary, whose categories carry no message text. The 69 stored statements and the immutable and
/// mutable table lists are internal to the implementation and are not exported. `execution_database` is move-only and
/// thread-confined. The module imports neither format, acceptance, runtime nor command code.
export module planar.engine.ledger.execution_schema;

import std;
export import planar.engine.ledger.sqlite;
export import planar.engine.ledger.storage_types;

export namespace planar::engine::ledger {

/// @brief Closed native failure categories of the format3 execution ledger; no message text is carried.
enum class execution_error_code {
  unsupported_ledger_version,
  foreign_ledger,
  ledger_not_found,
  ledger_exists,
  ledger_corrupt,
  ledger_busy,
  storage_full,
  io,
  path_denied,
  recovery_required,
  request_conflict,
  revision_conflict,
  state_conflict,
  claim_conflict,
  fenced,
  resource_limit,
  invalid_change,
  hash_collision,
  unknown_run,
  check_in_progress,
  drifted,
  unknown_reservation
};

/// @brief Native failure with its closed category and the detail fields that particular categories define.
struct execution_error {
  execution_error_code code;          ///< Closed category.
  std::string          state;         ///< claim_conflict state or fenced run_state.
  std::string          limit;         ///< resource_limit limit name.
  std::uint64_t        maximum = 0;   ///< resource_limit maximum.
  std::string          expected;      ///< revision_conflict expected head: canonical decimal or "none".
  std::string          actual;        ///< revision_conflict actual head: canonical decimal or "none".
  std::string          record_kind;   ///< hash_collision kind: "file", "directory", "value" or "request".
  std::string          record_digest; ///< 32 lowercase hex digest; empty for a "request" collision.
};

/// @brief Verified identity of a format3 ledger, as stored in its single ledger_meta row.
struct execution_metadata {
  std::string identity;     ///< Thirty-two lowercase hexadecimal digits.
  std::string workspace_id; ///< Declaration identifier grammar.
  std::string created_at;   ///< UTC RFC3339 timestamp with milliseconds.
};

/// @brief Create a new format3 ledger at an explicit target in one transaction.
/// @param path Absent filename in an existing real directory; no existing target is ever replaced.
/// @param workspace_id ASCII declaration identifier, [a-z][a-z0-9_]{0,63}.
/// @return Stored identity metadata, or ledger_exists for an existing target, path_denied for a missing parent, or another
/// closed category.
std::expected<execution_metadata, execution_error> initialize_format3(const std::filesystem::path& path,
                                                                      std::string_view             workspace_id);

/// @brief Requested access to a format3 ledger after its independent verification.
enum class execution_access { read_only, read_write };

/// @brief Opaque verified format3 ledger owner; move-only and thread-confined.
/// @note The owner keeps one SQLite connection with the product settings applied. No SQL statement or table list is
/// exported, and the stored metadata is cached from verification.
class execution_database {
public:
  /// @brief Transfer the verified owner, leaving the source unusable.
  execution_database(execution_database&&) noexcept;
  /// @brief Release this owner and transfer the other verified owner.
  /// @return This owner.
  execution_database& operator=(execution_database&&) noexcept;
  /// @brief Release the connection and every retained resource.
  ~execution_database();
  /// @brief Get the metadata verified when this owner was opened.
  /// @return Cached identity metadata, valid for the lifetime of this owner.
  const execution_metadata& metadata() const;
  /// @brief Access the verified connection for the execution operations of this module and its callers.
  /// @return Connection that stays owned by this object and is confined to its thread.
  connection& sql();

private:
  struct impl; // defined in execution_schema.cpp
  explicit execution_database(std::unique_ptr<impl>) noexcept;
  std::unique_ptr<impl>                                     _impl;
  friend std::expected<execution_database, execution_error> open_format3(const std::filesystem::path&, execution_access);
};

/// @brief Open an existing format3 ledger after the liveness-aware and exact-schema checks.
/// @param path Explicit existing no-follow ledger path; a missing ledger is ledger_not_found, never created.
/// @param access Read-only or read-write; both verify the same schema before any writable transition.
/// @return Verified owner, or ledger_not_found, path_denied, foreign_ledger, unsupported_ledger_version, recovery_required,
/// ledger_corrupt, ledger_busy, storage_full or io.
std::expected<execution_database, execution_error> open_format3(const std::filesystem::path& path, execution_access access);

/// @brief Verify that a connection holds the exact format3 schema, markers and identity metadata.
/// @param db Open connection to a format3 ledger, read by the verifier and not modified.
/// @return Verified metadata, or ledger_corrupt when any stored object or marker differs, or a storage category.
std::expected<execution_metadata, execution_error> verify_format3_schema(connection& db);

} // namespace planar::engine::ledger
