/// @file
/// @brief CLI11 bindings for recover; native intents retain secrets and private paths.
export module planar.cmd.execution.recover;
import std;
import cli11;
export import planar.cmd.response;
export import planar.engine.runtime;
export namespace planar::cmd::handlers {
using recover_options=engine::command_recover_options;
/// Attach this family to an invocation-local parser.
CLI::App& attach_recover(CLI::App&, recover_options&);
/// Execute the embedded family.
response recover_command(const recover_options&,engine::command_vm* = nullptr);
/// Execute the selected profile family.
response recover_with_profile(const recover_options&,std::string_view);
}
