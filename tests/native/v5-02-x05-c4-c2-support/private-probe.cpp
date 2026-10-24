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
registration.add({name='inspect',family='extension_read_v1',handler_id='custom.inspect',help='inspect',response_kind='extension_v1',capabilities=engine.array{'history_read_v1','projection_apply_v1'},options=engine.array{
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
TEST_CASE("C4C2 compiled layout and native initial charge", "[c4c2-layout]") {
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
}
TEST_CASE("C4C2 actual service protects outer frame across projection and assembly", "[c4c2-private]") {
  for (bool v2 : {false, true}) {
    fixture                     f("local h=ctx.read_history({}); local p=ctx.project({id='custom'}); " + std::string(success),
                                  "local t=0; for i=1,10000 do t=t+i end; return "
                                  "{version=1,sections=engine.array{{id='custom',text=input.ledger,mandatory=false}}}",
                                  v2);
    auto                        owner = f.owner();
    observed_vfs                observer;
    std::array<std::size_t, 19> before{}, after{}, assembled{};
    bool                        projected = false, finished = false;
    stage = [&](int n) {
      if (n == 10)
        before = owner.private_probe();
      if (n == 11) {
        after     = owner.private_probe();
        projected = true;
      }
      if (n == 21) {
        assembled = owner.private_probe();
        finished  = true;
      }
    };
    auto result = owner.execute();
    stage       = {};
    REQUIRE(result);
    CHECK(projected);
    CHECK(finished);
    CHECK(observer.reads > 0);
    CHECK(observer.opens == observer.closes);
    for (auto i : {8, 9, 10, 11, 12, 13, 14, 15, 16, 17}) {
      CHECK(before[i] == after[i]);
      CHECK(before[i] == assembled[i]);
    }
    for (auto i : {4, 5, 6}) {
      CHECK(after[i] >= before[i]);
      CHECK(assembled[i] >= after[i]);
    }
    auto clean = owner.private_probe();
    for (auto i : {8, 9, 13, 14, 15, 17})
      CHECK(clean[i] == 0);
  }
}
TEST_CASE("C4C2 real native and Lua allocation failure remain terminal with clean outer owner", "[c4c2-private]") {
  for (int mode : {12, 20, 21}) {
    fixture f("local h=ctx.read_history({}); pcall(function() ctx.project({id='custom'}) end); " + std::string(semantic));
    auto    owner = f.owner();
    bool    fired = false;
    c4_failed_lua = 0;
    stage         = [&](int n) {
      if (n == mode && !fired) {
        fired = true;
        if (mode == 20)
          c4_fail_lua = true;
        else
          fail_next = true;
      }
      if (n == 21 && mode == 20)
        c4_fail_lua = false;
    };
    auto result = owner.execute();
    stage       = {};
    c4_fail_lua = false;
    fail_next   = false;
    CHECK(fired);
    failure(result, "resource_limit");
    if (mode == 12)
      CHECK(std::get<registration_error>(result.error()).field == "catalog.projections[1]");
    if (mode != 12)
      CHECK(std::get<registration_error>(result.error()).field == "catalog.commands[6]");
    if (mode == 20)
      CHECK(c4_failed_lua >= 2);
    auto clean = owner.private_probe();
    for (auto i : {8, 9, 13, 14, 15, 17})
      CHECK(clean[i] == 0);
    failure(owner.execute(), "resource_limit");
  }
}
TEST_CASE("C4C2 actual projection instruction exhaustion is sticky", "[c4c2-private]") {
  fixture f("local h=ctx.read_history({}); pcall(function() ctx.project({id='custom'}) end); " + std::string(semantic),
            "while true do end");
  auto    owner  = f.owner();
  auto    result = owner.execute();
  failure(result, "resource_limit");
  CHECK(owner.private_probe()[6] >= 10000000);
}
TEST_CASE("C4C2 original receipts suppress fully assembled projection", "[c4c2-private]") {
  for (bool ledger : {false, true}) {
    fixture f("local h=ctx.read_history({}); local p=ctx.project({id='custom'}); return "
              "{ok=true,value={version=1,kind='extension',command=inv.command,data=p}}");
    auto    owner   = f.owner();
    bool    mutated = false;
    stage           = [&](int n) {
      if (n == 21) {
        f.mutate(ledger);
        mutated = true;
      }
    };
    auto result = owner.execute();
    stage       = {};
    CHECK(mutated);
    failure(result, "capture_changed");
  }
}
TEST_CASE("C4C2 lazy semantic handler does no native history work", "[c4c2-private]") {
  fixture f(semantic);
  std::ofstream(f.root / "ledger", std::ios::trunc) << "not sqlite";
  auto         owner = f.owner();
  observed_vfs observer;
  auto         result = owner.execute();
  REQUIRE(result);
  CHECK(observer.reads == 0);
  CHECK(observer.opens == 0);
}

TEST_CASE("C4C2 allocation admission distinguishes unowned E from retained exact E", "[c4c2-private]") {
  for (int mode : {31, 32}) {
    fixture f(
        "local h=ctx.read_history({}); pcall(function() ctx.project({id='custom'}) end); " + std::string(semantic),
        "local x='x'; for i=1,20 do x=x..x end; return {version=1,sections=engine.array{{id='required',text=x,mandatory=true}}}");
    auto owner    = f.owner();
    bool fired    = false;
    c4_failed_lua = 0;
    stage         = [&](int n) {
      if (n == mode && !fired) {
        fired = true;
        if (mode == 31)
          fail_next = true;
        else
          c4_fail_lua = true;
      }
    };
    auto result = owner.execute();
    stage       = {};
    fail_next   = false;
    c4_fail_lua = false;
    CHECK(fired);
    REQUIRE_FALSE(result);
    if (mode == 31)
      failure(result, "resource_limit");
    else {
      CHECK(c4_failed_lua >= 2);
      REQUIRE(result.error().index() == 3);
      value_ptr owned;
      std::visit(
          [&](const auto& e) {
            if constexpr (requires { e.error; })
              owned = e.error;
          },
          result.error());
      REQUIRE(owned);
      CHECK(owned->fields.at("code")->string == "budget_exceeded");
      CHECK(owned->fields.at("details")->fields.at("budget_bytes")->integer == 1048576);
      auto moved = std::move(owner);
      auto retry = moved.execute();
      REQUIRE_FALSE(retry);
      CHECK(retry.error().index() == 3);
    }
  }
}
TEST_CASE("C4C2 one failed Lua request with successful emergency retry stays healthy", "[c4c2-private]") {
  fixture f("local h=ctx.read_history({}); local p=ctx.project({id='custom'}); " + std::string(success));
  auto    owner = f.owner();
  bool    fired = false;
  c4_failed_lua = 0;
  c4_one_shot   = true;
  stage         = [&](int n) {
    if (n == 20) {
      fired       = true;
      c4_fail_lua = true;
    }
    if (n == 21)
      c4_fail_lua = false;
  };
  auto result = owner.execute();
  stage       = {};
  c4_fail_lua = false;
  c4_one_shot = false;
  REQUIRE(result);
  CHECK(fired);
  CHECK(c4_failed_lua == 1);
  CHECK(owner.private_probe()[18] == 0);
}

namespace {
void prior_projections(fixture& f) {
  auto          path = f.root / "trusted/module.lua";
  std::ifstream input(path);
  std::string   source(std::istreambuf_iterator<char>{input}, {});
  std::ofstream out(path);
  for (int i = 1; i <= 9; ++i)
    out << "registration.projection({id='prior" << i
        << "',abi='projection_v1',revision='r1',input='history_snapshot_v1',help='prior'},function() error('not selected') "
           "end)\n";
  out << source;
}
} // namespace
TEST_CASE("C4C2 selected projection failures preserve original ordinal through actual copy refusal and moves",
          "[c4c2-private][c4c2-location]") {
  for (int mode : {9, 12, 34, 35}) {
    fixture f("local h=ctx.read_history({}); pcall(function() ctx.project({id='custom'}) end); " + std::string(semantic),
              mode == 34   ? "return {version=1,sections=engine.array{{id='recorded_history',text='x',mandatory=false}}}"
              : mode == 35 ? "while true do end"
                           : "return {version=1,sections=engine.array{}}");
    prior_projections(f);
    auto owner  = f.owner();
    bool fired  = false;
    failed_size = 0;
    stage       = [&](int n) {
      if (n == (mode == 35 ? 34 : mode) && !fired) {
        fired     = true;
        fail_next = true;
      }
    };
    auto result = owner.execute();
    stage       = {};
    fail_next   = false;
    CHECK(fired);
    CHECK(failed_size > 0);
    failure(result, mode == 34 ? "internal" : "resource_limit");
    auto original = std::get<registration_error>(result.error());
    CHECK(original.field == "catalog.projections[10]");
    auto moved = std::move(owner);
    CHECK_FALSE(moved.verify());
    CHECK_FALSE(moved.admit("inspect", {}, {}));
    CHECK_FALSE(moved.prepare_history());
    auto retry = moved.execute();
    REQUIRE_FALSE(retry);
    REQUIRE(std::holds_alternative<registration_error>(retry.error()));
    const auto& repeated = std::get<registration_error>(retry.error());
    CHECK(repeated.field == original.field);
    CHECK(repeated.detail.code == original.detail.code);
    CHECK(repeated.detail.message == original.detail.message);
    CHECK(repeated.detail.limit == original.detail.limit);
    CHECK(repeated.detail.maximum == original.detail.maximum);
  }
}
TEST_CASE("C4C2 command admission and trusted assembly failures keep their own location", "[c4c2-private][c4c2-location]") {
  fixture f("pcall(function() ctx.project({id='custom'}) end); " + std::string(semantic));
  prior_projections(f);
  auto owner   = f.owner();
  bool entered = false;
  stage        = [&](int n) {
    if (n == 9 || n == 10)
      entered = true;
  };
  auto result = owner.execute();
  stage       = {};
  failure(result, "internal");
  CHECK_FALSE(entered);
  CHECK(std::get<registration_error>(result.error()).field == "catalog.commands[6]");
}
