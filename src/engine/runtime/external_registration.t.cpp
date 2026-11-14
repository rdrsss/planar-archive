// Actual captured ordered registration and immutable profile ownership.
#include <catch2/catch_test_macros.hpp>
#include <sys/stat.h>
#include <unistd.h>
import std;
import planar.engine.runtime;
import planar.engine.runtime_budget_test;
import planar.engine.hash;
import planar.engine.capture;
import planar.engine.json_codec;
import planar.engine.acceptance.intent;
using namespace planar::engine;
namespace {
profile_snapshot profile(std::vector<std::string> modules) {
  auto p     = std::make_shared<profile_closure>();
  p->profile = {"profile", "profile.json", "0644", "{\"version\":1,\"modules\":["};
  for (std::size_t i = 0; i < modules.size(); ++i) {
    if (i)
      p->profile.bytes += ",";
    auto path = std::format("m{}.lua", i + 1);
    p->profile.bytes += "\"" + path + "\"";
    p->modules.push_back({"modules", path, "0644", std::move(modules[i])});
  }
  p->profile.bytes += "]}";
  return p;
}
std::string command(std::string name = "inspect") {
  return "local D={name='" + name + "',family='extension_read_v1',handler_id='custom." + name +
         "',options=engine.array{},help='initial',response_kind='extension_v1',capabilities=engine.array{}}; "
         "registration.add(D,function() return D.help end); D.help='final'";
}
} // namespace
namespace {
std::string shape_source(std::string tail = {}) {
  return "local D={id='custom',abi='shape_v1',revision='initial',stages='optional',help='shape'}; "
         "registration.shape(D,function() error('shape F must not execute') end); D.revision='final'" +
         tail;
}
value_ptr shape_text(std::string text) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = std::move(text);
  return v;
}
value_ptr shape_record(std::map<std::string, value_ptr> fields) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::record;
  v->fields = std::move(fields);
  return v;
}
profile_snapshot named_profile(std::vector<std::pair<std::string, std::string>> modules) {
  auto p     = std::make_shared<profile_closure>();
  p->profile = {"profile", "profile.json", "0644", "{\"version\":1,\"modules\":["};
  for (auto& [path, bytes] : modules) {
    if (!p->modules.empty())
      p->profile.bytes += ",";
    p->profile.bytes += "\"" + path + "\"";
    p->modules.push_back({"modules", path, "0644", bytes});
  }
  p->profile.bytes += "]}";
  return p;
}
std::string shape_digest(profile_snapshot p) {
  auto vm = command_vm::create(std::move(p));
  REQUIRE(vm);
  auto definition = vm->resolve_shape_definition("custom");
  REQUIRE(definition);
  return definition->digest;
}
} // namespace
TEST_CASE("X05 B2 A final shape definition matches independent canonical hash construction",
          "[external-registration][shape-definition]") {
  auto source = shape_source();
  auto vm     = command_vm::create(profile({source}));
  REQUIRE(vm);
  auto result = vm->resolve_shape_definition("custom");
  REQUIRE(result);
  CHECK(result->id == "custom");
  CHECK(result->revision == "final");
  // Build the specified object independently of runtime owner/catalog assembly.
  std::string hex;
  for (unsigned char b : source)
    hex += std::format("{:02x}", b);
  auto d        = shape_record({{"id", shape_text("custom")},
                                {"abi", shape_text("shape_v1")},
                                {"revision", shape_text("final")},
                                {"stages", shape_text("optional")},
                                {"help", shape_text("shape")}});
  auto expected = hash_value(
      "shape_definition_v1",
      shape_record({{"descriptor", d},
                    {"owner", shape_record({{"tier", shape_text("external")}, {"module", shape_text("external/m1.lua")}})},
                    {"module_bytes", shape_text(hex)}}));
  REQUIRE(expected);
  CHECK(result->digest == *expected);
  CHECK(result->digest == "xxh3_128:b6d3718ddeb0cd1bc49b9e866a638b99");
  auto again = vm->resolve_shape_definition("custom");
  REQUIRE(again);
  CHECK(again->digest == result->digest);
  result->id       = "caller changed";
  result->revision = "caller changed";
  result->digest   = "caller changed";
  auto third       = vm->resolve_shape_definition("custom");
  REQUIRE(third);
  CHECK(third->id == "custom");
  CHECK(third->revision == "final");
  CHECK(third->digest == *expected);
}
TEST_CASE("X05 B2 A selected source owner and final descriptor determine shape identity",
          "[external-registration][shape-definition]") {
  auto source   = shape_source();
  auto original = shape_digest(named_profile({{"shape.lua", source}}));
  CHECK(shape_digest(named_profile({{"shape.lua", source + "\n-- exact selected comment"}})) != original);
  CHECK(shape_digest(named_profile({{"renamed.lua", source}})) != original);
  CHECK(shape_digest(named_profile({{"shape.lua", source + "; D.help='changed final descriptor'"}})) != original);
  std::string unrelated =
      "registration.projection({id='view',abi='projection_v1',revision='v1',input='role_context_v1',help=''},function() end)";
  CHECK(shape_digest(named_profile({{"shape.lua", source}, {"view.lua", unrelated}})) == original);
  CHECK(shape_digest(named_profile({{"view.lua", unrelated + " -- unrelated edit"}, {"shape.lua", source}})) == original);
  CHECK(shape_digest(named_profile({{"shape.lua", source + "; " + unrelated}})) != original);
  auto whitespace = std::make_shared<profile_closure>(*named_profile({{"shape.lua", source}}));
  whitespace->profile.bytes += " \n";
  whitespace->profile.mode    = "0755";
  whitespace->modules[0].mode = "0755";
  CHECK(shape_digest(whitespace) == original);
}
TEST_CASE("X05 B2 A actual host profile selectors do not enter selected shape identity",
          "[external-registration][shape-definition]") {
  char pattern[] = "/tmp/planar-b2-a-XXXXXX";
  auto name      = mkdtemp(pattern);
  REQUIRE(name);
  struct cleanup {
    std::filesystem::path root;
    ~cleanup() {
      std::filesystem::remove_all(root);
    }
  } temporary{std::filesystem::canonical(name)};
  for (auto selector : {"first profile.json", "second profile.json"})
    std::ofstream(temporary.root / selector) << "{\"version\":1,\"modules\":[\"m1.lua\"]}";
  std::ofstream(temporary.root / "m1.lua") << shape_source();
  auto first  = profile_capture::open((temporary.root / "first profile.json").string());
  auto second = profile_capture::open((temporary.root / "second profile.json").string());
  REQUIRE(first);
  REQUIRE(second);
  auto a = first->snapshot(), b = second->snapshot();
  REQUIRE(a);
  REQUIRE(b);
  auto original = shape_digest(*a);
  CHECK(shape_digest(*b) == original);
  std::ofstream(temporary.root / "m1.lua") << "error('private replacement')";
  CHECK_FALSE(first->verify());
  CHECK(shape_digest(*a) == original);
}
TEST_CASE("X05 B2 A definition lookup severs caller capture aliases and never invokes shape Lua",
          "[external-registration][shape-definition]") {
  auto captured = std::make_shared<profile_closure>(*profile({shape_source()}));
  auto vm       = command_vm::create(captured);
  REQUIRE(vm);
  auto before = vm->resolve_shape_definition("custom");
  REQUIRE(before);
  captured->modules[0].bytes = "error('private caller mutation')";
  captured->modules[0].path  = "different.lua";
  captured->profile.bytes.clear();
  for (int i = 0; i < 20; ++i) {
    auto after = vm->resolve_shape_definition("custom");
    REQUIRE(after);
    CHECK(after->digest == before->digest);
  }
}
TEST_CASE("X05 B2 A wrong namespace embedded and moved owners refuse without consuming lookup",
          "[external-registration][shape-definition]") {
  auto vm = command_vm::create(profile({shape_source(), command()}));
  REQUIRE(vm);
  for (auto id : {"unknown", "inspect", "pipeline", "fanout_review"}) {
    auto bad = vm->resolve_shape_definition(id);
    REQUIRE_FALSE(bad);
    CHECK(bad.error().detail.code == "internal");
    CHECK(bad.error().detail.message == "Invalid external profile protocol");
    CHECK(bad.error().field == "catalog.shapes");
  }
  REQUIRE(vm->resolve_shape_definition("custom"));
  auto moved = std::move(*vm);
  CHECK_FALSE(vm->resolve_shape_definition("custom"));
  REQUIRE(moved.resolve_shape_definition("custom"));
  auto embedded = command_vm::create();
  REQUIRE(embedded);
  CHECK_FALSE(embedded->resolve_shape_definition("pipeline"));
}
TEST_CASE("X05 B2 A repeated native definition work is cumulative and resource refusal is terminal",
          "[external-registration][shape-definition]") {
  auto source = shape_source("\n--" + std::string(524288, 'x'));
  auto vm     = command_vm::create(profile({source}));
  REQUIRE(vm);
  std::size_t successes = 0;
  for (; successes < 200; ++successes) {
    auto result = vm->resolve_shape_definition("custom");
    if (result)
      continue;
    CHECK(result.error().detail.code == "resource_limit");
    CHECK(result.error().detail.limit == "native_bytes");
    CHECK(result.error().field == "catalog.shapes");
    break;
  }
  CHECK(successes > 0);
  REQUIRE(successes < 200);
  auto retry = vm->resolve_shape_definition("custom");
  REQUIRE_FALSE(retry);
  CHECK(retry.error().detail.code == "resource_limit");
  auto fresh = command_vm::create(profile({shape_source()}));
  REQUIRE(fresh);
  REQUIRE(fresh->resolve_shape_definition("custom"));
}
TEST_CASE("X05 A3 empty captured profile retains catalog and digest", "[external-registration]") {
  auto vm = command_vm::create(profile({}));
  REQUIRE(vm);
  CHECK(vm->catalog()->fields.at("version")->integer == 2);
  CHECK(vm->catalog()->fields.at("commands")->items.size() == 17);
  CHECK_FALSE(vm->effective_catalog().empty());
  CHECK(vm->effective_digest().size() == 32);
}
TEST_CASE("X05 A3 ordered external descriptors finalize and retain native owners", "[external-registration]") {
  auto vm = command_vm::create(profile(
      {command(),
       "registration.shape({id='custom',abi='shape_v1',revision='v1',stages='optional',help='shape'},function() return {} end); "
       "registration.projection({id='custom',abi='projection_v1',revision='v1',input='role_context_v1',help='projection'},"
       "function() return {} end)"}));
  REQUIRE(vm);
  auto c = vm->catalog();
  auto e = c->fields.at("commands")->items.at(8);
  CHECK(e->fields.at("descriptor")->fields.at("name")->string == "inspect");
  CHECK(e->fields.at("descriptor")->fields.at("help")->string == "final");
  CHECK(e->fields.at("owner")->fields.at("module")->string == "external/m1.lua");
  CHECK(c->fields.at("shapes")->items.at(0)->fields.at("owner")->fields.at("ordinal")->integer == 2);
}
TEST_CASE("X05 A3 external environment is pure frozen and self rooted", "[external-registration]") {
  auto vm = command_vm::create(
      profile({"assert(_G==_G._G); assert(engine.kind and engine.object and engine.array); assert(not engine.hash_value and not "
               "engine.json_encode and not engine.json_decode and not io and not load and not debug); assert(registration.add "
               "and registration.shape and registration.projection); local alias=registration.add; " +
               command()}));
  REQUIRE(vm);
  for (auto code : {"_G.escape=1", "engine.hash_value=function() end", "table.new=1", "registration.add=function() end"}) {
    auto bad = command_vm::create(profile({code}));
    CHECK_FALSE(bad);
  }
}
TEST_CASE("X05 A3 source only chunks require actual zero results", "[external-registration]") {
  for (auto code : {"return nil", "return {}", "error('private secret')", "local ="}) {
    auto bad = command_vm::create(profile({code}));
    CHECK_FALSE(bad);
  }
}
TEST_CASE("X05 A3 malformed reserved duplicate and caught registration refuse", "[external-registration]") {
  for (auto code :
       {"pcall(registration.add,{},function() end)", "pcall(registration.add,{},1)",
        "registration.shape({id='pipeline',abi='shape_v1',revision='v1',stages='optional',help=''},function() end)"}) {
    auto bad = command_vm::create(profile({code}));
    CHECK_FALSE(bad);
  }
  CHECK_FALSE(command_vm::create(profile({command(), command()})));
  auto vm = command_vm::create(
      profile({"local add=registration.add; local "
               "D={name='inspect',family='extension_read_v1',handler_id='custom.inspect',options=engine.array{},help='',response_"
               "kind='extension_v1',capabilities=engine.array{}}; assert(pcall(add,D,function() end))"}));
  REQUIRE(vm);
}
namespace {
void refused(const std::string& code, const std::string& expected, std::string field = {}) {
  INFO(code);
  auto vm = command_vm::create(profile({code}));
  REQUIRE_FALSE(vm);
  CHECK(vm.error().detail.code == expected);
  CHECK(vm.error().detail.message.find("private") == std::string::npos);
  if (!field.empty())
    CHECK(vm.error().field == field);
}
class untouched_services final : public command_services {
public:
  int                    calls = 0;
  command_service_result open_workspace(const command_render_options&) override {
    ++calls;
    return {};
  }
  command_service_result capture_render(const command_render_options&, const value_ptr&) override {
    ++calls;
    return {};
  }
  command_service_result initialize_ledger(const command_init_options&) override {
    ++calls;
    return {};
  }
  command_service_result read_history(const command_history_options&, const command_history_selector&) override {
    ++calls;
    return {};
  }
  command_service_result export_source(const command_export_options&) override {
    ++calls;
    return {};
  }
};
} // namespace
TEST_CASE("X05 A3 profile owners refuse invocation before provider effects", "[external-registration]") {
  auto vm = command_vm::create(profile({command(), shape_source()}));
  REQUIRE(vm);
  REQUIRE(vm->resolve_shape_definition("custom"));
  untouched_services services;
  auto               result = vm->invoke(command_init_options{}, services);
  REQUIRE_FALSE(result);
  CHECK(result.error().message == "Invalid external profile protocol");
  CHECK(services.calls == 0);
}
TEST_CASE("X05 A3 exact descriptor schemas and external capability restrictions", "[external-registration]") {
  for (auto bad :
       {"registration.shape({id='custom',abi='shape_v1',revision='v1',stages='optional',help='',owner={}},function() end)",
        "registration.shape({id='custom',abi='shape_v2',revision='v1',stages='optional',help=''},function() end)",
        "registration.projection({id='custom',abi='projection_v1',revision='v1',input='host',help=''},function() end)",
        "registration.add({name='custom',family='extension_read_v1',handler_id='custom.v1',options=engine.array{},help='',"
        "response_kind='extension_v1',capabilities=engine.array{'accept_load_v1'}},function() end)",
        "registration.add({name='custom',family='extension_read_v1',handler_id='custom.v1',options=engine.array{{name='--"
        "revision',type='history_id',required=false,help=''}},help='',response_kind='extension_v1',capabilities=engine.array{}},"
        "function() end)"})
    refused(bad, "internal");
}
TEST_CASE("X05 A3 separate identity namespaces and all reserved IDs", "[external-registration]") {
  for (auto id : {"pipeline", "fanout_review"})
    refused("registration.shape({id='" + std::string(id) +
                "',abi='shape_v1',revision='v1',stages='optional',help=''},function() end)",
            "reference", "catalog.shapes[1]");
  for (auto id : {"worker", "reviewer", "recovery"})
    refused("registration.projection({id='" + std::string(id) +
                "',abi='projection_v1',revision='v1',input='role_context_v1',help=''},function() end)",
            "reference", "catalog.projections[1]");
  refused(command("render"), "reference", "catalog.commands[18]");
  auto reserved = command("with-profile");
  reserved.replace(reserved.find("custom.with-profile"), 19, "custom.with_profile");
  refused(reserved, "reference", "catalog.commands[18]");
}
TEST_CASE("X05 A3 resource ceilings remain sticky when protected calls catch", "[external-registration]") {
  refused("pcall(function() while true do end end)", "resource_limit", "profile.modules[1]");
  refused("pcall(function() local t={} for i=1,10000000 do t[i]={} end end)", "resource_limit");
  refused("for i=1,65 do pcall(registration.shape,{id='s',abi='shape_v1',revision='v1',stages='optional',help=''},function() "
          "end) end",
          "resource_limit", "catalog.shapes[65]");
}
TEST_CASE("X05 A3 retained upvalue writers and native callable roots refuse", "[external-registration]") {
  refused(
      "local n=0; registration.shape({id='custom',abi='shape_v1',revision='v1',stages='optional',help=''},function() n=n+1 end)",
      "internal");
  refused("registration.shape({id='custom',abi='shape_v1',revision='v1',stages='optional',help=''},next)", "internal");
}
TEST_CASE("X05 A3 validates structural snapshot before any module evaluation", "[external-registration]") {
  auto p             = std::make_shared<profile_closure>(*profile({"error('private')"}));
  p->modules[0].path = "wrong.lua";
  auto bad           = command_vm::create(p);
  REQUIRE_FALSE(bad);
  CHECK(bad.error().field == "profile.modules[1]");
  p = std::make_shared<profile_closure>(*profile({""}));
  p->modules[0].bytes.resize(1048577, ' ');
  bad = command_vm::create(p);
  REQUIRE_FALSE(bad);
  CHECK(bad.error().detail.code == "resource_limit");
  p                = std::make_shared<profile_closure>(*profile({}));
  p->profile.bytes = "{\"version\":1,\"modules\":[\"../private.lua\"]}";
  bad              = command_vm::create(p);
  REQUIRE_FALSE(bad);
  CHECK(bad.error().field == "profile.modules[1]");
}
TEST_CASE("X05 A3 retained source byte and mode changes affect exact effective digest", "[external-registration]") {
  auto p = profile({command()});
  auto a = command_vm::create(p);
  REQUIRE(a);
  auto q = std::make_shared<profile_closure>(*p);
  q->modules[0].bytes += "\n-- exact byte change\n";
  auto b = command_vm::create(q);
  REQUIRE(b);
  CHECK(a->effective_digest() != b->effective_digest());
  q->modules[0].mode = "0755";
  auto c             = command_vm::create(q);
  REQUIRE(c);
  CHECK(b->effective_digest() != c->effective_digest());
  q->profile.bytes += " \n";
  auto d = command_vm::create(q);
  REQUIRE(d);
  CHECK(c->effective_digest() != d->effective_digest());
}
TEST_CASE("X05 A3 real capture owns original source after filesystem changes", "[external-registration]") {
  char pattern[] = "/tmp/planar-a3-XXXXXX";
  auto name      = mkdtemp(pattern);
  REQUIRE(name);
  struct cleanup {
    std::filesystem::path root;
    ~cleanup() {
      std::filesystem::remove_all(root);
    }
  } temporary{std::filesystem::canonical(name)};
  auto root = temporary.root;
  std::ofstream(root / "profile.json") << "{\"version\":1,\"modules\":[\"module.lua\"]}";
  std::ofstream(root / "module.lua") << command();
  auto capture = profile_capture::open((root / "profile.json").string());
  REQUIRE(capture);
  auto captured = capture->snapshot();
  REQUIRE(captured);
  std::ofstream(root / "module.lua") << "error('private replacement must never execute')";
  auto vm = command_vm::create(*captured);
  REQUIRE(vm);
  CHECK_FALSE(capture->verify());
  std::filesystem::remove(root / "module.lua");
  auto again = command_vm::create(*captured);
  REQUIRE(again);
  CHECK(vm->effective_digest() == again->effective_digest());
}
TEST_CASE("X05 A3 instruction accounting does not restart for later chunks", "[external-registration]") {
  std::string work   = "for i=1,2000000 do local n=i*i end";
  auto        single = command_vm::create(profile({work}));
  REQUIRE(single);
  auto total = command_vm::create(profile({work, work, work}));
  REQUIRE_FALSE(total);
  CHECK(total.error().detail.code == "resource_limit");
  CHECK(total.error().detail.limit == "instructions");
  CHECK(total.error().field == "profile.modules[3]");
}
TEST_CASE("X05 A3 normalized options detect duplicate identities independently", "[external-registration]") {
  refused("registration.add({name='custom',family='extension_read_v1',handler_id='custom.v1',options=engine.array{{name='--thing'"
          ",type='text',required=false,help=''},{name='--thing',type='text',required=false,help=''}},help='',response_kind='"
          "extension_v1',capabilities=engine.array{}},function() end)",
          "reference", "catalog.commands[18]");
  refused(
      "registration.add({name='custom',family='extension_read_v1',handler_id='custom.v1',options=engine.array{{name='--help',"
      "type='text',required=false,help=''}},help='',response_kind='extension_v1',capabilities=engine.array{}},function() end)",
      "reference", "catalog.commands[18]");
  refused("registration.shape({id='custom',abi='shape_v1',revision='v1',stages='optional',help=string.char(1)},function()end)",
          "internal", "catalog.shapes[1]");
}
TEST_CASE("X05 A3 C01 malformed sort keys retain original fields", "[external-registration]") {
  for (int kind = 0; kind < 3; ++kind)
    for (int bad = 0; bad < 3; ++bad)
      for (bool missing : {false, true}) {
        std::string source;
        for (int i = 0; i < 3; ++i) {
          auto key  = i == bad ? (missing     ? ""
                                  : kind == 0 ? "name=1,"
                                              : "id=1,")
                               : std::format("{}='{}',", kind == 0 ? "name" : "id",
                                             i == 0   ? "z"
                                             : i == 1 ? "a"
                                                      : "m");
          auto rest = kind == 0   ? std::format("family='extension_read_v1',handler_id='custom.e{}',options=engine.array{{}},"
                                                "help='',response_kind='extension_v1',capabilities=engine.array{{}}",
                                                i)
                      : kind == 1 ? "abi='shape_v1',revision='v1',stages='optional',help=''"
                                  : "abi='projection_v1',revision='v1',input='role_context_v1',help=''";
          source += std::format("registration.{}({{{}{}}},function() end);",
                                kind == 0   ? "add"
                                : kind == 1 ? "shape"
                                            : "projection",
                                key, rest);
        }
        INFO(source);
        auto vm = command_vm::create(profile({source}));
        REQUIRE_FALSE(vm);
        CHECK(vm.error().detail.code == "internal");
        CHECK(vm.error().detail.message == "Invalid external profile protocol");
        CHECK(vm.error().field == std::format("catalog.{}[{}]",
                                              kind == 0   ? "commands"
                                              : kind == 1 ? "shapes"
                                                          : "projections",
                                              bad + 1 + (kind == 0 ? 17 : 0)));
      }
}
TEST_CASE("X05 A3 C01 handler and option controls retain original command", "[external-registration]") {
  for (auto fault : {"D.handler_id=1;", "D.options=engine.array{{name=1}};"}) {
    auto source = command("z") + ";" + command("a");
    // The second local D is malformed; canonical order moves it before z.
    source += ";" + std::string(fault);
    auto vm = command_vm::create(profile({source}));
    REQUIRE_FALSE(vm);
    CHECK(vm.error().detail.code == "internal");
    CHECK(vm.error().detail.message == "Invalid external profile protocol");
    CHECK(vm.error().field == "catalog.commands[19]");
  }
}
TEST_CASE("X05 A3 C01 high catalog bytes remain valid", "[external-registration]") {
  std::string source = "local help='" + std::string(4096, 'h') + "';";
  for (int kind = 0; kind < 3; ++kind)
    for (int i = 0; i < (kind == 0 ? 47 : 64); ++i) {
      auto descriptor = kind == 0   ? std::format("name='v{}',family='extension_read_v1',handler_id='custom.v{}',options=engine."
                                                  "array{{}},response_kind='extension_v1',capabilities=engine.array{{}},",
                                                  i, i)
                        : kind == 1 ? std::format("id='s{}',abi='shape_v1',revision='v1',stages='optional',", i)
                                    : std::format("id='p{}',abi='projection_v1',revision='v1',input='role_context_v1',", i);
      if (kind == 0) {
        auto at = descriptor.find("options=engine.array{}");
        descriptor.replace(at, std::string_view("options=engine.array{}").size(),
                           i < 13 ? "options=engine.array{{name='--detail',type='text',required=false,help=help},{name='--extra',"
                                    "type='text',required=false,help=help}}"
                                  : "options=engine.array{{name='--detail',type='text',required=false,help=help}}");
      }
      source += std::format("registration.{}({{{}help=help}},function() end);",
                            kind == 0   ? "add"
                            : kind == 1 ? "shape"
                                        : "projection",
                            descriptor);
    }
  auto vm = command_vm::create(profile({source}));
  INFO((vm ? "success" : vm.error().detail.code + " / " + vm.error().detail.message + " / " + vm.error().field));
  REQUIRE(vm);
  CHECK(vm->effective_catalog().size() > 1040000);
  CHECK(vm->effective_catalog().size() < 1048576);
  CHECK(vm->catalog()->fields.at("commands")->items.size() == 64);
  CHECK(vm->catalog()->fields.at("shapes")->items.size() == 64);
  CHECK(vm->catalog()->fields.at("projections")->items.size() == 64);
}
TEST_CASE("X05 A3 C02 sealing retains original mixed kind and module locations", "[external-registration]") {
  auto entry = [](int kind, int id, std::string_view callable) {
    auto descriptor = kind == 0   ? std::format("name='c{}',family='extension_read_v1',handler_id='custom.c{}',options=engine."
                                                "array{{}},help='',response_kind='extension_v1',capabilities=engine.array{{}}",
                                                id, id)
                      : kind == 1 ? std::format("id='s{}',abi='shape_v1',revision='v1',stages='optional',help=''", id)
                                  : std::format("id='p{}',abi='projection_v1',revision='v1',input='role_context_v1',help=''", id);
    return std::format("registration.{}({{{}}},{});",
                       kind == 0   ? "add"
                       : kind == 1 ? "shape"
                                   : "projection",
                       descriptor, callable);
  };
  for (bool second_module : {false, true})
    for (int kind = 0; kind < 3; ++kind)
      for (int bad = 0; bad < 3; ++bad)
        for (bool native : {false, true}) {
          std::string source = "local n=0;";
          // Each target kind occurs three times, interleaved with both other kinds.
          for (int i = 0; i < 3; ++i)
            for (int offset = 0; offset < 3; ++offset) {
              int  current  = (kind + offset) % 3;
              auto callable = current == kind && i == bad ? (native ? "next" : "function() n=n+1 end") : "function() end";
              source += entry(current, i + 10, callable);
            }
          std::vector<std::string> modules;
          if (second_module)
            modules.push_back(entry(0, 1, "function()end") + entry(1, 1, "function()end") + entry(2, 1, "function()end"));
          modules.push_back(source);
          INFO(source);
          auto vm = command_vm::create(profile(std::move(modules)));
          REQUIRE_FALSE(vm);
          CHECK(vm.error().detail.code == "internal");
          CHECK(vm.error().detail.message == "Invalid external profile protocol");
          CHECK(vm.error().field == std::format("catalog.{}[{}]",
                                                kind == 0   ? "commands"
                                                : kind == 1 ? "shapes"
                                                            : "projections",
                                                bad + 1 + (kind == 0 ? 17 : 0) + (second_module ? 1 : 0)));
        }
}
TEST_CASE("X05 A3 C03 two digit seal locations retain original indices", "[external-registration]") {
  auto entry = [](int kind, int id, std::string_view callable) {
    auto descriptor = kind == 0   ? std::format("name='c{}',family='extension_read_v1',handler_id='custom.c{}',options=engine."
                                                "array{{}},help='',response_kind='extension_v1',capabilities=engine.array{{}}",
                                                id, id)
                      : kind == 1 ? std::format("id='s{}',abi='shape_v1',revision='v1',stages='optional',help=''", id)
                                  : std::format("id='p{}',abi='projection_v1',revision='v1',input='role_context_v1',help=''", id);
    return std::format("registration.{}({{{}}},{});",
                       kind == 0   ? "add"
                       : kind == 1 ? "shape"
                                   : "projection",
                       descriptor, callable);
  };
  for (int kind = 0; kind < 3; ++kind)
    for (int index : {kind == 0 ? 20 : 10, 64})
      for (int fault = 0; fault < 3; ++fault) {
        // Prior module entries and other kinds must not shift the target kind's
        // original index. The seventeen shipped commands count toward command64.
        auto first  = entry(0, 1, "function()end") + entry(1, 1, "function()end") + entry(2, 1, "function()end");
        auto second = "local n=0;" + entry((kind + 1) % 3, 2, "function()end");
        int  total  = index - (kind == 0 ? 17 : 0);
        for (int id = 2; id <= total; ++id) {
          auto callable = id != total || fault == 0 ? "function()end" : fault == 1 ? "function() n=n+1 end" : "next";
          second += entry(kind, id, callable);
        }
        auto vm = command_vm::create(profile({first, second}));
        INFO(std::format("kind={} index={} fault={}", kind, index, fault));
        if (!fault) {
          REQUIRE(vm);
          continue;
        }
        REQUIRE_FALSE(vm);
        CHECK(vm.error().detail.code == "internal");
        CHECK(vm.error().detail.message == "Invalid external profile protocol");
        CHECK(vm.error().field == std::format("catalog.{}[{}]",
                                              kind == 0   ? "commands"
                                              : kind == 1 ? "shapes"
                                                          : "projections",
                                              index));
      }
}

TEST_CASE("X05 A3 C04 successful callbacks leave module boundary and first refusal sticky", "[external-registration]") {
  const std::string good = "registration.projection({id='p',abi='projection_v1',revision='v1',input='role_context_v1',help=''},"
                           "function()end);";
  for (auto tail : {"error('private later module error')", "return true", "registration.shape({},1)",
                    "pcall(registration.shape,{},1); pcall(registration.projection,{},function()end)"}) {
    // Use distinct first-module identity so the second module reaches its tail.
    auto first = good;
    first.replace(first.find("id='p'"), 6, "id='q'");
    auto vm = command_vm::create(profile({first, good + tail}));
    REQUIRE_FALSE(vm);
    CHECK(vm.error().detail.code == "internal");
    CHECK(vm.error().detail.message == "Invalid external profile protocol");
    CHECK(vm.error().field == "profile.modules[2]");
  }
}
TEST_CASE("X05 A3 C04 original descriptor errors survive canonical remap at two digit indices", "[external-registration]") {
  for (int kind = 0; kind < 3; ++kind)
    for (int index : {kind == 0 ? 20 : 10, 64}) {
      auto entry = [kind](int id, bool bad) {
        auto d = kind == 0 ? std::format("name='c{}',family='{}',handler_id='custom.c{}',options=engine.array{{}},help='',"
                                         "response_kind='extension_v1',capabilities=engine.array{{}}",
                                         id, bad ? "wrong" : "extension_read_v1", id)
                 : kind == 1
                     ? std::format("id='s{}',abi='{}',revision='v1',stages='optional',help=''", id, bad ? "wrong" : "shape_v1")
                     : std::format("id='p{}',abi='{}',revision='v1',input='role_context_v1',help=''", id,
                                   bad ? "wrong" : "projection_v1");
        return std::format("registration.{}({{{}}},function()end);", kind == 0 ? "add" : kind == 1 ? "shape" : "projection", d);
      };
      auto        first = entry(1, false);
      std::string second;
      int         total = index - (kind == 0 ? 17 : 0);
      for (int id = 2; id <= total; ++id)
        second += entry(id, id == total);
      auto vm = command_vm::create(profile({first, second}));
      REQUIRE_FALSE(vm);
      CHECK(vm.error().detail.code == "internal");
      CHECK(vm.error().detail.message == "Invalid external profile protocol");
      CHECK(vm.error().field == std::format("catalog.{}[{}]",
                                            kind == 0   ? "commands"
                                            : kind == 1 ? "shapes"
                                                        : "projections",
                                            index));
    }
}
TEST_CASE("X05 A3 C05 nested ordinary errors permit later valid registration", "[external-registration]") {
  auto vm = command_vm::create(
      profile({"local ok=pcall(function() local inner=pcall(function() error('private ordinary') end); assert(not inner); "
               "error('private outer') end); assert(not ok); "
               "registration.projection({id='after_error',abi='projection_v1',revision='v1',input='role_context_v1',help=''},"
               "function()end)"}));
  REQUIRE(vm);
  CHECK(vm->catalog()->fields.at("projections")->items.size() == 1);
}
TEST_CASE("X05 A3 C05 nested resource failure preserves first registration entry", "[external-registration]") {
  refused("pcall(function() pcall(function() pcall(function() for i=1,65 do "
          "registration.shape({id='s',abi='shape_v1',revision='v1',stages='optional',help=''},function()end) end end) end) "
          "end); error('private later error')",
          "resource_limit", "catalog.shapes[65]");
}

