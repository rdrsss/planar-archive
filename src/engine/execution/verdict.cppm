/// @file verdict.cppm
/// @brief Native reader of the reviewer verdict document (execution contract R18-2): the explicit no-follow single-file
/// capture (step 1), the 1 MiB bound (step 2), Glaze strict grammar (step 3), the finding and summary bounds (step 4),
/// the shape and field table (step 5), and normalization (step 6 authority, step 7 approve rules).
///
/// Entry points: read_verdict_file, parse_verdict_bytes and normalize_verdict. Every refusal is a verdict_error with
/// an existing EX-7 code and message: io, path_denied and capture_changed (P2-4 file capture rows), resource_limit
/// (EX-8 verdict bounds, message "Execution resource limit exceeded", field empty), schema (field verdict for the
/// grammar, otherwise verdict. plus keys and decimal indexes) and unsupported_policy (R18-7). The reference rule of
/// step 7 (a repair role that owns no target) needs the execution snapshot and is not checked here. All functions are
/// thread-safe and keep no shared state.
export module planar.engine.execution.verdict;
import std;
export import planar.engine.value;
export import planar.engine.history.execution_codec;
export namespace planar::engine::execution {
/// Typed refusal of the verdict reader. limit and maximum are set only for resource_limit.
struct verdict_error {
  std::string   code;        ///< io, path_denied, capture_changed, resource_limit, schema or unsupported_policy.
  std::string   message;     ///< Existing fixed message for the code.
  std::string   field;       ///< Field path for schema, otherwise empty.
  std::string   limit;       ///< Bound name for resource_limit, otherwise empty.
  std::uint64_t maximum = 0; ///< Bound maximum for resource_limit, otherwise zero.
};
/// Largest accepted verdict file in bytes (EX-8 verdict_bytes).
inline constexpr std::size_t verdict_bytes_maximum = 1048576;
/// @brief Apply R18-2 steps 2 to 5 to captured bytes.
/// @param captured Exact captured file bytes.
/// @return The parsed structural document (authority unchanged), or the first refusal in check order.
std::expected<value_ptr, verdict_error> parse_verdict_bytes(std::string_view captured);
/// @brief Apply R18-2 step 1 through the existing explicit no-follow single-file capture, then steps 2 to 5.
/// @param file Host path of the verdict file; it is resolved like an explicit input (a missing leaf is io).
/// @return The parsed structural document, or the first refusal in check order with the P2-4 messages.
std::expected<value_ptr, verdict_error> read_verdict_file(const std::filesystem::path& file);
/// @brief Apply R18-2 step 6 (authority) and step 7 (approve rules) to a parsed document and normalize it.
/// @param document Parsed document; steps 4 and 5 are re-applied so any structural value is safe to pass.
/// @return The typed normalized verdict with authority reviewer_role_v1, or schema / unsupported_policy / resource_limit.
std::expected<history::verdict_v1, verdict_error> normalize_verdict(const value_ptr& document);
} // namespace planar::engine::execution
