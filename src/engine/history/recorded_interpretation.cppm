/// @file
/// @brief Pure bounded historical interpretation dispatch and raw provenance integrity without storage or Lua.
export module planar.engine.history.recorded_interpretation;
import std;
export import planar.engine.history.interpretation_codec;
export import planar.engine.history.interpretation_v2_codec;
export namespace planar::engine::history {
/// Non-authoritative family hint selecting fixed embedded or effective reconstruction bounds.
enum class recorded_family { embedded, effective };
/// Closed historical refusal; unsupported interpretation is a separate verified-integrity result.
enum class recorded_error_code { malformed, unsupported_manifest, resource_limit, invalid_tree, invalid_correspondence };
/// Redacted pure refusal, retaining no supplied bytes or host state.
struct recorded_error {
  recorded_error_code code; ///< Canonical, provenance or correspondence category.
};
/// A supported interpretation or safely unsupported semantics after independent provenance verification.
enum class recorded_status { decoded, unsupported_interpretation };
/// Owned supported payload common to the fixed v1 and v2 envelopes; ordinary data without acceptance authority.
struct decoded_recorded_interpretation {
  value_ptr   interpretation;   ///< Complete alias-severed supported envelope.
  value_ptr   versions;         ///< Separate versions equal to embedded versions.
  std::string encoded;          ///< Exact canonical interpretation bytes.
  std::string versions_encoded; ///< Exact canonical versions bytes.
};
/// Verified structural integrity, without same-VM, callability or semantic-honesty authority.
struct verified_recorded_interpretation {
  recorded_family                                family;  ///< Internally repeated family classification.
  recorded_status                                status;  ///< Whether fixed semantic decoding succeeded.
  record_digest                                  digest;  ///< Independently recomputed complete raw-tree identity.
  std::optional<decoded_recorded_interpretation> decoded; ///< Present exactly for supported interpretation.
};
/// @brief Select fixed provenance reconstruction bounds from bounded canonical recorded markers.
/// @param encoded Interpretation bytes, at most 16 MiB, decoded first after both size preflights.
/// @param versions_encoded Separate versions bytes, at most 16 MiB.
/// @return Effective for extensions or provenance/registration field presence; otherwise embedded, or canonical refusal.
/// @note Numeric version alone never selects a family. This hint verifies no envelope or tree and is never authority.
std::expected<recorded_family, recorded_error> classify_recorded_interpretation(std::string_view encoded,
                                                                                std::string_view versions_encoded);
/// @brief Verify raw provenance, fixed historical syntax and workspace/digest correspondence without current policy.
/// @param encoded Complete canonical interpretation bytes, at most 16 MiB.
/// @param versions_encoded Complete separate versions bytes, at most 16 MiB.
/// @param raw_files Complete reconstructed raw tree, stable during the call; no caller digest or profile is accepted.
/// @param workspace Expected recorded workspace identifier, compared exactly even for unsupported interpretation.
/// @return Owned decoded data, or unsupported status without semantic payload, or typed refusal.
/// @note Thread-safe and alias-severing. Order: classification; tree bounds/topology; provenance; fixed decoder;
/// workspace/digest. Unknown manifest refuses before interpretation decoding; future interpretation still verifies provenance.
std::expected<verified_recorded_interpretation, recorded_error>
verify_recorded_interpretation(std::string_view encoded, std::string_view versions_encoded, std::span<const tree_file> raw_files,
                               std::string_view workspace);
} // namespace planar::engine::history
