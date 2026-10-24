/// @file interpretation_codec.cppm
/// @brief Pure fixed interpretation encoding and historical decoding without runtime, storage or embedded code.
export module planar.engine.history.interpretation_codec;
import std;
export import planar.engine.value;
export namespace planar::engine::history {
/// Interpretation boundary categories; live declaration diagnostics remain distinct from recorded-format failures.
enum class interpretation_error_code { malformed, unsupported_version, resource_limit, validation, runtime };
/// Redacted native failure or the unchanged typed trusted declaration diagnostic.
struct interpretation_error {
  interpretation_error_code code;            ///< Native boundary category.
  diagnostic                detail     = {}; ///< Sanitized runtime or codec detail, when applicable.
  value_ptr                 validation = {}; ///< Original trusted error value, only for live validation.
};
/// Complete immutable typed storage values and their exact canonical_v1 encodings; no database-local IDs.
struct recorded_interpretation {
  value_ptr   interpretation;   ///< Exact interpretation_v1 envelope.
  value_ptr   versions;         ///< Exact versions map also present in the envelope.
  std::string encoded;          ///< Canonical interpretation bytes, at most 16 MiB.
  std::string versions_encoded; ///< Canonical versions bytes, at most 16 MiB.
};
/// @brief Decode a historical envelope and its separate versions value without executing Lua or current declaration policy.
/// @param encoded Exact canonical_v1 interpretation bytes, at most 16 MiB.
/// @param versions_encoded Exact canonical_v1 versions bytes, required to equal the embedded versions.
/// @return Complete immutable values after fixed envelope, version, container, ordering and correspondence checks.
/// @note Thread-safe; historical stdlib digests are validated syntactically and never compared to today's code.
std::expected<recorded_interpretation, interpretation_error> decode_interpretation(std::string_view encoded,
                                                                                   std::string_view versions_encoded);
/// @brief Assemble fixed version metadata and encode through the shared historical envelope checker.
/// @param bundle Complete normalized bundle, without applying current declaration policy.
/// @param semantic Complete recorded semantic model.
/// @param stdlib_digest Lowercase 32-digit digest supplied as recorded data; no authenticity is asserted.
/// @return Complete canonical values or the same typed codec refusal as historical decoding.
/// @note Thread-safe; no Lua, embedded-source, filesystem or storage access occurs.
std::expected<recorded_interpretation, interpretation_error>
encode_interpretation(const value_ptr& bundle, const value_ptr& semantic, std::string_view stdlib_digest);
} // namespace planar::engine::history
