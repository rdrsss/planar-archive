// Independent request-v3 frame vectors, digests, strict decode refusals, operation and profile rules,
// the 64 MiB frame bound, and separation from request-v2 frames.
#include <catch2/catch_test_macros.hpp>
#include <xxhash.h>
import std;
import planar.engine.canonical;
import planar.engine.history.request_v3;
using namespace planar::engine;
using namespace planar::engine::history;
namespace {
constexpr std::size_t maximum = 64 * 1024 * 1024;

void wide(std::string& bytes, std::uint64_t n) {
  for (int shift = 56; shift >= 0; shift -= 8)
    bytes += static_cast<char>((n >> shift) & 255);
}

void field(std::string& bytes, std::string_view part) {
  wide(bytes, part.size());
  bytes.append(part);
}

// Magic, version byte 03 and the three length-prefixed fields; callers append the selector.
std::string head(std::string_view operation, std::string_view actor, std::string_view parameters) {
  std::string bytes = std::string("PLANAR-REQUEST") + '\x03';
  field(bytes, operation);
  field(bytes, actor);
  field(bytes, parameters);
  return bytes;
}

// Hand-built complete frame: selector 00 without a profile, or selector 01 with a length-prefixed profile.
std::string frame(std::string_view operation, std::string_view actor, std::string_view parameters,
                  std::optional<std::string_view> profile = std::nullopt) {
  auto bytes = head(operation, actor, parameters);
  if (!profile) {
    bytes += '\0';
    return bytes;
  }
  bytes += '\x01';
  field(bytes, *profile);
  return bytes;
}

value_ptr string_value(std::string s) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = std::move(s);
  return v;
}

std::string canonical(const value_ptr& v) {
  return canonical_encode(v).value_or(std::string());
}

// Canonical_v1 bytes of the record { workspace: <text> }, the parameter map of every intent here.
std::string parameters(std::string_view workspace) {
  auto root                 = std::make_shared<value>();
  root->kind                = value_kind::record;
  root->fields["workspace"] = string_value(std::string(workspace));
  return canonical(root);
}

// Seed-zero XXH3-128 of the exact bytes in canonical big-endian order.
record_digest xxh3(std::string_view bytes) {
  XXH128_canonical_t form;
  XXH128_canonicalFromHash(&form, XXH3_128bits(bytes.data(), bytes.size()));
  record_digest result{};
  std::ranges::copy(form.digest, result.begin());
  return result;
}
} // namespace

TEST_CASE("request v3 independent complete frame vectors", "[request_v3]") {
  const auto params         = parameters("/workspace");
  const auto embedded_bytes = frame("activate", "driver:é", params);
  REQUIRE(embedded_bytes.size() > 15);
  CHECK(std::string_view(embedded_bytes).substr(0, 15) == std::string_view("PLANAR-REQUEST\x03", 15));
  CHECK(embedded_bytes.back() == '\0');
  request_v3 embedded{"activate", "driver:é", params, std::nullopt};
  auto       encoded = encode_request_v3(embedded);
  REQUIRE(encoded);
  CHECK(*encoded == embedded_bytes);
  auto decoded = decode_request_v3(embedded_bytes);
  REQUIRE(decoded);
  CHECK(decoded->operation == "activate");
  CHECK(decoded->actor == "driver:é");
  CHECK(decoded->parameters == params);
  CHECK_FALSE(decoded->profile_argument);

  const auto profile_bytes = frame("claim", "driver", params, std::string_view("/private/profiles/é.json"));
  request_v3 profiled{"claim", "driver", params, std::string("/private/profiles/é.json")};
  auto       encoded_profile = encode_request_v3(profiled);
  REQUIRE(encoded_profile);
  CHECK(*encoded_profile == profile_bytes);
  auto decoded_profile = decode_request_v3(profile_bytes);
  REQUIRE(decoded_profile);
  CHECK(decoded_profile->operation == "claim");
  CHECK(decoded_profile->profile_argument == std::optional<std::string>("/private/profiles/é.json"));
  CHECK(encode_request_v3(*decoded_profile).value_or(std::string()) == profile_bytes);
}

TEST_CASE("request v3 digest covers the complete frame", "[request_v3]") {
  const request_v3 base{"activate", "driver", parameters("/w"), std::string("/p")};
  auto             digest = digest_request_v3(base);
  REQUIRE(digest);
  CHECK(*digest == xxh3(frame("activate", "driver", parameters("/w"), std::string_view("/p"))));

  request_v3 operation      = base;
  operation.operation       = "claim";
  request_v3 actor          = base;
  actor.actor               = "driver2";
  request_v3 params         = base;
  params.parameters         = parameters("/x");
  request_v3 profile        = base;
  profile.profile_argument  = std::string("/q");
  request_v3 embedded       = base;
  embedded.profile_argument = std::nullopt;
  for (const auto* variant : {&operation, &actor, &params, &profile, &embedded}) {
    auto changed = digest_request_v3(*variant);
    REQUIRE(changed);
    CHECK(*changed != *digest);
  }
}

