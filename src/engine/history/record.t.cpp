// Independent record framing/hash vectors, local validation and bounded corruption refusal.
#include <catch2/catch_test_macros.hpp>
import std;
import planar.engine.history.record;
using namespace planar::engine::history;
namespace {
std::string unhex(std::string_view text) {
  std::string bytes;
  for (std::size_t i = 0; i < text.size(); i += 2)
    bytes += static_cast<char>(std::stoul(std::string(text.substr(i, 2)), nullptr, 16));
  return bytes;
}
std::string hex(const record_digest& digest) {
  std::string result;
  for (auto byte : digest)
    result += std::format("{:02x}", byte);
  return result;
}
std::string prefix(unsigned kind) {
  return std::string("PLANAR-RECORD\x01") + static_cast<char>(kind);
}
void wide(std::string& bytes, std::uint64_t n) {
  for (int shift = 56; shift >= 0; shift -= 8)
    bytes += static_cast<char>((n >> shift) & 255);
}
request_record request() {
  return {"load", "driver:é", "none", "../workspace.lua", "./registry.lua"};
}
directory_record directory() {
  return {{{"A.txt", record_kind::file, 1, {}}, {"bin", record_kind::directory, 2, {}}}};
}
} // namespace
TEST_CASE("record independent literal framing and canonical digest vectors", "[record]") {
  std::ifstream input(std::string(PLANAR_RECORD_VECTOR_DIR) + "/record-v1.tsv");
  REQUIRE(input);
  std::string line;
  unsigned    count = 0;
  while (std::getline(input, line)) {
    if (line.empty() || line.front() == '#')
      continue;
    std::istringstream fields(line);
    std::string        name, stored_hex, portable_hex, digest_hex;
    REQUIRE(static_cast<bool>(std::getline(fields, name, '\t')));
    REQUIRE(static_cast<bool>(std::getline(fields, stored_hex, '\t')));
    REQUIRE(static_cast<bool>(std::getline(fields, portable_hex, '\t')));
    REQUIRE(static_cast<bool>(std::getline(fields, digest_hex)));
    INFO(name);
    auto stored  = unhex(stored_hex);
    auto decoded = decode_record(stored);
    REQUIRE(decoded);
    CHECK(encode_record(*decoded).value() == stored);
    CHECK(hex(digest_record(*decoded).value()) == digest_hex);
    if (name == "file-binary-executable") {
      const auto& f = std::get<file_record>(*decoded);
      CHECK(f.mode == 493);
      CHECK(f.payload == std::string("A\0\xff\r\n", 5));
      CHECK(encode_record(file_record{493, std::string("A\0\xff\r\n", 5)}).value() == stored);
    } else if (name == "directory-two") {
      const auto& d = std::get<directory_record>(*decoded);
      REQUIRE(d.children.size() == 2);
      CHECK(d.children[0].name == "A.txt");
      CHECK(d.children[0].id == 1);
      CHECK(d.children[1].name == "bin");
      CHECK(d.children[1].id == 9223372036854775807ULL);
      auto portable = stored;
      // Positions calculated from independent fixed framing: two exact u64 IDs only.
      portable.erase(73, 8);
      portable.erase(37, 8);
      CHECK(portable == unhex(portable_hex));
    } else if (name == "request-none") {
      const auto& r = std::get<request_record>(*decoded);
      CHECK(r.operation == "load");
      CHECK(r.actor == "driver:é");
      CHECK(r.expected_head == "none");
      CHECK(r.workspace_argument == "/work/../tree/workspace.lua");
      CHECK(r.registry_argument == "/registry.lua");
    }
    for (std::size_t size = 0; size < stored.size(); ++size)
      CHECK_FALSE(decode_record(std::string_view(stored).substr(0, size)));
    CHECK_FALSE(decode_record(stored + '\0'));
    ++count;
  }
  CHECK(count == 10);
}
TEST_CASE("record directory local IDs change storage while metadata changes portable digest", "[record]") {
  auto d           = directory();
  auto before      = encode_record(d).value();
  auto digest      = digest_record(d).value();
  d.children[0].id = 99;
  CHECK(encode_record(d).value() != before);
  CHECK(digest_record(d).value() == digest);
  d.children[0].name = "B.txt";
  CHECK(digest_record(d).value() != digest);
  d                  = directory();
  d.children[0].kind = record_kind::directory;
  CHECK(digest_record(d).value() != digest);
  d                        = directory();
  d.children[0].digest[15] = 1;
  CHECK(digest_record(d).value() != digest);
  CHECK(digest_record(file_record{420, "x"}).value() != digest_record(file_record{493, "x"}).value());
  CHECK(digest_record(file_record{420, "x"}).value() != digest_record(file_record{420, "y"}).value());
  auto r               = request();
  auto original        = digest_record(r).value();
  r.workspace_argument = "workspace.lua";
  CHECK(digest_record(r).value() != original);
}
TEST_CASE("record rejects invalid modes names aliases ordering kinds and local IDs", "[record]") {
  for (auto mode : {0u, 421u, 511u})
    CHECK_FALSE(encode_record(file_record{mode, ""}));
  for (auto name : {"", ".", "..", ".git", ".GIT", "a/b", "a\\b", "é", "a?"}) {
    auto d             = directory();
    d.children[0].name = name;
    CHECK_FALSE(encode_record(d));
  }
  auto d             = directory();
  d.children[0].name = std::string(129, 'a');
  CHECK_FALSE(encode_record(d));
  d = directory();
  std::ranges::reverse(d.children);
  CHECK_FALSE(encode_record(d));
  d                  = directory();
  d.children[1].name = "A.txt";
  CHECK_FALSE(encode_record(d));
  d                  = directory();
  d.children[1].name = "a.TXT";
  CHECK_FALSE(encode_record(d));
  for (auto id : {0ULL, 9223372036854775808ULL, 18446744073709551615ULL}) {
    d                = directory();
    d.children[0].id = id;
    CHECK_FALSE(encode_record(d));
  }
  for (auto kind : {0, 5, 255}) {
    d                  = directory();
    d.children[0].kind = static_cast<record_kind>(kind);
    CHECK_FALSE(encode_record(d));
  }
  d                  = directory();
  d.children[0].name = std::string(128, 'A');
  CHECK(encode_record(d));
  // Standalone framing permits each defined kind; tree membership is contextual policy.
  for (auto kind : {record_kind::file, record_kind::directory, record_kind::value, record_kind::request}) {
    d                  = directory();
    d.children[0].kind = kind;
    CHECK(encode_record(d));
  }
}
TEST_CASE("record requests validate local text and head tokens without normalizing arguments", "[record]") {
  auto r = request();
  CHECK(encode_record(r));
  auto decoded = decode_record(encode_record(r).value()).value();
  CHECK(std::get<request_record>(decoded).workspace_argument == "../workspace.lua");
  for (auto token : {"", "0", "01", "+1", "-1", "9223372036854775808", "18446744073709551615", "None", "1x"}) {
    r               = request();
    r.expected_head = token;
    CHECK_FALSE(encode_record(r));
  }
  r               = request();
  r.expected_head = "9223372036854775807";
  CHECK(encode_record(r));
  for (auto text : {std::string(), std::string("\0", 1), std::string("\xff", 1)}) {
    r           = request();
    r.operation = text;
    CHECK_FALSE(encode_record(r));
    r       = request();
    r.actor = text;
    CHECK_FALSE(encode_record(r));
    r                    = request();
    r.workspace_argument = text;
    CHECK_FALSE(encode_record(r));
    r                   = request();
    r.registry_argument = text;
    CHECK_FALSE(encode_record(r));
  }
  r       = request();
  r.actor = std::string(256, 'a');
  CHECK(encode_record(r));
  r.actor += 'a';
  CHECK_FALSE(encode_record(r));
}
TEST_CASE("record values require exact supported bounded canonical data", "[record]") {
  for (auto bytes : {"00", "03000000000000000300ff0a", "040000000000000000", "050000000000000000"})
    CHECK(encode_record(value_record{"canonical_v1", unhex(bytes)}));
  for (auto bytes : {"", "06", "0000", "020000000080000000", "050000000000000001030000000000000001ff00"})
    CHECK_FALSE(encode_record(value_record{"canonical_v1", unhex(bytes)}));
  CHECK(encode_record(value_record{"canonical_v2", unhex("00")}).error().code == record_error_code::unsupported_codec);
  CHECK(encode_record(value_record{"canonical_v2", unhex("ff")}).error().code == record_error_code::unsupported_codec);
  auto unsupported = encode_record(value_record{"canonical_v1", unhex("00")}).value();
  unsupported[34]  = '2'; // Last byte of the twelve-byte codec identifier.
  CHECK(decode_record(unsupported).error().code == record_error_code::unsupported_codec);
  for (auto codec : {std::string(), std::string("\0", 1), std::string("\xff", 1)})
    CHECK(encode_record(value_record{codec, unhex("00")}).error().code == record_error_code::invalid_record);
}
TEST_CASE("record decoder rejects corrupt framing and oversized claims before allocation", "[record]") {
  auto bytes = encode_record(file_record{420, "payload"}).value();
  for (std::size_t offset : {std::size_t{0}, std::size_t{12}, std::size_t{13}, std::size_t{14}, std::size_t{17}}) {
    auto corrupt    = bytes;
    corrupt[offset] = static_cast<char>(255);
    CHECK_FALSE(decode_record(corrupt));
  }
  auto huge = prefix(1) + unhex("000001a4");
  wide(huge, 18446744073709551615ULL);
  CHECK(decode_record(huge).error().code == record_error_code::resource_limit);
  auto truncated = prefix(1) + unhex("000001a4");
  wide(truncated, 16 * 1024 * 1024);
  CHECK(decode_record(truncated).error().code == record_error_code::invalid_record);
  CHECK_FALSE(decode_record(prefix(2) + unhex("000001edffffffff")));
  CHECK_FALSE(decode_record(prefix(2) + unhex("000001ed00000001")));
  CHECK_FALSE(decode_record(prefix(2) + unhex("000001a400000000")));
  CHECK_FALSE(decode_record(prefix(3) + unhex("ffffffffffffffff")));
  auto directory_bytes = encode_record(directory()).value();
  for (std::size_t offset : {std::size_t{36}, std::size_t{37}, std::size_t{44}}) {
    auto corrupt = directory_bytes;
    if (offset == 36)
      corrupt[offset] = 5;
    else if (offset == 37)
      corrupt[offset] = static_cast<char>(128);
    else
      corrupt[offset] = 0;
    CHECK_FALSE(decode_record(corrupt));
  }
}
TEST_CASE("record file and value payload ceilings include canonical framing bytes", "[record]") {
  file_record f{493, std::string(16 * 1024 * 1024, '\xff')};
  auto        encoded = encode_record(f);
  REQUIRE(encoded);
  auto decoded = decode_record(*encoded);
  REQUIRE(decoded);
  CHECK(std::get<file_record>(*decoded).payload == f.payload);
  f.payload += 'x';
  CHECK(encode_record(f).error().code == record_error_code::resource_limit);
  // A canonical string header consumes nine of the sixteen MiB value bytes.
  std::string canonical(1, '\x03');
  wide(canonical, 16 * 1024 * 1024 - 9);
  canonical.append(16 * 1024 * 1024 - 9, '\0');
  CHECK(encode_record(value_record{"canonical_v1", canonical}));
  canonical += 'x';
  CHECK(encode_record(value_record{"canonical_v1", canonical}).error().code == record_error_code::resource_limit);
}

