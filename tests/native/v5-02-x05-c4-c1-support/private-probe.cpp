// Private actual-owner probes: counters, receipts, allocation refusal and cleanup.
#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <fcntl.h>
#include <new>
#include <sqlite3.h>
#include <unistd.h>
import std;
import planar.engine.runtime;
using namespace planar::engine;
using namespace planar::engine::ledger;
static bool                         fail_next   = false;
[[maybe_unused]] static std::size_t failed_size = 0;
void*                               operator new(std::size_t n) {
  if (std::exchange(fail_next, false)) {
    failed_size = n;
    throw std::bad_alloc();
  }
  if (auto* p = std::malloc(n ? n : 1))
    return p;
  throw std::bad_alloc();
}
void operator delete(void* p) noexcept {
  std::free(p);
}
void* operator new[](std::size_t n) {
  return ::operator new(n);
}
void operator delete[](void* p) noexcept {
  ::operator delete(p);
}
extern "C" {
bool        c4_one_shot        = false;
bool        c4_fail_lua        = false;
int         c4_failed_lua      = 0;
std::size_t c4_lua_allocations = 0;
}
static std::function<void(int)> stage;
extern "C" void                 c4_stage(int n) {
  if (stage)
    stage(n);
}
namespace {
#include "observer.inc"
constexpr auto success = "return {ok=true,value={version=1,kind='extension',command=inv.command,data={type='history',value=h}}}";
[[maybe_unused]] constexpr auto semantic = "return {ok=false,error={code='schema',message='intentional',field='input'}}";
struct fixture {
  std::filesystem::path root;
  fixture(std::string body = "local h=ctx.read_history({}); " + std::string(success),
          std::string projection =
              "return {version=1,sections=engine.array{{id='private_section',text=input.ledger,mandatory=false}}}",
          bool format2 = false) {
    char pattern[] = "/tmp/planar-c4b3-probe-XXXXXX";
    REQUIRE(::mkdtemp(pattern));
    root = std::filesystem::canonical(pattern);
    std::filesystem::create_directory(root / "trusted");
    std::ofstream(root / "trusted/profile.json") << R"({"version":1,"modules":["module.lua"]})";
    std::ofstream(root / "trusted/module.lua") << R"(
registration.add({name='inspect',family='extension_read_v1',handler_id='custom.inspect',help='inspect',response_kind='extension_v1',capabilities=engine.array{'history_read_v1'},options=engine.array{
{name='--ledger',type='text',required=true,help='ledger'},
{name='--revision',type='history_id',required=false,help='revision'},
{name='--at-event',type='history_id',required=false,help='event'},
{name='--after-event',type='history_id',required=false,help='cursor'},
{name='--request-id',type='text',required=false,help='request'},
{name='--limit',type='decimal',required=false,help='limit'}
}},function(inv,ctx) )" << body
                                               << " end) "
                                                  "registration.projection({id='custom',abi='projection_v1',revision='r1',input='"
                                                  "history_snapshot_v1',help='projection'},function(input) "
                                               << projection
                                               << " end) "
                                                  "registration.projection({id='wrong',abi='projection_v1',revision='r1',input='"
                                                  "role_context_v1',help='wrong'},function() error('wrong F ran') end)";
    REQUIRE((format2 ? initialize_new_v2(root / "ledger", "demo") : initialize_new(root / "ledger", "demo")));
  }
  ~fixture() {
    stage       = {};
    fail_next   = false;
    c4_fail_lua = false;
    std::filesystem::remove_all(root);
  }
  extension_invocation owner(std::vector<extension_parsed_option> options = {}) {
    auto capture = profile_capture::open((root / "trusted/profile.json").string());
    REQUIRE(capture);
    auto result = extension_invocation::create(std::move(*capture));
    REQUIRE(result);
    auto path = (root / "ledger").string();
    options.insert(options.begin(), {"ledger", true, path});
    REQUIRE(result->admit("inspect", options, {}));
    return std::move(*result);
  }
  void mutate(bool ledger = false) {
    std::ofstream(root / (ledger ? "ledger" : "trusted/module.lua"), std::ios::app) << "changed";
  }
};
void failure(const auto& result, std::string_view code) {
  REQUIRE_FALSE(result);
  REQUIRE(std::holds_alternative<registration_error>(result.error()));
  CHECK(std::get<registration_error>(result.error()).detail.code == code);
}
} // namespace
TEST_CASE("C4C1 unchanged persistent layout and native initial charge", "[c4c1-layout]") {
  fixture       f;
  auto          owner  = f.owner();
  auto          layout = owner.private_probe();
  std::ofstream out("LAYOUT.json");
  out << '[';
  for (std::size_t i = 0; i < layout.size(); ++i) {
    if (i)
      out << ',';
    out << (i >= 8 ? 0 : layout[i]);
  }
  out << "]\n";
  REQUIRE(owner.execute());
}
TEST_CASE("C4C1 authenticated nested projection resumes real handler in both formats", "[c4c1-private]") {
  for (bool format2 : {false, true}) {
    fixture f(
        "local h=ctx.read_history({}); local ticks=0; for i=1,10000 do ticks=ticks+i end; local read=ctx.read_history; local "
        "opaque=inv.options.ledger; "
        "local p=ctx.project_probe('custom'); assert(p.sections[1].text==h.ledger); "
        "assert(ctx.read_history==read and inv.options.ledger==opaque and inv.command=='inspect'); "
        "assert(h.kind=='history' and h.data.events[1].sequence=='1'); " +
            std::string(success),
        "local ticks=0; for i=1,5000 do ticks=ticks+i end; assert(not pcall(function() input.last_event='changed' end)); local "
        "s={id='private_section',text=input.ledger,mandatory=false}; "
        "return {version=1,sections=engine.array{s}}",
        format2);
    auto                        owner = f.owner();
    observed_vfs                observer;
    std::array<std::size_t, 19> before{}, after{};
    bool                        entered = false, returned = false;
    stage = [&](int n) {
      if (n == 10) {
        entered = true;
        before  = owner.private_probe();
      }
      if (n == 11) {
        returned = true;
        after    = owner.private_probe();
      }
    };
    auto result = owner.execute();
    stage       = {};
    REQUIRE(result);
    CHECK(entered);
    CHECK(returned);
    CHECK(observer.reads > 0);
    CHECK(observer.opens == observer.closes);
    for (auto i : {8, 9, 10, 11, 12, 13, 14, 15, 16, 17})
      CHECK(after[i] == before[i]);
    for (auto i : {4, 5, 6})
      CHECK(after[i] >= before[i]);
    CHECK(after[4] > before[4]);
    CHECK(after[5] > before[5]);
    CHECK(before[6] > 0);
    CHECK(after[6] > before[6]);
    auto clean = owner.private_probe();
    for (auto i : {8, 9, 10, 13, 14, 17})
      CHECK(clean[i] == 0);
  }
}
TEST_CASE("C4C1 caught malformed and throwing nested F retain projection location", "[c4c1-private]") {
  for (auto body : {"return {version=1,sections=engine.array{},extra=true}", "error('private throw')", "return {},{}"}) {
    fixture f("local h=ctx.read_history({}); pcall(function() ctx.project_probe('custom') end); " + std::string(semantic), body);
    auto    owner  = f.owner();
    auto    result = owner.execute();
    failure(result, "internal");
    CHECK(std::get<registration_error>(result.error()).field == "catalog.projections[1]");
    auto retry = owner.execute();
    failure(retry, "internal");
    CHECK(std::get<registration_error>(retry.error()).field == "catalog.projections[1]");
  }
}
TEST_CASE("C4C1 unknown and wrong-kind projections refuse before retained F", "[c4c1-private]") {
  for (auto id : {"unknown", "wrong"}) {
    fixture f("local h=ctx.read_history({}); pcall(function() ctx.project_probe('" + std::string(id) + "') end); " + semantic);
    auto    owner   = f.owner();
    bool    entered = false;
    stage           = [&](int n) {
      if (n == 12)
        entered = true;
    };
    auto result = owner.execute();
    stage       = {};
    failure(result, "internal");
    CHECK_FALSE(entered);
  }
}
TEST_CASE("C4C1 actual native allocation refusal restores frame and stays terminal", "[c4c1-private]") {
  fixture f("local h=ctx.read_history({}); pcall(function() ctx.project_probe('custom') end); " + std::string(semantic));
  auto    owner = f.owner();
  std::array<std::size_t, 19> before{}, after{};
  failed_size = 0;
  stage       = [&](int n) {
    if (n == 10)
      before = owner.private_probe();
    if (n == 13) {
      fail_next = true;
    }
    if (n == 11) {
      after = owner.private_probe();
    }
  };
  auto result = owner.execute();
  stage       = {};
  fail_next   = false;
  CHECK(failed_size > 0);
  failure(result, "resource_limit");
  for (auto i : {8, 9, 10, 11, 12, 13, 14, 15, 16, 17})
    CHECK(after[i] == before[i]);
  failure(owner.execute(), "resource_limit");
}
TEST_CASE("C4C1 actual completed Lua allocation refusal is sticky after catch", "[c4c1-private]") {
  fixture f("local h=ctx.read_history({}); pcall(function() ctx.project_probe('custom') end); " + std::string(semantic));
  auto    owner = f.owner();
  std::array<std::size_t, 19> before{}, after{};
  c4_failed_lua = 0;
  stage         = [&](int n) {
    if (n == 10)
      before = owner.private_probe();
    if (n == 12)
      c4_fail_lua = true;
    if (n == 11) {
      c4_fail_lua = false;
      after       = owner.private_probe();
    }
  };
  auto result = owner.execute();
  stage       = {};
  c4_fail_lua = false;
  CHECK(c4_failed_lua > 0);
  failure(result, "resource_limit");
  CHECK(after[18] == 1);
  for (auto i : {8, 9, 10, 11, 12, 13, 14, 15, 16, 17})
    CHECK(after[i] == before[i]);
  failure(owner.execute(), "resource_limit");
}
TEST_CASE("C4C1 real instruction exhaustion remains cumulative through nested catch", "[c4c1-private]") {
  fixture     f("local h=ctx.read_history({}); pcall(function() ctx.project_probe('custom') end); " + std::string(semantic),
                "while true do end");
  auto        owner  = f.owner();
  std::size_t before = 0, after = 0;
  stage = [&](int n) {
    if (n == 10)
      before = owner.private_probe()[6];
    if (n == 11)
      after = owner.private_probe()[6];
  };
  auto result = owner.execute();
  stage       = {};
  failure(result, "resource_limit");
  CHECK(after >= 100000000);
  CHECK(after > before);
  failure(owner.execute(), "resource_limit");
}
TEST_CASE("C4C1 original receipt seal still suppresses resumed nested success", "[c4c1-private]") {
  for (bool ledger : {false, true}) {
    fixture f("local h=ctx.read_history({}); local p=ctx.project_probe('custom'); " + std::string(success));
    auto    owner = f.owner();
    bool    hit   = false;
    stage         = [&](int n) {
      if (n == 11) {
        hit = true;
        f.mutate(ledger);
      }
    };
    auto result = owner.execute();
    stage       = {};
    CHECK(hit);
    failure(result, "capture_changed");
  }
}
TEST_CASE("C4C1 one null Lua request with successful emergency retry stays healthy", "[c4c1-private]") {
  fixture f("local h=ctx.read_history({}); local p=ctx.project_probe('custom'); " + std::string(success));
  auto    owner = f.owner();
  c4_failed_lua = 0;
  c4_one_shot   = true;
  stage         = [&](int n) {
    if (n == 12)
      c4_fail_lua = true;
  };
  auto result = owner.execute();
  stage       = {};
  c4_fail_lua = false;
  c4_one_shot = false;
  REQUIRE(result);
  CHECK(c4_failed_lua == 1);
  CHECK(owner.private_probe()[18] == 0);
}
TEST_CASE("C4C1 original typed history failure wins over later private projection guard", "[c4c1-private]") {
  fixture f("pcall(function() ctx.read_history({}) end); pcall(function() ctx.project_probe('custom') end); " +
            std::string(semantic));
  std::ofstream(f.root / "ledger", std::ios::trunc) << "bad";
  auto owner   = f.owner();
  bool entered = false;
  stage        = [&](int n) {
    if (n == 12)
      entered = true;
  };
  auto result = owner.execute();
  stage       = {};
  REQUIRE_FALSE(result);
  CHECK_FALSE(entered);
  REQUIRE(std::holds_alternative<format_error>(result.error()));
  CHECK(std::get<format_error>(result.error()).code == format_error_code::foreign_ledger);
}
