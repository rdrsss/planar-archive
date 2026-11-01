/// @file directory_recheck.cppm
/// @brief Internal partition: the observation directory result types and the read body with a recheck hook for tests.
///
/// This is a non-exported interface partition of the directory module; the primary interface imports it without
/// re-exporting and re-exports only the two result types. The hooked entry sits in the nested namespace recheck_detail,
/// which holds no exported entity, so only in-tree units that declare themselves part of the module (the directory
/// implementation and its test unit) can name it. The exported read_observation_directory calls it with an empty hook.
/// Thread-confined per call.
export module planar.engine.capture.directory:recheck;
import std;
export import planar.engine.value;
namespace planar::engine {
/// One regular file listed from an observation directory.
export struct observation_entry {
  std::string   category; ///< Closed category assigned from the entry name.
  std::string   name;     ///< Entry name inside the directory.
  std::string   bytes;    ///< Exact bytes read through the no-follow descriptor.
  std::uint64_t size;     ///< Byte count of the entry.
  std::uint64_t device;   ///< Device identity observed for the entry.
  std::uint64_t inode;    ///< Inode identity observed for the entry.
};
/// Typed result of one observation directory read.
export struct observation_directory {
  std::vector<observation_entry> entries; ///< Entries sorted by name in unsigned byte order.
};
namespace recheck_detail {
/// @brief Read an observation directory, calling the hook after listing and before the final recheck.
/// @param path Absolute canonical directory path.
/// @param before_recheck Test instrumentation called once between the read and the verification; may be empty.
/// @return Sorted entries, or a typed redacted diagnostic.
std::expected<observation_directory, diagnostic> read_observation_directory_impl(std::string_view      path,
                                                                                 std::function<void()> before_recheck);
} // namespace recheck_detail
} // namespace planar::engine