namespace {
value_ptr shape_input() {
  auto version     = std::make_shared<value>();
  version->kind    = value_kind::integer;
  version->integer = 1;
  auto shared      = shape_record({{"text", shape_text("captured")}});
  return shape_record({{"version", version},
                       {"job", shape_record({{"left", shared}, {"right", shared}})},
                       {"spec", shape_record({})},
                       {"assignments", shape_record({})},
                       {"reviewer", shape_record({})}});
}
std::string callable_shape(std::string body) {
  return "local D={id='custom',abi='shape_v1',revision='final',stages='optional',help='shape'}; "
         "registration.shape(D,function(input) " +
         body + " end)";
}
} // namespace
TEST_CASE("X05 B2 C actual retained shape preserves identity aliases immutable input and writable scratch",
          "[external-registration][shape-call]") {
  auto source = callable_shape(R"(
    assert(input.job.left==input.job.right)
    assert(not pcall(function() input.job.left.text='changed' end))
    assert(not pcall(function() D.revision='changed' end))
    assert(not pcall(function() engine.new_field=true end))
    local scratch=engine.object{}; scratch.value='scratch'
    return engine.object{alias=input.job.left,value=scratch.value,revision=D.revision}
  )");
  auto vm     = command_vm::create(profile({source}));
  REQUIRE(vm);
  auto definition = vm->resolve_shape_definition("custom");
  REQUIRE(definition);
  auto input = shape_input();
  for (int i = 0; i < 20; ++i) {
    auto response = vm->invoke_shape("custom", input);
    REQUIRE(response);
    CHECK(response->definition.digest == definition->digest);
    CHECK(response->result->fields.at("alias")->fields.at("text")->string == "captured");
    CHECK(response->result->fields.at("value")->string == "scratch");
    CHECK(response->result->fields.at("revision")->string == "final");
  }
  CHECK(input->fields.at("job")->fields.at("left")->fields.at("text")->string == "captured");
}
TEST_CASE("X05 B2 C admission errors are nonconsuming and raw semantic refusal is ordinary data",
          "[external-registration][shape-call]") {
  auto vm = command_vm::create(profile({callable_shape("return {ok=false,error={code='schema',message='refused'}}")}));
  REQUIRE(vm);
  CHECK_FALSE(vm->invoke_shape("missing", shape_input()));
  CHECK_FALSE(vm->invoke_shape("custom", shape_record({})));
  auto input                                                               = shape_input();
  std::const_pointer_cast<value>(input->fields.at("job"))->fields["cycle"] = input;
  CHECK_FALSE(vm->invoke_shape("custom", input));
  std::const_pointer_cast<value>(input->fields.at("job"))->fields.erase("cycle");
  auto response = vm->invoke_shape("custom", input);
  REQUIRE(response);
  CHECK_FALSE(response->result->fields.at("ok")->boolean);
  CHECK(vm->invoke_shape("custom", shape_input()));
}
TEST_CASE("X05 B2 C invalid retained returns are terminal with original shape location", "[external-registration][shape-call]") {
  for (auto body : {"return", "return {},{}", "return function() end", "local x={};x.x=x;return x", "error('private source')",
                    "return 1.5"}) {
    auto vm = command_vm::create(profile({callable_shape(body)}));
    REQUIRE(vm);
    auto response = vm->invoke_shape("custom", shape_input());
    REQUIRE_FALSE(response);
    CHECK(response.error().detail.code == "internal");
    CHECK(response.error().field == "catalog.shapes[1]");
    auto lookup = vm->resolve_shape_definition("custom");
    REQUIRE_FALSE(lookup);
    CHECK(lookup.error().field == response.error().field);
    CHECK(lookup.error().detail.message == response.error().detail.message);
    auto retry = vm->invoke_shape("missing", shape_input());
    REQUIRE_FALSE(retry);
    CHECK(retry.error().detail.message == response.error().detail.message);
    CHECK(retry.error().field == response.error().field);
  }
}
TEST_CASE("X05 B2 C complete raw transport enforces exact 16 MiB without semantic acceptance",
          "[external-registration][shape-call]") {
  // JSON string framing adds two bytes. These strings are intentionally invalid shape responses.
  for (int extra : {0, 1}) {
    auto vm = command_vm::create(profile({callable_shape(
        "local s='a'; for i=1,23 do s=s..s end; return s..string.sub(s,1,8388606+" + std::to_string(extra) + ")")}));
    REQUIRE(vm);
    auto response = vm->invoke_shape("custom", shape_input());
    if (!extra) {
      REQUIRE(response);
      CHECK(response->result->string.size() == 16777214);
    } else {
      REQUIRE_FALSE(response);
      CHECK(response.error().detail.code == "resource_limit");
      CHECK(response.error().field == "catalog.shapes[1]");
    }
  }
}
TEST_CASE("X05 B2 C repeated calls share invocation instructions and caught exhaustion stays terminal",
          "[external-registration][shape-call]") {
  auto vm =
      command_vm::create(profile({callable_shape("local n=0; pcall(function() for i=1,20000000 do n=n+1 end end); return n")}));
  REQUIRE(vm);
  std::optional<registration_error> failure;
  std::size_t                       successes = 0;
  for (int i = 0; i < 10; ++i) {
    auto response = vm->invoke_shape("custom", shape_input());
    if (!response) {
      failure = response.error();
      break;
    }
    ++successes;
  }
  REQUIRE(failure);
  CHECK(successes > 0);
  CHECK(successes < 10);
  CHECK(failure->detail.code == "resource_limit");
  CHECK(failure->detail.limit == "instructions");
  CHECK(failure->detail.maximum == 100000000);
  CHECK(failure->field == "catalog.shapes[1]");
  auto again = vm->invoke_shape("missing", shape_input());
  REQUIRE_FALSE(again);
  CHECK(again.error().detail.message == failure->detail.message);
  CHECK(again.error().field == failure->field);
}
TEST_CASE("X05 B2 C wrong callable kind is nonconsuming and execution uses original shape ordinal",
          "[external-registration][shape-call]") {
  auto source =
      std::string("registration.projection({id='visual',abi='projection_v1',revision='v1',input='role_context_v1',help=''},"
                  "function()error('wrong kind')end); ") +
      "registration.shape({id='zfirst',abi='shape_v1',revision='v1',stages='optional',help=''},function()return {}end); " +
      callable_shape("error('selected second')");
  auto vm = command_vm::create(profile({source}));
  REQUIRE(vm);
  auto wrong = vm->invoke_shape("visual", shape_input());
  REQUIRE_FALSE(wrong);
  CHECK(wrong.error().field == "catalog.shapes");
  auto response = vm->invoke_shape("custom", shape_input());
  REQUIRE_FALSE(response);
  CHECK(response.error().field == "catalog.shapes[2]");
}
TEST_CASE("X05 B2 C caught illegal registration stays sticky while ordinary caught Lua errors recover",
          "[external-registration][shape-call][shape-sticky]") {
  for (auto tail : {"return {}", "error('later private failure')", "return {},{}"}) {
    auto source =
        std::string(
            "local saved=registration.shape; local D={id='custom',abi='shape_v1',revision='v1',stages='optional',help=''}; "
            "registration.shape(D,function(input) assert(not pcall(saved,D,function()end)); ") +
        tail + " end)";
    auto vm = command_vm::create(profile({source}));
    REQUIRE(vm);
    auto response = vm->invoke_shape("custom", shape_input());
    REQUIRE_FALSE(response);
    CHECK(response.error().detail.code == "internal");
    CHECK(response.error().detail.message == "Invalid external profile protocol");
    CHECK(response.error().field == "catalog.shapes[1]");
    auto retry = vm->invoke_shape("custom", shape_input());
    REQUIRE_FALSE(retry);
    CHECK(retry.error().detail.message == response.error().detail.message);
  }
  auto ordinary =
      command_vm::create(profile({callable_shape("assert(not pcall(function()error('ordinary')end));return {ordinary=true}")}));
  REQUIRE(ordinary);
  for (int i = 0; i < 2; ++i) {
    auto response = ordinary->invoke_shape("custom", shape_input());
    REQUIRE(response);
    CHECK(response->result->fields.at("ordinary")->boolean);
  }
}

TEST_CASE("X05 B2 C prior lookup resource failure survives every later shape admission",
          "[external-registration][shape-call][shape-prior-resource]") {
  auto vm = command_vm::create(
      profile({callable_shape("return {}") + " registration.projection({id='visual',abi='projection_v1',revision='v1',input='"
                                             "role_context_v1',help=''},function()return {}end)"}));
  REQUIRE(vm);
  std::optional<registration_error> first;
  std::size_t                       successes = 0;
  for (; successes < 100000; ++successes) {
    auto definition = vm->resolve_shape_definition("custom");
    if (!definition) {
      first = std::move(definition.error());
      break;
    }
  }
  REQUIRE(first);
  CHECK(successes > 0);
  CHECK(first->detail.code == "resource_limit");
  CHECK(first->field == "catalog.shapes");
  auto same = [&](const registration_error& error) {
    CHECK(error.detail.code == first->detail.code);
    CHECK(error.detail.message == first->detail.message);
    CHECK(error.detail.limit == first->detail.limit);
    CHECK(error.detail.maximum == first->detail.maximum);
    CHECK(error.field == first->field);
  };
  for (int retry = 0; retry < 2; ++retry) {
    for (auto id : {"missing", "visual", "custom"}) {
      for (auto input : {value_ptr{}, shape_input()}) {
        auto response = vm->invoke_shape(id, input);
        REQUIRE_FALSE(response);
        same(response.error());
      }
      auto definition = vm->resolve_shape_definition(id);
      REQUIRE_FALSE(definition);
      same(definition.error());
    }
  }
}

