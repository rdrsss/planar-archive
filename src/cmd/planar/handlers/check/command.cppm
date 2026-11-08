/// @file
/// @brief CLI11 check intent binding with buffered command dispatch; options remain alive through parsing.
export module planar.cmd.execution.check;
import std;
import cli11;
export import planar.cmd.response;
export import planar.engine.runtime;
export namespace planar::cmd::handlers {
/// Private native host paths and scalar options for one invocation.
struct check_options {
  std::string                ledger;
  std::string                run;
  std::string                root;
  std::string                base_root;
  std::string                scratch;
  std::string                request_id;
  std::string                actor;
  std::optional<std::string> provenance;
};
/// @brief Attach the command and its exact required/optional options.
/// @param root Parent CLI11 parser.
/// @param options Storage retained until parse and dispatch finish.
/// @return Attached command.
CLI::App& attach_check(CLI::App& root, check_options& options);
/// @brief Invoke the retained embedded command.
/// @param options Parsed CLI11 intent.
/// @param vm Optional registered command VM.
/// @return Buffered machine response.
response check_command(const check_options& options, engine::command_vm* vm = nullptr);
/// @brief Invoke the command with a selected external profile.
/// @param options Parsed CLI11 intent.
/// @param selector Profile path.
/// @return Buffered machine response.
response check_with_profile(const check_options& options, std::string_view selector);
} // namespace planar::cmd::handlers
