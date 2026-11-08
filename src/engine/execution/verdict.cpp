module planar.engine.execution.verdict;
import std;
import planar.engine.capture;
import planar.engine.capture.host_path;
import planar.engine.json_codec;

namespace planar::engine::execution {
namespace {

constexpr std::string_view resource_message = "Execution resource limit exceeded";
constexpr std::string_view policy_message   = "Execution policy is not supported";

std::unexpected<verdict_error> refuse(std::string code, std::string message, std::string field = {}) {
  return std::unexpected(verdict_error{std::move(code), std::move(message), std::move(field), {}, 0});
}

std::unexpected<verdict_error> limit(std::string name, std::uint64_t maximum) {
  return std::unexpected(verdict_error{"resource_limit", std::string(resource_message), {}, std::move(name), maximum});
}

std::unexpected<verdict_error> from_codec(const history::execution_codec_error& error) {
  if (error.code == history::execution_codec_code::resource_limit)
    return limit(error.limit, error.maximum);
  // Unreachable for parsed documents: a codec invalid here is a shape the field table missed.
  return refuse("schema", "JSON codec refused input", "verdict");
}

// The P2-4 provenance file rows; the capture boundary already uses exactly these codes and messages.
std::unexpected<verdict_error> from_capture(const diagnostic& error) {
  if (error.code == "resource_limit")
    return limit("verdict_bytes", verdict_bytes_maximum);
  if (error.code == "capture_changed")
    return refuse("capture_changed", "File changed during read");
  if (error.code == "path_denied")
    return refuse("path_denied", error.message);
  if (error.code == "io" && error.message == "Explicit input must be a regular file")
    return refuse("io", "Explicit input must be a regular file");
  return refuse("io", "Cannot resolve explicit input path");
}

} // namespace

std::expected<value_ptr, verdict_error> parse_verdict_bytes(std::string_view captured) {
  // Step 2: the byte bound before any parse.
  if (captured.size() > verdict_bytes_maximum)
    return limit("verdict_bytes", verdict_bytes_maximum);
  // Step 3: Glaze strict grammar (duplicate keys, trailing bytes, null, non-integer numbers, NUL, invalid UTF-8).
  auto parsed = codec_decode(captured, codec_limits{verdict_bytes_maximum, 128, 100000});
  if (!parsed)
    return refuse("schema", "JSON codec refused input", "verdict");
  // Step 4: finding count, finding text and summary bounds.
  if (auto bounded = history::check_verdict_bounds(*parsed); !bounded)
    return from_codec(bounded.error());
  // Step 5: shape and field table.
  if (auto typed = history::verdict_from_value(*parsed); !typed)
    return refuse("schema", std::move(typed.error().message), std::move(typed.error().field));
  return std::move(*parsed);
}

std::expected<value_ptr, verdict_error> read_verdict_file(const std::filesystem::path& file) {
  try {
    // Step 1: explicit-input resolution, then the no-follow single-file capture used for --diagnostic.
    auto resolved = resolve_explicit_file(file.string());
    if (!resolved) {
      std::error_code ignored;
      const auto      status = std::filesystem::symlink_status(file, ignored);
      // A leaf that does not exist at all is an unresolvable input, not a wrong kind of file (P2-4 row 1).
      if (resolved.error().message == "Explicit input must be a regular file" &&
          (ignored || status.type() == std::filesystem::file_type::not_found))
        return refuse("io", "Cannot resolve explicit input path");
      return from_capture(resolved.error());
    }
    capture_session session;
    auto            captured = capture_diagnostic(session, *resolved, verdict_bytes_maximum);
    if (!captured)
      return from_capture(captured.error());
    return parse_verdict_bytes(captured->bytes);
  } catch (const std::bad_alloc&) {
    return limit("verdict_bytes", verdict_bytes_maximum);
  }
}

std::expected<history::verdict_v1, verdict_error> normalize_verdict(const value_ptr& document) {
  if (auto bounded = history::check_verdict_bounds(document); !bounded)
    return from_codec(bounded.error());
  auto typed = history::verdict_from_value(document);
  if (!typed)
    return refuse("schema", std::move(typed.error().message), std::move(typed.error().field));
  // Step 6: absent authority is the reviewer role; any other asserted authority has no policy (R18-7, Q16 A).
  if (document->fields.contains("authority") && typed->authority != history::verdict_authority_reviewer)
    return refuse("unsupported_policy", std::string(policy_message));
  typed->authority = std::string(history::verdict_authority_reviewer);
  // Step 7: approve with a must finding or a nonempty repair list.
  if (auto decided = history::check_verdict_decision(*typed); !decided)
    return refuse("schema", std::move(decided.error().message), std::move(decided.error().field));
  return std::move(*typed);
}

} // namespace planar::engine::execution
