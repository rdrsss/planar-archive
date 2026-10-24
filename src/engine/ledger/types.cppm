/// @file types.cppm
/// @brief Inert record identities and exact ledger failure vocabulary; no storage ownership or policy.
export module planar.engine.ledger.types;
import std;
export import planar.engine.history.record;
export import planar.engine.history.request_v2;
export namespace planar::engine::ledger::types {
/// @brief Format boundary failures, including deliberately unsupported history and recovery.
enum class format_error_code {
  invalid_argument,
  not_found,
  exists,
  unsafe_path,
  foreign_ledger,
  unsupported_version,
  corrupt,
  unsupported_state,
  recovery_required,
  busy,
  storage_full,
  io,
  publication_uncertain,
  hash_collision,
  request_conflict,
  revision_conflict,
  capture_changed
};
/// Owned heads captured at the locked revision comparison; present heads must be positive.
struct revision_conflict_detail {
  std::optional<std::int64_t> expected; ///< Requested head, or none for an empty ledger.
  std::optional<std::int64_t> actual;   ///< Head observed under the acceptance write lock.
};
/// Owned collision identity; request records must never retain their private digest.
struct record_collision_detail {
  history::record_kind                  kind;   ///< Stable record kind, validated at the response boundary.
  std::optional<history::record_digest> digest; ///< Required for public kinds, forbidden for request.
};
/// Finite native diagnostic payload; no arbitrary fields, record bytes or host paths.
using format_error_detail = std::variant<revision_conflict_detail, record_collision_detail>;
/// @brief Safe native failure; no database contents or host paths are included.
struct format_error {
  format_error_code                  code;     ///< Stable native refusal category.
  std::string                        message;  ///< Safe context without captured bytes or paths.
  std::optional<format_error_detail> detail{}; ///< Authoritative owned detail, absent for generic failures.
};
/// Native immutable record identity; directory child IDs remain database local.
struct record_reference {
  std::uint64_t          id;     ///< Positive signed-64 database-local identifier.
  history::record_kind   kind;   ///< Exact kind tag.
  history::record_digest digest; ///< Portable digest, never sufficient for equality.
};
/// Stored payload alternatives; record_v1 remains a separate immutable codec.
using stored_value = std::variant<history::file_record, history::directory_record, history::value_record, history::request_record,
                                  history::request_v2>;
/// Owned bounded record copied from a pinned snapshot.
struct stored_record {
  record_reference reference; ///< Validated native identity.
  stored_value     value;     ///< Exact typed payload including directory local IDs and request codec.
  /// Construct from an unchanged codec1 payload.
  /// @param identity Verified record identity.
  /// @param payload Exact codec1 value.
  stored_record(record_reference identity, history::record payload)
      : reference(identity), value(std::visit([](auto&& item) -> stored_value { return std::move(item); }, std::move(payload))) {
  }
  /// Construct from an independently framed codec2 request.
  /// @param identity Verified private request identity.
  /// @param payload Exact codec2 intent.
  stored_record(record_reference identity, history::request_v2 payload) : reference(identity), value(std::move(payload)) {
  }
};
/// Encode a stored value through its exact codec; no I/O or shared state.
/// @param input Owned typed stored record.
/// @return Exact bytes or the codec's redacted local refusal.
inline std::expected<std::string, history::record_error> encode_stored_record(const stored_record& input) {
  return std::visit(
      [](const auto& item) -> std::expected<std::string, history::record_error> {
        if constexpr (std::is_same_v<std::decay_t<decltype(item)>, history::request_v2>)
          return history::encode_request_v2(item);
        else
          return history::encode_record(history::record{item});
      },
      input.value);
}
} // namespace planar::engine::ledger::types
