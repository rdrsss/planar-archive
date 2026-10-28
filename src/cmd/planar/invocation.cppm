/// @file
/// @brief Retained command execution and finite native capture/storage adapters with buffered failure boundaries.
export module planar.cmd.invocation;
import std;
export import planar.cmd.response;
export import planar.engine.runtime;
import planar.engine.source_export;
import planar.engine.acceptance.intent;
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
/// The invocation's single cwd and workspace-environment snapshot; filled once, shared by every later slot.
struct invocation_snapshot {
  std::optional<std::string> cwd;                 ///< Cwd read at most once; absent when no path needed it.
  std::optional<std::string> workspace_directory; ///< PLANAR_WORKSPACE_DIR read at most once; absent when unused.
};
/// Prepare a with-profile invocation: normalize every known slot and the profile selector against one snapshot.
/// @param selector Raw profile selector; normalized lexically to an absolute path without requiring existence.
/// @param options Invocation-local typed options; paths become absolute exactly as prepare_invocation_paths does.
/// @param snapshot Receives the cwd and workspace-environment values actually read, once each.
/// @param cwd_reader Read cwd at most once per call; failures stay redacted.
/// @param workspace_directory_reader Read the workspace environment at most once per call.
/// @return The normalized selector, or a typed redacted usage/path/resource/io failure. No existence checks occur.
std::expected<std::string, engine::diagnostic>
prepare_profile_invocation(std::string_view selector, engine::command_options& options, invocation_snapshot& snapshot,
                           const std::function<std::expected<std::string, engine::diagnostic>()>& cwd_reader,
                           const std::function<std::expected<std::string, engine::diagnostic>()>& workspace_directory_reader);
/// Normalize a profile selector for selected help or an extension tail against the process cwd, read at most once.
/// @param selector Raw selector; normalized lexically to an absolute path without requiring existence.
/// @param snapshot Receives the cwd when the selector is relative.
/// @return The normalized selector, or a typed redacted path/resource/io failure.
std::expected<std::string, engine::diagnostic> prepare_process_profile_selector(std::string_view     selector,
                                                                                invocation_snapshot& snapshot);
/// Complete the invocation snapshot for an extension dispatch: the cwd and, when set and usable, the workspace environment.
/// @param snapshot Snapshot that already holds whatever earlier steps read; nothing is read twice.
/// @return Success, or a redacted io failure when the cwd cannot be obtained.
std::expected<void, engine::diagnostic> complete_process_snapshot(invocation_snapshot& snapshot);
/// Execute a with-profile load: early saved-request lookup, closed profile factory, prepare and commit, buffered response.
/// @param intent Codec2 load intent carrying the normalized profile selector; bound once and reused unchanged.
/// @param ledger_path Absolute normalized ledger selection, native only.
/// @return Complete buffered response; completion uncertainty after commit survives every later failure.
/// @note The early lookup, the closed factory, prepare_load and complete_acceptance all run here (see invocation.cpp, the
/// after-early-miss, after-factory-open, after-prepare-load and after-commit anchors); a failure after commit keeps the
/// completion-uncertain classification.
response execute_profile_load(const engine::acceptance::request_intent& intent, std::string_view ledger_path);
/// Execute one shipped family under a selected profile: normalize against one snapshot, open the closed owner and dispatch.
/// @param options Typed CLI11 intent exactly as the family handler produced it; host spellings become absolute.
/// @param profile_selector Raw selector from the envelope; normalized lexically without requiring existence.
/// @return Complete buffered response; render, init, history and export dispatch the admitted options once, load frames
/// its request intent here and takes the load path.
response execute_with_profile(engine::command_options options, std::string_view profile_selector);
/// Adapt a native export failure without exposing private strings.
/// @param error Typed native failure.
/// @return Buffered transport error with the cleanup exit policy.
response native_export_failure(const engine::source_export::export_error& error);
} // namespace planar::cmd
