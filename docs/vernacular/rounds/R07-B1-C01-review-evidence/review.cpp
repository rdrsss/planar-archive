// Production embedded schema exports, retained fixture bytes and refusal vectors.
#include <catch2/catch_test_macros.hpp>
#include <unistd.h>
import std;
import planar.kernel.runtime;
import planar.kernel.capture;
using namespace planar::kernel;
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
value_ptr refusal(std::string_view entry, value_ptr input, std::string_view code) {
  auto result = invoke(entry, input);
  REQUIRE(result->fields.contains("error"));
  auto error = result->fields.at("error");
  CHECK(error->fields.at("code")->string == code);
  REQUIRE(error->fields.contains("field"));
  REQUIRE(error->fields.contains("file"));
  REQUIRE(error->fields.contains("details"));
  return error;
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
  value_ptr spec_changed(std::string field, value_ptr replacement) {
    auto entry = bundle->fields.at("spec");
    return changed(bundle, "spec", changed(entry, "value", changed(entry->fields.at("value"), field, replacement)));
  }
};
} // namespace
TEST_CASE("schema production exports normalize every quota job and defaults", "[schema][r07b1]") {
  fixture f;
  auto    normalized = success("normalize_bundle", f.bundle);
  auto    jobs       = normalized->fields.at("jobs");
  REQUIRE(jobs->items.size() == 2);
  CHECK(jobs->items[0]->fields.at("value")->fields.at("id")->string == "combined");
  CHECK(jobs->items[1]->fields.at("value")->fields.at("id")->string == "split");
  for (auto& entry : jobs->items) {
    auto j = entry->fields.at("value");
    CHECK(j->fields.at("max_rounds")->integer == 2);
    for (auto& role : j->fields.at("roles")->items) {
      CHECK(role->fields.at("checks")->items.empty());
      CHECK(role->fields.at("budget_bytes")->integer == 131072);
    }
  }
  auto spec     = normalized->fields.at("spec");
  auto expected = spec->fields.at("contracts")->items[0]->fields.at("expected");
  CHECK(expected->fields.at("kind")->string == "text");
  CHECK(expected->fields.at("file")->fields.at("path")->string == "text/quota-api.md");
  CHECK(expected->fields.at("bytes")->string == read_bytes(f.root / "workspace/text/quota-api.md"));
  auto facet = spec->fields.at("facets")->items[0];
  CHECK(facet->fields.at("kind")->string == "files");
  CHECK(facet->fields.at("normalizer")->string == "exact_bytes_v1");
}
TEST_CASE("schema strict JSON preserves container Unicode and integer identities", "[schema][r07b1]") {
  auto v = success(
      "parse_json",
      record_value(
          {{"bytes",
            string_value(
                R"({"empty_object":{},"empty_array":[],"min":-2147483648,"max":2147483647,"unicode":"\uD83D\uDE00","zero":-0,"bool":false})")},
           {"file", string_value("registry.json")}}));
  CHECK(v->fields.at("empty_object")->kind == value_kind::record);
  CHECK(v->fields.at("empty_array")->kind == value_kind::list);
  CHECK(v->fields.at("min")->integer == -2147483648LL);
  CHECK(v->fields.at("max")->integer == 2147483647);
  CHECK(v->fields.at("unicode")->string == "\xf0\x9f\x98\x80");
  CHECK(v->fields.at("zero")->integer == 0);
  CHECK_FALSE(v->fields.at("bool")->boolean);
}
TEST_CASE("schema strict JSON refuses malformed syntax text duplicates and bounds", "[schema][r07b1]") {
  std::vector<std::string> vectors = {"",
                                      "null",
                                      "1.0",
                                      "1e1",
                                      "2147483648",
                                      "-2147483649",
                                      "01",
                                      "00",
                                      "-01",
                                      "+1",
                                      "[1,]",
                                      "{\"a\":1,}",
                                      "{\"a\":1,\"\\u0061\":2}",
                                      "{} {}",
                                      "truex",
                                      "[",
                                      "{a:1}",
                                      "\"\\u0000\"",
                                      "\"\\uD800\"",
                                      "\"\\uDC00\"",
                                      "\"\\uD800\\u0041\"",
                                      "\"\\x41\"",
                                      "\"unterminated",
                                      "[null]",
                                      "{\"a\":null}",
                                      std::string("\"\xc0\xaf\"", 4),
                                      std::string("\"\xed\xa0\x80\"", 5),
                                      std::string("\"\xf4\x90\x80\x80\"", 6),
                                      std::string("\"\0\"", 3),
                                      "\"\n\""};
  for (const auto& bytes : vectors) {
    INFO(bytes.size());
    auto error =
        refusal("parse_json", record_value({{"bytes", string_value(bytes)}, {"file", string_value("registry.json")}}), "schema");
    CHECK(error->fields.at("file")->string == "registry.json");
    CHECK(error->fields.at("field")->string == "$");
    CHECK(error->fields.at("details")->fields.at("offset")->integer >= 1);
  }
  for (auto bytes : {std::string(1048577, ' '), std::string(130, '[') + "0" + std::string(130, ']')})
    refusal("parse_json", record_value({{"bytes", string_value(bytes)}, {"file", string_value("registry.json")}}),
            "resource_limit");
}
TEST_CASE("schema refuses malformed constructors fields lists wrappers and versions", "[schema][r07b1]") {
  fixture f;
  auto    original = f.bundle->fields.at("spec")->fields.at("value");
  for (auto [field, replacement, code] :
       std::vector<std::tuple<std::string, value_ptr, std::string>>{{"unknown", string_value("x"), "schema"},
                                                                    {"goal", nullptr, "schema"},
                                                                    {"version", integer_value(2), "unsupported_version"},
                                                                    {"id", string_value("Bad"), "schema"},
                                                                    {"work", record_value(), "schema"},
                                                                    {"work", array_value(), "schema"},
                                                                    {"goal", original->fields.at("rationale"), "schema"},
                                                                    {"goal", string_value("not a constructor"), "schema"}}) {
    INFO(field);
    auto error = refusal("normalize_bundle", f.spec_changed(field, replacement), code);
    CHECK(error->fields.at("file")->string == "spec.lua");
  }
  auto duplicate = original->fields.at("work")->items;
  duplicate.push_back(duplicate[0]);
  refusal("normalize_bundle", f.spec_changed("work", array_value(duplicate)), "reference");
  auto empty_goal  = std::make_shared<value>();
  empty_goal->kind = value_kind::md;
  refusal("normalize_bundle", f.spec_changed("goal", empty_goal), "schema");
}
TEST_CASE("schema validates unselected jobs registry and exact workspace routes", "[schema][r07b1]") {
  fixture f;
  auto    jobs = f.bundle->fields.at("jobs")->items;
  jobs[1]      = changed(jobs[1], "value", changed(jobs[1]->fields.at("value"), "id", string_value("wrong")));
  auto error   = refusal("normalize_bundle", changed(f.bundle, "jobs", array_value(jobs)), "reference");
  CHECK(error->fields.at("file")->string == "split.lua");
  CHECK(error->fields.at("field")->string == "job.id");
  jobs.pop_back();
  refusal("normalize_bundle", changed(f.bundle, "jobs", array_value(jobs)), "reference");
  auto registry = f.bundle->fields.at("registry");
  for (auto bytes : {"{}", "[]", "{\"version\":2}", "{\"version\":1,\"version\":1}"})
    refusal("normalize_bundle", changed(f.bundle, "registry", changed(registry, "bytes", string_value(bytes))), "schema");
  auto route =
      success("route_workspace", record_value({{"workspace", f.workspace}, {"file", string_value("nested/workspace.lua")}}));
  CHECK(route->fields.at("spec")->string == "nested/spec.lua");
  CHECK(route->fields.at("jobs")->items.size() == 2);
  CHECK(route->fields.at("jobs")->items[1]->fields.at("file")->string == "nested/split.lua");
  auto bad = changed(f.workspace, "spec", string_value("../escape.lua"));
  refusal("route_workspace", record_value({{"workspace", bad}, {"file", string_value("workspace.lua")}}), "path_denied");
}
TEST_CASE("schema resolves retained text from declaring directory with exact bytes", "[schema][r07b1]") {
  fixture f;
  auto    spec      = f.bundle->fields.at("spec");
  auto    workspace = f.bundle->fields.at("workspace");
  auto    w         = changed(f.workspace, "spec", string_value("nested/spec.lua"));
  auto    files     = f.bundle->fields.at("files")->items;
  for (auto& file : files)
    file = changed(file, "path", string_value("nested/" + file->fields.at("path")->string));
  auto b          = changed(f.bundle, "workspace", changed(workspace, "value", w));
  b               = changed(b, "spec", changed(spec, "file", string_value("nested/spec.lua")));
  b               = changed(b, "files", array_value(files));
  auto normalized = success("normalize_bundle", b);
  auto text       = normalized->fields.at("spec")->fields.at("constraints")->items[0]->fields.at("text");
  CHECK(text->fields.at("file")->fields.at("path")->string == "nested/text/shared.md");
  CHECK(text->fields.at("bytes")->string == read_bytes(f.root / "workspace/text/shared.md"));
  refusal("normalize_bundle", changed(b, "files", array_value()), "io");
  for (auto bytes : {std::string("\0", 1), std::string("\xff", 1), std::string(1048577, 'a')}) {
    auto invalid_files = files;
    for (auto& file : invalid_files)
      file = changed(file, "bytes", string_value(bytes));
    refusal("normalize_bundle", changed(b, "files", array_value(invalid_files)), "schema");
  }
}
TEST_CASE("schema enforces registry descriptor types bounds enums and uniqueness", "[schema][r07b1]") {
  fixture f;
  auto    original    = f.bundle->fields.at("registry")->fields.at("bytes")->string;
  auto    check_bytes = [&](std::string bytes, std::string_view code = "schema") {
    return refusal("normalize_bundle",
                   changed(f.bundle, "registry", changed(f.bundle->fields.at("registry"), "bytes", string_value(bytes))), code);
  };
  auto mutate = [&](std::string_view from, std::string_view to, std::string_view code = "schema") {
    auto bytes = original;
    auto pos   = bytes.find(from);
    REQUIRE(pos != std::string::npos);
    bytes.replace(pos, from.size(), to);
    return check_bytes(bytes, code);
  };
  mutate("\"timeout_ms\": 10000", "\"timeout_ms\": 0");
  mutate("\"timeout_ms\": 10000", "\"timeout_ms\": 3600001");
  mutate("\"max_output_bytes\": 65536", "\"max_output_bytes\": 16777217");
  mutate("\"max_output_bytes\": 65536", "\"max_output_bytes\": true");
  mutate("\"selection\": \"nonempty\"", "\"selection\": \"unknown\"");
  mutate("\"inputs\": [", "\"inputs\": [\"own_output\",", "reference");
  mutate("\"python3\"", "\"\"");
  mutate("\"name\": \"PATH\"", "\"name\": \"path\"");
  mutate("\"name\": \"PYTHONDONTWRITEBYTECODE\"", "\"name\": \"PATH\"", "reference");
  mutate("\"revision\": \"v1\"", "\"revision\": \"wrong\"", "reference");
  mutate("\"version\": 1", "\"version\": 2", "unsupported_version");
  mutate("\"version\": 1", "\"version\": 1, \"unknown\": true");
}
TEST_CASE("schema enforces job intrinsic policy budgets preferences and sorted sets", "[schema][r07b1]") {
  fixture f;
  auto    jobs      = f.bundle->fields.at("jobs")->items;
  auto    original  = jobs[1]->fields.at("value");
  auto    check_job = [&](value_ptr job, std::string_view code = "schema") {
    auto modified = jobs;
    modified[1]   = changed(jobs[1], "value", job);
    return refusal("normalize_bundle", changed(f.bundle, "jobs", array_value(modified)), code);
  };
  check_job(changed(original, "id", string_value("Bad")));
  check_job(changed(original, "shape", string_value("unknown")));
  check_job(changed(original, "shape", string_value("pipeline")));
  check_job(changed(original, "stages", array_value()));
  check_job(changed(original, "budget_bytes", integer_value(0)));
  check_job(changed(original, "budget_bytes", integer_value(1048577)));
  check_job(changed(original, "max_rounds", integer_value(-1)));
  auto roles         = original->fields.at("roles")->items;
  auto changed_roles = roles;
  changed_roles[0]   = changed(roles[0], "targets", array_value({string_value("library")}));
  check_job(changed(original, "roles", array_value(changed_roles)));
  changed_roles    = roles;
  changed_roles[0] = changed(roles[0], "work", nullptr);
  check_job(changed(original, "roles", array_value(changed_roles)));
  changed_roles    = roles;
  changed_roles[0] = changed(roles[0], "preferences", record_value({{"model", string_value(std::string(257, 'a'))}}));
  check_job(changed(original, "roles", array_value(changed_roles)));
  auto normalized = success("normalize_bundle", f.bundle);
  auto units      = normalized->fields.at("jobs")->items[0]->fields.at("value")->fields.at("roles")->items[0]->fields.at("work");
  REQUIRE(units->items.size() == 2);
  CHECK(units->items[0]->string == "command");
  CHECK(units->items[1]->string == "library");
}
TEST_CASE("schema diagnostics choose logical file field code deterministically", "[schema][r07b1]") {
  fixture f;
  auto    bad  = f.spec_changed("unknown", string_value("x"));
  auto    jobs = bad->fields.at("jobs")->items;
  jobs[0]      = changed(jobs[0], "value", changed(jobs[0]->fields.at("value"), "version", integer_value(2)));
  auto error   = refusal("normalize_bundle", changed(bad, "jobs", array_value(jobs)), "unsupported_version");
  CHECK(error->fields.at("file")->string == "combined.lua");
  CHECK(error->fields.at("field")->string == "job.version");
  for (auto path : {"/private/host/path", "../escape", "a\\b", "a//b", "a/.git/b", "a/./b", "a/../b", "a*/b"}) {
    auto w = changed(f.workspace, "spec", string_value(path));
    auto e = refusal("route_workspace", record_value({{"workspace", w}, {"file", string_value("workspace.lua")}}), "path_denied");
    CHECK(e->fields.at("file")->string == "workspace.lua");
    CHECK(e->fields.at("message")->string.find(path) == std::string::npos);
  }
  refusal("route_workspace", record_value({{"workspace", f.workspace}, {"file", string_value("/private/host")}}), "path_denied");
  refusal("normalize_bundle", changed(f.bundle, "unexpected", string_value("x")), "schema");
}
TEST_CASE("schema enforces workspace intrinsic constraints and declaration constructors", "[schema][r07b1]") {
  fixture f;
  auto    route_input = [&](value_ptr workspace) {
    return record_value({{"workspace", workspace}, {"file", string_value("workspace.lua")}});
  };
  for (auto root_id : {"workspace", "registry", "observations"})
    refusal("route_workspace",
            route_input(changed(f.workspace, "root", changed(f.workspace->fields.at("root"), "id", string_value(root_id)))),
            "schema");
  auto jobs = f.workspace->fields.at("jobs")->items;
  jobs[1]   = changed(jobs[1], "file", jobs[0]->fields.at("file"));
  refusal("route_workspace", route_input(changed(f.workspace, "jobs", array_value(jobs))), "reference");
  for (auto name : {"toolchains", "checks", "jobs"})
    refusal("route_workspace", route_input(changed(f.workspace, name, array_value())), "schema");
  auto forged  = std::make_shared<value>(*f.workspace);
  forged->kind = value_kind::record;
  refusal("route_workspace", route_input(forged), "schema");
  auto files = f.bundle->fields.at("files")->items;
  files.push_back(files[0]);
  refusal("normalize_bundle", changed(f.bundle, "files", array_value(files)), "reference");
  files = f.bundle->fields.at("files")->items;
  for (auto& file : files)
    file = changed(file, "bytes", string_value("exact\r\n\t "));
  auto normalized = success("normalize_bundle", changed(f.bundle, "files", array_value(files)));
  CHECK(normalized->fields.at("spec")->fields.at("constraints")->items[0]->fields.at("text")->fields.at("bytes")->string ==
        "exact\r\n\t ");
}
TEST_CASE("schema JSON node limit and request envelopes refuse deterministically", "[schema][r07b1]") {
  std::string many = "[0";
  for (int i = 0; i < 100000; ++i)
    many += ",0";
  many += "]";
  auto error = refusal("parse_json", record_value({{"bytes", string_value(many)}, {"file", string_value("observations.json")}}),
                       "resource_limit");
  CHECK(error->fields.at("details")->fields.at("limit")->string == "json_nodes");
  refusal("parse_json", record_value(), "schema");
  refusal("route_workspace", array_value(), "schema");
  refusal("normalize_bundle", array_value(), "schema");
  refusal("parse_json", record_value({{"bytes", integer_value(1)}, {"file", string_value("registry.json")}}), "schema");
}
TEST_CASE("schema resolves text in every job relative to that job declaration", "[schema][r07b1]") {
  fixture f;
  auto    file_node   = std::make_shared<value>();
  file_node->kind     = value_kind::file;
  file_node->string   = "note.md";
  auto workspace_jobs = f.workspace->fields.at("jobs")->items;
  auto jobs           = f.bundle->fields.at("jobs")->items;
  auto files          = f.bundle->fields.at("files")->items;
  for (std::size_t i = 0; i < jobs.size(); ++i) {
    auto file         = "jobs/" + std::to_string(i) + "/job.lua";
    workspace_jobs[i] = changed(workspace_jobs[i], "file", string_value(file));
    jobs[i] =
        changed(changed(jobs[i], "file", string_value(file)), "value", changed(jobs[i]->fields.at("value"), "notes", file_node));
    files.push_back(record_value({{"path", string_value("jobs/" + std::to_string(i) + "/note.md")},
                                  {"bytes", string_value("job " + std::to_string(i) + "\r\n ")}}));
  }
  auto w          = changed(f.workspace, "jobs", array_value(workspace_jobs));
  auto bundle     = changed(f.bundle, "workspace", changed(f.bundle->fields.at("workspace"), "value", w));
  bundle          = changed(changed(bundle, "jobs", array_value(jobs)), "files", array_value(files));
  auto normalized = success("normalize_bundle", bundle);
  for (std::size_t i = 0; i < jobs.size(); ++i) {
    auto notes = normalized->fields.at("jobs")->items[i]->fields.at("value")->fields.at("notes");
    CHECK(notes->fields.at("file")->fields.at("path")->string == "jobs/" + std::to_string(i) + "/note.md");
    CHECK(notes->fields.at("bytes")->string == "job " + std::to_string(i) + "\r\n ");
  }
}
TEST_CASE("schema final text uses exact text identity and wrapped provenance", "[schema][r07b1c01]") {
  fixture f;
  auto    normalized = success("normalize_bundle", f.bundle);
  auto    spec       = normalized->fields.at("spec");
  for (auto key : {"goal", "rationale"}) {
    auto text = spec->fields.at(key);
    CHECK(text->fields.size() == 2);
    CHECK(text->fields.at("kind")->string == "text");
    CHECK(text->fields.at("bytes")->string ==
          (std::string_view(key) == "goal" ? f.bundle->fields.at("spec")->fields.at("value")->fields.at(key)->string
                                           : f.bundle->fields.at("spec")->fields.at("value")->fields.at(key)->items[0]->string));
  }
  auto text = spec->fields.at("contracts")->items[0]->fields.at("expected");
  CHECK(text->fields.size() == 3);
  CHECK(text->fields.at("kind")->string == "text");
  REQUIRE(text->fields.contains("file"));
  CHECK(text->fields.at("file")->fields.size() == 2);
  CHECK(text->fields.at("file")->fields.at("root")->string == "workspace");
  CHECK(text->fields.at("file")->fields.at("path")->string == "text/quota-api.md");
  CHECK(text->fields.at("bytes")->string == read_bytes(f.root / "workspace/text/quota-api.md"));
  auto file      = std::make_shared<value>();
  file->kind     = value_kind::file;
  file->string   = "text/quota-api.md";
  auto wrapper   = std::make_shared<value>();
  wrapper->kind  = value_kind::immaterial;
  wrapper->items = {file};
  auto w         = changed(f.workspace, "notes", wrapper);
  auto b         = changed(f.bundle, "workspace", changed(f.bundle->fields.at("workspace"), "value", w));
  auto wrapped   = success("normalize_bundle", b)->fields.at("workspace")->fields.at("notes");
  CHECK(wrapped->fields.size() == 3);
  CHECK(wrapped->fields.at("kind")->string == "text");
  CHECK(wrapped->fields.at("file")->fields.at("root")->string == "workspace");
  CHECK(wrapped->fields.at("file")->fields.at("path")->string == "text/quota-api.md");
  CHECK(wrapped->fields.at("bytes")->string == text->fields.at("bytes")->string);
}
TEST_CASE("schema ID record normalization sorts collections and preserves ordered lists", "[schema][r07b1c01]") {
  fixture f;
  auto    normalized = success("normalize_bundle", f.bundle);
  CHECK(normalized->fields.at("spec")->fields.at("work")->items[0]->fields.at("id")->string == "command");
  auto source = f.bundle->fields.at("spec")->fields.at("value");
  for (auto key : {"work", "milestones", "constraints", "prescriptions", "contracts", "facets", "dependencies", "questions"}) {
    INFO(key);
    auto seed =
        std::string_view(key) == "questions"
            ? record_value(
                  {{"id", string_value("seed")}, {"scope", string_value("global")}, {"prompt", source->fields.at("goal")}})
            : source->fields.at(key)->items[0];
    auto z = changed(seed, "id", string_value("z"));
    auto a = changed(seed, "id", string_value("a"));
    for (auto items : {std::vector<value_ptr>{z, a}, std::vector<value_ptr>{a, z}}) {
      auto result = success("normalize_bundle", f.spec_changed(key, array_value(items)))->fields.at("spec")->fields.at(key);
      REQUIRE(result->items.size() == 2);
      CHECK(result->items[0]->fields.at("id")->string == "a");
      CHECK(result->items[1]->fields.at("id")->string == "z");
    }
  }
  auto workspace_jobs = f.workspace->fields.at("jobs")->items;
  std::reverse(workspace_jobs.begin(), workspace_jobs.end());
  auto w        = changed(f.workspace, "jobs", array_value(workspace_jobs));
  auto bundle   = changed(f.bundle, "workspace", changed(f.bundle->fields.at("workspace"), "value", w));
  auto supplied = bundle->fields.at("jobs")->items;
  std::reverse(supplied.begin(), supplied.end());
  bundle    = changed(bundle, "jobs", array_value(supplied));
  auto jobs = bundle->fields.at("jobs")->items;
  for (auto& entry : jobs) {
    auto job   = entry->fields.at("value");
    auto roles = job->fields.at("roles")->items;
    std::reverse(roles.begin(), roles.end());
    entry = changed(entry, "value", changed(job, "roles", array_value(roles)));
  }
  bundle     = changed(bundle, "jobs", array_value(jobs));
  normalized = success("normalize_bundle", bundle);
  auto route = success("route_workspace", record_value({{"workspace", w}, {"file", string_value("workspace.lua")}}));
  CHECK(route->fields.at("jobs")->items[0]->fields.at("id")->string == "combined");
  CHECK(normalized->fields.at("workspace")->fields.at("jobs")->items[0]->fields.at("id")->string == "combined");
  CHECK(normalized->fields.at("jobs")->items[0]->fields.at("value")->fields.at("id")->string == "combined");
  for (auto& entry : normalized->fields.at("jobs")->items) {
    auto roles = entry->fields.at("value")->fields.at("roles");
    for (std::size_t i = 1; i < roles->items.size(); ++i)
      CHECK(roles->items[i - 1]->fields.at("id")->string < roles->items[i]->fields.at("id")->string);
  }
  auto milestone = source->fields.at("milestones")->items[0];
  auto criterion = milestone->fields.at("acceptance")->items[0];
  auto criteria  = array_value({changed(criterion, "id", string_value("z")), changed(criterion, "id", string_value("a"))});
  auto result =
      success("normalize_bundle", f.spec_changed("milestones", array_value({changed(milestone, "acceptance", criteria)})));
  CHECK(
      result->fields.at("spec")->fields.at("milestones")->items[0]->fields.at("acceptance")->items[0]->fields.at("id")->string ==
      "z");
  auto duplicate = array_value({criterion, criterion});
  refusal("normalize_bundle", f.spec_changed("milestones", array_value({changed(milestone, "acceptance", duplicate)})),
          "reference");
  auto pipeline_jobs = f.bundle->fields.at("jobs")->items;
  auto stages        = array_value({array_value({string_value("z"), string_value("a")}), array_value({string_value("m")})});
  auto pipeline    = changed(changed(pipeline_jobs[1]->fields.at("value"), "shape", string_value("pipeline")), "stages", stages);
  pipeline_jobs[1] = changed(pipeline_jobs[1], "value", pipeline);
  auto normalized_pipeline = success("normalize_bundle", changed(f.bundle, "jobs", array_value(pipeline_jobs)));
  auto normalized_stages   = normalized_pipeline->fields.at("jobs")->items[1]->fields.at("value")->fields.at("stages");
  REQUIRE(normalized_stages->items.size() == 2);
  CHECK(normalized_stages->items[0]->items[0]->string == "a");
  CHECK(normalized_stages->items[0]->items[1]->string == "z");
  CHECK(normalized_stages->items[1]->items[0]->string == "m");
  auto toolchain = [](std::string id) {
    return "{\"id\":\"" + id +
           "\",\"description\":\"tool\",\"command\":[\"z\",\"a\"],\"environment\":[{\"name\":\"Z\",\"value\":\"1\"},{\"name\":"
           "\"A\",\"value\":\"2\"}]}";
  };
  auto check = [](std::string id) {
    return "{\"id\":\"" + id +
           "\",\"toolchain\":\"a\",\"argv\":[\"z\",\"a\"],\"coverage\":\"test\",\"inputs\":[\"own_output\"],\"timeout_ms\":1,"
           "\"max_output_bytes\":1,\"selection\":\"nonempty\"}";
  };
  for (bool reverse : {false, true}) {
    auto first  = reverse ? "a" : "z";
    auto second = reverse ? "z" : "a";
    auto bytes  = "{\"version\":1,\"id\":\"quota_tools\",\"revision\":\"v1\",\"toolchains\":[" + toolchain(first) + "," +
                  toolchain(second) + "],\"checks\":[" + check(first) + "," + check(second) + "]}";
    auto r      = success("normalize_bundle",
                          changed(f.bundle, "registry", changed(f.bundle->fields.at("registry"), "bytes", string_value(bytes))))
                      ->fields.at("registry");
    CHECK(r->fields.at("toolchains")->items[0]->fields.at("id")->string == "a");
    CHECK(r->fields.at("checks")->items[0]->fields.at("id")->string == "a");
    CHECK(r->fields.at("toolchains")->items[0]->fields.at("command")->items[0]->string == "z");
    CHECK(r->fields.at("toolchains")->items[0]->fields.at("environment")->items[0]->fields.at("name")->string == "Z");
    CHECK(r->fields.at("checks")->items[0]->fields.at("argv")->items[0]->string == "z");
  }
}
TEST_CASE("schema JSON counts container entries for empty and nonempty terminals", "[schema][r07b1c01]") {
  for (int pattern : {0, 1, 2}) {
    for (bool empty : {false, true}) {
      for (int depth : {127, 128, 129}) {
        INFO(pattern);
        INFO(empty);
        INFO(depth);
        std::string bytes, closes;
        for (int i = 0; i < depth; ++i) {
          bool object = pattern == 1 || (pattern == 2 && i % 2);
          bytes += object ? "{" : "[";
          if (object && (!empty || i < depth - 1))
            bytes += "\"x\":";
          closes.insert(closes.begin(), object ? '}' : ']');
        }
        bytes += empty ? "" : "0";
        bytes += closes;
        auto input = record_value({{"bytes", string_value(bytes)}, {"file", string_value("input.json")}});
        if (depth <= 128)
          success("parse_json", input);
        else {
          auto e = refusal("parse_json", input, "resource_limit");
          CHECK(e->fields.at("details")->fields.at("limit")->string == "json_depth");
        }
      }
    }
  }
}
TEST_CASE("schema validates joined declaration and text paths before diagnostics", "[schema][r07b1c01]") {
  fixture     f;
  std::string prefix;
  for (int i = 0; i < 7; ++i)
    prefix += std::string(128, 'a') + "/";
  auto workspace_file = prefix + "workspace.lua";
  for (auto target : {"job", "spec", "text"}) {
    for (int length : {121, 122, 128}) {
      INFO(target);
      INFO(length);
      auto        leaf = std::string(length, 'b');
      auto        w    = f.workspace;
      std::string field;
      if (std::string_view(target) == "job") {
        auto listed = w->fields.at("jobs")->items;
        // Reverse IDs to ensure joined-path diagnostics retain declaration indexes.
        std::reverse(listed.begin(), listed.end());
        listed[0] = changed(listed[0], "file", string_value(leaf));
        w         = changed(w, "jobs", array_value(listed));
        field     = "workspace.jobs[1].file";
      } else if (std::string_view(target) == "spec") {
        w     = changed(w, "spec", string_value(leaf));
        field = "workspace.spec";
      } else {
        auto file    = std::make_shared<value>();
        file->kind   = value_kind::file;
        file->string = leaf;
        w            = changed(w, "notes", file);
        field        = "workspace.notes";
      }
      auto input         = record_value({{"workspace", w}, {"file", string_value(workspace_file)}});
      auto b             = changed(f.bundle, "workspace", record_value({{"file", string_value(workspace_file)}, {"value", w}}));
      auto joined_length = prefix.size() + leaf.size();
      CHECK(joined_length == static_cast<std::size_t>(903 + length));
      if (joined_length > 1024) {
        for (auto [entry, request] :
             std::vector<std::pair<std::string, value_ptr>>{{"route_workspace", input}, {"normalize_bundle", b}}) {
          auto e = refusal(entry, request, "path_denied");
          CHECK(e->fields.at("file")->string == workspace_file);
          CHECK(e->fields.at("field")->string == field);
        }
      } else {
        auto route      = success("route_workspace", input);
        auto spec_entry = f.bundle->fields.at("spec");
        b               = changed(b, "spec", changed(spec_entry, "file", route->fields.at("spec")));
        auto jobs       = f.bundle->fields.at("jobs")->items;
        for (auto& job : jobs) {
          for (auto& routed : route->fields.at("jobs")->items) {
            if (routed->fields.at("id")->string == job->fields.at("value")->fields.at("id")->string)
              job = changed(job, "file", routed->fields.at("file"));
          }
        }
        b          = changed(b, "jobs", array_value(jobs));
        auto files = f.bundle->fields.at("files")->items;
        for (auto& file : files)
          file = changed(file, "path", string_value(prefix + file->fields.at("path")->string));
        if (std::string_view(target) == "text")
          files.push_back(record_value({{"path", string_value(prefix + leaf)}, {"bytes", string_value("exact\r\n ")}}));
        b          = changed(b, "files", array_value(files));
        auto final = success("normalize_bundle", b);
        if (std::string_view(target) == "text") {
          CHECK(route->fields.at("workspace")->fields.at("notes")->fields.at("path")->string == prefix + leaf);
          auto text = final->fields.at("workspace")->fields.at("notes");
          CHECK(text->fields.at("file")->fields.at("path")->string == prefix + leaf);
          CHECK(text->fields.at("bytes")->string == "exact\r\n ");
        }
      }
    }
  }
}
TEST_CASE("schema sorted records retain declaration indexes for intrinsic diagnostics", "[schema][r07b1c01]") {
  fixture f;
  auto    listed = f.workspace->fields.at("jobs")->items;
  std::reverse(listed.begin(), listed.end());
  listed[1]  = changed(listed[1], "file", listed[0]->fields.at("file"));
  auto w     = changed(f.workspace, "jobs", array_value(listed));
  auto error = refusal("route_workspace", record_value({{"workspace", w}, {"file", string_value("workspace.lua")}}), "reference");
  CHECK(error->fields.at("field")->string == "workspace.jobs[2].file");
  auto jobs  = f.bundle->fields.at("jobs")->items;
  auto job   = jobs[0]->fields.at("value");
  auto roles = job->fields.at("roles")->items;
  std::reverse(roles.begin(), roles.end());
  roles[1] = changed(roles[1], "work", nullptr);
  jobs[0]  = changed(jobs[0], "value", changed(job, "roles", array_value(roles)));
  error    = refusal("normalize_bundle", changed(f.bundle, "jobs", array_value(jobs)), "schema");
  CHECK(error->fields.at("field")->string == "job.roles[2].work");
  auto bytes =
      R"({"version":1,"id":"quota_tools","revision":"v1","toolchains":[{"id":"z","description":"z","command":[""],"environment":[]},{"id":"a","description":"a","command":["ok"],"environment":[]}],"checks":[]})";
  error =
      refusal("normalize_bundle",
              changed(f.bundle, "registry", changed(f.bundle->fields.at("registry"), "bytes", string_value(bytes))), "schema");
  CHECK(error->fields.at("field")->string == "registry.toolchains[1].command[1]");
}

