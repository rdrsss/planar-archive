/// @file
/// @brief The `orchestrator` authoring family: option state, CLI11 attach and handler (RA-1 wave 1 skeleton).
/// Entry points: attach_orchestrator and orchestrator_command. Errors are buffered responses.
export module planar.cmd.authoring.orchestrator;
import std;
import cli11;
export import planar.cmd.authoring;
export namespace planar::cmd::authoring {
/// Invocation-owned option state of `orchestrator`; alive through parsing and dispatch.
struct orchestrator_options {
  std::string workspace;   ///< Raw `--workspace` value.
  std::string registry;    ///< Raw `--registry` value.
  std::string job;         ///< Raw `--job` value.
  std::string ledger = {}; ///< Raw --ledger value, exclusive with the declaration form.
};
/// Attach the `orchestrator` syntax to the invocation-local root parser.
/// @param root Invocation-local parser.
/// @param options State retained through parsing and dispatch.
/// @return The attached subcommand.
CLI::App& attach_orchestrator(CLI::App& root, orchestrator_options& options);
/// Run `orchestrator` for already parsed options.
/// @param options Parsed values.
/// @param io Process observation seam.
/// @param execute Native execution step; empty answers the stub refusal.
/// @return Buffered response.
response orchestrator_command(const orchestrator_options& options, const readers& io = process_readers(),
                              const executor& execute = {});
} // namespace planar::cmd::authoring
