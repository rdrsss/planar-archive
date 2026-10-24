// Native restricted declaration boundary tests.
#include <catch2/catch_test_macros.hpp>
import std;
import planar.engine.lua;
using namespace planar::engine;
TEST_CASE("loader preserves structural constructor tags", "[loader]") {
  auto result = load_declaration("return planar.spec {version=1, nested={}, values=planar.list {true, 12, planar.md('text')}, "
                                 "ref=planar.file('notes/a.md'), note=planar.immaterial(planar.md('optional'))}");
  REQUIRE(result);
  CHECK(result->root->kind == value_kind::spec);
  CHECK(result->root->fields.at("nested")->kind == value_kind::record);
  CHECK(result->root->fields.at("values")->items.size() == 3);
  CHECK(result->root->fields.at("note")->kind == value_kind::immaterial);
  CHECK(result->references == std::vector<std::string>{"notes/a.md"});
}
TEST_CASE("loader denies ambient capabilities and all trusted mutation", "[loader]") {
  for (auto source : {"return os.execute('true')", "return require('io')", "x=1; return planar.spec {}",
                      "planar.spec=1; return planar.spec {}", "planar.new=1; return planar.spec {}",
                      "local x=planar.spec {version=1}; x.version=2; return x", "local x=planar.spec {}; x.new=1; return x"}) {
    INFO(source);
    auto result = load_declaration(source);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "capability_denied");
  }
}
TEST_CASE("loader rejects unsupported structures", "[loader]") {
  for (auto source : {"return planar.spec {n=1.0}", "return planar.spec {n=-1}", "return planar.spec {n=2147483648}",
                      "return planar.spec {f=function() end}", "return planar.spec {l=planar.list {[2]=true}}",
                      "return planar.spec {l=planar.list {true,x=true}}", "local x={}; x.x=x; return planar.spec(x)",
                      "local x={}; return planar.spec {a=x,b=x}", "local x=planar.md('x'); return planar.spec {a=x,b=x}",
                      "return {}", "return planar.spec {}, planar.job {}", "return", "return planar.md('x')",
                      "return planar.spec {[1]=true}", "return planar.spec {l=planar.list(planar.list {})}"}) {
    INFO(source);
    auto result = load_declaration(source);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "schema");
  }
  CHECK(load_declaration("this is not Lua").error().code == "lua_syntax");
  CHECK(load_declaration(std::string("\x1bLua")).error().code == "capability_denied");
  CHECK_FALSE(load_declaration("return planar.spec {s='\\000'}"));
  CHECK_FALSE(load_declaration(std::string("return planar.spec {s='") + char(0xff) + "'}"));
}
TEST_CASE("loader enforces independent instruction allocation and structural bounds", "[loader]") {
  auto loop = load_declaration("while true do end");
  REQUIRE_FALSE(loop);
  CHECK(loop.error().limit == "instructions");
  auto allocation = load_declaration("local x='abcdefghijklmnop'; for i=1,30 do x=x..x end; return planar.spec {} ");
  REQUIRE_FALSE(allocation);
  CHECK(allocation.error().limit == "lua_bytes");
  auto depth = load_declaration("local x={}; for i=1,70 do x={nested=x} end; return planar.spec(x)");
  REQUIRE_FALSE(depth);
  CHECK(depth.error().limit == "depth");
  auto nodes = load_declaration("local x={}; for i=1,100001 do x[i]=true end; return planar.spec {l=planar.list(x)}");
  REQUIRE_FALSE(nodes);
  CHECK(nodes.error().limit == "nodes");
  CHECK(load_declaration(std::string(1048577, ' ')).error().limit == "input_bytes");
  CHECK(load_declaration("return planar.spec {}"));
}

TEST_CASE("loader bounds native amplification of reused scalar strings", "[loader]") {
  auto result = load_declaration("local s='x'; for i=1,16 do s=s..s end; local t={}; for i=1,2000 do t[i]=s end; return "
                                 "planar.spec {values=planar.list(t)}");
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "resource_limit");
  CHECK(result.error().limit == "native_bytes");
  CHECK(result.error().maximum == 67108864);
}

TEST_CASE("loader constructed nested records remain immutable and readable", "[loader]") {
  auto readable = load_declaration("local x=planar.spec {version=1,nested={flag=true}, values=planar.list {12}}; if x.version~=1 "
                                   "or x.nested.flag~=true or x.values[1]~=12 then forbidden() end; return x");
  REQUIRE(readable);
  for (auto source : {"local x=planar.spec {nested={flag=true}}; x.nested.flag=false; return x",
                      "local x=planar.spec {values=planar.list {12}}; x.values[1]=13; return x"}) {
    auto result = load_declaration(source);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "capability_denied");
  }
  auto invalid = load_declaration("return planar.spec {ref=planar.file('../outside')}");
  REQUIRE_FALSE(invalid);
  CHECK(invalid.error().code == "path_denied");
}
TEST_CASE("loader retains table identities across garbage collection", "[loader]") {
  auto result =
      load_declaration("for i=1,10000 do local discarded=planar.spec {id='temporary'} end; return planar.spec {id='retained'}");
  REQUIRE(result);
  CHECK(result->root->fields.at("id")->string == "retained");
}
