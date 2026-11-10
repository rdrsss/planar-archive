// Retained embedded command catalog and finite native protocol regressions.
#include <catch2/catch_test_macros.hpp>
#include <unistd.h>
import std;
import planar.engine.runtime;
using namespace planar::engine;
TEST_CASE("command registry seals mandatory embedded families without services", "[commands]") {
  auto vm = command_vm::create();
  INFO((vm ? "created" : vm.error().message));
  REQUIRE(vm.has_value());
  REQUIRE(vm->catalog());
  CHECK(vm->catalog()->fields.at("commands")->items.size() == 17);
  CHECK_FALSE(vm->seal().empty());
}
import planar.engine.command_test_0;
import planar.engine.command_test_1;
import planar.engine.command_test_2;
import planar.engine.command_test_3;
import planar.engine.command_test_4;
import planar.engine.command_test_5;
import planar.engine.command_test_6;
import planar.engine.command_test_7;
import planar.engine.command_test_8;
import planar.engine.command_test_9;
import planar.engine.command_test_10;
import planar.engine.command_test_11;
import planar.engine.command_test_12;
import planar.engine.command_test_13;
import planar.engine.command_test_14;
import planar.engine.command_test_15;
import planar.engine.command_test_16;
import planar.engine.command_test_17;
import planar.engine.command_test_18;
import planar.engine.command_test_19;
import planar.engine.command_test_20;
import planar.engine.json_codec;
import planar.engine.capture;
namespace altered = planar::engine::command_test_0;
namespace {
value_ptr text(std::string s) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = std::move(s);
  return v;
}
value_ptr number(std::int64_t n) {
  auto v     = std::make_shared<value>();
  v->kind    = value_kind::integer;
  v->integer = n;
  return v;
}
value_ptr record(std::map<std::string, value_ptr> fields = {}) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::record;
  v->fields = std::move(fields);
  return v;
}
value_ptr array(std::vector<value_ptr> items = {}) {
  auto v   = std::make_shared<value>();
  v->kind  = value_kind::list;
  v->items = std::move(items);
  return v;
}
value_ptr error_value() {
  return record({{"code", text("ledger_busy")}, {"message", text("Exact native failure")}, {"field", text("")}});
}
struct real_types {
  using services = command_services;
  using render   = command_render_options;
  using init     = command_init_options;
  using history  = command_history_options;
  using source   = command_export_options;
  using selector = command_history_selector;
  using result   = command_service_result;
};
struct altered_types {
  using services = altered::command_services;
  using render   = altered::command_render_options;
  using init     = altered::command_init_options;
  using history  = altered::command_history_options;
  using source   = altered::command_export_options;
  using selector = altered::command_history_selector;
  using result   = altered::command_service_result;
};
struct heavy_types {
  using services = command_test_17::command_services;
  using render   = command_test_17::command_render_options;
  using init     = command_test_17::command_init_options;
  using history  = command_test_17::command_history_options;
  using source   = command_test_17::command_export_options;
  using selector = command_test_17::command_history_selector;
  using result   = command_test_17::command_service_result;
};
struct closure_types {
  using services = command_test_18::command_services;
  using render   = command_test_18::command_render_options;
  using init     = command_test_18::command_init_options;
  using history  = command_test_18::command_history_options;
  using source   = command_test_18::command_export_options;
  using selector = command_test_18::command_history_selector;
  using result   = command_test_18::command_service_result;
};
template <class Types> struct fake_services : Types::services {
  using result       = typename Types::result;
  int          calls = 0;
  bool         fail = false, malformed = false;
  value_ptr    workspace, render_input;
  std::string  selected, id, key;
  std::int32_t limit = 0;
  result       open_workspace(const typename Types::render& o) override {
    ++calls;
    CHECK(o.workspace == "private-workspace.lua");
    return {record({{"file", text("workspace.lua")}, {"workspace", workspace}}), {}};
  }
  result capture_render(const typename Types::render&, const value_ptr& route) override {
    ++calls;
    CHECK(route->fields.at("file")->string == "workspace.lua");
    return {render_input, {}};
  }
  result initialize_ledger(const typename Types::init& o) override {
    ++calls;
    CHECK(o.ledger == "private-ledger.db");
    if (fail)
      return {{}, error_value()};
    auto facts = record({{"ledger", text(std::string(32, 'a'))},
                         {"workspace", text(o.workspace_id)},
                         {"active_revision", text("none")},
                         {"event", text("1")}});
    if (malformed) {
      auto v               = std::make_shared<value>(*facts);
      v->fields["unknown"] = text("bad");
      facts                = v;
    }
    return {facts, {}};
  }
  result read_history(const typename Types::history&, const typename Types::selector& selector) override {
    ++calls;
    selected = selector.selector;
    id       = selector.id;
    key      = selector.request_id;
    limit    = selector.limit;
    return {record({{"version", number(1)},
                    {"kind", text("history")},
                    {"ledger", text(std::string(32, 'b'))},
                    {"active_revision", text("none")},
                    {"last_event", text("0")},
                    {"data", record({{"selector", text(selected)}, {"events", array()}})}}),
            {}};
  }
  result export_source(const typename Types::source& o) override {
    ++calls;
    CHECK(o.target == "private-target");
    return {record({{"ledger", text(std::string(32, 'c'))},
                    {"revision", text(std::to_string(o.revision))},
                    {"source", text("9223372036854775806")}}),
            {}};
  }
};
} // namespace
TEST_CASE("command registry rejects malformed catalog owner callable ABI duplicate and missing entries atomically",
          "[commands]") {
#define REFUSED(N)                                                                                                               \
  {                                                                                                                              \
    auto vm = planar::engine::command_test_##N::command_vm::create();                                                            \
    REQUIRE_FALSE(vm.has_value());                                                                                               \
    CHECK(vm.error().code == "internal");                                                                                        \
  }
  REFUSED(1);
  REFUSED(2);
  REFUSED(3);
  REFUSED(4);
  REFUSED(5);
  REFUSED(6);
  REFUSED(7);
  REFUSED(8);
  REFUSED(11);
  REFUSED(12);
  REFUSED(13);
  REFUSED(14);
  REFUSED(15);
  REFUSED(16);
#undef REFUSED
}
TEST_CASE("command registration caught exhaustion is terminal without native services", "[commands]") {
  auto instruction = planar::engine::command_test_9::command_vm::create();
  REQUIRE_FALSE(instruction.has_value());
  CHECK(instruction.error().code == "resource_limit");
  CHECK(instruction.error().limit == "instructions");
  CHECK(instruction.error().maximum == 10000000);
  auto allocation = planar::engine::command_test_10::command_vm::create();
  REQUIRE_FALSE(allocation.has_value());
  CHECK(allocation.error().code == "resource_limit");
  CHECK(allocation.error().limit == "lua_bytes");
  CHECK(allocation.error().maximum == 64 * 1024 * 1024);
}
TEST_CASE("command retained initialization and export handlers project exact native facts after moving VM", "[commands]") {
  fake_services<real_types> provider;
  auto                      created = command_vm::create();
  REQUIRE(created.has_value());
  auto       saved = std::string(created->seal());
  command_vm moved = std::move(*created);
  CHECK_FALSE(created->catalog());
  CHECK(moved.seal() == saved);
  auto result = moved.invoke(command_init_options{"private-ledger.db", "example"}, provider);
  INFO((result ? "success" : result.error().message));
  REQUIRE(result.has_value());
  CHECK((*result)->fields.at("ok")->boolean);
  CHECK((*result)->fields.at("value")->fields.at("workspace")->string == "example");
  CHECK(provider.calls == 1);
  CHECK_FALSE(moved.invoke(command_init_options{"private-ledger.db", "example"}, provider).has_value());
  auto exported = command_vm::create();
  REQUIRE(exported.has_value());
  auto output = exported->invoke(command_export_options{"private-ledger.db", "private-target", 9223372036854775807LL}, provider);
  REQUIRE(output.has_value());
  CHECK((*output)->fields.at("value")->fields.at("revision")->string == "9223372036854775807");
  CHECK((*output)->fields.at("value")->fields.at("source")->string == "9223372036854775806");
  auto replacement = command_vm::create();
  REQUIRE(replacement.has_value());
  replacement = command_vm::create();
  REQUIRE(replacement.has_value());
}
TEST_CASE("command history selects exact typed decimal strings and defaults without Lua number conversion", "[commands]") {
  for (int scenario = 0; scenario < 6; ++scenario) {
    command_history_options o;
    o.ledger             = "private-ledger.db";
    std::string selected = "page", id = "0", key = "";
    int         limit = 50;
    if (scenario == 1) {
      o.revision = 9223372036854775807LL;
      selected   = "revision";
      id         = "9223372036854775807";
    }
    if (scenario == 2) {
      o.at_event = 9223372036854775806LL;
      selected   = "state";
      id         = "9223372036854775806";
    }
    if (scenario == 3) {
      o.request_id = "request:1";
      selected     = "request";
      key          = "request:1";
    }
    if (scenario == 4) {
      o.after_event = 2147483648LL;
      id            = "2147483648";
      o.limit       = 100;
      limit         = 100;
    }
    if (scenario == 5) {
      o.limit = 1;
      limit   = 1;
    }
    auto vm = command_vm::create();
    REQUIRE(vm.has_value());
    fake_services<real_types> provider;
    auto                      r = vm->invoke(o, provider);
    INFO(scenario);
    INFO((r ? "success" : r.error().message));
    REQUIRE(r.has_value());
    CHECK(provider.calls == 1);
    CHECK(provider.selected == selected);
    CHECK(provider.id == id);
    CHECK(provider.key == key);
    CHECK(provider.limit == limit);
  }
}
namespace {
// Fake services whose history and export facts carry a caller-chosen decimal id, to probe the native fact validation.
struct id_services : fake_services<real_types> {
  std::string last_event = "1", active = "1", source_id = "1";
  result      read_history(const command_history_options&, const command_history_selector&) override {
    ++calls;
    return {record({{"version", number(1)},
                    {"kind", text("history")},
                    {"ledger", text(std::string(32, 'b'))},
                    {"active_revision", text(active)},
                    {"last_event", text(last_event)},
                    {"data", record({{"selector", text("page")}, {"events", array()}})}}),
            {}};
  }
  result export_source(const command_export_options& o) override {
    ++calls;
    return {record({{"ledger", text(std::string(32, 'c'))},
                    {"revision", text(std::to_string(o.revision))},
                    {"source", text(source_id)}}),
            {}};
  }
};
// True when the retained history handler accepts the given last_event and active_revision facts.
bool history_accepts(std::string_view last_event, std::string_view active) {
  id_services provider;
  provider.last_event = last_event;
  provider.active     = active;
  auto vm             = command_vm::create();
  REQUIRE(vm.has_value());
  command_history_options o;
  o.ledger = "private-ledger.db";
  return vm->invoke(o, provider).has_value();
}
// True when the retained export handler accepts the given source id fact.
bool export_accepts(std::string_view source_id) {
  id_services provider;
  provider.source_id = source_id;
  auto vm            = command_vm::create();
  REQUIRE(vm.has_value());
  return vm->invoke(command_export_options{"private-ledger.db", "private-target", 1}, provider).has_value();
}
} // namespace
TEST_CASE("X08 C01 native decimal id facts are bounded at INT64_MAX exactly", "[commands][x08][c01]") {
  // Mutants killed: decimal_id maximum raised by one (accepts 9223372036854775808) and raised to UINT64_MAX (accepts the
  // 20-digit values), for history last_event and active_revision and for export source.
  CHECK(history_accepts("1", "1"));
  CHECK(history_accepts("9223372036854775807", "9223372036854775807"));
  CHECK_FALSE(history_accepts("9223372036854775808", "1"));
  CHECK_FALSE(history_accepts("1", "9223372036854775808"));
  CHECK_FALSE(history_accepts("18446744073709551615", "1"));
  CHECK_FALSE(history_accepts("1", "18446744073709551615"));
  CHECK_FALSE(history_accepts("10000000000000000000", "1"));
  CHECK(export_accepts("9223372036854775807"));
  CHECK_FALSE(export_accepts("9223372036854775808"));
  CHECK_FALSE(export_accepts("18446744073709551615"));
}
TEST_CASE("command real callable replacement is effective and private binding survives later lexical replacement", "[commands]") {
  fake_services<altered_types> provider;
  auto                         vm = altered::command_vm::create();
  REQUIRE(vm.has_value());
  auto result = vm->invoke(altered::command_init_options{"private-ledger.db", "projection"}, provider);
  REQUIRE(result.has_value());
  CHECK_FALSE((*result)->fields.at("ok")->boolean);
  CHECK((*result)->fields.at("error")->fields.at("message")->string == "Altered retained handler");
  CHECK(provider.calls == 0);
  auto real = command_vm::create();
  REQUIRE(real.has_value());
  CHECK(real->seal() != vm->seal());
}
TEST_CASE("command sealed globals libraries and engine cannot be altered by retained callable", "[commands]") {
  fake_services<altered_types> provider;
  auto                         vm = altered::command_vm::create();
  REQUIRE(vm.has_value());
  auto r = vm->invoke(altered::command_init_options{"private-ledger.db", "freeze"}, provider);
  INFO((r ? "success" : r.error().message));
  REQUIRE(r.has_value());
  CHECK((*r)->fields.at("ok")->boolean);
  CHECK(provider.calls == 1);
}
TEST_CASE("command family order exact argument maps missing service and manufactured success are refused", "[commands]") {
  for (std::string mode : {"skip", "cross", "badargs", "unknownargs", "repeat", "replace"}) {
    INFO(mode);
    fake_services<altered_types> provider;
    auto                         vm = altered::command_vm::create();
    REQUIRE(vm.has_value());
    auto r = vm->invoke(altered::command_init_options{"private-ledger.db", mode}, provider);
    REQUIRE_FALSE(r.has_value());
    CHECK(r.error().code == "internal");
    CHECK(provider.calls == ((mode == "repeat" || mode == "replace") ? 1 : 0));
  }
  fake_services<real_types> provider;
  provider.malformed = true;
  auto vm            = command_vm::create();
  REQUIRE(vm.has_value());
  auto r = vm->invoke(command_init_options{"private-ledger.db", "example"}, provider);
  REQUIRE_FALSE(r.has_value());
  CHECK(r.error().code == "internal");
}
TEST_CASE("command native service failure wins over suppression retry and manufactured success", "[commands]") {
  for (std::string mode : {"suppress", "poison"}) {
    fake_services<altered_types> provider;
    provider.fail = true;
    auto vm       = altered::command_vm::create();
    REQUIRE(vm.has_value());
    auto r = vm->invoke(altered::command_init_options{"private-ledger.db", mode}, provider);
    REQUIRE(r.has_value());
    CHECK_FALSE((*r)->fields.at("ok")->boolean);
    CHECK((*r)->fields.at("error")->fields.at("message")->string == "Exact native failure");
    CHECK(provider.calls == 1);
  }
}
TEST_CASE("command caught execution depth stack and instruction exhaustion are terminal", "[commands]") {
  for (std::string mode : {"depth", "stack", "multi_stack", "catch_instruction", "cumulative"}) {
    INFO(mode);
    fake_services<altered_types> provider;
    auto                         vm = altered::command_vm::create();
    REQUIRE(vm.has_value());
    auto r = vm->invoke(altered::command_init_options{"private-ledger.db", mode}, provider);
    REQUIRE_FALSE(r.has_value());
    CHECK(r.error().code == "resource_limit");
    CHECK(r.error().limit == (mode == "depth"                              ? "depth"
                              : (mode == "stack" || mode == "multi_stack") ? "stack"
                              : mode == "cumulative"                       ? "native_bytes"
                                                                           : "instructions"));
    CHECK(provider.calls == 0);
  }
}
TEST_CASE("command typed native invocation rejects structural option invariants before services", "[commands]") {
  for (int mode = 0; mode < 7; ++mode) {
    command_history_options o;
    o.ledger = "private-ledger.db";
    if (mode == 0)
      o.revision = 0;
    if (mode == 1) {
      o.revision = 1;
      o.at_event = 1;
    }
    if (mode == 2) {
      o.revision = 1;
      o.limit    = 2;
    }
    if (mode == 3)
      o.limit = 101;
    if (mode == 4)
      o.request_id = "bad key";
    if (mode == 5)
      o.after_event = -1;
    if (mode == 6)
      o.ledger = "--bad";
    fake_services<real_types> p;
    auto                      vm = command_vm::create();
    REQUIRE(vm.has_value());
    CHECK_FALSE(vm->invoke(o, p).has_value());
    CHECK(p.calls == 0);
  }
}
TEST_CASE("command opaque path slots reject forged wrong-type reused extra-field and out-of-order calls", "[commands]") {
  for (std::string mode : {"forged", "wrongslot", "reused", "extra", "order"}) {
    INFO(mode);
    fake_services<altered_types> provider;
    provider.workspace = record();
    auto vm            = altered::command_vm::create();
    REQUIRE(vm.has_value());
    altered::command_render_options o;
    o.workspace = "private-workspace.lua";
    o.registry  = "private-registry.json";
    o.root      = "quota=private-root";
    o.job       = mode;
    o.role      = "worker";
    auto r      = vm->invoke(o, provider);
    REQUIRE_FALSE(r.has_value());
    CHECK(r.error().code == "internal");
    CHECK(provider.calls == (mode == "reused" ? 1 : 0));
  }
}
TEST_CASE("command execution caught live allocation exhaustion stays terminal", "[commands]") {
  fake_services<altered_types> provider;
  auto                         vm = altered::command_vm::create();
  REQUIRE(vm.has_value());
  auto r = vm->invoke(altered::command_init_options{"private-ledger.db", "catch_allocation"}, provider);
  REQUIRE_FALSE(r.has_value());
  CHECK(r.error().code == "resource_limit");
  CHECK(provider.calls == 0);
}
TEST_CASE("command initialization semantic policy refuses invalid workspace IDs before publication", "[commands]") {
  for (std::string id : std::vector<std::string>{"Invalid", "with-dash", "", std::string(65, 'a')}) {
    fake_services<real_types> p;
    auto                      vm = command_vm::create();
    REQUIRE(vm.has_value());
    auto r = vm->invoke(command_init_options{"private-ledger.db", id}, p);
    if (id.empty())
      CHECK_FALSE(r.has_value());
    else {
      REQUIRE(r.has_value());
      CHECK_FALSE((*r)->fields.at("ok")->boolean);
      CHECK((*r)->fields.at("error")->fields.at("code")->string == "schema");
    }
    CHECK(p.calls == 0);
  }
}
TEST_CASE("command export publication state survives invalid projection and caught exhaustion", "[commands]") {
  for (std::int64_t id : {11, 12, 13}) {
    fake_services<altered_types> provider;
    auto                         vm = altered::command_vm::create();
    REQUIRE(vm.has_value());
    auto r = vm->invoke(altered::command_export_options{"private-ledger.db", "private-target", id}, provider);
    REQUIRE_FALSE(r.has_value());
    CHECK(vm->export_may_be_complete() == (id != 13));
    CHECK(provider.calls == (id == 13 ? 0 : 1));
  }
}

