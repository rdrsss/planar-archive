// Production pinned-metadata retained-cell guard regression checks.
#include <catch2/catch_test_macros.hpp>
extern "C" {
#include "lauxlib.h"
#include "lstate.h"
#include "lua.h"
#include "lualib.h"
}
#include <cstdlib>
import std;
import planar.engine.runtime.closure_guard;
namespace {
using namespace planar::engine::closure_guard;
constexpr int slot = LUA_RIDX_LAST + 3;
int           snapshot_call(lua_State* l) {
  snapshot_curated_natives(l, slot);
  return 0;
}
struct vm {
  lua_State* l = luaL_newstate();
  vm() {
    REQUIRE(l);
    luaL_requiref(l, "_G", luaopen_base, 1);
    lua_pop(l, 1);
    luaL_requiref(l, "string", luaopen_string, 1);
    lua_pop(l, 1);
    REQUIRE(lua_checkstack(l, 2048));
    lua_pushcfunction(l, snapshot_call);
    REQUIRE(lua_pcall(l, 0, 0, 0) == LUA_OK);
  }
  ~vm() {
    lua_close(l);
  }
  void source(std::string_view text) {
    REQUIRE(luaL_loadbufferx(l, text.data(), text.size(), "fixture", "t") == LUA_OK);
    REQUIRE(lua_pcall(l, 0, 1, 0) == LUA_OK);
    REQUIRE(lua_isfunction(l, -1));
  }
  auto audit(std::pmr::memory_resource& resource = *std::pmr::new_delete_resource()) {
    return audit_retained(l, lua_gettop(l), slot, resource);
  }
};
int native_counter(lua_State* l) {
  auto n = lua_tointeger(l, lua_upvalueindex(1));
  lua_pushinteger(l, n + 1);
  lua_replace(l, lua_upvalueindex(1));
  lua_pushinteger(l, n);
  return 1;
}
struct failing_allocator {
  bool        refuse   = false;
  std::size_t requests = 0;
  std::size_t live = 0, peak = 0, maximum = 64 * 1024 * 1024;
};
void* allocate(void* user, void* old, std::size_t previous, std::size_t bytes) {
  auto& owner = *static_cast<failing_allocator*>(user);
  if (!old)
    previous = 0;
  if (!bytes) {
    owner.live -= previous;
    std::free(old);
    return nullptr;
  }
  ++owner.requests;
  if (owner.refuse || bytes > owner.maximum || owner.live - previous > owner.maximum - bytes)
    return nullptr;
  auto next = std::realloc(old, bytes);
  if (next) {
    owner.live = owner.live - previous + bytes;
    owner.peak = std::max(owner.peak, owner.live);
  }
  return next;
}
struct frame_observation {
  failing_allocator*         allocator = nullptr;
  bool                       reserve = false, refuse_reservation = false, reserved = false, audited = false, accepted = false;
  std::optional<std::size_t> root;
  failure                    error          = failure::invalid_metadata;
  int                        entry_headroom = 0, exit_headroom = 0, before = 0, after = 0;
  std::size_t                before_requests = 0, after_requests = 0;
};
// Only trivial locals precede lua_checkstack; the audit's RAII begins after successful reservation.
int audit_frame(lua_State* l) {
  auto* observed           = static_cast<frame_observation*>(lua_touserdata(l, lua_upvalueindex(1)));
  observed->entry_headroom = static_cast<int>(l->ci->top.p - l->top.p);
  observed->before         = lua_gettop(l);
  if (observed->allocator) {
    observed->before_requests   = observed->allocator->requests;
    observed->allocator->refuse = observed->refuse_reservation;
  }
  if (observed->reserve) {
    observed->reserved = lua_checkstack(l, audit_stack_slots);
    if (!observed->reserved) {
      observed->error             = failure::resource;
      observed->after             = lua_gettop(l);
      observed->exit_headroom     = static_cast<int>(l->ci->top.p - l->top.p);
      observed->after_requests    = observed->allocator->requests;
      observed->allocator->refuse = false;
      return 0;
    }
  }
  {
    const int root     = 1;
    auto      result   = audit_roots_detailed(l, std::span<const int>(&root, 1), slot, *std::pmr::new_delete_resource());
    observed->audited  = true;
    observed->accepted = result.has_value();
    if (!result) {
      observed->error = result.error().code;
      observed->root  = result.error().root;
    }
  }
  observed->after         = lua_gettop(l);
  observed->exit_headroom = static_cast<int>(l->ci->top.p - l->top.p);
  if (observed->allocator) {
    observed->after_requests    = observed->allocator->requests;
    observed->allocator->refuse = false;
  }
  return 0;
}
void call_frame(lua_State* l, frame_observation& observed) {
  int top = lua_gettop(l);
  lua_pushlightuserdata(l, &observed);
  lua_pushcclosure(l, audit_frame, 1);
  lua_pushvalue(l, top);
  REQUIRE(lua_pcall(l, 1, 0, 0) == LUA_OK);
  CHECK(lua_gettop(l) == top);
  CHECK(observed.before == observed.after);
  CHECK(observed.exit_headroom >= 0);
}
} // namespace
TEST_CASE("X05 A1 rejects direct inherited shared and unreachable retained writes", "[x05-a1]") {
  for (auto text :
       {"local n=0;return function() n=n+1;return n end", "local n=0;return function() local function f() n=n+1 end;f() end",
        "local n=0;return function() local function f() local function g() n=n+1 end;g() end;f() end",
        "local n=0;local function writer() n=n+1 end;return function() return writer,n end",
        "local n=0;return function() if false then n=1 end;return n end"}) {
    INFO(text);
    vm v;
    v.source(text);
    auto result = v.audit();
    REQUIRE_FALSE(result);
    CHECK(result.error() == failure::sealed_write);
  }
}
TEST_CASE("X05 A1 permits invocation local nested counters and helpers", "[x05-a1]") {
  vm v;
  v.source("local stable=17;return function() local n=0;local function inc() n=n+1 end;inc();inc();return n+stable end");
  auto result = v.audit();
  REQUIRE(result);
  CHECK(result->lua_closures == 1);
  CHECK(result->prototypes == 2);
  lua_pushvalue(v.l, -1);
  REQUIRE(lua_pcall(v.l, 0, 1, 0) == LUA_OK);
  CHECK(lua_tointeger(v.l, -1) == 19);
  lua_pop(v.l, 1);
  lua_pushvalue(v.l, -1);
  REQUIRE(lua_pcall(v.l, 0, 1, 0) == LUA_OK);
  CHECK(lua_tointeger(v.l, -1) == 19);
}
TEST_CASE("X05 A1 audits actual helper closures with shared cell and graph aliases", "[x05-a1]") {
  vm v;
  v.source("local n=7;local function a() return n end;local function b() return n end;local t={a,a,b};t.self=t;return function() "
           "return t end");
  auto result = v.audit();
  REQUIRE(result);
  CHECK(result->lua_closures == 3);
  CHECK(result->cells == 2);
  vm unsafe;
  unsafe.source("local n=7;local function a() n=9 end;local t={a};t.self=t;return function() return t end");
  auto bad = unsafe.audit();
  REQUIRE_FALSE(bad);
  CHECK(bad.error() == failure::sealed_write);
}
TEST_CASE("X05 A1 distinguishes table alias freezing from lexical sealing", "[x05-a1]") {
  vm v;
  v.source("local t={n=1};local alias=t;return function() alias.n=2;return t.n end");
  REQUIRE(v.audit());
  lua_pushvalue(v.l, -1);
  REQUIRE(lua_pcall(v.l, 0, 1, 0) == LUA_OK);
  CHECK(lua_tointeger(v.l, -1) == 2);
}
TEST_CASE("X05 A1 refuses C roots retained iterators and same pointer new contexts", "[x05-a1]") {
  vm v;
  v.source("return string.len");
  auto root = v.audit();
  REQUIRE_FALSE(root);
  CHECK(root.error() == failure::c_root);
  vm stateful;
  stateful.source("local it=string.gmatch('abc','.');return function() return it() end");
  auto iterator = stateful.audit();
  REQUIRE_FALSE(iterator);
  CHECK(iterator.error() == failure::unknown_native);
  vm known;
  lua_pushinteger(known.l, 10);
  lua_pushcclosure(known.l, native_counter, 1);
  // Model a reviewed native install site with an exact identity, then create another context.
  remember_installed_callback(known.l, slot, lua_gettop(known.l));
  lua_setglobal(known.l, "safe_identity");
  lua_pushinteger(known.l, 20);
  lua_pushcclosure(known.l, native_counter, 1);
  lua_setglobal(known.l, "different_context");
  known.source("local f=different_context;return function() return f() end");
  auto different = known.audit();
  REQUIRE_FALSE(different);
  CHECK(different.error() == failure::unknown_native);
}
TEST_CASE("X05 A1 allows curated helpers and invocation local string iterators", "[x05-a1]") {
  vm v;
  v.source("local len=string.len;local match=string.gmatch;return function() local n=0;for x in match('abc','.') do n=n+len(x) "
           "end;return n end");
  REQUIRE(v.audit());
  lua_pushvalue(v.l, -1);
  REQUIRE(lua_pcall(v.l, 0, 1, 0) == LUA_OK);
  CHECK(lua_tointeger(v.l, -1) == 3);
}
namespace {
class cumulative_resource final : public std::pmr::memory_resource {
  void* do_allocate(std::size_t bytes, std::size_t alignment) override {
    if (bytes > maximum - charged)
      throw std::bad_alloc();
    charged += bytes;
    return std::pmr::new_delete_resource()->allocate(bytes, alignment);
  }
  void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override {
    std::pmr::new_delete_resource()->deallocate(p, bytes, alignment);
  }
  bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
    return this == &other;
  }

public:
  std::size_t charged = 0, maximum = 512 * 1024 * 1024;
};
} // namespace
TEST_CASE("X05 A1 repeated audits exhaust cumulative native accounting without refunds", "[x05-a1]") {
  vm v;
  v.source("local t={a={b=1}};return function() return t end");
  cumulative_resource resource;
  REQUIRE(v.audit(resource));
  const auto first = resource.charged;
  REQUIRE(first > 0);
  resource.maximum = 2 * first;
  REQUIRE(v.audit(resource));
  CHECK(resource.charged == 2 * first);
  const int top     = lua_gettop(v.l);
  auto      refused = v.audit(resource);
  REQUIRE_FALSE(refused);
  CHECK(refused.error() == failure::resource);
  CHECK(lua_gettop(v.l) == top);
  CHECK(resource.charged == 2 * first);
}
TEST_CASE("X05 A1 shared prototypes memoize exact masks across distinct actual closures", "[x05-a1]") {
  vm v;
  v.source("local t={};for i=1,1000 do t[i]=function() return i end end;return function() return t end");
  auto result = v.audit();
  REQUIRE(result);
  CHECK(result->lua_closures == 1001);
  CHECK(result->prototypes == 2);
  CHECK(result->cells == 1001);
}
TEST_CASE("X05 A1 retains graph depth refusal and restores stack", "[x05-a1]") {
  vm v;
  v.source("local t={};local p=t;for i=1,257 do p.next={};p=p.next end;return function() return t end");
  const int top    = lua_gettop(v.l);
  auto      result = v.audit();
  REQUIRE_FALSE(result);
  CHECK(result.error() == failure::depth);
  CHECK(lua_gettop(v.l) == top);
}
TEST_CASE("X05 A1 source compiled NEWTABLE and SETLIST payloads inspect and run", "[x05-a1]") {
  std::string source = "return function() local t={";
  for (int i = 0; i < 1200; ++i)
    source += "1,";
  source += "};return #t end";
  vm v;
  v.source(source);
  auto result = v.audit();
  REQUIRE(result);
  CHECK(result->newtable == 1);
  CHECK(result->setlist_extra > 0);
  lua_pushvalue(v.l, -1);
  REQUIRE(lua_pcall(v.l, 0, 1, 0) == LUA_OK);
  CHECK(lua_tointeger(v.l, -1) == 1200);
  vm empty;
  empty.source("return function() return {} end");
  auto small = empty.audit();
  REQUIRE(small);
  CHECK(small->newtable == 1);
}
TEST_CASE("X05 A1 native identity snapshot allocation failure stays protected", "[x05-a1]") {
  failing_allocator owner;
  auto              l = lua_newstate(allocate, &owner, 0);
  REQUIRE(l);
  REQUIRE(lua_checkstack(l, 2048));
  lua_pushcfunction(l, snapshot_call);
  owner.refuse = true;
  auto result  = lua_pcall(l, 0, 0, 0);
  owner.refuse = false;
  CHECK(result == LUA_ERRMEM);
  lua_close(l);
}

