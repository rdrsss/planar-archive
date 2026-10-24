/// @file
/// @brief Pure bounded request_v2 framing and full-frame digests; no storage or host access.
export module planar.engine.history.request_v2;
import std;
export import planar.engine.history.record;
export namespace planar::engine::history {
/// Exact version-two intent; owned values have no shared mutable state.
struct request_v2 {
  std::string operation = "load"; ///< Only load is supported.
  std::string actor;              ///< One to 256 UTF-8 bytes without NUL.
  std::string expected_head;      ///< none or canonical positive signed-64 decimal.
  std::string workspace_argument; ///< Nonempty UTF-8 without NUL, retained exactly; intent normalization is external.
  std::string registry_argument;  ///< Nonempty UTF-8 without NUL, retained exactly; intent normalization is external.
  std::optional<std::string> profile_argument; ///< Absent for embedded; otherwise normalized absolute lexical path.
};
/// @brief Encode exact request intent without a record_v1 wrapper.
/// @param input Owned intent; no normalization or filesystem access occurs.
/// @return Complete frame at most 64 MiB or redacted local validation/resource failure.
std::expected<std::string, record_error> encode_request_v2(const request_v2& input);
/// @brief Decode exactly one bounded version-two request with strict local validation.
/// @param bytes Complete frame; unknown versions/tags and trailing bytes are refused.
/// @return Owned intent or redacted failure; no shared mutable state is used.
std::expected<request_v2, record_error> decode_request_v2(std::string_view bytes);
/// @brief Hash the complete exact frame with seed-zero XXH3-128.
/// @param input Valid version-two intent.
/// @return Canonical sixteen-byte digest or redacted failure; no shared mutable state is used.
std::expected<record_digest, record_error> digest_request_v2(const request_v2& input);
} // namespace planar::engine::history
