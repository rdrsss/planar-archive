// read_provenance: explicit single-file capture and strict driver_provenance_v1 normalization (P2-4, D146, L17).
module;
#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
module planar.engine.execution.provenance_input;

import std;
import planar.engine.capture;
import planar.engine.json_codec;
import planar.engine.history.execution_codec;

namespace planar::engine::execution {

namespace {

constexpr std::size_t      depth_maximum    = 4;
constexpr std::size_t      settings_maximum = 32;
constexpr std::size_t      string_maximum   = 1024;
constexpr std::string_view limit_message    = "Execution resource limit exceeded";

provenance_error limit(std::string_view name, std::uint64_t maximum) {
  return provenance_error{"resource_limit", std::string(limit_message), {}, std::string(name), maximum};
}

provenance_error schema(std::string message, std::string field) {
  return provenance_error{"schema", std::move(message), std::move(field), {}, 0};
}

provenance_error from_diagnostic(const diagnostic& error) {
  if (error.code == "resource_limit")
    return limit("provenance_bytes", provenance_file_maximum);
  return provenance_error{error.code, error.message, {}, {}, 0};
}

/// Container depth with the root counting 1.
std::size_t depth_of(const value& node) {
  if (node.kind != value_kind::record && node.kind != value_kind::list)
    return 0;
  std::size_t deepest = 0;
  for (const auto& [key, child] : node.fields)
    if (child)
      deepest = std::max(deepest, depth_of(*child));
  for (const auto& child : node.items)
    if (child)
      deepest = std::max(deepest, depth_of(*child));
  return deepest + 1;
}

bool oversized_string(const value& node) {
  if (node.kind == value_kind::string)
    return node.string.size() > string_maximum;
  for (const auto& [key, child] : node.fields)
    if (key.size() > string_maximum || (child && oversized_string(*child)))
      return true;
  for (const auto& child : node.items)
    if (child && oversized_string(*child))
      return true;
  return false;
}

/// Shape refusals carry the schema normalizer messages and the dotted path from the document root.
struct shape_failure {
  provenance_error error;
};

[[noreturn]] void shape(std::string message, std::string field) {
  throw shape_failure{schema(std::move(message), std::move(field))};
}

const value& object_at(const value& node, const std::string& field) {
  if (node.kind != value_kind::record)
    shape("Expected object", field);
  return node;
}

void keys(const value& node, std::initializer_list<std::string_view> names, const std::string& field) {
  for (const auto& [key, unused] : node.fields)
    if (std::ranges::find(names, std::string_view(key)) == names.end())
      shape("Unknown field", field + "." + key);
  for (auto name : names)
    if (!node.fields.contains(std::string(name)))
      shape("Required field", field + "." + std::string(name));
}

const value& child(const value& node, std::string_view key) {
  return *node.fields.at(std::string(key));
}

bool valid_leaf_string(const value& node) {
  return node.kind == value_kind::string && !node.string.empty() && node.string.find('\0') == std::string::npos &&
         valid_text(node.string);
}

void leaf(const value& node, const std::string& field, bool allow_unknown = true) {
  if (valid_leaf_string(node))
    return;
  if (allow_unknown && node.kind == value_kind::record && node.fields.size() == 1 && node.fields.contains("status")) {
    const auto& status = child(node, "status");
    if (status.kind == value_kind::string && status.string == "unknown")
      return;
  }
  shape("Invalid string", field);
}

void group(const value& document, std::string_view name, std::initializer_list<std::string_view> leaves) {
  const std::string field = "provenance." + std::string(name);
  const auto&       node  = object_at(child(document, name), field);
  keys(node, leaves, field);
  for (auto key : leaves)
    leaf(child(node, key), field + "." + std::string(key));
}

value_ptr validate_shape(const value_ptr& root) {
  const std::string field    = "provenance";
  const auto&       document = object_at(*root, field);
  keys(document, {"version", "driver", "trial", "view", "model", "session", "settings"}, field);
  const auto& version = child(document, "version");
  if (version.kind != value_kind::integer)
    shape("Expected version integer", field + ".version");
  if (version.integer != 1)
    shape("Unsupported schema version", field + ".version");
  group(document, "driver", {"name", "version"});
  group(document, "trial", {"id"});
  group(document, "view", {"id"});
  group(document, "model", {"requested", "actual"});
  group(document, "session", {"requested", "actual"});
  const auto& settings = child(document, "settings");
  if (settings.kind != value_kind::list)
    shape("Expected list", field + ".settings");
  std::set<std::string> names;
  for (std::size_t i = 0; i < settings.items.size(); ++i) {
    const std::string at   = std::format("{}.settings.{}", field, i);
    const auto&       item = object_at(*settings.items[i], at);
    keys(item, {"name", "value"}, at);
    // A setting name is a string leaf, never the unknown object.
    leaf(child(item, "name"), at + ".name", false);
    leaf(child(item, "value"), at + ".value");
    if (!names.insert(child(item, "name").string).second)
      shape("Duplicate value", at + ".name");
  }
  // Normalization: settings sorted by name in unsigned byte order.
  auto                   copy   = std::make_shared<value>(document);
  std::vector<value_ptr> sorted = settings.items;
  std::ranges::sort(
      sorted, [](const value_ptr& a, const value_ptr& b) { return a->fields.at("name")->string < b->fields.at("name")->string; });
  value list{};
  list.kind                = value_kind::list;
  list.items               = std::move(sorted);
  copy->fields["settings"] = std::make_shared<const value>(std::move(list));
  return copy;
}

/// Closes the gate descriptor on every path.
struct descriptor {
  int fd = -1;
  ~descriptor() {
    if (fd >= 0)
      ::close(fd);
  }
};

} // namespace

std::expected<value_ptr, provenance_error> parse_provenance(std::string_view bytes) {
  if (bytes.size() > provenance_file_maximum)
    return std::unexpected(limit("provenance_bytes", provenance_file_maximum));
  // (2) Grammar: Glaze strict parse; depth and node ceilings far above the provenance bounds only guard the parser.
  auto decoded = codec_decode(bytes, codec_limits{provenance_file_maximum, 64, 100000});
  if (!decoded) {
    if (decoded.error().detail.code == "resource_limit" && decoded.error().detail.limit == "json_depth")
      return std::unexpected(limit("provenance_depth", depth_maximum));
    return std::unexpected(schema("Invalid JSON", "provenance"));
  }
  const auto& root = **decoded;
  // (3) Depth, root counting 1.
  if (depth_of(root) > depth_maximum)
    return std::unexpected(limit("provenance_depth", depth_maximum));
  // (4) Settings count.
  if (root.kind == value_kind::record) {
    const auto settings = root.fields.find("settings");
    if (settings != root.fields.end() && settings->second && settings->second->kind == value_kind::list &&
        settings->second->items.size() > settings_maximum)
      return std::unexpected(limit("provenance_settings", settings_maximum));
  }
  // (5) Strings, so a 1025-byte leaf is resource_limit and never schema.
  if (oversized_string(root))
    return std::unexpected(limit("provenance_string_bytes", string_maximum));
  // (6) Shape and leaf rules.
  try {
    auto normalized = validate_shape(*decoded);
    // The stored record codec applies the same bounds and shape; a refusal here would be a codec disagreement.
    if (!history::encode_driver_provenance_v1(normalized))
      return std::unexpected(schema("Invalid JSON", "provenance"));
    return normalized;
  } catch (const shape_failure& failure) {
    return std::unexpected(failure.error);
  }
}

std::expected<value_ptr, provenance_error> read_provenance(const std::filesystem::path& path) {
  const std::string spelled = path.string();
  {
    // (1) Raw size from the no-follow descriptor before any read (L17). O_NONBLOCK keeps a FIFO from blocking.
    descriptor gate{::open(spelled.c_str(), O_RDONLY | O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC)};
    if (gate.fd < 0) {
      if (errno == ELOOP)
        return std::unexpected(provenance_error{"path_denied", "Explicit input file is a symbolic link", {}, {}, 0});
      return std::unexpected(provenance_error{"io", "Cannot resolve explicit input path", {}, {}, 0});
    }
    struct stat status{};
    if (::fstat(gate.fd, &status))
      return std::unexpected(provenance_error{"io", "Cannot resolve explicit input path", {}, {}, 0});
    if (!S_ISREG(status.st_mode))
      return std::unexpected(provenance_error{"io", "Explicit input must be a regular file", {}, {}, 0});
    if (status.st_size < 0 || static_cast<std::uint64_t>(status.st_size) > provenance_file_maximum)
      return std::unexpected(limit("provenance_bytes", provenance_file_maximum));
  }
  // The shared single-file capture reads once and detects a change between the gate and the end of the read.
  capture_session session;
  auto            captured = capture_diagnostic(session, spelled, provenance_file_maximum);
  if (!captured)
    return std::unexpected(from_diagnostic(captured.error()));
  return parse_provenance(captured->bytes);
}

} // namespace planar::engine::execution
