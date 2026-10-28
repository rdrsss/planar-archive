// Native restricted declaration boundary tests.
#include <catch2/catch_test_macros.hpp>
import std;
import planar.engine.lua;
import planar.engine.capture.admission;
using namespace planar::engine;
namespace {
struct counting_sink : budget_sink {
  std::size_t                     native = 0, instructions = 0, live = 0, peak = 0, native_calls = 0;
  std::vector<std::size_t>        native_requests;
  std::size_t                     native_ceiling = 0, instruction_ceiling = 0, lua_ceiling = 0; ///< Zero means none.
  std::expected<void, diagnostic> charge_native(std::size_t bytes) override {
    ++native_calls;
    native_requests.push_back(bytes);
    if (native_ceiling && bytes > native_ceiling - std::min(native, native_ceiling))
      return std::unexpected(
          diagnostic{"resource_limit", "Native conversion allocation exceeded", "native_bytes", native_ceiling});
    native += bytes;
    return {};
  }
  std::expected<void, diagnostic> charge_instructions(std::size_t count) override {
    if (instruction_ceiling && count > instruction_ceiling - std::min(instructions, instruction_ceiling))
      return std::unexpected(diagnostic{"resource_limit", "Lua instruction limit exceeded", "instructions", instruction_ceiling});
    instructions += count;
    return {};
  }
  std::expected<void, diagnostic> acquire_lua(std::size_t bytes) override {
    if (lua_ceiling && bytes > lua_ceiling - std::min(live, lua_ceiling))
      return std::unexpected(diagnostic{"resource_limit", "Lua allocation exceeded", "lua_bytes", lua_ceiling});
    live += bytes;
    peak = std::max(peak, live);
    return {};
  }
  void release_lua(std::size_t bytes) noexcept override {
    live -= std::min(live, bytes);
  }
};
} // namespace
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

