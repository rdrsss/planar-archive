/// @file
/// @brief The `spec-prompt` authoring family: option state, CLI11 attach and handler (RA-1 wave 1 skeleton).
/// Entry points: attach_spec_prompt and spec_prompt_command. Errors are buffered responses.
export module planar.cmd.authoring.spec_prompt;
import std;
import cli11;
export import planar.cmd.authoring;
export namespace planar::cmd::authoring {
/// Invocation-owned option state of `spec-prompt`; alive through parsing and dispatch.
struct spec_prompt_options {
  std::string workspace; ///< Raw `--workspace` value.
  std::string registry;  ///< Raw `--registry` value.
};
/// Attach the `spec-prompt` syntax to the invocation-local root parser.
/// @param root Invocation-local parser.
/// @param options State retained through parsing and dispatch.
/// @return The attached subcommand.
CLI::App& attach_spec_prompt(CLI::App& root, spec_prompt_options& options);
/// Run `spec-prompt` for already parsed options.
/// @param options Parsed values.
/// @param io Process observation seam.
/// @return Buffered response.
response spec_prompt_command(const spec_prompt_options& options, const readers& io = process_readers());
} // namespace planar::cmd::authoring
