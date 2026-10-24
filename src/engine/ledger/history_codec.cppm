/// @file history_codec.cppm
/// @brief Pure typed v1 event and saved load-result JSON boundaries with redacted failures.
export module planar.engine.ledger.history_codec;
import std;
export namespace planar::engine::ledger {
/// Codec-owned boundary failure categories.
enum class history_codec_error { malformed, unsupported_version, resource_limit };
/// Fixed v1 event vocabulary.
enum class event_reason { initialized, declaration_accepted, identical_declaration };
/// Successful load outcomes.
enum class load_outcome { accepted, noop };
/// Database-local positive record references; no relationship between their numeric IDs is implied.
struct event_references {
  std::int64_t source;         ///< Complete source tree identity.
  std::int64_t registry;       ///< Registry provenance identity.
  std::int64_t stdlib;         ///< Trusted standard-library provenance identity.
  std::int64_t interpretation; ///< Stored interpretation identity.
  std::int64_t versions;       ///< Stored versions identity.
  /// @brief Compare every typed field.
  /// @param other Value to compare.
  /// @return Whether all typed fields are equal.
  bool operator==(const event_references& other) const = default;
};
/// Inert event data. Only initialization omits references; optional heads encode as none.
struct history_event {
  std::optional<std::int64_t>     before;     ///< Prior active head.
  std::optional<std::int64_t>     after;      ///< Resulting active head.
  event_reason                    reason;     ///< Fixed operation vocabulary.
  std::optional<event_references> references; ///< Present for accepted and no-op loads.
  /// @brief Compare every typed field.
  /// @param other Value to compare.
  /// @return Whether all typed fields are equal.
  bool operator==(const history_event& other) const = default;
};
/// Saved successful result. Parent always belongs to the returned revision, including no-op.
struct load_result {
  std::string                 ledger;     ///< Lowercase 32-hex ledger identity.
  std::string                 request_id; ///< Bounded ASCII request token.
  load_outcome                outcome;    ///< Accepted revision or unchanged existing revision.
  std::int64_t                revision;   ///< Positive returned revision.
  std::optional<std::int64_t> parent;     ///< Absent or positive and older than revision.
  std::int64_t                event;      ///< Positive event sequence.
  /// @brief Compare every typed field.
  /// @param other Value to compare.
  /// @return Whether all typed fields are equal.
  bool operator==(const load_result& other) const = default;
};
/// @brief Encode validated event data as canonical Glaze JSON, without a terminal LF.
/// @param event Typed operation data; version is owned by this codec.
/// @return Exact bytes or a redacted typed failure. Thread-safe, with no I/O or shared state.
std::expected<std::string, history_codec_error> encode_history_event(const history_event& event);
/// @brief Decode exact canonical v1 event bytes under the inclusive one-MiB ceiling.
/// @param bytes Stored event JSON.
/// @return Validated operation or a redacted failure. Thread-safe and storage independent.
std::expected<history_event, history_codec_error> decode_history_event(std::string_view bytes);
/// @brief Encode validated result data with codec-owned version and load kind.
/// @param result Saved successful load result.
/// @return Canonical bytes or a redacted typed failure. Thread-safe, without I/O.
std::expected<std::string, history_codec_error> encode_load_result(const load_result& result);
/// @brief Decode exact canonical v1 saved load-result bytes under the inclusive one-MiB ceiling.
/// @param bytes Stored result JSON.
/// @return Validated result or a redacted typed failure. Thread-safe and storage independent.
std::expected<load_result, history_codec_error> decode_load_result(std::string_view bytes);
} // namespace planar::engine::ledger