TEST_CASE("X05 A1 source compiled LOADKX fits actual module and Lua memory ceilings", "[x05-a1]") {
  // Pinned SIZE_Bx=17: first LOADKX needs constant index 131072, hence 131073 constants.
  // Fixed three-character base62 strings are unique and keep source under one MiB.
  constexpr std::string_view alphabet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  std::string                source   = "return function() local t={";
  for (std::size_t n = 0; n < 131073; ++n) {
    source += "'";
    source += alphabet[n / (62 * 62)];
    source += alphabet[(n / 62) % 62];
    source += alphabet[n % 62];
    source += "',";
  }
  source += "};return t[131073] end";
  REQUIRE(source.size() <= 1048576);
  failing_allocator owner;
  auto              raw = lua_newstate(allocate, &owner, 0);
  REQUIRE(raw);
  std::unique_ptr<lua_State, decltype(&lua_close)> l(raw, lua_close);
  REQUIRE(lua_checkstack(raw, 2048));
  lua_pushcfunction(raw, snapshot_call);
  REQUIRE(lua_pcall(raw, 0, 0, 0) == LUA_OK);
  REQUIRE(luaL_loadbufferx(raw, source.data(), source.size(), "loadkx-source", "t") == LUA_OK);
  REQUIRE(lua_pcall(raw, 0, 1, 0) == LUA_OK);
  auto result = audit_retained(raw, lua_gettop(raw), slot, *std::pmr::new_delete_resource());
  REQUIRE(result);
  INFO("source_bytes=" << source.size() << " peak_lua_bytes=" << owner.peak << " LOADKX=" << result->loadkx
                       << " inspected_words=" << result->instructions);
  CHECK(result->loadkx > 0);
  CHECK(owner.peak <= 67108864);
  lua_pushvalue(raw, -1);
  REQUIRE(lua_pcall(raw, 0, 1, 0) == LUA_OK);
  CHECK(std::string_view(lua_tostring(raw, -1)) == "Y64");
  CHECK(owner.peak <= 67108864);
}

