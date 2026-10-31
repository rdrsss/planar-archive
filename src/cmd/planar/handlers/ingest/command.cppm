/// @file
/// @brief The `ingest` authoring family: option state, CLI11 attach and handler (RA-1 wave 1 skeleton).
/// Entry points: attach_ingest and ingest_command. Errors are buffered responses.
export module planar.cmd.authoring.ingest;
import std;
import cli11;
export import planar.cmd.authoring;
export namespace planar::cmd::authoring {
/// Invocation-owned option state of `ingest`; alive through parsing and dispatch.
struct ingest_options {
  std::string workspace; ///< Raw `--workspace` value.
  std::string registry;  ///< Raw `--registry` value.
};
/// Attach the `ingest` syntax to the invocation-local root parser.
/// @param root Invocation-local parser.
/// @param options State retained through parsing and dispatch.
/// @return The attached subcommand.
CLI::App& attach_ingest(CLI::App& root, ingest_options& options);
/// Run `ingest` for already parsed options.
/// @param options Parsed values.
/// @param io Process observation seam.
/// @return Buffered response.
response ingest_command(const ingest_options& options, const readers& io = process_readers());
} // namespace planar::cmd::authoring
