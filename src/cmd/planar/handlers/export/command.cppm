/// @file
/// @brief Strict export bindings and redacted buffered publication results; invocation-local state only.
export module planar.cmd.planar.handlers.source_export;
import std;
import cli11;
export import planar.cmd.response;
import planar.engine.runtime;
export import planar.engine.source_export;

export namespace planar::cmd::handlers {
/// Explicit options retained through CLI11 parsing and native dispatch.
struct export_options {
  std::string  ledger;              ///< Existing ledger spelling.
  std::string  revision;            ///< Canonical positive signed-64 decimal ID.
  std::string  target;              ///< Original explicit new-directory spelling.
  std::int64_t revision_number = 0; ///< CLI11-coerced canonical revision.
};
/// Attach export options without filesystem access.
/// @param root Invocation-owned parser.
/// @param options State retained through dispatch.
/// @return Root-owned command.
CLI::App& attach_export(CLI::App& root, export_options& options);
/// Convert validated CLI11 bindings to the native export intent without any host access.
/// @param options Validated CLI11 bindings.
/// @return The typed intent a direct or with-profile invocation executes.
engine::command_options export_intent(const export_options& options);
/// Publish the selected source tree and encode a complete public result.
/// @param options Validated explicit bindings.
/// @param vm Already validated retained registry, or null for direct invocation.
/// @return Buffered success or redacted failure; post-publication failures instruct inspection.
response export_command(const export_options& options, engine::command_vm* vm = nullptr);
/// Adapt a native export refusal with stable redacted bytes.
/// @param error Typed export failure; only validated staging tokens may escape.
/// @return Buffered public error.
response export_failure(const engine::source_export::export_error& error);
} // namespace planar::cmd::handlers
