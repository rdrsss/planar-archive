/// @file
/// @brief Retained command execution and finite native capture/storage adapters with buffered failure boundaries.
export module planar.cmd.invocation;
import std;
export import planar.cmd.response;
export import planar.engine.runtime;
import planar.engine.source_export;
export namespace planar::cmd {
/// Invoke the retained callable with invocation-local native services and encode its checked response.
/// @param options Typed CLI11 intent; host spellings remain native.
/// @param registered Optional already validated invocation-local registry, retained by the caller.
/// @return Complete buffered response; export publication uncertainty survives every later failure.
/// @note Independent calls share no mutable state. No Lua callback executes under a native write lock.
response execute(engine::command_options options, engine::command_vm* registered = nullptr);
/// Prepare all selected private paths after catalog admission and before native service creation.
/// @param options Invocation-local typed options; paths become absolute with parent spelling preserved.
/// @param cwd_reader Read cwd once only when at least one selected path is relative; failures stay redacted.
/// @param workspace_directory_reader Validate and own PLANAR_WORKSPACE_DIR once only for a valid bare workspace name.
/// @return Success or typed redacted usage/path/resource/io failure. Readers are synchronous and not retained.
/// This preparation has no shared state and performs no path existence checks.
std::expected<void, engine::diagnostic>
prepare_invocation_paths(engine::command_options&                                               options,
                         const std::function<std::expected<std::string, engine::diagnostic>()>& cwd_reader,
                         const std::function<std::expected<std::string, engine::diagnostic>()>& workspace_directory_reader);
/// Adapt a native export failure without exposing private strings.
/// @param error Typed native failure.
/// @return Buffered transport error with the cleanup exit policy.
response native_export_failure(const engine::source_export::export_error& error);
} // namespace planar::cmd
