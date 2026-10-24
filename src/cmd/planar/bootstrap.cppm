/// @file
/// @brief CLI11 root registration and dispatch with buffered output and canonical errors.
export module planar.cmd.bootstrap;
import std;
export import planar.cmd.response;

export namespace planar::cmd {
/// Parse explicit CLI11 arguments and dispatch the selected command handler.
/// @param arguments Arguments excluding argv[0], borrowed only during this call.
/// @return Complete buffered response; no ambient configuration or database access.
response invoke(std::span<const std::string_view> arguments);
} // namespace planar::cmd
