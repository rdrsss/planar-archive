// Pure profile decoding needs no host capture, filesystem or Lua runtime.
#include <catch2/catch_test_macros.hpp>
import std;
import planar.engine.capture.profile_codec;
using namespace planar::engine;
namespace {
void refuses(std::string_view bytes, std::string_view code, std::string_view field = "profile") {
  auto result = decode_profile(bytes);
  REQUIRE_FALSE(result);
  CHECK(result.error().detail.code == code);
  CHECK(result.error().field == field);
  CHECK_FALSE(result.error().detail.message.empty());
}
} // namespace
TEST_CASE("profile codec decodes exact ordered paths in memory", "[profile][codec]") {
  auto result = decode_profile(R"( {"modules":["z.lua","Dir/a.lua","b\u002elua"],"version":1} )");
  REQUIRE(result);
  CHECK(*result == std::vector<std::string>{"z.lua", "Dir/a.lua", "b.lua"});
  REQUIRE(decode_profile(R"({"version":1,"modules":[]})"));
}
TEST_CASE("profile codec keeps strict schema and arbitrary integer classification", "[profile][codec]") {
  for (auto bytes :
       {R"({"version":1,"modules":[],"x":0})", R"({"version":1,"version":1})", R"({"version":1,"mod\u0075les":[],"modules":[]})",
        R"({"version":1.0,"modules":[]})", R"({"version":true,"modules":[]})", R"({"version":1,"modules":[]}x)",
        R"({"version":1 "modules":[]})", R"({"version":1,"modules":["a.lua",]})"})
    refuses(bytes, "schema");
  for (auto version : {"0", "-1", "184467440737095516160000000000000000000000"})
    refuses(std::format(R"({{"version":{},"modules":[]}})", version), "unsupported_version");
  refuses(R"({"version":1,"modules":["a.lua",false]})", "schema", "profile.modules[2]");
  std::string invalid = R"({"version":1,"modules":[")";
  invalid += char(0xff);
  invalid += R"(.lua"]})";
  refuses(invalid, "schema");
}
TEST_CASE("profile codec preserves path policy and logical error fields", "[profile][codec]") {
  for (auto path : {"../x.lua", "/x.lua", "a//x.lua", "x.LUA", "planar/x.lua", "__PLANAR__/x.lua", "a\\x.lua"}) {
    // Backslash is escaped by the explicit fixture below.
    if (std::string_view(path).find('\\') == std::string_view::npos)
      refuses(std::format(R"({{"version":1,"modules":["a.lua","{}"]}})", path), "path", "profile.modules[2]");
  }
  refuses(R"({"version":1,"modules":["a\\x.lua"]})", "path", "profile.modules[1]");
  refuses(R"({"version":1,"modules":["A/x.lua","a/y.lua"]})", "path", "profile.modules[2]");
  refuses(R"({"version":1,"modules":["a.lua","a.lua"]})", "path", "profile.modules[2]");
  refuses(R"({"version":1,"modules":["a\u0000.lua"]})", "schema");
}
TEST_CASE("profile codec enforces bounded strings arrays nodes depth and input", "[profile][codec]") {
  std::string bytes = R"({"version":1,"modules":[]})";
  bytes.resize(65536, ' ');
  REQUIRE(decode_profile(bytes));
  bytes += ' ';
  refuses(bytes, "resource_limit");
  bytes = R"({"version":1,"modules":[)";
  for (int i = 0; i < 64; ++i) {
    if (i)
      bytes += ',';
    bytes += std::format("\"m{}.lua\"", i);
  }
  REQUIRE(decode_profile(bytes + "]}"));
  refuses(bytes + R"(,"extra.lua"]})", "resource_limit");
  refuses(std::format(R"({{"version":1,"modules":["{}"]}})", std::string(1025, 'x')), "resource_limit");
  refuses(R"({"version":1,"modules":[[[[[]]]]]})", "resource_limit");
  bytes = "{";
  for (int i = 0; i < 132; ++i) {
    if (i)
      bytes += ',';
    bytes += std::format("\"k{}\":0", i);
  }
  refuses(bytes + "}", "resource_limit");
}
