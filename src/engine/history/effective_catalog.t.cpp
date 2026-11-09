// Catalog-v2 native structural sealing, immutable ownership and bounded pure validation.
#include <catch2/catch_test_macros.hpp>
import std;
import planar.engine.history.effective_catalog;
import planar.engine.canonical;
using namespace planar::engine;
using namespace planar::engine::history;
namespace {
using mutable_value = std::shared_ptr<value>;
mutable_value str(std::string_view s) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = s;
  return v;
}
mutable_value num(int n) {
  auto v     = std::make_shared<value>();
  v->kind    = value_kind::integer;
  v->integer = n;
  return v;
}
mutable_value flag(bool b) {
  auto v     = std::make_shared<value>();
  v->kind    = value_kind::boolean;
  v->boolean = b;
  return v;
}
mutable_value obj(std::map<std::string, value_ptr> fields) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::record;
  v->fields = std::move(fields);
  return v;
}
mutable_value arr(std::vector<value_ptr> items = {}) {
  auto v   = std::make_shared<value>();
  v->kind  = value_kind::list;
  v->items = std::move(items);
  return v;
}
mutable_value mut(const value_ptr& v) {
  return std::const_pointer_cast<value>(v);
}
mutable_value at(const value_ptr& v, std::string key) {
  return mut(v->fields.at(key));
}
mutable_value opt(std::string name, std::string type = "text", bool required = false) {
  return obj({{"name", str(name)}, {"type", str(type)}, {"required", flag(required)}, {"help", str("option")}});
}
mutable_value owner(bool shipped = true, int ordinal = 2) {
  return obj({{"tier", str(shipped ? "shipped" : "external")},
              {"module",
               str(shipped ? (ordinal == 1 ? "shipped/planar/json.lua" : "shipped/planar/commands.lua") : "external/ext/a.lua")},
              {"ordinal", num(ordinal)}});
}
mutable_value entry(mutable_value descriptor, bool shipped = true, int ordinal = 2) {
  return obj({{"descriptor", descriptor}, {"owner", owner(shipped, ordinal)}});
}
mutable_value descriptor(const value_ptr& v) {
  return at(v, "descriptor");
}
mutable_value command(std::string name, std::vector<value_ptr> options, std::string cap, bool shipped = true) {
  std::ranges::sort(options, {}, [](const auto& o) { return o->fields.at("name")->string; });
  return entry(obj({{"name", str(name)},
                    {"family", str(shipped ? name : "extension_read_v1")},
                    {"handler_id", str(shipped ? "shipped." + name + ".v1" : "custom." + name + ".v1")},
                    {"options", arr(std::move(options))},
                    {"help", str("help")},
                    {"response_kind", str(shipped ? name + "_v1" : "extension_v1")},
                    {"capabilities", arr(cap.empty() ? std::vector<value_ptr>{} : std::vector<value_ptr>{str(cap)})}}),
               shipped, shipped ? 2 : 1);
}
std::vector<value_ptr> role() {
  return {opt("--workspace", "text", true),
          opt("--registry", "text", true),
          opt("--root", "text", true),
          opt("--job", "text", true),
          opt("--role", "text", true),
          opt("--base-root"),
          opt("--observations"),
          opt("--budget-bytes", "decimal"),
          opt("--capture-outputs", "flag")};
}
std::vector<value_ptr> history_options(std::string id = "decimal") {
  return {opt("--ledger", "text", true), opt("--request-id"),      opt("--revision", id),
          opt("--at-event", id),         opt("--after-event", id), opt("--limit", "decimal")};
}
mutable_value catalog() {
  std::vector<value_ptr> commands;
  auto add = [&](std::string name, std::vector<value_ptr> options, std::vector<std::string> caps, std::string response = "",
                 std::string handler = "") {
    auto                   e = command(name, std::move(options), "");
    auto                   d = descriptor(e);
    std::vector<value_ptr> values;
    std::ranges::sort(caps);
    for (auto& cap : caps)
      values.push_back(str(cap));
    d->fields["capabilities"] = arr(std::move(values));
    if (!response.empty())
      d->fields["response_kind"] = str(response);
    if (!handler.empty())
      d->fields["handler_id"] = str(handler);
    commands.push_back(e);
  };
  auto required = [](std::initializer_list<std::string_view> names) {
    std::vector<value_ptr> out;
    for (auto name : names)
      out.push_back(opt(std::string(name), "text", true));
    return out;
  };
  add("render", role(), {"render_capture_v1"});
  auto history = history_options();
  history.push_back(opt("--event", "decimal"));
  add("history", history, {"read_history_v1"});
  add("init", required({"--ledger", "--workspace-id"}), {"initialize_ledger_v1"});
  add("export", {opt("--ledger", "text", true), opt("--target", "text", true), opt("--revision", "decimal", true)},
      {"export_source_v1"});
  add("load", required({"--ledger", "--workspace", "--registry", "--expect-revision", "--request-id", "--actor"}),
      {"accept_load_v1"});
  add("activate", required({"--ledger", "--job", "--expect-revision", "--expect-job", "--request-id", "--actor"}),
      {"execution_commit_v1"});
  add("status", {opt("--ledger", "text", true), opt("--role"), opt("--request-id"), opt("--workspace"), opt("--registry")},
      {"draft_capture_v1", "execution_read_v1"});
  add("preview", {opt("--ledger", "text", true), opt("--role", "text", true), opt("--since")}, {"execution_read_v1"});
  for (auto name : {"claim", "takeover"}) {
    auto o = required({"--ledger", "--job", "--role", "--request-id", "--actor"});
    o.push_back(opt("--provenance"));
    if (std::string_view(name) == "takeover")
      o.push_back(opt("--run-id", "text", true));
    add(name, o, {"execution_commit_v1", "provenance_read_v1"}, "claim_v1");
  }
  for (auto name : {"refresh", "checkpoint", "release"}) {
    auto o = required({"--ledger", "--run", "--request-id", "--actor"});
    o.push_back(opt("--observations", "text", std::string_view(name) == "checkpoint"));
    if (std::string_view(name) == "refresh")
      o.push_back(opt("--expect-fp", "text", true));
    if (std::string_view(name) == "release") {
      o.push_back(opt("--reason", "text", true));
      o.push_back(opt("--provenance"));
    }
    add(name, o, {"execution_commit_v1"});
  }
  add("recover", {opt("--ledger", "text", true), opt("--role", "text", true), opt("--root"), opt("--base-root")},
      {"execution_read_v1"});
  auto checks = required({"--ledger", "--run", "--root", "--base-root", "--scratch", "--request-id", "--actor"});
  checks.push_back(opt("--provenance"));
  add("check", checks, {"check_run_v1", "execution_commit_v1", "provenance_read_v1"});
  add("check-recover", required({"--ledger", "--run-id", "--reservation", "--confirm-stopped", "--request-id", "--actor"}),
      {"execution_commit_v1"}, "check_v1", "shipped.check_recover.v1");
  add("verdict", required({"--ledger", "--run", "--verdict", "--request-id", "--actor"}),
      {"execution_commit_v1", "verdict_read_v1"}, "verdict_response_v1");
  std::ranges::sort(commands, {}, [](const auto& e) { return descriptor(e)->fields.at("name")->string; });
  return obj({{"version", num(2)}, {"commands", arr(commands)}, {"shapes", arr()}, {"projections", arr()}});
}
constexpr std::array<std::string_view, 2> shipped{"planar/json.lua", "planar/commands.lua"};
constexpr std::array<std::string_view, 1> external{"ext/a.lua"};
auto                                      check(const value_ptr& v) {
  return validate_catalog(v, shipped, external);
}
mutable_value add_command(mutable_value root, mutable_value e) {
  auto list = at(root, "commands");
  list->items.push_back(e);
  std::ranges::sort(list->items, {}, [](const auto& v) { return descriptor(v)->fields.at("name")->string; });
  return e;
}
mutable_value custom(mutable_value root, std::string cap = "") {
  return descriptor(add_command(root, command("zextra", {}, cap, false)));
}
mutable_value shape(std::string id, bool shipped = false) {
  return entry(obj({{"id", str(id)},
                    {"abi", str("shape_v1")},
                    {"revision", str("r1")},
                    {"stages", str("optional")},
                    {"help", str("shape")}}),
               shipped, shipped ? 1 : 1);
}
mutable_value projection(std::string id, std::string input = "role_context_v1", bool shipped = false) {
  return entry(obj({{"id", str(id)},
                    {"abi", str("projection_v1")},
                    {"revision", str("r1")},
                    {"input", str(input)},
                    {"help", str("projection")}}),
               shipped, 1);
}
void refused(const value_ptr& v) {
  auto r = check(v);
  REQUIRE_FALSE(r);
  REQUIRE(r.error().field.starts_with("catalog"));
}
} // namespace
TEST_CASE("replacement.effective catalog roundtrip and immutable aliases") {
  auto v      = catalog();
  auto help   = at(descriptor(at(v, "commands")->items[0]), "help");
  auto result = check(v);
  REQUIRE(result);
  auto bytes   = std::string(result->bytes());
  help->string = "changed";
  at(v, "commands")->items.clear();
  REQUIRE(result->bytes() == bytes);
  auto encoded = canonical_encode(result->data());
  REQUIRE(encoded);
  REQUIRE(*encoded == bytes);
  auto decoded = decode_catalog(bytes, shipped, external);
  REQUIRE(decoded);
  REQUIRE(decoded->bytes() == bytes);
}
TEST_CASE("replacement.effective catalog mandatory shipped signatures") {
  for (std::size_t i = 0; i < 17; ++i) {
    auto v = catalog();
    at(v, "commands")->items.erase(at(v, "commands")->items.begin() + i);
    refused(v);
    for (auto key : {"family", "handler_id", "response_kind"}) {
      v                                                        = catalog();
      at(descriptor(at(v, "commands")->items[i]), key)->string = "incorrect";
      refused(v);
    }
    v                                                                  = catalog();
    at(descriptor(at(v, "commands")->items[i]), "capabilities")->items = {str("history_read_v1")};
    refused(v);
    for (bool extra : {false, true}) {
      v         = catalog();
      auto caps = at(descriptor(at(v, "commands")->items[i]), "capabilities");
      if (extra)
        caps->items.push_back(str("zzz_extra_v1"));
      else
        caps->items.pop_back();
      refused(v);
      v            = catalog();
      auto options = at(descriptor(at(v, "commands")->items[i]), "options");
      if (extra)
        options->items.push_back(opt("--zzz-extra"));
      else
        options->items.pop_back();
      refused(v);
    }
    auto count = at(descriptor(at(catalog(), "commands")->items[i]), "options")->items.size();
    for (std::size_t j = 0; j < count; ++j) {
      v                     = catalog();
      auto o                = at(descriptor(at(v, "commands")->items[i]), "options")->items[j];
      at(o, "type")->string = "history_id";
      refused(v);
      v                          = catalog();
      o                          = at(descriptor(at(v, "commands")->items[i]), "options")->items[j];
      at(o, "required")->boolean = !at(o, "required")->boolean;
      refused(v);
    }
  }
}
TEST_CASE("replacement.effective catalog external descriptors and reserved namespaces") {
  auto v                      = catalog();
  auto d                      = custom(v);
  at(d, "options")->items     = {opt("--ledger"), opt("--uninterpreted-path")};
  at(v, "shapes")->items      = {shape("custom"), shape("pipeline", true)};
  at(v, "projections")->items = {projection("custom"), projection("history", "history_snapshot_v1"),
                                 projection("worker", "role_context_v1", true)};
  REQUIRE(check(v));
  for (auto id : {"pipeline", "fanout_review"}) {
    v                      = catalog();
    at(v, "shapes")->items = {shape(id)};
    refused(v);
  }
  for (auto id : {"worker", "reviewer", "recovery"}) {
    v                           = catalog();
    at(v, "projections")->items = {projection(id)};
    refused(v);
  }
  v                      = catalog();
  at(v, "shapes")->items = {shape("extra_shipped", true)};
  REQUIRE(check(v));
  v                           = catalog();
  d                           = custom(v);
  at(d, "handler_id")->string = "shipped.custom.v1";
  REQUIRE(check(v));
}
TEST_CASE("replacement.effective catalog static read capability signatures") {
  for (bool role_read : {false, true}) {
    auto v       = catalog();
    auto d       = custom(v, role_read ? "role_context_read_v1" : "history_read_v1");
    auto options = role_read ? role() : history_options("history_id");
    std::ranges::sort(options, {}, [](auto& o) { return at(o, "name")->string; });
    at(d, "options")->items = options;
    REQUIRE(check(v));
    at(d, "capabilities")->items.insert(at(d, "capabilities")->items.begin(), str("projection_apply_v1"));
    std::ranges::sort(at(d, "capabilities")->items, {}, [](auto& x) { return x->string; });
    REQUIRE(check(v));
    at(v, "projections")->items = {projection("role"), projection("snapshot", "history_snapshot_v1")};
    REQUIRE(check(v));
    auto count = options.size();
    for (std::size_t j = 0; j < count; ++j) {
      auto old = at(d, "options")->items;
      at(d, "options")->items.erase(at(d, "options")->items.begin() + j);
      refused(v);
      at(d, "options")->items    = old;
      auto o                     = options[j];
      at(o, "required")->boolean = !at(o, "required")->boolean;
      refused(v);
      at(o, "required")->boolean = !at(o, "required")->boolean;
    }
    auto extra = opt("--zzdata", "decimal");
    at(d, "options")->items.push_back(extra);
    REQUIRE(check(v));
    at(extra, "name")->string = "--zpath";
    at(extra, "type")->string = "text";
    REQUIRE(check(v));
    at(extra, "name")->string = role_read ? "--ledger" : "--workspace";
    std::ranges::sort(at(d, "options")->items, {}, [](auto& x) { return at(x, "name")->string; });
    refused(v);
  }
  for (auto caps :
       std::vector<std::vector<value_ptr>>{{str("projection_apply_v1")},
                                           {str("history_read_v1"), str("role_context_read_v1")},
                                           {str("history_read_v1"), str("projection_apply_v1"), str("role_context_read_v1")},
                                           {str("unknown")}}) {
    auto v                               = catalog();
    at(custom(v), "capabilities")->items = caps;
    refused(v);
  }
}
TEST_CASE("replacement.effective catalog history id capability matrix") {
  for (auto name : {"--revision", "--at-event", "--after-event", "--other"})
    for (auto cap : {"", "role_context_read_v1", "history_read_v1"})
      for (bool required : {false, true}) {
        auto v       = catalog();
        auto d       = custom(v, cap);
        auto options = std::string_view(cap) == "history_read_v1"        ? history_options("history_id")
                       : std::string_view(cap) == "role_context_read_v1" ? role()
                                                                         : std::vector<value_ptr>{};
        auto found   = std::ranges::find_if(options, [&](auto& o) { return at(o, "name")->string == name; });
        if (found == options.end())
          options.push_back(opt(name, "history_id", required));
        else
          *found = opt(name, "history_id", required);
        std::ranges::sort(options, {}, [](auto& o) { return at(o, "name")->string; });
        at(d, "options")->items = options;
        bool valid              = std::string_view(cap) == "history_read_v1" && std::string_view(name) != "--other" && !required;
        REQUIRE(static_cast<bool>(check(v)) == valid);
      }
  auto v       = catalog();
  auto d       = custom(v, "history_read_v1");
  auto options = history_options();
  std::ranges::sort(options, {}, [](auto& o) { return at(o, "name")->string; });
  at(d, "options")->items = options;
  refused(v);
}
TEST_CASE("replacement.effective catalog source ownership and ordinals") {
  for (auto kind : {"commands", "shapes", "projections"}) {
    auto v = catalog();
    if (std::string_view(kind) == "shapes")
      at(v, kind)->items = {shape("s")};
    if (std::string_view(kind) == "projections")
      at(v, kind)->items = {projection("p")};
    auto o = at(at(v, kind)->items[0], "owner");
    for (auto ordinal : {0, 3, 2147483647}) {
      auto old                  = at(o, "ordinal")->integer;
      at(o, "ordinal")->integer = ordinal;
      refused(v);
      at(o, "ordinal")->integer = old;
    }
    auto module    = at(o, "module");
    auto old       = module->string;
    module->string = "external/private-host-secret.lua";
    auto r         = check(v);
    REQUIRE_FALSE(r);
    REQUIRE(r.error().field.find("secret") == std::string::npos);
    module->string     = old;
    o->fields["extra"] = str("bad");
    refused(v);
  }
  auto v                    = catalog();
  auto o                    = at(at(v, "commands")->items[0], "owner");
  at(o, "ordinal")->integer = 1;
  at(o, "module")->string   = "shipped/planar/json.lua";
  refused(v);
  for (auto paths : std::vector<std::vector<std::string_view>>{{"ext/a.lua", "ext/a.lua"},
                                                               {"ext/a.lua", "EXT/b.lua"},
                                                               {"planar/a.lua"},
                                                               {"__PLANAR__/a.lua"},
                                                               {"../a.lua"},
                                                               {"ext/a.LUA"}})
    REQUIRE_FALSE(validate_catalog(catalog(), shipped, paths));
  REQUIRE_FALSE(validate_catalog(catalog(), std::array<std::string_view, 2>{"planar/commands.lua", "planar/json.lua"}, external));
}
TEST_CASE("replacement.effective catalog closed schemas and scalar kinds") {
  for (auto level : {0, 1, 2, 3, 4})
    for (auto bad : {0, 1, 2}) {
      auto          v      = catalog();
      mutable_value target = v;
      if (level > 0)
        target = mut(at(v, "commands")->items[0]);
      if (level > 1)
        target = descriptor(target);
      if (level > 2)
        target = mut(at(target, "options")->items[0]);
      if (level > 3)
        target = at(at(v, "commands")->items[0], "owner");
      if (bad == 0)
        target->fields["unknown"] = str("bad");
      if (bad == 1)
        target->fields.erase(target->fields.begin());
      if (bad == 2)
        target->kind = value_kind::list;
      refused(v);
    }
  for (auto kind : {"shapes", "projections"})
    for (auto key : {"id", "abi", "revision", "help"}) {
      auto v                     = catalog();
      auto e                     = std::string_view(kind) == "shapes" ? shape("a") : projection("a");
      at(v, kind)->items         = {e};
      descriptor(e)->fields[key] = num(1);
      refused(v);
    }
  for (auto id : {"Bad", "hyphen-id", "1bad", "", "tool.long"}) {
    auto v                 = catalog();
    at(v, "shapes")->items = {shape(id)};
    refused(v);
  }
  for (auto h : {"single", "bad..name", "Bad.name", "bad.name-"}) {
    auto v                              = catalog();
    at(custom(v), "handler_id")->string = h;
    auto result                         = check(v);
    REQUIRE_FALSE(result);
    REQUIRE(result.error().code == catalog_error_code::invalid_catalog);
  }
  for (auto key : {"family", "handler_id", "response_kind", "name", "capabilities", "options", "help"}) {
    auto root      = catalog();
    auto d         = custom(root);
    d->fields[key] = flag(false);
    refused(root);
  }
  for (auto key : {"name", "type", "required", "help"}) {
    auto root               = catalog();
    auto d                  = custom(root);
    auto o                  = opt("--a");
    at(d, "options")->items = {o};
    o->fields[key]          = num(1);
    refused(root);
  }
  for (auto key : {"tier", "module", "ordinal"}) {
    auto root                                                = catalog();
    at(at(root, "commands")->items[0], "owner")->fields[key] = flag(false);
    refused(root);
  }
  for (auto kind : {"shapes", "projections"}) {
    auto root             = catalog();
    auto e                = std::string_view(kind) == "shapes" ? shape("s") : projection("p");
    at(root, kind)->items = {e};
    descriptor(e)->fields[std::string_view(kind) == "shapes" ? "stages" : "input"] = str("invalid");
    refused(root);
    descriptor(e)->fields["unexpected"] = str("bad");
    refused(root);
  }
  auto v                    = catalog();
  at(v, "version")->integer = 1;
  refused(v);
  v                       = catalog();
  auto d                  = custom(v);
  at(d, "options")->items = {opt("--a", "flag", true)};
  refused(v);
}
TEST_CASE("replacement.effective catalog ordering and identity conflicts") {
  for (auto kind : {"commands", "shapes", "projections"}) {
    auto v = catalog();
    if (std::string_view(kind) == "shapes")
      at(v, kind)->items = {shape("a"), shape("b")};
    if (std::string_view(kind) == "projections")
      at(v, kind)->items = {projection("a"), projection("b")};
    std::swap(at(v, kind)->items[0], at(v, kind)->items[1]);
    refused(v);
    at(v, kind)->items[1] = at(v, kind)->items[0];
    refused(v);
  }
  auto v                      = catalog();
  auto d                      = custom(v);
  at(d, "handler_id")->string = "shipped.render.v1";
  auto duplicate              = check(v);
  REQUIRE_FALSE(duplicate);
  REQUIRE(duplicate.error().code == catalog_error_code::identity_conflict);
  for (auto name : {"with-profile", "history", "check", "check-recover", "verdict"}) {
    v                     = catalog();
    d                     = custom(v);
    at(d, "name")->string = name;
    refused(v);
  }
  for (auto name : {"--help", "--stdlib-profile"}) {
    v                       = catalog();
    d                       = custom(v);
    at(d, "options")->items = {opt(name)};
    refused(v);
  }
  v                       = catalog();
  d                       = custom(v);
  at(d, "options")->items = {opt("--b"), opt("--a")};
  refused(v);
  at(d, "options")->items = {opt("--a"), opt("--a")};
  refused(v);
  v                            = catalog();
  d                            = custom(v);
  at(d, "capabilities")->items = {str("projection_apply_v1"), str("history_read_v1")};
  refused(v);
}
TEST_CASE("replacement.effective catalog help and grammar bounds") {
  auto v    = catalog();
  auto d    = custom(v);
  auto h    = at(d, "help");
  h->string = std::string(4096, 'h');
  REQUIRE(check(v));
  h->string += 'h';
  refused(v);
  for (auto text : {std::string("x\0y", 3), std::string("\x7f"), std::string("\x01"), std::string("\xff")}) {
    h->string = text;
    refused(v);
  }
  h->string = "\n\t";
  REQUIRE(check(v));
  at(d, "name")->string = std::string(64, 'z');
  REQUIRE(check(v));
  at(d, "name")->string += 'z';
  refused(v);
}
TEST_CASE("replacement.effective catalog count and canonical byte ceilings") {
  for (auto kind : {"commands", "shapes", "projections"}) {
    auto v     = catalog();
    auto items = at(v, kind);
    while (items->items.size() < 64) {
      auto name = std::format("z{:02}", items->items.size());
      if (std::string_view(kind) == "commands")
        add_command(v, command(name, {}, "", false));
      else
        items->items.push_back(std::string_view(kind) == "shapes" ? shape(name) : projection(name));
    }
    REQUIRE(check(v));
    items->items.push_back(items->items.back());
    refused(v);
  }
  auto v = catalog();
  auto d = custom(v);
  for (int i = 0; i < 32; ++i)
    at(d, "options")->items.push_back(opt(std::format("--x{:02}", i)));
  REQUIRE(check(v));
  at(d, "options")->items.push_back(opt("--x32"));
  refused(v);
  // Populate enough individually valid help fields to reach exactly the aggregate byte boundary.
  v = catalog();
  for (int i = 0; i < 47; ++i) {
    auto e = command(std::format("z{:02}", i), {}, "", false);
    for (int j = 0; j < 6; ++j)
      at(descriptor(e), "options")->items.push_back(opt(std::format("--x{}", j)));
    add_command(v, e);
  }
  std::vector<mutable_value> helps;
  for (auto& e : at(v, "commands")->items) {
    auto dd = descriptor(e);
    helps.push_back(at(dd, "help"));
    for (auto& o : at(dd, "options")->items)
      helps.push_back(at(o, "help"));
  }
  auto bytes = canonical_encode(v);
  REQUIRE(bytes);
  std::size_t remaining = 1048576 - bytes->size();
  for (auto& h : helps) {
    auto extra = std::min(remaining, 4096 - h->string.size());
    h->string.append(extra, 'h');
    remaining -= extra;
  }
  REQUIRE(remaining == 0);
  auto sealed = check(v);
  REQUIRE(sealed);
  REQUIRE(sealed->bytes().size() == 1048576);
  auto h = std::ranges::find_if(helps, [](auto& x) { return x->string.size() < 4096; });
  REQUIRE(h != helps.end());
  (*h)->string += 'x';
  auto r = check(v);
  REQUIRE_FALSE(r);
  REQUIRE(r.error().code == catalog_error_code::resource_limit);
  // Exercise the full schema-node shape with tiny help, so bytes do not mask nodes.
  v = catalog();
  for (int i = 0; i < 47; ++i) {
    std::vector<value_ptr> options;
    for (int j = 0; j < 32; ++j)
      options.push_back(opt(std::format("--x{:02}", j)));
    add_command(v, command(std::format("z{:02}", i), std::move(options), "", false));
  }
  for (int i = 0; i < 64; ++i) {
    at(v, "shapes")->items.push_back(shape(std::format("s{:02}", i)));
    at(v, "projections")->items.push_back(projection(std::format("p{:02}", i)));
  }
  REQUIRE(check(v));
  std::vector<std::string>      names;
  std::vector<std::string_view> paths;
  for (int i = 0; i < 65; ++i)
    names.push_back(std::format("ext/m{:02}.lua", i));
  for (auto& name : names)
    paths.push_back(name);
  REQUIRE(validate_catalog(catalog(), shipped, std::span(paths).first(64)));
  auto too_many = validate_catalog(catalog(), shipped, paths);
  REQUIRE_FALSE(too_many);
  REQUIRE(too_many.error().code == catalog_error_code::resource_limit);
  std::string oversized(1048577, 'x');
  REQUIRE_FALSE(decode_catalog(oversized, shipped, external));
}
TEST_CASE("replacement.effective catalog malformed canonical and bounded trees") {
  auto encoded = canonical_encode(catalog());
  REQUIRE(encoded);
  for (auto bytes : {std::string{}, encoded->substr(0, encoded->size() - 1), *encoded + "x"})
    REQUIRE_FALSE(decode_catalog(bytes, shipped, external));
  auto v      = catalog();
  auto nested = arr();
  auto cursor = nested;
  for (int i = 0; i < 8; ++i) {
    auto next = arr();
    cursor->items.push_back(next);
    cursor = next;
  }
  v->fields["shapes"] = nested;
  refused(v);
  auto huge           = arr(std::vector<value_ptr>(23000, flag(false)));
  v                   = catalog();
  v->fields["shapes"] = huge;
  refused(v);
  v                   = catalog();
  v->fields["shapes"] = {};
  refused(v);
  v                   = catalog();
  v->fields["shapes"] = v;
  refused(v);
  v->fields.erase("shapes");
}