namespace {
std::string read_bytes(const std::filesystem::path& path) {
  std::ifstream in(path, std::ios::binary);
  REQUIRE(in.good());
  return {std::istreambuf_iterator<char>(in), {}};
}
struct render_fixture {
  std::filesystem::path root;
  value_ptr             workspace, bundle;
  render_fixture() {
    char pattern[] = "/tmp/planar-command-XXXXXX";
    REQUIRE(mkdtemp(pattern));
    root = std::filesystem::canonical(pattern);
    std::filesystem::copy(std::filesystem::path(PLANAR_SCHEMA_FIXTURE) / "workspace", root / "workspace",
                          std::filesystem::copy_options::recursive);
    capture_session capture;
    REQUIRE(capture.add_root("workspace", (root / "workspace").string()));
    auto loaded = capture.load("workspace.lua");
    REQUIRE(loaded.has_value());
    workspace   = loaded->root;
    auto routed = run_trusted("route_workspace", record({{"file", text("workspace.lua")}, {"workspace", workspace}}));
    REQUIRE(routed.has_value());
    auto route = (*routed)->fields.at("value");
    auto spec  = capture.load(route->fields.at("spec")->string);
    REQUIRE(spec.has_value());
    std::vector<value_ptr> jobs;
    for (auto& j : route->fields.at("jobs")->items) {
      auto job = capture.load(j->fields.at("file")->string);
      REQUIRE(job.has_value());
      jobs.push_back(record({{"file", j->fields.at("file")}, {"value", job->root}}));
    }
    auto snapshot = capture.finish();
    REQUIRE(snapshot.has_value());
    std::vector<value_ptr> files;
    for (auto& f : **snapshot)
      files.push_back(record({{"path", text(f.path)}, {"bytes", text(f.bytes)}}));
    bundle = record(
        {{"workspace", record({{"file", text("workspace.lua")}, {"value", workspace}})},
         {"spec", record({{"file", route->fields.at("spec")}, {"value", spec->root}})},
         {"jobs", array(jobs)},
         {"registry",
          record({{"file", text("input.json")},
                  {"bytes", text(read_bytes(std::filesystem::path(PLANAR_SCHEMA_FIXTURE) / "registry/quota-tools.json"))}})},
         {"files", array(files)}});
  }
  ~render_fixture() {
    std::filesystem::remove_all(root);
  }
};
} // namespace
TEST_CASE("command retained render runs real routing validation selection and projection with unchanged fixed export result",
          "[commands]") {
  render_fixture            f;
  fake_services<real_types> p;
  p.workspace = f.workspace;
  std::vector<value_ptr> retained;
  for (auto& entry : f.bundle->fields.at("files")->items)
    retained.push_back(record({{"root", text("workspace")},
                               {"path", entry->fields.at("path")},
                               {"mode", text("0644")},
                               {"bytes", entry->fields.at("bytes")}}));
  retained.push_back(record({{"root", text("registry")},
                             {"path", text("input.json")},
                             {"mode", text("0644")},
                             {"bytes", f.bundle->fields.at("registry")->fields.at("bytes")}}));
  p.render_input = record({{"bundle", f.bundle},
                           {"capture", record({{"status", text("not_captured")}})},
                           {"files", array(retained)},
                           {"job", text("combined")},
                           {"role", text("builder")}});
  auto baseline  = run_trusted("render", p.render_input);
  REQUIRE(baseline.has_value());
  REQUIRE((*baseline)->fields.contains("value"));
  auto vm = command_vm::create();
  REQUIRE(vm.has_value());
  command_render_options o;
  o.workspace = "private-workspace.lua";
  o.registry  = "private-registry.json";
  o.root      = "source=private-root";
  o.job       = "combined";
  o.role      = "builder";
  auto actual = vm->invoke(o, p);
  INFO((actual ? "success" : actual.error().message));
  INFO(p.calls);
  REQUIRE(actual.has_value());
  REQUIRE((*actual)->fields.at("ok")->boolean);
  CHECK(p.calls == 2);
  auto expected_json = codec_encode((*baseline)->fields.at("value")), actual_json = codec_encode((*actual)->fields.at("value"));
  REQUIRE(expected_json.has_value());
  REQUIRE(actual_json.has_value());
  CHECK(*actual_json == *expected_json);
}

