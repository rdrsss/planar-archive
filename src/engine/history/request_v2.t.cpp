// Independent request-v2 bytes and full hashes, strict refusal and exact bounds.
#include <catch2/catch_test_macros.hpp>
import std;
import planar.engine.history.request_v2;
using namespace planar::engine::history;
namespace {
request_v2 intent() {
  return {"load", "driver:é", "none", "../workspace.lua", "./registry.lua", std::nullopt};
}
std::string unhex(std::string_view s) {
  std::string out;
  for (std::size_t i = 0; i < s.size(); i += 2)
    out += static_cast<char>(std::stoul(std::string(s.substr(i, 2)), nullptr, 16));
  return out;
}
std::string hex(const record_digest& digest) {
  std::string out;
  for (auto b : digest)
    out += std::format("{:02x}", b);
  return out;
}
void wide(std::string& bytes, std::uint64_t n) {
  for (int shift = 56; shift >= 0; shift -= 8)
    bytes += static_cast<char>((n >> shift) & 255);
}
// Deliberately bypass validation to exercise semantic refusal through the wire reader.
std::string raw_frame(const request_v2& v) {
  std::string bytes  = "PLANAR-REQUEST\x02";
  auto        append = [&](std::string_view part) {
    wide(bytes, part.size());
    bytes.append(part);
  };
  for (std::string_view part : {std::string_view(v.operation), std::string_view(v.actor), std::string_view(v.expected_head),
                                std::string_view(v.workspace_argument), std::string_view(v.registry_argument)})
    append(part);
  bytes += v.profile_argument ? '\x01' : '\0';
  if (v.profile_argument)
    append(*v.profile_argument);
  return bytes;
}
} // namespace
TEST_CASE("request v2 independent complete frame and digest vectors", "[request_v2]") {
  std::ifstream input(std::string(PLANAR_RECORD_VECTOR_DIR) + "/request-v2.tsv");
  REQUIRE(input);
  std::string line;
  unsigned    count = 0;
  while (std::getline(input, line)) {
    if (line.empty() || line.front() == '#')
      continue;
    std::istringstream fields(line);
    std::string        name, bytes_hex, digest_hex;
    REQUIRE(std::getline(fields, name, '\t'));
    REQUIRE(std::getline(fields, bytes_hex, '\t'));
    REQUIRE(std::getline(fields, digest_hex));
    INFO(name);
    auto v = intent();
    if (name == "profile-maximum") {
      v.actor            = "actor";
      v.expected_head    = "9223372036854775807";
      v.profile_argument = "/private/profiles/é.json";
    }
    auto bytes = unhex(bytes_hex);
    CHECK(encode_request_v2(v).value() == bytes);
    CHECK(hex(digest_request_v2(v).value()) == digest_hex);
    auto decoded = decode_request_v2(bytes);
    REQUIRE(decoded);
    CHECK(encode_request_v2(*decoded).value() == bytes);
    CHECK(decoded->operation == v.operation);
    CHECK(decoded->actor == v.actor);
    CHECK(decoded->expected_head == v.expected_head);
    CHECK(decoded->workspace_argument == v.workspace_argument);
    CHECK(decoded->registry_argument == v.registry_argument);
    CHECK(decoded->profile_argument == v.profile_argument);
    for (std::size_t i = 0; i < bytes.size(); ++i)
      CHECK_FALSE(decode_request_v2(std::string_view(bytes).substr(0, i)));
    CHECK_FALSE(decode_request_v2(bytes + '\0'));
    request_record old{v.operation, v.actor, v.expected_head, v.workspace_argument, v.registry_argument};
    CHECK(encode_record(old).value() != bytes);
    CHECK(digest_record(old).value() != digest_request_v2(v).value());
    CHECK_FALSE(decode_record(bytes));
    CHECK_FALSE(decode_request_v2(encode_record(old).value()));
    ++count;
  }
  CHECK(count == 2);
}
TEST_CASE("request v2 refuses malformed framing tags versions and oversized claims", "[request_v2]") {
  auto bytes = encode_request_v2(intent()).value();
  for (auto version : {0, 1, 3, 255}) {
    auto bad = bytes;
    bad[14]  = static_cast<char>(version);
    CHECK_FALSE(decode_request_v2(bad));
  }
  for (auto tag : {2, 127, 255}) {
    auto bad   = bytes;
    bad.back() = static_cast<char>(tag);
    CHECK_FALSE(decode_request_v2(bad));
  }
  auto bad = bytes;
  bad[0]   = 'x';
  CHECK_FALSE(decode_request_v2(bad));
  bad        = bytes;
  bad.back() = '\x01';
  CHECK_FALSE(decode_request_v2(bad));
  bad = "PLANAR-REQUEST\x02";
  wide(bad, 18446744073709551615ULL);
  CHECK(decode_request_v2(bad).error().code == record_error_code::resource_limit);
  bad = "PLANAR-REQUEST\x02";
  wide(bad, 64 * 1024 * 1024);
  CHECK(decode_request_v2(bad).error().code == record_error_code::invalid_record);
  // Mutate every field length, including the profile suffix, to an overflowing claim.
  auto v             = intent();
  v.profile_argument = "/profile.json";
  bytes              = encode_request_v2(v).value();
  std::size_t offset = 15;
  for (const auto& part :
       {v.operation, v.actor, v.expected_head, v.workspace_argument, v.registry_argument, *v.profile_argument}) {
    if (offset + 8 > bytes.size())
      FAIL("missing field length");
    bad = bytes;
    bad.replace(offset, 8, 8, '\xff');
    CHECK(decode_request_v2(bad).error().code == record_error_code::resource_limit);
    offset += 8 + part.size();
    if (offset == bytes.size() - 8 - v.profile_argument->size() - 1)
      ++offset;
  }
}
TEST_CASE("request v2 inherits text head bounds and requires normalized absolute profile", "[request_v2]") {
  for (auto field : {0, 1, 2, 3, 4, 5}) {
    for (auto invalid : {std::string(), std::string("\0", 1), std::string("\xff", 1)}) {
      auto v             = intent();
      v.profile_argument = "/p";
      std::array<std::string*, 6> parts{
          &v.operation, &v.actor, &v.expected_head, &v.workspace_argument, &v.registry_argument, &*v.profile_argument};
      *parts[field] = invalid;
      CHECK_FALSE(encode_request_v2(v));
      CHECK_FALSE(decode_request_v2(raw_frame(v)));
    }
  }
  for (auto token : {"0", "01", "+1", "-1", "9223372036854775808", "18446744073709551615", "None", "1x"}) {
    auto v          = intent();
    v.expected_head = token;
    CHECK_FALSE(encode_request_v2(v));
    CHECK_FALSE(decode_request_v2(raw_frame(v)));
  }
  for (auto token : {"1", "9223372036854775807"}) {
    auto v          = intent();
    v.expected_head = token;
    CHECK(encode_request_v2(v));
  }
  for (auto path : {"relative", "/a/../p", "/a/./p", "/a//p"}) {
    auto v             = intent();
    v.profile_argument = path;
    CHECK_FALSE(encode_request_v2(v));
    CHECK_FALSE(decode_request_v2(raw_frame(v)));
  }
  auto v      = intent();
  v.operation = "history";
  CHECK_FALSE(encode_request_v2(v));
  CHECK_FALSE(decode_request_v2(raw_frame(v)));
  v = intent();
  v.actor.assign(256, 'a');
  CHECK(encode_request_v2(v));
  v.actor += 'a';
  CHECK_FALSE(encode_request_v2(v));
  CHECK_FALSE(decode_request_v2(raw_frame(v)));
  v = intent();
  // Local request codecs retain nonempty UTF-8 arguments exactly. The later
  // prepared intent seam, not this codec, requires normalized absolute workspace/registry.
  auto local = decode_request_v2(encode_request_v2(v).value());
  REQUIRE(local);
  CHECK(local->workspace_argument == "../workspace.lua");
  CHECK(local->registry_argument == "./registry.lua");
  auto embedded      = digest_request_v2(v).value();
  v.profile_argument = "/p";
  CHECK(digest_request_v2(v).value() != embedded);
  auto before        = digest_request_v2(v).value();
  v.profile_argument = "/q";
  CHECK(digest_request_v2(v).value() != before);
}
TEST_CASE("request v2 complete 64 MiB frame ceiling includes selector and suffix", "[request_v2]") {
  constexpr std::size_t maximum = 64 * 1024 * 1024;
  for (bool profile : {false, true}) {
    auto v = intent();
    if (profile)
      v.profile_argument = "/p";
    auto overhead = encode_request_v2(v).value().size() - v.workspace_argument.size();
    v.workspace_argument.assign(maximum - overhead, 'a');
    auto bytes = encode_request_v2(v);
    REQUIRE(bytes);
    CHECK(bytes->size() == maximum);
    CHECK(decode_request_v2(*bytes));
    v.workspace_argument += 'a';
    CHECK(encode_request_v2(v).error().code == record_error_code::resource_limit);
    *bytes += 'a';
    CHECK(decode_request_v2(*bytes).error().code == record_error_code::resource_limit);
  }
}
