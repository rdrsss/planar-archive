/// @file
/// @brief CLI11 claim bindings and buffered execution dispatch.
export module planar.cmd.execution.claim;
import std;
import cli11;
export import planar.cmd.response;
export import planar.engine.runtime;
export namespace planar::cmd::handlers {
/// Invocation-local native intent; retained through parsing and dispatch.
using claim_options = engine::command_run_options;
/// Attach required run acquisition options.
/// @param root Invocation-local CLI11 parser.
/// @param options Storage that outlives parsing and dispatch.
/// @return Attached command with the exact option grammar.
CLI::App& attach_claim(CLI::App& root, claim_options& options);
/// Invoke the embedded execution command.
/// @param options Parsed command intent.
/// @param vm Optional registered VM; null creates a fresh embedded invocation.
/// @return Buffered success or typed failure response.
response claim_command(const claim_options& options, engine::command_vm* vm = nullptr);
/// Invoke a selected profile.
/// @param options Parsed command intent.
/// @param selector External profile selector.
/// @return Buffered success or typed failure response; no direct process output.
response claim_with_profile(const claim_options& options, std::string_view selector);
} // namespace planar::cmd::handlers
