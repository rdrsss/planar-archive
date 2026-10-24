// Complete trusted interpretation, immutable canonical storage and historical decoding boundaries.
#include <catch2/catch_test_macros.hpp>
import std;
import planar.engine.canonical;
import planar.engine.history.interpretation_codec;
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
recorded_interpretation prepared() {
  const auto root = std::filesystem::path(PLANAR_INTERPRETATION_GOLDENS);
  auto result     = decode_interpretation(read_bytes(root / "interpretation.canonical"), read_bytes(root / "versions.canonical"));
  REQUIRE(result);
  return *result;
}
std::string encoded(value_ptr v) {
  auto result = canonical_encode(v);
  REQUIRE(result);
  return *result;
}
void check_decode_error(value_ptr envelope, value_ptr versions, interpretation_error_code code) {
  auto result = decode_interpretation(encoded(envelope), encoded(versions));
  REQUIRE_FALSE(result);
  CHECK(result.error().code == code);
  CHECK_FALSE(result.error().validation);
  CHECK(result.error().detail.message.empty());
}
} // namespace
TEST_CASE("interpretation codec refuses malformed fixed envelopes types containers ordering and correspondence",
          "[interpretation_codec]") {
  auto result = prepared();
  auto v = result.interpretation, versions = result.versions;
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
TEST_CASE("interpretation codec distinguishes supported historical envelopes from future policy versions",
          "[interpretation_codec]") {
  auto r = prepared();
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
TEST_CASE("interpretation codec canonical storage preserves containers signed32 and exact byte ceiling",
          "[interpretation_codec]") {
  auto r = prepared();
  auto b = r.interpretation->fields.at("bundle"), spec = b->fields.at("spec");
  auto values  = record_value({{"map", record_value()},
                               {"list", array_value()},
                               {"min", integer_value(-2147483648LL)},
                               {"max", integer_value(2147483647)}});
  auto v       = changed(r.interpretation, "bundle", changed(b, "spec", changed(spec, "old_extension", values)));
  auto decoded = decode_interpretation(encoded(v), r.versions_encoded);
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
TEST_CASE("interpretation codec assembly matches saved canonical fixtures exactly", "[interpretation_codec]") {
  auto saved     = prepared();
  auto envelope  = saved.interpretation;
  auto digest    = saved.versions->fields.at("stdlib_digest")->string;
  auto assembled = encode_interpretation(envelope->fields.at("bundle"), envelope->fields.at("semantic"), digest);
  REQUIRE(assembled);
  CHECK(assembled->encoded == saved.encoded);
  CHECK(assembled->versions_encoded == saved.versions_encoded);
  CHECK(encoded(assembled->interpretation) == saved.encoded);
  CHECK(encoded(assembled->versions) == saved.versions_encoded);
  // Digest data and historical policy survive without loading current policy or code.
  auto bundle     = envelope->fields.at("bundle");
  bundle          = changed(bundle, "spec", changed(bundle->fields.at("spec"), "historical_policy", string_value("opaque")));
  auto historical = encode_interpretation(bundle, envelope->fields.at("semantic"), std::string(32, '0'));
  REQUIRE(historical);
  CHECK(historical->versions->fields.at("stdlib_digest")->string == std::string(32, '0'));
  CHECK(historical->interpretation->fields.at("bundle")->fields.at("spec")->fields.contains("historical_policy"));
  for (auto invalid : {"", "ABCDEF0123456789abcdef0123456789ab", "0123456789abcdef0123456789abcdeg"}) {
    auto result = encode_interpretation(bundle, envelope->fields.at("semantic"), invalid);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == interpretation_error_code::malformed);
  }
  auto malformed = encode_interpretation({}, {}, digest);
  REQUIRE_FALSE(malformed);
  CHECK(malformed.error().code == interpretation_error_code::malformed);
  auto excessive = changed(
      bundle, "spec", changed(bundle->fields.at("spec"), "historical_policy", string_value(std::string(16 * 1024 * 1024, 'x'))));
  auto bounded = encode_interpretation(excessive, envelope->fields.at("semantic"), digest);
  REQUIRE_FALSE(bounded);
  CHECK(bounded.error().code == interpretation_error_code::resource_limit);
}