namespace {
value_ptr string_value(std::string bytes) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = std::move(bytes);
  return v;
}
value_ptr record_value(std::map<std::string, value_ptr> fields = {}) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::record;
  v->fields = std::move(fields);
  return v;
}
value_ptr array_value(std::vector<value_ptr> items = {}) {
  auto v   = std::make_shared<value>();
  v->kind  = value_kind::list;
  v->items = std::move(items);
  return v;
}
value_ptr changed(value_ptr original, std::string key, value_ptr replacement) {
  auto v = std::make_shared<value>(*original);
  if (replacement)
    v->fields[std::move(key)] = std::move(replacement);
  else
    v->fields.erase(key);
  return v;
}
std::string read_bytes(const std::filesystem::path& file) {
  std::ifstream stream(file, std::ios::binary);
  REQUIRE(stream.good());
  return {std::istreambuf_iterator<char>(stream), {}};
}
value_ptr invoke(std::string_view entry, value_ptr input) {
  auto result = run_trusted(entry, input);
  INFO(entry);
  INFO((result ? "native success" : result.error().message));
  REQUIRE(result);
  return *result;
}
value_ptr success(std::string_view entry, value_ptr input) {
  auto result = invoke(entry, input);
  if (result->fields.contains("error")) {
    auto e = result->fields.at("error");
    INFO(e->fields.at("file")->string);
    INFO(e->fields.at("field")->string);
    INFO(e->fields.at("message")->string);
    FAIL("Unexpected schema refusal");
  }
  return result->fields.at("value");
}
struct semantic_fixture {
  std::filesystem::path root;
  value_ptr             workspace;
  value_ptr             bundle;
  semantic_fixture(bool minimal = false) {
    std::string pattern = (std::filesystem::temp_directory_path() / "planar-schema-XXXXXX").string();
    REQUIRE(mkdtemp(pattern.data()));
    root = std::filesystem::canonical(pattern);
    std::filesystem::copy(std::filesystem::path(PLANAR_SCHEMA_FIXTURE) / "workspace", root / "workspace",
                          std::filesystem::copy_options::recursive);
    if (minimal) {
      std::ofstream spec(root / "workspace/spec.lua");
      spec << R"(local p=planar;local L=p.list
return p.spec{version=1,id='quota',goal=p.md'Goal',work=L{
{id='library',summary=p.md'Library',paths=L{'src/core/'},checks=L{'contract'}},
{id='command',summary=p.md'Command',paths=L{'src/cli/'},checks=L{'contract'}}},
milestones=L{{id='done',work=L{'library','command'},acceptance=L{{id='pass',text=p.md'Pass'}}}}})";
    }
    capture_session capture;
    REQUIRE(capture.add_root("workspace", (root / "workspace").string()));
    auto loaded_workspace = capture.load("workspace.lua");
    REQUIRE(loaded_workspace);
    workspace  = loaded_workspace->root;
    auto route = success("route_workspace", record_value({{"workspace", workspace}, {"file", string_value("workspace.lua")}}));
    auto spec  = capture.load(route->fields.at("spec")->string);
    REQUIRE(spec);
    std::vector<value_ptr> jobs;
    for (auto& j : route->fields.at("jobs")->items) {
      auto file = j->fields.at("file");
      auto job  = capture.load(file->string);
      REQUIRE(job);
      jobs.push_back(record_value({{"file", file}, {"value", job->root}}));
    }
    auto snapshot = capture.finish();
    REQUIRE(snapshot);
    std::vector<value_ptr> files;
    for (const auto& f : **snapshot)
      files.push_back(record_value({{"path", string_value(f.path)}, {"bytes", string_value(f.bytes)}}));
    bundle =
        record_value({{"workspace", record_value({{"file", string_value("workspace.lua")}, {"value", workspace}})},
                      {"spec", record_value({{"file", route->fields.at("spec")}, {"value", spec->root}})},
                      {"jobs", array_value(jobs)},
                      {"registry", record_value({{"file", string_value("input.json")},
                                                 {"bytes", string_value(read_bytes(std::filesystem::path(PLANAR_SCHEMA_FIXTURE) /
                                                                                   "registry/quota-tools.json"))}})},
                      {"files", array_value(files)}});
  }
  ~semantic_fixture() {
    std::filesystem::remove_all(root);
  }
};
} // namespace
namespace {
std::string semantic_snapshot(const value_ptr& v) {
  std::string result =
      std::format("{}:{}:{}:{}:{}", static_cast<int>(v->kind), v->boolean, v->integer, v->string.size(), v->string);
  for (const auto& [key, child] : v->fields)
    result += std::format("{}:{}{}", key.size(), key, semantic_snapshot(child));
  result += std::format("[{}]", v->items.size());
  for (const auto& child : v->items)
    result += semantic_snapshot(child);
  return result;
}
value_ptr semantic_validation(value_ptr bundle, bool custom = true) {
  if (custom) {
    std::vector<value_ptr> jobs;
    for (auto row : bundle->fields.at("jobs")->items)
      jobs.push_back(changed(row, "value", changed(row->fields.at("value"), "shape", string_value("custom"))));
    bundle = changed(bundle, "jobs", array_value(std::move(jobs)));
  }
  return record_value({{"bundle", bundle}, {"capture", record_value({{"status", string_value("not_captured")}})}});
}
std::string semantic_shape(std::string extra = {}) {
  return callable_shape(extra +
                        " return engine.object{ok=true,value=engine.object{obligations=engine.array{},guidance=input.job.id}} ");
}
value_ptr semantic_result(command_vm& vm, value_ptr input) {
  auto result = vm.validate_profile(input);
  INFO((result ? "native success" : result.error().detail.message));
  REQUIRE(result);
  REQUIRE((*result)->fields.contains("value"));
  return (*result)->fields.at("value");
}
} // namespace
TEST_CASE("X05 B3 B actual profile phases bind same-call final definition and preserve immutable inputs",
          "[external-registration][profile-semantic]") {
  semantic_fixture f;
  auto             input  = semantic_validation(f.bundle);
  auto             before = semantic_snapshot(input);
  auto             vm     = command_vm::create(profile({semantic_shape(R"(
    assert(input.version==1)
    assert(not pcall(function() input.job.id='forged' end))
    assert(not pcall(function() input.spec.work[1].id='forged' end))
    assert(not pcall(function() D.revision='forged' end))
    local scratch={}; scratch.ok=true; assert(scratch.ok)
  )")}));
  REQUIRE(vm);
  auto definition = vm->resolve_shape_definition("custom");
  REQUIRE(definition);
  auto result = semantic_result(*vm, input);
  REQUIRE(result->fields.at("jobs")->items.size() == 2);
  for (auto job : result->fields.at("jobs")->items) {
    auto selected = job->fields.at("selected_shape");
    CHECK(selected->fields.at("id")->string == "custom");
    CHECK(selected->fields.at("revision")->string == definition->revision);
    CHECK(selected->fields.at("definition")->string == definition->digest);
    CHECK(selected->fields.at("guidance")->string == job->fields.at("id")->string);
    CHECK(selected->fields.at("obligations")->kind == value_kind::list);
  }
  CHECK(semantic_snapshot(input) == before);
  CHECK_FALSE(result->fields.contains("shape_results"));
}
TEST_CASE("X05 B3 B all-job base failure prevents the first retained callable", "[external-registration][profile-semantic]") {
  semantic_fixture f;
  auto             input  = semantic_validation(f.bundle);
  auto             bundle = input->fields.at("bundle");
  auto             jobs   = bundle->fields.at("jobs")->items;
  jobs[1]                 = changed(jobs[1], "value", changed(jobs[1]->fields.at("value"), "unknown", string_value("invalid")));
  input                   = changed(input, "bundle", changed(bundle, "jobs", array_value(jobs)));
  auto vm                 = command_vm::create(profile({callable_shape("error('must not run')")}));
  REQUIRE(vm);
  auto result = vm->validate_profile(input);
  REQUIRE(result);
  CHECK((*result)->fields.contains("error"));
  // Semantic failure does not terminally poison the native owner.
  auto retry = vm->validate_profile(input);
  REQUIRE(retry);
  CHECK(*codec_encode(*result) == *codec_encode(*retry));
}
TEST_CASE("X05 B3 B semantic refusal and malformed response stop before later shape dispatch",
          "[external-registration][profile-semantic]") {
  semantic_fixture f;
  for (auto response :
       {"return engine.object{ok=false,error=engine.object{code='custom_refusal',message='refused',field='job.shape'}}",
        "return engine.object{ok=true,value=engine.object{}}"}) {
    auto vm = command_vm::create(
        profile({callable_shape(std::string("if input.job.id=='split' then error('later F executed') end; ") + response)}));
    REQUIRE(vm);
    auto result = vm->validate_profile(semantic_validation(f.bundle));
    REQUIRE(result);
    CHECK((*result)->fields.contains("error"));
    auto retry = vm->validate_profile(semantic_validation(f.bundle));
    REQUIRE(retry);
    CHECK(*codec_encode(*result) == *codec_encode(*retry));
  }
}
TEST_CASE("X05 B3 B embedded-only semantic result remains exact through selected profile phases",
          "[external-registration][profile-semantic]") {
  semantic_fixture f;
  auto             input    = semantic_validation(f.bundle, false);
  auto             expected = run_trusted("validate_bundle", input);
  REQUIRE(expected);
  auto vm = command_vm::create(profile({command()}));
  REQUIRE(vm);
  auto result = vm->validate_profile(input);
  REQUIRE(result);
  CHECK(*codec_encode(*result) == *codec_encode(*expected));
}
TEST_CASE("X05 B3 B admission rejects wrong owners and retains private refs across moves and repeats",
          "[external-registration][profile-semantic]") {
  semantic_fixture f;
  auto             input    = semantic_validation(f.bundle);
  auto             embedded = command_vm::create();
  REQUIRE(embedded);
  CHECK_FALSE(embedded->validate_profile(input));
  auto owner = command_vm::create(profile({semantic_shape()}));
  REQUIRE(owner);
  auto vm = std::move(*owner);
  CHECK_FALSE(owner->validate_profile(input));
  CHECK_FALSE(vm.validate_profile({}));
  auto first = semantic_result(vm, input);
  for (int i = 0; i < 12; ++i)
    CHECK(*codec_encode(semantic_result(vm, input)) == *codec_encode(first));
  CHECK(vm.resolve_shape_definition("custom"));
}
TEST_CASE("X05 B3 B native callable failure is terminal with its first logical diagnostic",
          "[external-registration][profile-semantic]") {
  semantic_fixture f;
  auto             vm = command_vm::create(profile({callable_shape("error('private source detail')")}));
  REQUIRE(vm);
  auto result = vm->validate_profile(semantic_validation(f.bundle));
  REQUIRE_FALSE(result);
  CHECK(result.error().detail.code == "internal");
  CHECK(result.error().field == "catalog.shapes[1]");
  CHECK(result.error().detail.message == "Invalid external profile protocol");
  auto retry = vm->validate_profile({});
  REQUIRE_FALSE(retry);
  CHECK(retry.error().field == result.error().field);
  CHECK(retry.error().detail.message == result.error().detail.message);
}
TEST_CASE("X05 B3 B validation input cannot inject phase context or select exported callables",
          "[external-registration][profile-semantic]") {
  semantic_fixture f;
  auto             vm = command_vm::create(profile({semantic_shape()}));
  REQUIRE(vm);
  auto input = semantic_validation(f.bundle);
  for (auto key : {"context", "shape_prepare", "definitions", "results", "dialect"}) {
    auto result = vm->validate_profile(changed(input, key, string_value("forged")));
    REQUIRE(result);
    CHECK((*result)->fields.contains("error"));
  }
  CHECK(semantic_result(*vm, input));
}
TEST_CASE("X05 B3 B additive graph obligations preserve base edges and report grouped cycles",
          "[external-registration][profile-semantic]") {
  semantic_fixture f;
  for (bool cycle : {false, true}) {
    auto body = std::string("return engine.object{ok=true,value=engine.object{guidance='edge',obligations=engine.array{") +
                "engine.object{id='edge',producer='" + (cycle ? "command" : "library") + "',consumer='" +
                (cycle ? "library" : "command") + "',phase='start',quality='done'}}}}";
    auto vm   = command_vm::create(profile({callable_shape(body)}));
    REQUIRE(vm);
    auto result = vm->validate_profile(semantic_validation(f.bundle));
    REQUIRE(result);
    if (cycle) {
      REQUIRE((*result)->fields.contains("error"));
      CHECK((*result)->fields.at("error")->fields.at("code")->string == "dependency_cycle");
    } else {
      auto models = (*result)->fields.at("value")->fields.at("jobs");
      for (auto model : models->items) {
        auto obligation = model->fields.at("selected_shape")->fields.at("obligations")->items[0];
        CHECK(obligation->fields.at("id")->string == "shape:custom:edge");
        CHECK(model->fields.at("graph")->fields.at("edges")->items.size() > 0);
      }
    }
  }
}
TEST_CASE("X05 B3 B final native descriptor stage policy applies before retained F",
          "[external-registration][profile-semantic]") {
  semantic_fixture f;
  auto             source = callable_shape("error('must not execute')");
  auto             start  = source.find("stages='optional'");
  REQUIRE(start != std::string::npos);
  source.replace(start, std::string("stages='optional'").size(), "stages='required'");
  auto vm = command_vm::create(profile({source}));
  REQUIRE(vm);
  auto result = vm->validate_profile(semantic_validation(f.bundle));
  REQUIRE(result);
  CHECK((*result)->fields.contains("error"));
}
TEST_CASE("X05 B3 B repeated semantic operations share finite budgets and retain first terminal refusal",
          "[external-registration][profile-semantic]") {
  semantic_fixture f;
  auto             vm =
      command_vm::create(profile({semantic_shape("local count=0; for i=1,500000 do count=count+1 end; assert(count==500000)")}));
  REQUIRE(vm);
  auto input     = semantic_validation(f.bundle);
  int  successes = 0;
  for (int i = 0; i < 150; ++i) {
    auto result = vm->validate_profile(input);
    if (!result) {
      CHECK(result.error().detail.code == "resource_limit");
      CHECK(successes > 1);
      auto retry = vm->validate_profile({});
      REQUIRE_FALSE(retry);
      CHECK(retry.error().field == result.error().field);
      CHECK(retry.error().detail.limit == result.error().detail.limit);
      return;
    }
    REQUIRE((*result)->fields.contains("value"));
    ++successes;
  }
  FAIL("repeated operations unexpectedly reset cumulative budgets");
}
TEST_CASE("X05 B3 B ordinary registry JSON refusal permits a later complete semantic operation",
          "[external-registration][profile-semantic]") {
  semantic_fixture f;
  auto             vm = command_vm::create(profile({semantic_shape()}));
  REQUIRE(vm);
  auto registry  = changed(f.bundle->fields.at("registry"), "bytes", string_value("{"));
  auto malformed = semantic_validation(changed(f.bundle, "registry", registry), false);
  auto embedded  = invoke("validate_bundle", malformed);
  REQUIRE(embedded->fields.contains("error"));
  for (int i = 0; i < 2; ++i) {
    auto result = vm->validate_profile(malformed);
    REQUIRE(result);
    REQUIRE((*result)->fields.contains("error"));
    CHECK(*codec_encode((*result)->fields.at("error")) == *codec_encode(embedded->fields.at("error")));
  }
  auto input = semantic_validation(f.bundle);
  CHECK(semantic_result(*vm, input));
  CHECK(semantic_result(*vm, input));
}

namespace {
value_ptr selection_request(value_ptr validation, std::string job = "split", std::string role = "library_worker") {
  auto                   bundle = validation->fields.at("bundle");
  std::vector<value_ptr> files;
  for (auto f : bundle->fields.at("files")->items)
    files.push_back(record_value({{"root", string_value("workspace")},
                                  {"path", f->fields.at("path")},
                                  {"mode", string_value("0644")},
                                  {"bytes", f->fields.at("bytes")}}));
  files.push_back(record_value({{"root", string_value("registry")},
                                {"path", string_value("input.json")},
                                {"mode", string_value("0644")},
                                {"bytes", bundle->fields.at("registry")->fields.at("bytes")}}));
  return record_value({{"bundle", bundle},
                       {"capture", validation->fields.at("capture")},
                       {"files", array_value(files)},
                       {"job", string_value(job)},
                       {"role", string_value(role)}});
}
value_ptr profile_selected(command_vm& vm, value_ptr request) {
  auto result = vm.select_profile(request);
  INFO((result ? "native success" : result.error().detail.message));
  REQUIRE(result);
  if ((*result)->fields.contains("error")) {
    INFO(*codec_encode(*result));
    FAIL("Unexpected selection refusal");
  }
  return (*result)->fields.at("value");
}
std::filesystem::path selection_support() {
  return std::filesystem::path(PLANAR_SCHEMA_FIXTURE).parent_path().parent_path().parent_path() /
         "tests/native/v5-02-x05-b3-c-support";
}
std::string selection_shape() {
  return read_bytes(selection_support() / "shape.lua");
}
std::string selected_work(command_vm& vm, value_ptr request) {
  return *codec_encode(profile_selected(vm, request)->fields.at("work"));
}
} // namespace
TEST_CASE("X05 B3 C native custom selection matches independently authored W and domain oracle",
          "[external-registration][profile-selection]") {
  semantic_fixture f(true);
  auto             request = selection_request(semantic_validation(f.bundle));
  auto             vm      = command_vm::create(profile({selection_shape()}));
  REQUIRE(vm);
  auto selected = profile_selected(*vm, request);
  auto expected = codec_decode(read_bytes(selection_support() / "expected-library-worker.json"));
  REQUIRE(expected);
  CHECK(*codec_encode(selected->fields.at("work")) == *codec_encode(*expected));
}
TEST_CASE("X05 B3 C shipped selection remains byte exact through profile semantics for every job role",
          "[external-registration][profile-selection]") {
  semantic_fixture f;
  for (auto job : {"split", "combined"})
    for (auto role : {"library_worker", "cli_worker", "builder", "reviewer"}) {
      if ((std::string(job) == "combined") != (std::string(role) == "builder" || std::string(role) == "reviewer")) {
        if (std::string(role) != "reviewer")
          continue;
      }
      auto request  = selection_request(semantic_validation(f.bundle, false), job, role);
      auto expected = invoke("select_role", request);
      auto vm       = command_vm::create(profile({selection_shape(), command()}));
      REQUIRE(vm);
      auto actual = vm->select_profile(request);
      REQUIRE(actual);
      CHECK(*codec_encode(*actual) == *codec_encode(expected));
    }
}
TEST_CASE("X05 B3 C obligations retain unit endpoints group semantics and synthetic ID sorting",
          "[external-registration][profile-selection]") {
  semantic_fixture f(true);
  for (auto job : {"split", "combined"})
    for (auto role : {"library_worker", "cli_worker", "reviewer", "builder"}) {
      if (std::string(job) == "combined" && std::string(role) != "builder" && std::string(role) != "reviewer")
        continue;
      if (std::string(job) == "split" && std::string(role) == "builder")
        continue;
      auto vm = command_vm::create(profile({selection_shape()}));
      REQUIRE(vm);
      auto selected    = profile_selected(*vm, selection_request(semantic_validation(f.bundle), job, role));
      auto obligations = selected->fields.at("work")->fields.at("value")->fields.at("shape")->fields.at("obligations");
      REQUIRE(obligations->items.size() == 2);
      CHECK(obligations->items[0]->fields.at("id")->string == "shape:custom:a");
      CHECK(obligations->items[1]->fields.at("id")->string == "shape:custom:z");
      CHECK(obligations->items[0]->fields.at("producer")->string == "library");
      CHECK(obligations->items[0]->fields.at("consumer")->string == "command");
      if (std::string(job) == "combined")
        CHECK(obligations->items[0]->fields.at("producer_role")->string == "builder");
    }
}
TEST_CASE("X05 B3 C selected module bytes and final revision affect every custom job role",
          "[external-registration][profile-selection]") {
  semantic_fixture f(true);
  for (auto role : {"library_worker", "cli_worker", "reviewer"}) {
    auto request = selection_request(semantic_validation(f.bundle), "split", role);
    auto base    = command_vm::create(profile({selection_shape()}));
    REQUIRE(base);
    auto baseline = selected_work(*base, request);
    for (auto source : {selection_shape() + "-- comment\n", selection_shape() + " D.revision='other'\n"}) {
      auto vm = command_vm::create(profile({source}));
      REQUIRE(vm);
      CHECK(selected_work(*vm, request) != baseline);
    }
  }
}
TEST_CASE("X05 B3 C unrelated modules change provenance while custom W remains exact",
          "[external-registration][profile-selection]") {
  semantic_fixture f(true);
  auto             request = selection_request(semantic_validation(f.bundle));
  auto             base    = command_vm::create(profile({selection_shape()}));
  REQUIRE(base);
  auto w = selected_work(*base, request);
  for (auto extra :
       {command(),
        std::string("registration.shape({id='other',abi='shape_v1',revision='v1',stages='optional',help=''},"
                    "function()error('unselected')end)"),
        std::string("registration.projection({id='view',abi='projection_v1',revision='v1',input='role_context_v1',help=''},"
                    "function()return {}end)")}) {
    auto vm = command_vm::create(profile({selection_shape(), extra}));
    REQUIRE(vm);
    CHECK(vm->effective_digest() != base->effective_digest());
    CHECK(selected_work(*vm, request) == w);
  }
}
TEST_CASE("X05 B3 C guidance from nonmaterial notes leaves selected definition and W unchanged",
          "[external-registration][profile-selection]") {
  semantic_fixture f(true);
  auto             validation = semantic_validation(f.bundle);
  auto             request    = selection_request(validation);
  auto             jobs       = validation->fields.at("bundle")->fields.at("jobs")->items;
  // The captured F source stays fixed. Omit/edit an existing immaterial notes constructor.
  auto original = jobs[1]->fields.at("value");
  jobs[1]       = changed(jobs[1], "value", changed(original, "notes", f.workspace->fields.at("notes")));
  auto modified =
      selection_request(changed(validation, "bundle", changed(validation->fields.at("bundle"), "jobs", array_value(jobs))));
  auto a = command_vm::create(profile({selection_shape()}));
  auto b = command_vm::create(profile({selection_shape()}));
  REQUIRE(a);
  REQUIRE(b);
  CHECK(selected_work(*a, request) == selected_work(*b, modified));
}
TEST_CASE("X05 B3 C selection refuses caller models and validates all jobs before role lookup",
          "[external-registration][profile-selection]") {
  semantic_fixture f(true);
  auto             request = selection_request(semantic_validation(f.bundle));
  auto             vm      = command_vm::create(profile({selection_shape()}));
  REQUIRE(vm);
  for (auto key : {"model", "context", "definitions", "selected_shape", "select_profile_material"}) {
    auto result = vm->select_profile(changed(request, key, record_value({})));
    REQUIRE(result);
    CHECK((*result)->fields.contains("error"));
  }
  auto validation = semantic_validation(f.bundle);
  auto jobs       = validation->fields.at("bundle")->fields.at("jobs")->items;
  jobs[0]         = changed(jobs[0], "value", changed(jobs[0]->fields.at("value"), "unknown", string_value("invalid")));
  auto invalid    = selection_request(
      changed(validation, "bundle", changed(validation->fields.at("bundle"), "jobs", array_value(jobs))), "unknown", "unknown");
  auto result = vm->select_profile(invalid);
  REQUIRE(result);
  REQUIRE((*result)->fields.contains("error"));
  CHECK((*result)->fields.at("error")->fields.at("field")->string != "selection.job");
}
TEST_CASE("X05 B3 C retained selection root survives repeats moves and immutable result lifetime",
          "[external-registration][profile-selection]") {
  semantic_fixture f(true);
  auto             request = selection_request(semantic_validation(f.bundle));
  auto             before  = semantic_snapshot(request);
  auto             owner   = command_vm::create(profile({selection_shape()}));
  REQUIRE(owner);
  auto vm = std::move(*owner);
  CHECK_FALSE(owner->select_profile(request));
  auto result = profile_selected(vm, request);
  auto first  = *codec_encode(result);
  for (int i = 0; i < 10; ++i)
    CHECK(*codec_encode(profile_selected(vm, request)) == first);
  CHECK(semantic_snapshot(request) == before);
  vm = std::move(*command_vm::create(profile({selection_shape()})));
  CHECK(*codec_encode(result) == first);
  auto embedded = command_vm::create();
  REQUIRE(embedded);
  CHECK_FALSE(embedded->select_profile(request));
}
TEST_CASE("X05 B3 C native shape failure remains terminal through selection retries",
          "[external-registration][profile-selection]") {
  semantic_fixture f;
  auto             vm = command_vm::create(profile({callable_shape("error('private')")}));
  REQUIRE(vm);
  auto failed = vm->select_profile(selection_request(semantic_validation(f.bundle)));
  REQUIRE_FALSE(failed);
  auto retry = vm->select_profile({});
  REQUIRE_FALSE(retry);
  CHECK(retry.error().field == failed.error().field);
  CHECK(retry.error().detail.message == failed.error().detail.message);
}

TEST_CASE("X05 B3 C custom obligations outside selected units are excluded without losing selected shape identity",
          "[external-registration][profile-selection]") {
  semantic_fixture f(true);
  auto             bundle = semantic_validation(f.bundle)->fields.at("bundle");
  auto             spec   = bundle->fields.at("spec");
  auto             work   = spec->fields.at("value")->fields.at("work")->items;
  work.push_back(changed(changed(work[0], "id", string_value("third")), "paths", array_value({string_value("src/third/")})));
  bundle          = changed(bundle, "spec", changed(spec, "value", changed(spec->fields.at("value"), "work", array_value(work))));
  spec            = bundle->fields.at("spec");
  auto milestones = spec->fields.at("value")->fields.at("milestones")->items;
  auto milestone_work = milestones[0]->fields.at("work")->items;
  milestone_work.push_back(string_value("third"));
  milestones[0] = changed(milestones[0], "work", array_value(milestone_work));
  bundle =
      changed(bundle, "spec", changed(spec, "value", changed(spec->fields.at("value"), "milestones", array_value(milestones))));
  auto jobs = bundle->fields.at("jobs")->items;
  for (auto& row : jobs) {
    auto job   = row->fields.at("value");
    auto roles = job->fields.at("roles")->items;
    for (auto& role : roles)
      if (role->fields.at("kind")->string == "reviewer") {
        auto targets = role->fields.at("targets")->items;
        targets.push_back(string_value("third"));
        role = changed(role, "targets", array_value(targets));
      }
    roles.push_back(record_value(
        {{"id", string_value("observer")}, {"kind", string_value("worker")}, {"work", array_value({string_value("third")})}}));
    row = changed(row, "value", changed(job, "roles", array_value(roles)));
  }
  auto vm = command_vm::create(profile({selection_shape()}));
  REQUIRE(vm);
  auto request  = selection_request(semantic_validation(changed(bundle, "jobs", array_value(jobs))), "split", "observer");
  auto selected = profile_selected(*vm, request);
  auto shape    = selected->fields.at("work")->fields.at("value")->fields.at("shape");
  CHECK(shape->fields.at("obligations")->items.empty());
  CHECK_FALSE(shape->fields.at("definition")->string.empty());
}
TEST_CASE("X05 B3 C same-module presentation source changes conservatively couple custom W",
          "[external-registration][profile-selection]") {
  semantic_fixture f(true);
  auto             request = selection_request(semantic_validation(f.bundle));
  auto             base    = command_vm::create(profile({selection_shape()}));
  REQUIRE(base);
  auto edited =
      command_vm::create(profile({selection_shape() + "registration.projection({id='view',abi='projection_v1',revision='v1',"
                                                      "input='role_context_v1',help=''},function()return {}end)"}));
  REQUIRE(edited);
  CHECK(base->effective_digest() != edited->effective_digest());
  CHECK(selected_work(*base, request) != selected_work(*edited, request));
}
TEST_CASE("X05 B3 C shipped pipeline W and complete selection stay exact with a selected profile",
          "[external-registration][profile-selection]") {
  semantic_fixture f;
  auto             bundle = f.bundle;
  auto             jobs   = bundle->fields.at("jobs")->items;
  for (auto& row : jobs) {
    auto                   job = row->fields.at("value");
    std::vector<value_ptr> stages;
    for (auto role : job->fields.at("roles")->items)
      if (role->fields.at("kind")->string == "worker")
        stages.push_back(array_value({role->fields.at("id")}));
    row = changed(row, "value", changed(changed(job, "shape", string_value("pipeline")), "stages", array_value(stages)));
  }
  bundle = changed(bundle, "jobs", array_value(jobs));
  for (auto role : {"library_worker", "cli_worker", "reviewer"}) {
    auto request  = selection_request(semantic_validation(bundle, false), "split", role);
    auto expected = invoke("select_role", request);
    REQUIRE(expected->fields.contains("value"));
    auto vm = command_vm::create(profile({selection_shape()}));
    REQUIRE(vm);
    auto actual = vm->select_profile(request);
    REQUIRE(actual);
    CHECK(*codec_encode(*actual) == *codec_encode(expected));
  }
}
TEST_CASE("X05 B3 C zero and single scoped obligation arrays survive native selection transport",
          "[external-registration][profile-selection]") {
  semantic_fixture f(true);
  for (bool single : {false, true}) {
    auto source = callable_shape(
        std::string("return engine.object{ok=true,value=engine.object{guidance='',obligations=engine.array{") +
        (single ? "engine.object{id='one',producer='library',consumer='command',phase='start',quality='done'}" : "") + "}}}");
    auto vm = command_vm::create(profile({source}));
    REQUIRE(vm);
    auto selected = profile_selected(*vm, selection_request(semantic_validation(f.bundle)));
    auto shape    = selected->fields.at("work")->fields.at("value")->fields.at("shape");
    CHECK(shape->fields.at("obligations")->items.size() == (single ? 1 : 0));
  }
}

namespace {
// Pure helper composition only: independent native semantic/selection setup is
// deliberately not evidence for the future same-call native render operation.
value_ptr render_material_input(value_ptr validation, value_ptr request, std::string source) {
  auto vm = command_vm::create(profile({source}));
  REQUIRE(vm);
  auto model    = semantic_result(*vm, validation);
  auto selected = profile_selected(*vm, request);
  return record_value({{"request", request}, {"model", model}, {"selected", selected}});
}
value_ptr render_number(std::size_t n) {
  auto v     = std::make_shared<value>();
  v->kind    = value_kind::integer;
  v->integer = static_cast<std::int64_t>(n);
  return v;
}
std::filesystem::path render_expected() {
  return std::filesystem::path(PLANAR_SCHEMA_FIXTURE).parent_path().parent_path() / "render/external-shape/expected";
}
} // namespace
TEST_CASE("X05 B4 A pure custom rendering preserves exact envelope policy and full prerequisites",
          "[external-registration][profile-render]") {
  semantic_fixture f(true);
  auto             validation = semantic_validation(f.bundle);
  for (auto role : {"library_worker", "cli_worker", "reviewer"}) {
    auto input  = render_material_input(validation, selection_request(validation, "split", role), selection_shape());
    auto before = semantic_snapshot(input);
    auto result = success("render_profile_material", input);
    CHECK(semantic_snapshot(input) == before);
    CHECK(*codec_encode(success("render_profile_material", input)) == *codec_encode(result));
    CHECK(result->fields.size() == 5);
    CHECK(result->fields.at("kind")->string == "render_external");
    CHECK(result->fields.at("version")->integer == 1);
    auto selected = input->fields.at("selected");
    auto W        = selected->fields.at("work")->fields.at("value");
    auto identity = record_value({{"id", string_value("custom")},
                                  {"revision", string_value("final")},
                                  {"definition", W->fields.at("shape")->fields.at("definition")}});
    CHECK(*codec_encode(result->fields.at("shape")) == *codec_encode(identity));
    auto manifest = result->fields.at("manifest");
    CHECK(*codec_encode(manifest->fields.at("selected_shape")) == *codec_encode(identity));
    auto versions = std::make_shared<value>(*W->fields.at("versions"));
    versions->items.push_back(string_value("projection_v2"));
    CHECK(*codec_encode(manifest->fields.at("versions")) == *codec_encode(versions));
    auto markdown = result->fields.at("packet")->fields.at("markdown")->string;
    auto policy   = read_bytes(render_expected() / "shape-policy.md");
    CHECK(markdown.find(policy) != std::string::npos);
    auto sections = manifest->fields.at("sections");
    REQUIRE(sections->items.size() > 7);
    CHECK(sections->items[6]->fields.at("id")->string == "dependencies");
    CHECK(sections->items[7]->fields.at("id")->string == "shape_policy");
    CHECK(sections->items[7]->fields.at("mandatory")->boolean);
    CHECK(sections->items[7]->fields.at("bytes")->integer == static_cast<std::int64_t>(policy.size() + 1));
    std::int64_t total = 0;
    for (auto section : sections->items)
      if (section->fields.at("included")->boolean)
        total += section->fields.at("bytes")->integer;
    CHECK(total == static_cast<std::int64_t>(markdown.size()));
    auto expected = codec_decode(read_bytes(render_expected() / "prerequisites.json"));
    REQUIRE(expected);
    CHECK(*codec_encode(manifest->fields.at("prerequisites")) == *codec_encode(*expected));
    CHECK(result->fields.at("packet")->fields.at("version")->integer == 1);
  }
}
TEST_CASE("X05 B4 A pure helper shipped shapes exactly preserve every existing role render",
          "[external-registration][profile-render]") {
  semantic_fixture f;
  auto             validation = semantic_validation(f.bundle, false);
  for (auto pair : {std::pair{"split", "library_worker"},
                    {"split", "cli_worker"},
                    {"split", "reviewer"},
                    {"combined", "builder"},
                    {"combined", "reviewer"}}) {
    auto request = selection_request(validation, pair.first, pair.second);
    auto input   = render_material_input(validation, request, selection_shape());
    CHECK(*codec_encode(invoke("render_profile_material", input)) == *codec_encode(invoke("render", request)));
  }
}
TEST_CASE("X05 B4 A pure helper refuses mismatched job role and shape identities", "[external-registration][profile-render]") {
  semantic_fixture f(true);
  auto             validation = semantic_validation(f.bundle);
  auto             input      = render_material_input(validation, selection_request(validation), selection_shape());
  auto             selected   = input->fields.at("selected");
  auto             work       = selected->fields.at("work");
  for (auto key : {"definition", "revision", "kind"}) {
    auto W        = work->fields.at("value");
    auto shape    = changed(W->fields.at("shape"), key, string_value("forged"));
    auto mismatch = changed(input, "selected", changed(selected, "work", changed(work, "value", changed(W, "shape", shape))));
    auto result   = invoke("render_profile_material", mismatch);
    REQUIRE(result->fields.contains("error"));
    CHECK(result->fields.at("error")->fields.at("field")->string == "render.selected");
  }
  for (auto key : {"job", "role"}) {
    auto result = invoke("render_profile_material",
                         changed(input, "request", changed(input->fields.at("request"), key, string_value("other"))));
    REQUIRE(result->fields.contains("error"));
  }
}
TEST_CASE("X05 B4 A pure helper mandatory policy participates in exact byte budget", "[external-registration][profile-render]") {
  semantic_fixture f(true);
  auto             validation = semantic_validation(f.bundle);
  auto             input      = render_material_input(validation, selection_request(validation), selection_shape());
  // Remove optional notes through trusted structural test setup; no product state mutates.
  auto selected = input->fields.at("selected");
  auto context  = changed(selected->fields.at("context"), "optional", record_value());
  input         = changed(input, "selected", changed(selected, "context", context));
  auto rendered = success("render_profile_material", input);
  auto bytes    = rendered->fields.at("packet")->fields.at("markdown")->string.size();
  auto request  = changed(input->fields.at("request"), "budget_bytes", render_number(bytes));
  CHECK(success("render_profile_material", changed(input, "request", request)));
  request      = changed(request, "budget_bytes", render_number(bytes - 1));
  auto refused = invoke("render_profile_material", changed(input, "request", request));
  REQUIRE(refused->fields.contains("error"));
  auto error = refused->fields.at("error");
  CHECK(error->fields.at("code")->string == "budget_exceeded");
  CHECK(error->fields.at("details")->fields.at("mandatory_bytes")->integer == static_cast<std::int64_t>(bytes));
  bool policy = false;
  for (auto section : error->fields.at("details")->fields.at("sections")->items)
    if (section->fields.at("id")->string == "shape_policy")
      policy = true;
  CHECK(policy);
}
TEST_CASE("X05 B4 A empty singleton and grouped shape policies retain complete normalized scoped records",
          "[external-registration][profile-render]") {
  semantic_fixture f(true);
  for (auto job : {"split", "combined"})
    for (bool single : {false, true}) {
      auto source = callable_shape(
          std::string("return engine.object{ok=true,value=engine.object{guidance='',obligations=engine.array{") +
          (single ? "engine.object{id='one',producer='library',consumer='command',phase='start',quality='done'}" : "") + "}}}");
      auto validation = semantic_validation(f.bundle);
      auto input      = render_material_input(
          validation, selection_request(validation, job, std::string(job) == "split" ? "library_worker" : "builder"), source);
      auto result        = success("render_profile_material", input);
      auto prerequisites = result->fields.at("manifest")->fields.at("prerequisites");
      REQUIRE(prerequisites->items.size() == (single ? 1 : 0));
      if (single) {
        auto expected = codec_decode(std::string(R"([{"consumer":"command","consumer_role":")") +
                                     (std::string(job) == "split" ? "cli_worker" : "builder") +
                                     R"(","id":"shape:custom:one","phase":"start","producer":"library","producer_role":")" +
                                     (std::string(job) == "split" ? "library_worker" : "builder") +
                                     R"(","quality":"done","source":"shape","status":"unverified"}])");
        REQUIRE(expected);
        CHECK(*codec_encode(prerequisites) == *codec_encode(*expected));
      }
      CHECK(result->fields.at("packet")
                ->fields.at("markdown")
                ->string.find("shape guidance; trusted policy\n```\n\n```\nshape obligations; canonical JSON\n```\n" +
                              std::string(single ? "[{" : "[]")) != std::string::npos);
    }
}
TEST_CASE("X05 B4 A guidance uses exact display controls fences and remains outside W",
          "[external-registration][profile-render]") {
  semantic_fixture f(true);
  auto             validation = semantic_validation(f.bundle);
  auto             input      = render_material_input(validation, selection_request(validation), semantic_shape());
  auto             original   = success("render_profile_material", input);
  auto             model      = input->fields.at("model");
  auto             jobs       = model->fields.at("jobs")->items;
  for (auto& job : jobs) {
    auto shape = job->fields.at("selected_shape");
    job        = changed(job, "selected_shape", changed(shape, "guidance", string_value("quoted ```\n\x01\xc2\x85")));
  }
  auto altered  = changed(input, "model", changed(model, "jobs", array_value(jobs)));
  auto rendered = success("render_profile_material", altered);
  auto markdown = rendered->fields.at("packet")->fields.at("markdown")->string;
  CHECK(markdown.find("## shape_policy\n\nshape guidance; trusted policy; display controls escaped as \\u00xx\n````\nquoted "
                      "```\n\\u0001\\u0085\n````\nshape obligations; canonical JSON\n```\n[]\n```\n") != std::string::npos);
  CHECK(*codec_encode(rendered->fields.at("manifest")->fields.at("work")) ==
        *codec_encode(original->fields.at("manifest")->fields.at("work")));
  CHECK(*codec_encode(rendered->fields.at("shape")) == *codec_encode(original->fields.at("shape")));
}
TEST_CASE("X05 B4 A observation errors precede custom mandatory budget refusal", "[external-registration][profile-render]") {
  semantic_fixture f(true);
  auto             validation = semantic_validation(f.bundle);
  auto             input      = render_material_input(validation, selection_request(validation), selection_shape());
  auto             request    = input->fields.at("request");
  auto             files      = request->fields.at("files")->items;
  files.push_back(record_value({{"root", string_value("observations")},
                                {"path", string_value("input.json")},
                                {"mode", string_value("0644")},
                                {"bytes", string_value("{")}}));
  request      = changed(changed(request, "files", array_value(files)), "budget_bytes", render_number(1));
  auto refused = invoke("render_profile_material", changed(input, "request", request));
  REQUIRE(refused->fields.contains("error"));
  CHECK(refused->fields.at("error")->fields.at("code")->string != "budget_exceeded");
  CHECK(refused->fields.at("error")->fields.at("file")->string == "input.json");
}
TEST_CASE("X05 B4 A optional omission preserves mandatory shape policy and exact old removal order",
          "[external-registration][profile-render]") {
  semantic_fixture f(true);
  auto             validation = semantic_validation(f.bundle);
  auto             input      = render_material_input(validation, selection_request(validation), selection_shape());
  auto             selected   = input->fields.at("selected");
  auto             context    = selected->fields.at("context");
  auto             note       = record_value({{"bytes", string_value(std::string(20000, 'x'))}});
  auto             optional   = record_value({{"rationale", note}, {"workspace_notes", note}, {"job_notes", note}});
  input                       = changed(input, "selected", changed(selected, "context", changed(context, "optional", optional)));
  auto request                = changed(input->fields.at("request"), "budget_bytes", render_number(16000));
  auto result                 = success("render_profile_material", changed(input, "request", request));
  auto expected               = codec_decode(R"(["job_notes","workspace_notes","rationale"])");
  REQUIRE(expected);
  CHECK(*codec_encode(result->fields.at("manifest")->fields.at("omitted")) == *codec_encode(*expected));
  CHECK(result->fields.at("packet")->fields.at("markdown")->string.find(read_bytes(render_expected() / "shape-policy.md")) !=
        std::string::npos);
}

TEST_CASE("X05 B4 A nonincident obligations render empty policy without mutating selection",
          "[external-registration][profile-render]") {
  semantic_fixture f(true);
  auto             bundle = semantic_validation(f.bundle)->fields.at("bundle");
  auto             spec   = bundle->fields.at("spec");
  auto             work   = spec->fields.at("value")->fields.at("work")->items;
  work.push_back(changed(changed(work[0], "id", string_value("third")), "paths", array_value({string_value("src/third/")})));
  bundle          = changed(bundle, "spec", changed(spec, "value", changed(spec->fields.at("value"), "work", array_value(work))));
  spec            = bundle->fields.at("spec");
  auto milestones = spec->fields.at("value")->fields.at("milestones")->items;
  auto milestone_work = milestones[0]->fields.at("work")->items;
  milestone_work.push_back(string_value("third"));
  milestones[0] = changed(milestones[0], "work", array_value(milestone_work));
  bundle =
      changed(bundle, "spec", changed(spec, "value", changed(spec->fields.at("value"), "milestones", array_value(milestones))));
  auto jobs = bundle->fields.at("jobs")->items;
  for (auto& row : jobs) {
    auto job   = row->fields.at("value");
    auto roles = job->fields.at("roles")->items;
    for (auto& role : roles)
      if (role->fields.at("kind")->string == "reviewer") {
        auto targets = role->fields.at("targets")->items;
        targets.push_back(string_value("third"));
        role = changed(role, "targets", array_value(targets));
      }
    roles.push_back(record_value(
        {{"id", string_value("observer")}, {"kind", string_value("worker")}, {"work", array_value({string_value("third")})}}));
    row = changed(row, "value", changed(job, "roles", array_value(roles)));
  }
  auto validation = semantic_validation(changed(bundle, "jobs", array_value(jobs)));
  auto input      = render_material_input(validation, selection_request(validation, "split", "observer"), selection_shape());
  auto before     = semantic_snapshot(input);
  auto result     = success("render_profile_material", input);
  CHECK(result->fields.at("manifest")->fields.at("prerequisites")->items.empty());
  CHECK(result->fields.at("packet")->fields.at("markdown")->string.find("shape obligations; canonical JSON\n```\n[]\n```\n") !=
        std::string::npos);
  CHECK(semantic_snapshot(input) == before);
}
TEST_CASE("X05 B4 A custom worker reviewer recovery checkpoints remain optional assertions",
          "[external-registration][profile-render]") {
  semantic_fixture f(true);
  auto             validation = semantic_validation(f.bundle);
  for (auto role : {"library_worker", "reviewer"}) {
    auto input   = render_material_input(validation, selection_request(validation, "split", role), selection_shape());
    auto request = input->fields.at("request");
    auto files   = request->fields.at("files")->items;
    files.push_back(record_value(
        {{"root", string_value("observations")},
         {"path", string_value("input.json")},
         {"mode", string_value("0644")},
         {"bytes",
          string_value(
              R"({"version":1,"checks":[],"findings":[],"checkpoint":{"progress":"draft","next":"review","unresolved":"none","files":[]}})")}}));
    request       = changed(request, "files", array_value(files));
    auto result   = success("render_profile_material", changed(input, "request", request));
    auto markdown = result->fields.at("packet")->fields.at("markdown")->string;
    CHECK(markdown.find("## checkpoint\n\nSession note supplied for this preview; history unavailable; no run is active.") !=
          std::string::npos);
    CHECK(markdown.find(read_bytes(render_expected() / "shape-policy.md")) != std::string::npos);
    auto manifest = result->fields.at("manifest");
    CHECK(manifest->fields.at("assertions")->fields.contains("checkpoint"));
    CHECK(manifest->fields.at("sections")->items.back()->fields.at("mandatory")->boolean == false);
  }
}

TEST_CASE("X05 B4 A C01 same-shape reversed semantic jobs refuse before guidance rendering",
          "[external-registration][profile-render][profile-render-correspondence]") {
  semantic_fixture f(true);
  auto             validation = semantic_validation(f.bundle);
  auto input = render_material_input(validation, selection_request(validation, "split", "library_worker"), semantic_shape());
  auto good  = success("render_profile_material", input);
  CHECK(good->fields.at("packet")->fields.at("markdown")->string.find("shape guidance; trusted policy\n```\nsplit\n```\n") !=
        std::string::npos);
  auto model = input->fields.at("model");
  auto jobs  = model->fields.at("jobs")->items;
  REQUIRE(jobs.size() == 2);
  CHECK(*codec_encode(jobs[0]->fields.at("selected_shape")->fields.at("definition")) ==
        *codec_encode(jobs[1]->fields.at("selected_shape")->fields.at("definition")));
  std::ranges::reverse(jobs);
  auto mismatch = changed(input, "model", changed(model, "jobs", array_value(jobs)));
  auto before   = semantic_snapshot(mismatch);
  auto result   = invoke("render_profile_material", mismatch);
  CHECK(semantic_snapshot(mismatch) == before);
  REQUIRE(result->fields.contains("error"));
  CHECK(result->fields.at("error")->fields.at("code")->string == "schema");
  CHECK(result->fields.at("error")->fields.at("field")->string == "render.selected");
  CHECK(*codec_encode(success("render_profile_material", input)) == *codec_encode(good));
}
TEST_CASE("X05 B4 A C01 complete job correspondence rejects missing duplicate and malformed identities",
          "[external-registration][profile-render][profile-render-correspondence]") {
  semantic_fixture f(true);
  auto             validation = semantic_validation(f.bundle);
  auto input  = render_material_input(validation, selection_request(validation, "split", "library_worker"), semantic_shape());
  auto model  = input->fields.at("model");
  auto bundle = model->fields.at("bundle");
  auto semantic_jobs = model->fields.at("jobs")->items;
  auto bundle_jobs   = bundle->fields.at("jobs")->items;
  REQUIRE(semantic_jobs.size() == 2);
  auto good   = *codec_encode(success("render_profile_material", input));
  auto refuse = [&](value_ptr bad_model) {
    auto mismatch = changed(input, "model", bad_model);
    auto before   = semantic_snapshot(mismatch);
    auto result   = invoke("render_profile_material", mismatch);
    CHECK(semantic_snapshot(mismatch) == before);
    REQUIRE(result->fields.contains("error"));
    auto error = result->fields.at("error");
    CHECK(error->fields.at("code")->string == "schema");
    CHECK(error->fields.at("field")->string == "render.selected");
    CHECK(error->fields.at("message")->string == "Finalized render material does not match request");
  };
  // Both selected and unselected rows must correspond, even with matching shapes.
  for (std::size_t index = 0; index < semantic_jobs.size(); ++index) {
    for (auto id : {value_ptr{}, string_value("wrong"), string_value(""), string_value("INVALID"), render_number(7),
                    record_value(), array_value()}) {
      auto rows   = semantic_jobs;
      rows[index] = changed(rows[index], "id", id);
      refuse(changed(model, "jobs", array_value(rows)));
      auto entries   = bundle_jobs;
      entries[index] = changed(entries[index], "value", changed(entries[index]->fields.at("value"), "id", id));
      refuse(changed(model, "bundle", changed(bundle, "jobs", array_value(entries))));
    }
    for (auto bad : {string_value("row"), array_value(), render_number(7)}) {
      auto rows   = semantic_jobs;
      rows[index] = bad;
      refuse(changed(model, "jobs", array_value(rows)));
      auto entries   = bundle_jobs;
      entries[index] = bad;
      refuse(changed(model, "bundle", changed(bundle, "jobs", array_value(entries))));
      entries        = bundle_jobs;
      entries[index] = changed(entries[index], "value", bad);
      refuse(changed(model, "bundle", changed(bundle, "jobs", array_value(entries))));
    }
    auto entries   = bundle_jobs;
    entries[index] = changed(entries[index], "value", {});
    refuse(changed(model, "bundle", changed(bundle, "jobs", array_value(entries))));
  }
  for (bool semantic : {false, true}) {
    auto install = [&](std::vector<value_ptr> rows) {
      return semantic ? changed(model, "jobs", array_value(rows))
                      : changed(model, "bundle", changed(bundle, "jobs", array_value(rows)));
    };
    auto rows = semantic ? semantic_jobs : bundle_jobs;
    std::ranges::reverse(rows);
    refuse(install(rows));
    rows = semantic ? semantic_jobs : bundle_jobs;
    rows.pop_back();
    refuse(install(rows));
    refuse(install({}));
    rows = semantic ? semantic_jobs : bundle_jobs;
    rows.push_back(rows.front());
    refuse(install(rows));
    rows    = semantic ? semantic_jobs : bundle_jobs;
    rows[0] = rows[1];
    refuse(install(rows));
  }
  // Even matching duplicates on both sides cannot identify one selected job.
  refuse(changed(changed(model, "jobs", array_value({semantic_jobs[1], semantic_jobs[1]})), "bundle",
                 changed(bundle, "jobs", array_value({bundle_jobs[1], bundle_jobs[1]}))));
  for (auto bad : {value_ptr{}, record_value(), string_value("container"), render_number(7)}) {
    refuse(changed(model, "jobs", bad));
    refuse(changed(model, "bundle", changed(bundle, "jobs", bad)));
    refuse(changed(model, "bundle", bad));
    refuse(bad);
  }
  CHECK(*codec_encode(success("render_profile_material", input)) == good);
}

namespace {
value_ptr native_render(command_vm& vm, value_ptr request) {
  auto result = vm.render_profile(request);
  INFO((result ? "native render success" : result.error().detail.message));
  REQUIRE(result);
  return *result;
}
} // namespace
TEST_CASE("X05 B4 B actual captured profile renders authentic same-call immutable material repeatedly",
          "[external-registration][profile-render][native-profile-render]") {
  semantic_fixture f(true);
  auto             source = selection_shape();
  std::ofstream(f.root / "profile.json") << "{\"version\":1,\"modules\":[\"m1.lua\"]}";
  std::ofstream(f.root / "m1.lua") << source;
  auto capture = profile_capture::open((f.root / "profile.json").string());
  REQUIRE(capture);
  auto snapshot = capture->snapshot();
  REQUIRE(snapshot);
  auto vm = command_vm::create(*snapshot);
  REQUIRE(vm);
  std::ofstream(f.root / "m1.lua") << "error('changed after capture')";
  auto        validation = semantic_validation(f.bundle);
  value_ptr   retained;
  std::string saved;
  for (auto job : {"split", "combined"})
    for (auto role : (std::string_view(job) == "split" ? std::vector<std::string>{"library_worker", "cli_worker", "reviewer"}
                                                       : std::vector<std::string>{"builder", "reviewer"})) {
      auto request  = selection_request(validation, job, role);
      auto before   = semantic_snapshot(request);
      auto expected = invoke("render_profile_material", render_material_input(validation, request, source));
      auto result   = native_render(*vm, request);
      CHECK(*codec_encode(result) == *codec_encode(expected));
      CHECK(semantic_snapshot(request) == before);
      CHECK(*codec_encode(native_render(*vm, request)) == *codec_encode(result));
      REQUIRE(result->fields.contains("value"));
      CHECK(result->fields.at("value")->fields.at("kind")->string == "render_external");
      if (!retained) {
        retained = result;
        saved    = *codec_encode(result);
      }
      CHECK(*codec_encode(retained) == saved);
    }
  auto moved = std::move(*vm);
  CHECK_FALSE(vm->render_profile(selection_request(validation)));
  REQUIRE(native_render(moved, selection_request(validation))->fields.contains("value"));
  CHECK(*codec_encode(retained) == saved);
}
TEST_CASE("X05 B4 B native shipped and mixed render outputs preserve ordinary exact bytes",
          "[external-registration][profile-render][native-profile-render]") {
  semantic_fixture f;
  auto             validation = semantic_validation(f.bundle, false);
  auto             vm         = command_vm::create(profile({selection_shape()}));
  REQUIRE(vm);
  for (auto pair : {std::pair{"split", "library_worker"},
                    {"split", "cli_worker"},
                    {"split", "reviewer"},
                    {"combined", "builder"},
                    {"combined", "reviewer"}}) {
    auto request = selection_request(validation, pair.first, pair.second);
    CHECK(*codec_encode(native_render(*vm, request)) == *codec_encode(invoke("render", request)));
  }
  auto bundle = validation->fields.at("bundle");
  auto jobs   = bundle->fields.at("jobs")->items;
  jobs[0]     = changed(jobs[0], "value", changed(jobs[0]->fields.at("value"), "shape", string_value("custom")));
  auto mixed  = changed(validation, "bundle", changed(bundle, "jobs", array_value(jobs)));
  for (auto pair : {std::pair{"split", "library_worker"}, {"combined", "builder"}}) {
    auto request  = selection_request(mixed, pair.first, pair.second);
    auto expected = invoke("render_profile_material", render_material_input(mixed, request, selection_shape()));
    CHECK(*codec_encode(native_render(*vm, request)) == *codec_encode(expected));
  }
}
TEST_CASE("X05 B4 B render preflight preserves both schema priorities before any shape execution",
          "[external-registration][profile-render][native-profile-render]") {
  semantic_fixture f(true);
  auto             bad_validation = changed(semantic_validation(f.bundle), "bundle", record_value());
  auto             request        = selection_request(semantic_validation(f.bundle));
  request                         = changed(request, "bundle", bad_validation->fields.at("bundle"));
  auto vm                         = command_vm::create(profile({callable_shape("error('must not execute')")}));
  REQUIRE(vm);
  for (auto field : {"budget_bytes", "job", "role"}) {
    auto invalid  = changed(request, field, render_number(0));
    auto expected = invoke("render", invalid);
    REQUIRE(expected->fields.contains("error"));
    CHECK(*codec_encode(native_render(*vm, invalid)) == *codec_encode(expected));
  }
  for (auto files : {record_value(), array_value({record_value()})}) {
    auto invalid  = changed(request, "files", files);
    auto expected = invoke("render", invalid);
    REQUIRE(expected->fields.contains("error"));
    CHECK(*codec_encode(native_render(*vm, invalid)) == *codec_encode(expected));
  }
  for (auto key : {"job", "role"}) {
    auto invalid  = changed(request, key, string_value("missing"));
    auto expected = invoke("render", invalid);
    REQUIRE(expected->fields.contains("error"));
    CHECK(*codec_encode(native_render(*vm, invalid)) == *codec_encode(expected));
  }
  // No native bundle/capture presence guard may mask full render schema errors.
  for (auto key : {"bundle", "capture"}) {
    auto invalid = changed(changed(request, key, {}), "budget_bytes", render_number(0));
    CHECK(*codec_encode(native_render(*vm, invalid)) == *codec_encode(invoke("render", invalid)));
  }
}
TEST_CASE("X05 B4 B native rendering keeps grouped empty and singleton obligations and immutable shape input",
          "[external-registration][profile-render][native-profile-render]") {
  semantic_fixture f(true);
  for (bool single : {false, true}) {
    auto source = callable_shape(
        std::string("assert(not pcall(function() input.job.id='changed' end)); return "
                    "engine.object{ok=true,value=engine.object{guidance='',obligations=engine.array{") +
        (single ? "engine.object{id='one',producer='library',consumer='command',phase='start',quality='done'}" : "") + "}}}");
    auto vm = command_vm::create(profile({source}));
    REQUIRE(vm);
    auto validation = semantic_validation(f.bundle);
    for (auto pair : {std::pair{"split", "library_worker"}, {"combined", "builder"}}) {
      auto request  = selection_request(validation, pair.first, pair.second);
      auto result   = native_render(*vm, request);
      auto expected = invoke("render_profile_material", render_material_input(validation, request, source));
      CHECK(*codec_encode(result) == *codec_encode(expected));
      REQUIRE(result->fields.contains("value"));
      CHECK(result->fields.at("value")->fields.at("manifest")->fields.at("prerequisites")->items.size() == (single ? 1 : 0));
    }
  }
}
TEST_CASE("X05 B4 B native observation and budget refusals retain pure render ordering and recoverability",
          "[external-registration][profile-render][native-profile-render]") {
  semantic_fixture f(true);
  auto             validation = semantic_validation(f.bundle);
  auto             request    = selection_request(validation);
  auto             vm         = command_vm::create(profile({selection_shape()}));
  REQUIRE(vm);
  std::vector<value_ptr> variants{changed(request, "budget_bytes", render_number(1))};
  auto                   files = request->fields.at("files")->items;
  files.push_back(record_value({{"root", string_value("observations")},
                                {"path", string_value("input.json")},
                                {"mode", string_value("0644")},
                                {"bytes", string_value("{")}}));
  variants.push_back(changed(changed(request, "files", array_value(files)), "budget_bytes", render_number(1)));
  for (auto invalid : variants) {
    auto material = render_material_input(validation, changed(invalid, "budget_bytes", {}), selection_shape());
    auto expected = invoke("render_profile_material", changed(material, "request", invalid));
    REQUIRE(expected->fields.contains("error"));
    CHECK(*codec_encode(native_render(*vm, invalid)) == *codec_encode(expected));
  }
  REQUIRE(native_render(*vm, request)->fields.contains("value"));
}
TEST_CASE("X05 B4 B native shape failure remains terminal across render selection validation moves and retries",
          "[external-registration][profile-render][native-profile-render]") {
  semantic_fixture f;
  auto             request = selection_request(semantic_validation(f.bundle));
  auto             vm      = command_vm::create(profile({callable_shape("error('private')")}));
  REQUIRE(vm);
  auto failed = vm->render_profile(request);
  REQUIRE_FALSE(failed);
  auto moved = std::move(*vm);
  for (auto retry : {moved.render_profile({}), moved.select_profile(request), moved.validate_profile({})}) {
    REQUIRE_FALSE(retry);
    CHECK(retry.error().field == failed.error().field);
    CHECK(retry.error().detail.message == failed.error().detail.message);
  }
  auto embedded = command_vm::create();
  REQUIRE(embedded);
  CHECK_FALSE(embedded->render_profile(request));
}
TEST_CASE("X05 B4 B native custom worker reviewer recovery retains assertions and policy exactly",
          "[external-registration][profile-render][native-profile-render]") {
  semantic_fixture f(true);
  auto             validation = semantic_validation(f.bundle);
  auto             vm         = command_vm::create(profile({selection_shape()}));
  REQUIRE(vm);
  for (auto role : {"library_worker", "reviewer"}) {
    auto request = selection_request(validation, "split", role);
    auto files   = request->fields.at("files")->items;
    files.push_back(record_value(
        {{"root", string_value("observations")},
         {"path", string_value("input.json")},
         {"mode", string_value("0644")},
         {"bytes",
          string_value(
              R"({"version":1,"checks":[],"findings":[],"checkpoint":{"progress":"draft","next":"review","unresolved":"none","files":[]}})")}}));
    request       = changed(request, "files", array_value(files));
    auto expected = invoke("render_profile_material", render_material_input(validation, request, selection_shape()));
    auto actual   = native_render(*vm, request);
    CHECK(*codec_encode(actual) == *codec_encode(expected));
    REQUIRE(actual->fields.contains("value"));
    CHECK(actual->fields.at("value")->fields.at("manifest")->fields.at("assertions")->fields.contains("checkpoint"));
  }
}
TEST_CASE("X05 B4 B C01 complete native render bound covers original oversized schema refusal",
          "[external-registration][profile-render][native-profile-render][render-complete-bound]") {
  semantic_fixture f;
  auto             request = selection_request(semantic_validation(f.bundle));
  auto             invalid = changed(request, std::string(16 * 1024 * 1024 + 1, 'a'), record_value());
  auto             vm      = command_vm::create(profile({callable_shape("error('must not execute')")}));
  REQUIRE(vm);
  auto failed = vm->render_profile(invalid);
  REQUIRE_FALSE(failed);
  CHECK(failed.error().detail.code == "resource_limit");
  CHECK(failed.error().detail.limit == "output_bytes");
  CHECK(failed.error().detail.maximum == 16777216);
  auto moved = std::move(*vm);
  for (auto retry : {moved.render_profile({}), moved.select_profile(request), moved.validate_profile({})}) {
    REQUIRE_FALSE(retry);
    CHECK(retry.error().field == failed.error().field);
    CHECK(retry.error().detail.code == failed.error().detail.code);
    CHECK(retry.error().detail.message == failed.error().detail.message);
    CHECK(retry.error().detail.limit == failed.error().detail.limit);
    CHECK(retry.error().detail.maximum == failed.error().detail.maximum);
  }
}
namespace {
struct render_boundary_case {
  std::string                name, source;
  value_ptr                  request;
  std::array<std::size_t, 7> phases;
};
std::vector<render_boundary_case> render_boundary_cases(value_ptr original) {
  auto validation = semantic_validation(original);
  auto request    = selection_request(validation);
  auto bundle     = validation->fields.at("bundle");
  auto jobs       = bundle->fields.at("jobs")->items;
  jobs[1]         = changed(jobs[1], "value", changed(jobs[1]->fields.at("value"), "unknown", string_value("invalid")));
  auto bad_base   = changed(request, "bundle", changed(bundle, "jobs", array_value(jobs)));
  auto cycle = callable_shape("return engine.object{ok=true,value=engine.object{guidance='edge',obligations=engine.array{"
                              "engine.object{id='edge',producer='command',consumer='library',phase='start',quality='done'}}}}");
  return {{"normalization", semantic_shape(), changed(request, "unknown", string_value("original")), {0, 0, 0, 0, 1, 0, 0}},
          {"preparation", semantic_shape(), changed(request, "bundle", record_value()), {1, 0, 0, 0, 1, 0, 0}},
          {"base-policy", semantic_shape(), bad_base, {1, 0, 0, 0, 1, 0, 0}},
          {"shape-check",
           callable_shape(
               "return engine.object{ok=false,error=engine.object{code='custom_refusal',message='original',field='job.shape'}}"),
           request,
           {1, 1, 0, 0, 1, 0, 1}},
          {"finalization", cycle, request, {1, 2, 1, 0, 1, 0, 2}},
          {"selection", semantic_shape(), changed(request, "job", string_value("missing")), {1, 2, 1, 1, 1, 0, 2}},
          {"render", semantic_shape(), changed(request, "budget_bytes", render_number(1)), {1, 2, 1, 1, 1, 1, 2}},
          {"success", semantic_shape(), request, {1, 2, 1, 1, 1, 1, 2}}};
}
} // namespace
TEST_CASE("X05 B4 B C01 ordinary responses retain exact errors recoverability and native lifetimes at every phase",
          "[external-registration][profile-render][native-profile-render][render-boundary-phases]") {
  semantic_fixture f;
  for (auto& scenario : render_boundary_cases(f.bundle)) {
    INFO(scenario.name);
    auto vm = command_vm::create(profile({scenario.source}));
    REQUIRE(vm);
    auto before = semantic_snapshot(scenario.request);
    auto first  = vm->render_profile(scenario.request);
    REQUIRE(first);
    CHECK((*first)->fields.contains(scenario.name == "success" ? "value" : "error"));
    if (scenario.name == "finalization")
      CHECK((*first)->fields.at("error")->fields.at("code")->string == "dependency_cycle");
    // Compare with the unchanged non-render path for the same ordinary error.
    value_ptr expected;
    if (scenario.name == "normalization") {
      expected = invoke("render", scenario.request);
    } else if (scenario.name != "render" && scenario.name != "success") {
      auto control = command_vm::create(profile({scenario.source}));
      REQUIRE(control);
      auto checked = scenario.name == "selection"
                         ? control->select_profile(scenario.request)
                         : control->validate_profile(record_value({{"bundle", scenario.request->fields.at("bundle")},
                                                                   {"capture", scenario.request->fields.at("capture")}}));
      REQUIRE(checked);
      expected = *checked;
    }
    if (expected)
      CHECK(*codec_encode(*first) == *codec_encode(expected));
    auto encoded = codec_encode(*first);
    REQUIRE(encoded);
    CHECK(semantic_snapshot(scenario.request) == before);
    std::optional<command_vm> moved{std::move(*vm)};
    auto                      repeated = moved->render_profile(scenario.request);
    REQUIRE(repeated);
    CHECK(*codec_encode(*repeated) == *encoded);
    if (scenario.name != "success") {
      // Ordinary errors remain data and do not poison later operations.
      if (scenario.name != "shape-check" && scenario.name != "finalization")
        CHECK(native_render(*moved, selection_request(semantic_validation(f.bundle)))->fields.contains("value"));
    }
    value_ptr retained = *first;
    moved.reset();
    CHECK(*codec_encode(retained) == *encoded);
  }
}

namespace {
// Algorithm fixtures only: native authenticates/prepares base and admits custom
// records in later C rounds. This private helper is not public F admission.
value_ptr assembly_section(std::string id, std::string text, bool mandatory) {
  auto flag     = std::make_shared<value>();
  flag->kind    = value_kind::boolean;
  flag->boolean = mandatory;
  return record_value({{"id", string_value(std::move(id))}, {"text", string_value(std::move(text))}, {"mandatory", flag}});
}
value_ptr assembly_input(std::vector<value_ptr> base, std::vector<value_ptr> custom, std::size_t budget) {
  return record_value({{"base", array_value(std::move(base))},
                       {"custom", array_value(std::move(custom))},
                       {"budget_bytes", render_number(budget)}});
}
value_ptr assembled(value_ptr input) {
  return success("assemble_projection_material", input);
}
void assembly_accounting(value_ptr result) {
  std::int64_t bytes = 0;
  for (auto entry : result->fields.at("sections")->items) {
    CHECK(entry->fields.size() == 3);
    if (entry->fields.at("included")->boolean)
      bytes += entry->fields.at("bytes")->integer;
  }
  CHECK(bytes == static_cast<std::int64_t>(result->fields.at("text")->string.size()));
}
} // namespace
TEST_CASE("X05 C1 exact projection omission vector preserves removal order and byte counts", "[projection-assembly]") {
  auto input = assembly_input(
      {assembly_section("base", "## base\n\nB\n", true)},
      {assembly_section("a", std::string(30, 'A') + "\n", false), assembly_section("b", std::string(30, 'B') + "\n", false)}, 60);
  auto before = semantic_snapshot(input);
  auto full   = assembled(changed(input, "budget_bytes", render_number(87)));
  CHECK(full->fields.at("text")->string.size() == 87);
  // Retaining a as mandatory isolates the first removal: notice growth makes
  // the intermediate b-omitted assembly 94 bytes, above the 86-byte budget.
  auto intermediate = assembly_input(
      {assembly_section("base", "## base\n\nB\n", true)},
      {assembly_section("a", std::string(30, 'A') + "\n", true), assembly_section("b", std::string(30, 'B') + "\n", false)}, 86);
  auto first_error = invoke("assemble_projection_material", intermediate)->fields.at("error");
  CHECK(first_error->fields.at("details")->fields.at("mandatory_bytes")->integer == 94);
  auto result = assembled(input);
  CHECK(result->fields.at("text")->string == "## base\n\nB\n\n## omissions\n\nOptional sections omitted: b, a.\n");
  auto omitted = result->fields.at("omitted")->items;
  REQUIRE(omitted.size() == 2);
  CHECK(omitted[0]->string == "b");
  CHECK(omitted[1]->string == "a");
  auto metadata = result->fields.at("sections")->items;
  REQUIRE(metadata.size() == 4);
  for (std::size_t i = 0; i < metadata.size(); ++i)
    CHECK(metadata[i]->fields.at("bytes")->integer == std::array{11, 37, 37, 48}[i]);
  CHECK(result->fields.at("text")->string.size() == 59);
  assembly_accounting(result);
  CHECK(semantic_snapshot(assembled(input)) == semantic_snapshot(result));
  CHECK(semantic_snapshot(input) == before);
  CHECK(semantic_snapshot(assembled(changed(input, "budget_bytes", render_number(59)))) == semantic_snapshot(result));
  auto error = invoke("assemble_projection_material", changed(input, "budget_bytes", render_number(58)))->fields.at("error");
  CHECK(error->fields.at("code")->string == "budget_exceeded");
  CHECK(error->fields.at("details")->fields.at("mandatory_bytes")->integer == 59);
}
TEST_CASE("X05 C1 custom optional priority precedes every shipped optional while mandatory bytes survive",
          "[projection-assembly]") {
  std::vector<value_ptr> base{assembly_section("base", "## base\n\nB\n", true)};
  for (auto id : {"rationale", "workspace_notes", "job_notes", "checkpoint"})
    base.push_back(assembly_section(id, "## " + std::string(id) + "\n\n" + std::string(150, 'N') + "\n", false));
  std::vector<value_ptr> custom{assembly_section("a", std::string(150, 'A') + "\n", false), assembly_section("keep", "M\n", true),
                                assembly_section("b", std::string(150, 'B') + "\n", false)};
  auto                   input    = assembly_input(base, custom, 160);
  auto                   before   = semantic_snapshot(input);
  auto                   result   = assembled(input);
  auto                   expected = std::array{"b", "a", "checkpoint", "job_notes", "workspace_notes", "rationale"};
  auto                   omitted  = result->fields.at("omitted")->items;
  REQUIRE(omitted.size() == expected.size());
  for (std::size_t i = 0; i < expected.size(); ++i)
    CHECK(omitted[i]->string == expected[i]);
  CHECK(result->fields.at("text")->string.starts_with("## base\n\nB\n\n## keep\n\nM\n"));
  auto metadata = result->fields.at("sections")->items;
  REQUIRE(metadata.size() == 9);
  CHECK(metadata[5]->fields.at("id")->string == "a");
  CHECK(metadata[6]->fields.at("included")->boolean);
  CHECK(metadata[7]->fields.at("id")->string == "b");
  assembly_accounting(result);
  CHECK(semantic_snapshot(input) == before);
  // At intermediate budgets stop after the exact first removable candidate.
  auto full    = assembled(changed(input, "budget_bytes", render_number(4096)));
  auto initial = full->fields.at("text")->string.size();
  auto first   = assembled(changed(input, "budget_bytes", render_number(initial - 1)));
  REQUIRE(first->fields.at("omitted")->items.size() == 1);
  CHECK(first->fields.at("omitted")->items[0]->string == "b");
}
TEST_CASE("X05 C1 notice growth can overflow after all optional text is removed", "[projection-assembly]") {
  auto input  = assembly_input({assembly_section("base", "## base\n\nB\n", true)}, {assembly_section("a", "", false)}, 17);
  auto result = invoke("assemble_projection_material", input);
  auto error  = result->fields.at("error");
  CHECK(error->fields.at("field")->string == "render.budget_bytes");
  CHECK(error->fields.at("message")->string == "Mandatory Markdown exceeds budget");
  auto details = error->fields.at("details");
  CHECK(details->fields.at("budget_bytes")->integer == 17);
  CHECK(details->fields.at("mandatory_bytes")->integer == 56);
  REQUIRE(details->fields.at("sections")->items.size() == 2);
  CHECK(details->fields.at("sections")->items.back()->fields.at("id")->string == "omissions");
}
TEST_CASE("X05 C1 custom mandatory sections refuse without truncating or omitting", "[projection-assembly]") {
  auto input  = assembly_input({assembly_section("base", "## base\n\nB\n", true)}, {assembly_section("keep", "ALL\n", true)}, 25);
  auto result = assembled(input);
  CHECK(result->fields.at("text")->string == "## base\n\nB\n\n## keep\n\nALL\n");
  CHECK(result->fields.at("omitted")->items.empty());
  assembly_accounting(result);
  auto error = invoke("assemble_projection_material", changed(input, "budget_bytes", render_number(24)))->fields.at("error");
  CHECK(error->fields.at("code")->string == "budget_exceeded");
  REQUIRE(error->fields.at("details")->fields.at("sections")->items.size() == 2);
}
TEST_CASE("X05 C1 assembly counts UTF8 bytes and preserves supplied trailing LF bytes", "[projection-assembly]") {
  for (auto body : {std::string{}, std::string("é"), std::string("é\n"), std::string("é\n\n")}) {
    auto expected = "## view\n\n" + body;
    auto input    = assembly_input({}, {assembly_section("view", body, true)}, expected.size());
    auto result   = assembled(input);
    CHECK(result->fields.at("text")->string == expected);
    CHECK(result->fields.at("sections")->items[0]->fields.at("bytes")->integer == static_cast<std::int64_t>(expected.size()));
    assembly_accounting(result);
    auto error = invoke("assemble_projection_material", changed(input, "budget_bytes", render_number(expected.size() - 1)));
    CHECK(error->fields.at("error")->fields.at("code")->string == "budget_exceeded");
  }
}
TEST_CASE("X05 C1 empty and singleton assemblies retain exact metadata shape", "[projection-assembly]") {
  auto empty = assembled(assembly_input({}, {}, 0));
  CHECK(empty->fields.at("text")->string.empty());
  CHECK(empty->fields.at("sections")->kind == value_kind::list);
  CHECK(empty->fields.at("sections")->items.empty());
  CHECK(empty->fields.at("omitted")->kind == value_kind::list);
  auto input  = assembly_input({assembly_section("base", "verbatim\n\n", true)}, {}, 10);
  auto before = semantic_snapshot(input);
  auto result = assembled(input);
  CHECK(result->fields.at("text")->string == "verbatim\n\n");
  REQUIRE(result->fields.at("sections")->items.size() == 1);
  CHECK(result->fields.at("sections")->items[0]->fields.at("bytes")->integer == 10);
  assembly_accounting(result);
  CHECK(semantic_snapshot(input) == before);
}

namespace {
std::string projection_source(std::string tail = {}) {
  return "local D={id='custom',abi='projection_v1',revision='initial',input='role_context_v1',help='projection'}; "
         "registration.projection(D,function() error('projection F must not execute') end); D.revision='final'" +
         tail;
}
std::string projection_digest(profile_snapshot p) {
  auto vm = command_vm::create(std::move(p));
  REQUIRE(vm);
  auto definition = vm->resolve_projection_definition("custom");
  REQUIRE(definition);
  return definition->digest;
}
} // namespace
TEST_CASE("X05 C2 A final projection definition matches independent canonical hash construction",
          "[external-registration][projection-definition]") {
  auto source = projection_source();
  auto vm     = command_vm::create(profile({source}));
  REQUIRE(vm);
  auto result = vm->resolve_projection_definition("custom");
  REQUIRE(result);
  CHECK(result->id == "custom");
  CHECK(result->revision == "final");
  // Build the specified object independently of runtime owner/catalog assembly.
  std::string hex;
  for (unsigned char b : source)
    hex += std::format("{:02x}", b);
  auto d        = shape_record({{"id", shape_text("custom")},
                                {"abi", shape_text("projection_v1")},
                                {"revision", shape_text("final")},
                                {"input", shape_text("role_context_v1")},
                                {"help", shape_text("projection")}});
  auto expected = hash_value(
      "projection_definition_v1",
      shape_record({{"descriptor", d},
                    {"owner", shape_record({{"tier", shape_text("external")}, {"module", shape_text("external/m1.lua")}})},
                    {"module_bytes", shape_text(hex)}}));
  REQUIRE(expected);
  CHECK(result->digest == *expected);
  CHECK(result->digest == "xxh3_128:11f11b5d6a3fd4c65e8b7cb163a73f1f");
  auto again = vm->resolve_projection_definition("custom");
  REQUIRE(again);
  CHECK(again->digest == result->digest);
  result->id       = "caller changed";
  result->revision = "caller changed";
  result->digest   = "caller changed";
  auto third       = vm->resolve_projection_definition("custom");
  REQUIRE(third);
  CHECK(third->id == "custom");
  CHECK(third->revision == "final");
  CHECK(third->digest == *expected);
}
TEST_CASE("X05 C2 A selected source owner and final descriptor determine projection identity",
          "[external-registration][projection-definition]") {
  auto source   = projection_source();
  auto original = projection_digest(named_profile({{"shape.lua", source}}));
  CHECK(projection_digest(named_profile({{"shape.lua", source + "\n-- exact selected comment"}})) != original);
  CHECK(projection_digest(named_profile({{"renamed.lua", source}})) != original);
  CHECK(projection_digest(named_profile({{"shape.lua", source + "; D.help='changed final descriptor'"}})) != original);
  std::string unrelated =
      "registration.shape({id='other',abi='shape_v1',revision='v1',stages='optional',help=''},function() end)";
  CHECK(projection_digest(named_profile({{"shape.lua", source}, {"view.lua", unrelated}})) == original);
  CHECK(projection_digest(named_profile({{"view.lua", unrelated + " -- unrelated edit"}, {"shape.lua", source}})) == original);
  CHECK(projection_digest(named_profile({{"shape.lua", source + "; " + unrelated}})) != original);
  auto whitespace = std::make_shared<profile_closure>(*named_profile({{"shape.lua", source}}));
  whitespace->profile.bytes += " \n";
  whitespace->profile.mode    = "0755";
  whitespace->modules[0].mode = "0755";
  CHECK(projection_digest(whitespace) == original);
}
TEST_CASE("X05 C2 A actual host profile selectors do not enter selected projection identity",
          "[external-registration][projection-definition]") {
  char pattern[] = "/tmp/planar-c2-a-XXXXXX";
  auto name      = mkdtemp(pattern);
  REQUIRE(name);
  struct cleanup {
    std::filesystem::path root;
    ~cleanup() {
      std::filesystem::remove_all(root);
    }
  } temporary{std::filesystem::canonical(name)};
  for (auto selector : {"first profile.json", "second profile.json"})
    std::ofstream(temporary.root / selector) << "{\"version\":1,\"modules\":[\"m1.lua\"]}";
  std::ofstream(temporary.root / "m1.lua") << projection_source();
  auto first  = profile_capture::open((temporary.root / "first profile.json").string());
  auto second = profile_capture::open((temporary.root / "second profile.json").string());
  REQUIRE(first);
  REQUIRE(second);
  auto a = first->snapshot(), b = second->snapshot();
  REQUIRE(a);
  REQUIRE(b);
  auto original = projection_digest(*a);
  CHECK(projection_digest(*b) == original);
  std::ofstream(temporary.root / "m1.lua") << "error('private replacement')";
  CHECK_FALSE(first->verify());
  CHECK(projection_digest(*a) == original);
}
TEST_CASE("X05 C2 A definition lookup severs caller capture aliases and never invokes projection Lua",
          "[external-registration][projection-definition]") {
  auto captured = std::make_shared<profile_closure>(*profile({projection_source()}));
  auto vm       = command_vm::create(captured);
  REQUIRE(vm);
  auto before = vm->resolve_projection_definition("custom");
  REQUIRE(before);
  captured->modules[0].bytes = "error('private caller mutation')";
  captured->modules[0].path  = "different.lua";
  captured->profile.bytes.clear();
  for (int i = 0; i < 20; ++i) {
    auto after = vm->resolve_projection_definition("custom");
    REQUIRE(after);
    CHECK(after->digest == before->digest);
  }
}
TEST_CASE("X05 C2 A wrong namespace embedded and moved owners refuse without consuming lookup",
          "[external-registration][projection-definition]") {
  auto vm = command_vm::create(profile({projection_source(), shape_source("; D.id='other'"), command()}));
  REQUIRE(vm);
  for (auto id : {"unknown", "inspect", "pipeline", "fanout_review", "worker", "reviewer", "recovery"}) {
    auto bad = vm->resolve_projection_definition(id);
    REQUIRE_FALSE(bad);
    CHECK(bad.error().detail.code == "internal");
    CHECK(bad.error().detail.message == "Invalid external profile protocol");
    CHECK(bad.error().field == "catalog.projections");
  }
  CHECK_FALSE(vm->resolve_projection_definition("other"));
  REQUIRE(vm->resolve_projection_definition("custom"));
  auto moved = std::move(*vm);
  CHECK_FALSE(vm->resolve_projection_definition("custom"));
  REQUIRE(moved.resolve_projection_definition("custom"));
  auto embedded = command_vm::create();
  REQUIRE(embedded);
  CHECK_FALSE(embedded->resolve_projection_definition("pipeline"));
}

TEST_CASE("X05 C2 A every final projection descriptor field is material", "[external-registration][projection-definition]") {
  auto original = projection_digest(profile({projection_source()}));
  for (auto tail : {"; D.help='changed'", "; D.input='history_snapshot_v1'", "; D.revision='changed'"})
    CHECK(projection_digest(profile({projection_source(tail)})) != original);
  auto vm = command_vm::create(profile({projection_source("; D.id='changed'")}));
  REQUIRE(vm);
  auto changed = vm->resolve_projection_definition("changed");
  REQUIRE(changed);
  CHECK(changed->id == "changed");
  CHECK(changed->digest != original);
}
TEST_CASE("X05 C2 A cumulative definition exhaustion preserves first error across APIs and moves",
          "[external-registration][projection-definition]") {
  auto vm = command_vm::create(profile({projection_source("\n--" + std::string(524288, 'x')), shape_source()}));
  REQUIRE(vm);
  std::optional<registration_error> first;
  std::size_t                       successes = 0;
  for (; successes < 200; ++successes) {
    auto result = vm->resolve_projection_definition("custom");
    if (!result) {
      first = std::move(result.error());
      break;
    }
  }
  REQUIRE(first);
  CHECK(successes > 0);
  CHECK(first->detail.code == "resource_limit");
  CHECK(first->detail.limit == "native_bytes");
  CHECK(first->field == "catalog.projections");
  auto same = [&](const registration_error& error) {
    CHECK(error.detail.code == first->detail.code);
    CHECK(error.detail.message == first->detail.message);
    CHECK(error.detail.limit == first->detail.limit);
    CHECK(error.detail.maximum == first->detail.maximum);
    CHECK(error.field == first->field);
  };
  for (int pass = 0; pass < 2; ++pass) {
    auto projection = vm->resolve_projection_definition("unknown");
    REQUIRE_FALSE(projection);
    same(projection.error());
    auto shape = vm->resolve_shape_definition("custom");
    REQUIRE_FALSE(shape);
    same(shape.error());
    auto call = vm->invoke_shape("custom", {});
    REQUIRE_FALSE(call);
    same(call.error());
    auto validation = vm->validate_profile({});
    REQUIRE_FALSE(validation);
    same(validation.error());
    auto selection = vm->select_profile({});
    REQUIRE_FALSE(selection);
    same(selection.error());
    auto render = vm->render_profile({});
    REQUIRE_FALSE(render);
    same(render.error());
    auto moved = std::move(*vm);
    *vm        = std::move(moved);
  }
}

TEST_CASE("X05 C2 A owned projection identity survives native owner destruction",
          "[external-registration][projection-definition]") {
  auto make = [] {
    auto vm = command_vm::create(profile({projection_source()}));
    REQUIRE(vm);
    auto identity = vm->resolve_projection_definition("custom");
    REQUIRE(identity);
    return *identity;
  };
  auto identity = make();
  CHECK(identity.id == "custom");
  CHECK(identity.revision == "final");
  CHECK(identity.digest == "xxh3_128:11f11b5d6a3fd4c65e8b7cb163a73f1f");
}
TEST_CASE("X05 C2 A earlier shape resource refusal remains exact on projection lookup",
          "[external-registration][projection-definition]") {
  auto vm = command_vm::create(profile({shape_source("\n--" + std::string(524288, 'x')), projection_source()}));
  REQUIRE(vm);
  std::optional<registration_error> first;
  for (int i = 0; i < 200; ++i) {
    auto result = vm->resolve_shape_definition("custom");
    if (!result) {
      first = std::move(result.error());
      break;
    }
  }
  REQUIRE(first);
  CHECK(first->field == "catalog.shapes");
  auto result = vm->resolve_projection_definition("custom");
  REQUIRE_FALSE(result);
  CHECK(result.error().detail.code == first->detail.code);
  CHECK(result.error().detail.message == first->detail.message);
  CHECK(result.error().detail.limit == first->detail.limit);
  CHECK(result.error().detail.maximum == first->detail.maximum);
  CHECK(result.error().field == first->field);
}

namespace {
std::string projection_callable(std::string body, std::string kind = "role_context_v1", std::string id = "custom") {
  return "registration.projection({id='" + id + "',abi='projection_v1',revision='final',input='" + kind +
         "',help='projection'},function(input) " + body + " end)";
}
value_ptr projection_input() {
  return shape_record({{"payload", shape_text("authenticated caller fixture")}});
}
std::string projection_section(std::string fields = "id='custom_section',text='hello',mandatory=false") {
  return "return {version=1,sections=engine.array{{" + fields + "}}}";
}
} // namespace
TEST_CASE("X05 C2 B2 retained projection admits both kinds and owns results across moves and destruction",
          "[external-registration][projection-admission]") {
  for (auto kind : {projection_input_kind::role_context_v1, projection_input_kind::history_snapshot_v1}) {
    const auto name = kind == projection_input_kind::role_context_v1 ? "role_context_v1" : "history_snapshot_v1";
    auto       vm   = command_vm::create(profile({projection_callable(projection_section(), name)}));
    REQUIRE(vm);
    auto definition = vm->resolve_projection_definition("custom");
    REQUIRE(definition);
    auto moved    = std::move(*vm);
    auto response = moved.invoke_projection("custom", kind, projection_input());
    REQUIRE(response);
    CHECK(response->definition.digest == definition->digest);
    auto encoded = codec_encode(response->result);
    REQUIRE(encoded);
    CHECK(*encoded == "{\"sections\":[{\"id\":\"custom_section\",\"mandatory\":false,\"text\":\"hello\"}],\"version\":1}");
    auto again = moved.invoke_projection("custom", kind, projection_input());
    REQUIRE(again);
    CHECK(again->result.get() != response->result.get());
    response->result.reset();
    REQUIRE(codec_encode(again->result));
    CHECK_FALSE(vm->invoke_projection("custom", kind, projection_input()));
  }
  value_ptr retained;
  {
    auto vm = command_vm::create(profile({projection_callable("return {version=1,sections=engine.array{}} ")}));
    REQUIRE(vm);
    auto response = vm->invoke_projection("custom", projection_input_kind::role_context_v1, projection_input());
    REQUIRE(response);
    retained = response->result;
  }
  REQUIRE(codec_encode(retained));
}
TEST_CASE("X05 C2 B2 preadmission guards do not execute F or consume healthy owner",
          "[external-registration][projection-admission]") {
  auto vm = command_vm::create(profile({projection_callable("error('called F')")}));
  REQUIRE(vm);
  for (auto id : {"absent", "custom"}) {
    auto failure = vm->invoke_projection(id, projection_input_kind::history_snapshot_v1, projection_input());
    REQUIRE_FALSE(failure);
    CHECK(failure.error().detail.code == "internal");
  }
  auto malformed = shape_record({{"child", {}}});
  auto invalid   = vm->invoke_projection("custom", projection_input_kind::role_context_v1, malformed);
  REQUIRE_FALSE(invalid);
  REQUIRE(vm->resolve_projection_definition("custom"));
  auto wrong_kind = vm->invoke_projection("custom", static_cast<projection_input_kind>(99), projection_input());
  REQUIRE_FALSE(wrong_kind);
  auto null = vm->invoke_projection("custom", projection_input_kind::role_context_v1, {});
  REQUIRE_FALSE(null);
  REQUIRE(vm->resolve_projection_definition("custom"));
  auto executed = vm->invoke_projection("custom", projection_input_kind::role_context_v1, projection_input());
  REQUIRE_FALSE(executed);
  CHECK(executed.error().field == "catalog.projections[1]");
  auto retry = vm->resolve_projection_definition("custom");
  REQUIRE_FALSE(retry);
  CHECK(retry.error().field == executed.error().field);
}
TEST_CASE("X05 C2 B2 global reserved section union applies to both input kinds",
          "[external-registration][projection-admission]") {
  constexpr std::array ids{"authority",    "goal",         "acceptance",      "requirements",    "contracts",
                           "boundary",     "dependencies", "shape_policy",    "checks",          "outputs",
                           "observations", "preferences",  "rationale",       "workspace_notes", "job_notes",
                           "checkpoint",   "omissions",    "recorded_history"};
  for (auto kind : {projection_input_kind::role_context_v1, projection_input_kind::history_snapshot_v1}) {
    auto name = kind == projection_input_kind::role_context_v1 ? "role_context_v1" : "history_snapshot_v1";
    for (auto id : ids) {
      INFO(id);
      auto vm = command_vm::create(
          profile({projection_callable(projection_section(std::string("id='") + id + "',text='x',mandatory=true"), name)}));
      REQUIRE(vm);
      auto result = vm->invoke_projection("custom", kind, projection_input());
      REQUIRE_FALSE(result);
      CHECK(result.error().detail.code == "internal");
      CHECK(result.error().field == "catalog.projections[1]");
    }
  }
}
TEST_CASE("X05 C2 B2 raw projection schema refuses malformed values before copy",
          "[external-registration][projection-admission]") {
  for (std::string body :
       {"return nil",
        "return true",
        "return {}",
        "return {version=2,sections=engine.array{}}",
        "return {version=1,sections=engine.array{},extra=true}",
        "return {version=1,sections={}}",
        "return engine.array{version=1,sections=engine.array{}}",
        "return {version=1,sections=engine.object{}}",
        "return {version=1,sections=engine.array{[2]={id='a',text='x',mandatory=false}}}",
        "return {version=1,sections=engine.array{a={id='a',text='x',mandatory=false}}}",
        "return {version=1,sections=engine.array{{id='a',text='x'}}}",
        "return {version=1,sections=engine.array{{id='a',text='x',mandatory=0}}}",
        "return {version=1,sections=engine.array{{id='a',text=1,mandatory=true}}}",
        "return {version=1,sections=engine.array{{id=1,text='x',mandatory=true}}}",
        "return {version=1,sections=engine.array{{id='a',text='x',mandatory=true,extra=1}}}",
        "return {version=1,sections=engine.array{engine.array{id='a',text='x',mandatory=true}}}",
        "return {version=1,sections=engine.array{{id='a',text='x',mandatory=false},{id='a',text='y',mandatory=true}}}",
        "local a={}; a.sections=engine.array{a}; a.version=1; return a",
        "return {version=1,sections=engine.array{{id='A',text='x',mandatory=true}}}",
        "return {version=1,sections=engine.array{{id='a',text=string.char(255),mandatory=true}}}",
        "return {version=1,sections=engine.array{{id='a',text=string.char(0),mandatory=true}}}"}) {
    INFO(body);
    auto vm = command_vm::create(profile({projection_callable(body)}));
    REQUIRE(vm);
    auto result = vm->invoke_projection("custom", projection_input_kind::role_context_v1, projection_input());
    REQUIRE_FALSE(result);
    CHECK(result.error().detail.code == "internal");
  }
}
TEST_CASE("X05 C2 B2 retained projection arity and Lua errors are terminal at original projection index",
          "[external-registration][projection-admission]") {
  for (auto body : {"return", "return {},{}", "error('secret source bytes')"}) {
    auto vm = command_vm::create(profile({projection_callable(projection_section(), "role_context_v1", "z_first") + "; " +
                                          projection_callable(body, "role_context_v1", "a_second")}));
    REQUIRE(vm);
    auto result = vm->invoke_projection("a_second", projection_input_kind::role_context_v1, projection_input());
    REQUIRE_FALSE(result);
    CHECK(result.error().field == "catalog.projections[2]");
    auto moved = std::move(*vm);
    auto retry = moved.invoke_projection("z_first", projection_input_kind::role_context_v1, projection_input());
    REQUIRE_FALSE(retry);
    CHECK(retry.error().field == result.error().field);
    auto shape = moved.invoke_shape("absent", {});
    REQUIRE_FALSE(shape);
    CHECK(shape.error().field == result.error().field);
    auto render = moved.render_profile({});
    REQUIRE_FALSE(render);
    CHECK(render.error().field == result.error().field);
  }
}
TEST_CASE("X05 C2 B2 immutable alias input and known proxy result retain transport identity",
          "[external-registration][projection-admission]") {
  auto p =
      profile({projection_callable("if input.left ~= input.right then error('alias lost') end; "
                                   "local ok=pcall(function() input.left.text='changed' end); if ok then error('mutable') end; "
                                   "return {version=1,sections=engine.array{input.left}}")});
  auto vm = command_vm::create(p);
  REQUIRE(vm);
  auto mandatory  = std::make_shared<value>();
  mandatory->kind = value_kind::boolean;
  auto section    = shape_record({{"id", shape_text("alias")}, {"text", shape_text("retained")}, {"mandatory", mandatory}});
  auto input      = shape_record({{"left", section}, {"right", section}});
  auto result     = vm->invoke_projection("custom", projection_input_kind::role_context_v1, input);
  REQUIRE(result);
  CHECK(section->fields.at("text")->string == "retained");
  auto again = vm->invoke_projection("custom", projection_input_kind::role_context_v1, input);
  REQUIRE(again);
  auto rooted_vm = command_vm::create(profile({projection_callable("return input")}));
  REQUIRE(rooted_vm);
  auto version     = std::make_shared<value>();
  version->kind    = value_kind::integer;
  version->integer = 1;
  auto sections    = std::make_shared<value>();
  sections->kind   = value_kind::list;
  sections->items.push_back(section);
  auto rooted = rooted_vm->invoke_projection("custom", projection_input_kind::role_context_v1,
                                             shape_record({{"version", version}, {"sections", sections}}));
  REQUIRE(rooted);
}
TEST_CASE("X05 C2 B2 section count and text bounds produce resource refusals", "[external-registration][projection-admission]") {
  for (auto body :
       {"local a=engine.array{}; for i=1,65 do a[i]={id='s'..i,text='',mandatory=false} end; return {version=1,sections=a}",
        "local t='a'; for i=1,20 do t=t..t end; return {version=1,sections=engine.array{{id='a',text=t..'a',mandatory=false}}}",
        "local a=engine.array{}; local t='a'; for j=1,20 do t=t..t end; for i=1,17 do a[i]={id='s'..i,text=t,mandatory=false} "
        "end; "
        "return {version=1,sections=a}",
        "local a=engine.array{}; local t=string.char(1); for j=1,20 do t=t..t end; for i=1,3 do "
        "a[i]={id='s'..i,text=t,mandatory=false} end; return {version=1,sections=a}"}) {
    INFO(body);
    auto vm = command_vm::create(profile({projection_callable(body)}));
    REQUIRE(vm);
    auto result = vm->invoke_projection("custom", projection_input_kind::role_context_v1, projection_input());
    REQUIRE_FALSE(result);
    CHECK(result.error().detail.code == "resource_limit");
    auto retry = vm->invoke_projection("custom", projection_input_kind::role_context_v1, projection_input());
    REQUIRE_FALSE(retry);
    CHECK(retry.error().field == result.error().field);
    CHECK(retry.error().detail.message == result.error().detail.message);
  }
}
TEST_CASE("X05 C2 B2 maximum section text and shared noncyclic sections remain valid",
          "[external-registration][projection-admission]") {
  auto vm = command_vm::create(
      profile({projection_callable("local t='a'; for i=1,20 do t=t..t end; local s={id='max_text',text=t,mandatory=true}; "
                                   "return {version=1,sections=engine.array{s}} ")}));
  REQUIRE(vm);
  auto response = vm->invoke_projection("custom", projection_input_kind::role_context_v1, projection_input());
  REQUIRE(response);
  CHECK(response->result->fields.at("sections")->items[0]->fields.at("text")->string.size() == 1048576);
  auto many = command_vm::create(profile({projection_callable(
      "local a=engine.array{}; for i=1,64 do a[i]={id='s'..i,text='',mandatory=false} end; return {version=1,sections=a}")}));
  REQUIRE(many);
  auto maximum = many->invoke_projection("custom", projection_input_kind::role_context_v1, projection_input());
  REQUIRE(maximum);
  CHECK(maximum->result->fields.at("sections")->items.size() == 64);
}

namespace {
std::string extension_protocol_source(std::string capability = {}) {
  std::string options;
  auto        add = [&](std::string_view name, std::string_view type, bool required = false) {
    if (!options.empty())
      options += ",";
    options += std::format("{{name='--{}',type='{}',required={},help='option'}}", name, type, required ? "true" : "false");
  };
  if (capability == "history_read_v1") {
    add("ledger", "text", true);
    add("revision", "history_id");
    add("at-event", "history_id");
    add("after-event", "history_id");
    add("request-id", "text");
    add("limit", "decimal");
  } else if (capability == "role_context_read_v1") {
    for (auto name : {"workspace", "registry", "root", "job", "role"})
      add(name, "text", true);
    add("base-root", "text");
    add("observations", "text");
    add("budget-bytes", "decimal");
    add("capture-outputs", "flag");
  } else {
    for (auto name : {"ledger", "workspace", "registry", "root", "base-root", "observations", "target"})
      add(name, "text");
    add("count", "decimal");
    add("flag", "flag");
  }
  return "registration.add({name='inspect',family='extension_read_v1',handler_id='custom.inspect',options=engine.array{" +
         options + "},help='inspect',response_kind='extension_v1',capabilities=engine.array{" +
         (capability.empty() ? "" : "'" + capability + "'") +
         "}},function() error('handler must not execute during native preparation') end)";
}
const extension_prepared_option& extension_option(const extension_prepared_invocation& result, std::string_view name) {
  auto found = std::ranges::find(result.options, name, &extension_prepared_option::name);
  REQUIRE(found != result.options.end());
  return *found;
}
void extension_usage(command_vm& vm, std::span<const extension_parsed_option> options) {
  auto result = vm.prepare_extension_invocation("inspect", options);
  REQUIRE_FALSE(result);
  CHECK(result.error().kind == extension_preparation_failure::usage);
  CHECK(result.error().failure.detail.code == "usage");
  CHECK(result.error().failure.detail.message == "Invalid extension invocation");
  CHECK(result.error().failure.field == "catalog.commands");
}
} // namespace
TEST_CASE("C3 A pure preparation keeps typed defaults and capability-free path text as data",
          "[external-registration][extension-preparation]") {
  auto vm = command_vm::create(profile({extension_protocol_source()}));
  REQUIRE(vm);
  auto result = vm->prepare_extension_invocation("inspect", {});
  REQUIRE(result);
  CHECK(result->options.size() == 9);
  CHECK(result->pending_hosts.empty());
  CHECK_FALSE(result->history_query);
  CHECK(std::get<std::string>(extension_option(*result, "ledger").value).empty());
  CHECK(std::get<std::int32_t>(extension_option(*result, "count").value) == 0);
  CHECK_FALSE(std::get<bool>(extension_option(*result, "flag").value));
  for (auto& option : result->options)
    CHECK_FALSE(option.present);
  std::array input{extension_parsed_option{"ledger", true, "/private/unopened"},
                   extension_parsed_option{"count", true, "2147483647"}, extension_parsed_option{"flag", true, {}}};
  auto       supplied = vm->prepare_extension_invocation("inspect", input);
  REQUIRE(supplied);
  CHECK(supplied->pending_hosts.empty());
  CHECK(std::get<std::string>(extension_option(*supplied, "ledger").value) == "/private/unopened");
  CHECK(std::get<std::int32_t>(extension_option(*supplied, "count").value) == 2147483647);
  CHECK(std::get<bool>(extension_option(*supplied, "flag").value));
  for (auto& option : supplied->options)
    CHECK(option.present == (option.name == "ledger" || option.name == "count" || option.name == "flag"));
  for (auto name : {"workspace", "registry", "root", "base-root", "observations", "target"}) {
    std::array path_input{extension_parsed_option{name, true, "../private/unopened"}};
    auto       path_result = vm->prepare_extension_invocation("inspect", path_input);
    REQUIRE(path_result);
    CHECK(path_result->pending_hosts.empty());
    CHECK(std::get<std::string>(extension_option(*path_result, name).value) == "../private/unopened");
  }
}
TEST_CASE("C3 A every history ID preserves exact signed64 strings and authoritative presence",
          "[external-registration][extension-preparation]") {
  auto vm = command_vm::create(profile({extension_protocol_source("history_read_v1")}));
  REQUIRE(vm);
  std::array input{extension_parsed_option{"ledger", true, "secret ledger"}, extension_parsed_option{"revision", false, {}}};
  auto       absent = vm->prepare_extension_invocation("inspect", input);
  REQUIRE(absent);
  for (auto name : {"revision", "at-event", "after-event"}) {
    auto& option = extension_option(*absent, name);
    CHECK_FALSE(option.present);
    CHECK(std::get<std::string>(option.value) == "0");
    for (auto token : {"1", "2147483647", "2147483648", "9223372036854775807"}) {
      input[1]    = {name, true, token};
      auto result = vm->prepare_extension_invocation("inspect", input);
      REQUIRE(result);
      CHECK(extension_option(*result, name).present);
      CHECK(std::get<std::string>(extension_option(*result, name).value) == token);
      REQUIRE(result->history_query);
      CHECK(result->history_query->id == token);
    }
  }
  REQUIRE(absent->history_query);
  CHECK(absent->history_query->id == "0");
  CHECK(absent->history_query->limit == 50);
  CHECK(std::get<std::int32_t>(extension_option(*absent, "limit").value) == 0);
  CHECK_FALSE(extension_option(*absent, "limit").present);
  CHECK(std::holds_alternative<extension_slot_index>(extension_option(*absent, "ledger").value));
  REQUIRE(absent->pending_hosts.size() == 1);
  CHECK(absent->pending_hosts[0].kind == extension_host_kind::ledger);
  CHECK(absent->pending_hosts[0].selector == "secret ledger");
}
TEST_CASE("C3 A every malformed history ID refuses as usage without poisoning preparation",
          "[external-registration][extension-preparation]") {
  auto vm = command_vm::create(profile({extension_protocol_source("history_read_v1")}));
  REQUIRE(vm);
  for (auto name : {"revision", "at-event", "after-event"})
    for (auto token :
         {"0", "00", "01", "+1", "-1", "1.0", "1e0", "", " ", " 1", "1 ", "9223372036854775808", "999999999999999999999999999"}) {
      std::array input{extension_parsed_option{"ledger", true, "secret"}, extension_parsed_option{name, true, token}};
      extension_usage(*vm, input);
    }
  std::array input{extension_parsed_option{"ledger", true, "secret"}};
  REQUIRE(vm->prepare_extension_invocation("inspect", input));
}
TEST_CASE("C3 A history selectors exclude each other and paging while paging defaults stay native",
          "[external-registration][extension-preparation]") {
  auto vm = command_vm::create(profile({extension_protocol_source("history_read_v1")}));
  REQUIRE(vm);
  for (auto left : {"revision", "at-event", "request-id"})
    for (auto right : {"revision", "at-event", "request-id", "after-event", "limit"}) {
      if (std::string_view(left) == right)
        continue;
      std::array input{extension_parsed_option{"ledger", true, "secret"}, extension_parsed_option{left, true, "1"},
                       extension_parsed_option{right, true, "1"}};
      extension_usage(*vm, input);
    }
  for (auto token : {"1", "100"}) {
    std::array input{extension_parsed_option{"ledger", true, "secret"},
                     extension_parsed_option{"after-event", true, "9223372036854775807"},
                     extension_parsed_option{"limit", true, token}};
    auto       result = vm->prepare_extension_invocation("inspect", input);
    REQUIRE(result);
    REQUIRE(result->history_query);
    CHECK(result->history_query->selector == "page");
    CHECK(result->history_query->id == "9223372036854775807");
    CHECK(result->history_query->limit == (std::string_view(token) == "1" ? 1 : 100));
  }
  for (auto token : {"0", "101", "2147483647", "2147483648", "9223372036854775807"}) {
    std::array input{extension_parsed_option{"ledger", true, "secret"}, extension_parsed_option{"limit", true, token}};
    extension_usage(*vm, input);
  }
}
TEST_CASE("C3 A request IDs use exact bounded ASCII and query selector", "[external-registration][extension-preparation]") {
  auto vm = command_vm::create(profile({extension_protocol_source("history_read_v1")}));
  REQUIRE(vm);
  for (auto token : {std::string("x"), std::string("aAZ09_.:-"), std::string(128, 'x')}) {
    std::array input{extension_parsed_option{"ledger", true, "secret"}, extension_parsed_option{"request-id", true, token}};
    auto       result = vm->prepare_extension_invocation("inspect", input);
    REQUIRE(result);
    CHECK(result->history_query->selector == "request");
    CHECK(result->history_query->request_id == token);
    CHECK(result->history_query->id == "0");
  }
  for (auto token : {std::string(""), std::string(129, 'x'), std::string("a/b"), std::string("a b"), std::string("é")}) {
    std::array input{extension_parsed_option{"ledger", true, "secret"}, extension_parsed_option{"request-id", true, token}};
    extension_usage(*vm, input);
  }
}
TEST_CASE("C3 A parsed record invariants and general scalar limits refuse closed usage",
          "[external-registration][extension-preparation]") {
  auto vm = command_vm::create(profile({extension_protocol_source()}));
  REQUIRE(vm);
  for (auto input : std::vector<std::vector<extension_parsed_option>>{{{"unknown", true, "secret"}},
                                                                      {{"--ledger", true, "secret"}},
                                                                      {{"ledger", false, "secret"}},
                                                                      {{"ledger", true, {}}},
                                                                      {{"flag", true, "true"}},
                                                                      {{"flag", false, ""}},
                                                                      {{"ledger", false, {}}, {"ledger", false, {}}}})
    extension_usage(*vm, input);
  for (auto token : {"0", "01", "-1", "+1", "2147483648", "9223372036854775807", "1.0", "1e0", " 1", ""}) {
    std::array input{extension_parsed_option{"count", true, token}};
    extension_usage(*vm, input);
  }
  for (auto token :
       {std::string(""), std::string("--private"), std::string("a\0b", 3), std::string("\xff", 1), std::string(65537, 'x')}) {
    std::array input{extension_parsed_option{"ledger", true, token}};
    extension_usage(*vm, input);
  }
  std::string text(65536, 'x');
  std::array  input{extension_parsed_option{"ledger", true, text}};
  auto        result = vm->prepare_extension_invocation("inspect", input);
  REQUIRE(result);
  CHECK(std::get<std::string>(extension_option(*result, "ledger").value) == text);
}
TEST_CASE("C3 A role intent separates all pending hosts and retains matching presence guards",
          "[external-registration][extension-preparation]") {
  auto vm = command_vm::create(profile({extension_protocol_source("role_context_read_v1")}));
  REQUIRE(vm);
  std::vector<extension_parsed_option> input{{"workspace", true, "private workspace"},
                                             {"registry", true, "private registry"},
                                             {"root", true, "r=relative/../unopened"},
                                             {"job", true, "job"},
                                             {"role", true, "worker"}};
  auto                                 base = vm->prepare_extension_invocation("inspect", input);
  REQUIRE(base);
  REQUIRE(base->pending_hosts.size() == 3);
  CHECK(base->pending_hosts[0].kind == extension_host_kind::registry);
  CHECK(base->pending_hosts[1].kind == extension_host_kind::root);
  CHECK(base->pending_hosts[2].kind == extension_host_kind::workspace);
  CHECK(std::ranges::is_sorted(base->options, {}, &extension_prepared_option::name));
  CHECK_FALSE(extension_option(*base, "observations").present);
  CHECK_FALSE(std::get<extension_slot_index>(extension_option(*base, "observations").value).index);
  CHECK_FALSE(std::get<extension_slot_index>(extension_option(*base, "base-root").value).index);
  CHECK(std::get<std::string>(extension_option(*base, "job").value) == "job");
  input.push_back({"capture-outputs", true, {}});
  extension_usage(*vm, input);
  input.pop_back();
  input.push_back({"base-root", true, "r=base"});
  extension_usage(*vm, input);
  input.push_back({"capture-outputs", true, {}});
  input.push_back({"observations", true, "private observations"});
  input.push_back({"budget-bytes", true, "1048576"});
  auto full = vm->prepare_extension_invocation("inspect", input);
  REQUIRE(full);
  REQUIRE(full->pending_hosts.size() == 5);
  CHECK(full->pending_hosts[0].kind == extension_host_kind::base_root);
  CHECK(full->pending_hosts[1].kind == extension_host_kind::observations);
  CHECK(full->pending_hosts[2].kind == extension_host_kind::registry);
  CHECK(full->pending_hosts[3].kind == extension_host_kind::root);
  CHECK(full->pending_hosts[4].kind == extension_host_kind::workspace);
  for (auto& option : full->options)
    if (auto slot = std::get_if<extension_slot_index>(&option.value)) {
      REQUIRE(slot->index);
      CHECK(*slot->index < full->pending_hosts.size());
    }
  input.back().token = "1048577";
  extension_usage(*vm, input);
  input.back().token = "1";
  REQUIRE(vm->prepare_extension_invocation("inspect", input));
  input[5].token = "different=base";
  extension_usage(*vm, input);
  input[5].token = "r=base";
  for (auto token : {"missing", "=path", "r="}) {
    input[2].token = token;
    extension_usage(*vm, input);
  }
  input[2].token = "r=path";
  for (std::size_t i = 0; i < 5; ++i) {
    auto saved       = input[i];
    input[i].present = false;
    input[i].token.reset();
    extension_usage(*vm, input);
    input[i] = saved;
  }
}
TEST_CASE("C3 A preparation requires a live selected owner and results survive moves and destruction",
          "[external-registration][extension-preparation]") {
  auto embedded = command_vm::create();
  REQUIRE(embedded);
  auto refusal = embedded->prepare_extension_invocation("render", {});
  REQUIRE_FALSE(refusal);
  CHECK(refusal.error().kind == extension_preparation_failure::protocol);
  std::optional<extension_prepared_invocation> saved;
  {
    auto vm = command_vm::create(profile({extension_protocol_source()}));
    REQUIRE(vm);
    for (auto name : {"missing", "render"}) {
      auto result = vm->prepare_extension_invocation(name, {});
      REQUIRE_FALSE(result);
      CHECK(result.error().kind == extension_preparation_failure::protocol);
    }
    command_vm moved = std::move(*vm);
    auto       old   = vm->prepare_extension_invocation("inspect", {});
    REQUIRE_FALSE(old);
    CHECK(old.error().kind == extension_preparation_failure::protocol);
    auto result = moved.prepare_extension_invocation("inspect", {});
    REQUIRE(result);
    saved = std::move(*result);
    REQUIRE(moved.prepare_extension_invocation("inspect", {}));
  }
  REQUIRE(saved);
  CHECK(saved->command == "inspect");
  CHECK(saved->options.size() == 9);
}
TEST_CASE("C3 A repeated preparation charges cumulative native storage and preserves first terminal refusal",
          "[external-registration][extension-preparation]") {
  auto vm = command_vm::create(profile({extension_protocol_source()}));
  REQUIRE(vm);
  std::string                       text(65536, 'x');
  std::array                        input{extension_parsed_option{"ledger", true, text}};
  std::optional<registration_error> first;
  std::size_t                       successes = 0;
  for (std::size_t i = 0; i < 10000; ++i) {
    auto result = vm->prepare_extension_invocation("inspect", input);
    if (result) {
      ++successes;
      continue;
    }
    CHECK(result.error().kind == extension_preparation_failure::resource);
    first = std::move(result.error().failure);
    break;
  }
  REQUIRE(first);
  CHECK(successes > 0);
  CHECK(first->detail.code == "resource_limit");
  CHECK(first->detail.limit == "native_bytes");
  auto check = [&](const registration_error& error) {
    CHECK(error.detail.code == first->detail.code);
    CHECK(error.detail.message == first->detail.message);
    CHECK(error.detail.limit == first->detail.limit);
    CHECK(error.detail.maximum == first->detail.maximum);
    CHECK(error.field == first->field);
  };
  for (auto name : {"inspect", "unknown"}) {
    auto retry = vm->prepare_extension_invocation(name, {});
    REQUIRE_FALSE(retry);
    check(retry.error().failure);
  }
  auto projection = vm->resolve_projection_definition("unknown");
  REQUIRE_FALSE(projection);
  check(projection.error());
  auto shape = vm->resolve_shape_definition("unknown");
  REQUIRE_FALSE(shape);
  check(shape.error());
  auto render = vm->render_profile({});
  REQUIRE_FALSE(render);
  check(render.error());
  auto validate = vm->validate_profile({});
  REQUIRE_FALSE(validate);
  check(validate.error());
  auto select = vm->select_profile({});
  REQUIRE_FALSE(select);
  check(select.error());
  auto moved = std::move(*vm);
  auto retry = moved.prepare_extension_invocation("inspect", {});
  REQUIRE_FALSE(retry);
  check(retry.error().failure);
}
TEST_CASE("C3 A earlier retained callable failure dominates preparation without entering handler",
          "[external-registration][extension-preparation]") {
  auto vm = command_vm::create(profile(
      {extension_protocol_source() +
       "; registration.projection({id='bad',abi='projection_v1',revision='r1',input='role_context_v1',help='bad'},function() "
       "error('private') end)"}));
  REQUIRE(vm);
  auto failed = vm->invoke_projection("bad", projection_input_kind::role_context_v1, projection_input());
  REQUIRE_FALSE(failed);
  auto result = vm->prepare_extension_invocation("inspect", {});
  REQUIRE_FALSE(result);
  CHECK(result.error().failure.detail.code == failed.error().detail.code);
  CHECK(result.error().failure.detail.message == failed.error().detail.message);
  CHECK(result.error().failure.field == failed.error().field);
}

namespace {
struct extension_fixture {
  std::filesystem::path path;
  std::string           cwd, workspace, registry, root, base, observations, ledger;
  /// @param capability Capability named by the generated module.
  /// @param root_bytes Nonzero pads the mkdtemp name so the canonical root has exactly this many bytes on every platform
  ///        (the default keeps the 28-byte Linux and 36-byte macOS roots).
  explicit extension_fixture(std::string capability = "role_context_read_v1", std::size_t root_bytes = 0) {
    std::string pattern = "/tmp/planar-extension-XXXXXX";
    if (root_bytes != 0) {
      // The canonical spelling of /tmp differs by platform (/tmp or /private/tmp); the name absorbs the difference.
      const auto                 tmp_bytes  = std::filesystem::canonical("/tmp").string().size();
      const auto                 name_bytes = root_bytes - tmp_bytes - 1;
      constexpr std::string_view stem       = "planar-extension-";
      REQUIRE(root_bytes > tmp_bytes + 1 + stem.size() + 6);
      pattern = "/tmp/" + std::string(stem) + std::string(name_bytes - stem.size() - 6, 'p') + "XXXXXX";
      REQUIRE(tmp_bytes + 1 + (pattern.size() - 5) == root_bytes);
    }
    auto created = ::mkdtemp(pattern.data());
    REQUIRE(created);
    path = std::filesystem::canonical(created);
    for (auto directory : {"trusted", "workspaces/demo", "source", "base", "branch"})
      std::filesystem::create_directories(path / directory);
    put("trusted/profile.json", "{\"version\":1,\"modules\":[\"module.lua\"]}");
    put("trusted/module.lua", extension_protocol_source(capability));
    put("trusted/registry.json", "not parsed during admission");
    put("workspaces/demo/workspace.lua", "error('declaration must not execute')");
    put("observations.json", "not parsed during admission");
    put("ledger.db", "not opened as a database during admission");
    cwd          = path.string();
    workspace    = "workspaces/demo/workspace.lua";
    registry     = "trusted/registry.json";
    root         = "r=source";
    base         = "r=base";
    observations = "observations.json";
    ledger       = "ledger.db";
  }
  ~extension_fixture() {
    std::filesystem::remove_all(path);
  }
  void put(std::string_view name, std::string_view bytes) {
    std::ofstream(path / name, std::ios::binary) << bytes;
  }
  extension_invocation owner() {
    auto capture = profile_capture::open((path / "trusted/profile.json").string());
    REQUIRE(capture);
    auto result = extension_invocation::create(std::move(*capture));
    REQUIRE(result);
    return std::move(*result);
  }
  std::vector<extension_parsed_option> role(bool optional = false) {
    std::vector<extension_parsed_option> result{{"workspace", true, workspace},
                                                {"registry", true, registry},
                                                {"root", true, root},
                                                {"job", true, "j"},
                                                {"role", true, "worker"}};
    if (optional) {
      result.push_back({"base-root", true, base});
      result.push_back({"observations", true, observations});
      result.push_back({"capture-outputs", true, {}});
    }
    return result;
  }
};
void extension_same_error(const registration_error& actual, const registration_error& original) {
  CHECK(actual.detail.code == original.detail.code);
  CHECK(actual.detail.message == original.detail.message);
  CHECK(actual.detail.limit == original.detail.limit);
  CHECK(actual.detail.maximum == original.detail.maximum);
  CHECK(actual.field == original.field);
}
void extension_closed(extension_invocation& owner, const registration_error& original) {
  auto retry = owner.admit("unknown", {}, {});
  REQUIRE_FALSE(retry);
  extension_same_error(retry.error(), original);
  auto check = owner.verify();
  REQUIRE_FALSE(check);
  extension_same_error(check.error(), original);
  CHECK(owner.effective_catalog().empty());
}
} // namespace
static_assert(!std::is_copy_constructible_v<extension_invocation>);
static_assert(!std::is_default_constructible_v<extension_invocation>);
TEST_CASE("C3 B2 original captured owner admits all role slots using explicit snapshots",
          "[external-registration][extension-admission]") {
  extension_fixture f;
  auto              owner = f.owner();
  CHECK_FALSE(owner.effective_catalog().empty());
  f.root           = "r=branch/../source";
  f.base           = "r=branch/../base";
  f.workspace      = "demo";
  auto environment = (f.path / "workspaces").string();
  REQUIRE(owner.admit("inspect", f.role(true), f.cwd, environment));
  REQUIRE(owner.verify());
  // The same captured owner retains the exited physical branch too.
  std::filesystem::rename(f.path / "branch", f.path / "old-branch");
  std::filesystem::create_directory(f.path / "branch");
  auto changed = owner.verify();
  REQUIRE_FALSE(changed);
  CHECK(changed.error().detail.code == "capture_changed");
  extension_closed(owner, changed.error());
}
TEST_CASE("C3 B2 pure commands keep same-spelled host options as data", "[external-registration][extension-admission]") {
  extension_fixture f("");
  auto              owner = f.owner();
  std::array        options{extension_parsed_option{"workspace", true, "../missing"},
                            extension_parsed_option{"root", true, "not a mapping"},
                            extension_parsed_option{"ledger", true, "/no/such/input"}};
  REQUIRE(owner.admit("inspect", options, {}, {}));
  REQUIRE(owner.verify());
}
TEST_CASE("C3 B2 history admits its unused ledger without reading database content",
          "[external-registration][extension-admission]") {
  extension_fixture f("history_read_v1");
  auto              owner = f.owner();
  std::array        options{extension_parsed_option{"ledger", true, f.ledger},
                            extension_parsed_option{"revision", true, "9223372036854775807"}};
  REQUIRE(owner.admit("inspect", options, f.cwd));
  REQUIRE(owner.verify());
  f.put("ledger.db", "changed");
  auto changed = owner.verify();
  REQUIRE_FALSE(changed);
  CHECK(changed.error().detail.code == "capture_changed");
}
TEST_CASE("C3 B2 appended observations file refuses verify and stays refused", "[external-registration][extension-admission]") {
  extension_fixture f;
  auto              owner = f.owner();
  REQUIRE(owner.admit("inspect", f.role(true), f.cwd));
  REQUIRE(owner.verify());
  std::ofstream(f.path / "observations.json", std::ios::app) << "more bytes";
  auto changed = owner.verify();
  REQUIRE_FALSE(changed);
  CHECK(changed.error().detail.code == "capture_changed");
  CHECK_FALSE(owner.verify());
}
TEST_CASE("C3 B2 optional host slots are admitted even without later service use",
          "[external-registration][extension-admission]") {
  for (auto slot : {"base-root", "observations"}) {
    extension_fixture f;
    if (std::string_view(slot) == "base-root")
      f.base = "r=trusted";
    else
      f.observations = "trusted/registry.json";
    auto owner  = f.owner();
    auto result = owner.admit("inspect", f.role(true), f.cwd);
    REQUIRE_FALSE(result);
    CHECK(result.error().detail.code == "unsafe_path");
    CHECK(result.error().field == "profile");
    extension_closed(owner, result.error());
  }
}
TEST_CASE("C3 B2 final syntax and all path normalization precede metadata admission",
          "[external-registration][extension-admission]") {
  extension_fixture f;
  f.base      = "r=trusted"; // First canonical slot would overlap if touched.
  f.workspace = "demo";
  auto owner  = f.owner();
  auto result = owner.admit("inspect", f.role(true), f.cwd, {});
  REQUIRE_FALSE(result);
  CHECK(result.error().detail.code == "usage");
  extension_closed(owner, result.error());
  auto second  = f.owner();
  f.workspace  = "workspaces/demo/workspace.lua";
  f.root       = "wrong=source";
  auto invalid = second.admit("inspect", f.role(true), f.cwd);
  REQUIRE_FALSE(invalid);
  CHECK(invalid.error().detail.code == "usage");
  CHECK(invalid.error().field == "catalog.commands");
}
TEST_CASE("C3 B2 physical parent spelling refuses symlink missing and regular intermediates",
          "[external-registration][extension-admission]") {
  extension_fixture f;
  std::filesystem::create_directory_symlink(f.path / "branch", f.path / "link");
  f.put("regular", "not a directory");
  for (auto component : {"link", "missing", "regular"}) {
    f.root      = std::format("r={}/../source", component);
    auto owner  = f.owner();
    auto result = owner.admit("inspect", f.role(), f.cwd);
    REQUIRE_FALSE(result);
    CHECK(result.error().field == "profile");
    CHECK(result.error().detail.message.find(component) == std::string::npos);
    extension_closed(owner, result.error());
  }
}
TEST_CASE("C3 B2 root overlap checks both physical containment directions", "[external-registration][extension-admission]") {
  extension_fixture f;
  std::filesystem::create_directory(f.path / "trusted/nested");
  for (auto spelling : {"trusted", "trusted/nested", ".", "branch/../trusted"}) {
    f.root      = std::string("r=") + spelling;
    auto owner  = f.owner();
    auto result = owner.admit("inspect", f.role(), f.cwd);
    REQUIRE_FALSE(result);
    CHECK(result.error().detail.code == "unsafe_path");
  }
}
TEST_CASE("C3 B2 selected file aliases refuse while registry directory colocation succeeds",
          "[external-registration][extension-admission]") {
  extension_fixture f;
  auto              allowed = f.owner();
  REQUIRE(allowed.admit("inspect", f.role(), f.cwd));
  std::filesystem::create_hard_link(f.path / "trusted/module.lua", f.path / "alias");
  for (auto slot : {"registry", "workspace", "observations"}) {
    auto owner   = f.owner();
    auto options = f.role(true);
    for (auto& option : options)
      if (option.name == slot)
        option.token = "./alias";
    auto result = owner.admit("inspect", options, f.cwd);
    REQUIRE_FALSE(result);
    CHECK(result.error().detail.code == "unsafe_path");
  }
  extension_fixture history("history_read_v1");
  std::filesystem::create_hard_link(history.path / "trusted/module.lua", history.path / "alias");
  auto       owner = history.owner();
  std::array options{extension_parsed_option{"ledger", true, "alias"}};
  auto       result = owner.admit("inspect", options, history.cwd);
  REQUIRE_FALSE(result);
  CHECK(result.error().detail.code == "unsafe_path");
}
TEST_CASE("C3 B2 missing nonregular and symlink leaf slots retain native errors",
          "[external-registration][extension-admission]") {
  extension_fixture f("history_read_v1");
  std::filesystem::create_symlink(f.path / "ledger.db", f.path / "link");
  for (auto spelling : {"missing", "source", "link"}) {
    auto       owner = f.owner();
    std::array options{extension_parsed_option{"ledger", true, spelling}};
    auto       result = owner.admit("inspect", options, f.cwd);
    REQUIRE_FALSE(result);
    CHECK(result.error().detail.code == (std::string_view(spelling) == "missing" ? "io" : "path_denied"));
    CHECK(result.error().field == "profile");
    extension_closed(owner, result.error());
  }
}
TEST_CASE("C3 B2 verification retains original profile module workspace and late root receipts",
          "[external-registration][extension-admission]") {
  for (auto selected : {"trusted/profile.json", "trusted/module.lua", "workspaces/demo/workspace.lua", "trusted/registry.json",
                        "observations.json", "source", "base"}) {
    extension_fixture f;
    auto              owner = f.owner();
    REQUIRE(owner.admit("inspect", f.role(true), f.cwd));
    auto original = f.path / selected;
    std::filesystem::rename(original, original.string() + ".old");
    if (std::string_view(selected) == "source" || std::string_view(selected) == "base")
      std::filesystem::create_directory(original);
    else
      f.put(selected, "replacement");
    auto result = owner.verify();
    REQUIRE_FALSE(result);
    CHECK(result.error().detail.code == "capture_changed");
    extension_closed(owner, result.error());
  }
}
TEST_CASE("C3 B2 final admission verifies the original capture after registration",
          "[external-registration][extension-admission]") {
  extension_fixture f("");
  auto              capture = profile_capture::open((f.path / "trusted/profile.json").string());
  REQUIRE(capture);
  auto owner = extension_invocation::create(std::move(*capture));
  REQUIRE(owner);
  f.put("trusted/module.lua", "error('replacement must not execute')");
  auto result = owner->admit("inspect", {}, {});
  REQUIRE_FALSE(result);
  CHECK(result.error().detail.code == "capture_changed");
  extension_closed(*owner, result.error());
}
TEST_CASE("C3 B2 move and one admission lifecycle preserve the first terminal failure",
          "[external-registration][extension-admission]") {
  extension_fixture f;
  auto              owner      = f.owner();
  auto              registered = std::move(owner);
  CHECK_FALSE(owner.admit("inspect", {}, {}));
  REQUIRE(registered.admit("inspect", f.role(), f.cwd));
  auto admitted = f.owner();
  admitted      = std::move(registered);
  REQUIRE(admitted.verify());
  auto duplicate = admitted.admit("inspect", f.role(), f.cwd);
  REQUIRE_FALSE(duplicate);
  CHECK(duplicate.error().detail.code == "internal");
  auto failed = std::move(admitted);
  extension_closed(failed, duplicate.error());
  auto premature = f.owner();
  auto first     = premature.verify();
  REQUIRE_FALSE(first);
  extension_closed(premature, first.error());
  auto missing_family = f.owner();
  auto unknown        = missing_family.admit("render", {}, {});
  REQUIRE_FALSE(unknown);
  CHECK(unknown.error().detail.code == "internal");
  extension_closed(missing_family, unknown.error());
}
TEST_CASE("C3 B2 creation refuses moved or poisoned original captures", "[external-registration][extension-admission]") {
  extension_fixture f;
  auto              capture = profile_capture::open((f.path / "trusted/profile.json").string());
  REQUIRE(capture);
  auto moved  = std::move(*capture);
  auto absent = extension_invocation::create(std::move(*capture));
  REQUIRE_FALSE(absent);
  CHECK(absent.error().detail.code == "capture_changed");
  CHECK_FALSE(moved.admit_root("overlap", (f.path / "trusted").string()));
  auto poisoned = extension_invocation::create(std::move(moved));
  REQUIRE_FALSE(poisoned);
  CHECK(poisoned.error().detail.code == "capture_changed");
}
TEST_CASE("C3 B2 cumulative native reservation refuses huge snapshots before path allocation",
          "[external-registration][extension-admission]") {
  extension_fixture f("history_read_v1");
  auto              owner = f.owner();
  std::array        options{extension_parsed_option{"ledger", true, "ledger.db"}};
  std::string       huge_cwd = "/" + std::string(8 * 1024 * 1024, 'a');
  auto              result   = owner.admit("inspect", options, huge_cwd);
  REQUIRE_FALSE(result);
  CHECK(result.error().detail.code == "resource_limit");
  CHECK(result.error().detail.limit == "native_bytes");
  CHECK(result.error().detail.maximum == 512 * 1024 * 1024);
  auto moved = std::move(owner);
  extension_closed(moved, result.error());
}
TEST_CASE("C3 B2 named workspace uses approved lexical environment while explicit paths retain traversal",
          "[external-registration][extension-admission]") {
  extension_fixture f;
  std::filesystem::create_directory_symlink(f.path / "branch", f.path / "link");
  f.workspace      = "demo";
  auto environment = (f.path / "link/../workspaces").string();
  auto named       = f.owner();
  REQUIRE(named.admit("inspect", f.role(), f.cwd, environment));
  REQUIRE(named.verify());
  f.workspace         = "link/../workspaces/demo/workspace.lua";
  auto explicit_owner = f.owner();
  auto result         = explicit_owner.admit("inspect", f.role(), f.cwd, environment);
  REQUIRE_FALSE(result);
  CHECK(result.error().detail.code == "path_denied");
}
TEST_CASE("C3 B2 malformed oversized relevant snapshots retain text before size precedence",
          "[external-registration][extension-admission]") {
  constexpr std::size_t maximum = 64 * 1024 * 1024;
  std::string           snapshot(maximum + 1, 'x');
  snapshot[0] = '/';
  for (bool malformed : {true, false}) {
    snapshot[1] = malformed ? '\0' : 'x';
    extension_fixture history("history_read_v1");
    auto              owner = history.owner();
    std::array        options{extension_parsed_option{"ledger", true, history.ledger}};
    auto              cwd = owner.admit("inspect", options, snapshot);
    REQUIRE_FALSE(cwd);
    CHECK(cwd.error().detail.code == (malformed ? "path_denied" : "resource_limit"));
    if (!malformed)
      CHECK(cwd.error().detail.limit == "path_bytes");
    extension_fixture role;
    role.workspace   = "demo";
    auto named       = role.owner();
    auto environment = named.admit("inspect", role.role(), role.cwd, snapshot);
    REQUIRE_FALSE(environment);
    CHECK(environment.error().detail.code == (malformed ? "usage" : "resource_limit"));
    if (!malformed)
      CHECK(environment.error().detail.limit == "path_bytes");
  }
}
TEST_CASE("C3 B2 absolute explicit hosts ignore irrelevant malformed snapshots", "[external-registration][extension-admission]") {
  extension_fixture f;
  f.workspace             = (f.path / f.workspace).string();
  f.registry              = (f.path / f.registry).string();
  f.root                  = "r=" + (f.path / "source").string();
  auto              owner = f.owner();
  const std::string bad("relative\0snapshot", 17);
  REQUIRE(owner.admit("inspect", f.role(), bad, bad));
  REQUIRE(owner.verify());
}

namespace {
std::string extension_execution_source(std::string_view body, std::string_view name = "inspect") {
  auto source = extension_protocol_source();
  auto start  = source.find("function() error(");
  REQUIRE(start != std::string::npos);
  source.replace(start, std::string::npos, "function(inv,ctx,...) assert(select('#',...)==0); " + std::string(body) + " end)");
  if (name != "inspect") {
    std::size_t at = 0;
    while ((at = source.find("inspect", at)) != std::string::npos) {
      source.replace(at, 7, name);
      at += name.size();
    }
  }
  return source;
}
std::string extension_summary(std::string_view text = "'done'") {
  return "return {ok=true,value={version=1,kind='extension',command=inv.command,data={type='summary',text=" + std::string(text) +
         "}}}";
}
void extension_execution_refused(std::string_view body, std::string_view code = "internal") {
  extension_fixture f("");
  f.put("trusted/module.lua", extension_execution_source(body));
  auto owner = f.owner();
  REQUIRE(owner.admit("inspect", {}, {}));
  auto result = owner.execute();
  REQUIRE_FALSE(result);
  CHECK(std::get<registration_error>(result.error()).detail.code == code);
  CHECK(std::get<registration_error>(result.error()).field == "catalog.commands[18]");
  auto original = std::get<registration_error>(result.error());
  auto moved    = std::move(owner);
  auto retry    = moved.execute();
  REQUIRE_FALSE(retry);
  extension_same_error(std::get<registration_error>(retry.error()), original);
  extension_closed(moved, original);
}
} // namespace
TEST_CASE("C4 A executes exact retained function with scalar types defaults presence and empty context",
          "[external-registration][extension-execution]") {
  extension_fixture f("");
  f.put(
      "trusted/module.lua",
      extension_execution_source(
          "assert(select('#',inv,ctx)==2); assert(inv.version==1 and inv.command=='inspect'); "
          "assert(type(inv.options.count)=='number' and math.type(inv.options.count)=='integer' and inv.options.count==42); "
          "assert(type(inv.options.flag)=='boolean' and inv.options.flag); "
          "assert(type(inv.options.workspace)=='string' and inv.options.workspace=='/private/host/data'); "
          "assert(inv.options.ledger=='' and inv.options.registry=='' and inv.options.root=='' and inv.options['base-root']=='' "
          "and inv.options.observations=='' and inv.options.target==''); "
          "local n=0; for k,v in pairs(inv.present) do n=n+1; assert(type(v)=='boolean'); "
          "assert(v==(k=='count' or k=='flag' or k=='workspace')) end; assert(n==9); "
          "assert(type(ctx)=='table' and next(ctx)==nil); " +
          extension_summary("inv.options.workspace")));
  auto                                 owner = f.owner();
  std::vector<extension_parsed_option> options{
      {"count", true, "42"}, {"flag", true, {}}, {"workspace", true, "/private/host/data"}};
  REQUIRE(owner.admit("inspect", options, {}));
  auto result = owner.execute();
  REQUIRE(result);
  CHECK((*result)->fields.at("ok")->boolean);
  CHECK((*result)->fields.at("value")->fields.at("data")->fields.at("text")->string == "/private/host/data");
  REQUIRE_FALSE(owner.execute());
}
TEST_CASE("C4 A absent scalar defaults remain typed and input context mutations refuse",
          "[external-registration][extension-execution]") {
  for (auto target : {"inv.version=2", "inv.options.count=1", "inv.present.flag=true", "ctx.x=1"}) {
    extension_execution_refused(std::string(target) + "; " + extension_summary());
  }
  extension_fixture f("");
  f.put("trusted/module.lua",
        extension_execution_source("assert(inv.options.count==0 and math.type(inv.options.count)=='integer'); "
                                   "assert(inv.options.flag==false and inv.present.flag==false); " +
                                   extension_summary("''")));
  auto owner = f.owner();
  REQUIRE(owner.admit("inspect", {}, {}));
  auto result = owner.execute();
  REQUIRE(result);
  CHECK((*result)->fields.at("value")->fields.at("data")->fields.at("text")->string.empty());
}
TEST_CASE("C4 A retained registration aliases proxy returns preserve lifetime across owner moves",
          "[external-registration][extension-execution]") {
  extension_fixture f("");
  auto              source = extension_execution_source("return response");
  source                   = "local text='retained'; local response={ok=true,value={version=1,kind='extension',command='inspect',"
                             "data={type='summary',text=text}}}; " +
                             source;
  f.put("trusted/module.lua", source);
  auto owner = f.owner();
  REQUIRE(owner.admit("inspect", {}, {}));
  auto result = owner.execute();
  REQUIRE(result);
  auto response = *result;
  auto moved    = std::move(owner);
  CHECK(response->fields.at("value")->fields.at("data")->fields.at("text")->string == "retained");
  REQUIRE_FALSE(owner.execute());
  REQUIRE_FALSE(moved.execute());
}
TEST_CASE("C4 A maximum summary and UTF8 boundaries are exact", "[external-registration][extension-execution]") {
  for (auto expression : {"(function() local s='a'; for i=1,20 do s=s..s end; return s end)()", "'é🙂'"}) {
    extension_fixture f("");
    f.put("trusted/module.lua", extension_execution_source(extension_summary(expression)));
    auto owner = f.owner();
    REQUIRE(owner.admit("inspect", {}, {}));
    REQUIRE(owner.execute());
  }
  extension_execution_refused(extension_summary("(function() local s='a'; for i=1,20 do s=s..s end; return s..'a' end)()"),
                              "resource_limit");
  for (auto expression : {"string.char(0)", "string.char(255)", "string.char(192,128)"})
    extension_execution_refused(extension_summary(expression));
}
TEST_CASE("C4 A rejects malformed return arity root envelope and summary protocol",
          "[external-registration][extension-execution]") {
  for (auto body :
       {"return", "return nil", "return true", "return {},{}", "return {ok=true}", "return {ok=1,value={}}",
        "return {ok=true,value={},extra=1}", "return {ok=true,error={}}",
        "return {ok=true,value={version=1,kind='extension',command='inspect',data={type='summary',text='x'},extra=1}}",
        "return {ok=true,value={version=2,kind='extension',command='inspect',data={type='summary',text='x'}}}",
        "return {ok=true,value={version=1.0,kind='extension',command='inspect',data={type='summary',text='x'}}}",
        "return {ok=true,value={version=1,kind='wrong',command='inspect',data={type='summary',text='x'}}}",
        "return {ok=true,value={version=1,kind='extension',command='other',data={type='summary',text='x'}}}",
        "return {ok=true,value={version=1,kind='extension',command='inspect',data={type='role',text='x'}}}",
        "return {ok=true,value={version=1,kind='extension',command='inspect',data={type='summary',text=1}}}",
        "return {ok=true,value={version=1,kind='extension',command='inspect',data={type='summary',text='x',extra=true}}}"})
    extension_execution_refused(body);
}
TEST_CASE("C4 A accepts inner semantic errors for every render code and exact details",
          "[external-registration][extension-execution]") {
  for (auto error :
       {"{code='usage',message='',field='',details={option='--custom'}}",
        "{code='unsupported_version',message='version',field='version',details={supported=engine.array{1},received=-2147483648}}",
        "{code='lua_syntax',message='syntax',field='',file='spec.lua',line=2147483647}",
        "{code='capability_denied',message='denied',field=''}", "{code='schema',message='bad',field='spec.work[1]'}",
        "{code='reference',message='missing',field=''}",
        "{code='ownership',message='overlap',field='',details={paths=engine.array{'a/','b.txt'}}}",
        "{code='ownership',message='',field='',details={paths=engine.array{'a','a'}}}",
        "{code='dependency_cycle',message='cycle',field='',details={cycle=engine.array{'work:a:start','work:a:complete','role:r:"
        "start','role:r:complete','work:a:accepted'}}}",
        "{code='path_denied',message='denied',field=''}", "{code='capture_changed',message='changed',field=''}",
        "{code='resource_limit',message='limit',field='',details={limit='native_bytes',maximum=2147483647}}",
        "{code='observation_target_mismatch',message='target',field='',details={work='unit'}}",
        "{code='budget_exceeded',message='budget',field='',details={budget_bytes=0,mandatory_bytes=2147483647,sections=engine."
        "array{{id='base',bytes=0}}}}",
        "{code='io',message='io',field='',details={operation='read'}}", "{code='internal',message='semantic',field=''}"}) {
    extension_fixture f("");
    f.put("trusted/module.lua", extension_execution_source("return {ok=false,error=" + std::string(error) + "}"));
    auto owner = f.owner();
    REQUIRE(owner.admit("inspect", {}, {}));
    auto result = owner.execute();
    REQUIRE(result);
    CHECK_FALSE((*result)->fields.at("ok")->boolean);
    CHECK((*result)->fields.at("error")->fields.contains("code"));
    REQUIRE_FALSE(owner.execute());
  }
}
TEST_CASE("C4 A rejects public wrapper unknown code malformed semantic fields and details",
          "[external-registration][extension-execution]") {
  for (auto error : {"{version=1,kind='error',error={code='schema',message='x',field=''}}",
                     "{code='unknown',message='',field=''}",
                     "{code='schema',message='',field='',extra=1}",
                     "{code='schema',message='',field='',file='/host/path'}",
                     "{code='schema',message='',field='',file=''}",
                     "{code='schema',message='',field='',line=0}",
                     "{code='schema',message='',field='',line=2147483648}",
                     "{code='schema',message='',field='',line=1.0}",
                     "{code='schema',message=string.char(0),field=''}",
                     "{code='schema',message='',field='',details={arbitrary={nested=1}}}",
                     "{code='usage',message='',field='',details={option=1}}",
                     "{code='io',message='',field='',details={operation='',extra=1}}",
                     "{code='ownership',message='',field='',details={paths=engine.array{'b','a'}}}",
                     "{code='dependency_cycle',message='',field='',details={cycle=engine.array{'work:bad id:start'}}}",
                     "{code='resource_limit',message='',field='',details={limit='x',maximum=-1}}",
                     "{code='resource_limit',message='',field='',details={limit='x',maximum=2147483648}}",
                     "{code='resource_limit',message='',field='',details={limit='x',maximum=1.0}}",
                     "{code='observation_target_mismatch',message='',field='',details={work='bad.id'}}",
                     "{code='unsupported_version',message='',field='',details={supported=engine.array{2},received=2}}",
                     "{code='unsupported_version',message='',field='',details={supported=engine.array{1},received=2147483648}}",
                     "{code='unsupported_version',message='',field='',details={supported=engine.array{1,1},received=2}}",
                     "{code='budget_exceeded',message='',field='',details={budget_bytes=1,mandatory_bytes=2,sections=engine."
                     "array{{id='x',bytes=-1}}}}"})
    extension_execution_refused("return {ok=false,error=" + std::string(error) + "}");
}
TEST_CASE("C4 A closed context exposes no service", "[external-registration][extension-execution]") {
  extension_execution_refused("assert(ctx.read_role==nil and ctx.read_history==nil and ctx.project==nil); "
                              "ctx.read_role()");
}
TEST_CASE("C4 A original receipt mutation suppresses execution and first failure survives moves",
          "[external-registration][extension-execution]") {
  extension_fixture f("");
  f.put("trusted/module.lua", extension_execution_source("while true do end"));
  auto owner = f.owner();
  REQUIRE(owner.admit("inspect", {}, {}));
  f.put("trusted/module.lua", "changed");
  auto result = owner.execute();
  REQUIRE_FALSE(result);
  CHECK(std::get<registration_error>(result.error()).detail.code == "capture_changed");
  auto moved = std::move(owner);
  auto retry = moved.execute();
  REQUIRE_FALSE(retry);
  extension_same_error(std::get<registration_error>(retry.error()), std::get<registration_error>(result.error()));
}
TEST_CASE("C4 A original command index differs from module and canonical order", "[external-registration][extension-execution]") {
  extension_fixture f("");
  f.put("trusted/profile.json", "{\"version\":1,\"modules\":[\"module.lua\",\"second.lua\"]}");
  f.put("trusted/module.lua",
        extension_execution_source(extension_summary(), "zeta") + extension_execution_source("return nil", "alpha"));
  f.put("trusted/second.lua", extension_execution_source("return nil", "beta"));
  for (auto [name, index] : {std::pair{"alpha", 19}, std::pair{"beta", 20}}) {
    auto owner = f.owner();
    REQUIRE(owner.admit(name, {}, {}));
    auto result = owner.execute();
    REQUIRE_FALSE(result);
    CHECK(std::get<registration_error>(result.error()).field == std::format("catalog.commands[{}]", index));
  }
}
TEST_CASE("C4 A lifecycle refuses before admission and instruction failures remain terminal",
          "[external-registration][extension-execution]") {
  extension_fixture f("");
  auto              owner  = f.owner();
  auto              result = owner.execute();
  REQUIRE_FALSE(result);
  CHECK(std::get<registration_error>(result.error()).field == "catalog.commands");
  extension_closed(owner, std::get<registration_error>(result.error()));
  extension_execution_refused("while true do end", "resource_limit");
  extension_execution_refused("pcall(function() while true do end end); " + extension_summary(), "resource_limit");
  extension_execution_refused("error('/private/host/path: unexpected source traceback')");
}
TEST_CASE("C4 A semantic text has no standalone quota and whole result obeys reserved codec bound",
          "[external-registration][extension-execution]") {
  extension_fixture f("");
  f.put("trusted/module.lua",
        extension_execution_source(
            "local t='a'; for i=1,21 do t=t..t end; return {ok=false,error={code='schema',message=t,field=''}}"));
  auto owner = f.owner();
  REQUIRE(owner.admit("inspect", {}, {}));
  auto result = owner.execute();
  REQUIRE(result);
  CHECK((*result)->fields.at("error")->fields.at("message")->string.size() == 2097152);
  extension_execution_refused("local t='a'; for i=1,24 do t=t..t end; "
                              "return {ok=false,error={code='schema',message=t,field=''}}",
                              "resource_limit");
}
TEST_CASE("C4 A Lua allocation exhaustion and unsupported graph values suppress all results",
          "[external-registration][extension-execution]") {
  extension_execution_refused("local t='a'; while true do t=t..t end", "resource_limit");
  extension_execution_refused("return {ok=false,error={code='schema',message=function() end,field=''}}");
  extension_execution_refused(
      "local t={}; t[1]=t; return {ok=false,error={code='ownership',message='',field='',details={paths=t}}}");
  extension_execution_refused(
      "return {ok=false,error={code='ownership',message='',field='',details={paths=engine.array{[2]='a'}}}}");
  extension_execution_refused(
      "return {ok=false,error={code='resource_limit',message='',field='',details={limit='x',maximum=0/0}}}");
}
TEST_CASE("C4 B3 caught missing capability calls poison zero-capability handler without granting methods",
          "[external-registration][extension-execution]") {
  for (auto method : {"read_history", "read_role", "project"})
    extension_execution_refused("assert(next(ctx)==nil); pcall(function() ctx." + std::string(method) + "({}) end); " +
                                extension_summary());
}
TEST_CASE("C4 general decimal maximum remains Lua integer and history projection is lazy",
          "[external-registration][extension-execution]") {
  extension_fixture f("");
  f.put("trusted/module.lua", extension_execution_source("assert(math.type(inv.options.count)=='integer' and "
                                                         "inv.options.count==2147483647 and inv.present.count); " +
                                                         extension_summary()));
  auto       owner = f.owner();
  std::array options{extension_parsed_option{"count", true, "2147483647"}};
  REQUIRE(owner.admit("inspect", options, {}));
  REQUIRE(owner.execute());
  for (auto value : {"2147483648", "9223372036854775807"}) {
    auto       invalid = f.owner();
    std::array supplied{extension_parsed_option{"count", true, value}};
    auto       admitted = invalid.admit("inspect", supplied, {});
    REQUIRE_FALSE(admitted);
    CHECK(admitted.error().detail.code == "usage");
  }
  extension_fixture mixed("history_read_v1");
  auto              source = extension_protocol_source("history_read_v1");
  auto              start  = source.find("'history_read_v1'");
  REQUIRE(start != std::string::npos);
  source.replace(start, std::string("'history_read_v1'").size(), "'history_read_v1','projection_apply_v1'");
  start = source.find("function() error(");
  REQUIRE(start != std::string::npos);
  source.replace(start, std::string::npos,
                 "function() return {ok=false,error={code='schema',message='pair admitted without read',field='input'}} end)");
  mixed.put("trusted/module.lua", source);
  auto       admitted_pair = mixed.owner();
  std::array ledger{extension_parsed_option{"ledger", true, mixed.ledger}};
  REQUIRE(admitted_pair.admit("inspect", ledger, mixed.cwd));
  auto result = admitted_pair.execute();
  REQUIRE(result);
  CHECK_FALSE((*result)->fields.at("ok")->boolean);
  CHECK((*result)->fields.at("error")->fields.at("message")->string == "pair admitted without read");
}

TEST_CASE("C4 B3 C01 missing raw service lookup poisons pure handler even when caught",
          "[external-registration][extension-execution][c4b3c01]") {
  for (auto method : {"read_history", "read_role", "project"})
    for (bool raw : {true, false})
      for (bool call : {false, true})
        for (bool semantic : {false, true}) {
          auto lookup  = raw ? "rawget(ctx,'" + std::string(method) + "')" : "ctx." + std::string(method);
          auto attempt = "assert(next(ctx)==nil); pcall(function() local f=" + lookup + "; assert(f==nil); ";
          if (call)
            attempt += "f({}); ";
          attempt += "end); ";
          extension_execution_refused(
              attempt +
              (semantic ? "return {ok=false,error={code='schema',message='intentional',field='input'}}" : extension_summary()));
        }
}

namespace {
constexpr std::string_view c01_semantic = "return {ok=false,error={code='schema',message='intentional',field='input'}}";
constexpr std::string_view c01_history_success =
    "return {ok=true,value={version=1,kind='extension',command=inv.command,data={type='history',value=h}}}";
void c01_history_handler(extension_fixture& f, std::string_view body) {
  auto source = extension_protocol_source("history_read_v1");
  auto start  = source.find("function() error(");
  REQUIRE(start != std::string::npos);
  source.replace(start, std::string::npos, "function(inv,ctx) " + std::string(body) + " end)");
  f.put("trusted/module.lua", "local get=rawget; local advance=next; local entries=pairs; " + source);
}
void c01_refused(const auto& result) {
  REQUIRE_FALSE(result);
  REQUIRE(std::holds_alternative<registration_error>(result.error()));
  CHECK(std::get<registration_error>(result.error()).detail.code == "internal");
  CHECK(std::get<registration_error>(result.error()).field == "catalog.commands[18]");
}
} // namespace
TEST_CASE("C4 B3 C01 history missing known services refuse through ordinary and raw aliases",
          "[external-registration][extension-execution][c4b3c01]") {
  for (auto method : {"read_role", "project"})
    for (bool raw : {false, true})
      for (bool call : {false, true}) {
        extension_fixture f("history_read_v1");
        auto              lookup = raw ? "get(ctx,'" + std::string(method) + "')" : "ctx." + std::string(method);
        c01_history_handler(f, "pcall(function() local f=" + lookup + "; assert(f==nil); " + (call ? "f({}); " : "") + "end); " +
                                   std::string(c01_semantic));
        auto       owner = f.owner();
        std::array options{extension_parsed_option{"ledger", true, f.ledger}};
        REQUIRE(owner.admit("inspect", options, f.cwd));
        c01_refused(owner.execute());
        c01_refused(owner.execute());
      }
}
TEST_CASE("C4 B3 C01 actual history method has one backing across indexing rawget next and pairs",
          "[external-registration][extension-execution][c4b3c01]") {
  for (bool v2 : {false, true}) {
    extension_fixture f("history_read_v1");
    std::filesystem::remove(f.path / f.ledger);
    REQUIRE((v2 ? ledger::initialize_new_v2(f.path / f.ledger, "demo") : ledger::initialize_new(f.path / f.ledger, "demo")));
    c01_history_handler(
        f,
        "local read=ctx.read_history; assert(type(read)=='function' and read==rawget(ctx,'read_history') and "
        "read==get(ctx,'read_history')); "
        "local k,v=advance(ctx); assert(k=='read_history' and v==read and advance(ctx,k)==nil); "
        "local count=0; for k,v in entries(ctx) do count=count+1; assert(k=='read_history' and v==read) end; assert(count==1); "
        "assert(get(ctx,'unrelated')==nil and ctx.unrelated==nil and #ctx==0); "
        "local h=get(ctx,'read_history')({}); " +
            std::string(c01_history_success));
    auto       owner = f.owner();
    std::array options{extension_parsed_option{"ledger", true, f.ledger}};
    REQUIRE(owner.admit("inspect", options, f.cwd));
    REQUIRE(owner.execute());
  }
}
TEST_CASE("C4 B3 C01 service named data in scratch and sealed tables keeps ordinary lookup semantics",
          "[external-registration][extension-execution][c4b3c01]") {
  auto source = extension_execution_source(
      "local scratch={read_history=1,read_role=2,project=3}; "
      "for k,v in pairs(scratch) do assert(rawget(scratch,k)==v and rawget(sealed,k)==v and sealed[k]==v) end; "
      "assert(rawget({},'read_history')==nil and rawget({},'read_role')==nil and rawget({},'project')==nil); "
      "assert(rawget(sealed,'unrelated')==nil and rawget(ctx,'unrelated')==nil and ctx.unrelated==nil); local n=0; for i,v in "
      "ipairs(ctx) do n=n+1 end; assert(n==0 and table.concat(ctx)==''); assert(rawget(empty,'read_history')==nil and "
      "rawget(empty,'read_role')==nil and rawget(empty,'project')==nil); "
      "assert(next(ctx)==nil and #ctx==0); local n=0; for k,v in pairs(ctx) do n=n+1 end; assert(n==0); " +
      extension_summary());
  extension_fixture f("");
  f.put("trusted/module.lua", "local sealed={read_history=1,read_role=2,project=3}; local empty={}; " + source);
  auto owner = f.owner();
  REQUIRE(owner.admit("inspect", {}, {}));
  REQUIRE(owner.execute());
}

extern "C" void          planar_admission_watch(std::uint64_t device, std::uint64_t inode);
extern "C" std::uint64_t planar_admission_read_attempts();
namespace {
struct d3_watch {
  explicit d3_watch(const std::filesystem::path& path) {
    struct stat value{};
    REQUIRE(::stat(path.c_str(), &value) == 0);
    planar_admission_watch(value.st_dev, value.st_ino);
  }
  ~d3_watch() {
    planar_admission_watch(0, 0);
  }
};
void d3_handler(extension_fixture& f, std::string body, std::string shape = {}) {
  auto source = extension_protocol_source("role_context_read_v1");
  auto start  = source.find("function() error(");
  REQUIRE(start != std::string::npos);
  source.replace(start, std::string::npos, "function(inv,ctx) " + body + " end)");
  f.put("trusted/module.lua", source + shape);
}
std::string d3_return(std::string expression = "role") {
  return "return {ok=true,value={version=1,kind='extension',command=inv.command,data={type='role',value=" + expression + "}}}";
}
void d3_inputs(extension_fixture& f, bool custom = false) {
  auto fixture = std::filesystem::path(PLANAR_SCHEMA_FIXTURE);
  std::filesystem::copy(fixture / "workspace", f.path / "workspaces/demo",
                        std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
  f.put("trusted/registry.json", read_bytes(fixture / "registry/quota-tools.json"));
  f.put("observations.json", "{\"version\":1,\"checks\":[],\"findings\":[]}");
  f.root = "source=source";
  f.base = "source=base";
  if (custom)
    for (auto file : {"split.lua", "combined.lua"}) {
      auto bytes = read_bytes(f.path / "workspaces/demo" / file);
      auto start = bytes.find("fanout_review");
      REQUIRE(start != std::string::npos);
      bytes.replace(start, 13, "custom");
      f.put("workspaces/demo/" + std::string(file), bytes);
    }
}
std::vector<extension_parsed_option> d3_options(extension_fixture& f, bool outputs = false) {
  auto options     = f.role(outputs);
  options[3].token = "split";
  options[4].token = "library_worker";
  return options;
}
value_ptr d3_role(std::expected<value_ptr, extension_execution_error> result) {
  if (!result) {
    if (auto error = std::get_if<registration_error>(&result.error()))
      INFO(error->detail.message + " at " + error->field);
    if (auto error = std::get_if<assembly_error>(&result.error()))
      INFO(*codec_encode(error->error));
  }
  REQUIRE(result);
  REQUIRE((*result)->fields.at("ok")->boolean);
  return (*result)->fields.at("value")->fields.at("data")->fields.at("value");
}
} // namespace
TEST_CASE("C4 D3 lazy role service exposes exact methods and opaque original slots without reads",
          "[external-registration][role-service]") {
  extension_fixture f;
  d3_handler(f, R"(
    assert(type(inv.options.workspace)=='userdata' and type(inv.options.registry)=='userdata')
    assert(type(inv.options.root)=='userdata' and inv.options['base-root']==false and not inv.present['base-root'])
    assert(type(ctx.read_role)=='function' and rawget(ctx,'read_role')==ctx.read_role)
    local n=0; for k in pairs(ctx) do assert(k=='read_role'); n=n+1 end; assert(n==1)
    return {ok=false,error={code='schema',message='lazy',field='input'}}
  )");
  d3_watch watched(f.path / "workspaces/demo/workspace.lua");
  auto     owner = f.owner();
  REQUIRE(owner.admit("inspect", f.role(), f.cwd));
  auto moved  = std::move(owner);
  auto result = moved.execute();
  REQUIRE(result);
  CHECK((*result)->fields.at("error")->fields.at("message")->string == "lazy");
  CHECK(moved.verify());
  CHECK(planar_admission_read_attempts() == 0);
}
TEST_CASE("C4 D3 shipped role equals original trusted selection with exact empty guidance and metadata base",
          "[external-registration][role-service]") {
  extension_fixture f;
  d3_inputs(f);
  d3_handler(f, "local role=ctx.read_role({}); assert(not pcall(function() role.context.job='forged' end)); " + d3_return());
  d3_watch watched(f.path / "workspaces/demo/workspace.lua");
  auto     owner = f.owner();
  REQUIRE(owner.admit("inspect", d3_options(f), f.cwd));
  auto role = d3_role(owner.execute());
  CHECK(planar_admission_read_attempts() > 0);
  semantic_fixture expected;
  auto             selected = invoke("select_role", selection_request(semantic_validation(expected.bundle, false)));
  REQUIRE(selected->fields.contains("value"));
  auto fields = selected->fields.at("value")->fields;
  fields.emplace("shape_guidance", string_value(""));
  CHECK(*codec_encode(role) == *codec_encode(record_value(std::move(fields))));
  CHECK(role->fields.at("base")->kind == value_kind::record);
  CHECK(role->fields.at("shape_guidance")->string.empty());
  CHECK(owner.verify());
}
TEST_CASE("C4 D3 same VM all job custom shapes yield authentic guidance and immutable retained handler aliases",
          "[external-registration][role-service]") {
  extension_fixture f;
  d3_inputs(f, true);
  d3_handler(f, R"(
    local alias=inv.options; local method=ctx.read_role
    local role=method({}); assert(inv.options==alias and ctx.read_role==method)
    assert(role.shape_guidance=='split' and role.work.value.shape.kind=='custom')
    assert(not pcall(function() role.work.value.shape.obligations[1]={} end))
  )" + d3_return(),
             semantic_shape());
  auto owner = f.owner();
  REQUIRE(owner.admit("inspect", d3_options(f), f.cwd));
  auto moved = std::move(owner);
  auto role  = d3_role(moved.execute());
  CHECK(role->fields.at("shape_guidance")->string == "split");
  CHECK(role->fields.at("work")->fields.at("value")->fields.at("shape")->fields.contains("definition"));
}
TEST_CASE("C4 D3 unselected job custom shape semantic refusal remains first even when caught and followed by handler error",
          "[external-registration][role-service]") {
  extension_fixture f;
  d3_inputs(f, true);
  d3_handler(f, "pcall(function() ctx.read_role({}) end); error('later handler')", callable_shape(R"(
    if input.job.id=='combined' then return {ok=false,error={code='ownership',message='first alljob refusal',field='job.roles'}} end
    error('later shape must not run')
  )"));
  auto owner = f.owner();
  REQUIRE(owner.admit("inspect", d3_options(f), f.cwd));
  auto result = owner.execute();
  REQUIRE_FALSE(result);
  auto error = std::get_if<assembly_error>(&result.error());
  REQUIRE(error);
  CHECK(error->error->fields.at("message")->string == "first alljob refusal");
  auto again = owner.execute();
  REQUIRE_FALSE(again);
  REQUIRE(std::get_if<assembly_error>(&again.error()));
  CHECK(*codec_encode(std::get<assembly_error>(again.error()).error) == *codec_encode(error->error));
}
TEST_CASE("C4 D3 observations validate even without assertion transport and retain original details",
          "[external-registration][role-service]") {
  extension_fixture f;
  d3_inputs(f);
  f.put(
      "observations.json",
      R"({"version":1,"checks":[],"findings":[{"id":"bad","target":{"work":"library","output":"xxh3_128:00000000000000000000000000000000"},"severity":"required","text":"bad target"}]})");
  d3_handler(f, "pcall(function() ctx.read_role({}) end); return {ok=false,error={code='schema',message='later',field=''}}");
  auto options = d3_options(f);
  options.push_back({"observations", true, f.observations});
  auto owner = f.owner();
  REQUIRE(owner.admit("inspect", options, f.cwd));
  auto result = owner.execute();
  REQUIRE_FALSE(result);
  auto error = std::get_if<assembly_error>(&result.error());
  REQUIRE(error);
  CHECK(error->error->fields.at("code")->string == "observation_target_mismatch");
  CHECK(error->error->fields.at("details")->fields.at("work")->string == "library");
}
TEST_CASE("C4 D3 captured source base trees preserve exclusions outputs and exact worker reviewer optionality",
          "[external-registration][role-service]") {
  extension_fixture f;
  d3_inputs(f);
  for (auto root : {"source", "base"}) {
    std::filesystem::create_directories(f.path / root / "src/core");
    std::filesystem::create_directories(f.path / root / "generated");
    f.put(std::string(root) + "/src/core/quota.py", root);
    f.put(std::string(root) + "/generated/excluded", "excluded");
  }
  d3_handler(f,
             "assert(inv.present['base-root'] and type(inv.options['base-root'])=='userdata'); local role=ctx.read_role({}); " +
                 d3_return());
  for (auto selected_role : {"reviewer", "library_worker"}) {
    auto options     = d3_options(f, true);
    options[4].token = selected_role;
    auto owner       = f.owner();
    REQUIRE(owner.admit("inspect", options, f.cwd));
    auto role   = d3_role(owner.execute());
    bool worker = std::string_view(selected_role) == "library_worker";
    CHECK(role->fields.contains("role_output") == worker);
    CHECK(role->fields.at("observation")->fields.at("status")->string == "captured");
    CHECK(role->fields.at("targets")->items.size() == (worker ? 0 : 2));
    for (const auto& file : role->fields.at("files")->items)
      CHECK(file->fields.at("path")->string.find("generated") == std::string::npos);
  }
}
TEST_CASE("C4 D3 exact empty arguments once missing services and forged role responses remain terminal",
          "[external-registration][role-service]") {
  for (const auto& body :
       {"ctx.read_role()", "ctx.read_role({unexpected=true})", "ctx.read_role({}, {})", "ctx.read_role({}); ctx.read_role({})",
        "local missing=ctx.project", "local missing=rawget(ctx,'read_history')"}) {
    extension_fixture f;
    d3_inputs(f);
    d3_handler(f, "pcall(function() " + std::string(body) +
                      " end); return {ok=false,error={code='schema',message='caught',field=''}}");
    auto owner = f.owner();
    REQUIRE(owner.admit("inspect", d3_options(f), f.cwd));
    auto result = owner.execute();
    REQUIRE_FALSE(result);
    REQUIRE(std::get_if<registration_error>(&result.error()));
    CHECK(std::get<registration_error>(result.error()).detail.code == "internal");
  }
  for (auto expression : {"{}", "{shape_guidance='forged'}"}) {
    extension_fixture f;
    d3_inputs(f);
    d3_handler(f, "ctx.read_role({}); " + d3_return(expression));
    auto owner = f.owner();
    REQUIRE(owner.admit("inspect", d3_options(f), f.cwd));
    CHECK_FALSE(owner.execute());
  }
}
TEST_CASE("C4 D3 original root file and late slots refuse post admission changes before role publication",
          "[external-registration][role-service]") {
  for (auto file : {"workspaces/demo/workspace.lua", "trusted/registry.json", "observations.json"}) {
    extension_fixture f;
    d3_inputs(f);
    d3_handler(f, "local role=ctx.read_role({}); " + d3_return());
    auto options = d3_options(f);
    options.push_back({"observations", true, f.observations});
    auto owner = f.owner();
    REQUIRE(owner.admit("inspect", options, f.cwd));
    d3_watch watched(f.path / file);
    std::ofstream(f.path / file, std::ios::app) << "changed";
    CHECK_FALSE(owner.execute());
    CHECK(planar_admission_read_attempts() == 0);
  }
}
TEST_CASE("C4 D3 discovery accepts pre discovery reference bytes and preserves them in authentic selected work",
          "[external-registration][role-service]") {
  extension_fixture f;
  d3_inputs(f);
  d3_handler(f, "local role=ctx.read_role({}); " + d3_return());
  auto owner = f.owner();
  REQUIRE(owner.admit("inspect", d3_options(f), f.cwd));
  std::ofstream(f.path / "workspaces/demo/text/shared.md", std::ios::app) << "original discovery bytes";
  auto role         = d3_role(owner.execute());
  auto requirements = role->fields.at("work")->fields.at("value")->fields.at("constraints");
  REQUIRE_FALSE(requirements->items.empty());
  CHECK(requirements->items[0]->fields.at("text")->fields.at("bytes")->string.ends_with("original discovery bytes"));
}
TEST_CASE("C4 D3 named workspace optional budget and repeated owner construction preserve all trusted roots",
          "[external-registration][role-service]") {
  extension_fixture f;
  d3_inputs(f);
  d3_handler(f, "local role=ctx.read_role({}); " + d3_return());
  f.workspace           = "demo";
  std::string directory = (f.path / "workspaces").string();
  for (int i = 0; i < 12; ++i) {
    auto owner   = f.owner();
    auto options = d3_options(f);
    options.push_back({"budget-bytes", true, "1"});
    REQUIRE(owner.admit("inspect", options, f.cwd, directory));
    auto role = d3_role(owner.execute());
    CHECK(role->fields.at("context")->fields.at("workspace")->string == "quota");
    CHECK(role->fields.at("context")->fields.at("budget_bytes")->integer == 131072);
    CHECK(owner.verify());
  }
}
TEST_CASE("C4 D3 unselected declaration schema failure prevents custom callbacks and root mapping is semantic",
          "[external-registration][role-service]") {
  for (bool mapping : {false, true}) {
    extension_fixture f;
    d3_inputs(f, true);
    d3_handler(f, "pcall(function() ctx.read_role({}) end); error('later')", callable_shape("error('F must not run')"));
    if (mapping)
      f.root = "wrong=source";
    else {
      auto bytes = read_bytes(f.path / "workspaces/demo/combined.lua");
      auto start = bytes.find("id = \"combined\",");
      REQUIRE(start != std::string::npos);
      bytes.insert(start, "unexpected = true, ");
      f.put("workspaces/demo/combined.lua", bytes);
    }
    auto owner = f.owner();
    REQUIRE(owner.admit("inspect", d3_options(f), f.cwd));
    auto result = owner.execute();
    REQUIRE_FALSE(result);
    auto error = std::get_if<assembly_error>(&result.error());
    REQUIRE(error);
    CHECK(error->error->fields.at("code")->string == (mapping ? "reference" : "schema"));
  }
}

namespace {
void d4_handler(extension_fixture& f, std::string body, std::string projection = {}, std::string shape = {},
                std::string input = "role_context_v1") {
  d3_handler(f, std::move(body), shape + projection_callable(projection.empty() ? projection_section() : projection, input));
  auto source = read_bytes(f.path / "trusted/module.lua");
  auto at     = source.find("'role_context_read_v1'");
  REQUIRE(at != std::string::npos);
  source.replace(at, std::string("'role_context_read_v1'").size(), "'role_context_read_v1','projection_apply_v1'");
  f.put("trusted/module.lua", source);
}
constexpr std::string_view d4_return = "return {ok=true,value={version=1,kind='extension',command=inv.command,data=p}}";
value_ptr                  d4_result(std::expected<value_ptr, extension_execution_error> result) {
  std::string message;
  if (!result) {
    if (auto e = std::get_if<registration_error>(&result.error()))
      message = e->detail.message + " at " + e->field;
    if (auto e = std::get_if<assembly_error>(&result.error()))
      message = *codec_encode(e->error);
  }
  INFO(message);
  REQUIRE(result);
  REQUIRE((*result)->fields.at("ok")->boolean);
  return (*result)->fields.at("value")->fields.at("data");
}
} // namespace
TEST_CASE("C4 D4 role projection uses original role and exact shipped headed base", "[external-registration][role-projection]") {
  extension_fixture f;
  d3_inputs(f);
  d4_handler(f, "local role=ctx.read_role({}); local p=ctx.project({id='custom'}); " + std::string(d4_return),
             "assert(input.context.job=='split' and input.context.role=='library_worker' and input.shape_guidance==''); "
             "assert(not pcall(function() input.context.job='changed' end)); return {version=1,sections=engine.array{}}");
  auto owner = f.owner();
  REQUIRE(owner.admit("inspect", d3_options(f), f.cwd));
  auto             projected = d4_result(owner.execute());
  semantic_fixture expected;
  auto             request  = selection_request(semantic_validation(expected.bundle, false));
  auto             rendered = success("render", request);
  CHECK(projected->fields.at("text")->string == rendered->fields.at("packet")->fields.at("markdown")->string);
  auto metadata = rendered->fields.at("manifest")->fields.at("sections");
  REQUIRE(projected->fields.at("sections")->items.size() == metadata->items.size());
  for (std::size_t i = 0; i < metadata->items.size(); ++i) {
    auto fields = metadata->items[i]->fields;
    fields.erase("mandatory");
    CHECK(*codec_encode(projected->fields.at("sections")->items[i]) == *codec_encode(record_value(std::move(fields))));
  }
  CHECK(owner.verify());
}
TEST_CASE("C4 D4 pair stays lazy and performed role DATA remains valid after project",
          "[external-registration][role-projection]") {
  for (auto body : {std::string(c01_semantic), "local role=ctx.read_role({}); " + d3_return(),
                    "local role=ctx.read_role({}); ctx.project({id='custom'}); " + d3_return()}) {
    extension_fixture f;
    d3_inputs(f);
    d4_handler(f, body);
    auto owner = f.owner();
    REQUIRE(owner.admit("inspect", d3_options(f), f.cwd));
    REQUIRE(owner.execute());
    CHECK(owner.verify());
  }
}
TEST_CASE("C4 D4 order once arguments missing services and forged results poison caught failures",
          "[external-registration][role-projection]") {
  for (std::string body :
       {"ctx.project({id='custom'})", "ctx.read_role({});ctx.read_role({})",
        "ctx.read_role({});ctx.project({id='custom'});ctx.project({id='custom'})", "ctx.read_role({});ctx.project({})",
        "ctx.read_role({});ctx.project({id='custom',input={}})", "ctx.read_role({});ctx.project({id='missing'})",
        "local f=rawget(ctx,'read_history')", "local f=ctx.read_history"}) {
    extension_fixture f;
    d3_inputs(f);
    d4_handler(f, "pcall(function() " + body + " end); " + std::string(c01_semantic));
    auto owner = f.owner();
    REQUIRE(owner.admit("inspect", d3_options(f), f.cwd));
    c01_refused(owner.execute());
  }
  for (std::string body : std::vector<std::string>{
           "local role=ctx.read_role({});local p=ctx.project({id='custom'}); return "
           "{ok=true,value={version=1,kind='extension',command=inv.command,data={type='projection',id=p.id,revision=p.revision,"
           "definition=p.definition,text=p.text..'forged',sections=p.sections,omitted=p.omitted}}}",
           "local p={type='projection'}; " + std::string(d4_return)}) {
    extension_fixture f;
    d3_inputs(f);
    d4_handler(f, body);
    auto owner = f.owner();
    REQUIRE(owner.admit("inspect", d3_options(f), f.cwd));
    c01_refused(owner.execute());
  }
}
TEST_CASE("C4 D4 role projection refuses mismatched input and reserved section IDs", "[external-registration][role-projection]") {
  constexpr std::array ids{"authority",    "goal",         "acceptance",      "requirements",    "contracts",
                           "boundary",     "dependencies", "shape_policy",    "checks",          "outputs",
                           "observations", "preferences",  "rationale",       "workspace_notes", "job_notes",
                           "checkpoint",   "omissions",    "recorded_history"};
  for (auto id : ids) {
    extension_fixture f;
    d3_inputs(f);
    d4_handler(f, "ctx.read_role({});local p=ctx.project({id='custom'}); " + std::string(d4_return),
               projection_section("id='" + std::string(id) + "',text='x',mandatory=false"));
    auto owner = f.owner();
    REQUIRE(owner.admit("inspect", d3_options(f), f.cwd));
    auto result = owner.execute();
    REQUIRE_FALSE(result);
    CHECK(std::get<registration_error>(result.error()).field == "catalog.projections[1]");
  }
  extension_fixture f;
  d3_inputs(f);
  d4_handler(f, "ctx.read_role({});ctx.project({id='custom'}); " + std::string(c01_semantic), {}, {}, "history_snapshot_v1");
  auto owner = f.owner();
  REQUIRE(owner.admit("inspect", d3_options(f), f.cwd));
  c01_refused(owner.execute());
}
TEST_CASE("C4 D4 custom shape headed guidance and appended sections preserve ordinary render bytes and W",
          "[external-registration][role-projection]") {
  extension_fixture f;
  d3_inputs(f, true);
  d4_handler(f,
             "local role=ctx.read_role({});assert(role.shape_guidance=='split');local p=ctx.project({id='custom'}); " +
                 std::string(d4_return),
             "assert(input.shape_guidance=='split' and input.work.value.shape.kind=='custom');" + projection_section(),
             semantic_shape());
  auto owner = f.owner();
  REQUIRE(owner.admit("inspect", d3_options(f), f.cwd));
  auto             actual = d4_result(owner.execute());
  semantic_fixture fixture;
  auto             validation = semantic_validation(fixture.bundle);
  auto             request    = selection_request(validation);
  auto             vm         = command_vm::create(named_profile({{"module.lua", read_bytes(f.path / "trusted/module.lua")}}));
  REQUIRE(vm);
  auto model    = semantic_result(*vm, validation);
  auto selected = profile_selected(*vm, request);
  auto material = record_value({{"request", request}, {"model", model}, {"selected", selected}});
  auto rendered = success("render_profile_material", material);
  CHECK(actual->fields.at("text")->string ==
        rendered->fields.at("packet")->fields.at("markdown")->string + "\n## custom_section\n\nhello");
  CHECK(actual->fields.at("text")->string.find("## shape_policy\n\n") != std::string::npos);
}
TEST_CASE("C4 D4 default role budget and explicit override preserve reverse custom omissions",
          "[external-registration][role-projection]") {
  for (bool override_budget : {false, true}) {
    extension_fixture f;
    d3_inputs(f);
    d4_handler(f, "ctx.read_role({});local p=ctx.project({id='custom'}); " + std::string(d4_return),
               "local t='x';for i=1,18 do t=t..t end;return "
               "{version=1,sections=engine.array{{id='a',text=t,mandatory=false},{id='b',text=t,mandatory=false}}}");
    auto owner   = f.owner();
    auto options = d3_options(f);
    if (override_budget)
      options.push_back({"budget-bytes", true, "1048576"});
    REQUIRE(owner.admit("inspect", options, f.cwd));
    auto        actual  = d4_result(owner.execute());
    const auto& omitted = actual->fields.at("omitted")->items;
    if (override_budget) {
      CHECK(omitted.empty());
      CHECK(actual->fields.at("text")->string.size() > 400000);
    } else {
      REQUIRE(omitted.size() == 2);
      CHECK(omitted[0]->string == "b");
      CHECK(omitted[1]->string == "a");
      CHECK(actual->fields.at("text")->string.size() <= 131072);
      CHECK(actual->fields.at("text")->string.ends_with("Optional sections omitted: b, a.\n"));
    }
  }
}
TEST_CASE("C4 D4 mandatory budget error belongs to trusted assembly and survives caught handler error",
          "[external-registration][role-projection]") {
  for (bool small : {false, true}) {
    extension_fixture f;
    d3_inputs(f);
    d4_handler(
        f, "ctx.read_role({});pcall(function()ctx.project({id='custom'})end);error('later')",
        small ? "return {version=1,sections=engine.array{}}"
              : "local t='x';for i=1,18 do t=t..t end;return {version=1,sections=engine.array{{id='a',text=t,mandatory=true}}}");
    auto options = d3_options(f);
    if (small)
      options.push_back({"budget-bytes", true, "1"});
    auto owner = f.owner();
    REQUIRE(owner.admit("inspect", options, f.cwd));
    auto actual = owner.execute();
    REQUIRE_FALSE(actual);
    auto error = std::get_if<assembly_error>(&actual.error());
    REQUIRE(error);
    CHECK(error->error->fields.at("code")->string == "budget_exceeded");
    CHECK(error->error->fields.at("field")->string == "render.budget_bytes");
    CHECK(error->error->fields.at("details")->fields.at("budget_bytes")->integer == (small ? 1 : 131072));
  }
}
TEST_CASE("C4 D4 projection tenth ordinal and native section resource refusal remain terminal",
          "[external-registration][role-projection]") {
  for (std::string body :
       {"error('selected failure')",
        "local a=engine.array{};for i=1,65 do a[i]={id='s'..i,text='',mandatory=false}end;return {version=1,sections=a}"}) {
    extension_fixture f;
    d3_inputs(f);
    std::string earlier;
    for (int i = 1; i < 10; ++i)
      earlier += projection_callable("error('unselected')", "role_context_v1", "z" + std::to_string(i));
    d4_handler(f, "ctx.read_role({});pcall(function()ctx.project({id='custom'})end); " + std::string(c01_semantic), body,
               earlier);
    auto owner = f.owner();
    REQUIRE(owner.admit("inspect", d3_options(f), f.cwd));
    auto actual = owner.execute();
    REQUIRE_FALSE(actual);
    auto error = std::get_if<registration_error>(&actual.error());
    REQUIRE(error);
    CHECK(error->field == "catalog.projections[10]");
    CHECK(error->detail.code == (body.starts_with("error") ? "internal" : "resource_limit"));
  }
}
TEST_CASE("C4 D4 all job validation and observations precede any projection", "[external-registration][role-projection]") {
  for (bool observations : {false, true}) {
    extension_fixture f;
    d3_inputs(f, true);
    d4_handler(
        f, "pcall(function()ctx.read_role({});ctx.project({id='custom'})end);error('later')", "error('projection must not run')",
        observations ? semantic_shape()
                     : callable_shape("return {ok=false,error={code='ownership',message='first alljob',field='job.roles'}}"));
    auto options = d3_options(f);
    if (observations) {
      f.put("observations.json", "{\"version\":1,\"checks\":[{\"id\":\"bad\"}],\"findings\":[]}");
      options.push_back({"observations", true, f.observations});
    }
    auto owner = f.owner();
    REQUIRE(owner.admit("inspect", options, f.cwd));
    auto result = owner.execute();
    REQUIRE_FALSE(result);
    auto error = std::get_if<assembly_error>(&result.error());
    REQUIRE(error);
    CHECK(error->error->fields.at("code")->string == (observations ? "schema" : "ownership"));
  }
}
TEST_CASE("C4 D4 existing history projection keeps exact mandatory base and fixed budget",
          "[external-registration][role-projection]") {
  for (bool v2 : {false, true}) {
    extension_fixture f("history_read_v1");
    std::filesystem::remove(f.path / f.ledger);
    REQUIRE((v2 ? ledger::initialize_new_v2(f.path / f.ledger, "demo") : ledger::initialize_new(f.path / f.ledger, "demo")));
    auto source = extension_protocol_source("history_read_v1");
    auto cap    = source.find("'history_read_v1'");
    source.replace(cap, std::string("'history_read_v1'").size(), "'history_read_v1','projection_apply_v1'");
    auto body = source.find("function() error(");
    source.replace(body, std::string::npos,
                   "function(inv,ctx)local h=ctx.read_history({});local p=ctx.project({id='custom'});assert(p.text=='## "
                   "recorded_history\\n\\nRecorded history; ledger '..h.ledger..'; last event '..h.last_event..'. No "
                   "current-policy reinterpretation.\\n\\n## custom_section\\n\\nhello'); " +
                       std::string(d4_return) + " end)");
    f.put("trusted/module.lua", source + projection_callable(projection_section(), "history_snapshot_v1"));
    auto       owner = f.owner();
    std::array options{extension_parsed_option{"ledger", true, f.ledger}};
    REQUIRE(owner.admit("inspect", options, f.cwd));
    auto result = d4_result(owner.execute());
    CHECK(result->fields.at("omitted")->items.empty());
    CHECK(owner.verify());
  }
}

TEST_CASE("C4 D4 optional base priorities follow reverse custom omissions including checkpoint",
          "[external-registration][role-projection]") {
  extension_fixture f;
  d3_inputs(f);
  auto replace = [&](std::string file, std::string old, std::string replacement) {
    auto text = read_bytes(f.path / file);
    auto at   = text.find(old);
    REQUIRE(at != std::string::npos);
    text.replace(at, old.size(), replacement);
    f.put(file, text);
  };
  replace("workspaces/demo/workspace.lua", "The driver supplies stable sibling source snapshots.", std::string(60000, 'w'));
  replace("workspaces/demo/spec.lua",
          "A tiny in-memory quota makes grouping and exact-target review visible without an application framework.",
          std::string(60000, 'r'));
  replace("workspaces/demo/split.lua", "  roles = L {",
          "  notes = p.immaterial(p.md \"" + std::string(60000, 'j') + "\"),\n  roles = L {");
  f.put(
      "observations.json",
      R"({"version":1,"checks":[],"findings":[],"checkpoint":{"progress":"draft","next":"review","unresolved":"none","files":[]}})");
  d4_handler(f, "ctx.read_role({});local p=ctx.project({id='custom'}); " + std::string(d4_return),
             "local t='x';for i=1,18 do t=t..t end;return "
             "{version=1,sections=engine.array{{id='a',text=t,mandatory=false},{id='b',text=t,mandatory=false}}}");
  auto options = d3_options(f);
  options.push_back({"observations", true, f.observations});
  options.push_back({"budget-bytes", true, "30000"});
  auto owner = f.owner();
  REQUIRE(owner.admit("inspect", options, f.cwd));
  auto actual   = d4_result(owner.execute());
  auto expected = codec_decode(R"(["b","a","checkpoint","job_notes","workspace_notes","rationale"])");
  REQUIRE(expected);
  CHECK(*codec_encode(actual->fields.at("omitted")) == *codec_encode(*expected));
  CHECK(actual->fields.at("text")->string.size() <= 30000);
}

// X06 D C02 role preparation under the factory budget. These tests run the budget variant (composed ceilings injected through
// PLANAR_BUDGET_*), adopting an opaque factory owner as an extension, and compare against owners created without a factory.
namespace {
namespace rb = planar::engine::runtime_budget_test;
struct c02_outcome {
  bool        ok = false;
  value_ptr   role;
  std::string code, limit, message;
  std::size_t maximum = 0;
};
c02_outcome c02_execute(rb::extension_invocation& owner) {
  c02_outcome out;
  auto        result = owner.execute();
  if (result) {
    REQUIRE((*result)->fields.at("ok")->boolean);
    out.ok   = true;
    out.role = (*result)->fields.at("value")->fields.at("data")->fields.at("value"); ///< Small marker, not the role.
    return out;
  }
  auto error = std::get_if<rb::registration_error>(&result.error());
  REQUIRE(error);
  out.code    = error->detail.code;
  out.limit   = error->detail.limit;
  out.message = error->detail.message;
  out.maximum = error->detail.maximum;
  return out;
}
std::vector<rb::extension_parsed_option> c02_options(const std::vector<extension_parsed_option>& options) {
  std::vector<rb::extension_parsed_option> converted;
  for (const auto& option : options)
    converted.push_back({option.name, option.present, option.token});
  return converted;
}
/// Extension owner adopted from an opaque factory owner (the factory budget stays attached), admitted for the role.
rb::extension_invocation c02_factory_owner(extension_fixture& f, bool outputs) {
  auto opened = rb::profile_invocation::open_opaque((f.path / "trusted/profile.json").string());
  REQUIRE(opened);
  auto extension = std::move(*opened).into_extension();
  REQUIRE(extension);
  auto options  = c02_options(d3_options(f, outputs));
  auto admitted = extension->admit("inspect", options, f.cwd);
  if (!admitted)
    INFO(admitted.error().detail.code << " " << admitted.error().detail.message << " " << admitted.error().field);
  REQUIRE(admitted);
  return std::move(*extension);
}
/// Owner created without a factory, exactly as extension_fixture::owner() does but from the budget variant.
rb::extension_invocation c02_plain_owner(extension_fixture& f) {
  auto capture = profile_capture::open((f.path / "trusted/profile.json").string());
  REQUIRE(capture);
  auto created = rb::extension_invocation::create(std::move(*capture));
  REQUIRE(created);
  auto options  = c02_options(d3_options(f));
  auto admitted = created->admit("inspect", options, f.cwd);
  REQUIRE(admitted);
  return std::move(*created);
}
void c02_prefix(extension_fixture& f, std::string_view file, int iterations) {
  auto bytes = read_bytes(f.path / "workspaces/demo" / file);
  f.put("workspaces/demo/" + std::string(file), "for i=1," + std::to_string(iterations) + " do end\n" + bytes);
}
constexpr int c02_tail = 27400000; ///< Handler loop: ceiling minus 1,600,000 (29,000,000); two 900000 prefixes still cross it.
                                   ///< One prefix uses 28,620,200 instructions, below the independent 28,680,000 margin fault.
} // namespace
TEST_CASE("X06 D C02 role preparation charges declaration chunks to the factory budget",
          "[external-registration][role-service][x06d]") {
  auto handler = "for i=1," + std::to_string(c02_tail) + " do end local role=ctx.read_role({}); " + d3_return();
  {
    extension_fixture f;
    d3_inputs(f);
    d3_handler(f, handler);
    auto owner   = c02_factory_owner(f, false);
    auto control = c02_execute(owner);
    INFO(control.code << " " << control.limit << " " << control.message);
    REQUIRE(control.ok);
  }
  {
    extension_fixture f;
    d3_inputs(f);
    c02_prefix(f, "workspace.lua", 900000);
    d3_handler(f, handler);
    auto owner  = c02_factory_owner(f, false);
    auto middle = c02_execute(owner);
    INFO(middle.code << " " << middle.limit << " " << middle.message);
    CHECK(middle.ok);
  }
  extension_fixture f;
  d3_inputs(f);
  c02_prefix(f, "workspace.lua", 900000);
  c02_prefix(f, "spec.lua", 900000);
  d3_handler(f, handler);
  auto owner   = c02_factory_owner(f, false);
  auto subject = c02_execute(owner);
  REQUIRE_FALSE(subject.ok);
  CHECK(subject.code == "resource_limit");
  CHECK(subject.limit == "instructions");
  CHECK(subject.maximum == PLANAR_BUDGET_INSTRUCTION_MAX);
  CHECK(subject.message == "Lua instruction limit exceeded");
  auto again = c02_execute(owner);
  REQUIRE_FALSE(again.ok);
  CHECK(again.code == subject.code);
  CHECK(again.limit == subject.limit);
  CHECK(again.maximum == subject.maximum);
  CHECK(again.message == subject.message);
}
// Mutant M2: extension_invocation::create builds and attaches a factory budget. Owners created without a factory keep role
// declarations outside any budget, so the same prefixes succeed.
TEST_CASE("X06 D C02 extension owners created without a factory keep role declarations outside the budget",
          "[external-registration][role-service][x06d]") {
  extension_fixture f;
  d3_inputs(f);
  c02_prefix(f, "workspace.lua", 900000);
  c02_prefix(f, "spec.lua", 900000);
  d3_handler(f, "for i=1," + std::to_string(c02_tail) + " do end local role=ctx.read_role({}); " + d3_return());
  auto owner   = c02_plain_owner(f);
  auto outcome = c02_execute(owner);
  INFO(outcome.code << " " << outcome.limit << " " << outcome.message);
  REQUIRE(outcome.ok);
}
// Workspace padding pre-consumes native bytes up to the exact boundary of the outputs-off role. The outputs-on role also
// snapshots the directories and converts the captured records, so it must refuse as resource_limit/native_bytes with the
// owner maximum at that boundary while the outputs-off control passes, and the boundary gap between the two modes must not
// shrink below the measured snapshot and conversion charges of the outputs-on path. The mutants that kill this test are X12
// (prepare_role snapshot/directories VM charge deleted) and X13 (capture-outputs ignored, outputs forced off). The tree
// enumeration and the registry and observation read charges (X11, X11b, M_REG and M_OBS) are killed by the X06 D C03 role
// test below, not by this one.
namespace {
// Canonical byte length of the C02 budget fixture root on every platform. The default fixture roots (36 bytes on macOS, 28 on
// Linux) shift the measured boundary gap, so the C02 fixture pads its mkdtemp name to this fixed length.
constexpr std::size_t c02_root_bytes = 64;
static_assert(c02_root_bytes > 36);
c02_outcome c02_padded(std::size_t pad, bool outputs) {
  extension_fixture f{"role_context_read_v1", c02_root_bytes};
  d3_inputs(f);
  auto bytes = read_bytes(f.path / "workspaces/demo/workspace.lua");
  // The padding is spread over two .lua files so that no single text file crosses the 1 MiB text quota (C3-4e).
  const auto half = pad / 2;
  f.put("workspaces/demo/workspace.lua", "--" + std::string(half, 'x') + "\n" + bytes);
  auto spec = read_bytes(f.path / "workspaces/demo/spec.lua");
  f.put("workspaces/demo/spec.lua", "--" + std::string(pad - half, 'x') + "\n" + spec);
  d3_handler(f, "local role=ctx.read_role({}); " + d3_return());
  auto owner = c02_factory_owner(f, outputs);
  return c02_execute(owner);
}
// Largest workspace padding for which the role still prepares (so the role sits at the native boundary), or -1 when even
/// the unpadded role refuses.
std::int64_t c02_boundary(bool outputs) {
  auto padded = [&](std::size_t pad) { return c02_padded(pad, outputs); };
  if (!padded(0).ok)
    return -1;
  // The upper bracket stays below two 900000-byte files; it must refuse on native bytes, never on the text quota.
  std::size_t lo = 0, hi = 1 << 19;
  auto        upper = padded(hi);
  REQUIRE_FALSE(upper.ok);
  CHECK(upper.code == "resource_limit");
  CHECK(upper.limit == "native_bytes");
  CHECK(upper.maximum == PLANAR_BUDGET_NATIVE_MAX);
  while (hi - lo > 1) {
    auto mid                   = lo + (hi - lo) / 2;
    (padded(mid).ok ? lo : hi) = mid;
  }
  return static_cast<std::int64_t>(lo);
}
// Measured gap, in workspace padding bytes, between the outputs-off and outputs-on native boundaries. The outputs-on role
// additionally charges the two tree enumerations, the directory snapshot and the captured-record conversions; deleting any of
// those charges (or ignoring capture-outputs) lowers the gap, so it is a lower bound that must not shrink. Trees stay empty
// here to isolate the snapshot and conversion charges; the tree enumeration and the registry and observation read charges
// are pinned by the X06 D C03 role test below.
// 4525 at the former ceilings; one padding byte of boundary quantization (slope about 58 native bytes per byte) at the
// raised native ceiling. The gap grows with the canonical fixture root length (about 7 padding bytes per 1 root byte), so it is
// measured at the fixed c02_root_bytes root: 4742 in both Debug and RelWithDebInfo on macOS and 4737 in both on Linux (libc++
// container and string sizing differ by a few native bytes). The pin is the measured value of the platform running the test, so
// it leaves no slack there: a 2-byte charge deletion (snapshot/directories or exclusion lists) must fail on either platform.
// The unpadded roots measured 4524 (macOS, 36 bytes) and 4468 (Linux, 28 bytes).
#if defined(__APPLE__)
constexpr std::int64_t c02_outputs_gap  = 4742;
constexpr std::int64_t c02_off_boundary = 379083; // C07 measured 51 MiB pin; see C07 calibration record
#else
constexpr std::int64_t c02_outputs_gap  = 4737;
constexpr std::int64_t c02_off_boundary = 379109; // C07 measured Linux 51 MiB pin; see C07 calibration record
#endif
// Both re-pins (macOS 228586 to 227593 and Linux 228612 to 227619 after gate run 1; macOS 227593 to 227525 and Linux 227619 to
// 227551 after the probe repair, 68 padding bytes) are authorised by the operator decision of 2026-10-30 ('re-pin and resume').
// RA01-B1 re-pin: the embedded commands.lua grew by the six authoring registrations (2618 source bytes), which the profile
// preparation charges across its embedded payload passes. Both platforms moved by the same 993 padding bytes (macOS 228586 to
// 227593, Linux 228612 to 227619), so the shift is the stdlib source size and not a sizing difference. The mutant window below
// is unchanged. The probe repair then grew commands.lua again (fresh option tables per authoring row), moving both platforms by
// a further 68 padding bytes (macOS 227593 to 227525, Linux 227619 to 227551). Any later change to the byte length of
// commands.lua moves this pin again; re-measure it in that round.
// The outputs-off boundary moves by one padding byte per about 58 native bytes charged. Deleting the request-record conversion
// charge (6 * 320 + sizeof(value) native bytes, paid on both the outputs-off and outputs-on sides) raises the boundary by about
// 33 padding bytes and shifts no gap, so the gap pin cannot see it; this window is the only observable. The window is far
// smaller than that shift.
constexpr std::int64_t c02_off_window = 8;
} // namespace
TEST_CASE("X08 C01 role preparation outputs-off boundary pins the request-record conversion charge",
          "[external-registration][role-service][x08][c01]") {
  // Mutant killed: runtime.cpp prepare_role charge(s, 6 * 320 + sizeof(value)) deleted (BUD-CONV); the boundary then rises by
  // about 33 padding bytes, above the window.
  const auto off = c02_boundary(false);
  INFO("outputs-off boundary " << off << " pinned " << c02_off_boundary);
  CHECK(off >= c02_off_boundary - c02_off_window);
  CHECK(off <= c02_off_boundary + c02_off_window);
}
TEST_CASE("X06 D C02 role preparation charges enumeration and registry reads to the factory budget",
          "[external-registration][role-service][x06d]") {
  auto off = c02_boundary(false);
  auto on  = c02_boundary(true);
  INFO("outputs-off boundary " << off << " outputs-on boundary " << on);
  REQUIRE(off >= 0);
  REQUIRE(on >= 0);
  CHECK(c02_padded(static_cast<std::size_t>(off), false).ok);
  auto subject = c02_padded(static_cast<std::size_t>(off), true);
  INFO(subject.code << " " << subject.limit << " " << subject.message);
  REQUIRE_FALSE(subject.ok);
  CHECK(subject.code == "resource_limit");
  CHECK(subject.limit == "native_bytes");
  CHECK(subject.maximum == PLANAR_BUDGET_NATIVE_MAX);
  CHECK(off - on >= c02_outputs_gap);
}

// Pins the fixed canonical root of the C02 budget fixture the two gap tests share; a platform-dependent root would shift the
// measured boundary gap (Linux 28 bytes versus macOS 36 bytes before the fixture padded its name).
TEST_CASE("X08 C01 the C02 budget fixture root has a fixed canonical byte length",
          "[external-registration][role-service][x08c01]") {
  extension_fixture f{"role_context_read_v1", c02_root_bytes};
  CHECK(f.path.string().size() == c02_root_bytes);
  CHECK(f.path == std::filesystem::canonical(f.path));
  CHECK(std::filesystem::is_directory(f.path));
}

// X06 D C03 role preparation charges tree enumeration and registry and observation reads to the factory budget. Workspace
// padding (spread over several .lua files so the 1 MiB text quota is never reached before the native ceiling) pre-consumes
// native bytes; the boundary is the largest padding for which the outputs-on role still prepares. Adding files to either
// tree, or padding the registry or the observations, lowers that boundary by the extra charged bytes, so each charge is
// observed as a strictly positive drop (mutants X11 current tree, X11b base tree, M_REG registry read and M_OBS observation
// read each remove one drop).
namespace {
struct c03_config {
  std::size_t current_files = 0, base_files = 0, registry_pad = 0, observation_pad = 0;
};
/// Files per tree: 40 needs 45,642 nodes and 44.3 MB native, beyond what the node bisect and native sweeps allow (346
/// nodes and 218 KB per entry per tree).
constexpr std::size_t c03_entries = 1;
constexpr std::size_t c03_registry_pad =
    200000; ///< Trailing whitespace added to registry or observations (D16 cap, below the text quota).
constexpr std::size_t c03_role_pad_files =
    2; ///< .lua files that share the role padding (see the Margin record section of docs/vernacular/rounds/V5-02-X06-D-C04.md).
constexpr std::size_t  c03_pad_file_max = 900000;
constexpr std::int64_t c03_window       = 1 << 18;
constexpr std::size_t  c03_bytes_per_entry =
    100; ///< Divisor sizing one garbage fill to about three quarters of the live ceiling (75 to 110 recover).
// Drop thresholds in padding units: the rounded midpoint of measured M and the predicted missing-charge drop, at least
// floor(0.90 * M). A tree mutant removes a whole enumeration (predicted drop about 0), so its floor is floor(0.90 * M). A
// registry or observation mutant removes only the read charge, one native byte per padded byte out of a slope of 58
// (registry) or 51 (observations) bytes per byte, so f is 1/58 or 1/51 and the predicted mutated drops are 196552 and
// 172416. The drops scale with c03_entries (= 1) and c03_registry_pad (= 200000): recalibrate when c03_entries,
// c03_registry_pad or a ceiling changes. Measurements: the Margin record section of docs/vernacular/rounds/V5-02-X06-D-C04.md.
// C07 tree floors for c03_entries = 1: floor(0.90 * M), M = 3770 current and 3768 base.
// Current measurements: docs/vernacular/rounds/V5-02-R15-R18-C07-calibration.md.
constexpr std::int64_t c03_cur_floor  = 3393;
constexpr std::int64_t c03_base_floor = 3391;
constexpr std::int64_t c03_reg_floor  = 198276; // measured 200000; f = 1/58, mutated drop 196552
constexpr std::int64_t c03_obs_floor  = 174140; // measured 175864; f = 1/51, mutated drop 172416

void c03_tree(extension_fixture& f, std::string_view directory, std::size_t files) {
  for (std::size_t i = 0; i < files; ++i)
    f.put(std::format("{}/t{:03}.txt", directory, i), "x\n");
}
c02_outcome c03_padded(std::size_t pad, c03_config cfg) {
  extension_fixture f;
  d3_inputs(f);
  c03_tree(f, "source", cfg.current_files);
  c03_tree(f, "base", cfg.base_files);
  f.put("trusted/registry.json", read_bytes(f.path / "trusted/registry.json") + std::string(cfg.registry_pad, ' '));
  f.put("observations.json", read_bytes(f.path / "observations.json") + std::string(cfg.observation_pad, ' '));
  constexpr const char* files[] = {"workspace.lua", "spec.lua", "split.lua", "combined.lua"};
  static_assert(c03_role_pad_files <= std::size(files));
  auto share = (pad + c03_role_pad_files - 1) / c03_role_pad_files;
  REQUIRE(share <= c03_pad_file_max);
  if (pad)
    for (std::size_t i = 0; i < c03_role_pad_files; ++i) {
      auto bytes = read_bytes(f.path / "workspaces/demo" / files[i]);
      f.put(std::string("workspaces/demo/") + files[i], "--" + std::string(share, 'x') + "\n" + bytes);
    }
  d3_handler(f, "local role=ctx.read_role({}); " + d3_return());
  auto owner = c02_factory_owner(f, true);
  return c02_execute(owner);
}
/// Largest padding in [lo, hi) for which the role prepares; lo must prepare and hi must not.
std::int64_t c03_bisect(const c03_config& cfg, std::int64_t lo, std::int64_t hi) {
  while (hi - lo > 1) {
    auto mid                                                      = lo + (hi - lo) / 2;
    (c03_padded(static_cast<std::size_t>(mid), cfg).ok ? lo : hi) = mid;
  }
  return lo;
}
/// Boundary of the all-zero configuration by doubling; the refusal at the upper bracket must be the native ceiling.
std::int64_t c03_base_boundary() {
  std::int64_t lo = 0, hi = 1 << 16;
  while (c03_padded(static_cast<std::size_t>(hi), {}).ok) {
    lo = hi;
    hi *= 2;
    REQUIRE(hi <= static_cast<std::int64_t>(c03_role_pad_files * c03_pad_file_max));
  }
  auto refused = c03_padded(static_cast<std::size_t>(hi), {});
  INFO(refused.code << " " << refused.limit << " " << refused.message);
  REQUIRE(refused.code == "resource_limit");
  REQUIRE(refused.limit == "native_bytes");
  return c03_bisect({}, lo, hi);
}
/// Boundary of cfg searched in [base - window, base]; the window must bracket it.
std::int64_t c03_boundary(const c03_config& cfg, std::int64_t base) {
  auto lo = std::max<std::int64_t>(0, base - c03_window);
  auto hi = base + 1;
  INFO("window [" << lo << ", " << hi << ")");
  REQUIRE(c03_padded(static_cast<std::size_t>(lo), cfg).ok);
  REQUIRE_FALSE(c03_padded(static_cast<std::size_t>(hi), cfg).ok);
  return c03_bisect(cfg, lo, hi);
}
} // namespace
TEST_CASE("X06 D C03 role preparation charges tree enumeration and registry and observation reads to the factory budget",
          "[external-registration][role-service][x06d][c03]") {
  // Node headroom: two trees of c03_entries files plus padded registry and observations prepare at padding 0.
  auto both_trees = c03_padded(0, {c03_entries, c03_entries, 0, 0});
  INFO(both_trees.code << " " << both_trees.limit << " " << both_trees.message);
  REQUIRE(both_trees.ok);

  auto b0 = c03_base_boundary();
  INFO("base boundary " << b0);
  auto drop_cur  = b0 - c03_boundary({c03_entries, 0, 0, 0}, b0);
  auto drop_base = b0 - c03_boundary({0, c03_entries, 0, 0}, b0);
  auto drop_reg  = b0 - c03_boundary({0, 0, c03_registry_pad, 0}, b0);
  auto drop_obs  = b0 - c03_boundary({0, 0, 0, c03_registry_pad}, b0);
  INFO("drops cur " << drop_cur << " base " << drop_base << " reg " << drop_reg << " obs " << drop_obs);
  CHECK(drop_cur > 0);  // X11
  CHECK(drop_base > 0); // X11b
  CHECK(drop_reg > 0);  // M_REG
  CHECK(drop_obs > 0);  // M_OBS
  // Additivity: drop_both >= 0.9 * (drop_cur + drop_base) holds exactly when the padding one above that bound refuses,
  // so one probe replaces a fifth bisection (the role boundary is monotone in the padding).
  auto both_bound = b0 - (9 * (drop_cur + drop_base) + 9) / 10 + 1;
  INFO("both-trees probe padding " << both_bound);
  REQUIRE(both_bound > 0);
  CHECK_FALSE(c03_padded(static_cast<std::size_t>(both_bound), {c03_entries, c03_entries, 0, 0}).ok);
  CHECK(drop_cur >= c03_cur_floor);
  CHECK(drop_base >= c03_base_floor);
  CHECK(drop_reg >= c03_reg_floor);
  CHECK(drop_obs >= c03_obs_floor);
}

namespace {
std::string c03_illegal_shape(std::string_view prelude, std::string_view tail) {
  return "local saved=registration.shape; local D={id='custom',abi='shape_v1',revision='v1',stages='optional',help=''}; "
         "registration.shape(D,function(input) " +
         std::string(prelude) + " assert(not pcall(saved,D,function()end)); " + std::string(tail) + " end)";
}
} // namespace
TEST_CASE("X06 D C03 a caught illegal registration followed by exhaustion keeps the first failure in a direct shape call",
          "[external-registration][shape-call][x06d][c03]") {
  for (auto tail : {"while true do end", "local t={} while true do t[#t+1]={} end"}) {
    auto vm = rb::command_vm::create(profile({c03_illegal_shape("", tail)}));
    REQUIRE(vm);
    auto response = vm->invoke_shape("custom", shape_input());
    REQUIRE_FALSE(response);
    CHECK(response.error().detail.code == "internal");
    CHECK(response.error().detail.message == "Invalid external profile protocol");
    CHECK(response.error().field == "catalog.shapes[1]");
    auto retry = vm->invoke_shape("custom", shape_input());
    REQUIRE_FALSE(retry);
    CHECK(retry.error().detail.code == "internal");
    CHECK(retry.error().detail.message == "Invalid external profile protocol");
    CHECK(retry.error().field == "catalog.shapes[1]");
  }
}

// A recovered allocator refusal (Lua's emergency collection frees the garbage and retries) sets the exhausted flag while the
// shape keeps running; the later illegal registration must not record a protocol failure over the resource failure.
TEST_CASE("X06 D C03 a recovered allocation refusal followed by an illegal registration keeps the resource failure in a direct "
          "shape call",
          "[external-registration][shape-call][x06d][c03]") {
  // Each fill builds live tables of about three quarters of the ceiling and is dropped before the next, so a later fill
  // crosses the live ceiling, the emergency collection frees the previous fill and the allocation is retried.
  auto garbage = std::format("local function fill() local t={{}} for i=1,{} do t[i]={{}} end return #t end ",
                             PLANAR_BUDGET_NATIVE_MAX / c03_bytes_per_entry);
  auto fills   = std::string("local n=fill() n=fill() n=fill() n=fill()");
  {
    // Reach guard: after the same fills the shape is still running (the refusal was recovered, not fatal), so an endless loop
    // ends at the instruction ceiling instead of reporting the allocator.
    auto reach_source =
        "registration.shape({id='custom',abi='shape_v1',revision='v1',stages='optional',help=''},function(input) " + garbage +
        fills + " while true do end end)";
    auto reach_vm = rb::command_vm::create(profile({reach_source}));
    REQUIRE(reach_vm);
    auto reached = reach_vm->invoke_shape("custom", shape_input());
    REQUIRE_FALSE(reached);
    INFO(reached.error().detail.code << " " << reached.error().detail.message << " " << reached.error().detail.limit);
    REQUIRE(reached.error().detail.code == "resource_limit");
    REQUIRE(reached.error().detail.limit == "instructions");
  }
  auto source = c03_illegal_shape(garbage + fills, "return {}");
  auto vm     = rb::command_vm::create(profile({source}));
  REQUIRE(vm);
  auto response = vm->invoke_shape("custom", shape_input());
  REQUIRE_FALSE(response);
  INFO(response.error().detail.code << " " << response.error().detail.message << " " << response.error().detail.limit);
  CHECK(response.error().detail.code == "resource_limit");
  CHECK(response.error().detail.message == "Lua allocation exceeded");
  CHECK(response.error().detail.limit == "lua_bytes");
  auto retry = vm->invoke_shape("custom", shape_input());
  REQUIRE_FALSE(retry);
  CHECK(retry.error().detail.code == "resource_limit");
  CHECK(retry.error().detail.message == "Lua allocation exceeded");
  CHECK(retry.error().detail.limit == "lua_bytes");
}

// CL-2: a malformed registration caught by the profile module chunk is the first terminal failure; a later exhaustion in the
// same chunk must not replace it with resource_limit.
TEST_CASE("CL-2 profile module chunk keeps a caught malformed registration over later exhaustion",
          "[external-registration][cl2]") {
  auto baseline = command_vm::create(profile({"pcall(registration.shape,{},1)"}));
  REQUIRE_FALSE(baseline);
  REQUIRE(baseline.error().detail.code == "internal");
  for (std::string tail : {"while true do end", "local t={} while true do t[#t+1]={} end"}) {
    INFO(tail);
    auto vm = command_vm::create(profile({"pcall(registration.shape,{},1); " + tail}));
    REQUIRE_FALSE(vm);
    INFO(vm.error().detail.code << " " << vm.error().detail.message << " " << vm.error().detail.limit);
    CHECK(vm.error().detail.code == baseline.error().detail.code);
    CHECK(vm.error().detail.message == baseline.error().detail.message);
    CHECK(vm.error().detail.limit == baseline.error().detail.limit);
    CHECK(vm.error().field == baseline.error().field);
  }
}

// CL-4 context: public validate_profile dispatches the shape from native code between trusted phases, so a caught illegal
// registration followed by exhaustion inside the shape is recorded first by the shape call, not by the non-nested phase pcall.
TEST_CASE("CL-4 public validate_profile keeps a caught illegal registration in a shape over later exhaustion",
          "[external-registration][profile-semantic][cl4]") {
  semantic_fixture f;
  for (std::string tail : {"return {}", "while true do end", "local t={} while true do t[#t+1]={} end"}) {
    INFO(tail);
    auto vm = command_vm::create(profile({c03_illegal_shape("", tail)}));
    REQUIRE(vm);
    auto result = vm->validate_profile(semantic_validation(f.bundle));
    REQUIRE_FALSE(result);
    INFO(result.error().detail.code << " " << result.error().detail.message << " " << result.error().detail.limit);
    CHECK(result.error().detail.code == "internal");
    CHECK(result.error().detail.message == "Invalid external profile protocol");
    CHECK(result.error().field == "catalog.shapes[1]");
    auto retry = vm->validate_profile({});
    REQUIRE_FALSE(retry);
    CHECK(retry.error().detail.code == "internal");
    CHECK(retry.error().field == "catalog.shapes[1]");
  }
}

// X07 closed factory (wave A): admission, the single dispatch and the module-private invoke path. These tests drive only the
// exported surface; the factory returns an owner that retains the exact admitted options.
namespace {
class x07_history_services final : public command_services {
public:
  int                    calls = 0;
  std::string            seen_ledger;
  command_service_result open_workspace(const command_render_options&) override {
    ++calls;
    return {};
  }
  command_service_result capture_render(const command_render_options&, const value_ptr&) override {
    ++calls;
    return {};
  }
  command_service_result initialize_ledger(const command_init_options&) override {
    ++calls;
    return {};
  }
  command_service_result read_history(const command_history_options& options, const command_history_selector& selector) override {
    ++calls;
    seen_ledger = options.ledger;
    return {record_value({{"version",
                           [] {
                             auto v     = std::make_shared<value>();
                             v->kind    = value_kind::integer;
                             v->integer = 1;
                             return value_ptr(v);
                           }()},
                          {"kind", string_value("history")},
                          {"ledger", string_value(std::string(32, 'b'))},
                          {"active_revision", string_value("none")},
                          {"last_event", string_value("0")},
                          {"data", record_value({{"selector", string_value(selector.selector)}, {"events", array_value()}})}}),
            {}};
  }
  command_service_result export_source(const command_export_options&) override {
    ++calls;
    return {};
  }
};
std::string x07_profile_selector(const extension_fixture& f) {
  return (f.path / "trusted/profile.json").string();
}
command_history_options x07_history(const std::filesystem::path& ledger) {
  return {ledger.string(), std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt};
}
acceptance::request_intent x07_load_intent(const extension_fixture& f) {
  auto intent =
      acceptance::request_intent::create_v2("req", "actor", std::nullopt, (f.path / "workspaces/demo/workspace.lua").string(),
                                            (f.path / "trusted/registry.json").string(), x07_profile_selector(f));
  REQUIRE(intent);
  return std::move(*intent);
}
void x07_expect_protocol(const std::expected<value_ptr, diagnostic>& result) {
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "internal");
  CHECK(result.error().message == "Invalid external profile protocol");
}
} // namespace
TEST_CASE("X07 profile owner is consumed by its single dispatch", "[external-registration][x07]") {
  extension_fixture f;
  auto              owner = profile_invocation::open_with_profile(x07_profile_selector(f), x07_history(f.path / "ledger.db"));
  REQUIRE(owner);
  CHECK(owner->observe().live);
  x07_history_services services;
  auto                 first = std::move(*owner).run_known(x07_history(f.path / "ledger.db"), services);
  REQUIRE(first);
  CHECK(services.calls == 1);
  CHECK_FALSE(owner->observe().live);
  CHECK(owner->observe().consumed);
  // A second dispatch on the same owner refuses with the protocol diagnostic and runs nothing.
  x07_expect_protocol(std::move(*owner).run_known(x07_history(f.path / "ledger.db"), services));
  CHECK(services.calls == 1);
}
TEST_CASE("X07 closed factory keeps the original receipts through dispatch", "[external-registration][x07]") {
  extension_fixture f;
  auto              owner = profile_invocation::open_with_profile(x07_profile_selector(f), x07_history(f.path / "ledger.db"));
  REQUIRE(owner);
  // A trusted module changed after open must refuse at dispatch through the original receipts, never recapturing.
  f.put("trusted/module.lua", extension_protocol_source("") + "\n-- changed after open");
  x07_history_services services;
  auto                 result = std::move(*owner).run_known(x07_history(f.path / "ledger.db"), services);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "capture_changed");
  CHECK(result.error().message == "Captured external profile changed");
  CHECK(services.calls == 0);
}
TEST_CASE("X07 runtime has exactly one profile VM creation call", "[external-registration][x07]") {
  const auto  path = std::filesystem::path(__FILE__).parent_path() / "runtime.cpp";
  auto        text = read_bytes(path);
  std::string squashed;
  for (const char c : text)
    if (!std::isspace(static_cast<unsigned char>(c)))
      squashed += c;
  std::size_t count = 0;
  for (auto at = squashed.find("command_vm::create(*snapshot)"); at != std::string::npos;
       at      = squashed.find("command_vm::create(*snapshot)", at + 1))
    ++count;
  CHECK(count == 1);
}
TEST_CASE("X07 exported command_vm invoke still refuses profile owners", "[external-registration][x07]") {
  auto vm = command_vm::create(profile({command()}));
  REQUIRE(vm);
  untouched_services services;
  auto               result = vm->invoke(command_history_options{}, services);
  REQUIRE_FALSE(result);
  CHECK(result.error().message == "Invalid external profile protocol");
  CHECK(services.calls == 0);
}
TEST_CASE("X07 closed factory registers exactly once for every family", "[external-registration][x07]") {
  extension_fixture f;
  f.put("source/keep", "x");
  const auto                         selector = x07_profile_selector(f);
  const std::vector<command_options> families{
      command_render_options{(f.path / "workspaces/demo/workspace.lua").string(), (f.path / "trusted/registry.json").string(),
                             "r=" + (f.path / "source").string(), "j", "worker", std::nullopt, std::nullopt, std::nullopt, false},
      command_init_options{(f.path / "new-ledger.db").string(), "w"},
      command_history_options{(f.path / "ledger.db").string(), std::nullopt, std::nullopt, std::nullopt, std::nullopt,
                              std::nullopt, std::nullopt},
      command_export_options{(f.path / "ledger.db").string(), (f.path / "export-target").string(), 1},
  };
  auto load = profile_invocation::open_with_profile(x07_load_intent(f), (f.path / "ledger.db").string());
  REQUIRE(load);
  CHECK(load->observe().registrations == 1);
  CHECK(load->observe().known_load);
  auto opaque = profile_invocation::open_opaque(selector);
  REQUIRE(opaque);
  CHECK(opaque->observe().registrations == 1);
  std::size_t index = 0;
  for (const auto& family : families) {
    CAPTURE(index++);
    auto owner = profile_invocation::open_with_profile(selector, family);
    REQUIRE(owner);
    const auto seen = owner->observe();
    CHECK(seen.live);
    CHECK(seen.registrations == 1);
    CHECK_FALSE(seen.known_load);
  }
}
TEST_CASE("X07 selector overload refuses the load alternative before capture", "[external-registration][x07]") {
  extension_fixture f;
  // Control: the selector overload admits a history family on a valid profile, so the refusal below is the alternative.
  auto control = profile_invocation::open_with_profile(x07_profile_selector(f), x07_history(f.path / "ledger.db"));
  REQUIRE(control);
  // A load alternative never reaches capture: a missing selector gives the protocol diagnostic, not a path error.
  auto refused = profile_invocation::open_with_profile((f.path / "trusted/absent.json").string(), command_load_options{});
  REQUIRE_FALSE(refused);
  CHECK(refused.error().detail.code == "internal");
  CHECK(refused.error().detail.message == "Invalid external profile protocol");
  CHECK(refused.error().field == "profile");
  auto valid_selector = profile_invocation::open_with_profile(x07_profile_selector(f), command_load_options{});
  REQUIRE_FALSE(valid_selector);
  CHECK(valid_selector.error().detail.message == "Invalid external profile protocol");
}
TEST_CASE("X07 run_known refuses options that differ from the admitted options", "[external-registration][x07]") {
  extension_fixture f;
  f.put("ledger2.db", "second ledger");
  auto owner = profile_invocation::open_with_profile(x07_profile_selector(f), x07_history(f.path / "ledger.db"));
  REQUIRE(owner);
  x07_history_services services;
  x07_expect_protocol(std::move(*owner).run_known(x07_history(f.path / "ledger2.db"), services));
  const auto seen = owner->observe();
  CHECK(seen.live);
  CHECK_FALSE(seen.consumed);
  CHECK(seen.registrations == 1);
  CHECK(services.calls == 0);
  // The admitted options still dispatch afterwards, and exactly those options reach the service.
  auto good = std::move(*owner).run_known(x07_history(f.path / "ledger.db"), services);
  REQUIRE(good);
  CHECK(services.calls == 1);
  CHECK(services.seen_ledger == (f.path / "ledger.db").string());
}
TEST_CASE("X07 run_known refuses opaque and known load owners", "[external-registration][x07]") {
  extension_fixture          f;
  const auto                 target = f.path / "fresh-ledger.db";
  const command_init_options init{target.string(), "w"};
  x07_history_services       services;
  auto                       opaque = profile_invocation::open_opaque(x07_profile_selector(f));
  REQUIRE(opaque);
  x07_expect_protocol(std::move(*opaque).run_known(init, services));
  CHECK(opaque->observe().live);
  auto known = profile_invocation::open_known_load(x07_load_intent(f), (f.path / "ledger.db").string());
  REQUIRE(known);
  x07_expect_protocol(std::move(*known).run_known(init, services));
  CHECK(known->observe().live);
  CHECK(services.calls == 0);
  // A control history owner from the closed factory dispatches in the same test.
  {
    auto control = profile_invocation::open_with_profile(x07_profile_selector(f), x07_history(f.path / "ledger.db"));
    REQUIRE(control);
    auto result = std::move(*control).run_known(x07_history(f.path / "ledger.db"), services);
    REQUIRE(result);
    CHECK(services.calls == 1);
  }
  CHECK_FALSE(std::filesystem::exists(target));
}
TEST_CASE("X07 new runtime refusals use only closed table messages", "[external-registration][x07]") {
  const std::set<std::string> closed{"External profile resource limit exceeded", "Invalid external profile protocol",
                                     "Captured external profile changed",        "External profile overlaps a supplied input",
                                     "Invalid external profile structure",       "Invalid embedded command protocol"};
  const auto                  source = std::filesystem::path(__FILE__).parent_path();
  for (const char* file : {"runtime.cpp", "runtime.cppm"}) {
    const auto text = read_bytes(source / file);
    for (const char* name : {"profile_invocation::run_known(", "profile_invocation::open_with_profile(",
                             "profile_invocation::open_admitted(", "invoke_known("}) {
      for (auto at = text.find(name); at != std::string::npos; at = text.find(name, at + 1)) {
        // Only definitions with a body are scanned; declarations end at the semicolon first.
        const auto open = text.find('{', at);
        const auto stop = text.find(';', at);
        if (open == std::string::npos || (stop != std::string::npos && stop < open))
          continue;
        int         depth = 0;
        std::size_t end   = open;
        for (; end < text.size(); ++end) {
          depth += text[end] == '{' ? 1 : text[end] == '}' ? -1 : 0;
          if (depth == 0)
            break;
        }
        const auto body = text.substr(open, end - open);
        for (std::size_t i = 0; i < body.size(); ++i) {
          if (body[i] != '"')
            continue;
          std::string literal;
          for (++i; i < body.size() && body[i] != '"'; ++i) {
            if (body[i] == '\\' && i + 1 < body.size())
              ++i;
            literal += body[i];
          }
          // Non-message tokens are identifier-like code and field names; anything else must be a closed table message.
          const bool token = !literal.empty() && std::ranges::all_of(literal, [](char c) {
            return std::islower(static_cast<unsigned char>(c)) || std::isdigit(static_cast<unsigned char>(c)) || c == '_' ||
                   c == '.' || c == '[' || c == ']';
          });
          INFO(file << ' ' << name << ' ' << literal);
          CHECK((token || closed.contains(literal)));
        }
      }
    }
  }
}

