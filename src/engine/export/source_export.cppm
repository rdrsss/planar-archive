/// @file source_export.cppm
/// @brief Exact recorded source export into a new directory, with typed redacted failures and no-replace publication.
export module planar.engine.source_export;
import std;
export import planar.engine.ledger.types;
export namespace planar::engine::source_export {
/// Native export failure categories; storage retains the original ledger category.
enum class error_code {
  storage,
  revision_not_found,
  invalid_argument,
  path_denied,
  target_exists,
  io,
  publication_uncertain,
  cleanup_incomplete
};
/// Redacted export refusal, never containing source bytes or caller-supplied paths.
struct export_error {
  error_code                                 code;    ///< Export category.
  std::optional<ledger::types::format_error> storage; ///< Present for an underlying ledger failure only.
  std::string location; ///< Only cleanup_incomplete may contain the invocation-generated sibling stage basename.
};
/// Exact exported revision identity, with native signed-64 IDs.
struct export_result {
  std::string  ledger;   ///< Verified ledger identity.
  std::int64_t revision; ///< Selected chronological revision.
  std::int64_t source;   ///< Recorded complete source root ID.
};
/// @brief Verify one pinned source snapshot completely, then publish its exact bytes and normalized modes into a new target.
/// @param ledger_path Explicit existing ledger file; no default discovery or mutation.
/// @param revision Positive signed-64 revision ID.
/// @param target Explicit new directory in an existing real parent; original components must traverse without symlinks.
/// @return Identity after publication and sync, or a typed refusal. Uncertain publication requires inspecting the explicit target.
/// @note Call-local and thread confined. Cooperative directory ownership is required. Cleanup never deletes a published target;
/// crash leftovers are never adopted. Unsupported safe rename capability refuses without an overwrite fallback.
std::expected<export_result, export_error> export_source(const std::filesystem::path& ledger_path, std::int64_t revision,
                                                         const std::filesystem::path& target);
} // namespace planar::engine::source_export
