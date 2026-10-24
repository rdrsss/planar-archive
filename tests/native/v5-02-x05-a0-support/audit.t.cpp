// Test-only source compiler and retained-cell audit feasibility checks.
#include <catch2/catch_test_macros.hpp>
extern "C" {
#include "lauxlib.h"
#include "lstate.h"
#include "lua.h"
#include "lualib.h"
}
#include <cstdlib>
import std;
import planar.engine.x05_closure_audit;
namespace {
using namespace planar::engine::x05_closure_audit;
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
  auto audit(limits b = {}) {
    return audit_retained(l, lua_gettop(l), slot, b);
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
  failing_allocator* allocator = nullptr;
  bool               reserve = false, refuse_reservation = false, reserved = false, audited = false, accepted = false;
  failure            error          = failure::invalid_metadata;
  int                entry_headroom = 0, exit_headroom = 0, before = 0, after = 0;
  std::size_t        before_requests = 0, after_requests = 0;
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
    auto result        = audit_retained(l, 1, slot);
    observed->audited  = true;
    observed->accepted = result.has_value();
    if (!result)
      observed->error = result.error();
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
TEST_CASE("X05 A0 rejects direct inherited shared and unreachable retained writes", "[x05-a0]") {
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
TEST_CASE("X05 A0 permits invocation local nested counters and helpers", "[x05-a0]") {
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
TEST_CASE("X05 A0 audits actual helper closures with shared cell and graph aliases", "[x05-a0]") {
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
TEST_CASE("X05 A0 distinguishes table alias freezing from lexical sealing", "[x05-a0]") {
  vm v;
  v.source("local t={n=1};local alias=t;return function() alias.n=2;return t.n end");
  REQUIRE(v.audit());
  lua_pushvalue(v.l, -1);
  REQUIRE(lua_pcall(v.l, 0, 1, 0) == LUA_OK);
  CHECK(lua_tointeger(v.l, -1) == 2);
}
TEST_CASE("X05 A0 refuses C roots retained iterators and same pointer new contexts", "[x05-a0]") {
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
TEST_CASE("X05 A0 allows curated helpers and invocation local string iterators", "[x05-a0]") {
  vm v;
  v.source("local len=string.len;local match=string.gmatch;return function() local n=0;for x in match('abc','.') do n=n+len(x) "
           "end;return n end");
  REQUIRE(v.audit());
  lua_pushvalue(v.l, -1);
  REQUIRE(lua_pcall(v.l, 0, 1, 0) == LUA_OK);
  CHECK(lua_tointeger(v.l, -1) == 3);
}
TEST_CASE("X05 A0 checks graph prototype instruction depth and arena ceilings", "[x05-a0]") {
  vm v;
  v.source("local t={a={b=1}};return function() return t end");
  for (int i = 0; i < 5; ++i) {
    limits b;
    if (i == 0)
      b.edges = 1;
    if (i == 1)
      b.prototypes = 0;
    if (i == 2)
      b.instructions = 1;
    if (i == 3)
      b.depth = 1;
    if (i == 4)
      b.native_bytes = 1;
    int  top    = lua_gettop(v.l);
    auto result = v.audit(b);
    REQUIRE_FALSE(result);
    CHECK(result.error() == failure::resource);
    CHECK(lua_gettop(v.l) == top);
  }
  REQUIRE(v.audit());
}
TEST_CASE("X05 A0 source compiled NEWTABLE and SETLIST payloads inspect and run", "[x05-a0]") {
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
TEST_CASE("X05 A0 native identity snapshot allocation failure stays protected", "[x05-a0]") {
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

TEST_CASE("X05 A0 source compiled LOADKX fits actual module and Lua memory ceilings", "[x05-a0]") {
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
  auto result = audit_retained(raw, lua_gettop(raw), slot);
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

TEST_CASE("X05 A0 unreserved C frame refuses before Lua allocation or push", "[x05-a0]") {
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
  lua_close(l);
  CHECK(owner.live == 0);
}
TEST_CASE("X05 A0 reserved active C frame audits realistic deep retained graph", "[x05-a0]") {
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
TEST_CASE("X05 A0 active C frame reservation allocation refusal preserves state", "[x05-a0]") {
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