// X07 wave B N30 narrowing rows (D7-8): late-slot admission, role capture, history decode and projection transport charge
// the factory budget owner. They reuse the X06 D C02 budget variant and its boundary technique: workspace padding
// pre-consumes native bytes up to the ceiling, and a charged stage lowers the largest padding that still prepares.
// Existing pins: the C02/C03 role tests already pin the role capture charges (snapshot, enumeration, registry and observation
// reads), so I96 below drives those same helpers as a named guard. No existing test pins admission path charging, history
// decode under run_known or projection transport after into_extension.
namespace {
namespace n30 {
c02_outcome execute(rb::extension_invocation& owner);
c02_outcome role_run(std::size_t pad, int hops, bool project) {
  extension_fixture f;
  d3_inputs(f);
  f.root = "source=source";
  for (int hop = 0; hop < hops; ++hop)
    f.root += "/../source";
  const auto half  = pad / 2;
  auto       bytes = read_bytes(f.path / "workspaces/demo/workspace.lua");
  f.put("workspaces/demo/workspace.lua", "--" + std::string(half, 'x') + "\n" + bytes);
  auto spec = read_bytes(f.path / "workspaces/demo/spec.lua");
  f.put("workspaces/demo/spec.lua", "--" + std::string(pad - half, 'x') + "\n" + spec);
  if (project)
    d4_handler(f, "local role=ctx.read_role({});local p=ctx.project({id='custom'}); " + std::string(d4_return),
               "local t='x';for i=1,18 do t=t..t end;return "
               "{version=1,sections=engine.array{{id='a',text=t,mandatory=false}}}");
  else
    d3_handler(f, "local role=ctx.read_role({}); " + d3_return());
  auto owner = c02_factory_owner(f, false);
  return execute(owner);
}
// Like c02_execute but without reading a role marker, so projection responses are accepted too.
c02_outcome execute(rb::extension_invocation& owner) {
  c02_outcome out;
  auto        result = owner.execute();
  if (result) {
    REQUIRE((*result)->fields.at("ok")->boolean);
    out.ok = true;
    return out;
  }
  auto error = std::get_if<rb::registration_error>(&result.error());
  REQUIRE(error);
  out.code    = error->detail.code;
  out.limit   = error->detail.limit;
  out.message = error->detail.message;
  out.maximum = error->detail.maximum;
  return out;
}
// Largest padding for which the run still succeeds, or -1 when even the unpadded run refuses.
std::int64_t boundary(const std::function<c02_outcome(std::size_t)>& run) {
  if (!run(0).ok)
    return -1;
  std::size_t lo = 0, hi = 1 << 19;
  auto        upper = run(hi);
  REQUIRE_FALSE(upper.ok);
  CHECK(upper.code == "resource_limit");
  CHECK(upper.limit == "native_bytes");
  CHECK(upper.maximum == PLANAR_BUDGET_NATIVE_MAX);
  while (hi - lo > 1) {
    auto mid                = lo + (hi - lo) / 2;
    (run(mid).ok ? lo : hi) = mid;
  }
  return static_cast<std::int64_t>(lo);
}
} // namespace n30
} // namespace
// Mutant: delete the admission path charges for pending hosts in extension_invocation::admit; a longer root spelling then no
// longer lowers the padding boundary.
TEST_CASE("X07 N30 late slot admission charges the factory budget owner", "[external-registration][x07][x07n30]") {
  const auto shorter = n30::boundary([](std::size_t pad) { return n30::role_run(pad, 0, false); });
  const auto longer  = n30::boundary([](std::size_t pad) { return n30::role_run(pad, 150, false); });
  INFO("short spelling boundary " << shorter << " long spelling boundary " << longer);
  REQUIRE(shorter >= 0);
  REQUIRE(longer >= 0);
  // About 1500 more spelling bytes are charged to the owner, at roughly 58 native bytes per padding byte.
  CHECK(shorter - longer >= 10);
}
// Mutant: delete the prepare_role snapshot charge. Guard over the existing C02 pins: the outputs-on role adds directory
// snapshots and record conversion, so its boundary must stay at least the measured gap below the outputs-off boundary.
TEST_CASE("X07 N30 role capture charges the factory budget owner", "[external-registration][x07][x07n30]") {
  const auto off = c02_boundary(false);
  const auto on  = c02_boundary(true);
  INFO("outputs-off boundary " << off << " outputs-on boundary " << on);
  REQUIRE(off >= 0);
  REQUIRE(on >= 0);
  auto subject = c02_padded(static_cast<std::size_t>(off), true);
  REQUIRE_FALSE(subject.ok);
  CHECK(subject.code == "resource_limit");
  CHECK(subject.limit == "native_bytes");
  CHECK(subject.maximum == PLANAR_BUDGET_NATIVE_MAX);
  CHECK(off - on >= c02_outputs_gap);
}
// Mutant: delete the projection transport charge. Guard: a 262144-byte section raw result must lower the boundary by at least
// its own size divided by the measured slope after into_extension.
TEST_CASE("X07 N30 projection transport charges the factory budget owner", "[external-registration][x07][x07n30]") {
  const auto plain     = n30::boundary([](std::size_t pad) { return n30::role_run(pad, 0, false); });
  const auto projected = n30::boundary([](std::size_t pad) { return n30::role_run(pad, 0, true); });
  INFO("plain boundary " << plain << " projected boundary " << projected);
  REQUIRE(plain >= 0);
  REQUIRE(projected >= 0);
  CHECK(plain - projected >= 1000);
}
namespace {
namespace n30 {
// Budget-variant history service returning a record padded with `chunks` strings of `chunk_bytes` bytes.
class padded_history_services final : public rb::command_services {
public:
  padded_history_services(std::size_t chunks, std::size_t chunk_bytes) : _chunks(chunks), _chunk_bytes(chunk_bytes) {
  }
  rb::command_service_result open_workspace(const rb::command_render_options&) override {
    return {};
  }
  rb::command_service_result capture_render(const rb::command_render_options&, const value_ptr&) override {
    return {};
  }
  rb::command_service_result initialize_ledger(const rb::command_init_options&) override {
    return {};
  }
  rb::command_service_result read_history(const rb::command_history_options&,
                                          const rb::command_history_selector& selector) override {
    auto data = std::map<std::string, value_ptr>{{"selector", string_value(selector.selector)}, {"events", array_value()}};
    for (std::size_t i = 0; i < _chunks; ++i)
      data.emplace(std::format("pad{:04d}", i), string_value(std::string(_chunk_bytes, 'h')));
    auto number     = std::make_shared<value>();
    number->kind    = value_kind::integer;
    number->integer = 1;
    return {record_value({{"version", number},
                          {"kind", string_value("history")},
                          {"ledger", string_value(std::string(32, 'b'))},
                          {"active_revision", string_value("none")},
                          {"last_event", string_value("0")},
                          {"data", record_value(std::move(data))}}),
            {}};
  }
  rb::command_service_result export_source(const rb::command_export_options&) override {
    return {};
  }

private:
  std::size_t _chunks, _chunk_bytes;
};
// Dispatch the shipped history callable under the budget variant against a service returning the padded record.
std::expected<value_ptr, diagnostic> known_history(std::size_t chunks) {
  extension_fixture f;
  auto              ledger = (f.path / "ledger.db").string();
  auto              opened = rb::profile_invocation::open_with_profile(
      (f.path / "trusted/profile.json").string(),
      rb::command_history_options{ledger, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt});
  INFO((opened ? std::string("opened")
               : opened.error().detail.code + " " + opened.error().detail.message + " " + opened.error().field));
  REQUIRE(opened);
  padded_history_services services(chunks, 65536);
  return std::move(*opened).run_known(
      rb::command_history_options{ledger, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt},
      services);
}
} // namespace n30
} // namespace
// Mutant: delete the history transport charge in the profile dispatch path. A history record of about 80 percent of the
// native ceiling stays under the live Lua ceiling on its own, so only a native transport charge can refuse it.
TEST_CASE("X07 N30 history decode charges the factory budget owner", "[external-registration][x07][x07n30]") {
  const auto control = n30::known_history(4);
  if (!control)
    INFO(control.error().code << " " << control.error().message);
  REQUIRE(control);
  const std::size_t chunks = (PLANAR_BUDGET_NATIVE_MAX / 10 * 8) / 65536;
  const auto        big    = n30::known_history(chunks);
  REQUIRE_FALSE(big);
  CHECK(big.error().code == "resource_limit");
  CHECK((big.error().limit == "native_bytes" || big.error().limit == "lua_bytes"));
  CHECK(big.error().maximum == PLANAR_BUDGET_NATIVE_MAX);
}

