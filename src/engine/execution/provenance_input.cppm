/// @file
/// @brief The `read_provenance({provenance:SLOT})` service: an explicit single-file capture of a `--provenance` file
/// and its strict `driver_provenance_v1` normalization (execution-contract P2-4, D146, R15-B L17).
///
/// Entry points are `read_provenance` (capture, then parse) and `parse_provenance` (bytes only). Check order, first
/// failure wins: the raw captured size above 65536 bytes is `provenance_bytes` before any parse (L17); then JSON grammar
/// (`schema`, field `provenance`); nesting depth above 4 (`provenance_depth`); more than 32 settings
/// (`provenance_settings`); any string above 1024 bytes (`provenance_string_bytes`); then shape and leaf violations
/// (`schema`, field `provenance.<keys>`). Capture refusals reuse the authoring `--diagnostic` codes and messages. Every
/// failure is a `provenance_error` that names an existing code and message; no new message exists. Thread-safe and free
/// of shared state; R15-D wires it to `claim` and `takeover`.
export module planar.engine.execution.provenance_input;

import std;
export import planar.engine.value;

export namespace planar::engine::execution {

/// @brief Refusal of the provenance input: an existing code and message, the schema field and the resource bound.
struct provenance_error {
  std::string   code;        ///< `io`, `path_denied`, `capture_changed`, `resource_limit` or `schema`.
  std::string   message;     ///< Existing message of the code at this site, byte-exact.
  std::string   field;       ///< `provenance` or `provenance.<keys>` for `schema`; empty otherwise.
  std::string   limit;       ///< Bound name for `resource_limit`; empty otherwise.
  std::uint64_t maximum = 0; ///< Bound maximum for `resource_limit`; zero otherwise.
};

/// @brief Maximum raw provenance file size in bytes (EX-8 `provenance_bytes`).
inline constexpr std::uint64_t provenance_file_maximum = 65536;

/// @brief Validate and normalize the bytes of a provenance file (P2-4 steps 2 to 6).
/// @param bytes Captured file bytes, at most `provenance_file_maximum`.
/// @return The normalized `driver_provenance_v1` document (settings sorted by name), or the first refusal.
std::expected<value_ptr, provenance_error> parse_provenance(std::string_view bytes);

/// @brief Capture one explicit provenance file and normalize it (P2-4 steps 1 to 6).
/// @param path Lexically normalized absolute host path from the native `--provenance` slot; never from Lua.
/// @return The normalized document, or the first refusal; the raw size check precedes any read of the content.
std::expected<value_ptr, provenance_error> read_provenance(const std::filesystem::path& path);

} // namespace planar::engine::execution
