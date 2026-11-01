/// @file
/// @brief Descriptor-confined read of one observation directory for the embedded authoring `observe` verb.
/// Entry point: read_observation_directory. Errors are typed redacted diagnostics; nothing throws by contract.
/// The test-only recheck hook lives in the internal partition `:recheck`, never in this exported interface.
export module planar.engine.capture.directory;
import std;
export import planar.engine.value;
import :recheck;
export namespace planar::engine {
/// One regular file listed from an observation directory (defined by the internal partition, re-exported here).
using ::planar::engine::observation_entry;
/// Typed result of one observation directory read (defined by the internal partition, re-exported here).
using ::planar::engine::observation_directory;
/// @brief Read an observation directory once and verify it did not change during the read.
/// @param path Absolute canonical directory path from the path stage.
/// @return Sorted entries, or a typed redacted refusal: `capture_changed`, `path_denied`, `resource_limit` or `io`.
/// @note Thread-confined per call.
std::expected<observation_directory, diagnostic> read_observation_directory(std::string_view path);
} // namespace planar::engine
