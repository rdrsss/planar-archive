/// @file
/// @brief CLI11 bindings for refresh; native intents retain secrets and private paths.
export module planar.cmd.execution.refresh;
import std;
import cli11;
export import planar.cmd.response;
export import planar.engine.runtime;
export namespace planar::cmd::handlers {
using refresh_options=engine::command_recovery_options;
/// Attach this family to an invocation-local parser.
CLI::App& attach_refresh(CLI::App&, refresh_options&);
/// Execute the embedded family.
response refresh_command(const refresh_options&,engine::command_vm* = nullptr);
/// Execute the selected profile family.
response refresh_with_profile(const refresh_options&,std::string_view);
}
