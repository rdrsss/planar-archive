module;
extern "C" {
#include "lauxlib.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstate.h"
#include "lua.h"
}
module planar.engine.x05_closure_audit;
import std;
namespace planar::engine::x05_closure_audit {
namespace {
// Allocating snapshot runs only in the caller's Lua protected C frame. Fixed trivial frames
// avoid all C++ destructor/longjmp interaction; catalog doubles as native identity memo.
void snapshot(lua_State* l, int object, int memo, int depth, std::size_t* edges) {
  if (++*edges > 1000000 || depth > 256 || !lua_checkstack(l, 8))
    luaL_error(l, "x05_snapshot_limit");
  int kind = lua_type(l, object);
  if (kind != LUA_TTABLE && kind != LUA_TFUNCTION)
    return;
  lua_pushvalue(l, object);
  lua_rawget(l, memo);
  bool seen = !lua_isnil(l, -1);
  lua_pop(l, 1);
  if (seen)
    return;
  lua_pushvalue(l, object);
  lua_pushboolean(l, true);
  lua_rawset(l, memo);
  if (kind == LUA_TTABLE) {
    lua_pushnil(l);
    while (lua_next(l, object)) {
      snapshot(l, lua_absindex(l, -2), memo, depth + 1, edges);
      snapshot(l, lua_absindex(l, -1), memo, depth + 1, edges);
      lua_pop(l, 1);
    }
  } else if (!lua_iscfunction(l, object)) {
    luaL_error(l, "x05_snapshot_not_pristine");
  }
}
// lua_checkstack belongs to the protected caller, before destructor-bearing native work.
// The graph admits relative top <=2040; pushnil plus lua_next need at most two more.
// Reserving relative capacity 2048 also covers the catalog/root and six spare slots.
constexpr int graph_stack_top = 2040;
void          require_stack(lua_State* l, int extra) {
  if (l->top.p > l->ci->top.p || l->ci->top.p - l->top.p < extra)
    throw failure::resource;
}
struct inspector {
  lua_State*                           l;
  int                                  catalog;
  limits                               bound;
  report                               result;
  std::pmr::unordered_set<const void*> seen, cells;
  inspector(lua_State* vm, int natives, limits b, std::pmr::memory_resource* arena)
      : l(vm), catalog(natives), bound(b), seen(arena), cells(arena) {
  }
  void charge(std::size_t& value, std::size_t maximum, std::size_t amount = 1) {
    if (amount > maximum || value > maximum - amount)
      throw failure::resource;
    value += amount;
  }
  void proto(const Proto* p, const std::bitset<256>& sealed, std::size_t depth) {
    if (!p || depth > bound.depth || p->sizeupvalues < 0 || p->sizeupvalues > 255 || p->sizecode < 0 || p->sizep < 0)
      throw failure::invalid_metadata;
    charge(result.prototypes, bound.prototypes);
    for (int pc = 0; pc < p->sizecode; ++pc) {
      charge(result.instructions, bound.instructions);
      auto word = p->code[pc];
      auto op   = GET_OPCODE(word);
      if (op >= NUM_OPCODES || op == OP_EXTRAARG)
        throw failure::invalid_metadata;
      if (op == OP_SETUPVAL) {
        auto target = GETARG_B(word);
        if (target >= p->sizeupvalues)
          throw failure::invalid_metadata;
        if (sealed[target])
          throw failure::sealed_write;
      }
      // Pinned 5.5 VM: NEWTABLE always consumes its payload, even k=0;
      // LOADKX always consumes one; SETLIST consumes one iff k is set.
      bool extra = op == OP_LOADKX || op == OP_NEWTABLE || (op == OP_SETLIST && TESTARG_k(word));
      if (op == OP_LOADKX)
        ++result.loadkx;
      if (op == OP_NEWTABLE)
        ++result.newtable;
      if (op == OP_SETLIST && TESTARG_k(word))
        ++result.setlist_extra;
      if (extra) {
        if (++pc >= p->sizecode || GET_OPCODE(p->code[pc]) != OP_EXTRAARG)
          throw failure::invalid_metadata;
        charge(result.instructions, bound.instructions);
      }
    }
    for (int n = 0; n < p->sizep; ++n) {
      auto child = p->p[n];
      if (!child || child->sizeupvalues < 0 || child->sizeupvalues > 255)
        throw failure::invalid_metadata;
      std::bitset<256> inherited;
      for (int u = 0; u < child->sizeupvalues; ++u) {
        auto descriptor = child->upvalues[u];
        if (!descriptor.instack) {
          if (descriptor.idx >= p->sizeupvalues)
            throw failure::invalid_metadata;
          inherited[u] = sealed[descriptor.idx];
        }
      }
      proto(child, inherited, depth + 1);
    }
  }
  void graph(int index, std::size_t depth, bool root = false) {
    charge(result.edges, bound.edges);
    if (depth > bound.depth || lua_gettop(l) > graph_stack_top)
      throw failure::resource;
    int kind = lua_type(l, index);
    if (kind == LUA_TFUNCTION && lua_iscfunction(l, index)) {
      if (root)
        throw failure::c_root;
      require_stack(l, 1);
      lua_pushvalue(l, index);
      lua_rawget(l, catalog);
      bool known = lua_toboolean(l, -1);
      lua_pop(l, 1);
      if (!known)
        throw failure::unknown_native;
      // Exact trusted native context is opaque; its upvalues are native-owned.
      return;
    }
    if (kind != LUA_TTABLE && kind != LUA_TFUNCTION) {
      if (kind != LUA_TNIL && kind != LUA_TBOOLEAN && kind != LUA_TNUMBER && kind != LUA_TSTRING)
        throw failure::unsupported_value;
      return;
    }
    if (!seen.insert(lua_topointer(l, index)).second)
      return;
    if (kind == LUA_TTABLE) {
      require_stack(l, 1);
      lua_pushnil(l);
      while ((require_stack(l, 1), lua_next(l, index))) {
        graph(lua_absindex(l, -2), depth + 1);
        graph(lua_absindex(l, -1), depth + 1);
        lua_pop(l, 1);
      }
    } else {
      auto closure = static_cast<const LClosure*>(lua_topointer(l, index));
      ++result.lua_closures;
      std::bitset<256> sealed;
      for (int n = 0; n < closure->nupvalues; ++n) {
        sealed[n] = true;
        cells.insert(closure->upvals[n]);
      }
      proto(closure->p, sealed, 1);
      for (int n = 1; (require_stack(l, 1), lua_getupvalue(l, index, n)); ++n) {
        graph(lua_absindex(l, -1), depth + 1);
        lua_pop(l, 1);
      }
    }
  }
};
} // namespace
void remember_installed_callback(lua_State* l, int slot, int index) {
  if (!lua_iscfunction(l, index))
    luaL_error(l, "x05_not_native");
  lua_rawgeti(l, LUA_REGISTRYINDEX, slot);
  lua_pushvalue(l, index);
  lua_pushboolean(l, true);
  lua_rawset(l, -3);
  lua_pop(l, 1);
}
void snapshot_curated_natives(lua_State* l, int slot) {
  lua_newtable(l);
  int memo = lua_gettop(l);
  lua_pushglobaltable(l);
  std::size_t edges = 0;
  snapshot(l, lua_gettop(l), memo, 1, &edges);
  lua_pop(l, 1);
  lua_rawseti(l, LUA_REGISTRYINDEX, slot);
}
std::expected<report, failure> audit_retained(lua_State* l, int root, int slot, limits bound) {
  int top = lua_gettop(l);
  // Algebra: headroom >=2048-top means this active frame authorizes relative top 2048.
  // Validate top before subtraction. This preflight performs no allocation and no Lua push.
  if (top < 0 || top > graph_stack_top || l->top.p > l->ci->top.p || l->ci->top.p - l->top.p < audit_stack_slots - top)
    return std::unexpected(failure::resource);
  if (!bound.native_bytes || bound.native_bytes > 64 * 1024 * 1024 || !bound.depth || bound.depth > 256)
    return std::unexpected(failure::resource);
  try {
    auto                                storage = std::make_unique<std::byte[]>(bound.native_bytes);
    std::pmr::monotonic_buffer_resource arena(storage.get(), bound.native_bytes, std::pmr::null_memory_resource());
    require_stack(l, 1);
    lua_rawgeti(l, LUA_REGISTRYINDEX, slot);
    if (!lua_istable(l, -1)) {
      lua_settop(l, top);
      return std::unexpected(failure::invalid_metadata);
    }
    inspector audit(l, lua_gettop(l), bound, &arena);
    audit.graph(root, 1, true);
    audit.result.cells = audit.cells.size();
    lua_settop(l, top);
    return audit.result;
  } catch (failure e) {
    lua_settop(l, top);
    return std::unexpected(e);
  } catch (const std::bad_alloc&) {
    lua_settop(l, top);
    return std::unexpected(failure::resource);
  }
}
} // namespace planar::engine::x05_closure_audit
