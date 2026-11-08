/// @file
/// @brief CLI11 bindings for the preview execution read; failures are buffered command responses.
export module planar.cmd.execution.preview;
import std;
import cli11;
export import planar.cmd.response;
import planar.engine.runtime;
export namespace planar::cmd::handlers {
/// Invocation-local option state, retained through dispatch.
struct preview_options {
  std::string ledger;
  std::string role;
  std::optional<std::string> since;
};
/// Attach the command to an invocation-local parser.
CLI::App& attach_preview(CLI::App& root, preview_options& options);
/// Execute the embedded command.
response preview_command(const preview_options& options, engine::command_vm* vm = nullptr);
/// Execute under the selected external profile.
response preview_with_profile(const preview_options& options, std::string_view selector);
}
