/// @file
/// @brief CLI11 bindings for checkpoint; native intents retain secrets and private paths.
export module planar.cmd.execution.checkpoint;
import std;
import cli11;
export import planar.cmd.response;
export import planar.engine.runtime;
export namespace planar::cmd::handlers {
using checkpoint_options=engine::command_recovery_options;
/// Attach this family to an invocation-local parser.
CLI::App& attach_checkpoint(CLI::App&, checkpoint_options&);
/// Execute the embedded family.
response checkpoint_command(const checkpoint_options&,engine::command_vm* = nullptr);
/// Execute the selected profile family.
response checkpoint_with_profile(const checkpoint_options&,std::string_view);
}
