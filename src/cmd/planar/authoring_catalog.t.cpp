// Authoring bundle catalog validator tests over the generated variant runtimes.
#include <catch2/catch_test_macros.hpp>
import std;
import planar.engine.runtime;
import planar.cmd.response;
import planar.engine.authoring_variant_0;
import planar.engine.authoring_variant_1;
import planar.engine.authoring_variant_2;
import planar.engine.authoring_variant_3;
import planar.engine.authoring_variant_4;
import planar.engine.authoring_variant_5;
import planar.engine.authoring_variant_6;
import planar.engine.authoring_variant_7;
import planar.engine.authoring_variant_8;
import planar.engine.authoring_variant_9;
import planar.engine.authoring_variant_10;
import planar.engine.authoring_variant_11;
import planar.engine.authoring_variant_12;
import planar.engine.authoring_variant_13;
import planar.engine.authoring_variant_14;
import planar.engine.authoring_variant_15;
import planar.engine.authoring_variant_16;
import planar.engine.authoring_variant_17;
import planar.engine.authoring_variant_18;
import planar.engine.authoring_variant_19;
namespace {
using planar::engine::value_kind;
using planar::engine::value_ptr;

struct outcome {
  bool                       sealed = false;
  planar::engine::diagnostic error{};
  value_ptr                  catalog{};
};
template <class Expected> outcome probe(Expected vm) {
  outcome out;
  if (!vm) {
    out.error = vm.error();
    return out;
  }
  out.sealed  = true;
  out.catalog = vm->catalog();
  return out;
}
outcome variant(int n) {
  switch (n) {
  case 0:
    return probe(planar::engine::authoring_variant_0::command_vm::create());
  case 1:
    return probe(planar::engine::authoring_variant_1::command_vm::create());
  case 2:
    return probe(planar::engine::authoring_variant_2::command_vm::create());
  case 3:
    return probe(planar::engine::authoring_variant_3::command_vm::create());
  case 4:
    return probe(planar::engine::authoring_variant_4::command_vm::create());
  case 5:
    return probe(planar::engine::authoring_variant_5::command_vm::create());
  case 6:
    return probe(planar::engine::authoring_variant_6::command_vm::create());
  case 7:
    return probe(planar::engine::authoring_variant_7::command_vm::create());
  case 8:
    return probe(planar::engine::authoring_variant_8::command_vm::create());
  case 9:
    return probe(planar::engine::authoring_variant_9::command_vm::create());
  case 10:
    return probe(planar::engine::authoring_variant_10::command_vm::create());
  case 11:
    return probe(planar::engine::authoring_variant_11::command_vm::create());
  case 12:
    return probe(planar::engine::authoring_variant_12::command_vm::create());
  case 13:
    return probe(planar::engine::authoring_variant_13::command_vm::create());
  case 14:
    return probe(planar::engine::authoring_variant_14::command_vm::create());
  case 15:
    return probe(planar::engine::authoring_variant_15::command_vm::create());
  case 16:
    return probe(planar::engine::authoring_variant_16::command_vm::create());
  case 17:
    return probe(planar::engine::authoring_variant_17::command_vm::create());
  case 18:
    return probe(planar::engine::authoring_variant_18::command_vm::create());
  case 19:
    return probe(planar::engine::authoring_variant_19::command_vm::create());
  default:
    break;
  }
  return {};
}
const std::vector<std::string> eleven = {"critique", "explain-rejection", "export",       "history", "ingest",     "init",
                                         "load",     "observe",           "orchestrator", "render",  "spec-prompt"};
const std::vector<std::string> six    = {"spec-prompt", "critique", "ingest", "explain-rejection", "observe", "orchestrator"};
struct row_spec {
  std::string                                             handler, kind, capability;
  std::vector<std::tuple<std::string, std::string, bool>> options;
};
const std::map<std::string, row_spec>& ra1() {
  static const std::map<std::string, row_spec> table = {
      {"spec-prompt",
       {"shipped.spec_prompt.v1",
        "spec_prompt_v1",
        "authoring_capture_v1",
        {{"--registry", "text", true}, {"--workspace", "text", true}}}},
      {"critique",
       {"shipped.critique.v1",
        "critique_v1",
        "authoring_capture_v1",
        {{"--registry", "text", true}, {"--workspace", "text", true}}}},
      {"ingest",
       {"shipped.ingest.v1", "ingest_v1", "authoring_capture_v1", {{"--registry", "text", true}, {"--workspace", "text", true}}}},
      {"explain-rejection",
       {"shipped.explain_rejection.v1", "explain_rejection_v1", "diagnostic_read_v1", {{"--diagnostic", "text", true}}}},
      {"observe", {"shipped.observe.v1", "observe_v1", "observation_directory_v1", {{"--from", "text", true}}}},
      {"orchestrator",
       {"shipped.orchestrator.v1",
        "orchestrator_v1",
        "orchestrator_capture_v1",
        {{"--job", "text", true}, {"--registry", "text", true}, {"--workspace", "text", true}}}}};
  return table;
}
std::vector<value_ptr> commands_of(const outcome& o) {
  REQUIRE(o.catalog);
  return o.catalog->fields.at("commands")->items;
}
std::string name_of(const value_ptr& d) {
  return d->fields.at("name")->string;
}
value_ptr find_row(const std::vector<value_ptr>& rows, const std::string& name) {
  for (auto& r : rows)
    if (name_of(r) == name)
      return r;
  return {};
}
void require_control() {
  auto control = variant(0);
  REQUIRE(control.sealed);
}
void expect_refused(int n) {
  CAPTURE(n);
  auto out = variant(n);
  INFO((out.sealed ? "expected refusal, got a sealed VM" : "refused"));
  REQUIRE_FALSE(out.sealed);
  CHECK(out.error.code == "internal");
  CHECK(out.error.message == "Invalid embedded command protocol");
}
template <class A, class B> bool same_tree(const A& a, const B& b) {
  if (!a || !b)
    return !a && !b;
  if (static_cast<int>(a->kind) != static_cast<int>(b->kind))
    return false;
  if (a->string != b->string || a->integer != b->integer || a->boolean != b->boolean)
    return false;
  if (a->fields.size() != b->fields.size() || a->items.size() != b->items.size())
    return false;
  for (auto& [k, v] : a->fields) {
    auto it = b->fields.find(k);
    if (it == b->fields.end() || !same_tree(v, it->second))
      return false;
  }
  for (std::size_t i = 0; i < a->items.size(); ++i)
    if (!same_tree(a->items[i], b->items[i]))
      return false;
  return true;
}
planar::engine::profile_snapshot inspect_profile() {
  auto p           = std::make_shared<planar::engine::profile_closure>();
  p->profile       = {"profile", "profile.json", "0644", "{\"version\":1,\"modules\":[\"m1.lua\"]}"};
  std::string name = "inspect";
  p->modules.push_back({"modules", "m1.lua", "0644",
                        "local D={name='" + name + "',family='extension_read_v1',handler_id='custom." + name +
                            "',options=engine.array{},help='initial',response_kind='extension_v1',capabilities=engine.array{}}; "
                            "registration.add(D,function() return D.help end); D.help='final'"});
  return p;
}
} // namespace

