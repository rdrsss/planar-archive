/// @file
/// @brief Required text-only load bindings and retained native acceptance dispatch.
export module planar.cmd.planar.handlers.load;
import std;
import cli11;
export import planar.cmd.response;
import planar.engine.runtime;
export namespace planar::cmd::handlers {
/// Invocation-owned six required text options; no private spelling enters Lua.
struct load_options {
  std::string ledger;          ///< Explicit ledger path.
  std::string workspace;       ///< File or workspace name.
  std::string registry;        ///< Explicit registry path.
  std::string expect_revision; ///< Canonical positive decimal or none.
  std::string request_id;      ///< ASCII idempotency key.
  std::string actor;           ///< UTF-8 driver assertion.
};
/// Attach the required load syntax without filesystem access.
/// @param root Invocation-local parser.
/// @param options State retained through parsing and dispatch.
/// @return Root-owned command.
CLI::App& attach_load(CLI::App& root, load_options& options);
/// Execute load through the validated retained command VM.
/// @param options Parsed required text values.
/// @param vm Already validated registry or null for direct invocation.
/// @return Buffered saved response or redacted failure.
response load_command(const load_options& options, engine::command_vm* vm = nullptr);
} // namespace planar::cmd::handlers
