/// @file
/// @brief CLI11 bindings for the status execution read; failures are buffered command responses.
export module planar.cmd.execution.status;
import std;
import cli11;
export import planar.cmd.response;
export import planar.cmd.execution_options;
import planar.engine.runtime;
export namespace planar::cmd::handlers {
/// Invocation-local option state, retained through dispatch.
using ::planar::cmd::execution_options::status_options;
/// Attach the command to an invocation-local parser.
/// @param root Invocation-local CLI11 parser.
/// @param options Storage that outlives parsing and dispatch.
/// @return Attached command with the exact option grammar.
CLI::App& attach_status(CLI::App& root, status_options& options);
/// Execute the embedded command.
/// @param options Parsed command intent.
/// @param vm Optional registered VM; null creates a fresh embedded invocation.
/// @return Buffered success or typed failure response.
response status_command(const status_options& options, engine::command_vm* vm = nullptr);
/// Execute under the selected external profile.
/// @param options Parsed command intent.
/// @param selector External profile selector.
/// @return Buffered success or typed failure response; no direct process output.
response status_with_profile(const status_options& options, std::string_view selector);
} // namespace planar::cmd::handlers
