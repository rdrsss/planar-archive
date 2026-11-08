/// @file
/// @brief Draft comparison of `status --workspace --registry` (execution-contract P2-1, draft decision DD-D6): the logical
/// portable paths of a captured declaration closure that differ from, are absent from, or are added to the active
/// revision's recorded source tree.
///
/// Entry point is `changed_paths`. A path is changed when it is present on one side only, or when its bytes or its
/// normalized mode differ. The result is sorted by unsigned bytes and holds each path once. The function is pure: no
/// I/O, no Lua and no shared state; it cannot fail, because both inputs are already verified complete trees.
export module planar.engine.execution.drafts;

import std;
export import planar.engine.history.tree_core;

export namespace planar::engine::execution {

/// @brief Compare a captured declaration closure with a recorded source tree.
/// @param captured Workspace-root files of the sealed draft capture, in any order; paths are unique.
/// @param recorded Files of the active revision's source tree, in any order; paths are unique.
/// @return Sorted unique portable paths that differ, are absent from `captured`, or are added in `captured`.
std::vector<std::string> changed_paths(std::span<const history::tree_file> captured,
                                       std::span<const history::tree_file> recorded);

} // namespace planar::engine::execution
