// Pure profile shape helpers through an exact runtime with test-only providers.
#include <catch2/catch_test_macros.hpp>
#include <unistd.h>
import std;
import planar.engine.runtime;
import planar.engine.json_codec;
import planar.engine.capture;
import planar.engine.shapes_test;
using namespace planar::engine;
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
value_ptr integer_value(std::int64_t n) {
  auto v     = std::make_shared<value>();
  v->kind    = value_kind::integer;
  v->integer = n;
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
struct fixture {
  std::filesystem::path root;
  value_ptr             workspace;
  value_ptr             bundle;
  fixture() {
    std::string pattern = (std::filesystem::temp_directory_path() / "planar-schema-XXXXXX").string();
    REQUIRE(mkdtemp(pattern.data()));
    root = std::filesystem::canonical(pattern);
    std::filesystem::copy(std::filesystem::path(PLANAR_SCHEMA_FIXTURE) / "workspace", root / "workspace",
                          std::filesystem::copy_options::recursive);
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
  ~fixture() {
    std::filesystem::remove_all(root);
  }
};
} // namespace
namespace {
value_ptr json(std::string_view bytes) {
  auto result = codec_decode(bytes);
  REQUIRE(result);
  return *result;
}
value_ptr boolean(bool b) {
  auto v     = std::make_shared<value>();
  v->kind    = value_kind::boolean;
  v->boolean = b;
  return v;
}
value_ptr job_field(value_ptr bundle, std::string_view id, std::string key, value_ptr v) {
  std::vector<value_ptr> jobs;
  for (auto entry : bundle->fields.at("jobs")->items) {
    auto job = entry->fields.at("value");
    if (job->fields.at("id")->string == id)
      entry = changed(entry, "value", changed(job, key, v));
    jobs.push_back(entry);
  }
  return changed(bundle, "jobs", array_value(jobs));
}
value_ptr custom(value_ptr bundle) {
  for (auto id : {"combined", "split"})
    bundle = job_field(bundle, id, "shape", string_value("custom"));
  return bundle;
}
value_ptr shape_request(value_ptr bundle, bool profile = true) {
  return record_value({{"bundle", bundle},
                       {"profile", boolean(profile)},
                       {"descriptors", json(R"({"custom":{"stages":"optional"}})")},
                       {"responses", record_value()},
                       {"response", json(R"({"ok":true,"value":{"obligations":[],"guidance":""}})")}});
}
value_ptr probe(value_ptr request) {
  auto result = shapes_test::run_trusted("shapes_probe", request);
  INFO((result ? "native success" : result.error().message));
  REQUIRE(result);
  if ((*result)->fields.contains("failure")) {
    INFO((*result)->fields.at("failure")->string);
    FAIL("Lua probe failed");
  }
  return *result;
}
value_ptr result_ok(value_ptr probe) {
  auto result = probe->fields.at("result");
  if (result->fields.contains("error")) {
    INFO(*codec_encode(result));
    FAIL("Unexpected shape refusal");
  }
  return result->fields.at("value");
}
value_ptr error_is(value_ptr p, std::string_view code) {
  auto r = p->fields.at("result");
  REQUIRE(r->fields.contains("error"));
  auto e = r->fields.at("error");
  INFO(*codec_encode(e));
  CHECK(e->fields.at("code")->string == code);
  return e;
}
void equal(value_ptr a, value_ptr b) {
  REQUIRE(codec_encode(a));
  REQUIRE(codec_encode(b));
  CHECK(*codec_encode(a) == *codec_encode(b));
}
value_ptr obligation(std::string_view producer, std::string_view consumer, std::string_view phase = "start") {
  return record_value(
      {{"ok", boolean(true)},
       {"value", record_value({{"guidance", string_value("shape guidance")},
                               {"obligations", array_value({record_value({{"id", string_value("order")},
                                                                          {"producer", string_value(std::string(producer))},
                                                                          {"consumer", string_value(std::string(consumer))},
                                                                          {"phase", string_value(std::string(phase))},
                                                                          {"quality", string_value("done")}})})}})}});
}
} // namespace

TEST_CASE("X05 B1 embedded default helper values remain exact", "[x05b1][shape]") {
  fixture f;
  for (bool normalize : {false, true}) {
    auto request = changed(shape_request(f.bundle, false), "normalize", boolean(normalize));
    auto actual  = probe(request);
    auto expected =
        success(normalize ? "normalize_bundle" : "validate_bundle",
                normalize ? f.bundle : record_value({{"bundle", f.bundle}, {"capture", json(R"({"status":"not_captured"})")}}));
    equal(result_ok(actual), expected);
    CHECK(actual->fields.at("calls")->integer == 0);
    auto profiled = result_ok(probe(changed(request, "profile", boolean(true))));
    if (!normalize)
      profiled = changed(profiled, "shape_results", nullptr);
    equal(profiled, expected);
  }
}
TEST_CASE("X05 B1 dialect is internal and unknown custom shapes are references", "[x05b1][shape]") {
  fixture f;
  auto    bundle = custom(f.bundle);
  error_is(probe(shape_request(bundle, false)), "schema");
  auto unknown = changed(shape_request(bundle), "descriptors", record_value());
  auto p       = probe(unknown);
  CHECK(error_is(p, "reference")->fields.at("field")->string == "job.shape");
  CHECK(p->fields.at("calls")->integer == 0);
  auto injected = changed(f.bundle, "dialect", string_value("declarations_shapes_v1"));
  error_is(probe(shape_request(injected)), "schema");
}
TEST_CASE("X05 B1 custom stages honor descriptor presence and worker sets", "[x05b1][shape]") {
  fixture f;
  for (auto policy : {"required", "forbidden", "optional"})
    for (bool stages : {false, true}) {
      auto bundle = custom(f.bundle);
      if (stages) {
        bundle = job_field(bundle, "combined", "stages", json(R"([["builder"]])"));
        bundle = job_field(bundle, "split", "stages", json(R"([["cli_worker","library_worker"]])"));
      }
      auto request = changed(shape_request(bundle), "descriptors",
                             record_value({{"custom", record_value({{"stages", string_value(policy)}})}}));
      auto p       = probe(request);
      if ((std::string_view(policy) == "required" && !stages) || (std::string_view(policy) == "forbidden" && stages)) {
        error_is(p, "schema");
        CHECK(p->fields.at("calls")->integer == 0);
      } else {
        result_ok(p);
        CHECK(p->fields.at("calls")->integer == 2);
      }
    }
  for (auto stages : {R"([])", R"([[]])", R"([["reviewer"]])", R"([["builder"],["builder"]])", R"([["missing"]])"}) {
    auto bundle = job_field(custom(f.bundle), "combined", "stages", json(stages));
    auto p      = probe(shape_request(bundle));
    // Empty outer stages is valid schema but fails mandatory worker coverage.
    error_is(p, std::string_view(stages) == "[[]]" ? "schema" : "reference");
    CHECK(p->fields.at("calls")->integer == 0);
  }
}
TEST_CASE("X05 B1 all mandatory jobs validate before any selected callback", "[x05b1][shape]") {
  fixture f;
  for (auto job : {"combined", "split"}) {
    auto bundle = job_field(custom(f.bundle), job, "roles", array_value());
    auto p      = probe(shape_request(bundle));
    error_is(p, "schema");
    CHECK(p->fields.at("calls")->integer == 0);
  }
  auto spec = f.bundle->fields.at("spec");
  auto bad  = changed(spec->fields.at("value"), "frozen", json(R"(["src/"])"));
  auto p    = probe(shape_request(changed(custom(f.bundle), "spec", changed(spec, "value", bad))));
  error_is(p, "ownership");
  CHECK(p->fields.at("calls")->integer == 0);
}
TEST_CASE("X05 B1 every custom shape sees exact normalized base input", "[x05b1][shape]") {
  fixture f;
  auto    p = probe(shape_request(custom(f.bundle)));
  result_ok(p);
  REQUIRE(p->fields.at("inputs")->items.size() == 2);
  auto normalized = success("normalize_bundle", f.bundle);
  auto baseline =
      success("validate_bundle", record_value({{"bundle", f.bundle}, {"capture", json(R"({"status":"not_captured"})")}}));
  for (std::size_t i = 0; i < 2; ++i) {
    auto input = p->fields.at("inputs")->items[i];
    CHECK(input->fields.size() == 5);
    CHECK(input->fields.at("version")->integer == 1);
    equal(input->fields.at("spec"), normalized->fields.at("spec"));
    auto expected = changed(normalized->fields.at("jobs")->items[i]->fields.at("value"), "shape", string_value("custom"));
    equal(input->fields.at("job"), expected);
    auto model = baseline->fields.at("jobs")->items[i];
    equal(input->fields.at("assignments"), model->fields.at("assignments"));
    equal(input->fields.at("reviewer"), model->fields.at("reviewer"));
  }
}
TEST_CASE("X05 B1 additive obligations retain unit endpoints and sorted synthetic identities", "[x05b1][shape]") {
  fixture f;
  auto    request = changed(shape_request(custom(f.bundle)), "response", obligation("library", "command"));
  auto    result  = result_ok(probe(request));
  REQUIRE(result->fields.at("shape_results")->items.size() == 2);
  auto row = result->fields.at("shape_results")->items[0];
  CHECK(row->fields.at("guidance")->string == "shape guidance");
  auto o = row->fields.at("obligations")->items[0];
  CHECK(o->fields.at("id")->string == "shape:custom:order");
  CHECK(o->fields.at("source")->string == "shape");
  CHECK(o->fields.at("producer_role")->string == "builder");
  CHECK(o->fields.at("consumer_role")->string == "builder");
  auto graph = result->fields.at("jobs")->items[0]->fields.at("graph");
  bool unit = false, role = false;
  for (auto edge : graph->fields.at("edges")->items) {
    auto from = edge->fields.at("from")->string, to = edge->fields.at("to")->string;
    unit |= from == "work:library:complete" && to == "work:command:start";
    role |= from == "role:builder:complete" && to == "role:builder:start";
  }
  CHECK(unit);
  CHECK_FALSE(role);
}
TEST_CASE("X05 B1 added full graph detects cycles for grouped units", "[x05b1][shape]") {
  fixture f;
  auto    request = changed(shape_request(custom(f.bundle)), "response", obligation("command", "library"));
  auto    p       = probe(request);
  auto    e       = error_is(p, "dependency_cycle");
  auto    cycle   = e->fields.at("details")->fields.at("cycle");
  REQUIRE(cycle->items.size() > 2);
  equal(cycle->items.front(), cycle->items.back());
  CHECK(p->fields.at("calls")->integer == 2);
}
TEST_CASE("X05 B1 semantic refusal preserves existing error vocabulary", "[x05b1][shape]") {
  fixture f;
  auto    response = json(
      R"({"ok":false,"error":{"code":"reference","message":"Policy requires a declaration","field":"job.roles","file":"job.lua","details":{"wanted":"worker"}}})");
  auto p = probe(changed(shape_request(custom(f.bundle)), "response", response));
  equal(error_is(p, "reference"), response->fields.at("error"));
  CHECK(p->fields.at("calls")->integer == 1);
}
TEST_CASE("X05 B1 malformed shape success and refusal values fail internal", "[x05b1][shape]") {
  fixture f;
  for (auto bytes : {R"({})", R"({"ok":true,"value":{"obligations":[],"guidance":"","extra":1}})",
                     R"({"ok":true,"value":{"obligations":{},"guidance":""}})", R"({"ok":false,"error":{}})",
                     R"({"ok":false,"error":{"code":"schema","message":"x","field":"job","file":"/host/path"}})",

                     R"({"ok":false,"error":{"code":"schema","message":"x","field":"job","line":1}})"}) {
    error_is(probe(changed(shape_request(custom(f.bundle)), "response", json(bytes))), "internal");
  }
}
TEST_CASE("X05 B1 obligations enforce exact identifiers endpoints phase and quality", "[x05b1][shape]") {
  fixture f;
  auto    original = obligation("library", "command");
  auto    v = original->fields.at("value"), o = v->fields.at("obligations")->items[0];
  for (auto pair : {std::pair{"id", "Bad"},
                    {"producer", "missing"},
                    {"consumer", "library"},
                    {"phase", "accepted"},
                    {"quality", "available"},
                    {"quality", "accepted"}}) {
    auto bad      = changed(o, pair.first, string_value(pair.second));
    auto response = changed(original, "value", changed(v, "obligations", array_value({bad})));
    error_is(probe(changed(shape_request(custom(f.bundle)), "response", response)), "internal");
  }
  for (auto bad : {changed(o, "extra", integer_value(1)), changed(o, "id", nullptr)}) {
    auto response = changed(original, "value", changed(v, "obligations", array_value({bad})));
    error_is(probe(changed(shape_request(custom(f.bundle)), "response", response)), "internal");
  }
  auto duplicated = changed(original, "value", changed(v, "obligations", array_value({o, o})));
  error_is(probe(changed(shape_request(custom(f.bundle)), "response", duplicated)), "internal");
}
TEST_CASE("X05 B1 guidance uses exact UTF8 and byte ceiling", "[x05b1][shape]") {
  fixture f;
  for (auto bytes : {std::string(65536, 'x'), std::string(65537, 'x'), std::string("\xc3\xa9")}) {
    auto request  = shape_request(custom(f.bundle));
    auto response = request->fields.at("response");
    response      = changed(response, "value", changed(response->fields.at("value"), "guidance", string_value(bytes)));
    auto p        = probe(changed(request, "response", response));
    if (bytes.size() > 65536)
      error_is(p, "internal");
    else
      result_ok(p);
  }
}
TEST_CASE("X05 B1 obligation quota accepts 10000 and refuses 10001", "[x05b1][shape]") {
  fixture f;
  for (int count : {10000, 10001}) {
    auto request =
        changed(changed(shape_request(custom(f.bundle)), "mode", string_value("generated")), "count", integer_value(count));
    auto p = probe(request);
    if (count == 10000) {
      auto result = result_ok(p);
      CHECK(result->fields.at("shape_results")->items[0]->fields.at("obligations")->items.size() == 10000);
    } else
      error_is(p, "resource_limit");
  }
}

TEST_CASE("X05 B1 controlled malformed Lua results cannot cross helper protocol", "[x05b1][shape]") {
  fixture f;
  for (auto variant : {"nil", "extra", "sparse", "utf8", "nul", "callable"}) {
    auto request =
        changed(changed(shape_request(custom(f.bundle)), "mode", string_value("malformed")), "variant", string_value(variant));
    error_is(probe(request), "internal");
  }
}
namespace {
value_ptr role_field(value_ptr bundle, std::string_view job_id, std::string_view role_id, std::string key,
                     value_ptr replacement) {
  for (auto entry : bundle->fields.at("jobs")->items) {
    auto job = entry->fields.at("value");
    if (job->fields.at("id")->string != job_id)
      continue;
    std::vector<value_ptr> roles;
    for (auto role : job->fields.at("roles")->items) {
      if (role->fields.at("id")->string == role_id)
        role = changed(role, key, replacement);
      roles.push_back(role);
    }
    return job_field(bundle, job_id, "roles", array_value(roles));
  }
  FAIL("Missing job");
  return nullptr;
}
value_ptr spec_field(value_ptr bundle, std::string key, value_ptr replacement) {
  auto entry = bundle->fields.at("spec");
  return changed(bundle, "spec", changed(entry, "value", changed(entry->fields.at("value"), key, replacement)));
}
} // namespace
TEST_CASE("X05 B1 semantic worker reviewer registry and subscription failures make zero calls", "[x05b1][shape]") {
  fixture                f;
  std::vector<value_ptr> bad;
  auto                   bundle = custom(f.bundle);
  bad.push_back(role_field(bundle, "split", "cli_worker", "work", json(R"(["library"])")));
  bad.push_back(role_field(bundle, "combined", "builder", "work", json(R"(["library"])")));
  bad.push_back(role_field(bundle, "split", "reviewer", "targets", json(R"(["library"])")));
  bad.push_back(role_field(bundle, "split", "reviewer", "checks", json(R"(["contract"])")));
  bad.push_back(role_field(bundle, "split", "cli_worker", "checks", json(R"(["unknown"])")));
  auto dependencies = bundle->fields.at("spec")->fields.at("value")->fields.at("dependencies");
  bad.push_back(spec_field(bundle, "dependencies", array_value({dependencies->items[1]})));
  bad.push_back(spec_field(bundle, "dependencies", array_value({dependencies->items[0]})));
  for (auto value : bad) {
    auto p = probe(shape_request(value));
    REQUIRE(p->fields.at("result")->fields.contains("error"));
    CHECK(p->fields.at("calls")->integer == 0);
  }
}
TEST_CASE("X05 B1 accepted dependency base cycles precede every custom callback", "[x05b1][shape]") {
  fixture f;
  auto    bundle       = custom(f.bundle);
  auto    dependencies = bundle->fields.at("spec")->fields.at("value")->fields.at("dependencies");
  auto    accepted     = changed(dependencies->items[1], "quality", string_value("accepted"));
  bundle               = spec_field(bundle, "dependencies", array_value({dependencies->items[0], accepted}));
  auto p               = probe(shape_request(bundle));
  error_is(p, "dependency_cycle");
  CHECK(p->fields.at("calls")->integer == 0);
}
TEST_CASE("X05 B1 done edges compose with start and complete facet phases", "[x05b1][shape]") {
  fixture f;
  for (auto phase : {"start", "complete"}) {
    auto bundle       = custom(f.bundle);
    auto dependencies = bundle->fields.at("spec")->fields.at("value")->fields.at("dependencies");
    // Add an independent start phase to the existing complete facet dependency.
    auto start   = changed(changed(dependencies->items[1], "id", string_value("start_edge")), "phase", string_value("start"));
    bundle       = spec_field(bundle, "dependencies", array_value({dependencies->items[0], dependencies->items[1], start}));
    auto request = changed(shape_request(bundle), "response", obligation("command", "library", phase));
    auto p       = probe(request);
    error_is(p, "dependency_cycle");
    CHECK(p->fields.at("calls")->integer == 2);
  }
}
TEST_CASE("X05 B1 cumulative obligations share native instruction accounting", "[x05b1][shape]") {
  fixture f;
  auto    original = f.bundle->fields.at("jobs")->items[0]->fields.at("value");
  for (int count : {3, 11}) {
    std::vector<value_ptr> jobs, routes;
    for (int i = 0; i < count; ++i) {
      auto id   = "copy" + std::to_string(i);
      auto file = id + ".lua";
      auto job  = changed(changed(original, "id", string_value(id)), "shape", string_value("custom"));
      jobs.push_back(record_value({{"file", string_value(file)}, {"value", job}}));
      routes.push_back(record_value({{"id", string_value(id)}, {"file", string_value(file)}}));
    }
    auto bundle    = changed(f.bundle, "jobs", array_value(jobs));
    auto workspace = bundle->fields.at("workspace");
    bundle         = changed(bundle, "workspace",
                             changed(workspace, "value", changed(workspace->fields.at("value"), "jobs", array_value(routes))));
    auto request   = changed(changed(shape_request(bundle), "mode", string_value("generated")), "count", integer_value(10000));
    if (count == 3) {
      auto result = result_ok(probe(request));
      CHECK(result->fields.at("shape_results")->items.size() == 3);
      for (auto row : result->fields.at("shape_results")->items)
        CHECK(row->fields.at("obligations")->items.size() == 10000);
    } else {
      // Unchanged native instruction accounting dominates before the semantic
      // total reaches 100000. This is not a proof of the exact global boundary.
      auto result = shapes_test::run_trusted("shapes_probe", request);
      REQUIRE_FALSE(result);
      CHECK(result.error().code == "resource_limit");
      CHECK(result.error().limit == "instructions");
      CHECK(result.error().maximum == 100000000);
    }
  }
}

TEST_CASE("X05 B1 normalization sorts additive IDs and preserves every base policy edge", "[x05b1][shape]") {
  fixture f;
  auto    request  = shape_request(custom(f.bundle));
  auto    response = obligation("library", "command");
  auto    value    = response->fields.at("value");
  auto    first    = value->fields.at("obligations")->items[0];
  auto    last     = changed(changed(first, "id", string_value("z")), "phase", string_value("complete"));
  first            = changed(first, "id", string_value("a"));
  response         = changed(response, "value", changed(value, "obligations", array_value({last, first})));
  auto result      = result_ok(probe(changed(request, "response", response)));
  auto baseline =
      success("validate_bundle", record_value({{"bundle", f.bundle}, {"capture", json(R"({"status":"not_captured"})")}}));
  equal(result->fields.at("dependencies"), baseline->fields.at("dependencies"));
  for (std::size_t i = 0; i < 2; ++i) {
    auto row = result->fields.at("shape_results")->items[i]->fields.at("obligations");
    CHECK(row->items[0]->fields.at("id")->string == "shape:custom:a");
    CHECK(row->items[1]->fields.at("id")->string == "shape:custom:z");
    auto actual = result->fields.at("jobs")->items[i], expected = baseline->fields.at("jobs")->items[i];
    equal(changed(actual, "graph", nullptr), changed(expected, "graph", nullptr));
    auto edges = actual->fields.at("graph")->fields.at("edges");
    for (auto base : expected->fields.at("graph")->fields.at("edges")->items) {
      bool found = false;
      for (auto edge : edges->items)
        found |= *codec_encode(base) == *codec_encode(edge);
      CHECK(found);
    }
  }
}

namespace {
value_ptr phased_prepare(value_ptr bundle) {
  return record_value({{"validation", record_value({{"bundle", bundle}, {"capture", json(R"({"status":"not_captured"})")}})},
                       {"context", json(R"({"dialect":"declarations_shapes_v1","shapes":{"custom":{"stages":"optional"}}})")}});
}
value_ptr phased_check(value_ptr request, value_ptr response, std::int64_t total = 0) {
  return record_value({{"request", request}, {"response", response}, {"total", integer_value(total)}});
}
value_ptr definition(std::int64_t index = 1, std::string job = "combined") {
  return changed(changed(json(R"({"id":"custom","revision":"revision1","definition":"0123456789abcdef0123456789abcdef"})"),
                         "index", integer_value(index)),
                 "job", string_value(job));
}
value_ptr phased_finish(value_ptr prepared, value_ptr results, value_ptr definitions) {
  return record_value({{"prepared", prepared}, {"results", results}, {"definitions", definitions}});
}
} // namespace
TEST_CASE("X05 B3 A structural preparation preserves mixed order and exact five field inputs", "[x05b3a][shape]") {
  fixture f;
  auto    bundle   = job_field(f.bundle, "split", "shape", string_value("custom"));
  auto    prepared = success("shape_prepare", phased_prepare(bundle));
  REQUIRE(prepared->fields.at("requests")->items.size() == 1);
  auto request = prepared->fields.at("requests")->items[0];
  CHECK(request->fields.at("index")->integer == 2);
  CHECK(request->fields.at("id")->string == "split");
  CHECK(request->fields.at("input")->fields.size() == 5);
  auto old = result_ok(probe(shape_request(bundle)));
  equal(request->fields.at("input"), probe(shape_request(bundle))->fields.at("inputs")->items[0]);
  equal(prepared->fields.at("base"), changed(old, "shape_results", nullptr));
  // Round trips retain empty requests and all normalized array tags.
  auto empty = success("shape_prepare", phased_prepare(f.bundle));
  CHECK(empty->fields.at("requests")->kind == value_kind::list);
  CHECK(empty->fields.at("requests")->items.empty());
  auto finished = success("shape_finish", phased_finish(empty, array_value(), array_value()));
  equal(finished, empty->fields.at("base"));
}
TEST_CASE("X05 B3 A mandatory errors and base cycles precede structural requests", "[x05b3a][shape]") {
  fixture f;
  for (auto id : {"combined", "split"}) {
    auto bad    = job_field(custom(f.bundle), id, "roles", array_value());
    auto phased = invoke("shape_prepare", phased_prepare(bad));
    equal(phased, probe(shape_request(bad))->fields.at("result"));
    CHECK_FALSE(phased->fields.contains("value"));
  }
  auto bundle       = custom(f.bundle);
  auto dependencies = bundle->fields.at("spec")->fields.at("value")->fields.at("dependencies");
  bundle =
      spec_field(bundle, "dependencies",
                 array_value({dependencies->items[0], changed(dependencies->items[1], "quality", string_value("accepted"))}));
  equal(invoke("shape_prepare", phased_prepare(bundle)), probe(shape_request(bundle))->fields.at("result"));
}
TEST_CASE("X05 B3 A check response matches B1 refusal malformed and guidance boundaries", "[x05b3a][shape]") {
  fixture f;
  auto    bundle   = custom(f.bundle);
  auto    prepared = success("shape_prepare", phased_prepare(bundle));
  auto    request  = prepared->fields.at("requests")->items[0];
  for (auto response : {json(R"({})"), json(R"({"ok":true,"value":{"obligations":{},"guidance":""}})"),
                        json(R"({"ok":false,"error":{"code":"reference","message":"policy","field":"job.roles"}})"),
                        json(R"({"ok":true,"value":{"obligations":[{}],"guidance":""}})")}) {
    equal(invoke("shape_check", phased_check(request, response)),
          probe(changed(shape_request(bundle), "response", response))->fields.at("result"));
  }
  for (int size : {65536, 65537}) {
    auto response = json(R"({"ok":true,"value":{"obligations":[],"guidance":""}})");
    response =
        changed(response, "value", changed(response->fields.at("value"), "guidance", string_value(std::string(size, 'x'))));
    auto checked = invoke("shape_check", phased_check(request, response));
    CHECK(checked->fields.contains("error") == (size == 65537));
  }
}
TEST_CASE("X05 B3 A cumulative count checks before normalization expansion", "[x05b3a][shape]") {
  fixture f;
  auto    prepared = success("shape_prepare", phased_prepare(custom(f.bundle)));
  auto    request  = prepared->fields.at("requests")->items[0];
  auto    response = obligation("library", "command");
  auto    checked  = success("shape_check", phased_check(request, response, 99999));
  CHECK(checked->fields.at("total")->integer == 100000);
  REQUIRE(checked->fields.at("obligations")->items.size() == 1);
  auto refused = invoke("shape_check", phased_check(request, response, 100000));
  CHECK(refused->fields.at("error")->fields.at("code")->string == "resource_limit");
  CHECK(refused->fields.at("error")->fields.at("details")->fields.at("maximum")->integer == 100000);
}
TEST_CASE("X05 B3 A finish binds supplied metadata and preserves B1 graph semantics", "[x05b3a][shape]") {
  fixture f;
  auto    bundle = custom(f.bundle);
  for (auto producer : {"library", "command"}) {
    auto                   prepared = success("shape_prepare", phased_prepare(bundle));
    auto                   response = obligation(producer, std::string_view(producer) == "library" ? "command" : "library");
    std::vector<value_ptr> results;
    std::int64_t           total = 0;
    for (auto request : prepared->fields.at("requests")->items) {
      auto checked = success("shape_check", phased_check(request, response, total));
      total        = checked->fields.at("total")->integer;
      results.push_back(checked);
    }
    auto finished = invoke("shape_finish",
                           phased_finish(prepared, array_value(results), array_value({definition(), definition(2, "split")})));
    auto old      = probe(changed(shape_request(bundle), "response", response))->fields.at("result");
    if (old->fields.contains("error")) {
      equal(finished, old);
      continue;
    }
    auto actual = finished->fields.at("value");
    CHECK_FALSE(actual->fields.contains("shape_results"));
    for (std::size_t i = 0; i < 2; ++i) {
      auto model    = actual->fields.at("jobs")->items[i];
      auto selected = model->fields.at("selected_shape");
      equal(changed(changed(selected, "obligations", nullptr), "guidance", nullptr),
            changed(changed(definition(), "index", nullptr), "job", nullptr));
      equal(selected->fields.at("obligations"),
            old->fields.at("value")->fields.at("shape_results")->items[i]->fields.at("obligations"));
      equal(changed(model, "selected_shape", nullptr), old->fields.at("value")->fields.at("jobs")->items[i]);
    }
  }
}
TEST_CASE("X05 B3 A finish refuses result and definition correspondence mismatch", "[x05b3a][shape]") {
  fixture f;
  auto    bundle   = job_field(f.bundle, "split", "shape", string_value("custom"));
  auto    prepared = success("shape_prepare", phased_prepare(bundle));
  auto result = success("shape_check", phased_check(prepared->fields.at("requests")->items[0], obligation("library", "command")));
  for (auto bad : {phased_finish(prepared, array_value(), array_value({definition(2, "split")})),
                   phased_finish(prepared, array_value({changed(result, "id", string_value("combined"))}),
                                 array_value({definition(2, "split")})),
                   phased_finish(prepared, array_value({result}),
                                 array_value({changed(definition(2, "split"), "id", string_value("other"))}))}) {
    auto refused = invoke("shape_finish", bad);
    CHECK(refused->fields.at("error")->fields.at("code")->string == "internal");
  }
}

TEST_CASE("X05 B3 A embedded structural preparation keeps original normalized semantic bytes", "[x05b3a][shape]") {
  fixture f;
  auto    validation = record_value({{"bundle", f.bundle}, {"capture", json(R"({"status":"not_captured"})")}});
  auto    prepared   = success("shape_prepare", record_value({{"validation", validation}}));
  equal(prepared->fields.at("base"), success("validate_bundle", validation));
  CHECK(prepared->fields.at("requests")->items.empty());
  auto                   custom_prepared = success("shape_prepare", phased_prepare(custom(f.bundle)));
  std::vector<value_ptr> results;
  for (auto request : custom_prepared->fields.at("requests")->items)
    results.push_back(success("shape_check", phased_check(request, obligation("library", "command"))));
  auto duplicate =
      invoke("shape_finish", phased_finish(custom_prepared, array_value(results), array_value({definition(), definition()})));
  CHECK(duplicate->fields.at("error")->fields.at("code")->string == "internal");
  auto missing = invoke("shape_finish",
                        changed(phased_finish(custom_prepared, array_value(results), array_value()), "definitions", nullptr));
  CHECK(missing->fields.at("error")->fields.at("code")->string == "internal");
}