TEST_CASE("command registration and execution transfers share one cumulative conversion budget", "[commands]") {
  // Discover a successful execution boundary using ordinary catalog help, then
  // repeat those exact transfers with larger valid retained catalog descriptors.
  // Neither registration alone nor that execution alone exhausts the native bound.
  int lower = 0, upper = 20000;
  while (lower + 1 < upper) {
    int  middle = lower + (upper - lower) / 2;
    auto vm     = altered::command_vm::create();
    REQUIRE(vm.has_value());
    fake_services<altered_types> p;
    auto r = vm->invoke(altered::command_init_options{"private-ledger.db", "combined" + std::to_string(middle)}, p);
    if (r) {
      CHECK_FALSE((*r)->fields.at("ok")->boolean);
      lower = middle;
    } else {
      REQUIRE(r.error().code == "resource_limit");
      REQUIRE(r.error().limit == "native_bytes");
      upper = middle;
    }
    CHECK(p.calls == 0);
  }
  REQUIRE(lower > 0);
  auto vm = command_test_17::command_vm::create();
  REQUIRE(vm.has_value());
  fake_services<heavy_types> p;
  auto r = vm->invoke(command_test_17::command_init_options{"private-ledger.db", "combined" + std::to_string(lower)}, p);
  REQUIRE_FALSE(r.has_value());
  CHECK(r.error().code == "resource_limit");
  CHECK(r.error().limit == "native_bytes");
  CHECK(p.calls == 0);
}

