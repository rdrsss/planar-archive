/// @file provenance_codec.cppm
/// @brief Pure bounded registry/stdlib provenance preparation and historical verification without embedded code, Lua or storage.
export module planar.engine.history.provenance_codec;
import std;
export import planar.engine.history.tree_core;
export namespace planar::engine::history {
/// Provenance-specific envelope, scheme or shared tree-boundary refusal.
enum class provenance_error_code { invalid_provenance, unsupported_manifest, resource_limit, invalid_tree };
/// Redacted preparation refusal, never retaining caller bytes.
struct provenance_error {
  provenance_error_code code; ///< Fixed native envelope or tree boundary category.
};
/// Ordered original source views, distinct from synthetic assembly files.
struct original_file_view {
  std::string_view path;  ///< Portable stdlib-relative original path.
  std::string_view bytes; ///< Exact captured original bytes.
};
/// Owned verified complete sorted files and portable identity; no database-local references.
struct prepared_provenance {
  std::vector<tree_file> files;  ///< Complete original and synthetic files, owning exact bytes.
  record_digest          digest; ///< Shared pure tree digest of these files.
};
/// @brief Prepare exactly input.json from already captured registry bytes and normalized mode.
/// @param bytes Captured registry bytes; no parsing or recapture occurs.
/// @param mode Captured normalized 0644 or 0755 mode.
/// @return Owned bounded registry provenance or refusal.
std::expected<prepared_provenance, provenance_error> prepare_registry(std::string_view bytes, std::uint32_t mode);
/// @brief Verify ordered concatenation and prepare originals with two explicit synthetic files.
/// @param originals Ordered captured originals; reserved namespace and aliases are refused.
/// @param separator Exact bytes appended after every original, including the last.
/// @param assembly Exact externally supplied assembled chunk, checked before copying.
/// @return Complete provenance with canonical_v1 manifest, or bounded refusal.
std::expected<prepared_provenance, provenance_error> prepare_stdlib(std::span<const original_file_view> originals,
                                                                    std::string_view separator, std::string_view assembly);
/// @brief Decode and verify a complete reconstructed stdlib set without running Lua.
/// @param files Complete owned reconstructed set; all originals and synthetics require 0644.
/// @return Verified owned sorted set and portable digest, or refusal.
std::expected<prepared_provenance, provenance_error> verify_stdlib(std::span<const tree_file> files);
} // namespace planar::engine::history
