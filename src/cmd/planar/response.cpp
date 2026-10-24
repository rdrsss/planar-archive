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