TEST_CASE("X05 A1 unreserved C frame refuses before Lua allocation or push", "[x05-a1]") {
  failing_allocator owner;
  auto              l = lua_newstate(allocate, &owner, 0);
  REQUIRE(l);
  REQUIRE(lua_checkstack(l, audit_stack_slots));
  constexpr std::string_view source = "local t={};local p=t;for i=1,100 do p.next={};p=p.next end;return function() return t end";
  REQUIRE(luaL_loadbufferx(l, source.data(), source.size(), "deep-frame", "t") == LUA_OK);
  REQUIRE(lua_pcall(l, 0, 1, 0) == LUA_OK);
  frame_observation observed;
  observed.allocator          = &owner;
  observed.refuse_reservation = true;
  call_frame(l, observed);
  CHECK(observed.entry_headroom == LUA_MINSTACK);
  CHECK(observed.audited);
  CHECK_FALSE(observed.accepted);
  CHECK(observed.error == failure::resource);
  CHECK(observed.before_requests == observed.after_requests);
  CHECK(observed.entry_headroom == observed.exit_headroom);
  CHECK_FALSE(observed.root);
  lua_close(l);
  CHECK(owner.live == 0);
}
TEST_CASE("X05 A1 reserved active C frame audits realistic deep retained graph", "[x05-a1]") {
  vm v;
  v.source("local t={};local p=t;for i=1,100 do p.next={};p=p.next end;return function() return t end");
  frame_observation observed;
  observed.reserve = true;
  call_frame(v.l, observed);
  CHECK(observed.entry_headroom == LUA_MINSTACK);
  CHECK(observed.reserved);
  CHECK(observed.audited);
  CHECK(observed.accepted);
  CHECK(observed.exit_headroom >= audit_stack_slots);
}
TEST_CASE("X05 A1 active C frame reservation allocation refusal preserves state", "[x05-a1]") {
  failing_allocator owner;
  auto              l = lua_newstate(allocate, &owner, 0);
  REQUIRE(l);
  constexpr std::string_view source = "return function() return 17 end";
  REQUIRE(luaL_loadbufferx(l, source.data(), source.size(), "reservation", "t") == LUA_OK);
  REQUIRE(lua_pcall(l, 0, 1, 0) == LUA_OK);
  frame_observation observed;
  observed.allocator          = &owner;
  observed.reserve            = true;
  observed.refuse_reservation = true;
  call_frame(l, observed);
  CHECK(observed.entry_headroom == LUA_MINSTACK);
  CHECK_FALSE(observed.reserved);
  CHECK_FALSE(observed.audited);
  CHECK(observed.error == failure::resource);
  CHECK(observed.after_requests > observed.before_requests);
  lua_pushvalue(l, 1);
  REQUIRE(lua_pcall(l, 0, 1, 0) == LUA_OK);
  CHECK(lua_tointeger(l, -1) == 17);
  lua_close(l);
  CHECK(owner.live == 0);
}

