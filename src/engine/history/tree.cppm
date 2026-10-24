/// @file tree.cppm
/// @brief Complete bounded immutable trees over captured logical views; typed refusals and caller-owned transactions.
export module planar.engine.history.tree;
import std;
export import planar.engine.ledger.format;
export import planar.engine.history.tree_core;
export namespace planar::engine::history {
/// @brief Build a complete immutable tree without filesystem access or hidden commit.
/// @param writer Caller-owned transaction, preserved on success, aborted on every failure or exception, including prior inserts.
/// @param files Sealed captured file views from one logical root; duplicates and aliases are errors.
/// @param profile Source (64 MiB, 4096 files, 8192 directories) or provenance (16 MiB, 4096 files).
/// @return Directory root reference or typed refusal. Root is depth zero and counts as a directory; files count toward depth 64.
/// @note Only root and file ancestors exist. Provenance directories are bounded by 1 + 4096 * 63. All owners are thread confined.
std::expected<ledger::record_reference, tree_error> build_tree(ledger::record_writer&          writer,
                                                               std::span<const tree_file_view> files, tree_profile profile);
/// @brief Reconstruct complete sorted owned files from one pinned verified snapshot.
/// @param reader Existing thread-confined pinned reader; its own sticky storage refusal semantics remain intact.
/// @param root Exact directory reference, including matching kind and digest.
/// @param profile Fixed independent source or provenance bounds; expanded shared occurrences count separately.
/// @return Complete files or typed failure, never successful partial output; non-root empty directories are refused.
std::expected<std::vector<tree_file>, tree_error> read_tree(const ledger::format_file& reader, ledger::record_reference root,
                                                            tree_profile profile);
/// @brief Reconstruct complete sorted owned files including pending records without committing.
/// @param writer Caller-owned writer, aborted on every failure or exception, including prior inserts.
/// @param root Exact directory reference, including matching kind and digest.
/// @param profile Fixed independent bounds, counting expanded shared occurrences.
/// @return Complete files or typed failure. No filesystem capture or Lua replay occurs.
std::expected<std::vector<tree_file>, tree_error> read_tree(ledger::record_writer& writer, ledger::record_reference root,
                                                            tree_profile profile);
} // namespace planar::engine::history
