module;
extern "C" {
#include "lauxlib.h"
#include "lua.h"
}
#include <cstdlib>
module planar.engine.lua;
import std;
import planar.engine.budget;
namespace planar::engine {
namespace {
constexpr std::size_t memory_max = 64 * 1024 * 1024;
struct state {
  std::size_t                       memory = 0, instructions = 0, nodes = 0, native_bytes = 0, anchored = 0;
  diagnostic                        error;
  std::map<const value*, value_ptr> owned;
  std::set<const void*>             tables;
  std::set<std::string>             references;
  const value*                      pending = nullptr;
  std::shared_ptr<budget_sink>      budget;
  std::optional<diagnostic>         composed;
};
void* allocate(void* ud, void* p, std::size_t old, std::size_t size) {
  auto& s = *static_cast<state*>(ud);
  if (!p)
    old = 0;
  if (!size) {
    std::free(p);
    s.memory -= old;
    if (s.budget && old)
      s.budget->release_lua(old);
    return nullptr;
  }
  if (size > memory_max - s.memory + old)
    return nullptr;
  if (s.budget && size > old) {
    std::size_t delta   = size - old;
    bool        refused = false;
    {
      auto r = s.budget->acquire_lua(delta);
      if (!r) {
        s.composed = std::move(r.error());
        refused    = true;
      }
    }
    if (refused)
      return nullptr;
  }
  auto result = std::realloc(p, size);
  if (result) {
    s.memory = s.memory - old + size;
    if (s.budget && size < old)
      s.budget->release_lua(old - size);
  } else if (s.budget && size > old)
    s.budget->release_lua(size - old);
  return result;
}
state& context(lua_State* l) {
  return **static_cast<state**>(lua_getextraspace(l));
}
int denied(lua_State* l) {
  return luaL_error(l, "capability_denied");
}
void hook(lua_State* l, lua_Debug*) {
  auto& s = context(l);
  s.instructions += 100;
  bool refused = false;
  if (s.budget) {
    auto r = s.budget->charge_instructions(100);
    if (!r) {
      refused = true;
      if (s.instructions < 1000000)
        s.error = std::move(r.error());
    }
  }
  if (s.instructions >= 1000000 || refused)
    luaL_error(l, "instruction_limit");
}
[[noreturn]] void refuse(std::string code, std::string message) {
  throw diagnostic{std::move(code), std::move(message)};
}
void charge(state& s, std::size_t bytes) {
  if (bytes > memory_max - s.native_bytes)
    throw diagnostic{"resource_limit", "Native conversion allocation exceeded", "native_bytes", memory_max};
  if (s.budget) {
    auto r = s.budget->charge_native(bytes);
    if (!r)
      throw std::move(r.error());
  }
  s.native_bytes += bytes;
}
int anchor_table(lua_State* l) {
  lua_rawgeti(l, LUA_REGISTRYINDEX, LUA_RIDX_LAST + 1);
  lua_pushvalue(l, 1);
  lua_rawseti(l, -2, ++context(l).anchored);
  return 0;
}
value_ptr convert(lua_State* l, int index, std::size_t depth, bool list = false) {
  auto& s = context(l);
  if (depth > 64)
    throw diagnostic{"resource_limit", "Declaration depth exceeded", "depth", 64};
  if (++s.nodes > 100000)
    throw diagnostic{"resource_limit", "Declaration node count exceeded", "nodes", 100000};
  index = lua_absindex(l, index);
  charge(s, 512);
  auto v = std::make_shared<value>();
  switch (lua_type(l, index)) {
  case LUA_TBOOLEAN:
    v->kind    = value_kind::boolean;
    v->boolean = lua_toboolean(l, index);
    break;
  case LUA_TNUMBER:
    if (!lua_isinteger(l, index))
      refuse("schema", "Only integers are supported");
    v->kind    = value_kind::integer;
    v->integer = lua_tointeger(l, index);
    if (v->integer < 0 || v->integer > 2147483647)
      refuse("schema", "Integer outside uint range");
    break;
  case LUA_TSTRING: {
    std::size_t n;
    const char* p = lua_tolstring(l, index, &n);
    if (n > 65536)
      throw diagnostic{"resource_limit", "String too large", "string_bytes", 65536};
    if (!valid_text({p, n}))
      refuse("schema", "Invalid text");
    charge(s, n + 1);
    v->kind = value_kind::string;
    v->string.assign(p, n);
    break;
  }
  case LUA_TUSERDATA: {
    auto p = static_cast<const value**>(lua_touserdata(l, index));
    if (!p)
      refuse("schema", "Unsupported userdata");
    auto found = s.owned.find(*p);
    if (found != s.owned.end())
      return found->second;
    refuse("schema", "Invalid private value");
  }
  case LUA_TTABLE: {
    if (lua_getmetatable(l, index)) {
      lua_pop(l, 1);
      refuse("schema", "Metatables are forbidden");
    }
    if (!s.tables.insert(lua_topointer(l, index)).second)
      refuse("schema", "Shared or cyclic table");
    // Retain input identities until state teardown. The nested protected call
    // catches allocation longjmp before it can cross native conversion objects.
    lua_pushcfunction(l, anchor_table);
    lua_pushvalue(l, index);
    auto anchored = lua_pcall(l, 1, 0, 0);
    if (anchored) {
      if (s.composed)
        throw *s.composed;
      if (!s.error.code.empty())
        throw s.error;
    }
    if (anchored)
      throw diagnostic{"resource_limit", "Lua allocation exceeded", "lua_bytes", memory_max};
    v->kind = list ? value_kind::list : value_kind::record;
    std::map<std::int64_t, value_ptr> items;
    lua_pushnil(l);
    while (lua_next(l, index)) {
      if (list) {
        if (!lua_isinteger(l, -2) || lua_tointeger(l, -2) < 1)
          refuse("schema", "Invalid list key");
        charge(s, 128);
        items.emplace(lua_tointeger(l, -2), convert(l, -1, depth + 1));
      } else {
        if (lua_type(l, -2) != LUA_TSTRING)
          refuse("schema", "Record keys must be strings");
        std::size_t n;
        const char* p = lua_tolstring(l, -2, &n);
        if (n > 65536 || !valid_text({p, n}))
          refuse("schema", "Invalid record key");
        charge(s, 128 + n + 1);
        v->fields.emplace(std::string(p, n), convert(l, -1, depth + 1));
      }
      lua_pop(l, 1);
    }
    std::int64_t next = 1;
    for (auto& [key, item] : items) {
      if (key != next++)
        refuse("schema", "Sparse list");
      v->items.push_back(std::move(item));
    }
    break;
  }
  default:
    refuse("schema", "Unsupported declaration value");
  }
  s.owned.emplace(v.get(), v);
  return v;
}
void construct_native(lua_State* l) {
  auto kind = static_cast<value_kind>(lua_tointeger(l, lua_upvalueindex(1)));
  if (lua_gettop(l) != 1)
    refuse("schema", "Constructor expects one argument");
  if ((kind == value_kind::list || kind == value_kind::workspace || kind == value_kind::spec || kind == value_kind::job) &&
      lua_type(l, 1) != LUA_TTABLE)
    refuse("schema", "Constructor expects a plain table");
  auto  p = convert(l, 1, 1, kind == value_kind::list);
  auto& s = context(l);
  charge(s, 512 + p->string.size() + 1 + 128 * p->items.size());
  for (const auto& [key, child] : p->fields)
    charge(s, 128 + key.size() + 1);
  auto v = std::make_shared<value>(*p);
  if (kind == value_kind::md || kind == value_kind::file) {
    if (p->kind != value_kind::string)
      refuse("schema", "Text constructor expects a string");
    if (kind == value_kind::file) {
      if (!valid_path(p->string))
        refuse("path_denied", "Invalid file reference path");
      charge(s, 128 + p->string.size() + 1);
      context(l).references.insert(p->string);
    }
  } else if (kind == value_kind::immaterial) {
    if (p->kind != value_kind::md && p->kind != value_kind::file)
      refuse("schema", "Immaterial expects text");
    charge(s, 128);
    v->items = {p};
    v->string.clear();
  } else if (p->kind != (kind == value_kind::list ? value_kind::list : value_kind::record))
    refuse("schema", "Constructor expects a table");
  v->kind = kind;
  context(l).owned.emplace(v.get(), v);
  context(l).pending = v.get();
}
int construct(lua_State* l) {
  if (!lua_checkstack(l, 256))
    return luaL_error(l, "stack_limit");
  bool failed = false;
  try {
    construct_native(l);
  } catch (diagnostic& e) {
    context(l).error = std::move(e);
    failed           = true;
  } catch (...) {
    context(l).error = {"internal", "Native conversion failed"};
    failed           = true;
  }
  // All native temporaries have unwound before any allocating Lua operation.
  if (failed)
    return luaL_error(l, "native_error");
  auto p = static_cast<const value**>(lua_newuserdatauv(l, sizeof(const value*), 0));
  *p     = context(l).pending;
  luaL_setmetatable(l, "planar.value");
  return 1;
}
const value* member(lua_State* l) {
  auto self = *static_cast<const value**>(lua_touserdata(l, 1));
  if (self->kind == value_kind::list && lua_isinteger(l, 2)) {
    auto index = lua_tointeger(l, 2);
    if (index > 0 && static_cast<std::uint64_t>(index) <= self->items.size())
      return self->items[index - 1].get();
  }
  if (lua_type(l, 2) == LUA_TSTRING) {
    std::size_t size;
    auto        bytes = lua_tolstring(l, 2, &size);
    auto        found = self->fields.find(std::string(bytes, size));
    if (found != self->fields.end())
      return found->second.get();
  }
  return nullptr;
}
int index_value(lua_State* l) {
  // Lookup completes all native temporary lifetimes before pushing Lua values.
  auto selected = member(l);
  if (!selected) {
    lua_pushnil(l);
    return 1;
  }
  switch (selected->kind) {
  case value_kind::boolean:
    lua_pushboolean(l, selected->boolean);
    break;
  case value_kind::integer:
    lua_pushinteger(l, selected->integer);
    break;
  case value_kind::string:
    lua_pushlstring(l, selected->string.data(), selected->string.size());
    break;
  default:
    *static_cast<const value**>(lua_newuserdatauv(l, sizeof(const value*), 0)) = selected;
    luaL_setmetatable(l, "planar.value");
    break;
  }
  return 1;
}
int setup(lua_State* l) {
  if (!lua_checkstack(l, 256))
    return luaL_error(l, "stack_limit");
  lua_newtable(l);
  lua_rawseti(l, LUA_REGISTRYINDEX, LUA_RIDX_LAST + 1);
  luaL_newmetatable(l, "planar.value");
  lua_pushcfunction(l, denied);
  lua_setfield(l, -2, "__newindex");
  lua_pushcfunction(l, index_value);
  lua_setfield(l, -2, "__index");
  lua_pushboolean(l, 0);
  lua_setfield(l, -2, "__metatable");
  lua_pop(l, 1);
  lua_newtable(l); // backing namespace
  const char* names[] = {"list", "workspace", "spec", "job", "md", "file", "immaterial"};
  value_kind  kinds[] = {value_kind::list, value_kind::workspace, value_kind::spec,      value_kind::job,
                         value_kind::md,   value_kind::file,      value_kind::immaterial};
  for (int i = 0; i < 7; ++i) {
    lua_pushinteger(l, static_cast<int>(kinds[i]));
    lua_pushcclosure(l, construct, 1);
    lua_setfield(l, -2, names[i]);
  }
  lua_newtable(l);
  lua_newtable(l);
  lua_pushvalue(l, -3);
  lua_setfield(l, -2, "__index");
  lua_pushcfunction(l, denied);
  lua_setfield(l, -2, "__newindex");
  lua_pushboolean(l, 0);
  lua_setfield(l, -2, "__metatable");
  lua_setmetatable(l, -2);
  lua_newtable(l);
  lua_pushvalue(l, -2);
  lua_setfield(l, -2, "planar"); // environment backing
  lua_newtable(l);
  lua_newtable(l);
  lua_pushvalue(l, -3);
  lua_setfield(l, -2, "__index");
  lua_pushcfunction(l, denied);
  lua_setfield(l, -2, "__newindex");
  lua_setmetatable(l, -2);
  return 1;
}
void validate_graph(const value_ptr& v, std::set<const value*>& seen, std::size_t depth, std::size_t& nodes) {
  if (depth > 64)
    throw diagnostic{"resource_limit", "Declaration depth exceeded", "depth", 64};
  if (++nodes > 100000)
    throw diagnostic{"resource_limit", "Declaration nodes exceeded", "nodes", 100000};
  if (!seen.insert(v.get()).second)
    refuse("schema", "Shared constructor value");
  for (const auto& [key, child] : v->fields)
    validate_graph(child, seen, depth + 1, nodes);
  for (const auto& child : v->items)
    validate_graph(child, seen, depth + 1, nodes);
}
} // namespace
std::expected<declaration, diagnostic> load_declaration(std::string_view source, std::shared_ptr<budget_sink> budget) {
  if (source.size() > 1048576)
    return std::unexpected(diagnostic{"resource_limit", "Declaration too large", "input_bytes", 1048576});
  if (!valid_text(source))
    return std::unexpected(diagnostic{"schema", "Invalid declaration text"});
  if (!source.empty() && source[0] == '\x1b')
    return std::unexpected(diagnostic{"capability_denied", "Bytecode forbidden"});
  state s;
  s.budget = std::move(budget);
  auto l   = lua_newstate(allocate, &s, 0);
  if (!l)
    return s.composed ? std::unexpected(*s.composed)
                      : std::unexpected(diagnostic{"resource_limit", "Lua allocation exceeded", "lua_bytes", memory_max});
  *static_cast<state**>(lua_getextraspace(l)) = &s;
  auto close                                  = std::unique_ptr<lua_State, decltype(&lua_close)>(l, lua_close);
  lua_sethook(l, hook, LUA_MASKCOUNT, 100);
  lua_pushcfunction(l, setup);
  int status = lua_pcall(l, 0, 1, 0);
  if (!status) {
    status = luaL_loadbufferx(l, source.data(), source.size(), "declaration", "t");
    if (!status) {
      lua_pushvalue(l, -2);
      lua_setupvalue(l, -2, 1);
      status = lua_pcall(l, 0, LUA_MULTRET, 0);
    } else if (status != LUA_ERRMEM)
      return std::unexpected(diagnostic{"lua_syntax", "Invalid Lua text chunk"});
  }
  if (status) {
    if (!s.error.code.empty())
      return std::unexpected(std::move(s.error));
    if (status == LUA_ERRMEM && s.composed)
      return std::unexpected(*s.composed);
    if (status == LUA_ERRMEM)
      return std::unexpected(diagnostic{"resource_limit", "Lua allocation exceeded", "lua_bytes", memory_max});
    if (s.instructions >= 1000000)
      return std::unexpected(diagnostic{"resource_limit", "Lua instruction limit exceeded", "instructions", 1000000});
    return std::unexpected(diagnostic{"capability_denied", "Forbidden capability or mutation"});
  }
  if (lua_gettop(l) != 2)
    return std::unexpected(diagnostic{"schema", "Chunk must return exactly one declaration"});
  try {
    auto root = convert(l, -1, 1);
    if (root->kind != value_kind::workspace && root->kind != value_kind::spec && root->kind != value_kind::job)
      refuse("schema", "Expected workspace, spec or job");
    std::set<const value*> seen;
    std::size_t            nodes = 0;
    validate_graph(root, seen, 1, nodes);
    if (s.budget) {
      std::size_t retained = sizeof(declaration);
      for (const auto& r : s.references)
        retained += r.size() + sizeof(std::string);
      if (auto r = s.budget->charge_native(retained); !r)
        return std::unexpected(std::move(r.error()));
    }
    return declaration{std::move(root), {s.references.begin(), s.references.end()}};
  } catch (diagnostic& e) {
    return std::unexpected(std::move(e));
  }
}
} // namespace planar::engine
