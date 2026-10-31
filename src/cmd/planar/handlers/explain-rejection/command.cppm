/// @file
/// @brief The `explain-rejection` authoring family: option state, CLI11 attach and handler (RA-1 wave 1 skeleton).
/// Entry points: attach_explain_rejection and explain_rejection_command. Errors are buffered responses.
export module planar.cmd.authoring.explain_rejection;
import std;
import cli11;
export import planar.cmd.authoring;
export namespace planar::cmd::authoring {
/// Invocation-owned option state of `explain-rejection`; alive through parsing and dispatch.
struct explain_rejection_options {
  std::string diagnostic; ///< Raw `--diagnostic` value.
};
/// Attach the `explain-rejection` syntax to the invocation-local root parser.
/// @param root Invocation-local parser.
/// @param options State retained through parsing and dispatch.
/// @return The attached subcommand.
CLI::App& attach_explain_rejection(CLI::App& root, explain_rejection_options& options);
/// Run `explain-rejection` for already parsed options.
/// @param options Parsed values.
/// @param io Process observation seam.
/// @return Buffered response.
response explain_rejection_command(const explain_rejection_options& options, const readers& io = process_readers());
} // namespace planar::cmd::authoring
