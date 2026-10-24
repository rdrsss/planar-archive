/// @file
/// @brief Pure interpretation-v2 encoding and raw effective-tree binding with closed recorded-data failures.
export module planar.engine.history.interpretation_v2_codec;
import std;
export import planar.engine.history.effective_provenance;
export import planar.engine.value;
export namespace planar::engine::history {
/// Closed recorded-data categories, independent of live validation and runtime errors.
enum class interpretation_v2_error_code { malformed, unsupported_version, resource_limit, invalid_tree, invalid_correspondence };
/// Redacted refusal without supplied content, host paths or runtime state.
struct interpretation_v2_error {
  interpretation_v2_error_code code; ///< Deterministic recorded-data boundary category.
};
/// Owned canonical recorded values; ordinary data, without same-VM or acceptance authority.
struct recorded_interpretation_v2 {
  value_ptr   interpretation;   ///< Complete recursively const-facing decoded envelope, with no input aliases.
  value_ptr   versions;         ///< Separate versions value equal to the embedded versions.
  std::string encoded;          ///< Exact canonical interpretation bytes.
  std::string versions_encoded; ///< Exact canonical versions bytes.
};
/// @brief Decode fixed historical structure and independently bind it to raw recorded effective provenance.
/// @param encoded Canonical interpretation-v2 bytes, at most 16 MiB.
/// @param versions_encoded Canonical separate versions bytes, at most 16 MiB.
/// @param raw_files Complete effective tree, stable throughout this call; caller digest claims are never accepted.
/// @return Owned canonical data or a redacted typed refusal; no callability or semantic honesty proof.
/// @note Pure and thread-safe; performs no Lua, host filesystem or storage access.
std::expected<recorded_interpretation_v2, interpretation_v2_error>
decode_interpretation_v2(std::string_view encoded, std::string_view versions_encoded, std::span<const tree_file> raw_files);
/// @brief Derive exact versions and extensions from raw verified provenance and structurally corresponding B/S.
/// @param bundle Complete normalized bundle, stable throughout this call.
/// @param semantic Complete recorded semantic model, stable throughout this call.
/// @param raw_files Complete effective tree, independently verified internally.
/// @return Alias-severed canonical data after the same structural and raw-tree checks as decoding.
/// @note Pure and thread-safe; structural B/S correspondence does not prove same-VM semantic execution.
std::expected<recorded_interpretation_v2, interpretation_v2_error>
encode_interpretation_v2(const value_ptr& bundle, const value_ptr& semantic, std::span<const tree_file> raw_files);
} // namespace planar::engine::history
