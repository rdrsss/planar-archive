/// @file
/// @brief CLI11 check-recover intent binding with buffered command dispatch; options remain alive through parsing.
export module planar.cmd.execution.check_recover;
import std;
import cli11;
export import planar.cmd.response;
export import planar.cmd.execution_options;
export import planar.engine.runtime;
export namespace planar::cmd::handlers {
/// Private native host paths and scalar options for one invocation.
using ::planar::cmd::execution_options::check_recover_options;
/// @brief Attach the command and its exact required/optional options.
/// @param root Parent CLI11 parser.
/// @param options Storage retained until parse and dispatch finish.
/// @return Attached command.
CLI::App& attach_check_recover(CLI::App& root, check_recover_options& options);
/// @brief Invoke the retained embedded command.
/// @param options Parsed CLI11 intent.
/// @param vm Optional registered command VM.
/// @return Buffered machine response.
response check_recover_command(const check_recover_options& options, engine::command_vm* vm = nullptr);
/// @brief Invoke the command with a selected external profile.
/// @param options Parsed CLI11 intent.
/// @param selector Profile path.
/// @return Buffered machine response.
response check_recover_with_profile(const check_recover_options& options, std::string_view selector);
} // namespace planar::cmd::handlers