TEST_CASE("record whole encoding ceiling is exact for requests", "[record]") {
  constexpr std::size_t maximum = 64 * 1024 * 1024;
  auto                  r       = request();
  // Header is 13 magic bytes, version, kind, then five eight-byte lengths.
  auto overhead = 15 + 5 * 8 + r.operation.size() + r.actor.size() + r.expected_head.size() + r.registry_argument.size();
  r.workspace_argument.assign(maximum - overhead, 'a');
  auto encoded = encode_record(r);
  REQUIRE(encoded);
  CHECK(encoded->size() == maximum);
  CHECK(decode_record(*encoded));
  CHECK(digest_record(r));
  r.workspace_argument += 'a';
  CHECK(encode_record(r).error().code == record_error_code::resource_limit);
  CHECK(digest_record(r).error().code == record_error_code::resource_limit);
  encoded->push_back('a');
  CHECK(decode_record(*encoded).error().code == record_error_code::resource_limit);
}
TEST_CASE("record decoding validates persisted local directory and request fields", "[record]") {
  auto d             = directory();
  d.children[0].name = "A.txt";
  d.children[1].name = "B.txt";
  auto bytes         = encode_record(d).value();
  // Same-sized spelling changes retain valid framing, exposing local validation.
  auto duplicate = bytes;
  duplicate.replace(69, 5, "A.txt");
  CHECK_FALSE(decode_record(duplicate));
  auto alias = bytes;
  alias.replace(69, 5, "a.TXT");
  CHECK_FALSE(decode_record(alias));
  auto unordered = bytes;
  unordered.replace(31, 5, "Z.txt");
  CHECK_FALSE(decode_record(unordered));
  auto invalid_name = bytes;
  invalid_name.replace(31, 5, "a/b.c");
  CHECK_FALSE(decode_record(invalid_name));
  auto r                = request();
  bytes                 = encode_record(r).value();
  auto actor_position   = 15 + 8 + r.operation.size() + 8;
  bytes[actor_position] = '\0';
  CHECK_FALSE(decode_record(bytes));
  auto v   = encode_record(value_record{"canonical_v1", unhex("00")}).value();
  v.back() = 6;
  CHECK_FALSE(decode_record(v));
}
