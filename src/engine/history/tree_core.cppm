/// @file tree_core.cppm
/// @brief Storage-independent bounded tree construction, hashing and reconstruction with per-call typed callbacks.
export module planar.engine.history.tree_core;
import std;
export import planar.engine.ledger.types;
export namespace planar::engine::history {
/// Fixed independent tree budgets; callers cannot raise these ceilings.
enum class tree_profile {
  source,              ///< Original 64 MiB, 4096 files and 8192 directories.
  provenance,          ///< Original 16 MiB, 4096 files and 1 + 4096 * 63 directories.
  effective_provenance ///< Effective closure: 32 MiB, 8192 files and 1 + 8192 * 63 directories.
};
/// Tree-local invalid topology, resource bounds or original typed storage failure.
enum class tree_error_code { invalid_tree, resource_limit, storage };
/// Redacted tree refusal; storage failures retain the owning ledger vocabulary.
struct tree_error {
  tree_error_code                            code;    ///< Tree boundary category.
  std::optional<ledger::types::format_error> storage; ///< Present exactly for storage failures.
};
/// Sealed one-root logical input; lifetime and stable captured bytes belong to the caller.
struct tree_file_view {
  std::string_view path;       ///< Portable relative file path, at most 1024 bytes and 64 segments.
  std::string_view payload;    ///< Exact arbitrary bytes, individually at most 16 MiB.
  std::uint32_t    mode = 420; ///< Normalized 0644 or 0755 only.
};
/// Owned complete reconstruction entry; directories are implicit file ancestors.
struct tree_file {
  std::string   path;    ///< Original logical spelling, sorted by unsigned ASCII bytes.
  std::string   payload; ///< Owned exact bytes, independent of the reader lifetime.
  std::uint32_t mode;    ///< Exact normalized mode.
};
/// @brief Compute the same portable root as build_tree without storage or a transaction.
/// @param files Sealed captured views, validated with the shared tree construction.
/// @param profile Fixed independent source, provenance or effective provenance budgets.
/// @return Portable digest or tree refusal; local placeholder IDs never enter the digest.
std::expected<record_digest, tree_error> digest_tree(std::span<const tree_file_view> files, tree_profile profile);
/// Per-call synchronous reader of codec-validated records; ownership and failure cleanup belong to the caller.
using tree_record_reader = std::function<std::expected<ledger::types::stored_record, ledger::types::format_error>(std::uint64_t)>;
/// Per-call synchronous interner returning the exact value identity; no implicit commit or transaction ownership.
using tree_record_intern =
    std::function<std::expected<ledger::types::record_reference, ledger::types::format_error>(const record&)>;
/// @brief Build through caller-owned storage using shared validation and exact typed failures.
/// @param files Sealed input views, alive for the complete call.
/// @param profile Fixed independent tree bounds.
/// @param intern Synchronous per-call interner, called only after structural input validation.
/// @return Complete root or tree/storage refusal; the caller owns cleanup on failure or exception.
std::expected<ledger::types::record_reference, tree_error>
build_tree_core(std::span<const tree_file_view> files, tree_profile profile, const tree_record_intern& intern);
/// @brief Reconstruct through a synchronous per-call reader without retaining its state.
/// @param root Exact directory identity.
/// @param profile Fixed independent bounds, counting every expanded occurrence.
/// @param reader Caller-owned reader; typed storage errors propagate unchanged.
/// @return Complete sorted files or refusal; the caller owns cleanup on failure or exception.
/// @note Records must already satisfy codec invariants, including variant/kind agreement. All algorithm state is call-local;
/// thread safety of callbacks and their owners belongs to the caller.
std::expected<std::vector<tree_file>, tree_error> read_tree_core(ledger::types::record_reference root, tree_profile profile,
                                                                 const tree_record_reader& reader);
} // namespace planar::engine::history
