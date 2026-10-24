// Complete trusted interpretation, immutable canonical storage and historical decoding boundaries.
#include <catch2/catch_test_macros.hpp>
#include <unistd.h>
import std;
import planar.engine.runtime;
import planar.engine.json_codec;
import planar.engine.capture;
import planar.engine.canonical;
import planar.engine.history.interpretation;
import planar.engine.history.tree_core;
using namespace planar::engine::history;
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
    std::filesystem::copy(std::filesystem::path(PLANAR_INTERPRETATION_FIXTURE) / "workspace", root / "workspace",
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
    bundle = record_value(
        {{"workspace", record_value({{"file", string_value("workspace.lua")}, {"value", workspace}})},
         {"spec", record_value({{"file", route->fields.at("spec")}, {"value", spec->root}})},
         {"jobs", array_value(jobs)},
         {"registry", record_value({{"file", string_value("input.json")},
                                    {"bytes", string_value(read_bytes(std::filesystem::path(PLANAR_INTERPRETATION_FIXTURE) /
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

namespace {
std::string encoded(value_ptr v) {
  auto result = canonical_encode(v);
  REQUIRE(result);
  return *result;
}
value_ptr sorted_jobs(value_ptr list, bool normalized) {
  auto v = std::make_shared<value>(*list);
  std::ranges::sort(v->items, {},
                    [&](auto& entry) { return (normalized ? entry->fields.at("value") : entry)->fields.at("id")->string; });
  return v;
}
std::string json(value_ptr v) {
  auto result = codec_encode(v);
  REQUIRE(result);
  return *result;
}
recorded_interpretation prepared(fixture& f) {
  auto result = prepare_interpretation(f.bundle);
  if (!result) {
    INFO(static_cast<int>(result.error().code));
    if (result.error().validation)
      INFO(json(result.error().validation));
  }
  REQUIRE(result);
  return result->recorded;
}
void check_decode_error(value_ptr envelope, value_ptr versions, interpretation_error_code code) {
  auto result = decode_interpretation(encoded(envelope), encoded(versions));
  REQUIRE_FALSE(result);
  CHECK(result.error().code == code);
  CHECK_FALSE(result.error().validation);
  CHECK(result.error().detail.message.empty());
}
} // namespace
TEST_CASE("interpretation complete quota outputs match reviewed JSON and canonical goldens", "[interpretation]") {
  fixture    f;
  auto       result = prepared(f);
  const auto root   = std::filesystem::path(PLANAR_INTERPRETATION_GOLDENS);
  // Only the exact embedded provenance changes; historical golden bytes remain immutable.
  const std::array originals{"planar/json.lua",         "planar/schema.lua",       "planar/semantic.lua", "planar/selection.lua",
                             "planar/presentation.lua", "planar/observations.lua", "planar/render.lua",   "planar/acceptance.lua",
                             "planar/commands.lua",     "planar/bootstrap.lua"};
  std::vector<tree_file> authored;
  std::vector<value_ptr> names;
  std::string            assembly;
  for (auto path : originals) {
    auto bytes = read_bytes(std::filesystem::path(PLANAR_STDLIB_DIR) / path);
    assembly += bytes;
    assembly += '\n';
    names.push_back(string_value(path));
    authored.push_back({path, std::move(bytes), 420});
  }
  auto manifest = record_value({{"version", integer_value(1)},
                                {"scheme", string_value("ordered_concat_v1")},
                                {"files", array_value(names)},
                                {"separator", string_value("\n")}});
  authored.push_back({"__planar__/assembly.canonical", encoded(manifest), 420});
  authored.push_back({"__planar__/assembly.lua", assembly, 420});
  std::vector<tree_file_view> views;
  for (auto& file : authored)
    views.push_back({file.path, file.payload, file.mode});
  auto stdlib = digest_tree(views, tree_profile::provenance);
  REQUIRE(stdlib.has_value());
  auto old_interpretation = canonical_decode(read_bytes(root / "interpretation.canonical"));
  auto old_versions       = canonical_decode(read_bytes(root / "versions.canonical"));
  REQUIRE(old_interpretation.has_value());
  REQUIRE(old_versions.has_value());
  CHECK(json(*old_interpretation) + "\n" == read_bytes(root / "interpretation.json"));
  CHECK(json(*old_versions) + "\n" == read_bytes(root / "versions.json"));
  std::string current_digest;
  for (auto byte : *stdlib)
    current_digest += std::format("{:02x}", byte);
  auto expected_versions       = changed(*old_versions, "stdlib_digest", string_value(current_digest));
  auto expected_interpretation = changed(*old_interpretation, "versions", expected_versions);
  CHECK(json(result.interpretation) == json(expected_interpretation));
  CHECK(json(result.versions) == json(expected_versions));
  CHECK(result.encoded == encoded(expected_interpretation));
  CHECK(result.versions_encoded == encoded(expected_versions));
  auto decoded = decode_interpretation(result.encoded, result.versions_encoded);
  REQUIRE(decoded);
  CHECK(encoded(decoded->interpretation) == result.encoded);
  CHECK(encoded(decoded->versions) == result.versions_encoded);
}
TEST_CASE("interpretation preserves every existing normalized and semantic field and captured provenance", "[interpretation]") {
  fixture f;
  auto    result = prepare_interpretation(f.bundle);
  REQUIRE(result);
  auto b          = result->recorded.interpretation->fields.at("bundle");
  auto normalized = success("normalize_bundle", f.bundle);
  normalized      = changed(normalized, "jobs", sorted_jobs(normalized->fields.at("jobs"), true));
  CHECK(encoded(b) == encoded(normalized));
  auto validated =
      success("validate_bundle",
              record_value({{"bundle", f.bundle}, {"capture", record_value({{"status", string_value("not_captured")}})}}));
  auto semantic = record_value(
      {{"dependencies", validated->fields.at("dependencies")}, {"jobs", sorted_jobs(validated->fields.at("jobs"), false)}});
  CHECK(encoded(result->recorded.interpretation->fields.at("semantic")) == encoded(semantic));
  REQUIRE(b->fields.at("jobs")->items.size() == 2);
  CHECK(b->fields.at("jobs")->items[0]->fields.at("value")->fields.at("id")->string == "combined");
  CHECK(b->fields.at("jobs")->items[1]->fields.at("value")->fields.at("id")->string == "split");
  auto expected = b->fields.at("spec")->fields.at("contracts")->items[0]->fields.at("expected");
  CHECK(expected->fields.at("bytes")->string == read_bytes(f.root / "workspace/text/quota-api.md"));
  auto registry = success("parse_json", record_value({{"file", string_value("input.json")},
                                                      {"bytes", f.bundle->fields.at("registry")->fields.at("bytes")}}));
  auto checks   = registry->fields.at("checks")->items;
  for (auto& check : checks) {
    auto inputs = check->fields.at("inputs")->items;
    std::ranges::sort(inputs, {}, [](auto& input) { return input->string; });
    check = changed(check, "inputs", array_value(inputs));
  }
  registry = changed(registry, "checks", array_value(checks));
  CHECK(encoded(b->fields.at("registry")) == encoded(registry));
  auto registry_tree = prepare_registry(f.bundle->fields.at("registry")->fields.at("bytes")->string, 420);
  REQUIRE(registry_tree);
  CHECK(registry_tree->files[0].payload ==
        read_bytes(std::filesystem::path(PLANAR_INTERPRETATION_FIXTURE) / "registry/quota-tools.json"));
  auto stdlib = prepare_embedded_stdlib();
  REQUIRE(stdlib);
  CHECK(result->stdlib.digest == stdlib->digest);
  std::string digest;
  for (auto byte : stdlib->digest)
    digest += std::format("{:02x}", byte);
  CHECK(result->recorded.versions->fields.at("stdlib_digest")->string == digest);
}
TEST_CASE("interpretation orders all jobs independently of workspace routing and captured order", "[interpretation]") {
  fixture f;
  auto    before = prepared(f);
  auto    jobs   = f.bundle->fields.at("jobs")->items;
  std::ranges::reverse(jobs);
  auto workspace = f.bundle->fields.at("workspace");
  auto w         = workspace->fields.at("value");
  auto routes    = w->fields.at("jobs")->items;
  std::ranges::reverse(routes);
  f.bundle   = changed(changed(f.bundle, "jobs", array_value(jobs)), "workspace",
                       changed(workspace, "value", changed(w, "jobs", array_value(routes))));
  auto after = prepared(f);
  CHECK(before.encoded == after.encoded);
  CHECK(before.versions_encoded == after.versions_encoded);
}
TEST_CASE("interpretation keeps live schema and ownership diagnostics with logical locations", "[interpretation]") {
  fixture f;
  auto    invalid = f.spec_changed("goal", string_value("wrong constructor"));
  auto    result  = prepare_interpretation(invalid);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == interpretation_error_code::validation);
  REQUIRE(result.error().validation);
  CHECK(result.error().validation->fields.at("file")->string == "spec.lua");
  CHECK(result.error().validation->fields.at("code")->string == "schema");
  auto existing = invoke("interpret_bundle", invalid);
  CHECK(encoded(result.error().validation) == encoded(existing->fields.at("error")));
  auto spec = f.bundle->fields.at("spec")->fields.at("value");
  auto work = spec->fields.at("work")->items;
  work[1]   = changed(work[1], "paths", work[0]->fields.at("paths"));
  result    = prepare_interpretation(f.spec_changed("work", array_value(work)));
  REQUIRE_FALSE(result);
  CHECK(result.error().code == interpretation_error_code::validation);
  CHECK(result.error().validation->fields.at("code")->string == "ownership");
  CHECK(result.error().validation->fields.at("file")->string == "spec.lua");
}
TEST_CASE("interpretation refuses malformed fixed envelopes types containers ordering and correspondence", "[interpretation]") {
  fixture f;
  auto    result = prepared(f);
  auto    v = result.interpretation, versions = result.versions;
  for (auto key : {"version", "bundle", "semantic", "versions"})
    check_decode_error(changed(v, key, nullptr), versions, interpretation_error_code::malformed);
  for (auto key : {"declarations", "normalization", "semantics", "selection", "normalizer", "canonical", "hash", "interpretation",
                   "projection", "stdlib_digest"}) {
    auto bad = changed(versions, key, nullptr);
    check_decode_error(changed(v, "versions", bad), bad, interpretation_error_code::malformed);
    bad = changed(versions, key, integer_value(1));
    check_decode_error(changed(v, "versions", bad), bad, interpretation_error_code::malformed);
  }
  check_decode_error(changed(v, "extra", integer_value(1)), versions, interpretation_error_code::malformed);
  auto bad = changed(versions, "extra", string_value("x"));
  check_decode_error(changed(v, "versions", bad), bad, interpretation_error_code::malformed);
  check_decode_error(changed(v, "version", string_value("1")), versions, interpretation_error_code::malformed);
  auto b = v->fields.at("bundle"), s = v->fields.at("semantic");
  for (auto key : {"workspace", "spec", "registry"})
    check_decode_error(changed(v, "bundle", changed(b, key, array_value())), versions, interpretation_error_code::malformed);
  for (auto key : {"jobs", "dependencies"})
    check_decode_error(changed(v, "semantic", changed(s, key, record_value())), versions, interpretation_error_code::malformed);
  auto spec = b->fields.at("spec");
  for (auto key : {"work", "questions", "contracts"})
    check_decode_error(changed(v, "bundle", changed(b, "spec", changed(spec, key, record_value()))), versions,
                       interpretation_error_code::malformed);
  auto jobs = b->fields.at("jobs")->items;
  std::ranges::reverse(jobs);
  check_decode_error(changed(v, "bundle", changed(b, "jobs", array_value(jobs))), versions, interpretation_error_code::malformed);
  jobs = s->fields.at("jobs")->items;
  jobs.pop_back();
  check_decode_error(changed(v, "semantic", changed(s, "jobs", array_value(jobs))), versions,
                     interpretation_error_code::malformed);
  jobs    = s->fields.at("jobs")->items;
  jobs[0] = changed(jobs[0], "file", string_value("wrong.lua"));
  check_decode_error(changed(v, "semantic", changed(s, "jobs", array_value(jobs))), versions,
                     interpretation_error_code::malformed);
  check_decode_error(changed(v, "versions", record_value()), versions, interpretation_error_code::malformed);
}
TEST_CASE("interpretation distinguishes supported historical envelopes from future policy versions", "[interpretation]") {
  fixture f;
  auto    r = prepared(f);
  check_decode_error(changed(r.interpretation, "version", integer_value(2)), r.versions,
                     interpretation_error_code::unsupported_version);
  for (auto key : {"declarations", "normalization", "semantics", "selection", "normalizer", "canonical", "hash", "interpretation",
                   "projection"}) {
    auto versions = changed(r.versions, key, string_value("future_v9"));
    check_decode_error(changed(r.interpretation, "versions", versions), versions, interpretation_error_code::unsupported_version);
  }
  for (auto digest : {"ABCDEF0123456789abcdef0123456789ab", "xyz", "0123456789abcdef0123456789abcdeg"}) {
    auto versions = changed(r.versions, "stdlib_digest", string_value(digest));
    check_decode_error(changed(r.interpretation, "versions", versions), versions, interpretation_error_code::malformed);
  }
  auto versions = changed(r.versions, "stdlib_digest", string_value(std::string(32, '0')));
  auto b        = r.interpretation->fields.at("bundle");
  // Recorded historical policy values need not satisfy today's declaration schemas.
  b               = changed(b, "spec", changed(b->fields.at("spec"), "historical_policy", string_value("opaque")));
  auto historical = changed(changed(r.interpretation, "versions", versions), "bundle", b);
  auto decoded    = decode_interpretation(encoded(historical), encoded(versions));
  REQUIRE(decoded);
  CHECK(encoded(decoded->interpretation) == encoded(historical));
}
TEST_CASE("interpretation canonical storage preserves containers signed32 and exact byte ceiling", "[interpretation]") {
  fixture f;
  auto    r = prepared(f);
  auto    b = r.interpretation->fields.at("bundle"), spec = b->fields.at("spec");
  auto    values  = record_value({{"map", record_value()},
                                  {"list", array_value()},
                                  {"min", integer_value(-2147483648LL)},
                                  {"max", integer_value(2147483647)}});
  auto    v       = changed(r.interpretation, "bundle", changed(b, "spec", changed(spec, "old_extension", values)));
  auto    decoded = decode_interpretation(encoded(v), r.versions_encoded);
  REQUIRE(decoded);
  CHECK(encoded(decoded->interpretation) == encoded(v));
  auto restored = decoded->interpretation->fields.at("bundle")->fields.at("spec")->fields.at("old_extension");
  CHECK(restored->fields.at("map")->kind == value_kind::record);
  CHECK(restored->fields.at("list")->kind == value_kind::list);
  for (auto n : {-2147483649LL, 2147483648LL})
    CHECK_FALSE(canonical_encode(integer_value(n)));
  v             = changed(r.interpretation, "bundle", changed(b, "spec", changed(spec, "old_extension", string_value(""))));
  auto overhead = encoded(v).size();
  auto bytes    = std::string(16 * 1024 * 1024 - overhead, 'x');
  v             = changed(r.interpretation, "bundle", changed(b, "spec", changed(spec, "old_extension", string_value(bytes))));
  auto exact    = encoded(v);
  CHECK(exact.size() == 16 * 1024 * 1024);
  CHECK(decode_interpretation(exact, r.versions_encoded));
  bytes += 'x';
  v              = changed(r.interpretation, "bundle", changed(b, "spec", changed(spec, "old_extension", string_value(bytes))));
  auto excessive = decode_interpretation(encoded(v), r.versions_encoded);
  REQUIRE_FALSE(excessive);
  CHECK(excessive.error().code == interpretation_error_code::resource_limit);
  auto corrupt = r.encoded + 'x';
  auto bad     = decode_interpretation(corrupt, r.versions_encoded);
  REQUIRE_FALSE(bad);
  CHECK(bad.error().code == interpretation_error_code::malformed);
}
TEST_CASE("interpretation stored record roundtrip reopens exact historical bytes without Lua", "[interpretation]") {
  const auto goldens = std::filesystem::path(PLANAR_INTERPRETATION_GOLDENS);
  auto       historical =
      decode_interpretation(read_bytes(goldens / "interpretation.canonical"), read_bytes(goldens / "versions.canonical"));
  REQUIRE(historical);
  auto r         = std::move(*historical);
  char pattern[] = "/tmp/planar-interpretation-records-XXXXXX";
  REQUIRE(mkdtemp(pattern));
  const auto root = std::filesystem::canonical(pattern);
  struct cleanup {
    std::filesystem::path root;
    ~cleanup() {
      std::filesystem::remove_all(root);
    }
  } cleanup_root{root};
  const auto path = root / "records.db";
  REQUIRE(ledger::initialize_new(path, "interpretation_test"));
  auto writer = ledger::begin_records(path);
  REQUIRE(writer);
  auto i = writer->intern(value_record{"canonical_v1", r.encoded});
  REQUIRE(i);
  auto v = writer->intern(value_record{"canonical_v1", r.versions_encoded});
  REQUIRE(v);
  REQUIRE(writer->commit());
  auto reader = ledger::open_existing(path);
  REQUIRE(reader);
  auto ir = reader->read_record(i->id), vr = reader->read_record(v->id);
  REQUIRE(ir);
  REQUIRE(vr);
  auto decoded = decode_interpretation(std::get<value_record>(ir->value).encoded, std::get<value_record>(vr->value).encoded);
  REQUIRE(decoded);
  CHECK(decoded->encoded == r.encoded);
  CHECK(decoded->versions_encoded == r.versions_encoded);
  CHECK(encoded(decoded->interpretation) == r.encoded);
}

TEST_CASE("interpretation separates native runtime resources from trusted validation", "[interpretation]") {
  auto invalid = prepare_interpretation({});
  REQUIRE_FALSE(invalid);
  CHECK(invalid.error().code == interpretation_error_code::runtime);
  CHECK_FALSE(invalid.error().validation);
  value_ptr nested = array_value();
  for (unsigned i = 0; i < 257; ++i)
    nested = array_value({nested});
  auto bounded = prepare_interpretation(nested);
  REQUIRE_FALSE(bounded);
  CHECK(bounded.error().code == interpretation_error_code::resource_limit);
  CHECK(bounded.error().detail.code == "resource_limit");
  CHECK_FALSE(bounded.error().validation);
  fixture f;
  auto    unsupported = prepare_interpretation(f.spec_changed("version", integer_value(2)));
  REQUIRE_FALSE(unsupported);
  CHECK(unsupported.error().code == interpretation_error_code::validation);
  REQUIRE(unsupported.error().validation);
  CHECK(unsupported.error().validation->fields.at("code")->string == "unsupported_version");
}

TEST_CASE("interpretation preserves absent optional notes without manufacturing fields", "[interpretation]") {
  fixture f;
  f.bundle       = f.spec_changed("rationale", {});
  auto workspace = f.bundle->fields.at("workspace");
  f.bundle = changed(f.bundle, "workspace", changed(workspace, "value", changed(workspace->fields.at("value"), "notes", {})));
  auto r   = prepared(f);
  auto b   = r.interpretation->fields.at("bundle");
  CHECK_FALSE(b->fields.at("spec")->fields.contains("rationale"));
  CHECK_FALSE(b->fields.at("workspace")->fields.contains("notes"));
}
