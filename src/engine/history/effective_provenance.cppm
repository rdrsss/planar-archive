/// @file
/// @brief Pure bounded effective stdlib assembly and structural historical verification without Lua, storage or host access.
export module planar.engine.history.effective_provenance;
import std;
export import planar.engine.history.provenance_codec;
export namespace planar::engine::history {
/// Closed structural refusal; unsupported old embedded manifests map to invalid_provenance.
enum class effective_error_code {
  invalid_provenance,
  resource_limit,
  invalid_tree,
  invalid_profile,
  invalid_catalog,
  identity_conflict
};
/// Redacted effective provenance failure, retaining no supplied bytes or host paths.
struct effective_error {
  effective_error_code code; ///< Component or resource category only.
};
/// Explicit captured inputs; views remain alive and unchanged throughout preparation.
struct effective_input {
  std::span<const tree_file>
                   embedded;     ///< Complete old embedded provenance, with old synthetic names; claimed digests are absent.
  std::string_view profile;      ///< Original JSON bytes, including whitespace.
  std::uint32_t    profile_mode; ///< Captured normalized 0644 or 0755 mode.
  std::span<const tree_file_view> modules; ///< Unprefixed profile-listed paths in exact profile order, with captured bytes/modes.
  std::string_view                catalog; ///< Complete canonical_v1 catalog-v2 bytes, independently revalidated.
};
/// @brief Revalidate components and assemble an owned effective_stdlib_v1 tree and recomputed portable digest.
/// @param input Explicit historical bytes; no current runtime identity or callable semantics are consulted.
/// @return Complete sorted owned files or bounded structural refusal. All state is call-local and no aliases survive.
std::expected<prepared_provenance, effective_error> prepare_effective_stdlib(const effective_input& input);
/// @brief Verify complete reconstructed effective provenance and sever all caller aliases.
/// @param files Exact full tree membership, including the canonical combined manifest; input stays stable during the call.
/// @return Owned verified tree and recomputed digest. Expected stored digest comparison belongs to the caller's storage boundary.
std::expected<prepared_provenance, effective_error> verify_effective_stdlib(std::span<const tree_file> files);
} // namespace planar::engine::history
