/// @file request_v3.cppm
/// @brief Pure bounded request_v3 framing and full-frame digests for execution operations; no storage or host access.
export module planar.engine.history.request_v3;
import std;
export import planar.engine.history.record;
export namespace planar::engine::history {
/// Exact version-three execution intent; owned values have no shared mutable state.
struct request_v3 {
  std::string
      operation; ///< One of activate, claim, takeover, refresh, checkpoint, release, check, check-recover or verdict; never load.
  std::string actor;      ///< One to 256 UTF-8 bytes without NUL.
  std::string parameters; ///< Canonical_v1 bytes of a record value holding the operation's parameter map.
  std::optional<std::string>
      profile_argument; ///< Absent for selector 00; otherwise an absolute lexically normalized valid text path (selector 01).
};
/// @brief Encode exact version-three request intent as the complete frame.
/// @param input Owned intent. Validates the operation set, actor grammar, parameters as a canonical_v1 record and the
/// profile selector rules; no normalization or filesystem access occurs.
/// @return Complete frame of at most 64 MiB, or a redacted validation or resource refusal. Thread-safe, without shared state.
std::expected<std::string, record_error> encode_request_v3(const request_v3& input);
/// @brief Decode exactly one bounded version-three request with strict local validation.
/// @param bytes Complete frame. Other versions, unknown operations or selectors, truncation, trailing bytes and
/// non-canonical parameters are refused, and frames above 64 MiB are a resource refusal.
/// @return Owned intent or a redacted refusal. Thread-safe, without shared state.
std::expected<request_v3, record_error> decode_request_v3(std::string_view bytes);
/// @brief Hash the complete exact frame with seed-zero XXH3-128.
/// @param input Valid version-three intent; every field and the selector contribute to the digest.
/// @return Canonical sixteen-byte digest, or a redacted refusal for an invalid intent. Thread-safe, without shared state.
std::expected<record_digest, record_error> digest_request_v3(const request_v3& input);
} // namespace planar::engine::history