namespace {
const std::array<std::string, 6> ra01_verbs{"spec-prompt", "critique", "ingest", "explain-rejection", "observe", "orchestrator"};
const std::array<std::string, 6> ra01_handlers{"shipped.spec_prompt.v1",       "shipped.critique.v1", "shipped.ingest.v1",
                                               "shipped.explain_rejection.v1", "shipped.observe.v1",  "shipped.orchestrator.v1"};
const std::array<std::string, 6> ra01_projection_ids{"orchestrator",       "orchestrator_authority",     "orchestrator_graph",
                                                     "orchestrator_roles", "orchestrator_prescriptions", "orchestrator_actions"};
void                             ra01_conflict(const std::string& source, const std::string& field) {
  INFO(source);
  auto vm = command_vm::create(profile({source}));
  REQUIRE_FALSE(vm);
  CHECK(vm.error().detail.code == "reference");
  CHECK(vm.error().detail.message == "External registration identity conflict");
  CHECK(vm.error().field == field);
  CHECK(vm.error().detail.message.find("private") == std::string::npos);
}
std::string ra01_projection(const std::string& id) {
  return projection_callable(projection_section(), "role_context_v1", id);
}
} // namespace
TEST_CASE("RA01-B1 profile registering a reserved authoring command name is refused as identity conflict",
          "[external-registration][ra01-b1]") {
  for (const auto& verb : ra01_verbs)
    ra01_conflict(command(verb), "catalog.commands[18]");
}
TEST_CASE("RA01-B1 reserved command collision field follows registration order", "[external-registration][ra01-b1]") {
  for (const auto& verb : ra01_verbs) {
    ra01_conflict(command("zzz_first") + "; " + command(verb), "catalog.commands[19]");
    ra01_conflict(command(verb) + "; " + command("aaa_second"), "catalog.commands[18]");
  }
}
TEST_CASE("RA01-B1 profile registering a reserved authoring family name is refused as identity conflict",
          "[external-registration][ra01-b1]") {
  for (const auto& verb : ra01_verbs) {
    auto source = command("mine");
    auto at     = source.find("family='extension_read_v1'");
    REQUIRE(at != std::string::npos);
    source.replace(at, std::string("family='extension_read_v1'").size(), "family='" + verb + "'");
    ra01_conflict(source, "catalog.commands[18]");
  }
}
TEST_CASE("RA01-B1 profile registering a reserved authoring handler id is refused as identity conflict",
          "[external-registration][ra01-b1]") {
  for (const auto& handler : ra01_handlers) {
    auto source = command("mine");
    auto at     = source.find("custom.mine");
    REQUIRE(at != std::string::npos);
    source.replace(at, std::string("custom.mine").size(), handler);
    ra01_conflict(source, "catalog.commands[18]");
  }
}
TEST_CASE("RA01-B1 profile registering a reserved projection id is refused as identity conflict",
          "[external-registration][ra01-b1]") {
  for (const auto& id : ra01_projection_ids)
    ra01_conflict(ra01_projection(id), "catalog.projections[1]");
}
TEST_CASE("RA01-B1 reserved projection collision field follows registration order", "[external-registration][ra01-b1]") {
  for (const auto& id : ra01_projection_ids) {
    ra01_conflict(ra01_projection("zzz") + "; " + ra01_projection(id), "catalog.projections[2]");
    ra01_conflict(ra01_projection(id) + "; " + ra01_projection("aaa"), "catalog.projections[1]");
  }
}
// W2-73. Mutant killed: the registration input check (runtime.cpp, projection descriptor input) accepting
// orchestrator_context_v1, which is reserved for the embedded orchestrator projection and is never an external input.
// Controls in the same case: the admitted role_context_v1 spelling registers, and the reserved projection IDs stay refused.
TEST_CASE("RA01-B2 orchestrator_context_v1 is refused as an external projection input", "[external-registration][ra01-b2]") {
  REQUIRE(command_vm::create(profile({projection_callable(projection_section(), "role_context_v1")})));
  REQUIRE(command_vm::create(profile({projection_callable(projection_section(), "history_snapshot_v1")})));
  refused(projection_callable(projection_section(), "orchestrator_context_v1"), "internal");
  refused(projection_callable(projection_section(), "orchestrator_context_v1", "mine_projection"), "internal");
  for (const auto& id : ra01_projection_ids) {
    ra01_conflict(ra01_projection(id), "catalog.projections[1]");
  }
}
// Mutant: skip the reserved command pre-check whenever the catalog also lists a projection.
TEST_CASE("RA01-B1 reserved command is refused when the profile also registers a projection",
          "[external-registration][ra01-b1]") {
  for (const auto& verb : ra01_verbs) {
    ra01_conflict(command(verb) + "; " + ra01_projection("mine_projection"), "catalog.commands[18]");
    ra01_conflict(ra01_projection("mine_projection") + "; " + command(verb), "catalog.commands[18]");
  }
  for (const auto& handler : ra01_handlers) {
    auto source = command("mine");
    auto at     = source.find("custom.mine");
    REQUIRE(at != std::string::npos);
    source.replace(at, std::string("custom.mine").size(), handler);
    ra01_conflict(source + "; " + ra01_projection("mine_projection"), "catalog.commands[18]");
  }
}
TEST_CASE("RA01-B1 non reserved near miss names stay registrable", "[external-registration][ra01-b1]") {
  for (auto name : {"observer", "statusx", "orchestrate"}) {
    INFO(name);
    REQUIRE(command_vm::create(profile({command(name)})));
  }
  {
    auto source = command("mine");
    auto at     = source.find("custom.mine");
    REQUIRE(at != std::string::npos);
    source.replace(at, std::string("custom.mine").size(), "custom.observe");
    REQUIRE(command_vm::create(profile({source})));
  }
  for (auto id : {"orchestrate", "orchestrator_extra", "observe"}) {
    INFO(id);
    REQUIRE(command_vm::create(profile({ra01_projection(id)})));
  }
  // An uppercase id is outside the existing id spelling rule, so it keeps that diagnostic and is never the reservation.
  {
    auto vm = command_vm::create(profile({ra01_projection("Orchestrator")}));
    REQUIRE_FALSE(vm);
    CHECK(vm.error().detail.message != "External registration identity conflict");
  }
}
TEST_CASE("RA01-B1 reserved pre check skips malformed descriptors and keeps their diagnostic",
          "[external-registration][ra01-b1]") {
  const std::string common = "options=engine.array{},help='',response_kind='extension_v1',capabilities=engine.array{}";
  for (const auto& source :
       {"registration.add({name='mine',family='extension_read_v1',handler_id=5," + common + "},function() end)",
        "registration.add({name='mine',family={},handler_id='custom.mine'," + common + "},function() end)"}) {
    INFO(source);
    auto vm = command_vm::create(profile({source}));
    REQUIRE_FALSE(vm);
    CHECK(vm.error().detail.code == "internal");
    CHECK(vm.error().detail.message == "Invalid external profile protocol");
  }
}
// Mutant E17 and its siblings: the reserved command pre-check also tests response_kind, help, an option name or a capability.
// Each variant is compared with the same source carrying the neutral string 'mine'; the outcomes must be identical.
TEST_CASE("RA01-B1 reserved strings in non reserved fields keep their HEAD diagnostic", "[external-registration][ra01-b1]") {
  struct outcome {
    bool        sealed;
    std::string code, message, field;
    bool        operator==(const outcome&) const = default;
  };
  const auto run = [](const std::string& source) {
    auto vm = command_vm::create(profile({source}));
    if (vm)
      return outcome{true, {}, {}, {}};
    return outcome{false, vm.error().detail.code, vm.error().detail.message, vm.error().field};
  };
  const auto build = [](const std::string& options, const std::string& help, const std::string& kind, const std::string& caps) {
    return "registration.add({name='mine',family='extension_read_v1',handler_id='custom.mine',options=engine.array{" + options +
           "},help='" + help + "',response_kind='" + kind + "',capabilities=engine.array{" + caps + "}},function() end)";
  };
  for (const std::string reserved : {"observe", "spec-prompt"}) {
    INFO(reserved);
    const auto option = [](const std::string& n) { return "{name='" + n + "',type='text',required=true,help=''}"; };
    {
      INFO("response_kind");
      const auto got = run(build("", "", reserved, ""));
      CHECK(got == run(build("", "", "mine", "")));
      CHECK_FALSE(got.sealed);
      CHECK(got.code == "internal");
      CHECK(got.message == "Invalid external profile protocol");
    }
    {
      INFO("help");
      const auto got = run(build("", reserved, "extension_v1", ""));
      CHECK(got == run(build("", "mine", "extension_v1", "")));
      CHECK(got.message != "External registration identity conflict");
    }
    {
      INFO("option name");
      const auto got = run(build(option(reserved), "", "extension_v1", ""));
      CHECK(got == run(build(option("mine"), "", "extension_v1", "")));
      CHECK(got.message != "External registration identity conflict");
    }
    {
      INFO("capability");
      const auto got = run(build("", "", "extension_v1", "'" + reserved + "'"));
      CHECK(got == run(build("", "", "extension_v1", "'mine'")));
      CHECK_FALSE(got.sealed);
      CHECK(got.code == "internal");
      CHECK(got.message == "Invalid external profile protocol");
    }
  }
  {
    INFO("critique option name");
    const auto got = run(build("{name='critique',type='text',required=true,help=''}", "", "extension_v1", ""));
    CHECK(got == run(build("{name='mine',type='text',required=true,help=''}", "", "extension_v1", "")));
    CHECK(got.message != "External registration identity conflict");
  }
}
// Mutants: delete the handler_id skip, delete the family skip, delete all three skips. Variant (c) only pins behaviour: the
// name skip is an equivalent mutant because collect_final_descriptors refuses a non-string name (text_field via command_refuse)
// before the pre-check runs, so (c) cannot kill it.
TEST_CASE("RA01-B1 reserved pre check skips an entry with any ill typed identity field", "[external-registration][ra01-b1]") {
  const std::string common = "options=engine.array{},help='',response_kind='extension_v1',capabilities=engine.array{}";
  const auto        make   = [&](const std::string& name, const std::string& family, const std::string& handler) {
    return "registration.add({name=" + name + ",family=" + family + ",handler_id=" + handler + "," + common + "},function() end)";
  };
  struct variant {
    std::string reserved, neutral;
  };
  const std::vector<variant> variants{
      {make("'observe'", "'extension_read_v1'", "5"), make("'mine'", "'extension_read_v1'", "5")},
      {make("'mine'", "{}", "'shipped.observe.v1'"), make("'mine'", "{}", "'custom.mine'")},
      {make("5", "'extension_read_v1'", "'shipped.observe.v1'"), make("5", "'extension_read_v1'", "'custom.mine'")}};
  for (const auto& v : variants) {
    INFO(v.reserved);
    auto vm      = command_vm::create(profile({v.reserved}));
    auto control = command_vm::create(profile({v.neutral}));
    REQUIRE_FALSE(vm);
    REQUIRE_FALSE(control);
    CHECK(vm.error().detail.code == "internal");
    CHECK(vm.error().detail.message == "Invalid external profile protocol");
    CHECK(vm.error().detail.message != "External registration identity conflict");
    CHECK(vm.error().detail.code == control.error().detail.code);
    CHECK(vm.error().detail.message == control.error().detail.message);
    CHECK(vm.error().field == control.error().field);
  }
}
TEST_CASE("RA01-B1 reserved pre check continues past a malformed entry sorted before a reserved entry",
          "[external-registration][ra01-b1]") {
  // 'a' sorts before 'observe' and has an ill typed family; the pre-check must skip it and still reach 'observe'.
  auto malformed = command("a");
  auto at        = malformed.find("family='extension_read_v1'");
  REQUIRE(at != std::string::npos);
  malformed.replace(at, std::string("family='extension_read_v1'").size(), "family={}");
  auto vm = command_vm::create(profile({malformed + "; " + command("observe")}));
  REQUIRE_FALSE(vm);
  CHECK(vm.error().detail.code == "reference");
  CHECK(vm.error().detail.message == "External registration identity conflict");
  CHECK(vm.error().field == "catalog.commands[19]");
}
TEST_CASE("RA01-B1 a broken profile reports its own error before a reserved name collision", "[external-registration][ra01-b1]") {
  // The broken statement follows the reserved registration inside the same module, so module execution fails first.
  auto a = command_vm::create(profile({command("observe") + "; error('boom')"}));
  auto b = command_vm::create(profile({"error('boom')"}));
  REQUIRE_FALSE(a);
  REQUIRE_FALSE(b);
  CHECK(a.error().detail.code == b.error().detail.code);
  CHECK(a.error().detail.message == b.error().detail.message);
  CHECK(a.error().field == b.error().field);
  CHECK(a.error().detail.code != "reference");
}
TEST_CASE("RA01-B1 projection response refuses the five orchestrator section ids for both input kinds",
          "[external-registration][ra01-b1]") {
  constexpr std::array ids{"orchestrator_authority", "orchestrator_graph", "orchestrator_roles", "orchestrator_prescriptions",
                           "orchestrator_actions"};
  for (auto kind : {projection_input_kind::role_context_v1, projection_input_kind::history_snapshot_v1}) {
    auto name = kind == projection_input_kind::role_context_v1 ? "role_context_v1" : "history_snapshot_v1";
    for (auto id : ids) {
      INFO(id);
      auto vm = command_vm::create(
          profile({projection_callable(projection_section(std::string("id='") + id + "',text='x',mandatory=true"), name)}));
      REQUIRE(vm);
      auto result = vm->invoke_projection("custom", kind, projection_input());
      REQUIRE_FALSE(result);
      CHECK(result.error().detail.code == "internal");
      CHECK(result.error().detail.message == "Invalid external profile protocol");
      CHECK(result.error().field == "catalog.projections[1]");
    }
    for (auto id : {"orchestrator_extra", "orchestrate"}) {
      INFO(id);
      auto vm = command_vm::create(
          profile({projection_callable(projection_section(std::string("id='") + id + "',text='x',mandatory=true"), name)}));
      REQUIRE(vm);
      auto result = vm->invoke_projection("custom", kind, projection_input());
      REQUIRE(result);
    }
  }
}
