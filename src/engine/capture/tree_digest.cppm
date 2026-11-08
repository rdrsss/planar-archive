/// @file
/// @brief Descriptor-relative bounded digest-only tree capture and sorted tree differences; failures are redacted diagnostics.
export module planar.engine.capture.tree_digest;
import std;
export import planar.engine.value;
export namespace planar::engine::capture {
using capture_error = diagnostic;
/// Capture policy. Calls are thread-confined; descriptors are borrowed.
struct tree_digest_spec {
  std::vector<std::string> excluded, retain_paths;
  std::uint64_t            max_hashed_bytes = 1073741824, max_files = 100000, max_depth = 64;
  std::uint64_t            max_retained_file_bytes = 1048576;
};
/// Exact file identity, independent of modification time.
struct tree_digest_entry {
  std::string               path, mode;
  std::uint64_t             size;
  std::array<std::byte, 16> digest;
  bool                      operator==(const tree_digest_entry&) const = default;
};
struct tree_digest {
  std::vector<tree_digest_entry>     entries;
  std::map<std::string, std::string> retained;
};
struct tree_change {
  std::string path;
  enum class kind { added, deleted, modified, mode } change;
};
/// @brief Capture without following links, refusing aliases and unstable files.
/// @param dir_fd Borrowed open root directory.
/// @param spec Bounds and portable exclusions.
/// @return Sorted digests and requested bytes, or a typed diagnostic.
std::expected<tree_digest, capture_error> capture_tree_digest(int dir_fd, const tree_digest_spec& spec);
/// @brief Compare sorted captured trees; renames appear as deletion and addition.
/// @param base Original tree.
/// @param current Current tree.
/// @return Changes sorted by portable path.
std::vector<tree_change> diff_trees(const tree_digest& base, const tree_digest& current);
/// @brief Format a digest with its canonical algorithm prefix.
/// @param digest Canonical big-endian digest bytes.
/// @return xxh3_128 text.
std::string digest_text(const std::array<std::byte, 16>& digest);
} // namespace planar::engine::capture
