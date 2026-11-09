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
  std::string                ledger; ///< Explicit ledger path; never inferred from home configuration.
  std::string                role;   ///< Selected role identifier.
  std::optional<std::string> since;  ///< Optional retained render digest for packet comparison.
};
/// Attach the command to an invocation-local parser.
/// @param root Invocation-local CLI11 parser.
/// @param options Storage that outlives parsing and dispatch.
/// @return Attached command with the exact option grammar.
CLI::App& attach_preview(CLI::App& root, preview_options& options);
/// Execute the embedded command.
/// @param options Parsed command intent.
/// @param vm Optional registered VM; null creates a fresh embedded invocation.
/// @return Buffered success or typed failure response.
response preview_command(const preview_options& options, engine::command_vm* vm = nullptr);
/// Execute under the selected external profile.
/// @param options Parsed command intent.
/// @param selector External profile selector.
/// @return Buffered success or typed failure response; no direct process output.
response preview_with_profile(const preview_options& options, std::string_view selector);
} // namespace planar::cmd::handlers