TEST_CASE("RA01-B1 authoring catalog seals the eleven RA-1 rows in sorted order", "[authoring-catalog]") {
  auto control = variant(0);
  REQUIRE(control.sealed);
  auto rows = commands_of(control);
  REQUIRE(rows.size() == 11);
  for (std::size_t i = 0; i < rows.size(); ++i) {
    CAPTURE(i);
    CHECK(name_of(rows[i]) == eleven[i]);
    CHECK(rows[i]->fields.at("family")->string == eleven[i]);
  }
}

TEST_CASE("RA01-B1 authoring handler ids and response kinds use underscores", "[authoring-catalog]") {
  require_control();
  auto rows = commands_of(variant(0));
  for (auto& [name, spec] : ra1()) {
    CAPTURE(name);
    auto row = find_row(rows, name);
    REQUIRE(row);
    CHECK(row->fields.at("handler_id")->string == spec.handler);
    CHECK(row->fields.at("response_kind")->string == spec.kind);
  }
}

TEST_CASE("RA01-B1 authoring capabilities are the exact per row sets", "[authoring-catalog]") {
  require_control();
  auto rows = commands_of(variant(0));
  for (auto& [name, spec] : ra1()) {
    CAPTURE(name);
    auto row = find_row(rows, name);
    REQUIRE(row);
    auto caps = row->fields.at("capabilities");
    REQUIRE(caps->items.size() == 1);
    CHECK(caps->items[0]->string == spec.capability);
  }
}

