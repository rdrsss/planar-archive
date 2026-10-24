/// @file
/// @brief Explicit new-ledger CLI11 bindings and buffered initialization; no ambient discovery or overwrite.
export module planar.cmd.planar.handlers.init;
import std;
import cli11;
export import planar.cmd.response;
import planar.engine.runtime;

export namespace planar::cmd::handlers {
/// Invocation-owned bindings; independent instances share no mutable state.
struct init_options {
  std::string ledger;       ///< Explicit new target filename.
  std::string workspace_id; ///< Explicit declaration workspace identifier.
};
/// Attach initialization bindings without touching the filesystem.
/// @param root Invocation-owned CLI11 root.
/// @param options Bindings retained through parsing and execution.
/// @return Subcommand owned by root.
CLI::App& attach_init(CLI::App& root, init_options& options);
/// Publish a new ledger through the native no-replace boundary.
/// @param options Parsed explicit bindings; target components retain filesystem meaning.
/// @param vm Already validated retained registry, or null for direct invocation.
/// @return Committed metadata envelope or a redacted buffered failure.
response init_command(const init_options& options, engine::command_vm* vm = nullptr);
} // namespace planar::cmd::handlers
