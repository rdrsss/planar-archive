/// @file
/// @brief Extension command dispatch for a captured profile owner; buffered redacted failures only.
///
/// The single entry point consumes an extension-ready profile owner after final parsing and returns a complete buffered
/// response. Expected failures use the shared response adapter vocabulary and never print host paths or Lua text.
export module planar.cmd.planar.handlers.extension;
import std;
export import planar.cmd.response;
import planar.cmd.invocation;
import planar.engine.runtime;

export namespace planar::cmd::handlers {
/// Dispatch one parsed external command on its retained profile owner.
/// @param owner Opaque profile owner adopted for extension dispatch; consumed by the call.
/// @param command Sealed external command name.
/// @param options Final parsed option records, borrowed for this synchronous call.
/// @param snapshot The invocation's single cwd and workspace-environment snapshot.
/// @return Buffered success or redacted failure; nothing is printed during registration.
response extension_command(engine::profile_invocation&& owner, std::string_view command,
                           std::span<const engine::extension_parsed_option> options, const invocation_snapshot& snapshot);
} // namespace planar::cmd::handlers