// X06 D C01 rows L1-L7: composed factory budget in the declaration loader.
TEST_CASE("X06 D C01 loader charges instructions to the factory budget", "[loader][x06dc01]") {
  auto sink   = std::make_shared<counting_sink>();
  auto result = load_declaration("for i=1,50000 do end return planar.spec{}", sink);
  REQUIRE(result);
  CHECK(sink->instructions >= 49900);
  CHECK(sink->instructions < 51000);
}
TEST_CASE("X06 D C01 loader refuses at the composed instruction ceiling with the owner diagnostic", "[loader][x06dc01]") {
  auto sink                 = std::make_shared<counting_sink>();
  sink->instruction_ceiling = 20000;
  auto result               = load_declaration("for i=1,200000 do end return planar.spec{}", sink);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "resource_limit");
  CHECK(result.error().limit == "instructions");
  CHECK(result.error().maximum == 20000);
  CHECK(sink->instructions <= 20000);
}
TEST_CASE("X06 D C01 loader inherited chunk limits still win over the composed budget", "[loader][x06dc01]") {
  auto sink = std::make_shared<counting_sink>();
  auto loop = load_declaration("while true do end", sink);
  REQUIRE_FALSE(loop);
  CHECK(loop.error().limit == "instructions");
  CHECK(loop.error().maximum == 1000000);
  CHECK(loop.error().message == "Lua instruction limit exceeded");
  auto nodes = load_declaration("local x={}; for i=1,100001 do x[i]=true end; return planar.spec {l=planar.list(x)}", sink);
  REQUIRE_FALSE(nodes);
  CHECK(nodes.error().limit == "nodes");
}
TEST_CASE("X06 D C01 loader charges retained declaration values to the factory budget byte for byte", "[loader][x06dc01]") {
  auto small = std::make_shared<counting_sink>();
  auto large = std::make_shared<counting_sink>();
  REQUIRE(load_declaration("return planar.spec {s='" + std::string(1000, 'a') + "'}", small));
  REQUIRE(load_declaration("return planar.spec {s='" + std::string(3000, 'a') + "'}", large));
  CHECK(small->native > 0);
  CHECK(large->native - small->native == 2000);
}
TEST_CASE("X06 D C01 loader refuses at the composed native ceiling and reports the owner diagnostic", "[loader][x06dc01]") {
  auto sink            = std::make_shared<counting_sink>();
  sink->native_ceiling = 5000;
  auto result = load_declaration("local t={}; for i=1,100 do t[i]=true end; return planar.spec {l=planar.list(t)}", sink);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "resource_limit");
  CHECK(result.error().limit == "native_bytes");
  CHECK(result.error().maximum == 5000);
  CHECK(result.error().message == "Native conversion allocation exceeded");
  CHECK(sink->native <= 5000);
}
TEST_CASE("X06 D C01 loader declaration Lua memory is live against the factory budget and released on close",
          "[loader][x06dc01]") {
  auto ok = std::make_shared<counting_sink>();
  REQUIRE(load_declaration("local t={}; for i=1,100 do t[i]='x'..i end; return planar.spec {}", ok));
  CHECK(ok->peak > 0);
  CHECK(ok->live == 0);
  auto failed = std::make_shared<counting_sink>();
  REQUIRE_FALSE(load_declaration("local t={}; for i=1,100 do t[i]='x'..i end; return {}", failed));
  CHECK(failed->peak > 0);
  CHECK(failed->live == 0);
}
TEST_CASE("X06 D C01 loader refuses at the composed live Lua ceiling with the owner diagnostic", "[loader][x06dc01]") {
  auto sink         = std::make_shared<counting_sink>();
  sink->lua_ceiling = 100000;
  auto result       = load_declaration("local x='abcdefghijklmnop'; for i=1,30 do x=x..x end; return planar.spec {} ", sink);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "resource_limit");
  CHECK(result.error().limit == "lua_bytes");
  CHECK(result.error().maximum == 100000);
  CHECK(sink->live == 0);
}
// Mutant L4c: load_declaration drops the final retained-declaration charge (sizeof(declaration) + references).
TEST_CASE("X06 D C01 loader charges the final retained declaration and references to the factory budget", "[loader][x06dc01]") {
  auto plain = std::make_shared<counting_sink>();
  REQUIRE(load_declaration("return planar.spec {version=1}", plain));
  REQUIRE_FALSE(plain->native_requests.empty());
  CHECK(plain->native_requests.back() == sizeof(declaration));
  auto referenced = std::make_shared<counting_sink>();
  auto result = load_declaration("return planar.spec {a=planar.file('notes/a.md'), b=planar.file('notes/bb.md')}", referenced);
  REQUIRE(result);
  REQUIRE(result->references.size() == 2);
  std::size_t expected = sizeof(declaration);
  for (const auto& r : result->references)
    expected += r.size() + sizeof(std::string);
  REQUIRE_FALSE(referenced->native_requests.empty());
  CHECK(referenced->native_requests.back() == expected);
}
// Mutant L5: charge() ignores the sink refusal. A single oversized value is refused while every later charge, including
// the final retained charge, still fits, so only the refusal itself can fail the load.
TEST_CASE("X06 D C01 loader refuses one oversized value even when every later charge fits", "[loader][x06dc01]") {
  auto sink            = std::make_shared<counting_sink>();
  sink->native_ceiling = 20000;
  auto result          = load_declaration("return planar.spec {s='" + std::string(30000, 'a') + "'}", sink);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "resource_limit");
  CHECK(result.error().limit == "native_bytes");
  CHECK(result.error().maximum == 20000);
  CHECK(sink->native < 20000);
}

