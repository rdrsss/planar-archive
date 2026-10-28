module planar.cmd.response;
import std;
namespace planar::cmd::detail {
value_ptr text(std::string value) {
  auto node    = std::make_shared<engine::value>();
  node->kind   = value_kind::string;
  node->string = std::move(value);
  return node;
}
value_ptr integer(std::int64_t number) {
  auto node     = std::make_shared<value>();
  node->kind    = value_kind::integer;
  node->integer = number;
  return node;
}
value_ptr object(std::map<std::string, value_ptr> fields) {
  auto node    = std::make_shared<value>();
  node->kind   = value_kind::record;
  node->fields = std::move(fields);
  return node;
}
value_ptr array(std::vector<value_ptr> items) {
  auto node   = std::make_shared<value>();
  node->kind  = value_kind::list;
  node->items = std::move(items);
  return node;
}
value_ptr native_error(const diagnostic& error) {
  std::map<std::string, value_ptr> fields{{"code", text(error.code)}, {"message", text(error.message)}, {"field", text("")}};
  if (!error.limit.empty())
    fields["details"] = object({{"limit", text(error.limit)}, {"maximum", integer(error.maximum)}});
  if (error.code == "io")
    fields["details"] = object({{"operation", text("capture")}});
  return object(std::move(fields));
}
value_ptr ledger_error(const engine::ledger::types::format_error& error) {
  using enum engine::ledger::types::format_error_code;
  value_ptr details;
  if (error.detail) {
    auto malformed = [] {
      return object({{"code", text("internal")}, {"message", text("Invalid embedded command protocol")}, {"field", text("")}});
    };
    if (auto revision = std::get_if<engine::ledger::types::revision_conflict_detail>(&*error.detail)) {
      if (error.code != revision_conflict || (revision->expected && *revision->expected <= 0) ||
          (revision->actual && *revision->actual <= 0))
        return malformed();
      auto head = [](std::optional<std::int64_t> value) { return text(value ? std::to_string(*value) : "none"); };
      details   = object({{"expected", head(revision->expected)}, {"actual", head(revision->actual)}});
    } else if (auto collision = std::get_if<engine::ledger::types::record_collision_detail>(&*error.detail)) {
      using enum engine::history::record_kind;
      if (error.code != hash_collision)
        return malformed();
      std::string kind;
      switch (collision->kind) {
      case file:
        kind = "file";
        break;
      case directory:
        kind = "directory";
        break;
      case value:
        kind = "value";
        break;
      case request:
        kind = "request";
        break;
      default:
        return malformed();
      }
      if ((collision->kind == request) == collision->digest.has_value())
        return malformed();
      std::map<std::string, value_ptr> collision_fields{{"kind", text(std::move(kind))}};
      if (collision->digest) {
        std::string digest;
        for (auto byte : *collision->digest)
          digest += std::format("{:02x}", byte);
        collision_fields["digest"] = text(std::move(digest));
      }
      details = object(std::move(collision_fields));
    } else {
      return malformed();
    }
  }
  std::string code, message;
  switch (error.code) {
  case invalid_argument:
    code    = "schema";
    message = "Invalid ledger arguments";
    break;
  case unsafe_path:
    code    = "path_denied";
    message = "Ledger path requires existing real parent directories and a valid target";
    break;
  case exists:
    code    = "ledger_exists";
    message = "Ledger target already exists";
    break;
  case not_found:
    code    = "ledger_not_found";
    message = "Ledger was not found";
    break;
  case foreign_ledger:
    code    = "foreign_ledger";
    message = "File is not a Planar ledger";
    break;
  case unsupported_version:
    code    = "unsupported_ledger_version";
    message = "Ledger version or state is unsupported";
    break;
  case unsupported_state:
    code    = "unsupported_ledger_version";
    message = "Ledger version or state is unsupported";
    break;
  case corrupt:
    code    = "ledger_corrupt";
    message = "Ledger integrity verification failed";
    break;
  case busy:
    code    = "ledger_busy";
    message = "Ledger is busy";
    break;
  case storage_full:
    code    = "storage_full";
    message = "Ledger storage is full";
    break;
  case io:
    code    = "io";
    message = "Ledger operation failed";
    break;
  case publication_uncertain:
    code    = "io";
    message = "Ledger may be published; inspect the explicit target before retrying";
    break;
  case recovery_required:
    code    = "io";
    message = "Explicit ledger recovery is required";
    break;
  case hash_collision:
    code    = "hash_collision";
    message = "Ledger record identity collision";
    break;
  case request_conflict:
    code    = "request_conflict";
    message = "Ledger request conflicts with its saved intent";
    break;
  case revision_conflict:
    code    = "revision_conflict";
    message = "Ledger revision does not match the expected revision";
    break;
  case capture_changed:
    code    = "capture_changed";
    message = "Captured declaration inputs changed";
    break;
  }
  std::map<std::string, value_ptr> fields{{"code", text(code)}, {"message", text(message)}, {"field", text("")}};
  if (details)
    fields["details"] = std::move(details);
  if (code == "io")
    fields["details"] = object({{"operation", text("ledger")}});
  return object(std::move(fields));
}
response failed(value_ptr error) {
  const auto& code = error->fields.at("code")->string;
  const int status = code == "usage" ? 2
                                     : (code == "io" || code == "internal" || code == "ledger_corrupt" || code == "ledger_busy" ||
                                                code == "storage_full"
                                            ? 4
                                            : 3);
  auto      encoded = codec_encode(object({{"version", integer(1)}, {"kind", text("error")}, {"error", error}}));
  if (!encoded)
    return {4, "", ""};
  return {status, "", *encoded + "\n"};
}
response usage() {
  return failed(native_error({"usage", "Expected command options or --help; see --help"}));
}
namespace {
struct closed_pair {
  std::string_view code, message;
};
// Every fixed {code, message} of the closed diagnostic table that profile capture, admission and registration can
// propagate. Anything outside it is a protocol failure and is redacted; no field, host path or Lua text is ever printed.
std::span<const closed_pair> closed_pairs() {
  static constexpr auto table = std::to_array<closed_pair>({
      {"usage", "Expected command options or --help; see --help"},
      {"path_denied", "Invalid explicit input path"},
      {"path_denied", "Invalid explicit input cwd"},
      {"resource_limit", "Explicit input path limit exceeded"},
      {"io", "Cannot resolve explicit input path"},
      {"io", "Cannot obtain invocation cwd"},
      {"path_denied", "Explicit input file is a symbolic link"},
      {"io", "Explicit input must be a regular file"},
      {"io", "Explicit root must be a directory"},
      {"path_denied", "Too many symbolic links in explicit root"},
      {"path_denied", "Explicit root is a symbolic link target"},
      {"io", "Cannot resolve explicit root directory"},
      {"path_denied", "Cannot resolve explicit input path"},
      {"resource_limit", "Load exceeds native resource limits"},
      {"internal", "Invalid native load preparation"},
      {"resource_limit", "Load request exceeds native resource limits"},
      {"schema", "Invalid load arguments"},
      {"schema", "Invalid external profile structure"},
      {"unsupported_version", "Unsupported external profile version"},
      {"path", "Invalid external module path"},
      {"unsafe_path", "External profile overlaps a supplied input"},
      {"reference", "External registration identity conflict"},
      {"capture_changed", "Captured external profile changed"},
      {"resource_limit", "External profile resource limit exceeded"},
      {"internal", "Invalid external profile protocol"},
      {"resource_limit", "Lua stack exceeded"},
      {"resource_limit", "Lua instruction limit exceeded"},
      {"resource_limit", "Lua allocation exceeded"},
      {"schema", "Invalid ledger arguments"},
      {"path_denied", "Ledger path requires existing real parent directories and a valid target"},
      {"ledger_exists", "Ledger target already exists"},
      {"ledger_not_found", "Ledger was not found"},
      {"foreign_ledger", "File is not a Planar ledger"},
      {"unsupported_ledger_version", "Ledger version or state is unsupported"},
      {"ledger_corrupt", "Ledger integrity verification failed"},
      {"ledger_busy", "Ledger is busy"},
      {"storage_full", "Ledger storage is full"},
      {"io", "Ledger operation failed"},
      {"io", "Ledger may be published; inspect the explicit target before retrying"},
      {"io", "Explicit ledger recovery is required"},
      {"hash_collision", "Ledger record identity collision"},
      {"request_conflict", "Ledger request conflicts with its saved intent"},
      {"revision_conflict", "Ledger revision does not match the expected revision"},
      {"capture_changed", "Captured declaration inputs changed"},
      {"revision_not_found", "Recorded history view is unavailable"},
      {"request_not_found", "Recorded history view is unavailable"},
      {"event_not_found", "Recorded history view is unavailable"},
      {"unsupported_interpretation", "Recorded history view is unavailable"},
      {"revision_not_found", "Recorded revision was not found"},
      {"schema", "Invalid export arguments"},
      {"path_denied", "Export path requires existing real parent directories and a valid target"},
      {"target_exists", "Export target already exists"},
      {"io", "Export operation failed"},
      {"internal", "Invalid export cleanup location"},
      {"internal", "Invalid embedded command protocol"},
      {"internal", "Missing native load capture"},
      {"internal", "Missing native load intent"},
      {"internal", "Invalid saved load response"},
      {"internal", "Missing load completion"},
      {"io", "Cannot write stdout"},
      {"resource_limit", "JSON codec refused input"},
  });
  return table;
}
bool in_closed_pairs(const diagnostic& detail) {
  return std::ranges::any_of(closed_pairs(),
                             [&](const closed_pair& row) { return row.code == detail.code && row.message == detail.message; });
}
// Existing propagated native capture, path and ledger refusals keep their exact messages; the registration vocabulary
// (schema, version, path, unsafe_path, reference, capture_changed, resource_limit, internal) never does.
bool is_native_passthrough(const diagnostic& detail) {
  const auto& code = detail.code;
  return code == "io" || code == "path_denied" || code == "ledger_not_found" || code == "foreign_ledger" ||
         code == "unsupported_ledger_version" || code == "ledger_corrupt" || code == "ledger_busy" || code == "storage_full" ||
         code == "ledger_exists" || code == "hash_collision" || code == "request_conflict" || code == "revision_conflict" ||
         code == "revision_not_found" || code == "request_not_found" || code == "event_not_found" ||
         code == "unsupported_interpretation" || code == "target_exists" || code == "export_incomplete";
}
} // namespace
response registration_failed(const diagnostic& detail) {
  try {
    if (in_closed_pairs(detail) || is_native_passthrough(detail))
      return failed(native_error(detail));
    return extension_protocol_failed();
  } catch (...) {
    return {4, "", ""};
  }
}
response extension_usage_failed() {
  return usage();
}
response extension_protocol_failed() {
  try {
    return failed(native_error({"internal", "Invalid external profile protocol"}));
  } catch (...) {
    return {4, "", ""};
  }
}
response extension_resource_failed(const diagnostic& detail) {
  try {
    if (detail.code != "resource_limit" || !in_closed_pairs(detail))
      return extension_protocol_failed();
    return failed(native_error(detail));
  } catch (...) {
    return {4, "", ""};
  }
}
response semantic_failed(value_ptr error) {
  try {
    // The exact trusted inner error is passed through only when it is a well formed record; anything else is redacted.
    const auto field = [&](const char* name, value_kind kind) -> const value* {
      if (!error || error->kind != value_kind::record)
        return nullptr;
      const auto found = error->fields.find(name);
      return found == error->fields.end() || !found->second || found->second->kind != kind ? nullptr : found->second.get();
    };
    const auto* code    = field("code", value_kind::string);
    const auto* message = field("message", value_kind::string);
    const auto* where   = field("field", value_kind::string);
    if (!code || !message || !where || code->string.empty())
      return extension_protocol_failed();
    return failed(std::move(error));
  } catch (...) {
    return {4, "", ""};
  }
}
} // namespace planar::cmd::detail
namespace planar::cmd {
using namespace detail;
response stdout_failure() {
  try {
    return failed(object({{"code", text("io")},
                          {"message", text("Cannot write stdout")},
                          {"field", text("")},
                          {"details", object({{"operation", text("stdout")}})}}));
  } catch (...) {
    return {4, "", ""};
  }
}
} // namespace planar::cmd
