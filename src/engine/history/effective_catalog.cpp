module planar.engine.history.effective_catalog;
import std;
import planar.engine.canonical;
namespace planar::engine::history {
namespace {
// Root:9; each command:25 +32*9 option nodes +3 capability strings;
// each shape/projection:21. Includes every map key and owner. Deepest leaf is
// root/list/entry/descriptor/options/option/field =7. Three is the known capability
// vocabulary ceiling (static relationships further restrict successful sets).
constexpr canonical_limits limits{.depth = 7, .nodes = 9 + 64 * (25 + 32 * 9 + 3) + 128 * 21, .bytes = 1048576};
struct checker {
  std::string       field = "catalog";
  [[noreturn]] void fail(catalog_error_code code = catalog_error_code::invalid_catalog) const {
    throw catalog_error{code, field};
  }
  void exact(const value_ptr& v, std::initializer_list<std::string_view> keys) const {
    if (!v || v->kind != value_kind::record || v->fields.size() != keys.size())
      fail();
    for (auto key : keys)
      if (!v->fields.contains(std::string(key)))
        fail();
  }
  const value_ptr& get(const value_ptr& v, std::string_view key) const {
    return v->fields.at(std::string(key));
  }
  std::string_view text(const value_ptr& v) const {
    if (!v || v->kind != value_kind::string)
      fail();
    return v->string;
  }
  std::string_view text(const value_ptr& v, std::string_view key) const {
    return text(get(v, key));
  }
  const std::vector<value_ptr>& list(const value_ptr& v, std::size_t maximum) const {
    if (!v || v->kind != value_kind::list)
      fail();
    if (v->items.size() > maximum)
      fail(catalog_error_code::resource_limit);
    return v->items;
  }
  bool boolean(const value_ptr& v) const {
    if (!v || v->kind != value_kind::boolean)
      fail();
    return v->boolean;
  }
  static bool grammar(std::string_view s, bool hyphens = false, std::size_t maximum = 64) {
    if (s.empty() || s.size() > maximum || s[0] < 'a' || s[0] > 'z')
      return false;
    for (char c : s)
      if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || (hyphens && c == '-')))
        return false;
    return true;
  }
  static bool handler(std::string_view s) {
    if (s.size() > 128 || s.find('.') == s.npos)
      return false;
    while (true) {
      auto n = s.find('.');
      if (!grammar(s.substr(0, n), false, 128))
        return false;
      if (n == s.npos)
        return true;
      s.remove_prefix(n + 1);
    }
  }
  void help(const value_ptr& v) const {
    auto s = text(v, "help");
    if (s.size() > 4096)
      fail(catalog_error_code::resource_limit);
    if (!valid_text(s))
      fail();
    for (unsigned char c : s)
      if ((c < 32 && c != '\n' && c != '\t') || c == 127)
        fail();
  }
  void sorted(std::string_view name, std::string_view& previous, bool& first) const {
    if (!first && name <= previous)
      fail(catalog_error_code::identity_conflict);
    first    = false;
    previous = name;
  }
  void paths(std::span<const std::string_view> paths, bool external) const {
    if (paths.size() > (external ? 64u : 4094u))
      fail(catalog_error_code::resource_limit);
    std::set<std::string>                   seen;
    std::map<std::string, std::string_view> spellings;
    for (auto p : paths) {
      if (!valid_path(p))
        fail();
      auto key = std::string(p);
      for (char& c : key)
        if (c >= 'A' && c <= 'Z')
          c += 'a' - 'A';
      if (!seen.insert(key).second)
        fail(catalog_error_code::identity_conflict);
      if (!external && (key == "__planar__" || key.starts_with("__planar__/")))
        fail();
      if (external) {
        auto top = key.substr(0, key.find('/'));
        if (!p.ends_with(".lua") || top == "planar" || top == "__planar__")
          fail();
      }
      for (std::size_t at = 0;;) {
        at          = p.find('/', at);
        auto prefix = p.substr(0, at);
        auto folded = std::string(prefix);
        for (char& ch : folded)
          if (ch >= 'A' && ch <= 'Z')
            ch += 'a' - 'A';
        auto [it, inserted] = spellings.emplace(std::move(folded), prefix);
        if (!inserted && it->second != prefix)
          fail(catalog_error_code::identity_conflict);
        if (at == p.npos)
          break;
        ++at;
      }
    }
  }
  bool owner(const value_ptr& entry, std::span<const std::string_view> shipped, std::span<const std::string_view> external,
             bool command) const {
    auto o = get(entry, "owner");
    exact(o, {"tier", "module", "ordinal"});
    auto tier = text(o, "tier"), module = text(o, "module");
    auto ordinal = get(o, "ordinal");
    if (ordinal->kind != value_kind::integer || ordinal->integer < 1)
      fail();
    bool is_shipped = tier == "shipped";
    if (!is_shipped && tier != "external")
      fail();
    auto manifest = is_shipped ? shipped : external;
    auto index    = static_cast<std::size_t>(ordinal->integer - 1);
    auto prefix   = is_shipped ? std::string_view("shipped/") : std::string_view("external/");
    if (index >= manifest.size() || !module.starts_with(prefix) || module.substr(prefix.size()) != manifest[index])
      fail();
    if (command && is_shipped && module != "shipped/planar/commands.lua")
      fail();
    return is_shipped;
  }
};
struct signature {
  std::string_view name, type;
  bool             required;
};
std::vector<signature> role_signature() {
  return {{"--workspace", "text", true},
          {"--registry", "text", true},
          {"--root", "text", true},
          {"--job", "text", true},
          {"--role", "text", true},
          {"--base-root", "text", false},
          {"--observations", "text", false},
          {"--budget-bytes", "decimal", false},
          {"--capture-outputs", "flag", false}};
}
std::vector<signature> history_signature(std::string_view id) {
  return {{"--ledger", "text", true}, {"--request-id", "text", false}, {"--revision", id, false},
          {"--at-event", id, false},  {"--after-event", id, false},    {"--limit", "decimal", false}};
}
std::vector<signature> shipped_signature(std::string_view name) {
  if (name == "render")
    return role_signature();
  if (name == "history")
    return history_signature("decimal");
  if (name == "init")
    return {{"--ledger", "text", true}, {"--workspace-id", "text", true}};
  if (name == "export")
    return {{"--ledger", "text", true}, {"--target", "text", true}, {"--revision", "decimal", true}};
  if (name == "load")
    return {{"--ledger", "text", true},          {"--workspace", "text", true},  {"--registry", "text", true},
            {"--expect-revision", "text", true}, {"--request-id", "text", true}, {"--actor", "text", true}};
  return {};
}
void validate(const value_ptr& root, std::span<const std::string_view> shipped, std::span<const std::string_view> external) {
  checker c;
  c.paths(shipped, false);
  c.paths(external, true);
  c.exact(root, {"version", "commands", "shapes", "projections"});
  auto version = c.get(root, "version");
  if (version->kind != value_kind::integer || version->integer != 2)
    c.fail();
  std::set<std::string_view>                         handlers, mandatory;
  const std::map<std::string_view, std::string_view> native{{"render", "render_capture_v1"},
                                                            {"init", "initialize_ledger_v1"},
                                                            {"history", "read_history_v1"},
                                                            {"export", "export_source_v1"},
                                                            {"load", "accept_load_v1"}};
  for (auto kind : {"commands", "shapes", "projections"}) {
    std::string_view previous;
    bool             first = true;
    std::size_t      index = 0;
    for (auto& entry : c.list(c.get(root, kind), 64)) {
      c.field = std::format("catalog.{}[{}]", kind, ++index);
      c.exact(entry, {"descriptor", "owner"});
      auto d       = c.get(entry, "descriptor");
      bool command = std::string_view(kind) == "commands", shape = std::string_view(kind) == "shapes";
      bool is_shipped = c.owner(entry, shipped, external, command);
      if (command)
        c.exact(d, {"name", "family", "handler_id", "options", "help", "response_kind", "capabilities"});
      else if (shape)
        c.exact(d, {"id", "abi", "revision", "stages", "help"});
      else
        c.exact(d, {"id", "abi", "revision", "input", "help"});
      auto name = c.text(d, command ? "name" : "id");
      if (!checker::grammar(name, command))
        c.fail();
      c.sorted(name, previous, first);
      c.help(d);
      if (!command) {
        if (!checker::grammar(c.text(d, "revision")) || c.text(d, "abi") != (shape ? "shape_v1" : "projection_v1"))
          c.fail();
        auto mode = c.text(d, shape ? "stages" : "input");
        if (shape ? (mode != "required" && mode != "forbidden" && mode != "optional")
                  : (mode != "role_context_v1" && mode != "history_snapshot_v1"))
          c.fail();
        bool reserved = shape ? (name == "pipeline" || name == "fanout_review")
                              : (name == "worker" || name == "reviewer" || name == "recovery");
        if (reserved && !is_shipped)
          c.fail(catalog_error_code::identity_conflict);
        continue;
      }
      auto family = c.text(d, "family"), handler = c.text(d, "handler_id"), response = c.text(d, "response_kind");
      if (!checker::grammar(family, true) || !checker::handler(handler))
        c.fail();
      if (!handlers.insert(handler).second)
        c.fail(catalog_error_code::identity_conflict);
      std::set<std::string_view> caps;
      std::string_view           cap_previous;
      bool                       cap_first = true;
      for (auto& cap : c.list(c.get(d, "capabilities"), 3)) {
        auto s = c.text(cap);
        c.sorted(s, cap_previous, cap_first);
        caps.insert(s);
      }
      std::vector<signature> expected;
      if (is_shipped) {
        if (!native.contains(name) || family != name || handler != std::string("shipped.") + std::string(name) + ".v1" ||
            response != std::string(name) + "_v1" || caps.size() != 1 || !caps.contains(native.at(name)))
          c.fail();
        mandatory.insert(name);
        expected = shipped_signature(name);
      } else {
        if (name == "with-profile" || native.contains(name) || native.contains(family) ||
            std::ranges::any_of(
                native, [&](const auto& pair) { return handler == std::string("shipped.") + std::string(pair.first) + ".v1"; }))
          c.fail(catalog_error_code::identity_conflict);
        if (family != "extension_read_v1" || response != "extension_v1")
          c.fail();
        for (auto cap : caps)
          if (cap != "role_context_read_v1" && cap != "history_read_v1" && cap != "projection_apply_v1")
            c.fail();
        bool role = caps.contains("role_context_read_v1"), history = caps.contains("history_read_v1");
        if ((role && history) || (caps.contains("projection_apply_v1") && !role && !history))
          c.fail();
        if (role)
          expected = role_signature();
        if (history)
          expected = history_signature("history_id");
      }
      auto& options = c.list(c.get(d, "options"), 32);
      if (is_shipped && options.size() != expected.size())
        c.fail();
      std::set<std::string_view> found;
      std::string_view           opt_previous;
      bool                       opt_first = true;
      for (auto& option : options) {
        c.exact(option, {"name", "type", "required", "help"});
        auto n = c.text(option, "name"), type = c.text(option, "type");
        if (!n.starts_with("--") || !checker::grammar(n.substr(2), true))
          c.fail();
        if (n == "--help" || n == "--stdlib-profile")
          c.fail(catalog_error_code::identity_conflict);
        c.sorted(n, opt_previous, opt_first);
        c.help(option);
        bool required = c.boolean(c.get(option, "required"));
        if ((type != "text" && type != "decimal" && type != "flag" && type != "history_id") || (type == "flag" && required))
          c.fail();
        auto it = std::ranges::find(expected, n, &signature::name);
        if (it != expected.end()) {
          if (type != it->type || required != it->required)
            c.fail();
          found.insert(n);
        } else {
          if (is_shipped || type == "history_id")
            c.fail();
          // Only commands with a read capability interpret recognized native host options.
          if (!expected.empty() && (n == "--workspace" || n == "--registry" || n == "--root" || n == "--base-root" ||
                                    n == "--observations" || n == "--ledger" || n == "--target"))
            c.fail();
        }
      }
      if (found.size() != expected.size())
        c.fail();
    }
    c.field = "catalog";
  }
  if (mandatory.size() != 5)
    c.fail();
}
catalog_error codec_error(const diagnostic& error) {
  return {error.code == "resource_limit" ? catalog_error_code::resource_limit : catalog_error_code::invalid_catalog, "catalog"};
}
} // namespace
validated_catalog::validated_catalog(value_ptr value, std::string bytes) : _value(std::move(value)), _bytes(std::move(bytes)) {
}
const value_ptr& validated_catalog::data() const {
  return _value;
}
std::string_view validated_catalog::bytes() const {
  return _bytes;
}
std::expected<validated_catalog, catalog_error> decode_catalog(std::string_view bytes, std::span<const std::string_view> shipped,
                                                               std::span<const std::string_view> external) {
  if (bytes.size() > limits.bytes)
    return std::unexpected(catalog_error{catalog_error_code::resource_limit, "catalog"});
  auto decoded = canonical_decode(bytes, limits);
  if (!decoded)
    return std::unexpected(codec_error(decoded.error()));
  try {
    validate(*decoded, shipped, external);
  } catch (const catalog_error& error) {
    return std::unexpected(error);
  }
  return validated_catalog(std::move(*decoded), std::string(bytes));
}
std::expected<validated_catalog, catalog_error>
validate_catalog(const value_ptr& input, std::span<const std::string_view> shipped, std::span<const std::string_view> external) {
  // Bound canonical traversal before structural scratch or retaining any caller strings.
  auto bytes = canonical_encode(input, limits);
  if (!bytes)
    return std::unexpected(codec_error(bytes.error()));
  return decode_catalog(*bytes, shipped, external);
}
} // namespace planar::engine::history