TEST_CASE("command captured direct nested transitive and table key state refuses writes", "[commands][c01]") {
  for (std::string mode : {"direct", "new", "nested", "key", "helper"}) {
    INFO(mode);
    auto vm = command_test_18::command_vm::create();
    REQUIRE(vm.has_value());
    using types = closure_types;
    fake_services<types> provider;
    auto                 r = vm->invoke(command_test_18::command_init_options{"private-ledger.db", mode}, provider);
    REQUIRE_FALSE(r.has_value());
    CHECK(r.error().code == "internal");
    CHECK(provider.calls == 0);
  }
}
TEST_CASE("command frozen shared cyclic and callable keys preserve reads and local scratch", "[commands][c01]") {
  auto vm = command_test_18::command_vm::create();
  REQUIRE(vm.has_value());
  using types = closure_types;
  fake_services<types> provider;
  auto                 r = vm->invoke(command_test_18::command_init_options{"private-ledger.db", "reads"}, provider);
  REQUIRE(r.has_value());
  CHECK(provider.calls == 1);
}
TEST_CASE("command native sealing accepts depth boundary and refuses next node with typed resource", "[commands][c01]") {
  auto at = command_test_19::command_vm::create();
  REQUIRE(at.has_value());
  auto over = command_test_20::command_vm::create();
  REQUIRE_FALSE(over.has_value());
  CHECK(over.error().code == "resource_limit");
  CHECK(over.error().limit == "depth");
  CHECK(over.error().maximum == 256);
}

TEST_CASE("command frozen returned module table converts private backing without changing its tag", "[commands][c01]") {
  auto vm = command_test_18::command_vm::create();
  REQUIRE(vm.has_value());
  fake_services<closure_types> provider;
  auto                         r = vm->invoke(command_test_18::command_init_options{"private-ledger.db", "return"}, provider);
  REQUIRE(r.has_value());
  CHECK_FALSE((*r)->fields.at("ok")->boolean);
  CHECK((*r)->fields.at("error")->fields.at("message")->string == "retained immutable refusal");
  CHECK(provider.calls == 0);
}