TEST_CASE("X05 A1 ordered roots share memo and retain existing global depth boundary", "[x05-a1]") {
  vm v;
  v.source("deep={};local p=deep;for i=1,254 do p.next={};p=p.next end;return function() return deep end");
  lua_pushglobaltable(v.l);
  const std::array roots{lua_gettop(v.l), 1};
  auto             result = audit_roots(v.l, roots, slot, *std::pmr::new_delete_resource());
  REQUIRE(result);
  CHECK(result->lua_closures == 1);
  CHECK(lua_gettop(v.l) == 2);
}
TEST_CASE("X05 A1 shared roots still inspect newly reachable writers and reject native callable roots", "[x05-a1]") {
  vm v;
  v.source("local n=0;local function write() n=n+1 end;return function() return write end");
  lua_pushglobaltable(v.l);
  const std::array roots{lua_gettop(v.l), 1};
  auto             result = audit_roots(v.l, roots, slot, *std::pmr::new_delete_resource());
  REQUIRE_FALSE(result);
  CHECK(result.error() == failure::sealed_write);
  lua_getglobal(v.l, "print");
  const std::array native_roots{2, 3};
  result = audit_roots(v.l, native_roots, slot, *std::pmr::new_delete_resource());
  REQUIRE_FALSE(result);
  CHECK(result.error() == failure::c_root);
}

