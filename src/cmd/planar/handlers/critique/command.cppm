/// @file
/// @brief The `critique` authoring family: option state, CLI11 attach and handler (RA-1 wave 1 skeleton).
/// Entry points: attach_critique and critique_command. Errors are buffered responses.
export module planar.cmd.authoring.critique;
import std;
import cli11;
export import planar.cmd.authoring;
export namespace planar::cmd::authoring {
/// Invocation-owned option state of `critique`; alive through parsing and dispatch.
struct critique_options {
  std::string workspace; ///< Raw `--workspace` value.
  std::string registry;  ///< Raw `--registry` value.
};
/// Attach the `critique` syntax to the invocation-local root parser.
/// @param root Invocation-local parser.
/// @param options State retained through parsing and dispatch.
/// @return The attached subcommand.
CLI::App& attach_critique(CLI::App& root, critique_options& options);
/// Run `critique` for already parsed options.
/// @param options Parsed values.
/// @param io Process observation seam.
/// @return Buffered response.
response critique_command(const critique_options& options, const readers& io = process_readers());
} // namespace planar::cmd::authoring