import planar.engine.load_test_0;
import planar.engine.load_c01_test;
import planar.engine.load_test_1;
import planar.engine.load_test_2;
import planar.engine.load_test_3;
import planar.engine.load_test_4;
import planar.engine.load_test_5;
import planar.engine.acceptance.load;
namespace {
struct load_types {
  using services = load_test_0::command_services;
  using render   = load_test_0::command_render_options;
  using init     = load_test_0::command_init_options;
  using history  = load_test_0::command_history_options;
  using source   = load_test_0::command_export_options;
  using selector = load_test_0::command_history_selector;
  using result   = load_test_0::command_service_result;
  using load     = load_test_0::command_load_options;
  using vm       = load_test_0::command_vm;
};
struct changed_load_types {
  using services = load_test_1::command_services;
  using render   = load_test_1::command_render_options;
  using init     = load_test_1::command_init_options;
  using history  = load_test_1::command_history_options;
  using source   = load_test_1::command_export_options;
  using selector = load_test_1::command_history_selector;
  using result   = load_test_1::command_service_result;
  using load     = load_test_1::command_load_options;
  using vm       = load_test_1::command_vm;
};
struct provenance_defect_types {
  using services = load_c01_test::command_services;
  using render   = load_c01_test::command_render_options;
  using init     = load_c01_test::command_init_options;
  using history  = load_c01_test::command_history_options;
  using source   = load_c01_test::command_export_options;
  using selector = load_c01_test::command_history_selector;
  using result   = load_c01_test::command_service_result;
  using load     = load_c01_test::command_load_options;
  using vm       = load_c01_test::command_vm;
};
struct load_fixture {
  std::filesystem::path root, workspace, registry;
  load_fixture() {
    std::string p = (std::filesystem::temp_directory_path() / "planar-load-vm-XXXXXX").string();
    REQUIRE(mkdtemp(p.data()));
    root = std::filesystem::canonical(p);
    std::filesystem::copy(std::filesystem::path(PLANAR_LOAD_FIXTURE) / "workspace", root / "workspace",
                          std::filesystem::copy_options::recursive);
    workspace = root / "workspace/workspace.lua";
    registry  = root / "registry.json";
    std::filesystem::copy_file(std::filesystem::path(PLANAR_LOAD_FIXTURE) / "registry/quota-tools.json", registry);
  }
  ~load_fixture() {
    std::filesystem::remove_all(root);
  }
  template <class Types> typename Types::load options(std::string mode = "normal") {
    return {(root / "ledger.db").string(), workspace.string(), registry.string(), "none", mode, "driver", {}};
  }
};
// Real D5 capture/preparation; completion is a finite test provider, not a ledger transaction claim.
template <class Types> struct load_services : fake_services<Types> {
  using result = typename Types::result;
  using load   = typename Types::load;
  using fake_services<Types>::open_workspace;
  std::optional<acceptance::load_capture>            capture;
  std::shared_ptr<const acceptance::acceptance_data> prepared;
  std::vector<std::string>                           order;
  std::string                                        command_bytes;
  history::record_digest                             digest{};
  bool                     fail_open = false, malformed_capture = false, malformed_success = false, throw_after = false;
  ledger::completion_phase phase = ledger::completion_phase::committed;
  result                   open_workspace(const load& o) override {
    order.push_back("open");
    if (fail_open)
      return {{}, error_value()};
    auto opened = acceptance::load_capture::open_workspace(o.workspace, o.registry);
    REQUIRE(opened);
    capture.emplace(std::move(*opened));
    auto w = capture->workspace();
    REQUIRE(w);
    return {*w, {}};
  }
  result capture_load(const load&, const value_ptr& route) override {
    order.push_back("capture");
    auto b = capture->capture(route);
    REQUIRE(b);
    if (malformed_capture)
      return {record({{"wrong", *b}}), {}};
    return {record({{"bundle", *b}}), {}};
  }
  result accept_load(const load& o, const value_ptr& b, const value_ptr& m, std::span<const history::tree_file> provenance,
                     ledger::completion_receipt& completion) override {
    order.push_back("accept");
    auto verified = history::verify_stdlib(provenance);
    REQUIRE(verified);
    digest = verified->digest;
    for (auto& f : provenance)
      if (f.path == "planar/commands.lua")
        command_bytes = f.payload;
    auto p = capture->prepare(b, m, provenance);
    if (!p)
      return {{}, error_value()};
    auto d = p->data();
    REQUIRE(d);
    prepared = *d;
    REQUIRE(p->verify());
    auto envelope = record({{"version", number(1)},
                            {"kind", text("load")},
                            {"ledger", text(std::string(32, 'a'))},
                            {"request_id", text(o.request_id)},
                            {"outcome", text("accepted")},
                            {"revision", text("1")},
                            {"parent", text("none")},
                            {"event", text("2")}});
    auto saved    = codec_encode(envelope);
    REQUIRE(saved);
    completion.saved_result = std::move(*saved);
    completion.phase        = phase;
    if (throw_after)
      throw std::runtime_error("after completion");
    if (malformed_success)
      return {record(), {}};
    return {envelope, {}};
  }
};
std::string load_bytes(const std::filesystem::path& p) {
  std::ifstream f(p, std::ios::binary);
  REQUIRE(f.good());
  return {std::istreambuf_iterator<char>(f), {}};
}
} // namespace
TEST_CASE("command load private catalog routes and interprets in retained VM with exact provenance", "[commands][load]") {
  load_fixture              f;
  load_services<load_types> provider;
  auto                      vm = load_types::vm::create();
  REQUIRE(vm);
  CHECK(vm->catalog()->fields.at("commands")->items.size() == 17);
  auto r = vm->invoke(f.options<load_types>(), provider);
  REQUIRE(r);
  CHECK((*r)->fields.at("ok")->boolean);
  CHECK(provider.order == std::vector<std::string>{"open", "capture", "accept"});
  REQUIRE(provider.prepared);
  CHECK(provider.prepared->source.size() == 6);
  CHECK(provider.prepared->interpretation.interpretation->fields.at("bundle")->fields.at("jobs")->items.size() == 2);
  CHECK(provider.command_bytes == load_bytes(std::filesystem::path(PLANAR_LOAD_GENERATED) / "commands_0.lua"));
  CHECK(vm->load_may_be_complete());
  CHECK_FALSE(vm->invoke(f.options<load_types>(), provider));
  auto production = command_vm::create();
  REQUIRE(production);
  fake_services<real_types> ordinary;
  CHECK_FALSE(production->invoke(command_load_options{"ledger", "workspace", "registry", "none", "id", "actor", {}}, ordinary));
  CHECK(ordinary.calls == 0);
}
TEST_CASE("command load altered retained callable carries its own original bytes and digest", "[commands][load]") {
  load_fixture              f;
  load_services<load_types> normal;
  auto                      a = load_types::vm::create();
  REQUIRE(a);
  REQUIRE(a->invoke(f.options<load_types>(), normal));
  load_services<changed_load_types> changed;
  auto                              b = changed_load_types::vm::create();
  REQUIRE(b);
  auto r = b->invoke(f.options<changed_load_types>(), changed);
  REQUIRE(r);
  REQUIRE((*r)->fields.at("ok")->boolean);
  REQUIRE(changed.prepared);
  CHECK(changed.prepared->interpretation.interpretation->fields.at("bundle")
            ->fields.at("spec")
            ->fields.at("goal")
            ->fields.at("bytes")
            ->string == "D6 altered retained policy");
  CHECK(changed.command_bytes == load_bytes(std::filesystem::path(PLANAR_LOAD_GENERATED) / "commands_1.lua"));
  CHECK(changed.digest != normal.digest);
  CHECK(b->seal() != a->seal());
}
TEST_CASE("command load refuses forged family order skipped duplicate and malformed arguments", "[commands][load]") {
  load_fixture f;
  for (auto mode : {"forged", "wrongslot", "cross", "order", "skipped", "double", "badargs"}) {
    INFO(mode);
    auto vm = load_types::vm::create();
    REQUIRE(vm);
    load_services<load_types> p;
    auto                      r = vm->invoke(f.options<load_types>(mode), p);
    CHECK_FALSE(r);
    CHECK_FALSE(vm->load_may_be_complete());
    CHECK(std::ranges::find(p.order, "accept") == p.order.end());
  }
}
TEST_CASE("command load native failure poisons caught subsequent services and wins", "[commands][load]") {
  load_fixture f;
  auto         vm = load_types::vm::create();
  REQUIRE(vm);
  load_services<load_types> p;
  p.fail_open = true;
  auto r      = vm->invoke(f.options<load_types>("poison"), p);
  REQUIRE(r);
  CHECK_FALSE((*r)->fields.at("ok")->boolean);
  CHECK((*r)->fields.at("error")->fields.at("message")->string == "Exact native failure");
  CHECK(p.order == std::vector<std::string>{"open"});
  CHECK_FALSE(vm->load_may_be_complete());
}
TEST_CASE("command load malformed bundle semantic and captured result fail before completion", "[commands][load]") {
  load_fixture f;
  for (auto mode : {"badbundle", "badsemantic", "capture"}) {
    INFO(mode);
    auto vm = load_types::vm::create();
    REQUIRE(vm);
    load_services<load_types> p;
    p.malformed_capture = std::string_view(mode) == "capture";
    auto r              = vm->invoke(f.options<load_types>(mode), p);
    if (r)
      CHECK_FALSE((*r)->fields.at("ok")->boolean);
    CHECK_FALSE(vm->load_may_be_complete());
    CHECK_FALSE(p.prepared);
  }
}
TEST_CASE("command load retains completion through altered malformed caught and throwing returns", "[commands][load]") {
  load_fixture f;
  for (auto mode : {"posterror", "postdouble", "postchanged", "postfalse", "postmalformed", "provider", "throw"}) {
    INFO(mode);
    auto vm = load_types::vm::create();
    REQUIRE(vm);
    load_services<load_types> p;
    p.malformed_success = std::string_view(mode) == "provider";
    p.throw_after       = std::string_view(mode) == "throw";
    auto r              = vm->invoke(f.options<load_types>(mode), p);
    CHECK_FALSE(r);
    CHECK(vm->load_may_be_complete());
    auto moved = std::move(*vm);
    CHECK(moved.load_may_be_complete());
    CHECK_FALSE(vm->load_may_be_complete());
  }
}
TEST_CASE("command load caught resource exhaustion is terminal before and after native completion", "[commands][load]") {
  load_fixture f;
  for (auto mode : {"memory", "instructions", "cumulative", "postmemory", "postinstructions", "postcumulative"}) {
    INFO(mode);
    auto vm = load_types::vm::create();
    REQUIRE(vm);
    load_services<load_types> p;
    auto                      r = vm->invoke(f.options<load_types>(mode), p);
    REQUIRE_FALSE(r);
    CHECK(r.error().code == "resource_limit");
    if (std::string_view(mode).ends_with("cumulative"))
      CHECK(r.error().limit == "native_bytes");
    CHECK(vm->load_may_be_complete() == std::string_view(mode).starts_with("post"));
  }
}
TEST_CASE("command load validates retained token head request and actor before services", "[commands][load]") {
  load_fixture f;
  for (int i = 0; i < 7; ++i) {
    auto o = f.options<load_types>();
    if (i == 0)
      o.expect_revision = "01";
    if (i == 1)
      o.expected_head = 1;
    if (i == 2) {
      o.expect_revision = "0";
      o.expected_head   = 0;
    }
    if (i == 3)
      o.request_id = "bad key";
    if (i == 4)
      o.actor = std::string("a\0b", 3);
    if (i == 5)
      o.actor = std::string(257, 'a');
    if (i == 6)
      o.actor = "\xff";
    auto vm = load_types::vm::create();
    REQUIRE(vm);
    load_services<load_types> p;
    CHECK_FALSE(vm->invoke(o, p));
    CHECK(p.order.empty());
  }
}

