/// @file
/// @brief Pure record_v1 framing, local validation and portable digests with typed bounded failures.
export module planar.engine.history.record;
import std;
export namespace planar::engine::history {
/// Exact stored record kind tags.
enum class record_kind : std::uint8_t { file = 1, directory = 2, value = 3, request = 4 };
/// Canonical big-endian XXH3-128 bytes; not a render-domain identity.
using record_digest = std::array<std::uint8_t, 16>;
/// Local codec failure vocabulary; graph references and storage policy belong to callers.
enum class record_error_code { invalid_record, unsupported_codec, resource_limit };
/// Redacted typed refusal without caller bytes, host paths or storage state.
struct record_error {
  record_error_code code; ///< Invalid local encoding, unknown payload codec or exceeded safety bound.
};
/// Exact arbitrary file payload with normalized permission bits.
struct file_record {
  std::uint32_t mode = 420; ///< Only 420 and 493 are supported.
  std::string   payload;    ///< Arbitrary bytes, at most 16 MiB.
};
/// Directory child; local IDs participate in stored equality but not portable digests.
struct record_child {
  std::string   name;                     ///< One portable ASCII segment, sorted and case-alias free.
  record_kind   kind = record_kind::file; ///< Claimed kind; contextual reference verification is external.
  std::uint64_t id   = 1;                 ///< Positive signed-64 database-local ID.
  record_digest digest{};                 ///< Claimed canonical child digest.
};
/// Sorted immutable directory contents, with implicit normalized mode 493.
struct directory_record {
  std::vector<record_child> children; ///< Strictly sorted unique portable names without case aliases.
};
/// Canonical typed value envelope, preserving the exact encoded value bytes.
struct value_record {
  std::string codec = "canonical_v1"; ///< Supported version identifier.
  std::string encoded;                ///< At most 16 MiB of valid canonical_v1 bytes.
};
/// Exact request body framing; argument normalization and operation semantics are external.
struct request_record {
  std::string operation;          ///< Nonempty ASCII without NUL.
  std::string actor;              ///< One to 256 UTF-8 bytes without NUL.
  std::string expected_head;      ///< "none" or a canonical positive signed-64 decimal ID.
  std::string workspace_argument; ///< Nonempty UTF-8 without NUL; retained exactly.
  std::string registry_argument;  ///< Nonempty UTF-8 without NUL; retained exactly.
};
/// One locally valid record kind; immutable ownership is the caller's responsibility.
using record = std::variant<file_record, directory_record, value_record, request_record>;
/// @brief Identify the record's exact binary kind tag.
/// @param input Record variant.
/// @return Kind corresponding to the active alternative.
record_kind kind_of(const record& input);
/// @brief Encode a locally valid record with a 64 MiB whole-record maximum.
/// @param input Exact payload and metadata, with sorted directory children.
/// @return Owned record_v1 bytes or a redacted validation/resource refusal. No shared mutable state is used.
std::expected<std::string, record_error> encode_record(const record& input);
/// @brief Decode exactly one bounded record_v1, validating local framing before allocation.
/// @param bytes Exact bytes; unknown versions, trailing bytes and malformed local fields are refused.
/// @return Owned typed record or a redacted refusal. No database, filesystem or graph access occurs.
std::expected<record, record_error> decode_record(std::string_view bytes);
/// @brief Compute seed-zero XXH3-128 in canonical sixteen-byte order.
/// @param input Locally valid record; only directory child IDs are omitted from the hash preimage.
/// @return Portable digest or a redacted refusal; no shared mutable state is used.
std::expected<record_digest, record_error> digest_record(const record& input);
} // namespace planar::engine::history
