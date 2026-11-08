/// @file
/// @brief CLI11 bindings for the status execution read; failures are buffered command responses.
export module planar.cmd.execution.status;
import std;
import cli11;
export import planar.cmd.response;
import planar.engine.runtime;
export namespace planar::cmd::handlers {
/// Invocation-local option state, retained through dispatch.
struct status_options {
  std::string ledger;
  std::optional<std::string> role;
  std::optional<std::string> request_id;
  std::optional<std::string> workspace;
  std::optional<std::string> registry;
};
/// Attach the command to an invocation-local parser.
CLI::App& attach_status(CLI::App& root, status_options& options);
/// Execute the embedded command.
response status_command(const status_options& options, engine::command_vm* vm = nullptr);
/// Execute under the selected external profile.
response status_with_profile(const status_options& options, std::string_view selector);
}