namespace {
// Independent documented feature cuts; never inferred from product implementation.
mutable_value epoch_catalog(int epoch) {
  const std::vector<std::string> order{"render",  "init",    "history", "export",        "load",    "activate",
                                       "status",  "preview", "claim",   "takeover",      "refresh", "checkpoint",
                                       "release", "recover", "check",   "check-recover", "verdict"};
  auto                           v    = catalog();
  auto                           rows = at(v, "commands");
  std::erase_if(rows->items, [&](const auto& e) {
    const auto name = at(descriptor(e), "name")->string;
    return std::ranges::find(std::span(order.data(), static_cast<std::size_t>(epoch)), name) ==
           std::span(order.data(), static_cast<std::size_t>(epoch)).end();
  });
  for (auto& e : rows->items) {
    auto       d    = descriptor(e);
    const auto name = at(d, "name")->string;
    std::erase_if(at(d, "options")->items, [&](const auto& o) {
      const auto option = at(o, "name")->string;
      return (epoch == 5 && name == "history" && option == "--event") ||
             (epoch == 10 && name == "preview" && option == "--since");
    });
  }
  return v;
}
auto historical(const value_ptr& v) {
  auto bytes = canonical_encode(v);
  REQUIRE(bytes);
  return decode_catalog(*bytes, shipped, external);
}
} // namespace
TEST_CASE("C05 historical catalog feature cuts retain exact schemas") {
  for (int epoch : {5, 10, 14, 16, 17}) {
    INFO(epoch);
    auto v = epoch_catalog(epoch);
    REQUIRE(historical(v));
    CHECK(static_cast<bool>(check(v)) == (epoch == 17));
    for (std::size_t i = 0; i < at(v, "commands")->items.size(); ++i) {
      for (auto field : {"family", "handler_id", "response_kind"}) {
        auto malformed                                                     = epoch_catalog(epoch);
        at(descriptor(at(malformed, "commands")->items[i]), field)->string = "wrong_v1";
        CHECK_FALSE(historical(malformed));
      }
      auto malformed                                                             = epoch_catalog(epoch);
      at(descriptor(at(malformed, "commands")->items[i]), "capabilities")->items = {str("history_read_v1")};
      CHECK_FALSE(historical(malformed));
      malformed                                                                = epoch_catalog(epoch);
      at(at(at(malformed, "commands")->items[i], "owner"), "ordinal")->integer = 99;
      CHECK_FALSE(historical(malformed));
    }
    auto malformed = epoch_catalog(epoch);
    at(malformed, "commands")->items.push_back(at(malformed, "commands")->items.back());
    CHECK_FALSE(historical(malformed));
    malformed = epoch_catalog(epoch);
    std::ranges::reverse(at(malformed, "commands")->items);
    CHECK_FALSE(historical(malformed));
    malformed = epoch_catalog(epoch);
    add_command(malformed, command("unknown", {}, "render_capture_v1"));
    CHECK_FALSE(historical(malformed));
  }
  for (int partial : {6, 9, 11, 15})
    CHECK_FALSE(historical(epoch_catalog(partial)));
  for (auto [epoch, name, option] : std::vector<std::tuple<int, std::string, std::string>>{
           {5, "history", "--event"}, {10, "history", "--event"}, {10, "preview", "--since"}, {14, "preview", "--since"}}) {
    auto v = epoch_catalog(epoch);
    for (auto& e : at(v, "commands")->items)
      if (at(descriptor(e), "name")->string == name) {
        auto       options = at(descriptor(e), "options");
        const auto removed = std::erase_if(options->items, [&](const auto& o) { return at(o, "name")->string == option; });
        if (!removed)
          options->items.push_back(opt(option, option == "--event" ? "decimal" : "text"));
        std::ranges::sort(options->items, {}, [](const auto& o) { return at(o, "name")->string; });
      }
    CHECK_FALSE(historical(v));
  }
}
TEST_CASE("C05 historical external namespaces do not inherit later reservations") {
  for (auto [epoch, name] :
       std::vector<std::pair<int, std::string>>{{5, "status"}, {5, "check"}, {10, "refresh"}, {14, "check"}, {16, "verdict"}}) {
    INFO(epoch);
    INFO(name);
    auto v = epoch_catalog(epoch);
    add_command(v, command(name, {}, "", false));
    REQUIRE(historical(v));
    CHECK_FALSE(check(v));
    auto current = catalog();
    // Name, family and handler collisions are independently closed in current admission.
    auto d                  = custom(current);
    at(d, "family")->string = name;
    CHECK_FALSE(check(current));
    current                     = catalog();
    d                           = custom(current);
    at(d, "handler_id")->string = "shipped." + name + ".v1";
    CHECK_FALSE(check(current));
  }
}

TEST_CASE("C08 historical handler pairing is independent of handler grammar") {
  for (int epoch : {5, 10, 14, 16, 17}) {
    INFO(epoch);
    auto valid = epoch_catalog(epoch);
    REQUIRE(historical(valid));
    at(descriptor(at(valid, "commands")->items.front()), "handler_id")->string = "shipped.unbound_probe.v1";
    CHECK_FALSE(historical(valid));
  }
}
