/// @file
/// @brief Strict CLI11 history bindings and buffered, bounded public transport from a verified native snapshot.
export module planar.cmd.planar.handlers.history;
import std;
import cli11;
export import planar.cmd.response;
import planar.engine.runtime;
export namespace planar::cmd::handlers {
/// Invocation-owned parsed history strings; absence remains distinct from explicit invalid values.
struct history_options {
  std::int64_t revision_number = 0;    ///< CLI11-coerced revision, retaining its token below.
  std::int64_t event_number    = 0;    ///< CLI11 event expansion selector.
  std::string  event;                  ///< Optional event expansion id.
  std::int64_t at_event_number    = 0; ///< CLI11-coerced state selector.
  std::int64_t after_event_number = 0; ///< CLI11-coerced page cursor.
  std::int64_t limit_number       = 0; ///< CLI11-coerced bounded count.
  std::string  ledger;                 ///< Explicit existing ledger.
  std::string  revision;               ///< Optional positive revision ID.
  std::string  request_id;             ///< Optional ASCII replay key.
  std::string  at_event;               ///< Optional positive event ID.
  std::string  after_event;            ///< Optional exclusive positive cursor.
  std::string  limit;                  ///< Optional count, one through 100.
};
/// Attach strict mutually exclusive history bindings without I/O.
/// @param root Invocation-owned CLI11 root.
/// @param options State retained through parsing and dispatch.
/// @return Subcommand owned by root.
CLI::App& attach_history(CLI::App& root, history_options& options);
/// Convert validated CLI11 bindings to the native history intent without any host access.
/// @param options Validated CLI11 bindings.
/// @return The typed intent a direct or with-profile invocation executes.
engine::command_options history_intent(const history_options& options);
/// Read and encode a complete bounded history response.
/// @param options Validated CLI11 bindings.
/// @param vm Already validated retained registry, or null for direct invocation.
/// @return Buffered public JSON with LF, or redacted error with empty stdout.
response history_command(const history_options& options, engine::command_vm* vm = nullptr);
} // namespace planar::cmd::handlers
