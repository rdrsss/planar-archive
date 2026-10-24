/// @file
/// @brief Explicit host input resolution and pure request-path normalization with typed, redacted failures.
export module planar.engine.capture.host_path;
import std;
export import planar.engine.value;
export namespace planar::engine {
/// Resolve an existing regular input file, preserving physical parent traversal and refusing a leaf symlink.
/// @param argument Original host path spelling; relative paths use the invocation cwd.
/// @return Canonical parent plus original leaf, or the existing redacted path/io diagnostic.
/// Filesystem failures are sanitized; allocation failures propagate. Cooperative resolution is not a race guarantee.
std::expected<std::string, diagnostic> resolve_explicit_file(std::string_view argument);
/// Resolve an existing directory with physical component traversal and refuse final-root symlink aliases.
/// @param argument Original host path spelling; relative paths use the invocation cwd.
/// @return Canonical directory or the existing redacted path/io diagnostic.
/// Filesystem failures are sanitized; allocation failures propagate. Intermediate links retain OS traversal semantics.
std::expected<std::string, diagnostic> resolve_explicit_directory(std::string_view argument);
/// Normalize request intent lexically without filesystem reads, existence checks or implicit cwd lookup.
/// @param argument Nonempty non-NUL UTF-8 absolute or relative path.
/// @param explicit_cwd Nonempty non-NUL UTF-8 absolute, lexically normalized cwd, even for absolute arguments.
/// @return Absolute lexical spelling or redacted path/resource diagnostic; allocation failures propagate.
/// Input strings, joined spelling and output are bounded to 64 MiB. Callers must use the result for both
/// request identity and capture; the request factory separately bounds the entire encoded record.
/// This function has no shared state and is safe for concurrent calls.
std::expected<std::string, diagnostic> normalize_intent_path(std::string_view argument, std::string_view explicit_cwd);
/// Native private path pair; neither spelling may enter declaration or public data.
struct prepared_host_path {
  std::string spelling; ///< Absolute host spelling retaining physical parent traversal.
  std::string intent;   ///< Absolute lexical-normal target for future request framing.
};
/// Classify a selector without filesystem or environment access.
/// @param selector Original CLI11 workspace value.
/// @return True for a bare name, false for an explicit path, or redacted usage.
std::expected<bool, diagnostic> workspace_is_name(std::string_view selector);
/// Validate native path text and its 64 MiB input bound without filesystem access.
/// @param argument Original path spelling.
/// @return Success or redacted path/resource diagnostic; text validation precedes size.
std::expected<void, diagnostic> validate_host_path(std::string_view argument);
/// Validate a named selector's supplied environment without allocation on success.
/// @param directory Caller-snapshotted workspace directory, relevant only for a bare name.
/// @return Existing usage/path_bytes refusal or success; performs no normalization or copying.
std::expected<void, diagnostic> validate_workspace_directory(std::optional<std::string_view> directory);
/// Validate path and relevant cwd text, size and absolute spelling before normalization allocation.
/// @param argument Supplied host path; absolute arguments ignore cwd completely.
/// @param explicit_cwd Caller-snapshotted cwd for relative arguments.
/// @return Existing text/size refusal or success; lexical cwd validity and joined-size checks remain in prepare_host_path.
std::expected<void, diagnostic> validate_host_path_inputs(std::string_view                argument,
                                                          std::optional<std::string_view> explicit_cwd = {});
/// Validate a named selector's environment value before copying it into native ownership.
/// @param directory Explicit environment view read once by the caller, absent if unset.
/// @return Owned snapshot or redacted usage/resource diagnostic; no allocation precedes validation.
std::expected<std::string, diagnostic> snapshot_workspace_directory(std::optional<std::string_view> directory);
/// Expand a selector using only a caller-snapshotted environment value, with no filesystem access.
/// @param selector Original workspace selector.
/// @param directory Explicit PLANAR_WORKSPACE_DIR snapshot; ignored for explicit files.
/// @return Expanded path spelling or redacted usage/path/resource diagnostic.
std::expected<std::string, diagnostic> expand_workspace(std::string_view selector, std::optional<std::string_view> directory);
/// Prepare private absolute spelling and lexical intent without consulting ambient cwd or filesystem.
/// @param argument Valid absolute or relative host spelling.
/// @param explicit_cwd Required lexical-normal absolute cwd only for relative arguments; ignored otherwise.
/// @return Bounded native path pair or redacted diagnostic. Thread-safe, with no shared state.
std::expected<prepared_host_path, diagnostic> prepare_host_path(std::string_view                argument,
                                                                std::optional<std::string_view> explicit_cwd = {});
} // namespace planar::engine
