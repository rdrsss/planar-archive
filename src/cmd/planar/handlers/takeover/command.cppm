/// @file
/// @brief CLI11 takeover bindings and buffered execution dispatch.
export module planar.cmd.execution.takeover;
import std;
import cli11;
export import planar.cmd.response;
export import planar.cmd.execution_options;
export import planar.engine.runtime;
export namespace planar::cmd::handlers {
/// Invocation-local CLI state shared by embedded and closed-profile dispatch.
using ::planar::cmd::execution_options::takeover_options;
/// Attach required run acquisition options.
/// @param root Invocation-local CLI11 parser.
/// @param options Storage that outlives parsing and dispatch.
/// @return Attached command with the exact option grammar.
CLI::App& attach_takeover(CLI::App& root, takeover_options& options);
/// Invoke the embedded execution command.
/// @param options Parsed command intent.
/// @param vm Optional registered VM; null creates a fresh embedded invocation.
/// @return Buffered success or typed failure response.
response takeover_command(const takeover_options& options, engine::command_vm* vm = nullptr);
/// Invoke a selected profile.
/// @param options Parsed command intent.
/// @param selector External profile selector.
/// @return Buffered success or typed failure response; no direct process output.
response takeover_with_profile(const takeover_options& options, std::string_view selector);
} // namespace planar::cmd::handlers
