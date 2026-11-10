/// @file
/// @brief CLI11 bindings for release; native intents retain secrets and private paths.
export module planar.cmd.execution.release;
import std;
import cli11;
export import planar.cmd.response;
export import planar.cmd.execution_options;
export import planar.engine.runtime;
export namespace planar::cmd::handlers {
/// Invocation-local CLI state shared by embedded and closed-profile dispatch.
using ::planar::cmd::execution_options::release_options;
/// Attach this family to an invocation-local parser.
/// @param root Invocation-local CLI11 parser.
/// @param options Storage that outlives parsing and dispatch.
/// @return Attached command with the exact option grammar.
CLI::App& attach_release(CLI::App& root, release_options& options);
/// Execute the embedded family.
/// @param options Parsed command intent.
/// @param vm Optional registered VM; null creates a fresh embedded invocation.
/// @return Buffered success or typed failure response.
response release_command(const release_options& options, engine::command_vm* vm = nullptr);
/// Execute the selected profile family.
/// @param options Parsed command intent.
/// @param selector External profile selector.
/// @return Buffered success or typed failure response; no direct process output.
response release_with_profile(const release_options& options, std::string_view selector);
} // namespace planar::cmd::handlers
