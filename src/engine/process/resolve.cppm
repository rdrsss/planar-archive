/// @file
/// @brief Executable resolution for check commands against an explicit environment (R17-2 rule 1).
///
/// Entry point: resolve_executable. A command0 containing a slash must be an absolute path naming a regular
/// file that the effective identity may execute; otherwise the PATH value of the supplied environment (never
/// the host environment) is searched. Every failure, including filesystem errors, is the single `missing`
/// outcome; allocation failures propagate as exceptions. The module holds no state, so calls are thread-safe.
export module planar.engine.process.resolve;
import std;
export namespace planar::engine::process {
/// Resolution outcome; `missing` is the only failure the resolver reports.
enum class resolve_outcome { found, missing };
/// Resolve the executable for a command against an explicit environment.
/// @param command0 First command element; an absolute path when it contains `/`, else a name searched in PATH.
/// @param environment Exact environment of the check, sorted by name and unique; only its `PATH` entry is read.
/// @return The absolute path to pass to posix_spawn (symlinks are not resolved), or `resolve_outcome::missing`.
std::expected<std::string, resolve_outcome>
resolve_executable(std::string_view command0, const std::vector<std::pair<std::string, std::string>>& environment);
} // namespace planar::engine::process