TEST_CASE("X05 A2 prior successful identities are opaque without trusting empty tables", "[x05-a2]") {
  vm v;
  v.source("local n=0;return function() n=n+1 end");
  constexpr int sealed_slot = slot + 6;
  lua_newtable(v.l);
  lua_pushvalue(v.l, 1);
  lua_pushboolean(v.l, true);
  lua_rawset(v.l, -3);
  lua_rawseti(v.l, LUA_REGISTRYINDEX, sealed_slot);
  const std::array roots{1};
  // Only native runtime may publish after successful freeze; this fixture models that trust boundary.
  auto result = audit_roots(v.l, roots, slot, *std::pmr::new_delete_resource(), sealed_slot);
  REQUIRE(result);
  CHECK(result->lua_closures == 0);
  auto raw = v.audit();
  REQUIRE_FALSE(raw);
  CHECK(raw.error() == failure::sealed_write);
}
TEST_CASE("X05 A2 successful atoms do not bless new writers or direct native roots", "[x05-a2]") {
  vm v;
  v.source("return function() return string.len end");
  constexpr int sealed_slot = slot + 6;
  lua_newtable(v.l);
  lua_pushvalue(v.l, 1);
  lua_pushboolean(v.l, true);
  lua_rawset(v.l, -3);
  lua_rawseti(v.l, LUA_REGISTRYINDEX, sealed_slot);
  v.source("local n=0;return function() n=n+1 end");
  const std::array roots{1, 2};
  auto             result = audit_roots(v.l, roots, slot, *std::pmr::new_delete_resource(), sealed_slot);
  REQUIRE_FALSE(result);
  CHECK(result.error() == failure::sealed_write);
  lua_getglobal(v.l, "print");
  const std::array natives{3};
  result = audit_roots(v.l, natives, slot, *std::pmr::new_delete_resource(), sealed_slot);
  REQUIRE_FALSE(result);
  CHECK(result.error() == failure::c_root);
}
TEST_CASE("X05 A2 invalid successful identity registry refuses without stack mutation", "[x05-a2]") {
  vm v;
  v.source("return function() return 1 end");
  const std::array roots{1};
  auto             result = audit_roots(v.l, roots, slot, *std::pmr::new_delete_resource(), slot + 6);
  REQUIRE_FALSE(result);
  CHECK(result.error() == failure::invalid_metadata);
  CHECK(lua_gettop(v.l) == 1);
}
namespace {
struct throwing_resource final : std::pmr::memory_resource {
  bool  unusual = false;
  void* do_allocate(std::size_t, std::size_t) override {
    if (unusual)
      throw 17;
    throw std::bad_alloc{};
  }
  void do_deallocate(void*, std::size_t, std::size_t) override {
  }
  bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
    return this == &other;
  }
};
} // namespace
TEST_CASE("X05 A1 C02 detailed guard reports selected roots and preserves wrapper", "[x05-a1]") {
  for (std::size_t bad = 0; bad < 3; ++bad) {
    vm v;
    for (std::size_t i = 0; i < 3; ++i)
      v.source(i == bad ? "local n=0;return function() n=n+1 end" : "return function() end");
    std::array<int, 3> roots{1, 2, 3};
    int                top    = lua_gettop(v.l);
    auto               detail = audit_roots_detailed(v.l, roots, slot, *std::pmr::new_delete_resource());
    REQUIRE_FALSE(detail);
    CHECK(detail.error().code == failure::sealed_write);
    CHECK(detail.error().root == bad);
    CHECK(lua_gettop(v.l) == top);
    auto old = audit_roots(v.l, roots, slot, *std::pmr::new_delete_resource());
    REQUIRE_FALSE(old);
    CHECK(old.error() == detail.error().code);
    CHECK(lua_gettop(v.l) == top);
  }
}
TEST_CASE("X05 A1 C02 detailed guard distinguishes pre root failures and restores stack", "[x05-a1]") {
  vm v;
  v.source("return function() end");
  std::array<int, 1> roots{1};
  int                top     = lua_gettop(v.l);
  auto               invalid = audit_roots_detailed(v.l, roots, slot + 1, *std::pmr::new_delete_resource());
  REQUIRE_FALSE(invalid);
  CHECK(invalid.error().code == failure::invalid_metadata);
  CHECK_FALSE(invalid.error().root);
  CHECK(lua_gettop(v.l) == top);
  throwing_resource resource;
  auto              allocation = audit_roots_detailed(v.l, roots, slot, resource);
  REQUIRE_FALSE(allocation);
  CHECK(allocation.error().code == failure::resource);
  CHECK(allocation.error().root == 0);
  CHECK(lua_gettop(v.l) == top);
  resource.unusual = true;
  CHECK_THROWS_AS(audit_roots_detailed(v.l, roots, slot, resource), int);
  CHECK(lua_gettop(v.l) == top);
  CHECK_THROWS_AS(audit_roots(v.l, roots, slot, resource), int);
  CHECK(lua_gettop(v.l) == top);
}
TEST_CASE("X05 A1 C02 detailed guard retains shared memo and report", "[x05-a1]") {
  vm v;
  v.source("local shared={};return function() return shared end");
  lua_pushvalue(v.l, 1);
  std::array<int, 2> roots{1, 2};
  auto               detail = audit_roots_detailed(v.l, roots, slot, *std::pmr::new_delete_resource());
  auto               old    = audit_roots(v.l, roots, slot, *std::pmr::new_delete_resource());
  REQUIRE(detail);
  REQUIRE(old);
  CHECK(detail->lua_closures == 1);
  CHECK(detail->lua_closures == old->lua_closures);
  CHECK(detail->edges == old->edges);
  CHECK(detail->prototypes == old->prototypes);
  CHECK(lua_gettop(v.l) == 2);
}