namespace {
/// Sink that refuses at a threshold with a distinctive owner diagnostic and counts calls made after the refusal.
struct refusing_sink : budget_sink {
  std::size_t                     instructions = 0, threshold = 20000, calls_after_refusal = 0;
  bool                            refused = false;
  std::expected<void, diagnostic> charge_native(std::size_t) override {
    return {};
  }
  std::expected<void, diagnostic> charge_instructions(std::size_t count) override {
    if (refused)
      ++calls_after_refusal;
    if (instructions + count > threshold) {
      refused = true;
      return std::unexpected(diagnostic{"resource_limit", "Distinctive owner refusal", "instructions", 987654});
    }
    instructions += count;
    return {};
  }
  std::expected<void, diagnostic> acquire_lua(std::size_t) override {
    return {};
  }
  void release_lua(std::size_t) noexcept override {
  }
};
std::string c02_read(const std::string& relative) {
  std::ifstream in(std::filesystem::path(PLANAR_SOURCE_ROOT) / relative, std::ios::binary);
  return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}
} // namespace
// Mutant M3: the loader hook ignores the sink diagnostic and reports its generic instruction diagnostic instead.
TEST_CASE("X06 D C02 loader reports the owner diagnostic when the composed owner refuses a declaration chunk",
          "[loader][x06d][c02]") {
  auto sink   = std::make_shared<refusing_sink>();
  auto result = load_declaration("for i=1,200000 do end return planar.spec{}", sink);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "resource_limit");
  CHECK(result.error().limit == "instructions");
  CHECK(result.error().maximum == 987654);
  CHECK(result.error().message == "Distinctive owner refusal");
  CHECK(sink->refused);
  CHECK(sink->calls_after_refusal == 0);
  CHECK(sink->instructions <= sink->threshold);
}
namespace {
/// Sink that refuses the first live Lua acquisition, which is the one lua_newstate makes for the state itself.
struct state_refusing_sink : budget_sink {
  std::size_t                     acquires = 0, releases = 0;
  std::expected<void, diagnostic> charge_native(std::size_t) override {
    return {};
  }
  std::expected<void, diagnostic> charge_instructions(std::size_t) override {
    return {};
  }
  std::expected<void, diagnostic> acquire_lua(std::size_t) override {
    ++acquires;
    return std::unexpected(diagnostic{"resource_limit", "Distinctive state refusal", "lua_bytes", 424242});
  }
  void release_lua(std::size_t) noexcept override {
    ++releases;
  }
};
} // namespace
// Mutant L7c: a composed refusal of Lua state creation reports the inherited diagnostic instead of the owner's.
TEST_CASE("X06 D C02 loader reports the owner diagnostic when the composed owner refuses Lua state creation",
          "[loader][x06d][c02]") {
  auto sink   = std::make_shared<state_refusing_sink>();
  auto result = load_declaration("return planar.spec{}", sink);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "resource_limit");
  CHECK(result.error().limit == "lua_bytes");
  CHECK(result.error().maximum == 424242);
  CHECK(result.error().message == "Distinctive state refusal");
  CHECK(sink->acquires == 1);
  CHECK(sink->releases == 0);
}
// Mutant M4: the loader re-imports capture admission, or budget.cppm imports it (module graph R6).
TEST_CASE("X06 D C02 loader module graph does not depend on capture admission", "[loader][x06d][c02]") {
  auto budget = c02_read("src/engine/budget/budget.cppm");
  REQUIRE_FALSE(budget.empty());
  CHECK(budget.find("export module planar.engine.budget;") != std::string::npos);
  std::vector<std::string> imports;
  std::istringstream       lines(budget);
  for (std::string line; std::getline(lines, line);) {
    auto at = line.find("import ");
    if (at != std::string::npos && line.find("//") > at)
      imports.push_back(line.substr(at, line.find(';', at) - at + 1));
  }
  CHECK_FALSE(imports.empty());
  for (const auto& entry : imports) {
    INFO(entry);
    CHECK((entry == "import std;" || entry == "import planar.engine.value;"));
  }
  CHECK(budget.find("planar.engine.capture.admission") == std::string::npos);
  for (auto file : {"src/engine/lua/loader.cppm", "src/engine/lua/loader.cpp"}) {
    INFO(file);
    auto text = c02_read(file);
    REQUIRE_FALSE(text.empty());
    CHECK(text.find("planar.engine.capture.admission") == std::string::npos);
  }
  auto interface = c02_read("src/engine/lua/loader.cppm") + c02_read("src/engine/lua/loader.cpp");
  CHECK(interface.find("import planar.engine.budget;") != std::string::npos);
}
