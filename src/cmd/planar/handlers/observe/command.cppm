/// @file
/// @brief The `observe` authoring family: option state, CLI11 attach and handler (RA-1 wave 1 skeleton).
/// Entry points: attach_observe and observe_command. Errors are buffered responses.
export module planar.cmd.authoring.observe;
import std;
import cli11;
export import planar.cmd.authoring;
export namespace planar::cmd::authoring {
/// Invocation-owned option state of `observe`; alive through parsing and dispatch.
struct observe_options {
  std::string from; ///< Raw `--from` value.
};
/// Attach the `observe` syntax to the invocation-local root parser.
/// @param root Invocation-local parser.
/// @param options State retained through parsing and dispatch.
/// @return The attached subcommand.
CLI::App& attach_observe(CLI::App& root, observe_options& options);
/// Run `observe` for already parsed options.
/// @param options Parsed values.
/// @param io Process observation seam.
/// @param execute Native execution step; empty answers the stub refusal.
/// @return Buffered response.
response observe_command(const observe_options& options, const readers& io = process_readers(), const executor& execute = {});
} // namespace planar::cmd::authoring