TEST_CASE("command load private admission refuses missing duplicate capability and signature variants", "[commands][load]") {
  CHECK_FALSE(load_test_2::command_vm::create());
  CHECK_FALSE(load_test_3::command_vm::create());
  CHECK_FALSE(load_test_4::command_vm::create());
  CHECK_FALSE(load_test_5::command_vm::create());
  load_fixture f;
  auto         vm = load_types::vm::create();
  REQUIRE(vm);
  fake_services<load_types> p;
  CHECK_FALSE(vm->invoke(f.options<load_types>(), p));
  CHECK_FALSE(vm->load_may_be_complete());
}
TEST_CASE("command load retains exact native receipt for replay uncertain and ordinary completion", "[commands][load]") {
  load_fixture f;
  for (auto phase :
       {ledger::completion_phase::committed, ledger::completion_phase::replayed, ledger::completion_phase::commit_uncertain}) {
    auto vm = load_types::vm::create();
    REQUIRE(vm);
    load_services<load_types> p;
    p.phase = phase;
    auto r  = vm->invoke(f.options<load_types>(), p);
    if (phase == ledger::completion_phase::commit_uncertain)
      CHECK_FALSE(r);
    else
      REQUIRE(r);
    CHECK(vm->load_may_be_complete());
    auto receipt = vm->load_completion();
    REQUIRE(receipt);
    CHECK(receipt->phase == phase);
    CHECK_FALSE(receipt->saved_result.empty());
    if (r) {
      auto encoded = codec_encode((*r)->fields.at("value"));
      REQUIRE(encoded);
      CHECK(*encoded == receipt->saved_result);
    }
  }
}

