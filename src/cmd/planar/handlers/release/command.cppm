/// @file
/// @brief CLI11 bindings for release; native intents retain secrets and private paths.
export module planar.cmd.execution.release;
import std;
import cli11;
export import planar.cmd.response;
export import planar.engine.runtime;
export namespace planar::cmd::handlers {
using release_options=engine::command_recovery_options;
/// Attach this family to an invocation-local parser.
CLI::App& attach_release(CLI::App&, release_options&);
/// Execute the embedded family.
response release_command(const release_options&,engine::command_vm* = nullptr);
/// Execute the selected profile family.
response release_with_profile(const release_options&,std::string_view);
}
