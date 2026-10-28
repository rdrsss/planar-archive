/// @file
/// @brief Render handler registration and sealed capture orchestration; failures are buffered diagnostics.
export module planar.cmd.planar.handlers.render;
import std;
import cli11;
export import planar.cmd.response;
import planar.engine.runtime;

export namespace planar::cmd::handlers {
/// Owned CLI11 bindings; keep this object alive until parsing and execution finish.
/// Independent instances share no mutable state and may run concurrently.
struct render_options {
  std::array<std::string, 8> values;                  ///< Workspace, registry, root, job, role, base, observations, budget.
  std::int32_t               budget_bytes    = 0;     ///< CLI11-coerced budget with the canonical token retained in values.
  bool                       capture_outputs = false; ///< Request captured current/base trees.
};
/// Attach the render command and its explicit CLI11 option bindings.
/// @param root Root command owned by this invocation.
/// @param options Bindings whose lifetime covers parsing and execution.
/// @return Registered subcommand; owned by root.
CLI::App& attach_render(CLI::App& root, render_options& options);
/// Convert validated CLI11 bindings to the native render intent without any host access.
/// @param options Validated CLI11 bindings.
/// @return The typed intent a direct or with-profile invocation executes.
engine::command_options render_intent(const render_options& options);
/// Resolve host inputs and render using captured values after CLI11 parsing.
/// @param options Parsed explicit options.
/// @param vm Already validated retained registry, or null for direct invocation.
/// @return Canonical buffered result, including semantic or native failures.
response render_command(const render_options& options, engine::command_vm* vm = nullptr);
} // namespace planar::cmd::handlers