TEST_CASE("command load defers provenance refusal until fresh acceptance", "[commands][load][d6c01]") {
  using types = provenance_defect_types;
  SECTION("registration and ordinary invocation bypass acceptance provenance") {
    auto vm = types::vm::create();
    REQUIRE(vm);
    CHECK(vm->catalog()->fields.at("commands")->items.size() == 17);
    fake_services<types> provider;
    auto                 result = vm->invoke(types::init{"private-ledger.db", "ordinary"}, provider);
    REQUIRE(result);
    CHECK((*result)->fields.at("ok")->boolean);
    CHECK(provider.calls == 1);
  }
  SECTION("fresh load captures but refuses before native acceptance") {
    load_fixture f;
    auto         vm = types::vm::create();
    REQUIRE(vm);
    load_services<types> provider;
    auto                 result = vm->invoke(f.options<types>(), provider);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "internal");
    CHECK(provider.order == std::vector<std::string>{"open", "capture"});
    CHECK_FALSE(provider.prepared);
    CHECK_FALSE(vm->load_may_be_complete());
    REQUIRE(vm->load_completion());
    CHECK(vm->load_completion()->phase == ledger::completion_phase::not_completed);
  }
}

namespace {
struct binding_load_types : real_types {
  using load = command_load_options;
  using vm   = command_vm;
};
template <class Base, class Activate, class Status, class Check> struct binding_types : Base {
  using activate = Activate;
  using status   = Status;
  using check    = Check;
};
using original_binding_types =
    binding_types<binding_load_types, command_activate_options, command_status_options, command_check_options>;
using changed_binding_types = binding_types<changed_load_types, load_test_1::command_activate_options,
                                            load_test_1::command_status_options, load_test_1::command_check_options>;
// The load provider receives verified accepted provenance through a separate service boundary.
// It is the oracle for execution identity, rather than a copy of the invocation's digest implementation.
template <class Types> std::string accepted_binding_digest() {
  load_fixture         fixture;
  load_services<Types> provider;
  auto                 vm = Types::vm::create();
  REQUIRE(vm);
  auto result = vm->invoke(fixture.options<Types>(), provider);
  REQUIRE(result);
  REQUIRE((*result)->fields.at("ok")->boolean);
  REQUIRE(provider.prepared);
  std::string digest;
  for (auto byte : provider.digest)
    digest += std::format("{:02x}", byte);
  return digest;
}
template <class Types> struct binding_services : fake_services<Types> {
  using result = typename Types::result;
  using fake_services<Types>::read_execution;
  std::vector<std::string> order;
  std::string              digest;
  void                     bind_execution_stdlib(std::string_view value) override {
    order.push_back("bind");
    digest = value;
  }
  result read_execution(const typename Types::activate&) override {
    order.push_back("read");
    return {{}, error_value()};
  }
  result read_execution(const typename Types::status&) override {
    order.push_back("read");
    return {{}, error_value()};
  }
  result check_step(const typename Types::check&, std::string_view operation, const value_ptr&) override {
    CHECK(operation == "read_execution");
    order.push_back("read");
    return {{}, error_value()};
  }
};
template <class Types> void check_bound_digest(std::string_view expected) {
  auto vm = Types::vm::create();
  REQUIRE(vm);
  binding_services<Types>  provider;
  typename Types::activate options{"private-ledger.db", "job", "1", "NONE", "activate", "driver"};
  auto                     result = vm->invoke(options, provider);
  REQUIRE(result);
  REQUIRE_FALSE((*result)->fields.at("ok")->boolean);
  CHECK((*result)->fields.at("error")->fields.at("code")->string == "ledger_busy");
  CHECK(provider.order == std::vector<std::string>{"bind", "read"});
  CHECK(provider.digest == expected);
}
} // namespace

TEST_CASE("command execution binds its own accepted stdlib identity before native reads", "[commands][execution_binding]") {
  const auto original = accepted_binding_digest<original_binding_types>();
  const auto changed  = accepted_binding_digest<changed_binding_types>();
  REQUIRE(original != changed);
  check_bound_digest<original_binding_types>(original);
  check_bound_digest<changed_binding_types>(changed);
}

TEST_CASE("command execution leaves interpretation-exempt reads unbound", "[commands][execution_binding]") {
  SECTION("bare status and request lookup intent") {
    for (const auto request : {false, true}) {
      auto vm = command_vm::create();
      REQUIRE(vm);
      binding_services<original_binding_types> provider;
      command_status_options                   options;
      options.ledger = "private-ledger.db";
      if (request)
        options.request_id = "saved-request";
      auto result = vm->invoke(options, provider);
      REQUIRE(result);
      REQUIRE_FALSE((*result)->fields.at("ok")->boolean);
      CHECK(provider.order == std::vector<std::string>{"read"});
      CHECK(provider.digest.empty());
    }
  }
  SECTION("bound role status still binds") {
    auto vm = command_vm::create();
    REQUIRE(vm);
    binding_services<original_binding_types> provider;
    command_status_options                   options;
    options.ledger = "private-ledger.db";
    options.role   = "worker";
    auto result    = vm->invoke(options, provider);
    REQUIRE(result);
    REQUIRE_FALSE((*result)->fields.at("ok")->boolean);
    CHECK(provider.order == std::vector<std::string>{"bind", "read"});
    CHECK(provider.digest == accepted_binding_digest<original_binding_types>());
  }
  SECTION("stopped-check recovery intent") {
    auto vm = command_vm::create();
    REQUIRE(vm);
    binding_services<original_binding_types> provider;
    command_check_options                    options;
    options.operation       = "check-recover";
    options.ledger          = "private-ledger.db";
    options.run_id          = "1";
    options.reservation     = "1";
    options.confirm_stopped = "true";
    options.request_id      = "recover-check";
    options.actor           = "driver";
    auto result             = vm->invoke(options, provider);
    REQUIRE(result);
    REQUIRE_FALSE((*result)->fields.at("ok")->boolean);
    CHECK(provider.order == std::vector<std::string>{"read"});
    CHECK(provider.digest.empty());
  }
}

TEST_CASE("command execution refuses moved and consumed owners before binding", "[commands][execution_binding]") {
  auto created = command_vm::create();
  REQUIRE(created);
  command_vm                               owner = std::move(*created);
  binding_services<original_binding_types> provider;
  command_activate_options                 options{"private-ledger.db", "job", "1", "NONE", "activate", "driver"};
  auto                                     moved_result = created->invoke(options, provider);
  REQUIRE_FALSE(moved_result);
  CHECK(provider.order.empty());
  auto first = owner.invoke(options, provider);
  REQUIRE(first);
  REQUIRE_FALSE((*first)->fields.at("ok")->boolean);
  REQUIRE(provider.order == std::vector<std::string>{"bind", "read"});
  auto consumed_result = owner.invoke(options, provider);
  REQUIRE_FALSE(consumed_result);
  CHECK(provider.order == std::vector<std::string>{"bind", "read"});
}