TEST_CASE("request v3 strict decode refusals", "[request_v3]") {
  const auto params = parameters("/w");
  const auto good   = frame("activate", "driver", params, std::string_view("/p"));
  REQUIRE(decode_request_v3(good));
  REQUIRE(decode_request_v3(frame("activate", "driver", params)));
  for (std::size_t i = 0; i < good.size(); ++i)
    CHECK_FALSE(decode_request_v3(std::string_view(good).substr(0, i)));
  CHECK_FALSE(decode_request_v3(good + '\0'));
  CHECK_FALSE(decode_request_v3(good + 'x'));

  auto bad_selector = head("activate", "driver", params);
  bad_selector += '\x02';
  CHECK_FALSE(decode_request_v3(bad_selector));

  CHECK_FALSE(decode_request_v3(frame("activate", "", params)));
  CHECK_FALSE(decode_request_v3(frame("activate", std::string(257, 'a'), params)));
  CHECK_FALSE(decode_request_v3(frame("activate", std::string("a\0b", 3), params)));
  REQUIRE(decode_request_v3(frame("activate", std::string(256, 'a'), params)));

  CHECK_FALSE(decode_request_v3(frame("activate", "driver", "not canonical")));
  CHECK_FALSE(decode_request_v3(frame("activate", "driver", canonical(string_value("x")))));
  CHECK_FALSE(decode_request_v3(frame("activate", "driver", params + '\0')));

  std::string overlong = std::string("PLANAR-REQUEST") + '\x03';
  wide(overlong, 18446744073709551615ULL);
  CHECK_FALSE(decode_request_v3(overlong));
}

TEST_CASE("request v3 operation set excludes load", "[request_v3]") {
  for (auto operation :
       {"activate", "claim", "takeover", "refresh", "checkpoint", "release", "check", "check-recover", "verdict"}) {
    const request_v3 intent{operation, "driver", parameters("/w"), std::nullopt};
    auto             encoded = encode_request_v3(intent);
    REQUIRE(encoded);
    CHECK(*encoded == frame(operation, "driver", parameters("/w")));
    auto decoded = decode_request_v3(*encoded);
    REQUIRE(decoded);
    CHECK(decoded->operation == operation);
  }
  for (auto operation : {"load", "history", "Activate", "", "activate ", "check-recover-x", "bogus"}) {
    const request_v3 intent{operation, "driver", parameters("/w"), std::nullopt};
    CHECK_FALSE(encode_request_v3(intent));
    CHECK_FALSE(decode_request_v3(frame(operation, "driver", parameters("/w"))));
  }
}

TEST_CASE("request v3 profile selector rules", "[request_v3]") {
  const auto       params = parameters("/w");
  const request_v3 absolute{"verdict", "driver", params, std::string("/a/b/c.json")};
  REQUIRE(encode_request_v3(absolute));
  REQUIRE(decode_request_v3(frame("verdict", "driver", params, std::string_view("/a/b/c.json"))));
  REQUIRE(decode_request_v3(frame("verdict", "driver", params)));
  for (auto profile : {"", "relative", "p/q", "./p", "/a/../p", "/a/./p", "/a//p"}) {
    const request_v3 bad{"verdict", "driver", params, std::string(profile)};
    CHECK_FALSE(encode_request_v3(bad));
    CHECK_FALSE(decode_request_v3(frame("verdict", "driver", params, std::string_view(profile))));
  }
  // Selector 00 carries no profile field, so a trailing profile is trailing bytes.
  auto suffixed = head("verdict", "driver", params);
  suffixed += '\0';
  field(suffixed, "/a");
  CHECK_FALSE(decode_request_v3(suffixed));
  // Selector 01 requires its profile field.
  auto missing = head("verdict", "driver", params);
  missing += '\x01';
  CHECK_FALSE(decode_request_v3(missing));
}

TEST_CASE("request v3 frame bound at limit and limit plus one", "[request_v3]") {
  request_v3 intent{"activate", "driver", parameters("/w"), std::string("/p")};
  auto       small = encode_request_v3(intent);
  REQUIRE(small);
  // Everything except the two profile bytes "/p": head, selector and the profile length field.
  const std::size_t fixed = small->size() - 2;

  const std::string at_limit = "/" + std::string(maximum - fixed - 1, 'a');
  intent.profile_argument    = at_limit;
  auto at                    = encode_request_v3(intent);
  REQUIRE(at);
  CHECK(at->size() == maximum);
  auto decoded = decode_request_v3(*at);
  REQUIRE(decoded);
  CHECK(decoded->profile_argument == std::optional<std::string>(at_limit));

  intent.profile_argument = at_limit + 'a';
  auto over               = encode_request_v3(intent);
  REQUIRE_FALSE(over);
  CHECK(over.error().code == record_error_code::resource_limit);

  const auto over_bytes = frame("activate", "driver", parameters("/w"), std::string_view(at_limit + 'a'));
  CHECK(over_bytes.size() == maximum + 1);
  auto over_decoded = decode_request_v3(over_bytes);
  REQUIRE_FALSE(over_decoded);
  CHECK(over_decoded.error().code == record_error_code::resource_limit);
}

TEST_CASE("request v3 and request v2 frames never decode as each other", "[request_v3]") {
  const auto params = parameters("/w");
  const auto v3     = frame("activate", "driver", params);
  REQUIRE(decode_request_v3(v3));

  // Hand-built request_v2 frame: magic, version byte 02, load, actor, expected head, workspace, registry, selector 00.
  std::string v2 = std::string("PLANAR-REQUEST") + '\x02';
  for (std::string_view part : {std::string_view("load"), std::string_view("driver"), std::string_view("none"),
                                std::string_view("/w.lua"), std::string_view("/r.lua")})
    field(v2, part);
  v2 += '\0';
  CHECK_FALSE(decode_request_v3(v2));

  for (char version : {'\0', '\x01', '\x02', '\x04', '\xff'}) {
    auto bad = v3;
    bad[14]  = version;
    CHECK_FALSE(decode_request_v3(bad));
  }
}