TEST_CASE("RA01-B1 authoring option signatures are exactly the RA-1 table", "[authoring-catalog]") {
  require_control();
  auto rows = commands_of(variant(0));
  for (auto& [name, spec] : ra1()) {
    CAPTURE(name);
    auto row = find_row(rows, name);
    REQUIRE(row);
    auto options = row->fields.at("options");
    REQUIRE(options->items.size() == spec.options.size());
    for (std::size_t i = 0; i < spec.options.size(); ++i) {
      CAPTURE(i);
      auto& o = options->items[i];
      CHECK(o->fields.at("name")->string == std::get<0>(spec.options[i]));
      CHECK(o->fields.at("type")->string == std::get<1>(spec.options[i]));
      CHECK(o->fields.at("required")->boolean == std::get<2>(spec.options[i]));
    }
  }
}

TEST_CASE("RA01-B1 authoring catalog keeps the five shipped descriptors identical", "[authoring-catalog]") {
  require_control();
  auto control = variant(0);
  auto prod    = planar::engine::command_vm::create();
  REQUIRE(prod);
  auto prod_rows = prod->catalog()->fields.at("commands")->items;
  auto rows      = commands_of(control);
  REQUIRE(prod_rows.size() == 5);
  REQUIRE(rows.size() == 11);
  for (auto& p : prod_rows) {
    CAPTURE(name_of(p));
    auto c = find_row(rows, name_of(p));
    REQUIRE(c);
    CHECK(same_tree(p, c));
  }
}

TEST_CASE("RA01-B1 control bundle seals and a refused bundle maps to internal exit 4", "[authoring-catalog]") {
  require_control();
  auto refused = variant(1);
  INFO((refused.sealed ? "expected refusal, got a sealed VM" : "refused"));
  REQUIRE_FALSE(refused.sealed);
  auto response = planar::cmd::detail::failed(planar::cmd::detail::native_error(refused.error));
  CHECK(response.status == 4);
  CHECK(response.output.empty());
  CHECK_FALSE(response.error.empty());
  CHECK(response.error.find("Invalid embedded command protocol") != std::string::npos);
  CHECK(response.error.find("\"internal\"") != std::string::npos);
}

TEST_CASE("RA01-B1 default catalog refuses authoring rows registered without the request", "[authoring-catalog]") {
  require_control();
  // The refusal is only meaningful beside a control that seals the full eleven-row bundle.
  REQUIRE(commands_of(variant(0)).size() == 11);
  expect_refused(18);
}

TEST_CASE("RA01-B1 validator refuses a catalog missing one row", "[authoring-catalog]") {
  require_control();
  for (int n = 1; n <= 7; ++n)
    expect_refused(n);
}

TEST_CASE("RA01-B1 validator refuses the five shipped rows when eleven are required", "[authoring-catalog]") {
  require_control();
  expect_refused(19);
}

TEST_CASE("RA01-B1 validator refuses a duplicated authoring row", "[authoring-catalog]") {
  require_control();
  expect_refused(9);
}

TEST_CASE("RA01-B1 validator refuses an extra twelfth row", "[authoring-catalog]") {
  require_control();
  expect_refused(8);
}

TEST_CASE("RA01-B1 validator refuses a mispaired family", "[authoring-catalog]") {
  require_control();
  expect_refused(10);
}

TEST_CASE("RA01-B1 validator refuses a mispaired handler id", "[authoring-catalog]") {
  require_control();
  expect_refused(11);
}

TEST_CASE("RA01-B1 validator refuses a mispaired response kind", "[authoring-catalog]") {
  require_control();
  expect_refused(12);
}

TEST_CASE("RA01-B1 validator refuses a mispaired capability", "[authoring-catalog]") {
  require_control();
  expect_refused(13);
}

TEST_CASE("RA01-B1 validator refuses a missing option", "[authoring-catalog]") {
  require_control();
  expect_refused(14);
}

TEST_CASE("RA01-B1 validator refuses a renamed option", "[authoring-catalog]") {
  require_control();
  expect_refused(15);
}

TEST_CASE("RA01-B1 validator refuses a wrong option type", "[authoring-catalog]") {
  require_control();
  expect_refused(16);
}

TEST_CASE("RA01-B1 validator refuses a wrong required flag", "[authoring-catalog]") {
  require_control();
  expect_refused(17);
}

TEST_CASE("RA01-B1 defined runtime keeps profile VMs at five rows", "[authoring-catalog]") {
  require_control();
  auto vm = planar::engine::authoring_variant_0::command_vm::create(inspect_profile());
  REQUIRE(vm);
  auto                     rows = vm->catalog()->fields.at("commands")->items;
  std::vector<std::string> names;
  // A profile catalog wraps every row as {descriptor, ...}; the shipped catalog lists bare descriptors.
  for (auto& r : rows)
    names.push_back(name_of(r->fields.at("descriptor")));
  CHECK(names == std::vector<std::string>{"export", "history", "init", "inspect", "load", "render"});
}
