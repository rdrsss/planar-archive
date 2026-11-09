/// @file
/// @brief CLI11 verdict intent binding with buffered command dispatch; options remain alive through parsing.
export module planar.cmd.execution.verdict;
import std;
import cli11;
export import planar.cmd.response;
export import planar.engine.runtime;
export namespace planar::cmd::handlers {
/// Private native host paths and scalar options for one invocation.
struct verdict_options {
  std::string ledger;     ///< Explicit ledger path; never inferred from home configuration.
  std::string run;        ///< Secret run token supplied by the driver.
  std::string verdict;    ///< Reviewer verdict JSON file path.
  std::string request_id; ///< Caller-provided idempotency key.
  std::string actor;      ///< Driver actor recorded with the event.
};
/// @brief Attach the command and its exact required/optional options.
/// @param root Parent CLI11 parser.
/// @param options Storage retained until parse and dispatch finish.
/// @return Attached command.
CLI::App& attach_verdict(CLI::App& root, verdict_options& options);
/// @brief Invoke the retained embedded command.
/// @param options Parsed CLI11 intent.
/// @param vm Optional registered command VM.
/// @return Buffered machine response.
response verdict_command(const verdict_options& options, engine::command_vm* vm = nullptr);
/// @brief Invoke the command with a selected external profile.
/// @param options Parsed CLI11 intent.
/// @param selector Profile path.
/// @return Buffered machine response.
response verdict_with_profile(const verdict_options& options, std::string_view selector);
} // namespace planar::cmd::handlers