TEST_CASE("independent exact JSON byte and node limits", "[independent]") {
 auto req=[](std::string s){return record_value({{"file",string_value("input.json")},{"bytes",string_value(std::move(s))}});};
 auto scalar=success("parse_json", req(std::string(1048575,' ') + "0")); CHECK(scalar->integer==0);
 auto e=refusal("parse_json", req(std::string(1048576,' ') + "0"), "resource_limit");
 CHECK(e->fields.at("details")->fields.at("limit")->string=="json_bytes");
 std::string s="[0"; for(int i=1;i<99999;++i) s+=",0"; s+="]";
 auto v=success("parse_json",req(s)); CHECK(v->items.size()==99999);
 s.insert(s.size()-1,",0"); e=refusal("parse_json",req(s),"resource_limit");
 CHECK(e->fields.at("details")->fields.at("limit")->string=="json_nodes");
}
TEST_CASE("independent JSON decoded text integer boundaries and duplicate false", "[independent]") {
 auto req=[](std::string s){return record_value({{"file",string_value("input.json")},{"bytes",string_value(std::move(s))}});};
 for(auto s:{R"({"a":false,"\u0061":true})", R"({"\uD83D\uDE00":[],"😀":{}})", "-21474836480", "21474836470", "--1", "-", "00", "-00", R"("\uD800\uD800")", R"("\uDBFF\uE000")", R"("\uDC00")", "[true false]", "[\v0]"}) refusal("parse_json",req(s),"schema");
 for(auto s:{"-2147483648","2147483647","-2147483647","2147483646","-0",R"("\u0001\b\f\n\r\t\/\\\"")",R"("\uDBFF\uDFFF")",R"({"a":false})"}) success("parse_json",req(s));
}
TEST_CASE("independent declaration ID path and list boundary checks", "[independent]") {
 fixture f;
 auto route=[&](value_ptr w){return record_value({{"file",string_value("workspace.lua")},{"workspace",w}});};
 auto id64="a"+std::string(63,'_');
 success("route_workspace",route(changed(f.workspace,"id",string_value(id64))));
 for(auto id:{id64+"a",std::string("_a"),std::string("aA"),std::string("a-"),std::string("é")}) refusal("route_workspace",route(changed(f.workspace,"id",string_value(id))),"schema");
 success("route_workspace",route(changed(f.workspace,"spec",string_value(std::string(128,'a')))));
 for(auto p:{std::string(129,'a'),std::string("a/"),std::string("/"),std::string("a/.GIT/b"),std::string("a//b"),std::string("a/b\\c"),std::string("a/../b")}) refusal("route_workspace",route(changed(f.workspace,"spec",string_value(p))),"path_denied");
 for(auto p:{std::string("a/.github/b"),std::string("a/..x/b"),std::string("a/A_0-./b")}) success("route_workspace",route(changed(f.workspace,"spec",string_value(p))));
 auto spec=f.bundle->fields.at("spec")->fields.at("value");
 auto work=spec->fields.at("work")->items;
 work[0]=changed(work[0],"checks",array_value({string_value("a"),string_value("a")}));
 refusal("normalize_bundle",f.spec_changed("work",array_value(work)),"reference");
 work=spec->fields.at("work")->items; work[0]=changed(work[0],"paths",array_value({string_value("a/"),string_value("a/")}));
 refusal("normalize_bundle",f.spec_changed("work",array_value(work)),"reference");
 for(auto key:{"work","milestones","constraints","prescriptions","contracts","facets","dependencies","questions","frozen"}) {
   INFO(key); refusal("normalize_bundle",f.spec_changed(key,record_value()),"schema");
 }
}
TEST_CASE("independent exact job defaults optional absence and schema bounds", "[independent]") {
 fixture f; auto jobs=f.bundle->fields.at("jobs")->items; auto job=jobs[0]->fields.at("value");
 auto roles=job->fields.at("roles")->items; roles[0]=changed(roles[0],"budget_bytes",integer_value(1));
 roles[0]=changed(roles[0],"preferences",record_value());
 auto mod=changed(changed(changed(job,"budget_bytes",nullptr),"max_rounds",nullptr),"roles",array_value(roles));
 jobs[0]=changed(jobs[0],"value",mod);
 auto n=success("normalize_bundle",changed(f.bundle,"jobs",array_value(jobs)))->fields.at("jobs")->items[0]->fields.at("value");
 CHECK(n->fields.at("budget_bytes")->integer==32768); CHECK(n->fields.at("max_rounds")->integer==2);
 CHECK_FALSE(n->fields.contains("stages")); CHECK_FALSE(n->fields.contains("notes"));
 for(auto& r:n->fields.at("roles")->items) { if(r->fields.contains("preferences")) { CHECK(r->fields.at("preferences")->fields.empty()); CHECK(r->fields.at("budget_bytes")->integer==1); } else CHECK(r->fields.at("budget_bytes")->integer==32768); }
 for(auto [field,num]:std::vector<std::pair<std::string,std::int64_t>>{{"max_rounds",2147483648LL},{"budget_bytes",-1},{"budget_bytes",1048577}}) {
   auto invalid=jobs; invalid[0]=changed(jobs[0],"value",changed(mod,field,integer_value(num)));
   refusal("normalize_bundle",changed(f.bundle,"jobs",array_value(invalid)),"schema");
 }
}
TEST_CASE("independent registry nested exact fields and all supplied jobs validated", "[independent]") {
 fixture f; auto reg=f.bundle->fields.at("registry"); auto base=reg->fields.at("bytes")->string;
 for(auto [from,to]:std::vector<std::pair<std::string,std::string>>{{"\"description\":", "\"extra\":1,\"description\":"},{"\"name\": \"PATH\"","\"name\": \"_PATH\""},{"\"argv\": [", "\"argv\": [null,"},{"\"coverage\":", "\"unknown\":[],\"coverage\":"}}) {
   auto b=base; auto p=b.find(from); REQUIRE(p!=std::string::npos); b.replace(p,from.size(),to);
   refusal("normalize_bundle",changed(f.bundle,"registry",changed(reg,"bytes",string_value(b))),"schema");
 }
 auto jobs=f.bundle->fields.at("jobs")->items;
 jobs.push_back(record_value({{"file",string_value("aaa.lua")},{"value",changed(jobs[0]->fields.at("value"),"budget_bytes",integer_value(0))}}));
 auto e=refusal("normalize_bundle",changed(f.bundle,"jobs",array_value(jobs)),"reference");
 CHECK(e->fields.at("file")->string=="aaa.lua"); CHECK(e->fields.at("field")->string=="job");
}
