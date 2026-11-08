/// @file
/// @brief CLI11 claim bindings and buffered execution dispatch.
export module planar.cmd.execution.claim;
import std;
import cli11;
export import planar.cmd.response;
export import planar.engine.runtime;
export namespace planar::cmd::handlers {
using claim_options=engine::command_run_options;
/// Attach required run acquisition options.
CLI::App& attach_claim(CLI::App&, claim_options&);
/// Invoke the embedded execution command.
response claim_command(const claim_options&, engine::command_vm* = nullptr);
/// Invoke a selected profile.
response claim_with_profile(const claim_options&, std::string_view);
}