import planar.engine.binding_budget_test;
namespace {
struct binding_budget_types {
  using services = binding_budget_test::command_services;
  using render   = binding_budget_test::command_render_options;
  using init     = binding_budget_test::command_init_options;
  using history  = binding_budget_test::command_history_options;
  using source   = binding_budget_test::command_export_options;
  using selector = binding_budget_test::command_history_selector;
  using result   = binding_budget_test::command_service_result;
  using load     = binding_budget_test::command_load_options;
  using vm       = binding_budget_test::command_vm;
  using activate = binding_budget_test::command_activate_options;
  using status   = binding_budget_test::command_status_options;
  using check    = binding_budget_test::command_check_options;
};
constexpr std::size_t binding_budget_ceiling = 512 * 1024 * 1024;
constexpr std::size_t binding_budget_upper   = 64 * 1024 * 1024;
struct binding_budget_corpus {
  std::size_t payload = 0;
  std::string digest;
};
binding_budget_corpus accepted_budget_corpus() {
  load_fixture                        fixture;
  load_services<binding_budget_types> provider;
  auto                                vm = binding_budget_types::vm::create();
  REQUIRE(vm);
  auto loaded = vm->invoke(fixture.options<binding_budget_types>(), provider);
  REQUIRE(loaded);
  REQUIRE((*loaded)->fields.at("ok")->boolean);
  REQUIRE(provider.prepared);
  binding_budget_corpus result;
  bool                  assembly = false, manifest = false;
  for (const auto& file : provider.prepared->stdlib.files) {
    if (file.path == "__planar__/assembly.canonical") {
      manifest = true;
      continue;
    }
    assembly |= file.path == "__planar__/assembly.lua";
    result.payload += file.payload.size();
  }
  REQUIRE(assembly);
  REQUIRE(manifest);
  // P is independently observed accepted original+assembly payload, not a copy of the binder's reservation expression.
  REQUIRE(result.payload > 65536);
  REQUIRE(result.payload < binding_budget_upper / 3);
  for (auto byte : provider.digest)
    result.digest += std::format("{:02x}", byte);
  return result;
}
std::string binding_budget_id(std::size_t remaining) {
  auto id = std::format("{:012}", remaining);
  REQUIRE(id.size() == 12);
  return id;
}
struct binding_budget_observation {
  bool                      native_failure = false;
  std::optional<diagnostic> refusal;
  std::vector<std::string>  order;
  std::string               digest;
};
void require_native_budget_refusal(const diagnostic& error) {
  CHECK(error.code == "resource_limit");
  CHECK(error.limit == "native_bytes");
  CHECK(error.maximum == binding_budget_ceiling);
}
binding_budget_observation observe_binding_room(bool bound, std::size_t remaining) {
  auto vm = binding_budget_types::vm::create();
  REQUIRE(vm);
  binding_services<binding_budget_types> provider;
  binding_budget_types::activate activate{"private-ledger.db", "job", "1", "NONE", binding_budget_id(remaining), "driver"};
  binding_budget_types::status   status;
  status.ledger                     = "private-ledger.db";
  status.request_id                 = binding_budget_id(remaining);
  auto                       result = bound ? vm->invoke(activate, provider) : vm->invoke(status, provider);
  binding_budget_observation observation;
  observation.order  = provider.order;
  observation.digest = provider.digest;
  if (!result) {
    require_native_budget_refusal(result.error());
    observation.refusal = result.error();
    return observation;
  }
  REQUIRE_FALSE((*result)->fields.at("ok")->boolean);
  REQUIRE((*result)->fields.at("error")->fields.at("code")->string == "ledger_busy");
  REQUIRE(provider.order == (bound ? std::vector<std::string>{"bind", "read"} : std::vector<std::string>{"read"}));
  if (!bound)
    REQUIRE(provider.digest.empty());
  observation.native_failure = true;
  return observation;
}
std::size_t minimum_binding_room(bool bound, const binding_budget_corpus& corpus) {
  const auto zero = observe_binding_room(bound, 0);
  REQUIRE_FALSE(zero.native_failure);
  REQUIRE(zero.refusal);
  REQUIRE(zero.order.empty());
  const auto generous = observe_binding_room(bound, binding_budget_upper);
  REQUIRE(generous.native_failure);
  if (bound)
    REQUIRE(generous.digest == corpus.digest);
  std::size_t rejected = 0, accepted = binding_budget_upper;
  while (accepted - rejected > 1) {
    const auto middle = rejected + (accepted - rejected) / 2;
    if (observe_binding_room(bound, middle).native_failure)
      accepted = middle;
    else
      rejected = middle;
  }
  REQUIRE_FALSE(observe_binding_room(bound, accepted - 1).native_failure);
  REQUIRE(observe_binding_room(bound, accepted).native_failure);
  return accepted;
}
} // namespace

TEST_CASE("command embedded binding native budget accounts for cumulative payload transfers",
          "[commands][binding_native_budget]") {
  const auto corpus = accepted_budget_corpus();
  // Both shipped handlers first read execution and receive the same native error. After the fixture insertion,
  // activation validates additional nonempty options, so its non-binder side cost cannot be less than bare status's.
  // Fixed-width request IDs keep validation sizes unchanged while room varies; preparation/registration precede it.
  const auto exempt = minimum_binding_room(false, corpus);
  const auto bound  = minimum_binding_room(true, corpus);
  std::println("R19 binding budget P={} exempt_min={} bound_min={} gap={} minimum_payload_transfers={}", corpus.payload, exempt,
               bound, bound >= exempt ? bound - exempt : 0, 3 * corpus.payload);
  REQUIRE(bound >= exempt);
  CHECK(bound - exempt >= 3 * corpus.payload);
}

TEST_CASE("command embedded binding native budget refuses insufficient room before callbacks and stays terminal",
          "[commands][binding_native_budget]") {
  const auto corpus   = accepted_budget_corpus();
  const auto generous = observe_binding_room(true, binding_budget_upper);
  REQUIRE(generous.native_failure);
  REQUIRE(generous.digest == corpus.digest);
  auto vm = binding_budget_types::vm::create();
  REQUIRE(vm);
  binding_services<binding_budget_types> provider;
  binding_budget_types::activate         options{
      "private-ledger.db", "job", "1", "NONE", binding_budget_id(2 * corpus.payload), "driver"};
  auto result = vm->invoke(options, provider);
  INFO(std::format("P={} room={} callback_count={}", corpus.payload, 2 * corpus.payload, provider.order.size()));
  REQUIRE_FALSE(result);
  require_native_budget_refusal(result.error());
  CHECK(provider.order.empty());
  CHECK(provider.digest.empty());
  auto repeated = vm->invoke(options, provider);
  REQUIRE_FALSE(repeated);
  CHECK(repeated.error().code == "internal");
  CHECK(repeated.error().message == "Command invocation already consumed");
  CHECK(provider.order.empty());
}
