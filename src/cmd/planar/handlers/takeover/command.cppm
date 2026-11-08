/// @file
/// @brief CLI11 takeover bindings and buffered execution dispatch.
export module planar.cmd.execution.takeover;
import std;
import cli11;
export import planar.cmd.response;
export import planar.engine.runtime;
export namespace planar::cmd::handlers {
using takeover_options=engine::command_run_options;
/// Attach required run acquisition options.
CLI::App& attach_takeover(CLI::App&, takeover_options&);
/// Invoke the embedded execution command.
response takeover_command(const takeover_options&, engine::command_vm* = nullptr);
/// Invoke a selected profile.
response takeover_with_profile(const takeover_options&, std::string_view);
}
