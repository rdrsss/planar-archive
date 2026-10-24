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
  CHECK(expected->fields.at("kind")->string == "file");
  CHECK(expected->fields.at("path")->string == "text/quota-api.md");
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
  CHECK(text->fields.at("path")->string == "nested/text/shared.md");
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
    CHECK(notes->fields.at("path")->string == "jobs/" + std::to_string(i) + "/note.md");
    CHECK(notes->fields.at("bytes")->string == "job " + std::to_string(i) + "\r\n ");
  }
}

TEST_CASE("review JSON exact container depth boundary", "[review]") {
  for (int depth : {127,128}) {
    auto bytes=std::string(depth,'[')+"0"+std::string(depth,']');
    success("parse_json",record_value({{"bytes",string_value(bytes)},{"file",string_value("input.json")}}));
  }
  auto nonempty=std::string(129,'[')+"0"+std::string(129,']');
  refusal("parse_json",record_value({{"bytes",string_value(nonempty)},{"file",string_value("input.json")}}),"resource_limit");
  auto empty=std::string(129,'[')+std::string(129,']');
  refusal("parse_json",record_value({{"bytes",string_value(empty)},{"file",string_value("input.json")}}),"resource_limit");
}
TEST_CASE("review normalization matches render contract text identity", "[review]") {
  fixture f;
  auto normalized=success("normalize_bundle",f.bundle);
  auto inline_text=normalized->fields.at("spec")->fields.at("goal");
  CHECK(inline_text->fields.at("kind")->string=="text");
  auto file_text=normalized->fields.at("spec")->fields.at("contracts")->items[0]->fields.at("expected");
  CHECK(file_text->fields.at("kind")->string=="text");
  CHECK(file_text->fields.contains("file"));
}
TEST_CASE("review strict JSON additional integer syntax and scalars", "[review]") {
  for(auto bytes:{"-2147483648","2147483647","-2147483647","2147483646","0","-0","true","false","\"\\u0001\"","{\"a\":false}","[[],{}]"})
    success("parse_json",record_value({{"bytes",string_value(bytes)},{"file",string_value("input.json")}}));
  for(auto bytes:{"21474836470","-21474836480","9999999999999999999999999999999999","--1","-","0e0","-00","[true false]","{\"a\":false,\"a\":true}","\"\\uDFFF\"","\"\\uD800\\uD800\""})
    refusal("parse_json",record_value({{"bytes",string_value(bytes)},{"file",string_value("input.json")}}),"schema");
}

TEST_CASE("review ID-keyed collections normalize by ID", "[review]") {
  fixture f;
  auto normalized=success("normalize_bundle",f.bundle);
  auto work=normalized->fields.at("spec")->fields.at("work");
  REQUIRE(work->items.size()==2);
  CHECK(work->items[0]->fields.at("id")->string=="command");
  CHECK(work->items[1]->fields.at("id")->string=="library");
  auto reordered=f.bundle->fields.at("spec")->fields.at("value")->fields.at("work")->items;
  std::reverse(reordered.begin(),reordered.end());
  auto second=success("normalize_bundle",f.spec_changed("work",array_value(reordered)));
  CHECK(second->fields.at("spec")->fields.at("work")->items[0]->fields.at("id")->string==work->items[0]->fields.at("id")->string);
}
TEST_CASE("review resolved job paths stay within portable limit", "[review]") {
  fixture f;
  std::string prefix;
  for(int i=0;i<7;++i) prefix+=std::string(128,'a')+"/";
  auto listed=f.workspace->fields.at("jobs")->items;
  listed[0]=changed(listed[0],"file",string_value(std::string(128,'b')));
  auto w=changed(f.workspace,"jobs",array_value(listed));
  auto b=changed(f.bundle,"workspace",record_value({{"file",string_value(prefix+"workspace.lua")},{"value",w}}));
  auto result=invoke("normalize_bundle",b);
  REQUIRE(result->fields.contains("error"));
  auto error=result->fields.at("error");
  INFO(error->fields.at("file")->string.size());
  CHECK(error->fields.at("file")->string.size()<=1024);
  CHECK(error->fields.at("code")->string=="path_denied");
}
