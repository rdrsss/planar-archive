/// @file
/// @brief CLI11 bindings and dispatch of `planar activate` (execution-contract P2-1, P2-7).
///
/// Entry points are `attach_activate` (six required separate-token text options; `--expect-job` uses the metavariable
/// `JOB_OR_NONE`), `activate_intent`, `activate_command` (embedded factory) and `activate_with_profile`. The module name
/// sits outside the `planar.cmd.planar.handlers.` prefix on purpose (draft decision DD27): the audited composition
/// generators rewrite that prefix in the bootstrap copies they build, and this family has no generated copy, so every
/// composition dispatches activate through this real module and its own fresh VM. Errors are buffered responses; no
/// shared state.
export module planar.cmd.execution.activate;
import std;
import cli11;
export import planar.cmd.response;
import planar.engine.runtime;
export namespace planar::cmd::handlers {
/// Invocation-owned activate bindings; no private spelling enters Lua.
struct activate_options {
  std::string ledger;          ///< Explicit ledger path.
  std::string job;             ///< Job ID to activate.
  std::string expect_revision; ///< Canonical positive decimal or none.
  std::string expect_job;      ///< Uppercase NONE or the expected active job ID.
  std::string request_id;      ///< ASCII idempotency key.
  std::string actor;           ///< UTF-8 driver assertion.
};
/// Attach the required activate syntax without filesystem access.
/// @param root Invocation-local parser.
/// @param options State retained through parsing and dispatch.
/// @return Root-owned command.
CLI::App& attach_activate(CLI::App& root, activate_options& options);
/// Convert validated CLI11 bindings to the native activate intent.
/// @param options Validated CLI11 bindings.
/// @return The typed intent a direct or with-profile invocation executes.
engine::command_options activate_intent(const activate_options& options);
/// Execute activate with the embedded factory.
/// @param options Parsed required text values.
/// @param vm Already validated registry of the real runtime, or null for a fresh VM.
/// @return Buffered saved response or redacted failure.
response activate_command(const activate_options& options, engine::command_vm* vm = nullptr);
/// Execute activate under a selected profile.
/// @param options Parsed required text values.
/// @param profile_selector Raw selector from the envelope.
/// @return Buffered saved response or redacted failure.
response activate_with_profile(const activate_options& options, std::string_view profile_selector);
} // namespace planar::cmd::handlers
