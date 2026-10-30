module;
extern "C" {
#include "lauxlib.h"
#include "lstate.h"
#include "lua.h"
#include "lualib.h"
}
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
// Pinned libc++ allocation identities for concrete native codec staging.
#include <map>
#include <memory>
module planar.engine.runtime;
import planar.engine.runtime_source;
import std;
import planar.engine.hash;
import planar.engine.capture.host_path;
import planar.engine.json_codec;
import planar.engine.runtime.closure_guard;
import planar.engine.capture;
import planar.engine.history.effective_catalog;
import planar.engine.history.effective_provenance;
namespace planar::engine {
namespace {
constexpr std::size_t command_catalog_size = 5;
// The four ceilings default to the contract values; verification variants override them at compile time.
#ifndef PLANAR_RUNTIME_MEMORY_MAX
#define PLANAR_RUNTIME_MEMORY_MAX 512 * 1024 * 1024
#endif
#if PLANAR_RUNTIME_MEMORY_MAX == 512 * 1024 * 1024
constexpr std::size_t memory_max = 512 * 1024 * 1024;
#else
constexpr std::size_t memory_max = PLANAR_RUNTIME_MEMORY_MAX;
#endif
#ifndef PLANAR_RUNTIME_INSTRUCTION_MAX
#define PLANAR_RUNTIME_INSTRUCTION_MAX 100000000
#endif
#if PLANAR_RUNTIME_INSTRUCTION_MAX == 100000000
constexpr std::size_t instruction_max = 100000000;
#else
constexpr std::size_t instruction_max = PLANAR_RUNTIME_INSTRUCTION_MAX;
#endif
#ifndef PLANAR_RUNTIME_NODE_MAX
#define PLANAR_RUNTIME_NODE_MAX 4000000
#endif
#if PLANAR_RUNTIME_NODE_MAX == 4000000
constexpr std::size_t node_max = 4000000;
#else
constexpr std::size_t node_max = PLANAR_RUNTIME_NODE_MAX;
#endif
#ifndef PLANAR_RUNTIME_REGISTRATION_INSTRUCTION_MAX
#define PLANAR_RUNTIME_REGISTRATION_INSTRUCTION_MAX 10000000
#endif
constexpr std::size_t depth_max                  = 256;
constexpr int         tags_index                 = LUA_RIDX_LAST + 1;
constexpr int         backing_index              = tags_index + 1;
constexpr int         native_catalog_index       = LUA_RIDX_LAST + 4;
constexpr int         sealed_index               = native_catalog_index + 1;
constexpr int         external_environment_index = sealed_index + 1;
// Address keys cannot overlap the integer namespace allocated by luaL_ref.
constexpr char        shape_tags_key = 0, shape_backings_key = 0;
constexpr const char* kind_names[] = {"boolean", "integer", "string", "object", "array",     "workspace",
                                      "spec",    "job",     "md",     "file",   "immaterial"};
struct command_state;
struct factory_budget;
struct state {
  std::shared_ptr<factory_budget> budget; ///< Factory sink kept alive for receipts; its target is nulled at VM teardown.
  std::size_t               carried_instructions = 0; ///< Pre-frame declaration instructions carried into the invocation counter.
  command_state*            command              = nullptr;
  std::size_t               memory_limit = memory_max, instruction_limit = instruction_max;
  bool                      exhausted = false, lua_memory_failed = false, stack_exhausted = false, seal_failed = false;
  std::optional<diagnostic> conversion_failure;
  std::size_t               memory = 0, instructions = 0, nodes = 0, native_bytes = 0;
  closure_guard::report     seal_inspections;
  diagnostic                error;
  value_ptr                 input;
  value_ptr                 codec_result;
  bool                      semantic_active = false, native_memory_failed = false;
  bool                      preserving_aliases = false;
  int                       push_memo          = 0;
  std::string_view          entry;
  std::string               pending;
  std::set<const void*>     active;
};
state& context(lua_State* l) {
  return **static_cast<state**>(lua_getextraspace(l));
}
void* allocate(void* ud, void* p, std::size_t old, std::size_t size) {
  auto& s = *static_cast<state*>(ud);
  if (!p)
    old = 0;
  if (!size) {
    std::free(p);
    s.memory -= old;
    return nullptr;
  }
  if (size > s.memory_limit - s.memory + old) {
    s.exhausted = true;
    return nullptr;
  }
  auto result = std::realloc(p, size);
  if (result)
    s.memory = s.memory - old + size;
  return result;
}
void hook(lua_State* l, lua_Debug* event) {
  auto& s = context(l);
  if (event->event == LUA_HOOKCOUNT)
    s.instructions += 1000;
  if (s.command && (l->top.p - l->stack.p > 2048 || (isLua(l->ci) && l->ci->top.p - l->stack.p > 2048))) {
    s.stack_exhausted = true;
    return (void)luaL_error(l, "stack_limit");
  }
  if (s.instructions >= s.instruction_limit)
    luaL_error(l, "instruction_limit");
}
// Only trivial locals may survive these protected frames: Lua errors longjmp.
int protected_call(lua_State* l) {
  luaL_checkany(l, 1);
  lua_pushboolean(l, true);
  lua_insert(l, 1);
  int status = lua_pcall(l, lua_gettop(l) - 2, LUA_MULTRET, 0);
  // An initial allocator refusal may recover during Lua's emergency GC retry.
  // Only the completed protected operation proves terminal memory failure.
  if (status == LUA_ERRMEM)
    context(l).lua_memory_failed = true;
  // lua_pcall restores hook execution before this C frame resumes. Never
  // return control to Lua after exhaustion, even when an enclosing pcall
  // catches the error or the callee returns immediately after catching it.
  if (context(l).instructions >= context(l).instruction_limit || context(l).exhausted || context(l).lua_memory_failed ||
      context(l).stack_exhausted || context(l).native_memory_failed || context(l).conversion_failure)
    return luaL_error(l, "instruction_limit");
  lua_pushboolean(l, status == LUA_OK);
  lua_replace(l, 1);
  return lua_gettop(l);
}
[[noreturn]] void refuse() {
  throw diagnostic{"internal", "Unsupported trusted structural value"};
}
void charge(state& s, std::size_t bytes) {
  if (bytes > memory_max - s.native_bytes) {
    if (s.command)
      s.conversion_failure = diagnostic{"resource_limit", "Native conversion allocation exceeded", "native_bytes", memory_max};
    throw diagnostic{"resource_limit", "Native conversion allocation exceeded", "native_bytes", memory_max};
  }
  s.native_bytes += bytes;
}
// Allocation requests share the VM's cumulative native conversion accounting.
// Deallocation releases storage without refunding work already charged.
class closure_memory final : public std::pmr::memory_resource {
  state& _owner;
  void*  do_allocate(std::size_t bytes, std::size_t alignment) override {
    charge(_owner, bytes);
    return std::pmr::new_delete_resource()->allocate(bytes, alignment);
  }
  void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override {
    std::pmr::new_delete_resource()->deallocate(p, bytes, alignment);
  }
  bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
    return this == &other;
  }

public:
  explicit closure_memory(state& owner) : _owner(owner) {
  }
};
void visit(state& s, std::size_t depth) {
  if (depth > depth_max) {
    if (s.command)
      s.conversion_failure = diagnostic{"resource_limit", "Conversion depth exceeded", "depth", depth_max};
    throw diagnostic{"resource_limit", "Conversion depth exceeded", "depth", depth_max};
  }
  if (++s.nodes > node_max) {
    if (s.command)
      s.conversion_failure = diagnostic{"resource_limit", "Conversion node count exceeded", "nodes", node_max};
    throw diagnostic{"resource_limit", "Conversion node count exceeded", "nodes", node_max};
  }
  charge(s, 512);
}
struct active_visit {
  state&      owner;
  const void* identity;
  ~active_visit() {
    owner.active.erase(identity);
  }
};
void validate(state& s, const value_ptr& v, std::size_t depth) {
  visit(s, depth);
  if (!v || !s.active.insert(v.get()).second)
    refuse();
  active_visit cleanup{s, v.get()};
  if (v->kind < value_kind::boolean || v->kind > value_kind::immaterial)
    refuse();
  if (s.command && v->kind == value_kind::integer && (v->integer < -2147483648LL || v->integer > 2147483647LL))
    refuse();
  charge(s, v->string.size());
  bool fields = v->kind == value_kind::record || v->kind == value_kind::workspace || v->kind == value_kind::spec ||
                v->kind == value_kind::job;
  bool items  = v->kind == value_kind::list || v->kind == value_kind::immaterial;
  bool text   = v->kind == value_kind::string || v->kind == value_kind::md || v->kind == value_kind::file;
  if ((!fields && !v->fields.empty()) || (!items && !v->items.empty()) || (!text && !v->string.empty()) ||
      (v->kind != value_kind::boolean && v->boolean) || (v->kind != value_kind::integer && v->integer) ||
      (v->kind == value_kind::immaterial &&
       (v->items.size() != 1 || !v->items[0] || (v->items[0]->kind != value_kind::md && v->items[0]->kind != value_kind::file))))
    refuse();
  for (const auto& [key, child] : v->fields) {
    if (!valid_text(key))
      refuse();
    charge(s, key.size() + 128);
    validate(s, child, depth + 1);
  }
  for (const auto& child : v->items)
    validate(s, child, depth + 1);
}
// Called exclusively inside protected Lua frames. Only trivial native locals
// survive allocating Lua operations; input ownership remains in state.
void tag(lua_State* l, int index, value_kind kind) {
  index = lua_absindex(l, index);
  lua_rawgeti(l, LUA_REGISTRYINDEX, tags_index);
  lua_pushvalue(l, index);
  lua_pushinteger(l, static_cast<int>(kind));
  lua_rawset(l, -3);
  lua_pop(l, 1);
}
void push(lua_State* l, const value* v) {
  switch (v->kind) {
  case value_kind::boolean:
    lua_pushboolean(l, v->boolean);
    return;
  case value_kind::integer:
    lua_pushinteger(l, v->integer);
    return;
  case value_kind::string:
    lua_pushlstring(l, v->string.data(), v->string.size());
    return;
  default:
    break;
  }
  if (context(l).preserving_aliases) {
    lua_pushlightuserdata(l, const_cast<value*>(v));
    lua_rawget(l, context(l).push_memo);
    if (!lua_isnil(l, -1))
      return;
    lua_pop(l, 1);
  }
  lua_newtable(l);
  if (context(l).preserving_aliases) {
    lua_pushlightuserdata(l, const_cast<value*>(v));
    lua_pushvalue(l, -2);
    lua_rawset(l, context(l).push_memo);
  }
  tag(l, -1, v->kind);
  if (v->kind == value_kind::md || v->kind == value_kind::file) {
    lua_pushlstring(l, v->string.data(), v->string.size());
    lua_setfield(l, -2, "value");
  } else if (v->kind == value_kind::immaterial) {
    push(l, v->items[0].get());
    lua_setfield(l, -2, "value");
  } else {
    for (const auto& [key, child] : v->fields) {
      lua_pushlstring(l, key.data(), key.size());
      push(l, child.get());
      lua_rawset(l, -3);
    }
    for (std::size_t i = 0; i < v->items.size(); ++i) {
      push(l, v->items[i].get());
      lua_rawseti(l, -2, static_cast<lua_Integer>(i + 1));
    }
  }
}
value_kind table_kind(lua_State* l, int index) {
  index = lua_absindex(l, index);
  lua_rawgeti(l, LUA_REGISTRYINDEX, tags_index);
  lua_pushvalue(l, index);
  lua_rawget(l, -2);
  auto kind = lua_isinteger(l, -1) ? static_cast<value_kind>(lua_tointeger(l, -1)) : value_kind::record;
  lua_pop(l, 2);
  return kind;
}
// This sole union applies to both input kinds, including conditional and generated sections.
constexpr std::array<std::string_view, 18> projection_reserved_ids{
    "authority",    "goal",       "acceptance", "requirements",    "contracts",   "boundary",  "dependencies",
    "shape_policy", "checks",     "outputs",    "observations",    "preferences", "rationale", "workspace_notes",
    "job_notes",    "checkpoint", "omissions",  "recorded_history"};
struct projection_scan {
  std::array<projection_section_view, 64> sections{};
  std::size_t                             count = 0, text_bytes = 0;
  int                                     failure = 1; // 0 success, 1 malformed protocol, 2 resource bound.
};
// Leaves a raw table/backing on the stack; no metamethod is ever consulted.
int projection_table(lua_State* l, int index, value_kind kind) {
  index = lua_absindex(l, index);
  if (!lua_istable(l, index) || table_kind(l, index) != kind)
    return 0;
  lua_rawgeti(l, LUA_REGISTRYINDEX, backing_index);
  lua_pushvalue(l, index);
  lua_rawget(l, -2);
  lua_remove(l, -2);
  if (lua_istable(l, -1))
    return lua_gettop(l);
  lua_pop(l, 1);
  if (lua_getmetatable(l, index)) {
    lua_pop(l, 1);
    return 0;
  }
  lua_pushvalue(l, index);
  return lua_gettop(l);
}
bool projection_key(lua_State* l, int index, std::string_view expected) {
  if (lua_type(l, index) != LUA_TSTRING)
    return false;
  std::size_t size  = 0;
  const char* bytes = lua_tolstring(l, index, &size);
  return std::string_view(bytes, size) == expected;
}
// Protected POD-only traversal: strings are borrowed from the still-rooted return graph.
int scan_projection(lua_State* l) {
  auto* out = static_cast<projection_scan*>(lua_touserdata(l, 2));
  if (!lua_checkstack(l, 32)) {
    context(l).stack_exhausted = true;
    return 0;
  }
  int root = projection_table(l, 1, value_kind::record);
  if (!root)
    return 0;
  int fields = 0;
  lua_pushnil(l);
  while (lua_next(l, root)) {
    if (projection_key(l, -2, "version")) {
      if (!lua_isinteger(l, -1) || lua_tointeger(l, -1) != 1)
        return 0;
      fields |= 1;
    } else if (projection_key(l, -2, "sections")) {
      fields |= 2;
    } else
      return 0;
    lua_pop(l, 1);
  }
  if (fields != 3)
    return 0;
  lua_pushliteral(l, "sections");
  lua_rawget(l, root);
  int list = projection_table(l, -1, value_kind::list);
  if (!list)
    return 0;
  std::size_t count = 0;
  lua_pushnil(l);
  while (lua_next(l, list)) {
    if (!lua_isinteger(l, -2) || lua_tointeger(l, -2) < 1)
      return 0;
    if (++count > 64) {
      out->failure = 2;
      return 0;
    }
    lua_pop(l, 1);
  }
  for (std::size_t i = 0; i < count; ++i) {
    lua_rawgeti(l, list, static_cast<lua_Integer>(i + 1));
    int section = projection_table(l, -1, value_kind::record);
    if (!section)
      return 0;
    fields = 0;
    lua_pushnil(l);
    while (lua_next(l, section)) {
      if (projection_key(l, -2, "id")) {
        if (lua_type(l, -1) != LUA_TSTRING)
          return 0;
        std::size_t size    = 0;
        const char* bytes   = lua_tolstring(l, -1, &size);
        out->sections[i].id = {bytes, size};
        fields |= 1;
      } else if (projection_key(l, -2, "text")) {
        if (lua_type(l, -1) != LUA_TSTRING)
          return 0;
        std::size_t size  = 0;
        const char* bytes = lua_tolstring(l, -1, &size);
        if (size > 1048576 || size > 16777216 - out->text_bytes) {
          out->failure = 2;
          return 0;
        }
        out->sections[i].text = {bytes, size};
        out->text_bytes += size;
        fields |= 2;
      } else if (projection_key(l, -2, "mandatory")) {
        if (lua_type(l, -1) != LUA_TBOOLEAN)
          return 0;
        out->sections[i].mandatory = lua_toboolean(l, -1);
        fields |= 4;
      } else
        return 0;
      lua_pop(l, 1);
    }
    if (fields != 7)
      return 0;
    auto id = out->sections[i].id;
    if (id.empty() || id.size() > 64 || id.front() < 'a' || id.front() > 'z')
      return 0;
    for (char ch : id)
      if (!((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '_'))
        return 0;
    for (auto reserved : projection_reserved_ids)
      if (reserved == id)
        return 0;
    for (std::size_t j = 0; j < i; ++j)
      if (out->sections[j].id == id)
        return 0;
    if (!valid_text(out->sections[i].text))
      return 0;
    lua_pop(l, 2);
  }
  out->count   = count;
  out->failure = 0;
  return 0;
}
int anchor(lua_State* l) {
  tag(l, 1, table_kind(l, 1));
  return 0;
}
value_ptr convert(lua_State* l, int index, std::size_t depth) {
  auto& s = context(l);
  visit(s, depth);
  index  = lua_absindex(l, index);
  auto v = std::make_shared<value>();
  switch (lua_type(l, index)) {
  case LUA_TBOOLEAN:
    v->kind    = value_kind::boolean;
    v->boolean = lua_toboolean(l, index);
    break;
  case LUA_TNUMBER:
    if (!lua_isinteger(l, index))
      refuse();
    v->kind    = value_kind::integer;
    v->integer = lua_tointeger(l, index);
    if (s.command && (v->integer < -2147483648LL || v->integer > 2147483647LL))
      refuse();
    break;
  case LUA_TSTRING: {
    std::size_t n;
    auto        bytes = lua_tolstring(l, index, &n);
    charge(s, n);
    v->kind = value_kind::string;
    v->string.assign(bytes, n);
    break;
  }
  case LUA_TTABLE: {
    // Private backing lookup is raw/nonallocating; arbitrary metatables still
    // refuse. Keep tags and cycle identity attached to the original object.
    int  original = index;
    bool frozen   = false;
    if (s.command) {
      lua_rawgeti(l, LUA_REGISTRYINDEX, backing_index);
      if (lua_istable(l, -1)) {
        lua_pushvalue(l, original);
        lua_rawget(l, -2);
        lua_remove(l, -2);
        frozen = lua_istable(l, -1);
      }
      if (!frozen)
        lua_pop(l, 1);
    }
    if (!frozen && lua_getmetatable(l, index)) {
      lua_pop(l, 1);
      refuse();
    }
    auto identity = lua_topointer(l, index);
    if (!s.active.insert(identity).second)
      refuse();
    active_visit cleanup{s, identity};
    v->kind = table_kind(l, index);
    lua_pushcfunction(l, anchor);
    lua_pushvalue(l, index);
    auto status = lua_pcall(l, 1, 0, 0);
    // Anchoring runs only native tag operations, with no user handlers or Lua
    // instructions. Still preserve terminal execution classification here.
    if (s.instructions >= instruction_max)
      throw diagnostic{"resource_limit", "Lua instruction limit exceeded", "instructions", instruction_max};
    if (status)
      throw diagnostic{"resource_limit", "Lua allocation exceeded", "lua_bytes", memory_max};
    if (frozen)
      index = lua_gettop(l);
    std::map<lua_Integer, value_ptr> items;
    lua_pushnil(l);
    while (lua_next(l, index)) {
      charge(s, 128);
      auto child = convert(l, -1, depth + 1);
      if (v->kind == value_kind::list) {
        if (!lua_isinteger(l, -2) || lua_tointeger(l, -2) < 1)
          refuse();
        items.emplace(lua_tointeger(l, -2), std::move(child));
      } else {
        if (lua_type(l, -2) != LUA_TSTRING)
          refuse();
        std::size_t n;
        auto        bytes = lua_tolstring(l, -2, &n);
        if (!valid_text({bytes, n}))
          refuse();
        charge(s, n);
        v->fields.emplace(std::string(bytes, n), std::move(child));
      }
      lua_pop(l, 1);
    }
    lua_Integer next = 1;
    for (auto& [key, child] : items) {
      if (key != next++)
        refuse();
      v->items.push_back(std::move(child));
    }
    if (frozen)
      lua_pop(l, 1);
    if (v->kind == value_kind::md || v->kind == value_kind::file || v->kind == value_kind::immaterial) {
      if (v->fields.size() != 1 || !v->fields.contains("value"))
        refuse();
      auto payload = v->fields.at("value");
      if (v->kind == value_kind::immaterial) {
        if (payload->kind != value_kind::md && payload->kind != value_kind::file)
          refuse();
        v->items.push_back(payload);
      } else {
        if (payload->kind != value_kind::string)
          refuse();
        v->string = payload->string;
        charge(s, v->string.size());
      }
      v->fields.clear();
    }
    break;
  }
  default:
    refuse();
  }
  return v;
}
int container(lua_State* l) {
  luaL_checktype(l, 1, LUA_TTABLE);
  if (lua_gettop(l) != 1 || lua_getmetatable(l, 1))
    return luaL_error(l, "invalid_container");
  // Tags are assigned once; accidental private-tag erasure is refused.
  lua_rawgeti(l, LUA_REGISTRYINDEX, tags_index);
  lua_pushvalue(l, 1);
  lua_rawget(l, -2);
  if (!lua_isnil(l, -1))
    return luaL_error(l, "already_typed");
  lua_pop(l, 2);
  tag(l, 1, static_cast<value_kind>(lua_tointeger(l, lua_upvalueindex(1))));
  lua_pushvalue(l, 1);
  return 1;
}
int kind(lua_State* l) {
  const char* name = "unsupported";
  switch (lua_type(l, 1)) {
  case LUA_TBOOLEAN:
    name = "boolean";
    break;
  case LUA_TNUMBER:
    name = lua_isinteger(l, 1) ? "integer" : "unsupported";
    break;
  case LUA_TSTRING:
    name = "string";
    break;
  case LUA_TTABLE:
    name = kind_names[static_cast<int>(table_kind(l, 1))];
    break;
  default:
    break;
  }
  lua_pushstring(l, name);
  return 1;
}
constexpr std::size_t hash_string_alignment = std::max<std::size_t>(__STDCPP_DEFAULT_NEW_ALIGNMENT__, sizeof(void*));
// Pinned libc++ doubles capacity, then rounds allocation including the terminator.
// Successive heap requests at least double; the last is at most 2*N+2*A.
// Their cumulative sum is therefore bounded by 4*N+4*A. Charge incrementally
// through checked charge(), never unchecked multiplication of caller sizes.
void reserve_hash_bytes(state& s, std::size_t bytes) {
  for (int i = 0; i < 4; ++i)
    charge(s, bytes);
}
void reserve_hash_copy(state& s, std::size_t bytes) {
  // Assignment into an empty string may double its initial inline capacity.
  charge(s, bytes);
  charge(s, bytes);
  charge(s, hash_string_alignment);
}
// The existing 512-byte native slots bound graph/control blocks, encoder active
// set nodes plus cumulative buckets, and sorted-map pointers. Encoding work does
// not increment converted-node counters. The source graph is already bounded.
void reserve_hash_payload(state& s, const value_ptr& input) {
  charge(s, 512);
  switch (input->kind) {
  case value_kind::boolean:
    reserve_hash_bytes(s, 1);
    break;
  case value_kind::integer:
    reserve_hash_bytes(s, 9);
    break;
  case value_kind::string:
    reserve_hash_bytes(s, 9);
    reserve_hash_bytes(s, input->string.size());
    break;
  case value_kind::list:
    reserve_hash_bytes(s, 9);
    for (const auto& item : input->items)
      reserve_hash_payload(s, item);
    break;
  case value_kind::record:
    reserve_hash_bytes(s, 9);
    for (const auto& [key, child] : input->fields) {
      charge(s, 512);
      reserve_hash_bytes(s, 9);
      reserve_hash_bytes(s, key.size());
      reserve_hash_payload(s, child);
    }
    break;
  default:
    refuse();
  }
}
void reserve_hash_frame(state& s, std::size_t domain_size) {
  // Four new graph nodes (list/three strings) and four encoder visits. Established
  // slots cover their control blocks, active set and four-item pointer vector.
  for (int i = 0; i < 4; ++i)
    charge(s, 512);
  reserve_hash_copy(s, 6);
  reserve_hash_copy(s, 12);
  reserve_hash_copy(s, domain_size);
  reserve_hash_bytes(s, 9 + (9 + 6) + (9 + 12) + 9);
  reserve_hash_bytes(s, domain_size);
  charge(s, 4 * hash_string_alignment); // One complete canonical growing output.
  // hash.cpp grows the fixed 41-character digest once beyond inline capacity.
  const auto digest_request =
      ((2 * std::string{}.capacity() + hash_string_alignment) / hash_string_alignment) * hash_string_alignment;
  charge(s, digest_request);
}
void digest_native(lua_State* l) {
  auto& s         = context(l);
  auto  operation = lua_tointeger(l, lua_upvalueindex(1));
  if (lua_type(l, 1) != LUA_TSTRING || lua_gettop(l) != (operation == 0 ? 2 : 1))
    refuse();
  std::size_t                            n;
  auto                                   bytes = lua_tolstring(l, 1, &n);
  std::expected<std::string, diagnostic> result;
  if (operation == 0) {
    auto input = convert(l, 2, 1);
    if (s.semantic_active) {
      reserve_hash_payload(s, input);
      reserve_hash_frame(s, n);
    }
    result = hash_value({bytes, n}, input);
  } else {
    if (s.semantic_active) {
      // file_digest/fragment_digest first copy the raw source into one new text
      // graph node, then encode that node beneath the same hash envelope.
      charge(s, 512);
      reserve_hash_copy(s, n);
      charge(s, 512);
      reserve_hash_bytes(s, 9);
      reserve_hash_bytes(s, n);
      reserve_hash_frame(s, operation == 1 ? std::string_view("file_bytes_v1").size() : std::string_view("fragment_v1").size());
    }
    result = operation == 1 ? file_digest({bytes, n}) : fragment_digest({bytes, n});
  }
  if (!result)
    throw std::move(result.error());
  if (s.command && !s.semantic_active)
    charge(s, result->size() + 512);
  s.pending = std::move(*result);
}
int digest(lua_State* l) {
  if (!lua_checkstack(l, 2048))
    return luaL_error(l, "stack_limit");
  bool failed = false;
  try {
    digest_native(l);
  } catch (diagnostic& e) {
    if (context(l).semantic_active && e.code == "resource_limit" && !context(l).conversion_failure)
      context(l).conversion_failure = std::move(e);
    else
      context(l).error = std::move(e);
    failed = true;
  } catch (const std::bad_alloc&) {
    if (context(l).semantic_active)
      context(l).native_memory_failed = true;
    else
      context(l).error = {"internal", "Native hash failed"};
    failed = true;
  } catch (const std::length_error&) {
    if (context(l).semantic_active)
      context(l).native_memory_failed = true;
    else
      context(l).error = {"internal", "Native hash failed"};
    failed = true;
  } catch (...) {
    context(l).error = {"internal", "Native hash failed"};
    failed           = true;
  }
  if (failed) {
    // Opaque, Lua-owned identity and metadata. No native containers grow for
    // caught failures; Lua GC reclaims tokens and the allocator charges them.
    // All allocating Lua calls occur after digest_native temporaries unwind.
    lua_newuserdatauv(l, 0, 1);
    lua_createtable(l, 4, 0);
    lua_pushlstring(l, context(l).error.code.data(), context(l).error.code.size());
    lua_rawseti(l, -2, 1);
    lua_pushlstring(l, context(l).error.message.data(), context(l).error.message.size());
    lua_rawseti(l, -2, 2);
    lua_pushlstring(l, context(l).error.limit.data(), context(l).error.limit.size());
    lua_rawseti(l, -2, 3);
    lua_pushinteger(l, static_cast<lua_Integer>(context(l).error.maximum));
    lua_rawseti(l, -2, 4);
    lua_setiuservalue(l, -2, 1);
    context(l).error = {};
    return lua_error(l);
  }
  lua_pushlstring(l, context(l).pending.data(), context(l).pending.size());
  return 1;
}
// Native codec/conversion ownership stays in state across allocating Lua pushes.
// These callbacks return ordinary error data, never hash diagnostic userdata.
value_ptr codec_text(std::string text) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = std::move(text);
  return v;
}
value_ptr codec_number(std::size_t number) {
  auto v     = std::make_shared<value>();
  v->kind    = value_kind::integer;
  v->integer = static_cast<std::int64_t>(number);
  return v;
}
// Match the pinned libc++ empty-string reserve recipe used by the approved
// decoder. Charge payload and alignment separately so additions cannot overflow.
/// Charge the pinned libc++ reserve-from-empty buffer before any copied text allocation.
void codec_stage_string(state& s, std::size_t size) {
  const auto short_capacity = std::string{}.capacity();
  if (size <= short_capacity)
    return;
  constexpr auto alignment = std::max<std::size_t>(__STDCPP_DEFAULT_NEW_ALIGNMENT__, sizeof(void*));
  charge(s, size);
  const auto padding = alignment - size % alignment;
  charge(s, padding);
  if (size + padding == short_capacity + 2) {
#if defined(_LIBCPP_ABI_ALTERNATE_STRING_LAYOUT) != defined(_LIBCPP_BIG_ENDIAN)
    charge(s, 1);
#else
    charge(s, 2);
#endif
  }
}
/// Charge the exact pinned make_shared control/value block before constructing phase error data.
std::shared_ptr<value> codec_stage_node(state& s, value_kind kind) {
  charge(s, sizeof(std::__shared_ptr_emplace<value, std::allocator<value>>));
  auto result  = std::make_shared<value>();
  result->kind = kind;
  return result;
}
value_ptr codec_stage_text(state& s, std::string_view text) {
  codec_stage_string(s, text.size());
  auto result = codec_stage_node(s, value_kind::string);
  result->string.reserve(text.size());
  result->string.assign(text.data(), text.size());
  return result;
}
/// Charge the exact pinned tree node and key buffer before direct in-place field construction.
void codec_stage_field(state& s, value& object, std::string_view key, value_ptr child) {
  codec_stage_string(s, key.size());
  charge(s, sizeof(std::__tree_node<std::__value_type<std::string, value_ptr>, void*>));
  object.fields.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(std::move(child)));
}
void codec_failure(state& s, const codec_error& error) {
  if (s.semantic_active) {
    // No initializer-list graph or temporary owning strings: every native
    // control block, tree node and copied buffer is charged before construction.
    // Later phase conversion keeps its inherited node/transport accounting.
    auto result = codec_stage_node(s, value_kind::record);
    codec_stage_field(s, *result, "code",
                      codec_stage_text(s, error.detail.code == "syntax" ? std::string_view("schema") : error.detail.code));
    codec_stage_field(s, *result, "message", codec_stage_text(s, error.detail.message));
    auto details = codec_stage_node(s, value_kind::record);
    if (!error.detail.limit.empty()) {
      codec_stage_field(
          s, *details, "limit",
          codec_stage_text(s, error.detail.limit == "input_bytes" ? std::string_view("json_bytes") : error.detail.limit));
    } else {
      codec_stage_field(s, *details, "reason", codec_stage_text(s, error.detail.message));
    }
    auto number     = codec_stage_node(s, value_kind::integer);
    number->integer = static_cast<std::int64_t>(error.detail.limit.empty() ? error.offset + 1 : error.detail.maximum);
    codec_stage_field(s, *details, error.detail.limit.empty() ? "offset" : "maximum", std::move(number));
    codec_stage_field(s, *result, "details", std::move(details));
    s.codec_result = std::move(result);
    return;
  }
  auto result    = std::make_shared<value>();
  result->kind   = value_kind::record;
  result->fields = {{"code", codec_text(error.detail.code == "syntax" ? "schema" : error.detail.code)},
                    {"message", codec_text(error.detail.message)}};
  auto details   = std::make_shared<value>();
  details->kind  = value_kind::record;
  if (!error.detail.limit.empty()) {
    details->fields = {{"limit", codec_text(error.detail.limit == "input_bytes" ? "json_bytes" : error.detail.limit)},
                       {"maximum", codec_number(error.detail.maximum)}};
  } else {
    details->fields = {{"reason", codec_text(error.detail.message)}, {"offset", codec_number(error.offset + 1)}};
  }
  result->fields["details"] = std::move(details);
  s.codec_result            = std::move(result);
}
bool codec_native(lua_State* l) {
  auto& s = context(l);
  s.codec_result.reset();
  if (lua_gettop(l) != 1)
    refuse();
  if (lua_toboolean(l, lua_upvalueindex(1))) {
    auto input  = convert(l, 1, 1);
    auto result = s.semantic_active
                      ? codec_encode_reserved(
                            input, {&s,
                                    [](void* owner, const codec_reservation& capacity) -> std::expected<void, diagnostic> {
                                      try {
                                        charge(*static_cast<state*>(owner), capacity.total_bytes);
                                        return {};
                                      } catch (diagnostic& e) {
                                        return std::unexpected(std::move(e));
                                      }
                                    }})
                      : codec_encode(input);
    if (!result) {
      if (s.semantic_active && result.error().detail.code == "resource_limit") {
        if (!s.conversion_failure)
          s.conversion_failure = std::move(result.error().detail);
        throw *s.conversion_failure;
      }
      codec_failure(s, result.error());
      return false;
    }
    // The encoded buffer was reserved by the encoder and moves without copying.
    // Its native string-value/control wrapper is a separate allocation.
    if (s.semantic_active)
      charge(s, sizeof(std::__shared_ptr_emplace<value, std::allocator<value>>));
    s.codec_result = codec_text(std::move(*result));
    if (s.command)
      validate(s, s.codec_result, 1);
  } else {
    if (lua_type(l, 1) != LUA_TSTRING)
      refuse();
    std::size_t size;
    const auto  bytes = lua_tolstring(l, 1, &size);
    // Parser requests are cumulative even when Glaze abandons an alternative.
    // Immutable conversion is reserved before its first allocation. The later
    // validate pass deliberately retains the inherited graph/node and transport
    // charges: this conservative overlap does not refund codec ownership.
    auto result =
        s.semantic_active
            ? codec_decode_reserved(
                  {bytes, size}, {&s,
                                  [](void* owner, std::size_t bytes) -> std::expected<void, codec_error> {
                                    try {
                                      charge(*static_cast<state*>(owner), bytes);
                                      return {};
                                    } catch (diagnostic& error) {
                                      return std::unexpected(codec_error{std::move(error), 0});
                                    }
                                  },
                                  [](void* owner, const codec_decode_reservation& capacity) -> std::expected<void, codec_error> {
                                    try {
                                      charge(*static_cast<state*>(owner), capacity.total_bytes);
                                      return {};
                                    } catch (diagnostic& error) {
                                      return std::unexpected(codec_error{std::move(error), 0});
                                    }
                                  }})
            : codec_decode({bytes, size});
    if (!result) {
      if (s.semantic_active && result.error().detail.code == "resource_limit") {
        if (!s.conversion_failure)
          s.conversion_failure = std::move(result.error().detail);
        throw *s.conversion_failure;
      }
      codec_failure(s, result.error());
      return false;
    }
    // Preserve the cumulative native conversion budget for repeated calls.
    validate(s, *result, 1);
    s.codec_result = std::move(*result);
  }
  return true;
}
int json_codec(lua_State* l) {
  if (!lua_checkstack(l, 2048))
    return luaL_error(l, "stack_limit");
  bool success = false, allocation_failed = false;
  try {
    success = codec_native(l);
  } catch (const diagnostic& e) {
    try {
      if (context(l).semantic_active) {
        if (!context(l).conversion_failure)
          context(l).conversion_failure = e;
        allocation_failed = true;
      } else
        codec_failure(context(l), {e, 0});
    } catch (...) {
      if (context(l).semantic_active)
        context(l).native_memory_failed = true;
      allocation_failed = true;
    }
  } catch (...) {
    if (context(l).semantic_active)
      context(l).native_memory_failed = true;
    // Do not attempt recursive diagnostic allocation after native exhaustion.
    allocation_failed = true;
  }
  if (allocation_failed)
    return luaL_error(l, "codec_allocation");
  // No native RAII locals remain when Lua allocation can longjmp.
  if (!success)
    lua_pushnil(l);
  push(l, context(l).codec_result.get());
  if (success)
    lua_pushnil(l);
  return 2;
}
void remove(lua_State* l, const char* name) {
  lua_pushnil(l);
  lua_setglobal(l, name);
}
int setup_environment(lua_State* l) {
  if (!lua_checkstack(l, 2048))
    return luaL_error(l, "stack_limit");
  luaL_requiref(l, "_G", luaopen_base, 1);
  lua_pop(l, 1);
  const char* forbidden[] = {"dofile",         "loadfile", "load",         "print", "warn",
                             "collectgarbage", "tostring", "setmetatable", "xpcall"};
  for (auto name : forbidden)
    remove(l, name);
  lua_pushcfunction(l, protected_call);
  lua_setglobal(l, "pcall");
  luaL_requiref(l, "table", luaopen_table, 1);
  // Copy only data/allocation-bounded helpers. Sparse table lengths can drive
  // insert/remove/sort native loops without proportional storage or VM hooks.
  lua_createtable(l, 0, 4);
  for (auto name : {"concat", "create", "pack", "unpack"}) {
    lua_getfield(l, -2, name);
    lua_setfield(l, -2, name);
  }
  lua_setglobal(l, "table");
  lua_pop(l, 1);
  luaL_requiref(l, "string", luaopen_string, 1);
  // Pattern backtracking and empty repetition run native loops without VM
  // hooks. The string metatable indexes this table, closing method access too.
  for (auto name : {"dump", "format", "find", "match", "gmatch", "gsub", "rep"}) {
    lua_pushnil(l);
    lua_setfield(l, -2, name);
  }
  lua_pop(l, 1);
  luaL_requiref(l, "math", luaopen_math, 1);
  lua_pushnil(l);
  lua_setfield(l, -2, "random");
  lua_pushnil(l);
  lua_setfield(l, -2, "randomseed");
  lua_pop(l, 1);
  luaL_requiref(l, "utf8", luaopen_utf8, 1);
  lua_pop(l, 1);
  lua_newtable(l);
  lua_rawseti(l, LUA_REGISTRYINDEX, tags_index);
  lua_newtable(l);
  lua_pushcfunction(l, kind);
  lua_setfield(l, -2, "kind");
  const char* names[] = {"object", "array"};
  for (int i = 0; i < 2; ++i) {
    lua_pushinteger(l, static_cast<int>(i == 0 ? value_kind::record : value_kind::list));
    lua_pushcclosure(l, container, 1);
    lua_setfield(l, -2, names[i]);
  }
  const char* hashes[] = {"hash_value", "file_digest", "fragment_digest"};
  for (int i = 0; i < 3; ++i) {
    lua_pushinteger(l, i);
    lua_pushcclosure(l, digest, 1);
    lua_setfield(l, -2, hashes[i]);
  }
  for (int i = 0; i < 2; ++i) {
    lua_pushboolean(l, i);
    lua_pushcclosure(l, json_codec, 1);
    lua_setfield(l, -2, i ? "json_encode" : "json_decode");
  }
  lua_setglobal(l, "engine");
  return 0;
}
int setup(lua_State* l) {
  setup_environment(l);
  auto source = embedded::source();
  if (luaL_loadbufferx(l, source.data(), source.size(), "trusted", "t"))
    return lua_error(l);
  lua_call(l, 0, 1);
  if (lua_type(l, -1) != LUA_TTABLE)
    return luaL_error(l, "invalid_exports");
  lua_pushlstring(l, context(l).entry.data(), context(l).entry.size());
  lua_rawget(l, -2);
  if (lua_type(l, -1) != LUA_TFUNCTION)
    return luaL_error(l, "unknown_export");
  push(l, context(l).input.get());
  lua_call(l, 1, LUA_MULTRET);
  return lua_gettop(l) - 1; // Discard fixed export table; enforce exactly one return outside.
}
} // namespace
std::expected<value_ptr, diagnostic> run_trusted(std::string_view entry, const value_ptr& input) {
  state s;
  s.input = input;
  s.entry = entry;
  if (entry.size() > 128)
    return std::unexpected(diagnostic{"internal", "Unknown trusted export"});
  try {
    validate(s, input, 1);
  } catch (diagnostic& e) {
    return std::unexpected(std::move(e));
  }
  s.active.clear();
  s.nodes        = 0;
  s.native_bytes = 0;
  auto l         = lua_newstate(allocate, &s, 0);
  if (!l)
    return std::unexpected(diagnostic{"resource_limit", "Lua allocation exceeded", "lua_bytes", memory_max});
  *static_cast<state**>(lua_getextraspace(l)) = &s;
  auto close                                  = std::unique_ptr<lua_State, decltype(&lua_close)>(l, lua_close);
  lua_sethook(l, hook, LUA_MASKCOUNT, 1000);
  lua_pushcfunction(l, setup);
  auto status = lua_pcall(l, 0, LUA_MULTRET, 0);
  // Exhaustion is terminal on successful and failed protected returns alike.
  if (s.instructions >= instruction_max)
    return std::unexpected(diagnostic{"resource_limit", "Lua instruction limit exceeded", "instructions", instruction_max});
  if (status) {
    if (s.lua_memory_failed || status == LUA_ERRMEM)
      return std::unexpected(diagnostic{"resource_limit", "Lua allocation exceeded", "lua_bytes", memory_max});
    // Only the userdata actually escaping carries a native diagnostic. Trusted
    // code has no userdata constructor or access to the private uservalue.
    if (lua_type(l, -1) == LUA_TUSERDATA && lua_getiuservalue(l, -1, 1) == LUA_TTABLE) {
      lua_rawgeti(l, -1, 1);
      auto code = lua_tostring(l, -1);
      lua_rawgeti(l, -2, 2);
      auto message = lua_tostring(l, -1);
      lua_rawgeti(l, -3, 3);
      auto limit = lua_tostring(l, -1);
      lua_rawgeti(l, -4, 4);
      auto maximum = static_cast<std::size_t>(lua_tointeger(l, -1));
      return std::unexpected(diagnostic{code, message, limit, maximum});
    }
    return std::unexpected(diagnostic{"internal", "Trusted export failed"});
  }
  if (lua_gettop(l) != 1)
    return std::unexpected(diagnostic{"internal", "Trusted export must return exactly one value"});
  if (!lua_checkstack(l, 2048))
    return std::unexpected(diagnostic{"resource_limit", "Lua stack exceeded", "stack", 2048});
  try {
    return convert(l, 1, 1);
  } catch (diagnostic& e) {
    return std::unexpected(std::move(e));
  } catch (...) {
    return std::unexpected(diagnostic{"internal", "Native conversion failed"});
  }
}
namespace {
value_ptr command_text(std::string text) {
  return codec_text(std::move(text));
}
value_ptr command_integer(std::int64_t n) {
  auto v     = std::make_shared<value>();
  v->kind    = value_kind::integer;
  v->integer = n;
  return v;
}
value_ptr command_boolean(bool b) {
  auto v     = std::make_shared<value>();
  v->kind    = value_kind::boolean;
  v->boolean = b;
  return v;
}
value_ptr command_record(std::map<std::string, value_ptr> fields) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::record;
  v->fields = std::move(fields);
  return v;
}
value_ptr command_array(std::vector<value_ptr> items) {
  auto v   = std::make_shared<value>();
  v->kind  = value_kind::list;
  v->items = std::move(items);
  return v;
}
[[noreturn]] void command_refuse() {
  throw diagnostic{"internal", "Invalid embedded command protocol"};
}
void exact(const value_ptr& v, std::initializer_list<std::string_view> keys) {
  if (!v || v->kind != value_kind::record || v->fields.size() != keys.size())
    command_refuse();
  for (auto key : keys)
    if (!v->fields.contains(std::string(key)))
      command_refuse();
}
const std::string& text_field(const value_ptr& v, std::string_view key) {
  if (!v || !v->fields.contains(std::string(key)))
    command_refuse();
  auto& f = v->fields.at(std::string(key));
  if (f->kind != value_kind::string)
    command_refuse();
  return f->string;
}
bool same(const value_ptr& a, const value_ptr& b) {
  auto aa = canonical_encode(a), bb = canonical_encode(b);
  return aa && bb && *aa == *bb;
}
value_ptr copy_value(const value_ptr& v) {
  auto c = std::make_shared<value>(*v);
  c->fields.clear();
  c->items.clear();
  for (auto& [key, child] : v->fields)
    c->fields.emplace(key, copy_value(child));
  for (auto& child : v->items)
    c->items.push_back(copy_value(child));
  return c;
}
struct command_entry {
  value_ptr descriptor;
  int       reference = LUA_NOREF;
};
struct external_entry {
  int         descriptor_reference = LUA_NOREF, function_reference = LUA_NOREF;
  std::size_t kind = 0, ordinal = 0;
  value_ptr   descriptor;
};
struct module_fields {
  std::string active, fallback;
};
// Index zero denotes the already-owned module/global boundary; other indices
// are original one-based catalog positions. Selection never allocates, including
// callback/ref-retention, collection, validation remapping and sealing failures.
struct seal_location {
  std::size_t kind = 0, index = 0;
};
// One synchronous original-owner bridge, active only inside execute(). No provider escapes.
struct extension_history_bridge {
  extension_invocation* owner                                      = nullptr;
  const value* (*read)(extension_invocation&)                      = nullptr;
  const value* result                                              = nullptr;
  const value* (*project)(extension_invocation&, std::string_view) = nullptr;
  const value* projection_result                                   = nullptr;
  bool         projection_enabled = false, projection_attempted = false, role_enabled = false;
  const void*  context_identity = nullptr;
  bool         enabled = false, refused = false, resource_refusal = false;
};
struct command_state {
  std::vector<command_entry>        entries;
  profile_snapshot                  profile;
  std::vector<external_entry>       external_entries;
  std::vector<module_fields>        module_locations;
  seal_location                     pending_seal_location;
  std::array<std::size_t, 3>        registration_attempts{5, 0, 0};
  std::size_t                       active_module = 0;
  std::string                       effective_catalog_bytes, effective_digest_bytes, registration_field = "profile";
  value_ptr                         pending_descriptor, catalog, invocation, response, native_error, facts, opened;
  std::optional<diagnostic>         bridge_error;
  std::optional<registration_error> shape_failure;
  std::size_t                       shape_dispatch_count = 0; ///< Successful retained shape F dispatches in profile frames.
  std::array<int, 10>               shape_phases{LUA_NOREF, LUA_NOREF, LUA_NOREF, LUA_NOREF, LUA_NOREF,
                                                 LUA_NOREF, LUA_NOREF, LUA_NOREF, LUA_NOREF, LUA_NOREF};
  int                               shape_function     = LUA_NOREF;
  bool                              invocation_started = false, shape_baseline_ready = false, extension_execution = false;
  std::string                       seal, name;
  command_options                   options;
  std::vector<history::original_file_view> originals;
  std::string_view                         separator, assembly;
  std::vector<history::tree_file>          provenance;
  ledger::completion_receipt               completion;
  command_services*                        services = nullptr;
  extension_history_bridge                 extension_history;
  std::map<std::string, const void*>       handles;
  std::size_t                              calls       = 0;
  int                                      owner_first = 0, owner_last = 0;
  bool                                     registering = true, invoked = false, export_published = false;
  const void* profile_frame_owner = nullptr; ///< Identity of the profile_invocation that began the frame.
};
command_state& commands(lua_State* l) {
  return *context(l).command;
}
bool exhausted(const state& s) {
  return s.exhausted || s.lua_memory_failed || s.stack_exhausted || s.native_memory_failed || s.conversion_failure.has_value() ||
         s.instructions >= s.instruction_limit;
}
diagnostic command_resource(const state& s) {
  if (s.conversion_failure)
    return *s.conversion_failure;
  if (s.native_memory_failed)
    return {"resource_limit", "External profile resource limit exceeded"};
  if (s.stack_exhausted)
    return {"resource_limit", "Lua stack exceeded", "stack", 2048};
  if (s.instructions >= s.instruction_limit)
    return {"resource_limit", "Lua instruction limit exceeded", "instructions", s.instruction_limit};
  return {"resource_limit", "Lua allocation exceeded", "lua_bytes", s.memory_limit};
}
void command_failure(lua_State* l, diagnostic e) {
  // A failure raised after a resource ceiling was reached is not the first terminal failure.
  if (!commands(l).bridge_error && !exhausted(context(l)))
    commands(l).bridge_error = std::move(e);
}
// All C++ owners unwind before callbacks allocate Lua objects or raise errors.
void collect_descriptor(lua_State* l) {
  auto& c = commands(l);
  if (!c.registering || lua_gettop(l) != 2 || lua_type(l, 2) != LUA_TFUNCTION || c.entries.size() >= 64)
    command_refuse();
  lua_Debug caller{}, function{};
  if (!lua_getstack(l, 1, &caller) || !lua_getinfo(l, "Sl", &caller))
    command_refuse();
  lua_pushvalue(l, 2);
  lua_getinfo(l, ">S", &function);
  if (std::string_view(caller.source) != "trusted" || std::string_view(function.source) != "trusted" ||
      caller.currentline < c.owner_first || caller.currentline > c.owner_last || function.linedefined < c.owner_first ||
      function.lastlinedefined > c.owner_last || function.linedefined == 0)
    command_refuse();
  c.pending_descriptor = convert(l, 1, 1);
}
int register_command(lua_State* l) {
  bool failed = false;
  try {
    collect_descriptor(l);
  } catch (diagnostic& e) {
    command_failure(l, std::move(e));
    failed = true;
  } catch (...) {
    command_failure(l, {"internal", "Command registration failed"});
    failed = true;
  }
  if (failed)
    return luaL_error(l, "command_registration");
  lua_pushvalue(l, 2);
  int reference = luaL_ref(l, LUA_REGISTRYINDEX);
  try {
    commands(l).entries.push_back({std::move(commands(l).pending_descriptor), reference});
  } catch (...) {
    command_failure(l, {"internal", "Command registration failed"});
    failed = true;
  }
  if (failed)
    return luaL_error(l, "command_registration");
  return 0;
}
diagnostic external_protocol() {
  return {"internal", "Invalid external profile protocol"};
}
diagnostic external_resource() {
  return {"resource_limit", "External profile resource limit exceeded"};
}
int register_external(lua_State* l) {
  bool failed = false;
  try {
    auto& c    = commands(l);
    auto  kind = static_cast<std::size_t>(lua_tointeger(l, lua_upvalueindex(1)));
    // A caught callback refusal (including a Lua reference-allocation longjmp)
    // must retain its first location before any later callback can select one.
    if (c.bridge_error || exhausted(context(l)))
      throw external_protocol();
    c.pending_seal_location = {};
    if (!c.profile || !c.registering || !c.active_module || lua_gettop(l) != 2 || lua_type(l, 1) != LUA_TTABLE ||
        lua_type(l, 2) != LUA_TFUNCTION)
      throw external_protocol();
    c.pending_seal_location = {kind, c.registration_attempts[kind] + 1};
    // Count attempts before retaining references, including errors caught by Lua.
    if (c.registration_attempts[kind] >= 64)
      throw external_resource();
    ++c.registration_attempts[kind];
    charge(context(l), 2 * sizeof(external_entry));
    c.external_entries.push_back({LUA_NOREF, LUA_NOREF, kind, c.active_module, {}});
  } catch (diagnostic& e) {
    command_failure(l, std::move(e));
    failed = true;
  } catch (const std::bad_alloc&) {
    command_failure(l, external_resource());
    failed = true;
  } catch (...) {
    command_failure(l, external_protocol());
    failed = true;
  }
  if (failed)
    return luaL_error(l, "external_registration");
  // Vector ownership is installed before Lua allocation can longjmp.
  lua_pushvalue(l, 1);
  commands(l).external_entries.back().descriptor_reference = luaL_ref(l, LUA_REGISTRYINDEX);
  lua_pushvalue(l, 2);
  commands(l).external_entries.back().function_reference = luaL_ref(l, LUA_REGISTRYINDEX);
  commands(l).pending_seal_location                      = {};
  return 0;
}
struct signature_option {
  std::string_view name, type;
  bool             required;
};
std::vector<signature_option> command_signature(std::string_view name) {
  if (name == "render")
    return {{"--workspace", "text", true},
            {"--registry", "text", true},
            {"--root", "text", true},
            {"--job", "text", true},
            {"--role", "text", true},
            {"--base-root", "text", false},
            {"--observations", "text", false},
            {"--budget-bytes", "decimal", false},
            {"--capture-outputs", "flag", false}};
  if (name == "init")
    return {{"--ledger", "text", true}, {"--workspace-id", "text", true}};
  if (name == "history")
    return {{"--ledger", "text", true},       {"--request-id", "text", false},     {"--revision", "decimal", false},
            {"--at-event", "decimal", false}, {"--after-event", "decimal", false}, {"--limit", "decimal", false}};
  if (name == "export")
    return {{"--ledger", "text", true}, {"--target", "text", true}, {"--revision", "decimal", true}};
  if (name == "load" && command_catalog_size == 5)
    return {{"--ledger", "text", true},          {"--workspace", "text", true},  {"--registry", "text", true},
            {"--expect-revision", "text", true}, {"--request-id", "text", true}, {"--actor", "text", true}};
  command_refuse();
}
void help_text(const std::string& text) {
  if (text.size() > 4096 || !valid_text(text))
    command_refuse();
  for (unsigned char ch : text)
    if ((ch < 32 && ch != '\n' && ch != '\t') || ch == 127)
      command_refuse();
}
void seal_catalog(state& s) {
  auto& c = *s.command;
  if (c.entries.size() != command_catalog_size || c.bridge_error)
    command_refuse();
  std::set<std::string>                    names;
  std::vector<value_ptr>                   descriptors, bindings;
  const std::map<std::string, std::string> capabilities{{"render", "render_capture_v1"},
                                                        {"init", "initialize_ledger_v1"},
                                                        {"history", "read_history_v1"},
                                                        {"export", "export_source_v1"},
                                                        {"load", "accept_load_v1"}};
  for (auto& entry : c.entries) {
    auto d = entry.descriptor;
    exact(d, {"name", "family", "handler_id", "options", "help", "response_kind", "capabilities"});
    auto name      = text_field(d, "name");
    auto signature = command_signature(name);
    if (!names.insert(name).second || text_field(d, "family") != name ||
        text_field(d, "handler_id") != "shipped." + name + ".v1" || text_field(d, "response_kind") != name + "_v1")
      command_refuse();
    help_text(text_field(d, "help"));
    auto caps = d->fields.at("capabilities"), options = d->fields.at("options");
    if (caps->kind != value_kind::list || caps->items.size() != 1 || caps->items[0]->kind != value_kind::string ||
        caps->items[0]->string != capabilities.at(name) || options->kind != value_kind::list ||
        options->items.size() != signature.size())
      command_refuse();
    std::set<std::string>  option_names;
    std::vector<value_ptr> sorted_options;
    for (auto& option : options->items) {
      exact(option, {"name", "type", "required", "help"});
      auto option_name = text_field(option, "name");
      auto it          = std::ranges::find(signature, option_name, &signature_option::name);
      if (it == signature.end() || !option_names.insert(option_name).second || text_field(option, "type") != it->type ||
          option->fields.at("required")->kind != value_kind::boolean || option->fields.at("required")->boolean != it->required)
        command_refuse();
      help_text(text_field(option, "help"));
      sorted_options.push_back(option);
    }
    std::ranges::sort(sorted_options, {}, [](auto& option) { return text_field(option, "name"); });
    auto normalized               = std::make_shared<value>(*d);
    normalized->fields["options"] = command_array(std::move(sorted_options));
    entry.descriptor              = normalized;
    descriptors.push_back(normalized);
    bindings.push_back(
        command_record({{"handler_id", d->fields.at("handler_id")}, {"module", command_text("planar/commands.lua")}}));
  }
  std::ranges::sort(c.entries, {}, [](auto& e) { return text_field(e.descriptor, "name"); });
  std::ranges::sort(descriptors, {}, [](auto& d) { return text_field(d, "name"); });
  std::ranges::sort(bindings, {}, [](auto& d) { return text_field(d, "handler_id"); });
  c.catalog    = command_record({{"version", command_integer(1)}, {"commands", command_array(std::move(descriptors))}});
  auto encoded = canonical_encode(c.catalog, {.bytes = 1048576});
  if (!encoded)
    throw encoded.error();
  // Seal the exact assembled bundle plus original boundaries, not a later global lookup.
  std::vector<value_ptr> originals;
  for (auto& f : embedded::originals())
    originals.push_back(
        command_record({{"path", command_text(std::string(f.path))}, {"bytes", command_text(std::string(f.bytes))}}));
  // Borrow only immutable bytes from this VM's imported compiled bundle. Acceptance
  // provenance validation/ownership must wait until a fresh load reaches accept.
  auto embedded_originals = embedded::originals();
  charge(s, embedded_originals.size() * sizeof(history::original_file_view));
  c.originals.reserve(embedded_originals.size());
  for (auto& f : embedded_originals)
    c.originals.push_back({f.path, f.bytes});
  c.separator = embedded::separator();
  c.assembly  = embedded::source();
  auto digest = hash_value("command_bundle_v1", command_record({{"originals", command_array(std::move(originals))},
                                                                {"separator", command_text(std::string(embedded::separator()))},
                                                                {"source", command_text(std::string(embedded::source()))}}));
  if (!digest)
    throw digest.error();
  auto sealed = canonical_encode(command_record(
      {{"catalog", c.catalog}, {"bindings", command_array(std::move(bindings))}, {"bundle_digest", command_text(*digest)}}));
  if (!sealed)
    throw sealed.error();
  c.seal = std::move(*sealed);
}
int frozen_assignment(lua_State* l) {
  return luaL_error(l, "sealed_table");
}
// A private identity-to-backing map is installed before loading trusted Lua, so
// captured read aliases observe the same tables before and after sealing.
void read_backing(lua_State* l) {
  luaL_checktype(l, 1, LUA_TTABLE);
  lua_pushvalue(l, 1);
  lua_rawget(l, lua_upvalueindex(1));
  if (lua_isnil(l, -1)) {
    lua_pop(l, 1);
    lua_pushvalue(l, 1);
  }
}
// Record the cause before raising into Lua, so a caught protocol failure cannot
// be displaced by a later allocation/instruction failure in the handler.
void refuse_extension_service(lua_State* l) {
  auto& bridge = commands(l).extension_history;
  if (!bridge.refused) {
    bridge.resource_refusal = exhausted(context(l));
    bridge.refused          = true;
  }
}
// Only the exact live invocation context has missing-service semantics. Its
// proxy remains strongly rooted in the input stack and registry backing map
// until execute cleanup clears the bridge before releasing those roots.
void refuse_missing_extension_service(lua_State* l) {
  auto* c = context(l).command;
  if (c && c->extension_history.context_identity && lua_topointer(l, 1) == c->extension_history.context_identity &&
      lua_isnil(l, -1) &&
      (projection_key(l, 2, "read_history") || projection_key(l, 2, "read_role") || projection_key(l, 2, "project")))
    refuse_extension_service(l);
}
int frozen_rawget(lua_State* l) {
  luaL_checkany(l, 2);
  lua_settop(l, 2);
  read_backing(l);
  lua_pushvalue(l, 2);
  lua_rawget(l, -2);
  refuse_missing_extension_service(l);
  return 1;
}
int frozen_next(lua_State* l) {
  lua_settop(l, 2);
  read_backing(l);
  lua_pushvalue(l, 2);
  if (lua_next(l, -2))
    return 2;
  lua_pushnil(l);
  return 1;
}
int frozen_pairs(lua_State* l) {
  luaL_checktype(l, 1, LUA_TTABLE);
  lua_pushvalue(l, lua_upvalueindex(1));
  lua_pushvalue(l, 1);
  lua_pushnil(l);
  return 3;
}
int frozen_length(lua_State* l) {
  lua_pushinteger(l, static_cast<lua_Integer>(lua_rawlen(l, lua_upvalueindex(1))));
  return 1;
}
void freeze_stack(lua_State* l, int extra) {
  if (l->top.p - l->stack.p > 2048 - extra || !lua_checkstack(l, extra)) {
    context(l).stack_exhausted = true;
    luaL_error(l, "stack_limit");
  }
}
// Charge graph edges and frame storage to the same cumulative conversion budget.
// The diagnostic owner lives in state, outside the protected allocating frame.
void freeze_visit(lua_State* l, std::size_t depth) {
  bool failed = false;
  try {
    visit(context(l), depth);
  } catch (diagnostic&) {
    failed = true;
  }
  if (failed)
    luaL_error(l, "seal_limit");
}
struct freeze_frame {
  int object, phase, upvalue;
};
// Only trivial fixed storage survives Lua longjmp. At most 256 active objects,
// four retained Lua slots per table and twelve checked temporary slots are used.
// Memoize both tables and functions before following keys, values or upvalues.
void freeze_graph(lua_State* l, int root, int memo, int backings, int sealed) {
  freeze_frame frames[depth_max];
  int          count      = 0;
  int          parent_top = lua_gettop(l);
  freeze_stack(l, 12);
  lua_pushvalue(l, root);
  bool enter = true;
  for (;;) {
    freeze_stack(l, 12);
    if (enter) {
      int type = lua_type(l, -1);
      // Every edge is work, including aliases and primitive leaves.
      freeze_visit(l, 1);
      if (type != LUA_TTABLE && (type != LUA_TFUNCTION || lua_iscfunction(l, -1))) {
        lua_pop(l, 1);
      } else {
        lua_pushvalue(l, -1);
        lua_rawget(l, memo);
        bool seen = !lua_isnil(l, -1);
        lua_pop(l, 1);
        lua_pushvalue(l, -1);
        lua_rawget(l, sealed);
        seen = seen || !lua_isnil(l, -1);
        lua_pop(l, 1);
        if (seen) {
          lua_pop(l, 1);
        } else {
          if (count == static_cast<int>(depth_max)) {
            freeze_visit(l, depth_max + 1);
            return;
          }
          lua_pushvalue(l, -1);
          lua_pushboolean(l, true);
          lua_rawset(l, memo);
          frames[count++] = {lua_gettop(l), 0, 0};
          if (type == LUA_TTABLE) {
            lua_newtable(l);
            lua_pushvalue(l, -2);
            lua_pushvalue(l, -2);
            lua_rawset(l, backings);
            lua_pushnil(l);
          }
        }
      }
      enter = false;
    }
    if (!count)
      break;
    auto& frame  = frames[count - 1];
    int   object = frame.object;
    if (lua_type(l, object) == LUA_TFUNCTION) {
      if (lua_getupvalue(l, object, ++frame.upvalue)) {
        enter = true;
        continue;
      }
    } else {
      if (frame.phase == 0) {
        if (lua_next(l, object)) {
          frame.phase = 1;
          lua_pushvalue(l, -2); // Follow a key, including function/table keys.
          enter = true;
          continue;
        }
      } else if (frame.phase == 1) {
        frame.phase = 2;
        lua_pushvalue(l, -1);
        enter = true;
        continue;
      } else {
        lua_pushvalue(l, -2);
        lua_pushvalue(l, -2);
        lua_rawset(l, object + 1);
        lua_pop(l, 1);
        frame.phase = 0;
        continue;
      }
      // Children are complete. Empty the original in place, preserving identity.
      lua_pushnil(l);
      while (lua_next(l, object)) {
        freeze_visit(l, 1);
        lua_pop(l, 1);
        lua_pushvalue(l, -1);
        lua_pushnil(l);
        lua_rawset(l, object);
      }
      lua_newtable(l);
      lua_pushvalue(l, object + 1);
      lua_setfield(l, -2, "__index");
      lua_pushcfunction(l, frozen_assignment);
      lua_setfield(l, -2, "__newindex");
      lua_pushvalue(l, object + 1);
      lua_pushcclosure(l, frozen_length, 1);
      lua_setfield(l, -2, "__len");
      lua_pushboolean(l, false);
      lua_setfield(l, -2, "__metatable");
      lua_setmetatable(l, object);
    }
    lua_settop(l, object - 1);
    --count;
  }
  lua_settop(l, parent_top);
}
// Module sealing maps root pairs to the same original registration token used
// by callbacks and collection. Selection never allocates or changes owned text.
bool module_seal_location(lua_State* l, std::optional<std::size_t> root) {
  auto& c = commands(l);
  if (!root) {
    c.pending_seal_location = {};
    return true;
  }
  std::array<std::size_t, 3> indices{command_catalog_size, 0, 0};
  std::size_t                pair = 0;
  for (auto& e : c.external_entries) {
    ++indices[e.kind];
    if (e.ordinal != c.active_module)
      continue;
    if (pair++ != *root / 2)
      continue;
    c.pending_seal_location = {e.kind, indices[e.kind]};
    return true;
  }
  command_failure(l, {"internal", "Invalid external profile protocol"});
  return false;
}
// The module fallback is already installed. Failed formatting leaves it intact;
// successful formatting publishes once by nonallocating swap, outside Lua.
void render_seal_location(command_state& c) {
  auto location = c.pending_seal_location;
  if (!location.index)
    return;
  auto field = std::format("catalog.{}[{}]",
                           location.kind == 0   ? "commands"
                           : location.kind == 1 ? "shapes"
                                                : "projections",
                           location.index);
  c.registration_field.swap(field);
}
// Audit raw roots before the freezer installs proxy functions. All native owners
// unwind here before the protected caller may allocate or raise a Lua error.
bool audit_command_roots(lua_State* l, std::span<const int> roots, bool module_roots) {
  try {
    closure_memory memory(context(l));
    auto           result = closure_guard::audit_roots_detailed(l, roots, native_catalog_index, memory, sealed_index);
    if (!result) {
      if (module_roots && !module_seal_location(l, result.error().root))
        return false;
      if (result.error().code == closure_guard::failure::depth)
        throw diagnostic{"resource_limit", "Conversion depth exceeded", "depth", depth_max};
      if (result.error().code == closure_guard::failure::resource)
        throw diagnostic{"resource_limit", "External profile resource limit exceeded"};
      command_refuse();
    }
    // Inspection work is metadata, never VM instructions or converted nodes.
    for (auto member : {&closure_guard::report::edges, &closure_guard::report::prototypes, &closure_guard::report::instructions,
                        &closure_guard::report::lua_closures, &closure_guard::report::cells, &closure_guard::report::loadkx,
                        &closure_guard::report::newtable, &closure_guard::report::setlist_extra}) {
      auto& total  = context(l).seal_inspections.*member;
      auto  amount = (*result).*member;
      if (amount > std::numeric_limits<std::size_t>::max() - total)
        throw diagnostic{"resource_limit", "External profile resource limit exceeded"};
      total += amount;
    }
    return true;
  } catch (diagnostic& error) {
    command_failure(l, std::move(error));
  } catch (const std::bad_alloc&) {
    command_failure(l, {"resource_limit", "External profile resource limit exceeded"});
  }
  return false;
}
// Roots are native-owned absolute indices, not a synthetic graph container.
// This protected frame owns only trivial storage across allocating Lua calls.
int seal_roots(lua_State* l, std::span<const int> roots, bool cleanup_registration, bool module_roots = false) {
  if (context(l).seal_failed)
    return luaL_error(l, "seal_owner_failed");
  // Any unwind/longjmp leaves the owner terminal, including partially frozen graphs.
  context(l).seal_failed = true;
  if (!lua_checkstack(l, closure_guard::audit_stack_slots)) {
    context(l).stack_exhausted = true;
    return 0;
  }
  if (!audit_command_roots(l, roots, module_roots))
    return 0;
  freeze_stack(l, 12);
  lua_newtable(l);
  int memo = lua_gettop(l);
  lua_rawgeti(l, LUA_REGISTRYINDEX, backing_index);
  int backings = lua_gettop(l);
  lua_rawgeti(l, LUA_REGISTRYINDEX, sealed_index);
  int sealed = lua_gettop(l);
  if (cleanup_registration) {
    lua_getglobal(l, "registration");
    freeze_graph(l, lua_gettop(l), memo, backings, sealed);
    lua_pop(l, 1);
    remove(l, "registration");
  }
  for (std::size_t i = 0; i < roots.size(); ++i) {
    if (module_roots && !module_seal_location(l, i))
      return 0;
    freeze_graph(l, roots[i], memo, backings, sealed);
  }
  if (module_roots && !module_seal_location(l, std::nullopt))
    return 0;
  // Assemble the union privately. Publication is one nonallocating replacement
  // of an existing registry slot; a failed union leaves the prior set unchanged.
  lua_pushnil(l);
  while (lua_next(l, sealed)) {
    freeze_visit(l, 1);
    lua_pushvalue(l, -2);
    lua_pushboolean(l, true);
    lua_rawset(l, memo);
    lua_pop(l, 1);
  }
  lua_pushvalue(l, memo);
  lua_rawseti(l, LUA_REGISTRYINDEX, sealed_index);
  context(l).seal_failed = false;
  lua_settop(l, 0);
  return 0;
}
// Only trivial locals survive Lua allocation/longjmp. Native owners and the
// complete B2 transaction unwind inside read() before any Lua API is used.
int extension_read_history(lua_State* l) {
  auto& bridge = commands(l).extension_history;
  bool  failed = bridge.refused || !bridge.enabled || !bridge.owner || !bridge.read || bridge.result || exhausted(context(l)) ||
                 commands(l).bridge_error;
  if (!failed) {
    failed = lua_gettop(l) != 1 || lua_type(l, 1) != LUA_TTABLE || lua_getmetatable(l, 1);
    if (!failed) {
      lua_pushnil(l);
      failed = lua_next(l, 1) != 0;
    }
  }
  if (!failed) {
    try {
      bridge.result = bridge.read(*bridge.owner);
      failed        = !bridge.result;
    } catch (const std::bad_alloc&) {
      context(l).native_memory_failed = true;
      failed                          = true;
    } catch (const std::length_error&) {
      context(l).native_memory_failed = true;
      failed                          = true;
    } catch (diagnostic& error) {
      if (error.code == "resource_limit")
        context(l).native_memory_failed = true;
      failed = true;
    } catch (...) {
      failed = true;
    }
  }
  if (failed) {
    refuse_extension_service(l);
    return luaL_error(l, "extension_service");
  }
  lua_settop(l, 0);
  push(l, bridge.result);
  lua_newtable(l);
  lua_rawgeti(l, LUA_REGISTRYINDEX, backing_index);
  lua_rawgeti(l, LUA_REGISTRYINDEX, sealed_index);
  freeze_graph(l, 1, 2, 3, 4);
  lua_settop(l, 1);
  return 1;
}
// Native owners unwind in the callback before Lua result allocation or raising.
int extension_project(lua_State* l) {
  auto& b      = commands(l).extension_history;
  bool  failed = b.refused || !b.enabled || !b.projection_enabled || !b.owner || !b.project || !b.result ||
                 b.projection_attempted || exhausted(context(l)) || commands(l).bridge_error;
  if (!failed) {
    b.projection_attempted = true;
    failed                 = lua_gettop(l) != 1 || lua_type(l, 1) != LUA_TTABLE || lua_getmetatable(l, 1);
    if (!failed) {
      lua_pushnil(l);
      failed = lua_next(l, 1) == 0;
      if (!failed) {
        failed = !projection_key(l, -2, "id") || lua_type(l, -1) != LUA_TSTRING;
        lua_pop(l, 1);
        if (!failed)
          failed = lua_next(l, 1) != 0;
      }
    }
  }
  if (!failed) {
    lua_getfield(l, 1, "id");
    std::size_t size = 0;
    const char* id   = lua_tolstring(l, -1, &size);
    try {
      b.projection_result = b.project(*b.owner, std::string_view(id, size));
      failed              = !b.projection_result;
    } catch (const std::bad_alloc&) {
      context(l).native_memory_failed = true;
      failed                          = true;
    } catch (const std::length_error&) {
      context(l).native_memory_failed = true;
      failed                          = true;
    } catch (diagnostic& error) {
      command_failure(l, std::move(error));
      failed = true;
    } catch (...) {
      failed = true;
    }
  }
  if (failed) {
    refuse_extension_service(l);
    return luaL_error(l, "extension_service");
  }
  lua_settop(l, 0);
  push(l, b.projection_result);
  lua_newtable(l);
  lua_rawgeti(l, LUA_REGISTRYINDEX, backing_index);
  lua_rawgeti(l, LUA_REGISTRYINDEX, sealed_index);
  freeze_graph(l, 1, 2, 3, 4);
  lua_settop(l, 1);
  return 1;
}
int extension_context_index(lua_State* l) {
  lua_pushvalue(l, 2);
  lua_rawget(l, lua_upvalueindex(1));
  refuse_missing_extension_service(l);
  return 1;
}
int extension_opaque_string(lua_State* l) {
  return luaL_error(l, "opaque_slot");
}
// Records the registration-time registry membership once; later release restores exactly this set.
void ensure_shape_baseline(lua_State* l) {
  if (!commands(l).shape_baseline_ready) {
    for (int slot : {tags_index, backing_index}) {
      lua_newtable(l);
      int retained = lua_gettop(l);
      lua_rawgeti(l, LUA_REGISTRYINDEX, slot);
      lua_pushnil(l);
      while (lua_next(l, -2)) {
        freeze_visit(l, 1);
        lua_pop(l, 1);
        lua_pushvalue(l, -1);
        lua_pushboolean(l, true);
        lua_rawset(l, retained);
      }
      lua_pop(l, 1);
      lua_rawsetp(l, LUA_REGISTRYINDEX, slot == tags_index ? &shape_tags_key : &shape_backings_key);
    }
    commands(l).shape_baseline_ready = true;
  }
}
// Protected-call form of ensure_shape_baseline: freezing may raise.
int ensure_shape_baseline_protected(lua_State* l) {
  if (!lua_checkstack(l, closure_guard::audit_stack_slots)) {
    context(l).stack_exhausted = true;
    return 0;
  }
  ensure_shape_baseline(l);
  return 0;
}
// Temporary input proxies are never published into sealed_index. Their backing
// entries remain available until raw response conversion has copied every alias.
int execute_shape(lua_State* l) {
  if (!lua_checkstack(l, closure_guard::audit_stack_slots)) {
    context(l).stack_exhausted = true;
    return 0;
  }
  ensure_shape_baseline(l);
  lua_newtable(l);
  context(l).push_memo          = lua_gettop(l);
  context(l).preserving_aliases = true;
  push(l, context(l).input.get());
  context(l).preserving_aliases = false;
  int input                     = lua_gettop(l);
  if (commands(l).extension_execution && commands(l).extension_history.enabled) {
    lua_getfield(l, input, "invocation");
    lua_getfield(l, -1, "options");
    for (const char* key : {"ledger", "workspace", "registry", "root", "base-root", "observations"}) {
      lua_getfield(l, -1, key);
      bool supplied = lua_isboolean(l, -1) && lua_toboolean(l, -1);
      lua_pop(l, 1);
      if (!supplied)
        continue;
      lua_newuserdatauv(l, 0, 0);
      lua_newtable(l);
      lua_pushcfunction(l, extension_opaque_string);
      lua_setfield(l, -2, "__tostring");
      lua_pushboolean(l, false);
      lua_setfield(l, -2, "__metatable");
      lua_setmetatable(l, -2);
      lua_setfield(l, -2, key);
    }
    lua_pop(l, 2);
  }
  lua_newtable(l);
  int memo = lua_gettop(l);
  lua_rawgeti(l, LUA_REGISTRYINDEX, backing_index);
  int backings = lua_gettop(l);
  lua_rawgeti(l, LUA_REGISTRYINDEX, sealed_index);
  freeze_graph(l, input, memo, backings, lua_gettop(l));
  lua_settop(l, input);
  lua_rawgeti(l, LUA_REGISTRYINDEX, commands(l).shape_function);
  if (commands(l).extension_execution) {
    lua_getfield(l, input, "invocation");
    lua_getfield(l, input, "context");
    // Read methods and iteration share the exact registry backing, including
    // empty contexts whose metatable index need not identify that backing.
    commands(l).extension_history.context_identity = lua_topointer(l, -1);
    lua_getmetatable(l, -1);
    lua_rawgeti(l, LUA_REGISTRYINDEX, backing_index);
    lua_pushvalue(l, -3);
    lua_rawget(l, -2);
    lua_remove(l, -2);
    if (commands(l).extension_history.enabled) {
      lua_pushcfunction(l, extension_read_history);
      lua_setfield(l, -2, commands(l).extension_history.role_enabled ? "read_role" : "read_history");
      if (commands(l).extension_history.projection_enabled) {
        lua_pushcfunction(l, extension_project);
        lua_setfield(l, -2, "project");
      }
    }
    lua_pushcclosure(l, extension_context_index, 1);
    lua_setfield(l, -2, "__index");
    lua_pop(l, 1);
    lua_call(l, 2, LUA_MULTRET);
  } else {
    lua_pushvalue(l, input);
    lua_call(l, 1, LUA_MULTRET);
  }
  return lua_gettop(l) - input;
}
// Raw deletion never allocates and preserves the exact pre-invocation registry membership,
// including registration bookkeeping that is not itself a sealed graph root.
// It also removes engine-tagged invocation scratch and result tables.
void release_shape_temporaries(lua_State* l) {
  lua_settop(l, 0);
  if (!commands(l).shape_baseline_ready)
    return;
  for (int slot : {tags_index, backing_index}) {
    lua_rawgetp(l, LUA_REGISTRYINDEX, slot == tags_index ? &shape_tags_key : &shape_backings_key);
    lua_rawgeti(l, LUA_REGISTRYINDEX, slot);
    lua_pushnil(l);
    while (lua_next(l, 2)) {
      lua_pop(l, 1);
      lua_pushvalue(l, -1);
      lua_rawget(l, 1);
      bool retained = !lua_isnil(l, -1);
      lua_pop(l, 1);
      if (!retained) {
        lua_pushvalue(l, -1);
        lua_pushnil(l);
        lua_rawset(l, 2);
      }
    }
    lua_pop(l, 2);
  }
  lua_settop(l, 0);
}
// Observation-only registry membership; raw traversal, no allocation, stack restored.
struct registry_membership {
  std::size_t tags = 0, backings = 0, temporaries = 0;
};
registry_membership measure_registry(lua_State* l) noexcept {
  registry_membership out;
  const int           top = lua_gettop(l);
  if (!lua_checkstack(l, 8))
    return out;
  for (int slot : {tags_index, backing_index}) {
    lua_rawgetp(l, LUA_REGISTRYINDEX, slot == tags_index ? &shape_tags_key : &shape_backings_key);
    const int baseline = lua_gettop(l);
    lua_rawgeti(l, LUA_REGISTRYINDEX, slot);
    const int table = lua_gettop(l);
    lua_pushnil(l);
    while (lua_next(l, table)) {
      lua_pop(l, 1);
      (slot == tags_index ? out.tags : out.backings)++;
      if (!lua_isnil(l, baseline)) {
        lua_pushvalue(l, -1);
        lua_rawget(l, baseline);
        if (lua_isnil(l, -1))
          out.temporaries++;
        lua_pop(l, 1);
      }
    }
    lua_settop(l, baseline - 1);
  }
  lua_settop(l, top);
  return out;
}
// A synchronous call frame borrows the same sealed VM and cumulative owner. Only
// its protected trampoline allocates Lua input/proxies or invokes retained Lua.
// Stack results stay rooted through raw admission, counting and owned conversion;
// temporary registry roots belong to the enclosing invocation cleanup.
struct projection_call_frame {
  lua_State*     lua;
  state&         runtime;
  command_state& command;
  value_ptr      input;
  int            top, memo, function;
  bool           aliases, extension;
  projection_call_frame(lua_State* l, const value_ptr& next, int retained)
      : lua(l), runtime(context(l)), command(commands(l)), input(std::move(runtime.input)), top(lua_gettop(l)),
        memo(runtime.push_memo), function(command.shape_function), aliases(runtime.preserving_aliases),
        extension(command.extension_execution) {
    runtime.input               = next;
    runtime.push_memo           = 0;
    runtime.preserving_aliases  = false;
    command.shape_function      = retained;
    command.extension_execution = false;
  }
  ~projection_call_frame() {
    lua_settop(lua, top);
    runtime.input               = std::move(input);
    runtime.push_memo           = memo;
    runtime.preserving_aliases  = aliases;
    command.shape_function      = function;
    command.extension_execution = extension;
  }
};
std::expected<value_ptr, diagnostic> call_retained_projection(lua_State* l, int retained, const value_ptr& input) {
  auto& s = context(l);
  auto& c = commands(l);
  // Fixed native stack bookkeeping is charged before use, just like scanner
  // storage. No invocation transition, counters or registry baseline is reset.
  charge(s, sizeof(projection_call_frame) + sizeof(projection_scan));
  projection_call_frame restore(l, input, retained);
  const int             base = lua_gettop(l);
  if (!lua_checkstack(l, closure_guard::audit_stack_slots)) {
    s.stack_exhausted = true;
    return std::unexpected(command_resource(s));
  }
  lua_pushcfunction(l, execute_shape);
  auto status = lua_pcall(l, 0, LUA_MULTRET, 0);
  if (status == LUA_ERRMEM)
    s.lua_memory_failed = true;
  if (exhausted(s))
    return std::unexpected(command_resource(s));
  if (c.bridge_error)
    return std::unexpected(*c.bridge_error);
  if (status || lua_gettop(l) != base + 1)
    return std::unexpected(external_protocol());
  projection_scan scanned;
  lua_pushcfunction(l, scan_projection);
  lua_pushvalue(l, base + 1);
  lua_pushlightuserdata(l, &scanned);
  status = lua_pcall(l, 2, 0, 0);
  if (status == LUA_ERRMEM)
    s.lua_memory_failed = true;
  if (exhausted(s))
    return std::unexpected(command_resource(s));
  if (status || scanned.failure)
    return std::unexpected(scanned.failure == 2 ? external_resource() : external_protocol());
  auto counted = codec_projection_count(
      std::span(scanned.sections.data(), scanned.count),
      {&s, [](void* owner, const codec_projection_reservation& reservation) -> std::expected<void, diagnostic> {
         try {
           charge(*static_cast<state*>(owner), reservation.total_bytes);
           return {};
         } catch (diagnostic& error) {
           return std::unexpected(std::move(error));
         }
       }});
  if (!counted) {
    if (counted.error().kind == codec_projection_failure::resource)
      return std::unexpected(std::move(counted.error().detail.detail));
    return std::unexpected(counted.error().kind == codec_projection_failure::output_limit ? external_resource()
                                                                                          : external_protocol());
  }
  auto result = convert(l, base + 1, 1);
  return result;
}
// Same-VM nested semantic/shape phase. The enclosing handler owns all temporary
// registry roots; this frame restores only its borrowed stack and native aliases.
std::expected<value_ptr, diagnostic> call_retained_value(lua_State* l, int retained, const value_ptr& input) {
  auto& s = context(l);
  validate(s, input, 1);
  charge(s, sizeof(projection_call_frame));
  projection_call_frame restore(l, input, retained);
  const int             base = lua_gettop(l);
  if (!lua_checkstack(l, closure_guard::audit_stack_slots)) {
    s.stack_exhausted = true;
    return std::unexpected(command_resource(s));
  }
  lua_pushcfunction(l, execute_shape);
  auto status = lua_pcall(l, 0, LUA_MULTRET, 0);
  if (status == LUA_ERRMEM)
    s.lua_memory_failed = true;
  if (commands(l).bridge_error)
    return std::unexpected(*commands(l).bridge_error);
  if (exhausted(s))
    return std::unexpected(command_resource(s));
  if (status || lua_gettop(l) != base + 1)
    return std::unexpected(external_protocol());
  return convert(l, base + 1, 1);
}
void begin_invocation(state& s, command_state& c) {
  if (!c.invocation_started) {
    c.invocation_started   = true;
    s.memory_limit         = memory_max;
    s.instruction_limit    = instruction_max;
    s.instructions         = s.carried_instructions;
    s.carried_instructions = 0;
  }
}
// Private protected-call seam: arguments are exact retained roots, no global writes.
[[maybe_unused]] int seal_new_roots(lua_State* l) {
  if (context(l).seal_failed)
    return luaL_error(l, "seal_owner_failed");
  int count = lua_gettop(l);
  if (count > 2040) {
    context(l).seal_failed     = true;
    context(l).stack_exhausted = true;
    return 0;
  }
  std::array<int, 2040> roots{};
  for (int i = 0; i < count; ++i)
    roots[i] = i + 1;
  return seal_roots(l, std::span<const int>(roots.data(), count), false);
}
int seal_environment(lua_State* l) {
  // Reserve before root pushes or native RAII in this active protected frame.
  if (!lua_checkstack(l, closure_guard::audit_stack_slots)) {
    context(l).seal_failed     = true;
    context(l).stack_exhausted = true;
    return 0;
  }
  if (commands(l).entries.size() != command_catalog_size) {
    context(l).seal_failed = true;
    return luaL_error(l, "invalid_catalog");
  }
  std::array<int, command_catalog_size + 2 + std::tuple_size_v<decltype(command_state::shape_phases)>> roots{};
  lua_pushglobaltable(l);
  roots[0] = lua_gettop(l);
  for (std::size_t i = 0; i < command_catalog_size; ++i) {
    lua_rawgeti(l, LUA_REGISTRYINDEX, commands(l).entries[i].reference);
    roots[i + 1] = lua_gettop(l);
  }
  if (commands(l).profile) {
    lua_rawgeti(l, LUA_REGISTRYINDEX, external_environment_index);
    roots[command_catalog_size + 1] = lua_gettop(l);
    for (std::size_t i = 0; i < commands(l).shape_phases.size(); ++i) {
      lua_rawgeti(l, LUA_REGISTRYINDEX, commands(l).shape_phases[i]);
      roots[command_catalog_size + 2 + i] = lua_gettop(l);
    }
    return seal_roots(l, roots, false);
  }
  return seal_roots(l, std::span<const int>(roots.data(), command_catalog_size + 1), true);
}
// Bound C-library multi-result growth before it can reserve an arbitrary stack.
int bounded_codepoints(lua_State* l) {
  std::size_t bytes = 0;
  luaL_checklstring(l, 1, &bytes);
  lua_Integer first = luaL_optinteger(l, 2, 1), last = luaL_optinteger(l, 3, first);
  if (first < 0)
    first = static_cast<lua_Integer>(bytes) + first + 1;
  if (last < 0)
    last = static_cast<lua_Integer>(bytes) + last + 1;
  if (first < 1)
    first = 1;
  if (last > static_cast<lua_Integer>(bytes))
    last = static_cast<lua_Integer>(bytes);
  lua_Integer count     = last >= first ? last - first + 1 : 0;
  int         arguments = lua_gettop(l);
  if (count > 2048 - (l->top.p - l->stack.p) - arguments - 32) {
    context(l).stack_exhausted = true;
    return luaL_error(l, "stack_limit");
  }
  lua_pushvalue(l, lua_upvalueindex(1));
  lua_insert(l, 1);
  lua_call(l, arguments, LUA_MULTRET);
  return lua_gettop(l);
}
// Executed immediately after native installation with no intervening module code.
// Fixed native-owned field names are the complete post-module enrollment policy.
int remember_engine_callbacks(lua_State* l) {
  lua_getglobal(l, "engine");
  for (auto name : {"hash_value", "file_digest", "fragment_digest", "json_encode", "json_decode"}) {
    lua_getfield(l, -1, name);
    closure_guard::remember_installed_callback(l, native_catalog_index, lua_gettop(l));
    lua_pop(l, 1);
  }
  lua_pop(l, 1);
  return 0;
}
int setup_commands(lua_State* l) {
  setup_environment(l);
  // Do not allow registration closures to capture a mutation escape before sealing.
  remove(l, "rawset");
  remove(l, "getmetatable");
  lua_newtable(l);
  lua_pushvalue(l, -1);
  lua_rawseti(l, LUA_REGISTRYINDEX, backing_index);
  for (auto item : {std::pair{"rawget", frozen_rawget}, {"next", frozen_next}}) {
    lua_pushvalue(l, -1);
    lua_pushcclosure(l, item.second, 1);
    lua_setglobal(l, item.first);
  }
  lua_pop(l, 1);
  lua_getglobal(l, "next");
  lua_pushcclosure(l, frozen_pairs, 1);
  lua_setglobal(l, "pairs");
  lua_newtable(l);
  lua_rawseti(l, LUA_REGISTRYINDEX, sealed_index);
  lua_getglobal(l, "table");
  for (auto name : {"pack", "unpack", "create"}) {
    lua_pushnil(l);
    lua_setfield(l, -2, name);
  }
  lua_pop(l, 1);
  for (auto library : {"string", "utf8"}) {
    lua_getglobal(l, library);
    lua_getfield(l, -1, library[0] == 's' ? "byte" : "codepoint");
    lua_pushcclosure(l, bounded_codepoints, 1);
    lua_setfield(l, -2, library[0] == 's' ? "byte" : "codepoint");
    lua_pop(l, 1);
  }
  // Registration has data constructors but no hashing, output or native service capability.
  lua_getglobal(l, "engine");
  for (auto name : {"hash_value", "file_digest", "fragment_digest", "json_encode", "json_decode"}) {
    lua_pushnil(l);
    lua_setfield(l, -2, name);
  }
  lua_pop(l, 1);
  lua_newtable(l);
  lua_pushcfunction(l, register_command);
  lua_setfield(l, -2, "add");
  lua_setglobal(l, "registration");
  if (commands(l).profile) {
    lua_getglobal(l, "registration");
    for (int i = 0; i < 3; ++i) {
      lua_pushinteger(l, i);
      lua_pushcclosure(l, register_external, 1);
      lua_setfield(l, -2, i == 0 ? "add" : i == 1 ? "shape" : "projection");
    }
    // Shipped registration still uses its original native source-origin policy.
    lua_pushcfunction(l, register_command);
    lua_setfield(l, -2, "add");
    lua_pop(l, 1);
    // Build from pristine curated globals, before embedded evaluation or freezing.
    lua_newtable(l);
    int external = lua_gettop(l);
    lua_pushglobaltable(l);
    lua_pushnil(l);
    while (lua_next(l, -2)) {
      lua_pushvalue(l, -2);
      lua_pushvalue(l, -2);
      lua_rawset(l, external);
      lua_pop(l, 1);
    }
    lua_pop(l, 1);
    lua_pushvalue(l, external);
    lua_setfield(l, external, "_G");
    lua_newtable(l);
    lua_getglobal(l, "engine");
    for (auto name : {"kind", "object", "array"}) {
      lua_getfield(l, -1, name);
      lua_setfield(l, -3, name);
    }
    lua_pop(l, 1);
    lua_setfield(l, external, "engine");
    // The external API has its own add identity; all three are captured pristine.
    lua_newtable(l);
    for (int i = 0; i < 3; ++i) {
      lua_pushinteger(l, i);
      lua_pushcclosure(l, register_external, 1);
      lua_setfield(l, -2, i == 0 ? "add" : i == 1 ? "shape" : "projection");
    }
    lua_setfield(l, external, "registration");
    lua_pushvalue(l, external);
    lua_rawseti(l, LUA_REGISTRYINDEX, external_environment_index);
    lua_pop(l, 1);
  }
  closure_guard::snapshot_curated_natives(l, native_catalog_index);
  if (commands(l).profile) {
    // Snapshot exact external API natives through a temporary native-owned root.
    // The guard snapshot only follows table values, never opaque C upvalues.
    lua_rawgeti(l, LUA_REGISTRYINDEX, external_environment_index);
    lua_getfield(l, -1, "registration");
    for (auto name : {"add", "shape", "projection"}) {
      lua_getfield(l, -1, name);
      closure_guard::remember_installed_callback(l, native_catalog_index, lua_gettop(l));
      lua_pop(l, 1);
    }
    lua_pop(l, 2);
  }
  if (luaL_loadbufferx(l, embedded::source().data(), embedded::source().size(), "trusted", "t"))
    return lua_error(l);
  lua_call(l, 0, 1);
  if (lua_type(l, -1) != LUA_TTABLE)
    return luaL_error(l, "invalid_exports");
  if (commands(l).profile) {
    constexpr const char* names[] = {"shape_prepare",
                                     "shape_check",
                                     "shape_finish",
                                     "select_profile_material",
                                     "prepare_profile_render",
                                     "render_profile_material",
                                     "assemble_projection_material",
                                     "role_profile_material",
                                     "route_workspace",
                                     "role_projection_base_material"};
    for (std::size_t i = 0; i < commands(l).shape_phases.size(); ++i) {
      lua_getfield(l, -1, names[i]);
      if (!lua_isfunction(l, -1) || lua_iscfunction(l, -1))
        return luaL_error(l, "invalid_exports");
      commands(l).shape_phases[i] = luaL_ref(l, LUA_REGISTRYINDEX);
    }
  }
  lua_pop(l, 1);
  lua_getglobal(l, "engine");
  for (int i = 0; i < 3; ++i) {
    lua_pushinteger(l, i);
    lua_pushcclosure(l, digest, 1);
    lua_setfield(l, -2, i == 0 ? "hash_value" : i == 1 ? "file_digest" : "fragment_digest");
  }
  for (int i = 0; i < 2; ++i) {
    lua_pushboolean(l, i);
    lua_pushcclosure(l, json_codec, 1);
    lua_setfield(l, -2, i ? "json_encode" : "json_decode");
  }
  lua_pop(l, 1);
  return 0;
}
bool token(std::string_view t) {
  return !t.empty() && !t.starts_with("--") && t.find('\0') == std::string_view::npos;
}
bool positive(std::int64_t n) {
  return n > 0;
}
bool workspace_identifier(std::string_view id) {
  if (id.empty() || id.size() > 64 || id.front() < 'a' || id.front() > 'z')
    return false;
  return std::ranges::all_of(id, [](char c) { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'; });
}
bool ledger_identity(std::string_view id) {
  return id.size() == 32 && std::ranges::all_of(id, [](char c) { return (c >= 'a' && c <= 'f') || (c >= '0' && c <= '9'); });
}
bool decimal_id(std::string_view id, bool zero = false) {
  if (id == "0")
    return zero;
  if (id.empty() || id.front() < '1' || id.front() > '9' || id.size() > 19 || (id.size() == 19 && id > "9223372036854775807"))
    return false;
  return std::ranges::all_of(id, [](char c) { return c >= '0' && c <= '9'; });
}
void require_token(const std::string& t) {
  if (!token(t))
    command_refuse();
}
std::pair<std::string_view, std::string_view> mapping(const std::string& t) {
  require_token(t);
  auto p = t.find('=');
  if (p == 0 || p == std::string::npos || p + 1 == t.size())
    command_refuse();
  return {std::string_view(t).substr(0, p), std::string_view(t).substr(p + 1)};
}
void prepare_invocation(command_state& c) {
  std::map<std::string, value_ptr> options, present;
  auto                             add = [&](std::string name, value_ptr v, bool exists = true) {
    options[name] = std::move(v);
    present[name] = command_boolean(exists);
  };
  auto path = [&](std::string name, const std::string& text, bool exists = true) {
    if (exists) {
      require_token(text);
      c.handles.emplace(name, nullptr);
    }
    present[name] = command_boolean(exists);
  };
  std::visit(
      [&](const auto& o) {
        using T = std::decay_t<decltype(o)>;
        if constexpr (std::is_same_v<T, command_render_options>) {
          c.name = "render";
          path("workspace", o.workspace);
          path("registry", o.registry);
          path("root", o.root);
          auto root = mapping(o.root);
          if (o.base_root.has_value() != o.capture_outputs || (o.base_root && mapping(*o.base_root).first != root.first))
            command_refuse();
          path("base-root", o.base_root.value_or(""), o.base_root.has_value());
          path("observations", o.observations.value_or(""), o.observations.has_value());
          require_token(o.job);
          require_token(o.role);
          add("job", command_text(o.job));
          add("role", command_text(o.role));
          if (o.budget_bytes && (*o.budget_bytes < 1 || *o.budget_bytes > 1048576))
            command_refuse();
          add("budget-bytes", command_integer(o.budget_bytes.value_or(0)), o.budget_bytes.has_value());
          add("capture-outputs", command_boolean(o.capture_outputs), o.capture_outputs);
        } else if constexpr (std::is_same_v<T, command_init_options>) {
          c.name = "init";
          path("ledger", o.ledger);
          require_token(o.workspace_id);
          add("workspace-id", command_text(o.workspace_id));
        } else if constexpr (std::is_same_v<T, command_history_options>) {
          c.name = "history";
          path("ledger", o.ledger);
          auto id = [&](std::string name, auto v) {
            if (v && !positive(*v))
              command_refuse();
            add(name, command_text(v ? std::to_string(*v) : "0"), v.has_value());
          };
          id("revision", o.revision);
          id("at-event", o.at_event);
          id("after-event", o.after_event);
          int selectors = o.revision.has_value() + o.at_event.has_value() + o.request_id.has_value();
          if (selectors > 1 || (selectors && (o.after_event || o.limit)))
            command_refuse();
          if (o.request_id) {
            if (o.request_id->empty() || o.request_id->size() > 128)
              command_refuse();
            for (unsigned char ch : *o.request_id)
              if (!((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_' || ch == '.' ||
                    ch == ':' || ch == '-'))
                command_refuse();
          }
          add("request-id", command_text(o.request_id.value_or("")), o.request_id.has_value());
          if (o.limit && (*o.limit < 1 || *o.limit > 100))
            command_refuse();
          add("limit", command_integer(o.limit.value_or(0)), o.limit.has_value());
        } else if constexpr (std::is_same_v<T, command_load_options>) {
          if (command_catalog_size != 5)
            command_refuse();
          c.name = "load";
          path("ledger", o.ledger);
          path("workspace", o.workspace);
          path("registry", o.registry);
          if ((o.expected_head && (!positive(*o.expected_head) || o.expect_revision != std::to_string(*o.expected_head))) ||
              (!o.expected_head && o.expect_revision != "none") || o.request_id.empty() || o.request_id.size() > 128 ||
              o.actor.empty() || o.actor.size() > 256 || !valid_text(o.actor))
            command_refuse();
          for (unsigned char ch : o.request_id)
            if (!((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_' || ch == '.' ||
                  ch == ':' || ch == '-'))
              command_refuse();
          add("expect-revision", command_text(o.expect_revision));
          add("request-id", command_text(o.request_id));
          add("actor", command_text(o.actor));
        } else {
          c.name = "export";
          path("ledger", o.ledger);
          path("target", o.target);
          if (!positive(o.revision))
            command_refuse();
          add("revision", command_text(std::to_string(o.revision)));
        }
      },
      c.options);
  c.invocation = command_record({{"version", command_integer(1)},
                                 {"command", command_text(c.name)},
                                 {"options", command_record(std::move(options))},
                                 {"present", command_record(std::move(present))}});
}
void native_error_schema(const value_ptr& e) {
  if (!e || e->kind != value_kind::record || !e->fields.contains("code") || !e->fields.contains("message") ||
      !e->fields.contains("field"))
    command_refuse();
  for (auto& [key, v] : e->fields) {
    if (key == "details") {
      if (v->kind != value_kind::record)
        command_refuse();
    } else if (key == "code" || key == "message" || key == "file" || key == "field") {
      if (v->kind != value_kind::string || !valid_text(v->string))
        command_refuse();
    } else
      command_refuse();
  }
  if (text_field(e, "code").empty())
    command_refuse();
  if (e->fields.contains("file") && !text_field(e, "file").empty() && !valid_path(text_field(e, "file")))
    command_refuse();
}
// Closed external response scan. All borrowed strings stay rooted by the original
// return value. These frames own only trivial storage across Lua allocations.
std::string_view extension_raw_text(lua_State* l, int index) {
  if (lua_type(l, index) != LUA_TSTRING)
    return {};
  std::size_t size  = 0;
  auto        bytes = lua_tolstring(l, index, &size);
  return {bytes, size};
}
bool extension_raw_string(lua_State* l, int index) {
  return lua_type(l, index) == LUA_TSTRING && valid_text(extension_raw_text(l, index));
}
bool extension_raw_keys(lua_State* l, int table, std::initializer_list<std::string_view> required,
                        std::initializer_list<std::string_view> optional = {}) {
  std::size_t found = 0;
  lua_pushnil(l);
  while (lua_next(l, table)) {
    if (lua_type(l, -2) != LUA_TSTRING)
      return false;
    auto key = extension_raw_text(l, -2);
    if (std::ranges::find(required, key) != required.end())
      ++found;
    else if (std::ranges::find(optional, key) == optional.end())
      return false;
    lua_pop(l, 1);
  }
  return found == required.size();
}
void extension_raw_field(lua_State* l, int table, const char* key) {
  lua_pushstring(l, key);
  lua_rawget(l, table);
}
bool extension_raw_uint(lua_State* l, int index, bool positive = false) {
  return lua_isinteger(l, index) && lua_tointeger(l, index) >= (positive ? 1 : 0) && lua_tointeger(l, index) <= 2147483647;
}
bool extension_raw_string_field(lua_State* l, int table, const char* key) {
  extension_raw_field(l, table, key);
  bool result = extension_raw_string(l, -1);
  lua_pop(l, 1);
  return result;
}
bool extension_raw_uint_field(lua_State* l, int table, const char* key) {
  extension_raw_field(l, table, key);
  bool result = extension_raw_uint(l, -1);
  lua_pop(l, 1);
  return result;
}
bool extension_cycle_label(std::string_view text) {
  auto colon = text.find(':');
  if (colon == std::string_view::npos)
    return false;
  auto kind = text.substr(0, colon);
  text.remove_prefix(colon + 1);
  colon = text.find(':');
  if (colon == std::string_view::npos || !workspace_identifier(text.substr(0, colon)))
    return false;
  auto phase = text.substr(colon + 1);
  return (kind == "work" && (phase == "start" || phase == "complete" || phase == "accepted")) ||
         (kind == "role" && (phase == "start" || phase == "complete"));
}
// mode: supported versions, ownership paths, cycle labels, budget sections.
bool extension_raw_list(lua_State* l, int index, int mode) {
  int list = projection_table(l, index, value_kind::list);
  if (!list)
    return false;
  std::size_t count = 0;
  lua_pushnil(l);
  while (lua_next(l, list)) {
    if (!lua_isinteger(l, -2) || lua_tointeger(l, -2) < 1)
      return false;
    ++count;
    lua_pop(l, 1);
  }
  if (mode == 0 && count != 1)
    return false;
  std::string_view previous;
  for (std::size_t i = 0; i < count; ++i) {
    lua_rawgeti(l, list, static_cast<lua_Integer>(i + 1));
    if (mode == 0) {
      if (!lua_isinteger(l, -1) || lua_tointeger(l, -1) != 1)
        return false;
    } else if (mode == 3) {
      int section = projection_table(l, -1, value_kind::record);
      if (!section || !extension_raw_keys(l, section, {"id", "bytes"}) || !extension_raw_uint_field(l, section, "bytes"))
        return false;
      extension_raw_field(l, section, "id");
      if (!extension_raw_string(l, -1) || !workspace_identifier(extension_raw_text(l, -1)))
        return false;
      lua_pop(l, 2);
    } else {
      if (!extension_raw_string(l, -1))
        return false;
      auto text = extension_raw_text(l, -1);
      if (mode == 1) {
        if ((!valid_path(text) && !valid_path(text, true)) || (i && text < previous))
          return false;
        previous = text;
      } else if (!extension_cycle_label(text))
        return false;
    }
    lua_pop(l, 1);
  }
  lua_pop(l, 1);
  return true;
}
bool extension_raw_details(lua_State* l, int index, std::string_view code) {
  int table = projection_table(l, index, value_kind::record);
  if (!table)
    return false;
  if (code == "usage" || code == "io" || code == "observation_target_mismatch") {
    const char* key = code == "usage" ? "option" : code == "io" ? "operation" : "work";
    if (!extension_raw_keys(l, table, {key}) || !extension_raw_string_field(l, table, key))
      return false;
    if (code == "observation_target_mismatch") {
      extension_raw_field(l, table, key);
      if (!workspace_identifier(extension_raw_text(l, -1)))
        return false;
      lua_pop(l, 1);
    }
  } else if (code == "resource_limit") {
    if (!extension_raw_keys(l, table, {"limit", "maximum"}) || !extension_raw_string_field(l, table, "limit") ||
        !extension_raw_uint_field(l, table, "maximum"))
      return false;
  } else if (code == "unsupported_version") {
    if (!extension_raw_keys(l, table, {"supported", "received"}))
      return false;
    extension_raw_field(l, table, "received");
    if (!lua_isinteger(l, -1) || lua_tointeger(l, -1) < -2147483648LL || lua_tointeger(l, -1) > 2147483647)
      return false;
    lua_pop(l, 1);
    extension_raw_field(l, table, "supported");
    if (!extension_raw_list(l, -1, 0))
      return false;
    lua_pop(l, 1);
  } else if (code == "ownership" || code == "dependency_cycle") {
    const char* key = code == "ownership" ? "paths" : "cycle";
    if (!extension_raw_keys(l, table, {key}))
      return false;
    extension_raw_field(l, table, key);
    if (!extension_raw_list(l, -1, code == "ownership" ? 1 : 2))
      return false;
    lua_pop(l, 1);
  } else if (code == "budget_exceeded") {
    if (!extension_raw_keys(l, table, {"budget_bytes", "mandatory_bytes", "sections"}) ||
        !extension_raw_uint_field(l, table, "budget_bytes") || !extension_raw_uint_field(l, table, "mandatory_bytes"))
      return false;
    extension_raw_field(l, table, "sections");
    if (!extension_raw_list(l, -1, 3))
      return false;
    lua_pop(l, 1);
  } else
    return false;
  lua_pop(l, 1);
  return true;
}
bool extension_raw_error(lua_State* l, int index, bool trusted_global = false) {
  int table = projection_table(l, index, value_kind::record);
  if (!table || !extension_raw_keys(l, table, {"code", "message", "field"}, {"file", "line", "details"}) ||
      !extension_raw_string_field(l, table, "message") || !extension_raw_string_field(l, table, "field"))
    return false;
  extension_raw_field(l, table, "code");
  if (!extension_raw_string(l, -1))
    return false;
  auto                                       code = extension_raw_text(l, -1);
  constexpr std::array<std::string_view, 16> codes{"usage",           "unsupported_version",
                                                   "lua_syntax",      "capability_denied",
                                                   "schema",          "reference",
                                                   "ownership",       "dependency_cycle",
                                                   "path_denied",     "capture_changed",
                                                   "resource_limit",  "observation_target_mismatch",
                                                   "budget_exceeded", "io",
                                                   "internal",        ""};
  if (code.empty() || std::ranges::find(codes, code) == codes.end())
    return false;
  lua_pop(l, 1);
  extension_raw_field(l, table, "file");
  if (!lua_isnil(l, -1) && (!extension_raw_string(l, -1) ||
                            (!valid_path(extension_raw_text(l, -1)) && !(trusted_global && extension_raw_text(l, -1).empty()))))
    return false;
  lua_pop(l, 1);
  extension_raw_field(l, table, "line");
  if (!lua_isnil(l, -1) && !extension_raw_uint(l, -1, true))
    return false;
  lua_pop(l, 1);
  extension_raw_field(l, table, "details");
  if (!lua_isnil(l, -1) && !extension_raw_details(l, -1, code))
    return false;
  lua_pop(l, 2);
  return true;
}
// Exact finite trusted assembly ABI. Separate from raw custom-section admission.
// Only trivial locals cross protected Lua operations; the outer result stays rooted.
bool assembly_list(lua_State* l, int index, bool metadata) {
  int list = projection_table(l, index, value_kind::list);
  if (!list)
    return false;
  std::size_t count = 0;
  lua_pushnil(l);
  while (lua_next(l, list)) {
    if (++count > 66 || !lua_isinteger(l, -2) || lua_tointeger(l, -2) < 1 || lua_tointeger(l, -2) > 66)
      return false;
    lua_pop(l, 1);
  }
  for (std::size_t i = 0; i < count; ++i) {
    lua_rawgeti(l, list, static_cast<lua_Integer>(i + 1));
    if (metadata) {
      int entry = projection_table(l, -1, value_kind::record);
      if (!entry || !extension_raw_keys(l, entry, {"id", "bytes", "included"}) || !extension_raw_uint_field(l, entry, "bytes"))
        return false;
      extension_raw_field(l, entry, "id");
      if (!extension_raw_string(l, -1) || !workspace_identifier(extension_raw_text(l, -1)))
        return false;
      lua_pop(l, 1);
      extension_raw_field(l, entry, "included");
      if (lua_type(l, -1) != LUA_TBOOLEAN)
        return false;
      lua_pop(l, 2);
    } else if (!extension_raw_string(l, -1) || !workspace_identifier(extension_raw_text(l, -1)))
      return false;
    lua_pop(l, 1);
  }
  lua_pop(l, 1);
  return true;
}
int scan_assembly_response(lua_State* l) {
  if (!lua_checkstack(l, 64)) {
    context(l).stack_exhausted = true;
    return 0;
  }
  int root = projection_table(l, 1, value_kind::record);
  if (!root)
    return 0;
  extension_raw_field(l, root, "error");
  bool ok = lua_isnil(l, -1);
  lua_pop(l, 1);
  if (!extension_raw_keys(l, root, {ok ? "value" : "error"}))
    return 0;
  extension_raw_field(l, root, ok ? "value" : "error");
  if (!ok) {
    if (!extension_raw_error(l, -1, true))
      return 0;
  } else {
    int output = projection_table(l, -1, value_kind::record);
    if (!output || !extension_raw_keys(l, output, {"text", "sections", "omitted"}))
      return 0;
    extension_raw_field(l, output, "text");
    if (!extension_raw_string(l, -1) || extension_raw_text(l, -1).size() > 1048576)
      return 0;
    lua_pop(l, 1);
    extension_raw_field(l, output, "sections");
    if (!assembly_list(l, -1, true))
      return 0;
    lua_pop(l, 1);
    extension_raw_field(l, output, "omitted");
    if (!assembly_list(l, -1, false))
      return 0;
  }
  lua_pushboolean(l, true);
  return 1;
}
std::expected<value_ptr, diagnostic> call_retained_assembly(lua_State* l, int retained, const value_ptr& input) {
  auto& s = context(l);
  charge(s, sizeof(projection_call_frame));
  projection_call_frame restore(l, input, retained);
  const int             base = lua_gettop(l);
  if (!lua_checkstack(l, closure_guard::audit_stack_slots)) {
    s.stack_exhausted = true;
    return std::unexpected(command_resource(s));
  }
  lua_pushcfunction(l, execute_shape);
  auto status = lua_pcall(l, 0, LUA_MULTRET, 0);
  if (status == LUA_ERRMEM)
    s.lua_memory_failed = true;
  if (commands(l).bridge_error)
    return std::unexpected(*commands(l).bridge_error);
  if (exhausted(s))
    return std::unexpected(command_resource(s));
  if (status || lua_gettop(l) != base + 1)
    return std::unexpected(external_protocol());
  lua_pushcfunction(l, scan_assembly_response);
  lua_pushvalue(l, base + 1);
  status = lua_pcall(l, 1, 1, 0);
  if (status == LUA_ERRMEM)
    s.lua_memory_failed = true;
  if (exhausted(s))
    return std::unexpected(command_resource(s));
  if (status || !lua_toboolean(l, -1))
    return std::unexpected(external_protocol());
  auto result  = convert(l, base + 1, 1);
  auto encoded = codec_encode_reserved(
      result, {&s, [](void* owner, const codec_reservation& reservation) -> std::expected<void, diagnostic> {
                 try {
                   charge(*static_cast<state*>(owner), reservation.total_bytes);
                   return {};
                 } catch (diagnostic& error) {
                   return std::unexpected(std::move(error));
                 }
               }});
  if (!encoded)
    return std::unexpected(encoded.error().detail.code == "resource_limit" ? std::move(encoded.error().detail)
                                                                           : external_protocol());
  return result;
}
// Compare raw/proxy values directly to retained native facts before any owned
// response conversion. Expected depth/size is already admitted by validate().
bool extension_history_matches(lua_State* l, int index, const value* expected) {
  if (!lua_checkstack(l, 16)) {
    context(l).stack_exhausted = true;
    return false;
  }
  if (expected->kind == value_kind::string)
    return lua_type(l, index) == LUA_TSTRING && extension_raw_text(l, index) == expected->string;
  if (expected->kind == value_kind::boolean)
    return lua_type(l, index) == LUA_TBOOLEAN && bool(lua_toboolean(l, index)) == expected->boolean;
  if (expected->kind == value_kind::integer)
    return lua_isinteger(l, index) && lua_tointeger(l, index) == expected->integer;
  int table = projection_table(l, index, expected->kind);
  if (!table)
    return false;
  std::size_t count = 0;
  lua_pushnil(l);
  while (lua_next(l, table)) {
    if (++count > expected->fields.size() + expected->items.size())
      return false;
    lua_pop(l, 1);
  }
  if (count != expected->fields.size() + expected->items.size())
    return false;
  for (const auto& [key, child] : expected->fields) {
    lua_pushlstring(l, key.data(), key.size());
    lua_rawget(l, table);
    if (!extension_history_matches(l, -1, child.get()))
      return false;
    lua_pop(l, 1);
  }
  for (std::size_t i = 0; i < expected->items.size(); ++i) {
    lua_rawgeti(l, table, static_cast<lua_Integer>(i + 1));
    if (!extension_history_matches(l, -1, expected->items[i].get()))
      return false;
    lua_pop(l, 1);
  }
  lua_pop(l, 1);
  return true;
}
// Returns true for valid structure, integer2 for the summary bound, otherwise nil.
// No mutable native scan owner escapes.
int scan_extension_response(lua_State* l) {
  if (!lua_checkstack(l, 64)) {
    context(l).stack_exhausted = true;
    return 0;
  }
  int root = projection_table(l, 1, value_kind::record);
  if (!root)
    return 0;
  extension_raw_field(l, root, "ok");
  if (lua_type(l, -1) != LUA_TBOOLEAN)
    return 0;
  bool ok = lua_toboolean(l, -1);
  lua_pop(l, 1);
  if (!extension_raw_keys(l, root, {"ok", ok ? "value" : "error"}))
    return 0;
  extension_raw_field(l, root, ok ? "value" : "error");
  if (!ok) {
    if (!extension_raw_error(l, -1))
      return 0;
  } else {
    int envelope = projection_table(l, -1, value_kind::record);
    if (!envelope || !extension_raw_keys(l, envelope, {"version", "kind", "command", "data"}))
      return 0;
    extension_raw_field(l, envelope, "version");
    if (!lua_isinteger(l, -1) || lua_tointeger(l, -1) != 1)
      return 0;
    lua_pop(l, 1);
    extension_raw_field(l, envelope, "kind");
    if (!projection_key(l, -1, "extension"))
      return 0;
    lua_pop(l, 1);
    extension_raw_field(l, envelope, "command");
    if (!projection_key(l, -1, commands(l).name))
      return 0;
    lua_pop(l, 1);
    extension_raw_field(l, envelope, "data");
    int   data   = projection_table(l, -1, value_kind::record);
    auto& bridge = commands(l).extension_history;
    if (bridge.enabled) {
      if (data && bridge.projection_result && !bridge.refused) {
        extension_raw_field(l, data, "type");
        bool projected = projection_key(l, -1, "projection");
        lua_pop(l, 1);
        if (projected) {
          if (!extension_history_matches(l, -2, bridge.projection_result))
            return 0;
          lua_pushboolean(l, true);
          return 1;
        }
      }
      if (!data || !bridge.result || bridge.refused || !extension_raw_keys(l, data, {"type", "value"}))
        return 0;
      extension_raw_field(l, data, "type");
      if (!projection_key(l, -1, bridge.role_enabled ? "role" : "history"))
        return 0;
      lua_pop(l, 1);
      extension_raw_field(l, data, "value");
      if (!extension_history_matches(l, -1, bridge.result))
        return 0;
      lua_pushboolean(l, true);
      return 1;
    }
    if (!data || !extension_raw_keys(l, data, {"type", "text"}))
      return 0;
    extension_raw_field(l, data, "type");
    if (!projection_key(l, -1, "summary"))
      return 0;
    lua_pop(l, 1);
    extension_raw_field(l, data, "text");
    if (!extension_raw_string(l, -1))
      return 0;
    if (extension_raw_text(l, -1).size() > 1048576) {
      lua_pushinteger(l, 2);
      return 1;
    }
  }
  lua_pushboolean(l, true);
  return 1;
}

command_history_selector history_selector(const command_history_options& o) {
  command_history_selector s{"page", o.after_event ? std::to_string(*o.after_event) : "0", "", o.limit.value_or(50)};
  if (o.revision) {
    s.selector = "revision";
    s.id       = std::to_string(*o.revision);
  } else if (o.at_event) {
    s.selector = "state";
    s.id       = std::to_string(*o.at_event);
  } else if (o.request_id) {
    s.selector   = "request";
    s.id         = "0";
    s.request_id = *o.request_id;
  }
  return s;
}
void validate_route(command_state& c, const value_ptr& route) {
  exact(route, {"workspace", "file", "jobs", "root", "registry", "spec"});
  if (!c.opened || text_field(route, "file") != text_field(c.opened, "file") || !valid_path(text_field(route, "file")) ||
      !valid_path(text_field(route, "spec")))
    command_refuse();
  auto root = route->fields.at("root"), registry = route->fields.at("registry"), jobs = route->fields.at("jobs");
  exact(root, {"id", "base", "excluded"});
  exact(registry, {"id", "revision"});
  // The concrete capture service owns mapping agreement and its typed reference
  // refusal before reading any routed inputs. Keep protocol type checks here.
  text_field(root, "id");
  if (jobs->kind != value_kind::list)
    command_refuse();
  std::string previous;
  for (auto& j : jobs->items) {
    exact(j, {"id", "file"});
    auto id = text_field(j, "id");
    if (!previous.empty() && id <= previous)
      command_refuse();
    previous = id;
    if (!valid_path(text_field(j, "file")))
      command_refuse();
  }
}
// Native argument/result owners exist only in this non-allocating-Lua frame.
void call_service_native(lua_State* l, int operation) {
  auto& s = context(l);
  auto& c = commands(l);
  if (c.registering || !c.services || c.native_error || c.bridge_error || lua_gettop(l) != 1)
    command_refuse();
  if (exhausted(s))
    throw command_resource(s);
  command_service_result result;
  if (operation == 0) {
    if ((c.name != "render" && c.name != "load") || c.calls != 0 || lua_type(l, 1) != LUA_TTABLE || lua_getmetatable(l, 1))
      command_refuse();
    lua_pushnil(l);
    int fields = 0;
    while (lua_next(l, 1)) {
      ++fields;
      if (lua_type(l, -2) != LUA_TSTRING || std::string_view(lua_tostring(l, -2)) != "workspace" ||
          lua_type(l, -1) != LUA_TUSERDATA || lua_topointer(l, -1) != c.handles.at("workspace"))
        command_refuse();
      lua_pop(l, 1);
    }
    if (fields != 1)
      command_refuse();
    if (c.name == "load")
      result = c.services->open_workspace(std::get<command_load_options>(c.options));
    else
      result = c.services->open_workspace(std::get<command_render_options>(c.options));
  } else {
    auto args = convert(l, 1, 1);
    if (operation == 1) {
      if (c.name != "render" || c.calls != 1)
        command_refuse();
      exact(args, {"route"});
      validate_route(c, args->fields.at("route"));
      result = c.services->capture_render(std::get<command_render_options>(c.options), args->fields.at("route"));
    } else if (operation == 2) {
      if (c.name != "init" || c.calls != 0)
        command_refuse();
      exact(args, {"workspace_id"});
      auto& o = std::get<command_init_options>(c.options);
      if (text_field(args, "workspace_id") != o.workspace_id || !workspace_identifier(o.workspace_id))
        command_refuse();
      result = c.services->initialize_ledger(o);
    } else if (operation == 3) {
      if (c.name != "history" || c.calls != 0)
        command_refuse();
      exact(args, {"selector", "id", "request_id", "limit"});
      auto& o        = std::get<command_history_options>(c.options);
      auto  selector = history_selector(o);
      if (text_field(args, "selector") != selector.selector || text_field(args, "id") != selector.id ||
          text_field(args, "request_id") != selector.request_id || args->fields.at("limit")->kind != value_kind::integer ||
          args->fields.at("limit")->integer != selector.limit)
        command_refuse();
      result = c.services->read_history(o, selector);
    } else if (operation == 4) {
      if (c.name != "export" || c.calls != 0)
        command_refuse();
      exact(args, {"revision"});
      auto& o = std::get<command_export_options>(c.options);
      if (text_field(args, "revision") != std::to_string(o.revision))
        command_refuse();
      result = c.services->export_source(o);
      if (result.value)
        c.export_published = true;
    } else if (operation == 5) {
      if (c.name != "load" || c.calls != 1)
        command_refuse();
      exact(args, {"route"});
      validate_route(c, args->fields.at("route"));
      result = c.services->capture_load(std::get<command_load_options>(c.options), args->fields.at("route"));
    } else if (operation == 6) {
      if (c.name != "load" || c.calls != 2)
        command_refuse();
      exact(args, {"bundle", "semantic"});
      if (args->fields.at("bundle")->kind != value_kind::record || args->fields.at("semantic")->kind != value_kind::record)
        command_refuse();
      // Registration retains identity only. Charge the owned acceptance copies
      // cumulatively, before the codec allocates and before native completion.
      charge(s, c.assembly.size() + 65536);
      for (auto& f : c.originals)
        charge(s, f.path.size() + f.bytes.size() + sizeof(history::tree_file));
      auto provenance = history::prepare_stdlib(c.originals, c.separator, c.assembly);
      if (!provenance)
        command_refuse();
      c.provenance = std::move(provenance->files);
      result       = c.services->accept_load(std::get<command_load_options>(c.options), args->fields.at("bundle"),
                                             args->fields.at("semantic"), c.provenance, c.completion);
    } else
      command_refuse();
  }
  ++c.calls;
  if (bool(result.value) == bool(result.error))
    command_refuse();
  auto data = result.error ? result.error : result.value;
  validate(s, data, 1);
  auto copied = copy_value(data);
  if (result.error) {
    native_error_schema(copied);
    c.native_error = copied;
    c.response     = command_record({{"ok", command_boolean(false)}, {"error", copied}});
  } else {
    if (operation == 2) {
      exact(copied, {"ledger", "workspace", "active_revision", "event"});
      if (!ledger_identity(text_field(copied, "ledger")) ||
          text_field(copied, "workspace") != std::get<command_init_options>(c.options).workspace_id ||
          text_field(copied, "active_revision") != "none" || text_field(copied, "event") != "1")
        command_refuse();
    }
    if (operation == 4) {
      exact(copied, {"ledger", "revision", "source"});
      if (!ledger_identity(text_field(copied, "ledger")) ||
          text_field(copied, "revision") != std::to_string(std::get<command_export_options>(c.options).revision) ||
          !decimal_id(text_field(copied, "source")))
        command_refuse();
    }
    if (operation == 3) {
      exact(copied, {"version", "kind", "ledger", "active_revision", "last_event", "data"});
      if (copied->fields.at("version")->kind != value_kind::integer || copied->fields.at("version")->integer != 1 ||
          text_field(copied, "kind") != "history" || !ledger_identity(text_field(copied, "ledger")) ||
          (text_field(copied, "active_revision") != "none" && !decimal_id(text_field(copied, "active_revision"))) ||
          !decimal_id(text_field(copied, "last_event"), true))
        command_refuse();
    }
    if (operation == 5)
      exact(copied, {"bundle"});
    if (operation == 6) {
      exact(copied, {"version", "kind", "ledger", "request_id", "outcome", "revision", "parent", "event"});
      if (copied->fields.at("version")->kind != value_kind::integer || copied->fields.at("version")->integer != 1 ||
          text_field(copied, "kind") != "load" || !ledger_identity(text_field(copied, "ledger")) ||
          text_field(copied, "request_id") != std::get<command_load_options>(c.options).request_id ||
          (text_field(copied, "outcome") != "accepted" && text_field(copied, "outcome") != "noop") ||
          !decimal_id(text_field(copied, "revision")) || !decimal_id(text_field(copied, "event")) ||
          (text_field(copied, "parent") != "none" && !decimal_id(text_field(copied, "parent"))))
        command_refuse();
      auto encoded = codec_encode(copied);
      if (!encoded ||
          (c.completion.phase != ledger::completion_phase::committed &&
           c.completion.phase != ledger::completion_phase::replayed) ||
          *encoded != c.completion.saved_result)
        command_refuse();
    }
    c.facts = copied;
    if (operation == 0) {
      exact(copied, {"file", "workspace"});
      if (!valid_path(text_field(copied, "file")))
        command_refuse();
      c.opened = copied;
    }
    c.response = command_record({{"ok", command_boolean(true)}, {"value", copied}});
  }
}
int call_service(lua_State* l) {
  bool failed = false;
  try {
    call_service_native(l, static_cast<int>(lua_tointeger(l, lua_upvalueindex(1))));
  } catch (diagnostic& e) {
    command_failure(l, std::move(e));
    failed = true;
  } catch (...) {
    command_failure(l, {"internal", "Native command service failed"});
    failed = true;
  }
  if (failed)
    return luaL_error(l, "command_service");
  push(l, commands(l).response.get());
  return 1;
}
int execute_command(lua_State* l) {
  // Stable references were retained during registration in this very VM.
  for (auto& e : commands(l).entries)
    if (text_field(e.descriptor, "name") == commands(l).name) {
      lua_rawgeti(l, LUA_REGISTRYINDEX, e.reference);
      break;
    }
  push(l, commands(l).invocation.get());
  lua_getfield(l, -1, "options");
  for (auto& [name, identity] : commands(l).handles) {
    identity = lua_newuserdatauv(l, 0, 0); // identity is the address of this unique userdata
    identity = lua_topointer(l, -1);
    lua_setfield(l, -2, name.c_str());
  }
  lua_pop(l, 1);
  lua_newtable(l);
  const char* methods[] = {"open_workspace", "capture_render", "initialize_ledger", "read_history",
                           "export_source",  "capture_load",   "accept_load"};
  for (int i = 0; i < 7; ++i)
    if ((commands(l).name == "render" && i < 2) || (commands(l).name == "init" && i == 2) ||
        (commands(l).name == "history" && i == 3) || (commands(l).name == "export" && i == 4) ||
        (commands(l).name == "load" && (i == 0 || i == 5 || i == 6))) {
      lua_pushinteger(l, i);
      lua_pushcclosure(l, call_service, 1);
      lua_setfield(l, -2, methods[i]);
    }
  lua_call(l, 2, LUA_MULTRET);
  return lua_gettop(l);
}
void validate_response(command_state& c, const value_ptr& response) {
  if (!response || response->kind != value_kind::record || !response->fields.contains("ok") ||
      response->fields.at("ok")->kind != value_kind::boolean)
    command_refuse();
  if (!response->fields.at("ok")->boolean) {
    exact(response, {"ok", "error"});
    native_error_schema(response->fields.at("error"));
    if (c.name == "load" && c.calls == 3)
      command_refuse();
    return;
  }
  exact(response, {"ok", "value"});
  if (c.calls != (c.name == "render" ? 2 : c.name == "load" ? 3 : 1))
    command_refuse();
  auto envelope = response->fields.at("value");
  if (!envelope || envelope->kind != value_kind::record || !envelope->fields.contains("version") ||
      envelope->fields.at("version")->kind != value_kind::integer || envelope->fields.at("version")->integer != 1 ||
      (text_field(envelope, "kind") != c.name && !(c.name == "render" && text_field(envelope, "kind") == "render_external")))
    command_refuse();
  if (c.name == "history" || c.name == "load") {
    if (!same(envelope, c.facts))
      command_refuse();
  } else if (c.name == "init" || c.name == "export") {
    if (c.name == "init")
      exact(c.facts, {"ledger", "workspace", "active_revision", "event"});
    else
      exact(c.facts, {"ledger", "revision", "source"});
    auto expected               = std::make_shared<value>(*c.facts);
    expected->fields["version"] = command_integer(1);
    expected->fields["kind"]    = command_text(c.name);
    if (!same(envelope, expected))
      command_refuse();
  } else if (text_field(envelope, "kind") == "render_external") {
    // An external shape adds its identity record beside the packet and manifest.
    exact(envelope, {"version", "kind", "packet", "manifest", "shape"});
  } else {
    exact(envelope, {"version", "kind", "packet", "manifest"});
  }
  auto encoded = codec_encode(envelope);
  if (!encoded)
    throw encoded.error().detail;
}
} // namespace
namespace {
diagnostic factory_native_diagnostic() {
  return diagnostic{"resource_limit", "Native conversion allocation exceeded", "native_bytes", memory_max};
}
diagnostic factory_instruction_diagnostic(std::size_t maximum) {
  return diagnostic{"resource_limit", "Lua instruction limit exceeded", "instructions", maximum};
}
diagnostic factory_lua_diagnostic(std::size_t maximum) {
  return diagnostic{"resource_limit", "Lua allocation exceeded", "lua_bytes", maximum};
}
/// Factory-owned cumulative sink composing declaration loading, admission and native capture with one invocation VM.
/// Before the VM exists charges accumulate in the pre_* counters; attach() folds them into the VM state and from then on
/// every call delegates to it. detach() makes later calls inert so receipts that outlive the VM never touch freed state.
/// Single-thread-confined.
struct factory_budget final : budget_sink {
  state*                          target     = nullptr; ///< VM state once attached; null before attach and after teardown.
  bool                            detached   = false;   ///< Set when the VM state was destroyed.
  std::size_t                     pre_native = 0, pre_instructions = 0, pre_lua = 0;
  std::expected<void, diagnostic> charge_native(std::size_t bytes) override {
    if (detached)
      return std::unexpected(factory_native_diagnostic());
    if (!target) {
      if (bytes > memory_max - pre_native)
        return std::unexpected(factory_native_diagnostic());
      pre_native += bytes;
      return {};
    }
    try {
      charge(*target, bytes);
    } catch (const diagnostic& error) {
      return std::unexpected(error);
    }
    return {};
  }
  std::expected<void, diagnostic> charge_instructions(std::size_t count) override {
    if (detached)
      return std::unexpected(factory_native_diagnostic());
    if (!target) {
      if (pre_instructions >= instruction_max || count >= instruction_max - pre_instructions)
        return std::unexpected(factory_instruction_diagnostic(instruction_max));
      pre_instructions += count;
      return {};
    }
    auto& s = *target;
    if (!s.command || !s.command->invocation_started) {
      if (s.carried_instructions >= instruction_max || count >= instruction_max - s.carried_instructions)
        return std::unexpected(factory_instruction_diagnostic(instruction_max));
      s.carried_instructions += count;
      return {};
    }
    s.instructions += count;
    if (s.instructions >= s.instruction_limit)
      return std::unexpected(factory_instruction_diagnostic(s.instruction_limit));
    return {};
  }
  std::expected<void, diagnostic> acquire_lua(std::size_t bytes) override {
    if (detached)
      return std::unexpected(factory_native_diagnostic());
    if (!target) {
      if (bytes > memory_max - pre_lua)
        return std::unexpected(factory_lua_diagnostic(memory_max));
      pre_lua += bytes;
      return {};
    }
    auto& s = *target;
    if (bytes > s.memory_limit - s.memory)
      return std::unexpected(factory_lua_diagnostic(s.memory_limit));
    s.memory += bytes;
    return {};
  }
  void release_lua(std::size_t bytes) noexcept override {
    if (detached)
      return;
    if (!target)
      pre_lua -= std::min(bytes, pre_lua);
    else
      target->memory -= std::min(bytes, target->memory);
  }
};
/// Fold the pre-VM charges into the VM state and delegate from now on. May throw the checked native diagnostic.
void attach_budget(const std::shared_ptr<factory_budget>& budget, state& s) {
  if (budget->pre_native)
    charge(s, budget->pre_native);
  s.carried_instructions += budget->pre_instructions;
  s.memory += budget->pre_lua;
  budget->pre_native = budget->pre_instructions = budget->pre_lua = 0;
  budget->target                                                  = &s;
  s.budget                                                        = budget;
}
void detach_budget(state& s) noexcept {
  if (s.budget) {
    s.budget->target   = nullptr;
    s.budget->detached = true;
  }
}
} // namespace
/// Stable invocation owner; its allocator context outlives the Lua state.
struct command_vm::implementation {
  state         runtime;       ///< Cumulative resource and conversion ownership.
  command_state command;       ///< Sealed catalog, callable references and service facts.
  lua_State*    lua = nullptr; ///< Owned Lua state, closed on terminal invocation or destruction.
  implementation() {
    runtime.command      = &command;
    runtime.memory_limit = 64 * 1024 * 1024;
#if PLANAR_RUNTIME_REGISTRATION_INSTRUCTION_MAX == 10000000
    runtime.instruction_limit = 10000000;
#else
    runtime.instruction_limit = PLANAR_RUNTIME_REGISTRATION_INSTRUCTION_MAX;
#endif
  }
  ~implementation() {
    if (lua)
      lua_close(lua);
    detach_budget(runtime);
  }
};
namespace {
void check_profile(const profile_closure& profile) {
  auto paths = decode_profile(profile.profile.bytes);
  if (!paths)
    throw registration_error{paths.error().detail, paths.error().field};
  auto mode_ok = [](const captured_file& f) { return f.mode == "0644" || f.mode == "0755"; };
  if (profile.profile.root != "profile" || profile.profile.path != "profile.json" || !mode_ok(profile.profile) ||
      paths->size() != profile.modules.size())
    throw external_protocol();
  std::size_t remaining = 8388608;
  for (std::size_t i = 0; i < paths->size(); ++i) {
    auto& f = profile.modules[i];
    if (f.bytes.size() > 1048576 || f.bytes.size() > remaining)
      throw registration_error{external_resource(), std::format("profile.modules[{}]", i + 1)};
    remaining -= f.bytes.size();
    if (f.root != "modules" || f.path != (*paths)[i] || !mode_ok(f) || !valid_text(f.bytes) ||
        (!f.bytes.empty() && static_cast<unsigned char>(f.bytes.front()) == 0x1b))
      throw registration_error{external_protocol(), std::format("profile.modules[{}]", i + 1)};
  }
}
value_ptr external_owner(std::string tier, std::string module, std::size_t ordinal) {
  return command_record({{"tier", command_text(std::move(tier))},
                         {"module", command_text(std::move(module))},
                         {"ordinal", command_integer(static_cast<std::int64_t>(ordinal))}});
}
// Only already-owned native values are handled here; this function makes no Lua calls.
value_ptr profile_catalog(command_state& c) {
  std::array<std::vector<value_ptr>, 3> lists;
  auto                                  shipped = embedded::originals();
  std::size_t                           ordinal = 0;
  for (std::size_t i = 0; i < shipped.size(); ++i)
    if (shipped[i].path == "planar/commands.lua")
      ordinal = i + 1;
  for (auto& e : c.entries)
    lists[0].push_back(command_record(
        {{"descriptor", e.descriptor}, {"owner", external_owner("shipped", "shipped/planar/commands.lua", ordinal)}}));
  for (auto& e : c.external_entries) {
    if (!e.descriptor)
      continue;
    lists[e.kind].push_back(
        command_record({{"descriptor", e.descriptor},
                        {"owner", external_owner("external", "external/" + c.profile->modules[e.ordinal - 1].path, e.ordinal)}}));
  }
  for (std::size_t i = 0; i < lists.size(); ++i)
    std::ranges::sort(lists[i], {}, [i](auto& e) { return text_field(e->fields.at("descriptor"), i == 0 ? "name" : "id"); });
  return command_record({{"version", command_integer(2)},
                         {"commands", command_array(std::move(lists[0]))},
                         {"shapes", command_array(std::move(lists[1]))},
                         {"projections", command_array(std::move(lists[2]))}});
}
void validate_profile_catalog(state& runtime) {
  auto& c = *runtime.command;
  // Catalog construction/validation has no selected entry until the pure error
  // identifies one. Never inherit the last successfully collected descriptor.
  c.pending_seal_location = {};
  std::vector<std::string_view> shipped, external;
  for (auto& f : embedded::originals())
    shipped.push_back(f.path);
  for (auto& f : c.profile->modules)
    external.push_back(f.path);
  charge(runtime, 4096 * (c.entries.size() + c.external_entries.size() + 1));
  auto catalog = profile_catalog(c);
  // Charge structural conversion work and bounded canonical encode/decode buffers
  // before pure validation allocates. These reservations accumulate across modules.
  validate(runtime, catalog, 1);
  // encode result, validated_catalog's owned bytes, then retained runtime bytes.
  charge(runtime, 3 * 1048576);
  auto checked = history::validate_catalog(catalog, shipped, external);
  if (!checked) {
    auto code = checked.error().code;
    // Read the owned pure error in place. Prefix recognition, parsing, pointer
    // matching and original-index selection perform no allocation or string copy.
    // A canonical entry is not an original location until it has been matched.
    const auto&                               field = checked.error().field;
    constexpr std::array<std::string_view, 3> groups{"commands", "shapes", "projections"};
    constexpr std::array<std::string_view, 3> prefixes{"catalog.commands[", "catalog.shapes[", "catalog.projections["};
    bool                                      canonical_entry = false;
    for (std::size_t kind = 0; kind < groups.size(); ++kind) {
      auto prefix = prefixes[kind];
      if (!field.starts_with(prefix))
        continue;
      canonical_entry = true;
      auto digits     = std::string_view(field).substr(prefix.size());
      if (digits.size() < 2 || digits.back() != ']')
        break;
      std::size_t index       = 0;
      auto        parsed      = std::from_chars(digits.data(), digits.data() + digits.size() - 1, index);
      auto        group_entry = std::ranges::find_if(catalog->fields, [&](auto& pair) { return pair.first == groups[kind]; });
      auto&       items       = group_entry->second->items;
      if (parsed.ec != std::errc{} || parsed.ptr != digits.data() + digits.size() - 1 || index == 0 || index > items.size())
        break;
      auto descriptor     = std::ranges::find_if(items[index - 1]->fields, [](auto& pair) { return pair.first == "descriptor"; });
      auto target         = descriptor->second;
      std::size_t ordinal = kind == 0 ? command_catalog_size : 0;
      for (auto& entry : c.external_entries) {
        if (entry.kind != kind)
          continue;
        ++ordinal;
        if (entry.descriptor == target) {
          c.pending_seal_location = {kind, ordinal};
          break;
        }
      }
      break;
    }
    // Only non-entry pure fields already name a truthful global boundary. Move
    // their owned storage; unmatched canonical entries retain the caller boundary.
    if (!canonical_entry)
      c.registration_field = std::move(checked.error().field);
    if (code == history::catalog_error_code::identity_conflict)
      throw diagnostic{"reference", "External registration identity conflict"};
    if (code == history::catalog_error_code::resource_limit)
      throw external_resource();
    throw external_protocol();
  }
  c.catalog                 = checked->data();
  c.effective_catalog_bytes = checked->bytes();
}
int collect_final_descriptors(lua_State* l) {
  if (!lua_checkstack(l, 2048)) {
    context(l).stack_exhausted = true;
    return 0;
  }
  bool failed = false;
  try {
    auto&                      c = commands(l);
    std::array<std::size_t, 3> indices{command_catalog_size, 0, 0};
    for (auto& e : c.external_entries) {
      ++indices[e.kind];
      if (e.ordinal != c.active_module)
        continue;
      c.pending_seal_location = {e.kind, indices[e.kind]};
      lua_rawgeti(l, LUA_REGISTRYINDEX, e.descriptor_reference);
      auto d = convert(l, -1, 1);
      lua_pop(l, 1);
      // Sorting must not inspect an unchecked key after the original field advances.
      // Pure validation still owns spelling, schema and identity policy.
      static_cast<void>(text_field(d, e.kind == 0 ? "name" : "id"));
      if (e.kind == 0) {
        exact(d, {"name", "family", "handler_id", "options", "help", "response_kind", "capabilities"});
        // Normalize copies, never change the retained Lua descriptor during initialization.
        auto normalized = std::make_shared<value>(*d);
        auto options = d->fields.at("options"), caps = d->fields.at("capabilities");
        if (options->kind != value_kind::list || caps->kind != value_kind::list)
          throw external_protocol();
        if (options->items.size() > 32 || caps->items.size() > 3)
          throw external_resource();
        auto sorted_options = options->items, sorted_caps = caps->items;
        std::ranges::sort(sorted_options, {}, [](auto& o) { return text_field(o, "name"); });
        for (auto& cap : sorted_caps)
          if (cap->kind != value_kind::string)
            throw external_protocol();
        std::ranges::sort(sorted_caps, {}, [](auto& cap) { return cap->string; });
        normalized->fields["options"]      = command_array(std::move(sorted_options));
        normalized->fields["capabilities"] = command_array(std::move(sorted_caps));
        d                                  = std::move(normalized);
      }
      e.descriptor = std::move(d);
    }
    validate_profile_catalog(context(l));
  } catch (diagnostic& e) {
    if (e.code == "resource_limit" || e.code == "reference")
      command_failure(l, std::move(e));
    else
      command_failure(l, external_protocol());
    failed = true;
  } catch (const std::bad_alloc&) {
    command_failure(l, external_resource());
    failed = true;
  } catch (...) {
    command_failure(l, external_protocol());
    failed = true;
  }
  if (failed)
    return luaL_error(l, "external_descriptor");
  return 0;
}
int execute_profile_module(lua_State* l) {
  auto&       c = commands(l);
  const auto& f = c.profile->modules[c.active_module - 1];
  // pending owns the logical source name before entering this protected frame.
  if (luaL_loadbufferx(l, f.bytes.data(), f.bytes.size(), c.name.c_str(), "t"))
    return lua_error(l);
  lua_rawgeti(l, LUA_REGISTRYINDEX, external_environment_index);
  if (!lua_setupvalue(l, -2, 1))
    return luaL_error(l, "external_environment");
  lua_call(l, 0, LUA_MULTRET);
  if (lua_gettop(l) != 0)
    return luaL_error(l, "external_return");
  return 0;
}
int seal_profile_module(lua_State* l) {
  if (context(l).seal_failed)
    return luaL_error(l, "seal_owner_failed");
  if (!module_seal_location(l, std::nullopt)) {
    context(l).seal_failed = true;
    return 0;
  }
  if (!lua_checkstack(l, closure_guard::audit_stack_slots)) {
    context(l).seal_failed     = true;
    context(l).stack_exhausted = true;
    return 0;
  }
  std::array<int, 384> roots{};
  std::size_t          count = 0;
  auto&                c     = commands(l);
  for (auto& e : c.external_entries) {
    if (e.ordinal != c.active_module)
      continue;
    lua_rawgeti(l, LUA_REGISTRYINDEX, e.descriptor_reference);
    roots[count++] = lua_gettop(l);
    lua_rawgeti(l, LUA_REGISTRYINDEX, e.function_reference);
    roots[count++] = lua_gettop(l);
  }
  return seal_roots(l, std::span<const int>(roots.data(), count), false, true);
}
void register_profile(command_vm::implementation& owner) {
  auto& c    = owner.command;
  auto& s    = owner.runtime;
  auto  l    = owner.lua;
  auto  call = [&](lua_CFunction fn) {
    const bool sealing = fn == seal_profile_module;
    // Preserve C03's separately staged module fallback before protected sealing.
    // Registration and collection now select tokens without changing this text.
    // Terminal retries must retain the original failure location and token.
    if (sealing && !s.seal_failed) {
      c.registration_field.swap(c.module_locations[c.active_module - 1].fallback);
      c.pending_seal_location = {};
    }
    try {
      lua_pushcfunction(l, fn);
      auto status = lua_pcall(l, 0, 0, 0);
      if (exhausted(s) || status == LUA_ERRMEM)
        throw command_resource(s);
      if (c.bridge_error)
        throw *c.bridge_error;
      if (status || s.seal_failed)
        throw external_protocol();
    } catch (...) {
      // Covers every adapter, including native exceptions. A rendering
      // bad_alloc becomes the existing resource refusal with module fallback.
      render_seal_location(c);
      throw;
    }
    c.pending_seal_location = {};
  };
  // At most 64 modules: each stage owns two strings and two independent
  // NUL-terminated 19-byte maximum fields (profile.modules[64]). Reserve a
  // conservative 32-byte payload per string in addition to vector objects.
  // Charge before staging and publish only after every allocation succeeds,
  // while registration_field still identifies profile. This is logical native
  // allocation accounting, not an allocator-capacity or process RSS bound.
  charge(s, c.profile->modules.size() * (sizeof(module_fields) + 2 * 32));
  std::vector<module_fields> locations;
  locations.reserve(c.profile->modules.size());
  for (std::size_t i = 0; i < c.profile->modules.size(); ++i)
    locations.push_back({std::format("profile.modules[{}]", i + 1), std::format("profile.modules[{}]", i + 1)});
  c.module_locations = std::move(locations);
  // Final provenance work needs its own global fallback. Stage it under profile
  // before modules run; installing it later cannot expose the previous module if
  // string preparation refuses. Charge object and independent payload first.
  charge(s, sizeof(std::string) + 32);
  std::string catalog_location = "catalog";
  validate_profile_catalog(context(l));
  for (std::size_t i = 0; i < c.profile->modules.size(); ++i) {
    c.active_module = i + 1;
    c.registration_field.swap(c.module_locations[i].active);
    c.name        = "external/" + c.profile->modules[i].path;
    c.registering = true;
    call(execute_profile_module);
    c.registering = false;
    call(collect_final_descriptors);
    call(seal_profile_module);
    c.active_module = 0;
  }
  c.registering = false;
  c.registration_field.swap(catalog_location);
  if (c.originals.size() > 4094 || c.assembly.size() > 16 * 1048576)
    throw external_resource();
  for (auto& file : c.originals)
    if (file.path.size() > 1024 || file.bytes.size() > 16 * 1048576)
      throw external_resource();
  // Four pure tree digests touch embedded payloads: initial prepare; verify's
  // first digest; verify's nested prepare; final effective digest. Each constructs
  // a file record and encoded digest buffer. Three owned trees add three copies:
  // 4*2+3 = 11 embedded payload passes. External/profile use final digest+owner: 3.
  // These are charged logical capacities; allocator headers/growth remain library
  // overhead, not an RSS promise. Graph/scratch reservations are separate below.
  auto reserve = [&](std::size_t bytes, std::size_t passes) {
    if (passes != 0 && bytes > memory_max / passes)
      throw external_resource();
    charge(s, bytes * passes);
  };
  constexpr auto file_framing = std::string_view("PLANAR-RECORD\x01").size() + 1 + 4 + 8;
  // Six distinct final byte constructions: decoded catalog ownership; combined
  // manifest encoding; effective.bin record payload; encoded file record; owned
  // effective.bin; catalog+digest seal. The last two additions have separate room.
  reserve(1048576, 6);
  reserve(file_framing + 32, 1);
  // canonical_v1 metadata: map/list headers, four encoded keys, version integer,
  // scheme and separator string headers; each original adds its string header.
  // Derive from immutable inputs, independently of the six final buffer slots.
  std::size_t metadata     = 9 + (9 + 7) + 9 + (9 + 6) + (9 + 17) + (9 + 5) + 9 + (9 + 9) + 9;
  auto        metadata_add = [&](std::size_t bytes) {
    if (bytes > 16 * 1048576 - metadata)
      throw external_resource();
    metadata += bytes;
  };
  metadata_add(c.separator.size());
  for (auto& file : c.originals)
    metadata_add(9 + file.path.size());
  // Two metadata encodes plus the same eleven digest/owned payload passes.
  reserve(metadata, 13);
  // Two constructed metadata graphs and two decoded graphs, including codec
  // traversal scratch and original-order metadata. Separator strings pay per graph.
  reserve((c.originals.size() + 16) * 4096, 4);
  reserve(c.separator.size(), 4);
  // All four embedded digests frame each original, assembly and metadata record.
  reserve(file_framing, 4 * (c.originals.size() + 2));
  // Final digest frames each profile/module record (effective.bin paid above).
  reserve(file_framing, c.profile->modules.size() + 1);
  validate(s, c.catalog, 1); // pure effective catalog decode and manifest graph
  auto tree_charge = [&](std::string_view path, std::size_t passes) {
    auto components = static_cast<std::size_t>(std::ranges::count(path, '/')) + 2;
    // Per component: tree vector/node, child map, directory record and encoded
    // child; includes folded/name strings and vector growth in pinned libraries.
    charge(s, passes * components * 2048);
    charge(s, passes * (path.size() + 32) * 8);
  };
  charge(s, c.assembly.size() * 11);
  tree_charge("__planar__/embedded.lua", 4);
  tree_charge("__planar__/embedded.bin", 4);
  for (auto& file : c.originals) {
    charge(s, file.bytes.size() * 11);
    tree_charge(file.path, 4);
  }
  charge(s, c.profile->profile.bytes.size() * 3);
  tree_charge("profile/input.json", 1);
  tree_charge("__planar__/effective.bin", 1);
  charge(s, (c.profile->modules.size() + c.originals.size() + 16) * 4096);
  for (auto& file : c.profile->modules) {
    charge(s, file.bytes.size() * 3);
    tree_charge(file.path, 1);
  }
  auto embedded = history::prepare_stdlib(c.originals, c.separator, c.assembly);
  if (!embedded) {
    if (embedded.error().code == history::provenance_error_code::resource_limit)
      throw external_resource();
    throw external_protocol();
  }
  std::vector<history::tree_file_view> modules;
  for (auto& f : c.profile->modules)
    modules.push_back({f.path, f.bytes, f.mode == "0644" ? 0644u : 0755u});
  auto effective =
      history::prepare_effective_stdlib({embedded->files, c.profile->profile.bytes,
                                         c.profile->profile.mode == "0644" ? 0644u : 0755u, modules, c.effective_catalog_bytes});
  if (!effective) {
    if (effective.error().code == history::effective_error_code::resource_limit)
      throw external_resource();
    throw external_protocol();
  }
  for (auto b : effective->digest)
    c.effective_digest_bytes += std::format("{:02x}", b);
  c.provenance = std::move(effective->files);
  c.seal       = c.effective_catalog_bytes + c.effective_digest_bytes;
}
} // namespace
command_vm::command_vm(std::unique_ptr<implementation> owner) : _owner(std::move(owner)) {
}
command_vm::command_vm(command_vm&&) noexcept            = default;
command_vm& command_vm::operator=(command_vm&&) noexcept = default;
command_vm::~command_vm()                                = default;
value_ptr command_vm::catalog() const {
  return _owner ? _owner->command.catalog : value_ptr{};
}
std::string_view command_vm::seal() const {
  return _owner ? std::string_view(_owner->command.seal) : std::string_view{};
}
bool command_vm::export_may_be_complete() const {
  return _owner && _owner->command.export_published;
}
const ledger::completion_receipt* command_vm::load_completion() const {
  return _owner ? &_owner->command.completion : nullptr;
}
bool command_vm::load_may_be_complete() const {
  return _owner && _owner->command.completion.phase != ledger::completion_phase::not_completed;
}
command_service_result command_services::open_workspace(const command_load_options&) {
  command_refuse();
}
command_service_result command_services::capture_load(const command_load_options&, const value_ptr&) {
  command_refuse();
}
command_service_result command_services::accept_load(const command_load_options&, const value_ptr&, const value_ptr&,
                                                     std::span<const history::tree_file>, ledger::completion_receipt&) {
  command_refuse();
}
std::expected<command_vm, diagnostic> command_vm::create() {
  auto result = create_impl({});
  if (!result)
    return std::unexpected(std::move(result.error().detail));
  return std::move(*result);
}
std::expected<command_vm, registration_error> command_vm::create(profile_snapshot profile) {
  if (!profile)
    return std::unexpected(registration_error{external_protocol(), "profile"});
  return create_impl(std::move(profile));
}
namespace {
/// Whether a profile-owner frame matches the VM that its owner began; no authority is derived from the pointer value.
bool profile_frame_valid(const command_vm::implementation& owner, const void* frame_owner) {
  const auto& c = owner.command;
  return frame_owner && c.profile && !c.registering && !c.shape_failure && !owner.runtime.seal_failed && c.invoked &&
         c.profile_frame_owner == frame_owner;
}
} // namespace
std::string_view command_vm::effective_catalog() const {
  return _owner ? std::string_view(_owner->command.effective_catalog_bytes) : std::string_view{};
}
std::string_view command_vm::effective_digest() const {
  return _owner ? std::string_view(_owner->command.effective_digest_bytes) : std::string_view{};
}
std::expected<shape_definition, registration_error> command_vm::resolve_shape_definition(std::string_view id) {
  return resolve_shape_definition_impl(id, active_frame{});
}
std::expected<shape_definition, registration_error> command_vm::resolve_shape_definition_impl(std::string_view    id,
                                                                                              const active_frame& frame) {
  extension_invocation* const active  = frame.extension;
  auto                        failure = [&](diagnostic e) {
    if (_owner && _owner->command.shape_failure)
      return std::unexpected(*_owner->command.shape_failure);
    registration_error error{std::move(e), "catalog.shapes"};
    // Lookup and invocation share the profile owner's first terminal diagnostic.
    // Move ownership before the fallible response copy so adaptation can retry it.
    if (_owner && _owner->command.profile && error.detail.code == "resource_limit") {
      _owner->command.shape_failure = std::move(error);
      return std::unexpected(*_owner->command.shape_failure);
    }
    return std::unexpected(std::move(error));
  };
  auto allocation_failure = [&] {
    if (_owner && _owner->command.shape_failure)
      return std::unexpected(*_owner->command.shape_failure);
    if (!_owner)
      return failure(external_resource());
    auto& s = _owner->runtime;
    if (!s.conversion_failure)
      s.conversion_failure = external_resource();
    return failure(*s.conversion_failure);
  };
  // The outer boundary also protects diagnostic construction and response copies
  // performed by the inner exception adapters.
  try {
    try {
      if (_owner && _owner->command.shape_failure)
        return std::unexpected(*_owner->command.shape_failure);
      if (!_owner || !_owner->command.profile || _owner->runtime.seal_failed || _owner->command.registering ||
          (_owner->command.invoked && !frame.nested()))
        return failure(external_protocol());
      if (frame.profile_owner && !profile_frame_valid(*_owner, frame.profile_owner))
        return failure(external_protocol());
      if (active && (_owner->command.extension_history.owner != active || !_owner->command.extension_history.role_enabled ||
                     !_owner->command.extension_history.context_identity || !_owner->command.extension_execution ||
                     !_owner->command.invoked || _owner->command.extension_history.refused))
        return failure(external_protocol());
      auto& s = _owner->runtime;
      auto& c = _owner->command;
      if (exhausted(s))
        return failure(command_resource(s));
      const external_entry* selected = nullptr;
      for (auto& entry : c.external_entries)
        if (entry.kind == 1 && text_field(entry.descriptor, "id") == id) {
          selected = &entry;
          break;
        }
      if (!selected || selected->ordinal == 0 || selected->ordinal > c.profile->modules.size())
        return failure(external_protocol());
      const auto&                d           = selected->descriptor;
      const auto&                source      = c.profile->modules[selected->ordinal - 1];
      const auto&                name        = text_field(d, "id");
      const auto&                rev         = text_field(d, "revision");
      constexpr std::string_view domain      = "shape_definition_v1";
      const auto                 module_size = std::string_view("external/").size() + source.path.size();
      const auto                 hex_size    = source.bytes.size() * 2; // Captured modules are bounded to 1 MiB.
      // Final shape D is the validated flat five-string descriptor. It is borrowed,
      // never copied into a writable graph. Derive canonical byte capacity directly
      // from canonical.cpp: record/string headers are nine bytes, keys are strings.
      std::size_t descriptor_bytes = 9;
      for (const auto& [key, child] : d->fields)
        descriptor_bytes += 9 + key.size() + 9 + child->string.size();
      const auto owner_bytes = 9 + (9 + 4) + (9 + 8) + (9 + 6) + (9 + module_size);
      const auto input_bytes = 9 + (9 + 10) + descriptor_bytes + (9 + 12) + (9 + hex_size) + (9 + 5) + owner_bytes;
      const auto frame_bytes = 9 + (9 + 6) + (9 + 12) + (9 + domain.size());
      // Nine new graph nodes (input/owner/three strings and hash envelope/three
      // strings), six borrowed D nodes, and ten encoded map keys. Existing visit's
      // 512-byte node slots cover value/control blocks, encoder active-set nodes
      // and sorted-pointer scratch; five new map entries pay their separate slots.
      for (std::size_t i = 0; i < 25; ++i)
        visit(s, 1);
      charge(s, 5 * 128);
      // Two HEX payloads: staging and graph string. Two logical module paths:
      // staging and graph string. Envelope text, map keys and tier are new copies.
      charge(s, hex_size * 2 + module_size * 2 + 8 + 6 + 12 + domain.size() + 10 + 12 + 5 + 4 + 6);
      charge(s, input_bytes + frame_bytes);     // One canonical output; XXH3 reads it directly.
      charge(s, 41 + name.size() + rev.size()); // Hash result moves; returned ID/revision copy.
      std::string hex;
      hex.reserve(hex_size);
      constexpr std::string_view digits = "0123456789abcdef";
      for (unsigned char byte : source.bytes) {
        hex += digits[byte >> 4];
        hex += digits[byte & 15];
      }
      auto module = "external/" + source.path;
      auto input =
          command_record({{"descriptor", d},
                          {"owner", command_record({{"tier", command_text("external")}, {"module", command_text(module)}})},
                          {"module_bytes", command_text(hex)}});
      auto digest = hash_value(domain, input);
      if (!digest) {
        if (digest.error().code == "resource_limit") {
          s.conversion_failure = std::move(digest.error());
          return failure(*s.conversion_failure);
        }
        return failure(external_protocol());
      }
      return shape_definition{name, rev, std::move(*digest)};
    } catch (diagnostic& e) {
      if (e.code == "resource_limit")
        return failure(std::move(e));
      return failure(external_protocol());
    } catch (const std::bad_alloc&) {
      return allocation_failure();
    } catch (const std::length_error&) {
      return allocation_failure();
    } catch (...) {
      return failure(external_protocol());
    }
  } catch (const std::bad_alloc&) {
    return allocation_failure();
  } catch (const std::length_error&) {
    return allocation_failure();
  }
}
std::expected<projection_definition, registration_error> command_vm::resolve_projection_definition(std::string_view id) {
  return resolve_projection_definition_impl(id, 0);
}
std::expected<projection_definition, registration_error>
command_vm::resolve_projection_definition_impl(std::string_view id, std::size_t original_index, const active_frame& frame,
                                               registration_error* active_failure) {
  extension_invocation* const active  = frame.extension;
  auto                        failure = [&](diagnostic e) {
    if (_owner && _owner->command.shape_failure)
      return std::unexpected(*_owner->command.shape_failure);
    registration_error error{std::move(e), active_failure   ? std::move(active_failure->field)
                                           : original_index ? std::format("catalog.projections[{}]", original_index)
                                                            : "catalog.projections"};
    // Lookup and invocation share the profile owner's first terminal diagnostic.
    // Move ownership before the fallible response copy so adaptation can retry it.
    if (_owner && _owner->command.profile && (active || error.detail.code == "resource_limit")) {
      _owner->command.shape_failure = std::move(error);
      return std::unexpected(*_owner->command.shape_failure);
    }
    return std::unexpected(std::move(error));
  };
  auto allocation_failure = [&] {
    if (_owner && _owner->command.shape_failure)
      return std::unexpected(*_owner->command.shape_failure);
    if (!_owner)
      return failure(external_resource());
    auto& s = _owner->runtime;
    if (active_failure) {
      if (s.conversion_failure)
        active_failure->detail = std::move(*s.conversion_failure);
      _owner->command.shape_failure.emplace(std::move(*active_failure));
      return std::unexpected(*_owner->command.shape_failure);
    }
    if (!s.conversion_failure)
      s.conversion_failure = external_resource();
    return failure(*s.conversion_failure);
  };
  // The outer boundary also protects diagnostic construction and response copies
  // performed by the inner exception adapters.
  try {
    try {
      if (_owner && _owner->command.shape_failure)
        return std::unexpected(*_owner->command.shape_failure);
      if (!_owner || !_owner->command.profile || _owner->runtime.seal_failed || _owner->command.registering ||
          (_owner->command.invoked && !frame.nested()))
        return failure(external_protocol());
      if (frame.profile_owner && !profile_frame_valid(*_owner, frame.profile_owner))
        return failure(external_protocol());
      if (active) {
        const auto& bridge = _owner->command.extension_history;
        if (!active_failure || !bridge.enabled || !bridge.projection_enabled || bridge.owner != active ||
            !bridge.context_identity || !bridge.result || bridge.refused || !bridge.projection_attempted ||
            bridge.projection_result || !_owner->command.extension_execution || !_owner->command.invoked)
          return failure(external_protocol());
      }
      auto& s = _owner->runtime;
      auto& c = _owner->command;
      if (exhausted(s))
        return failure(command_resource(s));
      const external_entry* selected = nullptr;
      for (auto& entry : c.external_entries)
        if (entry.kind == 2 && text_field(entry.descriptor, "id") == id) {
          selected = &entry;
          break;
        }
      if (!selected || selected->ordinal == 0 || selected->ordinal > c.profile->modules.size())
        return failure(external_protocol());
      const auto&                d      = selected->descriptor;
      const auto&                source = c.profile->modules[selected->ordinal - 1];
      const auto&                name   = text_field(d, "id");
      const auto&                rev    = text_field(d, "revision");
      constexpr std::string_view domain = "projection_definition_v1";
      // Plan the prospective graph before creating any strings, maps or nodes.
      // Source/path are structurally bounded, but arithmetic stays explicitly checked.
      constexpr auto prefix = std::string_view("external/");
      if (source.path.size() > memory_max - prefix.size() || source.bytes.size() > memory_max / 2)
        return failure(external_resource());
      const auto module_size = prefix.size() + source.path.size();
      const auto hex_size    = source.bytes.size() * 2;
      // Same 25 logical value/key visits as the flat five-string definition graph.
      // Slots conservatively cover new value/control blocks, map nodes and encoder
      // scratch. Borrowed D's hash scratch is additionally reserved below, without
      // incrementing converted-node counts or copying the immutable descriptor.
      for (std::size_t i = 0; i < 25; ++i)
        visit(s, 1);
      charge(s, 5 * 128);
      reserve_hash_payload(s, d);
      reserve_hash_frame(s, domain.size());
      // canonical.cpp emits nine-byte map/string headers. Reserve all prospective
      // wrapper bytes using the pinned cumulative 4*N growth bound, not final size.
      reserve_hash_bytes(s, 9); // input map
      reserve_hash_bytes(s, 9); // owner map
      for (auto key : {std::string_view("descriptor"), std::string_view("owner"), std::string_view("module_bytes"),
                       std::string_view("tier"), std::string_view("module")}) {
        reserve_hash_bytes(s, 9);
        reserve_hash_bytes(s, key.size());
        reserve_hash_copy(s, key.size());
      }
      for (auto bytes : {std::size_t{8}, module_size, hex_size}) {
        reserve_hash_bytes(s, 9);
        reserve_hash_bytes(s, bytes);
      }
      // Staging reserve and graph copy each fit the empty-string assignment bound.
      // module is reserved once before appending, avoiding concatenation growth.
      for (int i = 0; i < 2; ++i) {
        reserve_hash_copy(s, hex_size);
        reserve_hash_copy(s, module_size);
      }
      reserve_hash_copy(s, 8); // external tier
      reserve_hash_copy(s, name.size());
      reserve_hash_copy(s, rev.size());
      std::string hex;
      hex.reserve(hex_size);
      constexpr std::string_view digits = "0123456789abcdef";
      for (unsigned char byte : source.bytes) {
        hex += digits[byte >> 4];
        hex += digits[byte & 15];
      }
      std::string module;
      module.reserve(module_size);
      module.append(prefix);
      module.append(source.path);
      auto input =
          command_record({{"descriptor", d},
                          {"owner", command_record({{"tier", command_text("external")}, {"module", command_text(module)}})},
                          {"module_bytes", command_text(hex)}});
      auto digest = hash_value(domain, input);
      if (!digest) {
        if (digest.error().code == "resource_limit") {
          s.conversion_failure = std::move(digest.error());
          return failure(*s.conversion_failure);
        }
        return failure(external_protocol());
      }
      return projection_definition{name, rev, std::move(*digest)};
    } catch (diagnostic& e) {
      if (e.code == "resource_limit")
        return failure(std::move(e));
      return failure(external_protocol());
    } catch (const std::bad_alloc&) {
      return allocation_failure();
    } catch (const std::length_error&) {
      return allocation_failure();
    } catch (...) {
      return failure(external_protocol());
    }
  } catch (const std::bad_alloc&) {
    return allocation_failure();
  } catch (const std::length_error&) {
    return allocation_failure();
  }
}
std::expected<extension_prepared_invocation, extension_preparation_error>
command_vm::prepare_extension_invocation(std::string_view name, std::span<const extension_parsed_option> options) {
  auto failure = [&](diagnostic detail) {
    registration_error error{std::move(detail), "catalog.commands"};
    if (_owner && _owner->command.shape_failure)
      error = *_owner->command.shape_failure;
    else if (_owner && _owner->command.profile && error.detail.code == "resource_limit") {
      _owner->command.shape_failure = std::move(error);
      error                         = *_owner->command.shape_failure;
    }
    auto kind = error.detail.code == "resource_limit" ? extension_preparation_failure::resource
                : error.detail.code == "usage"        ? extension_preparation_failure::usage
                                                      : extension_preparation_failure::protocol;
    return std::unexpected(extension_preparation_error{kind, std::move(error)});
  };
  auto allocation_failure = [&] {
    if (_owner && _owner->command.shape_failure)
      return failure(external_resource());
    if (_owner && !_owner->runtime.conversion_failure)
      _owner->runtime.conversion_failure = external_resource();
    return failure(_owner ? *_owner->runtime.conversion_failure : external_resource());
  };
  try {
    try {
      if (_owner && _owner->command.shape_failure)
        return failure(external_protocol());
      if (!_owner || !_owner->command.profile || _owner->runtime.seal_failed || _owner->command.registering ||
          _owner->command.invoked)
        return failure(external_protocol());
      auto& s = _owner->runtime;
      if (exhausted(s))
        return failure(command_resource(s));
      const external_entry* selected = nullptr;
      for (auto& entry : _owner->command.external_entries)
        if (entry.kind == 0 && text_field(entry.descriptor, "name") == name) {
          selected = &entry;
          break;
        }
      if (!selected || selected->function_reference == LUA_NOREF ||
          text_field(selected->descriptor, "family") != "extension_read_v1")
        return failure(external_protocol());
      auto& descriptor = selected->descriptor;
      auto& declared   = descriptor->fields.at("options")->items;
      auto  capability = [&](std::string_view id) {
        return std::ranges::any_of(descriptor->fields.at("capabilities")->items,
                                   [&](auto& value) { return value->string == id; });
      };
      const bool role = capability("role_context_read_v1"), history = capability("history_read_v1");
      auto       usage = [] { throw diagnostic{"usage", "Invalid extension invocation"}; };
      auto       find  = [&](std::string_view key) -> const extension_parsed_option* {
        for (auto& option : options)
          if (option.name == key)
            return &option;
        return nullptr;
      };
      if (options.size() > declared.size())
        usage();
      for (std::size_t i = 0; i < options.size(); ++i) {
        auto& option = options[i];
        for (std::size_t j = 0; j < i; ++j)
          if (options[j].name == option.name)
            usage();
        if (!std::ranges::any_of(declared,
                                 [&](auto& d) { return std::string_view(text_field(d, "name")).substr(2) == option.name; }) ||
            (!option.present && option.token))
          usage();
      }
      // Reserve complete vector capacity once. Every string copy below is separately
      // charged before allocation; inline object storage is conservatively included.
      charge(s, sizeof(extension_prepared_invocation));
      charge(s, declared.size() * sizeof(extension_prepared_option));
      charge(s, declared.size() * sizeof(extension_pending_host));
      auto copy = [&](std::string_view text) {
        charge(s, sizeof(std::string) + text.size() + 1);
        return std::string(text);
      };
      extension_prepared_invocation result;
      result.command = copy(name);
      result.options.reserve(declared.size());
      result.pending_hosts.reserve(declared.size());
      auto host_kind = [&](std::string_view key) -> std::optional<extension_host_kind> {
        if (history && key == "ledger")
          return extension_host_kind::ledger;
        if (!role)
          return {};
        if (key == "workspace")
          return extension_host_kind::workspace;
        if (key == "registry")
          return extension_host_kind::registry;
        if (key == "root")
          return extension_host_kind::root;
        if (key == "base-root")
          return extension_host_kind::base_root;
        if (key == "observations")
          return extension_host_kind::observations;
        return {};
      };
      for (auto& d : declared) {
        auto        key     = std::string_view(text_field(d, "name")).substr(2);
        const auto& type    = text_field(d, "type");
        auto        parsed  = find(key);
        bool        present = parsed && parsed->present;
        if (d->fields.at("required")->boolean && !present)
          usage();
        if (present && ((type == "flag") == parsed->token.has_value()))
          usage();
        auto                   token_value = present && parsed->token ? *parsed->token : std::string_view{};
        extension_option_value value       = false;
        if (type == "text") {
          if (present && (token_value.size() > 65536 || !token(token_value) || !valid_text(token_value)))
            usage();
          if (auto kind = host_kind(key)) {
            extension_slot_index slot;
            if (present) {
              slot.index = result.pending_hosts.size();
              result.pending_hosts.push_back({*kind, copy(token_value)});
            }
            value = slot;
          } else
            value = copy(token_value);
        } else if (type == "decimal" || type == "history_id") {
          std::int64_t number = 0;
          if (present) {
            if (token_value.size() > (type == "history_id" ? 19 : 10) || !decimal_id(token_value))
              usage();
            auto converted = std::from_chars(token_value.data(), token_value.data() + token_value.size(), number);
            if (converted.ec != std::errc{} || converted.ptr != token_value.data() + token_value.size())
              usage();
          }
          if (type == "history_id")
            value = copy(present ? token_value : "0");
          else {
            if (number > 2147483647 || (history && key == "limit" && number > 100) ||
                (role && key == "budget-bytes" && number > 1048576))
              usage();
            value = static_cast<std::int32_t>(number);
          }
        } else if (type == "flag")
          value = present;
        else
          return failure(external_protocol());
        result.options.push_back({copy(key), present, std::move(value)});
      }
      auto prepared = [&](std::string_view key) -> const extension_prepared_option& {
        for (auto& option : result.options)
          if (option.name == key)
            return option;
        throw external_protocol();
      };
      if (history) {
        auto& revision  = prepared("revision");
        auto& at        = prepared("at-event");
        auto& after     = prepared("after-event");
        auto& request   = prepared("request-id");
        auto& limit     = prepared("limit");
        auto  selectors = revision.present + at.present + request.present;
        if (selectors > 1 || (selectors && (after.present || limit.present)))
          usage();
        auto& request_text = std::get<std::string>(request.value);
        if (request.present && (request_text.size() > 128 || !std::ranges::all_of(request_text, [](unsigned char ch) {
                                  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') ||
                                         ch == '_' || ch == '.' || ch == ':' || ch == '-';
                                })))
          usage();
        result.history_query.emplace();
        auto& query      = *result.history_query;
        query.selector   = copy(revision.present ? "revision" : at.present ? "state" : request.present ? "request" : "page");
        query.id         = copy(std::get<std::string>(revision.present ? revision.value : at.present ? at.value : after.value));
        query.request_id = copy(request_text);
        query.limit      = limit.present ? std::get<std::int32_t>(limit.value) : 50;
      }
      if (role) {
        auto& base       = prepared("base-root");
        auto& capture    = prepared("capture-outputs");
        auto  mapping_id = [&](const extension_prepared_option& option) {
          auto& selector = result.pending_hosts.at(*std::get<extension_slot_index>(option.value).index).selector;
          auto  equal    = selector.find('=');
          if (equal == 0 || equal == std::string::npos || equal + 1 == selector.size())
            usage();
          return std::string_view(selector).substr(0, equal);
        };
        auto root = mapping_id(prepared("root"));
        if (base.present != capture.present || (base.present && mapping_id(base) != root))
          usage();
      }
      return result;
    } catch (diagnostic& detail) {
      return failure(std::move(detail));
    } catch (const std::bad_alloc&) {
      return allocation_failure();
    } catch (const std::length_error&) {
      return allocation_failure();
    } catch (...) {
      return failure(external_protocol());
    }
  } catch (const std::bad_alloc&) {
    return allocation_failure();
  } catch (const std::length_error&) {
    return allocation_failure();
  }
}
std::expected<command_vm, registration_error> command_vm::create_impl(profile_snapshot profile) {
  std::unique_ptr<implementation> owner;
  try {
    // Preallocate a checked fixed charge for the added inline root metadata.
    // Both owner kinds contain these slots; unchanged bootstrap keeps its prior
    // accounting. The synchronous history bridge adds exactly its inline size,
    // verified against the pre-B3 and corrective compiled layouts and cumulative initial charge.
    // Retained phases expand the aligned inline root/function block.
    // Include the aligned function slot and tail padding in the fixed precharge.
    struct alignas(value_ptr) retained_phase_storage {
      decltype(command_state::shape_phases) phases;
      int                                   function;
    };
    constexpr auto root_metadata_bytes = sizeof(retained_phase_storage) - sizeof(std::array<int, 4>);
    static_assert(root_metadata_bytes <= memory_max);
    const auto initial_native_bytes = root_metadata_bytes + sizeof(extension_history_bridge);
    owner                           = std::make_unique<implementation>();
    auto& s                         = owner->runtime;
    s.native_bytes                  = initial_native_bytes;
    auto& c                         = owner->command;
    if (profile) {
      check_profile(*profile);
      charge(s, profile->profile.bytes.size() + sizeof(profile_closure));
      for (auto& f : profile->modules)
        charge(s, f.bytes.size() + f.path.size() + sizeof(captured_file));
      c.profile = std::make_shared<const profile_closure>(*profile);
    }
    int line = 1;
    for (auto& file : embedded::originals()) {
      int end = line + static_cast<int>(std::ranges::count(file.bytes, '\n'));
      if (file.path == "planar/commands.lua") {
        c.owner_first = line;
        c.owner_last  = end;
      }
      line = end + 1;
    }
    if (!c.owner_first)
      command_refuse();
    owner->lua = lua_newstate(allocate, &s, 0);
    if (!owner->lua)
      throw command_resource(s);
    auto l                                      = owner->lua;
    *static_cast<state**>(lua_getextraspace(l)) = &s;
    lua_sethook(l, hook, LUA_MASKCOUNT | LUA_MASKCALL | LUA_MASKRET, 1000);
    if (!lua_checkstack(l, 2048)) {
      s.stack_exhausted = true;
      throw command_resource(s);
    }
    lua_pushcfunction(l, setup_commands);
    auto status = lua_pcall(l, 0, 0, 0);
    if (exhausted(s) || status == LUA_ERRMEM)
      throw command_resource(s);
    if (c.bridge_error)
      throw *c.bridge_error;
    if (status)
      command_refuse();
    lua_pushcfunction(l, remember_engine_callbacks);
    auto native_status = lua_pcall(l, 0, 0, 0);
    if (exhausted(s) || native_status == LUA_ERRMEM)
      throw command_resource(s);
    if (native_status)
      command_refuse();
    seal_catalog(s);
    c.registering = false;
    lua_pushcfunction(l, seal_environment);
    status = lua_pcall(l, 0, 0, 0);
    if (exhausted(s) || status == LUA_ERRMEM)
      throw command_resource(s);
    if (c.bridge_error)
      throw *c.bridge_error;
    if (status)
      command_refuse();
    if (c.profile)
      register_profile(*owner);
    return command_vm(std::move(owner));
  } catch (registration_error& e) {
    return std::unexpected(std::move(e));
  } catch (diagnostic& e) {
    auto field = owner ? std::move(owner->command.registration_field) : std::string("profile");
    if (owner && owner->command.profile && e.code == "internal")
      e = external_protocol();
    return std::unexpected(registration_error{std::move(e), std::move(field)});
  } catch (const std::bad_alloc&) {
    return std::unexpected(
        registration_error{profile ? external_resource() : diagnostic{"internal", "Command registration failed"},
                           owner ? std::move(owner->command.registration_field) : std::string("profile")});
  } catch (...) {
    return std::unexpected(
        registration_error{profile ? external_protocol() : diagnostic{"internal", "Command registration failed"},
                           owner ? std::move(owner->command.registration_field) : std::string("profile")});
  }
}
std::expected<shape_response, registration_error> command_vm::invoke_shape(std::string_view id, const value_ptr& input) {
  return invoke_shape_impl(id, input, active_frame{});
}
std::expected<shape_response, registration_error> command_vm::invoke_shape_impl(std::string_view id, const value_ptr& input,
                                                                                const active_frame& frame) {
  std::size_t                       selected_ordinal = 0;
  bool                              executing        = false;
  std::optional<registration_error> active_failure;
  auto                              failure = [&](diagnostic detail) -> std::expected<shape_response, registration_error> {
    if (_owner && _owner->command.shape_failure)
      return std::unexpected(*_owner->command.shape_failure);
    if (active_failure) {
      active_failure->detail = std::move(detail);
      if (_owner && executing) {
        _owner->command.shape_failure.emplace(std::move(*active_failure));
        return std::unexpected(*_owner->command.shape_failure);
      }
      return std::unexpected(std::move(*active_failure));
    }
    registration_error error{std::move(detail),
                             selected_ordinal ? std::format("catalog.shapes[{}]", selected_ordinal) : "catalog.shapes"};
    if (_owner && executing) {
      if (!_owner->command.shape_failure)
        _owner->command.shape_failure = std::move(error);
      return std::unexpected(*_owner->command.shape_failure);
    }
    return std::unexpected(std::move(error));
  };
  auto allocation_failure = [&]() -> std::expected<shape_response, registration_error> {
    executing = true;
    if (_owner) {
      if (_owner->command.shape_failure)
        return std::unexpected(*_owner->command.shape_failure);
      if (active_failure) {
        _owner->command.shape_failure.emplace(std::move(*active_failure));
        return std::unexpected(*_owner->command.shape_failure);
      }
      if (exhausted(_owner->runtime))
        return failure(command_resource(_owner->runtime));
      if (_owner->command.bridge_error)
        return failure(*_owner->command.bridge_error);
    }
    return failure(external_resource());
  };
  // The outer boundary also protects diagnostic construction and response copies
  // performed by the inner exception adapters.
  try {
    try {
      if (!_owner)
        return failure(external_protocol());
      auto& s = _owner->runtime;
      auto& c = _owner->command;
      if (c.shape_failure)
        return std::unexpected(*c.shape_failure);
      const external_entry* selected = nullptr;
      std::size_t           ordinal  = 0;
      for (auto& entry : c.external_entries) {
        if (entry.kind != 1)
          continue;
        ++ordinal;
        if (text_field(entry.descriptor, "id") == id) {
          selected         = &entry;
          selected_ordinal = ordinal;
          break;
        }
      }
      if (!selected)
        return failure(external_protocol());
      if (frame.nested()) {
        charge(s, sizeof(registration_error) + 256);
        active_failure.emplace(registration_error{external_resource(), std::format("catalog.shapes[{}]", selected_ordinal)});
      }
      auto definition = resolve_shape_definition_impl(id, frame);
      if (!definition) {
        executing = definition.error().detail.code == "resource_limit";
        return failure(std::move(definition.error().detail));
      }
      if (!input || input->kind != value_kind::record || input->fields.size() != 5 || !input->fields.contains("version") ||
          !input->fields.contains("job") || !input->fields.contains("spec") || !input->fields.contains("assignments") ||
          !input->fields.contains("reviewer"))
        return failure(external_protocol());
      auto version = input->fields.at("version");
      if (!version || version->kind != value_kind::integer || version->integer != 1)
        return failure(external_protocol());
      // Structural admission is deliberately separate from trusted Lua shape policy.
      validate(s, input, 1);
      if (!frame.profile_owner)
        begin_invocation(s, c);
      executing = true;
      if (frame.nested()) {
        auto result = call_retained_value(_owner->lua, selected->function_reference, input);
        if (!result)
          return failure(std::move(result.error()));
        auto encoded = codec_encode_reserved(
            *result, {&s, [](void* owner, const codec_reservation& reservation) -> std::expected<void, diagnostic> {
                        try {
                          charge(*static_cast<state*>(owner), reservation.total_bytes);
                          return {};
                        } catch (diagnostic& error) {
                          return std::unexpected(std::move(error));
                        }
                      }});
        if (!encoded)
          return failure(encoded.error().detail.code == "resource_limit" ? std::move(encoded.error().detail)
                                                                         : external_protocol());
        return shape_response{std::move(*definition), std::move(*result)};
      }
      s.input          = input;
      c.shape_function = selected->function_reference;
      auto l           = _owner->lua;
      struct cleanup {
        implementation& owner;
        ~cleanup() {
          owner.runtime.preserving_aliases = false;
          owner.runtime.push_memo          = 0;
          owner.runtime.input.reset();
          release_shape_temporaries(owner.lua);
        }
      } release{*_owner};
      lua_pushcfunction(l, execute_shape);
      auto status = lua_pcall(l, 0, LUA_MULTRET, 0);
      if (c.bridge_error)
        return failure(*c.bridge_error);
      if (exhausted(s) || status == LUA_ERRMEM)
        return failure(command_resource(s));
      if (status || lua_gettop(l) != 1)
        return failure(external_protocol());
      auto result  = convert(l, 1, 1);
      auto encoded = codec_encode_reserved(
          result, {&s, [](void* owner, const codec_reservation& reservation) -> std::expected<void, diagnostic> {
                     try {
                       charge(*static_cast<state*>(owner), reservation.total_bytes);
                       return {};
                     } catch (diagnostic& error) {
                       return std::unexpected(std::move(error));
                     }
                   }});
      if (!encoded) {
        if (encoded.error().detail.code == "resource_limit")
          return failure(std::move(encoded.error().detail));
        return failure(external_protocol());
      }
      return shape_response{std::move(*definition), std::move(result)};
    } catch (diagnostic& error) {
      if (error.code == "resource_limit")
        executing = true;
      return failure(error.code == "resource_limit" ? std::move(error) : external_protocol());
    } catch (const std::bad_alloc&) {
      return allocation_failure();
    } catch (const std::length_error&) {
      return allocation_failure();
    } catch (...) {
      return failure(external_protocol());
    }
  } catch (const std::bad_alloc&) {
    return allocation_failure();
  } catch (const std::length_error&) {
    return allocation_failure();
  }
}
std::expected<projection_response, registration_error>
command_vm::invoke_projection(std::string_view id, projection_input_kind kind, const value_ptr& input) {
  std::size_t selected_ordinal = 0;
  bool        executing        = false;
  auto        failure          = [&](diagnostic detail) -> std::expected<projection_response, registration_error> {
    if (_owner && _owner->command.shape_failure)
      return std::unexpected(*_owner->command.shape_failure);
    registration_error error{std::move(detail),
                             selected_ordinal ? std::format("catalog.projections[{}]", selected_ordinal) : "catalog.projections"};
    if (_owner && executing) {
      if (!_owner->command.shape_failure)
        _owner->command.shape_failure = std::move(error);
      return std::unexpected(*_owner->command.shape_failure);
    }
    return std::unexpected(std::move(error));
  };
  auto allocation_failure = [&]() -> std::expected<projection_response, registration_error> {
    executing = true;
    if (_owner) {
      if (_owner->command.shape_failure)
        return std::unexpected(*_owner->command.shape_failure);
      if (exhausted(_owner->runtime))
        return failure(command_resource(_owner->runtime));
      if (_owner->command.bridge_error)
        return failure(*_owner->command.bridge_error);
    }
    return failure(external_resource());
  };
  // The outer boundary also protects diagnostic construction and response copies
  // performed by the inner exception adapters.
  try {
    try {
      if (!_owner)
        return failure(external_protocol());
      auto& s = _owner->runtime;
      auto& c = _owner->command;
      if (c.shape_failure)
        return std::unexpected(*c.shape_failure);
      const external_entry* selected = nullptr;
      std::size_t           ordinal  = 0;
      for (auto& entry : c.external_entries) {
        if (entry.kind != 2)
          continue;
        ++ordinal;
        if (text_field(entry.descriptor, "id") == id) {
          selected         = &entry;
          selected_ordinal = ordinal;
          break;
        }
      }
      if (!selected)
        return failure(external_protocol());
      auto definition = resolve_projection_definition_impl(id, selected_ordinal);
      if (!definition) {
        executing = definition.error().detail.code == "resource_limit";
        return failure(std::move(definition.error().detail));
      }
      const auto expected_kind = kind == projection_input_kind::role_context_v1       ? "role_context_v1"
                                 : kind == projection_input_kind::history_snapshot_v1 ? "history_snapshot_v1"
                                                                                      : "";
      if (!input || input->kind != value_kind::record || expected_kind[0] == '\0' ||
          text_field(selected->descriptor, "input") != expected_kind)
        return failure(external_protocol());
      // Structural admission is deliberately separate from trusted read-data policy.
      validate(s, input, 1);
      begin_invocation(s, c);
      executing = true;
      auto l    = _owner->lua;
      struct cleanup {
        implementation& owner;
        ~cleanup() {
          owner.runtime.preserving_aliases = false;
          owner.runtime.push_memo          = 0;
          owner.runtime.input.reset();
          release_shape_temporaries(owner.lua);
        }
      } release{*_owner};
      auto result = call_retained_projection(l, selected->function_reference, input);
      if (!result)
        return failure(std::move(result.error()));
      return projection_response{std::move(*definition), std::move(*result)};
    } catch (diagnostic& error) {
      if (error.code == "resource_limit")
        executing = true;
      return failure(error.code == "resource_limit" ? std::move(error) : external_protocol());
    } catch (const std::bad_alloc&) {
      return allocation_failure();
    } catch (const std::length_error&) {
      return allocation_failure();
    } catch (...) {
      return failure(external_protocol());
    }
  } catch (const std::bad_alloc&) {
    return allocation_failure();
  } catch (const std::length_error&) {
    return allocation_failure();
  }
}
std::expected<value_ptr, registration_error> command_vm::validate_profile(const value_ptr& validation) {
  return validate_profile_impl(validation, {});
}
std::expected<value_ptr, registration_error> command_vm::select_profile(const value_ptr& selection) {
  return validate_profile_impl({}, selection);
}
std::expected<value_ptr, registration_error> command_vm::render_profile(const value_ptr& render) {
  return validate_profile_impl({}, {}, render);
}
std::expected<value_ptr, registration_error> command_vm::validate_profile_impl(const value_ptr& original_validation,
                                                                               const value_ptr& original_selection,
                                                                               const value_ptr& render, const active_frame& frame,
                                                                               value_ptr* basis) {
  extension_invocation* const active    = frame.extension;
  bool                        executing = false;
  auto                        failure   = [&](diagnostic detail) -> std::expected<value_ptr, registration_error> {
    if (_owner && _owner->command.shape_failure)
      return std::unexpected(*_owner->command.shape_failure);
    registration_error error{std::move(detail), "catalog.shapes"};
    if (_owner && executing) {
      _owner->command.shape_failure = std::move(error);
      return std::unexpected(*_owner->command.shape_failure);
    }
    return std::unexpected(std::move(error));
  };
  auto allocation_failure = [&]() -> std::expected<value_ptr, registration_error> {
    executing = true;
    if (_owner) {
      if (_owner->command.shape_failure)
        return std::unexpected(*_owner->command.shape_failure);
      if (exhausted(_owner->runtime))
        return failure(command_resource(_owner->runtime));
      if (_owner->command.bridge_error)
        return failure(*_owner->command.bridge_error);
    }
    return failure(external_resource());
  };
  try {
    try {
      if (_owner && _owner->command.shape_failure)
        return std::unexpected(*_owner->command.shape_failure);
      if (!_owner || !_owner->command.profile || _owner->command.registering || (_owner->command.invoked && !frame.nested()) ||
          _owner->runtime.seal_failed)
        return failure(external_protocol());
      if (frame.profile_owner)
        return failure(external_protocol()); // Profile frames enter only through validate_profile_phases.
      if (active && (_owner->command.extension_history.owner != active || !_owner->command.extension_history.role_enabled ||
                     !_owner->command.extension_history.context_identity || !_owner->command.extension_execution ||
                     !_owner->command.invoked || _owner->command.extension_history.refused))
        return failure(external_protocol());
      auto& s = _owner->runtime;
      auto& c = _owner->command;
      if (exhausted(s)) {
        executing = true;
        return failure(command_resource(s));
      }
      auto validation = original_validation;
      auto selection  = original_selection;
      if (selection) {
        if (selection->kind != value_kind::record || !selection->fields.contains("bundle") ||
            !selection->fields.contains("capture"))
          return failure(external_protocol());
        validate(s, selection, 1);
        visit(s, 1);
        charge(s, 256 + 6 + 7);
        validation = command_record({{"bundle", selection->fields.at("bundle")}, {"capture", selection->fields.at("capture")}});
      }
      if (!render) {
        if (!validation || validation->kind != value_kind::record)
          return failure(external_protocol());
        validate(s, validation, 1);
      }
      begin_invocation(s, c);
      executing = true;
      return validate_profile_phases(validation, selection, render, frame, basis);
    } catch (diagnostic& error) {
      if (error.code == "resource_limit")
        executing = true;
      return failure(error.code == "resource_limit" ? std::move(error) : external_protocol());
    } catch (const std::bad_alloc&) {
      return allocation_failure();
    } catch (const std::length_error&) {
      return allocation_failure();
    } catch (...) {
      return failure(external_protocol());
    }
  } catch (const std::bad_alloc&) {
    return allocation_failure();
  } catch (const std::length_error&) {
    return allocation_failure();
  }
}
std::expected<value_ptr, registration_error> command_vm::validate_profile_phases(value_ptr validation, value_ptr selection,
                                                                                 const value_ptr&    render,
                                                                                 const active_frame& frame, value_ptr* basis) {
  extension_invocation* const active    = frame.extension;
  const bool                  nested    = frame.nested();
  bool                        executing = true; // Every failure here follows the begun invocation frame.
  auto                        failure   = [&](diagnostic detail) -> std::expected<value_ptr, registration_error> {
    if (_owner && _owner->command.shape_failure)
      return std::unexpected(*_owner->command.shape_failure);
    registration_error error{std::move(detail), "catalog.shapes"};
    if (_owner && executing) {
      _owner->command.shape_failure = std::move(error);
      return std::unexpected(*_owner->command.shape_failure);
    }
    return std::unexpected(std::move(error));
  };
  auto allocation_failure = [&]() -> std::expected<value_ptr, registration_error> {
    executing = true;
    if (_owner) {
      if (_owner->command.shape_failure)
        return std::unexpected(*_owner->command.shape_failure);
      if (exhausted(_owner->runtime))
        return failure(command_resource(_owner->runtime));
      if (_owner->command.bridge_error)
        return failure(*_owner->command.bridge_error);
    }
    return failure(external_resource());
  };
  try {
    try {
      if (_owner && _owner->command.shape_failure)
        return std::unexpected(*_owner->command.shape_failure);
      if (!_owner || !_owner->command.profile || _owner->command.registering || _owner->runtime.seal_failed)
        return failure(external_protocol());
      if (frame.profile_owner && (render || !profile_frame_valid(*_owner, frame.profile_owner)))
        return failure(external_protocol());
      auto& s = _owner->runtime;
      auto& c = _owner->command;
      if (frame.profile_owner && exhausted(s))
        return failure(command_resource(s));
      struct semantic_scope {
        state& owner;
        ~semantic_scope() {
          owner.semantic_active = false;
        }
      } semantic_release{s};
      s.semantic_active = true;
      // Staging reservations precede every constructed node, map slot and text
      // copy. Borrowed subgraphs are charged by each protected transport pass.
      auto record = [&](std::initializer_list<std::pair<const std::string, value_ptr>> fields) {
        visit(s, 1);
        for (const auto& field : fields)
          charge(s, 128 + field.first.size());
        return command_record(std::map<std::string, value_ptr>(fields));
      };
      auto text = [&](std::string_view bytes) {
        visit(s, 1);
        charge(s, bytes.size());
        return command_text(std::string(bytes));
      };
      auto number = [&](std::int64_t n) {
        visit(s, 1);
        return command_integer(n);
      };
      auto array = [&](std::vector<value_ptr> items) {
        visit(s, 1);
        return command_array(std::move(items));
      };
      auto field = [](const value_ptr& object, std::string_view key) -> value_ptr {
        if (!object || object->kind != value_kind::record)
          throw external_protocol();
        auto it = std::ranges::find_if(object->fields, [&](const auto& entry) { return entry.first == key; });
        if (it == object->fields.end() || !it->second)
          throw external_protocol();
        return it->second;
      };
      auto phase = [&](std::size_t index, value_ptr input) -> value_ptr {
        if (nested) {
          auto response = call_retained_value(_owner->lua, c.shape_phases[index], input);
          if (!response)
            throw std::move(response.error());
          auto result = std::move(*response);
          if (!result || result->kind != value_kind::record || result->fields.size() != 1 ||
              (result->fields.contains("value") == result->fields.contains("error")))
            throw external_protocol();
          return result;
        }
        validate(s, input, 1);
        s.input          = std::move(input);
        c.shape_function = c.shape_phases[index];
        struct cleanup {
          implementation& owner;
          ~cleanup() {
            owner.runtime.preserving_aliases = false;
            owner.runtime.push_memo          = 0;
            owner.runtime.input.reset();
            release_shape_temporaries(owner.lua);
          }
        } release{*_owner};
        auto l = _owner->lua;
        lua_pushcfunction(l, execute_shape);
        auto status = lua_pcall(l, 0, LUA_MULTRET, 0);
        if (exhausted(s) || status == LUA_ERRMEM)
          throw command_resource(s);
        if (c.bridge_error)
          throw *c.bridge_error;
        if (status || lua_gettop(l) != 1)
          throw external_protocol();
        auto result = convert(l, 1, 1);
        if (!result || result->kind != value_kind::record || result->fields.size() != 1 ||
            (result->fields.contains("value") == result->fields.contains("error")))
          throw external_protocol();
        return result;
      };
      // Every ordinary render envelope shares the complete-response boundary.
      // Validation and selection retain their original transport behavior.
      auto finish = [&](value_ptr response) -> value_ptr {
        if (!render)
          return response;
        // Bound the complete render response, including manifest and envelope, using
        // the existing reserved codec. The temporary encoding moves and is discarded;
        // the native-owned result survives cleanup without a second graph copy.
        auto encoded = codec_encode_reserved(
            response, {&s, [](void* owner, const codec_reservation& reservation) -> std::expected<void, diagnostic> {
                         try {
                           charge(*static_cast<state*>(owner), reservation.total_bytes);
                           return {};
                         } catch (diagnostic& error) {
                           return std::unexpected(std::move(error));
                         }
                       }});
        if (!encoded) {
          if (encoded.error().detail.code == "resource_limit")
            throw std::move(encoded.error().detail);
          throw external_protocol();
        }
        return response;
      };
      value_ptr render_request;
      if (render) {
        auto normalized = phase(4, render);
        if (normalized->fields.contains("error"))
          return finish(normalized);
        auto intents   = field(normalized, "value");
        render_request = field(intents, "request");
        selection      = field(intents, "selection");
        validation     = record({{"bundle", field(selection, "bundle")}, {"capture", field(selection, "capture")}});
      }
      visit(s, 1);
      std::map<std::string, value_ptr> shape_fields;
      for (const auto& entry : c.external_entries) {
        if (entry.kind != 1)
          continue;
        auto& id = text_field(entry.descriptor, "id");
        charge(s, 128 + id.size());
        shape_fields.emplace(id, record({{"stages", field(entry.descriptor, "stages")}}));
      }
      auto shapes   = command_record(std::move(shape_fields));
      auto context  = record({{"dialect", text("declarations_shapes_v1")}, {"shapes", shapes}});
      auto envelope = phase(0, record({{"validation", validation}, {"context", context}}));
      if (envelope->fields.contains("error"))
        return finish(envelope);
      auto prepared = field(envelope, "value");
      auto requests = field(prepared, "requests");
      if (requests->kind != value_kind::list)
        throw external_protocol();
      charge(s, requests->items.size() * sizeof(value_ptr) * 2);
      std::vector<value_ptr> results, definitions;
      results.reserve(requests->items.size());
      definitions.reserve(requests->items.size());
      std::int64_t total = 0;
      for (const auto& request : requests->items) {
        auto index = field(request, "index"), job = field(request, "id"), id = field(request, "shape");
        if (index->kind != value_kind::integer || index->integer <= 0 || job->kind != value_kind::string ||
            id->kind != value_kind::string)
          throw external_protocol();
        auto response = invoke_shape_impl(id->string, field(request, "input"), frame);
        if (!response) {
          if (!c.shape_failure)
            c.shape_failure = std::move(response.error());
          return std::unexpected(*c.shape_failure);
        }
        ++c.shape_dispatch_count;
        auto checked = phase(1, record({{"request", request}, {"response", response->result}, {"total", number(total)}}));
        if (checked->fields.contains("error"))
          return finish(checked);
        auto result     = field(checked, "value");
        auto next_total = field(result, "total");
        if (next_total->kind != value_kind::integer || next_total->integer < total || next_total->integer > 100000 ||
            text_field(result, "id") != job->string || text_field(result, "shape") != id->string ||
            field(result, "index")->kind != value_kind::integer || field(result, "index")->integer != index->integer ||
            response->definition.id != id->string)
          throw external_protocol();
        total = next_total->integer;
        results.push_back(std::move(result));
        definitions.push_back(record({{"index", index},
                                      {"job", job},
                                      {"id", text(response->definition.id)},
                                      {"revision", text(response->definition.revision)},
                                      {"definition", text(response->definition.digest)}}));
      }
      auto finalized = phase(2, record({{"prepared", prepared},
                                        {"results", array(std::move(results))},
                                        {"definitions", array(std::move(definitions))}}));
      if (!selection || finalized->fields.contains("error"))
        return finish(finalized);
      auto model    = field(finalized, "value");
      auto selected = phase(3, record({{"request", selection}, {"model", model}}));
      if (!render || selected->fields.contains("error"))
        return finish(selected);
      if (active && basis)
        *basis = record({{"request", render_request}, {"model", model}, {"selected", field(selected, "value")}});
      auto rendered =
          phase(active ? 7 : 5, record({{"request", render_request}, {"model", model}, {"selected", field(selected, "value")}}));
      return finish(rendered);
    } catch (diagnostic& error) {
      if (error.code == "resource_limit")
        executing = true;
      return failure(error.code == "resource_limit" ? std::move(error) : external_protocol());
    } catch (const std::bad_alloc&) {
      return allocation_failure();
    } catch (const std::length_error&) {
      return allocation_failure();
    } catch (...) {
      return failure(external_protocol());
    }
  } catch (const std::bad_alloc&) {
    return allocation_failure();
  } catch (const std::length_error&) {
    return allocation_failure();
  }
}
namespace {
/// Releases every invocation-local reference and closes the Lua state when a command body ends, on every exit path.
struct command_terminal_release {
  command_vm::implementation& owner; ///< VM whose invocation state is released.
  ~command_terminal_release() {
    if (owner.lua) {
      lua_close(owner.lua);
      owner.lua = nullptr;
    }
    owner.command.services = nullptr;
    owner.command.handles.clear();
    owner.command.entries.clear();
    owner.command.options = command_render_options{};
    owner.command.pending_descriptor.reset();
    owner.command.invocation.reset();
    owner.command.response.reset();
    owner.command.native_error.reset();
    owner.command.facts.reset();
    owner.command.opened.reset();
    owner.runtime.input.reset();
    owner.runtime.codec_result.reset();
    owner.runtime.pending.clear();
  }
};
/// The shared command invocation body: one retained handler call over the supplied native services.
/// The caller decides whether a profile VM may enter; this body only refuses a consumed or failed VM.
std::expected<value_ptr, diagnostic> invoke_command_body(command_vm::implementation& owner, command_options options,
                                                         command_services& services) {
  if (owner.runtime.seal_failed || owner.command.invoked)
    return std::unexpected(diagnostic{"internal", "Command invocation already consumed"});
  auto& s   = owner.runtime;
  auto& c   = owner.command;
  auto  l   = owner.lua;
  c.invoked = true;
  command_terminal_release release{owner};
  try {
    c.options = std::move(options);
    prepare_invocation(c);
    begin_invocation(s, c);
    // Conversion charges include registration transfers; only Lua instructions restart.
    s.active.clear();
    c.services = &services;
    validate(s, c.invocation, 1);
    lua_pushcfunction(l, execute_command);
    auto status = lua_pcall(l, 0, LUA_MULTRET, 0);
    c.services  = nullptr;
    // A native failure is retained before creating any Lua response, and cannot be hidden.
    if (c.native_error)
      return command_record({{"ok", command_boolean(false)}, {"error", c.native_error}});
    if (exhausted(s) || status == LUA_ERRMEM)
      return std::unexpected(command_resource(s));
    if (c.bridge_error)
      return std::unexpected(*c.bridge_error);
    if (status || lua_gettop(l) != 1)
      command_refuse();
    auto response = convert(l, 1, 1);
    validate_response(c, response);
    return response;
  } catch (diagnostic& e) {
    c.services = nullptr;
    return std::unexpected(std::move(e));
  } catch (...) {
    c.services = nullptr;
    return std::unexpected(diagnostic{"internal", "Command execution failed"});
  }
}
} // namespace
std::expected<value_ptr, diagnostic> command_vm::invoke(command_options options, command_services& services) {
  if (_owner && _owner->command.profile)
    return std::unexpected(external_protocol());
  if (!_owner)
    return std::unexpected(diagnostic{"internal", "Command invocation already consumed"});
  return invoke_command_body(*_owner, std::move(options), services);
}
namespace {
/// Render under a profile: the same two native services as the shipped handler, with the approved render_profile phases
/// replacing the embedded renderer so that render_external_v1 and custom shapes apply. The VM never began a profile frame.
std::expected<value_ptr, diagnostic> invoke_known_render(command_vm& vm, command_vm::implementation& owner,
                                                         command_options options, command_services& services) {
  if (owner.runtime.seal_failed || owner.command.invoked)
    return std::unexpected(external_protocol());
  auto&                    s = owner.runtime;
  auto&                    c = owner.command;
  command_terminal_release release{owner};
  try {
    c.options = std::move(options);
    prepare_invocation(c);
    begin_invocation(s, c);
    s.active.clear();
    c.services = &services;
    validate(s, c.invocation, 1);
    auto l = owner.lua;
    {
      lua_pushcfunction(l, ensure_shape_baseline_protected);
      const int status = lua_pcall(l, 0, 0, 0);
      lua_settop(l, 0);
      if (status == LUA_ERRMEM || exhausted(s))
        throw command_resource(s);
      if (status || s.stack_exhausted || !c.shape_baseline_ready)
        throw external_resource();
    }
    const auto& render_options = std::get<command_render_options>(c.options);
    // One service result, copied and checked exactly as the shipped service bridge does; an error ends the invocation.
    auto accept = [&](command_service_result result) -> value_ptr {
      ++c.calls;
      if (bool(result.value) == bool(result.error))
        command_refuse();
      auto data = result.error ? result.error : result.value;
      validate(s, data, 1);
      auto copied = copy_value(data);
      if (result.error) {
        native_error_schema(copied);
        c.native_error = copied;
      }
      return copied;
    };
    auto failed = [&](const value_ptr& error) { return command_record({{"ok", command_boolean(false)}, {"error", error}}); };
    auto opened = accept(services.open_workspace(render_options));
    if (c.native_error)
      return failed(c.native_error);
    exact(opened, {"file", "workspace"});
    if (!valid_path(text_field(opened, "file")))
      command_refuse();
    c.opened = opened;
    charge(s, 3 * 128 + sizeof(value));
    auto route_input = command_record({{"file", opened->fields.at("file")}, {"workspace", opened->fields.at("workspace")}});
    auto routed      = call_retained_value(l, c.shape_phases[8], route_input);
    if (c.bridge_error)
      throw *c.bridge_error;
    if (exhausted(s))
      throw command_resource(s);
    if (!routed)
      throw routed.error().code == "resource_limit" ? std::move(routed.error()) : external_protocol();
    auto envelope = std::move(*routed);
    if (!envelope || envelope->kind != value_kind::record || envelope->fields.size() != 1 ||
        envelope->fields.contains("error") == envelope->fields.contains("value"))
      throw external_protocol();
    s.preserving_aliases = false;
    s.push_memo          = 0;
    s.input.reset();
    release_shape_temporaries(l);
    if (envelope->fields.contains("error")) {
      native_error_schema(envelope->fields.at("error"));
      return failed(envelope->fields.at("error"));
    }
    auto route = envelope->fields.at("value");
    validate_route(c, route);
    auto request = accept(services.capture_render(render_options, route));
    if (c.native_error)
      return failed(c.native_error);
    c.services    = nullptr;
    auto rendered = vm.render_profile(request);
    c.invoked     = true;
    if (!rendered)
      return std::unexpected(std::move(rendered.error().detail));
    auto result = std::move(*rendered);
    if (!result || result->kind != value_kind::record || result->fields.size() != 1 ||
        result->fields.contains("error") == result->fields.contains("value"))
      throw external_protocol();
    auto response = result->fields.contains("error")
                        ? failed(result->fields.at("error"))
                        : command_record({{"ok", command_boolean(true)}, {"value", result->fields.at("value")}});
    validate_response(c, response);
    return response;
  } catch (diagnostic& e) {
    c.services = nullptr;
    return std::unexpected(std::move(e));
  } catch (const std::bad_alloc&) {
    c.services = nullptr;
    return std::unexpected(external_resource());
  } catch (const std::length_error&) {
    c.services = nullptr;
    return std::unexpected(external_resource());
  } catch (...) {
    c.services = nullptr;
    return std::unexpected(external_protocol());
  }
}
/// Dispatch the sealed shipped callable of the admitted family in the retained VM.
/// Init, history and export run the shared command body; render runs the approved profile render phases.
std::expected<value_ptr, diagnostic> invoke_known(command_vm& vm, command_vm::implementation& owner, command_options options,
                                                  command_services& services) {
  if (std::holds_alternative<command_render_options>(options))
    return invoke_known_render(vm, owner, std::move(options), services);
  return invoke_command_body(owner, std::move(options), services);
}
} // namespace
/// Private stable pair retaining original trusted receipts and one final parsed admission.
struct extension_invocation::implementation {
  profile_capture                              capture;          ///< Original moved trusted and late-slot receipt owner.
  command_vm                                   vm;               ///< VM registered only from this capture's immutable snapshot.
  std::optional<extension_prepared_invocation> prepared;         ///< Private authenticated command/scalar preparation.
  std::vector<prepared_host_path>              paths;            ///< Same-order private spelling and intent for every host slot.
  std::string                                  workspace_parent; ///< Selected workspace file's physical parent spelling.
  std::optional<registration_error>            first_failure;    ///< First terminal diagnostic, preserved through moves.
  /// First terminal native history failure; returned without re-running any read.
  std::optional<std::variant<registration_error, ledger::format_error, history::history_read_error>> history_failure;
  std::optional<assembly_error>     assembly_failure;         ///< First trusted assembly failure, retained across moves/retries.
  value_ptr                         authenticated_projection; ///< Exact native-assembled DATA; never caller supplied.
  std::optional<acceptance_capture> role_capture;             ///< Original sealed role inputs, reverified through release.
  value_ptr                         authenticated_role;       ///< Exact private selected-role transport.
  value_ptr                         role_basis;   ///< Authentic same-call request/model/selection for future projection.
  value_ptr                         role_request; ///< Authentic captured request for later projection composition.
  bool                              role_attempted = false;    ///< Whether the one original-owner role preparation has started.
  value_ptr                         authenticated_history;     ///< Private native result; no mutable authority escapes.
  bool                              history_attempted = false; ///< Whether the one synchronous history preparation has started.
  bool                              attempted         = false, ///< Whether the one admission attempt has started.
      admitted                                        = false, ///< Whether original receipts passed final admission verification.
      executed                                        = false; ///< Whether the single execution attempt has started.
  /// @brief Move the original capture and its internally registered VM into stable ownership.
  /// @param captured Original receipt owner, never recaptured or replaced.
  /// @param registered VM built from captured.snapshot() by the private factory.
  implementation(profile_capture captured, command_vm registered) : capture(std::move(captured)), vm(std::move(registered)) {
  }
};
namespace {
// Pinned libc++ lexical normalization reserves 32 component pairs, then doubles.
// N+1 bounds components without parsing paths; the geometric total is below 4C.
// The separate string allowance covers path copies, reserved output and spelling growth.
void charge_extension_path(state& s, std::size_t bytes, std::size_t extra = 0) {
  // Refuse through the existing metric before summing hostile snapshot lengths.
  constexpr auto component_bytes = 4 * sizeof(std::pair<std::string_view, unsigned char>);
  constexpr auto fixed_bytes     = 8 * sizeof(std::string);
  constexpr auto safe_bytes      = (memory_max - fixed_bytes) / (component_bytes + 16) - 1;
  if (bytes > safe_bytes || extra > safe_bytes - bytes) {
    charge(s, memory_max);
    charge(s, 1);
    return;
  }
  const auto n = bytes + extra + 1;
  charge(s, component_bytes * std::max(std::size_t{32}, n) + 16 * n + fixed_bytes);
}
registration_error extension_protocol_error() {
  return {external_protocol(), "catalog.commands"};
}
} // namespace
extension_invocation::extension_invocation(std::unique_ptr<implementation> owner) : _owner(std::move(owner)) {
}
extension_invocation::extension_invocation(extension_invocation&& other) noexcept            = default;
extension_invocation& extension_invocation::operator=(extension_invocation&& other) noexcept = default;
extension_invocation::~extension_invocation()                                                = default;
extension_invocation::extension_invocation(profile_capture&& capture, command_vm&& vm)
    : _owner(std::make_unique<implementation>(std::move(capture), std::move(vm))) {
}
// The single registration path shared by create and the profile owner factories; charges the owner size once.
std::expected<command_vm, registration_error> extension_invocation::register_vm(const profile_capture& capture) {
  try {
    auto snapshot = capture.snapshot();
    if (!snapshot)
      return std::unexpected(registration_error{snapshot.error().detail, snapshot.error().field});
    capture.note_registration();
    auto vm = command_vm::create(*snapshot);
    if (!vm)
      return std::unexpected(std::move(vm.error()));
    charge(vm->_owner->runtime, sizeof(implementation) + 64);
    vm->_owner->command.registration_field = "catalog.commands";
    return std::move(*vm);
  } catch (const diagnostic& error) {
    return std::unexpected(registration_error{error, "profile"});
  } catch (const std::bad_alloc&) {
    return std::unexpected(registration_error{external_resource(), "profile"});
  } catch (const std::length_error&) {
    return std::unexpected(registration_error{external_resource(), "profile"});
  }
}
std::expected<extension_invocation, registration_error> extension_invocation::create(profile_capture capture) {
  try {
    auto vm = register_vm(capture);
    if (!vm)
      return std::unexpected(std::move(vm.error()));
    return extension_invocation(std::make_unique<implementation>(std::move(capture), std::move(*vm)));
  } catch (const diagnostic& error) {
    return std::unexpected(registration_error{error, "profile"});
  } catch (const std::bad_alloc&) {
    return std::unexpected(registration_error{external_resource(), "profile"});
  } catch (const std::length_error&) {
    return std::unexpected(registration_error{external_resource(), "profile"});
  }
}
/// Private stable owner of the original capture, its once-registered VM and the initial admission receipts.
/// Metric: the owner allocation is charged once per owner as sizeof(implementation)+64 to the cumulative native_bytes,
/// never refunded; this excludes receipts and capture, whose storage is charged by the admission/native layers.
struct profile_invocation::implementation {
  std::shared_ptr<factory_budget>                 budget;  ///< Factory sink; declared first so it outlives capture and receipts.
  profile_capture                                 capture; ///< Original capture; never recaptured.
  command_vm                                      vm;      ///< VM registered once from this capture's snapshot.
  std::optional<retained_root_receipt>            root_receipt;        ///< Original supplied workspace root token (known load).
  std::optional<retained_file_receipt>            declaration_receipt; ///< Original selected declaration token (known load).
  std::optional<retained_file_receipt>            registry_receipt;    ///< Original registry token (known load).
  std::size_t                                     registrations_at_adoption = 0; ///< Capture registration count at adoption.
  std::shared_ptr<const acceptance::request_data> bound_intent; ///< Immutable intent bound at open_known_load; null when opaque.
  std::size_t                                     owner_charge = 0;     ///< Bytes charged for this allocation by the factory.
  bool                                            known_load   = false; ///< Set only by open_known_load.
  std::optional<command_options>                  admitted; ///< Exact options admitted by the closed factory; empty otherwise.
  bool                                            frame_begun = false; ///< Whether the single active frame has begun.
  bool                                            consumed    = false; ///< Whether into_extension took the capture and VM.
  std::optional<registration_error>               first_failure;       ///< First terminal diagnostic, preserved across moves.
  /// @param sink Factory budget shared with the capture and receipts.
  /// @param captured Original receipt owner.
  /// @param registered VM registered from captured.
  implementation(std::shared_ptr<factory_budget> sink, profile_capture captured, command_vm registered)
      : budget(std::move(sink)), capture(std::move(captured)), vm(std::move(registered)) {
  }
};
namespace {
registration_error profile_owner_protocol_error() {
  return {external_protocol(), "profile"};
}
registration_error from_profile_error(profile_error error) {
  return {std::move(error.detail), std::move(error.field)};
}
/// The family error for a supplied ledger that does not exist; admission never registers in that case.
registration_error admitted_ledger_absent() {
  return {diagnostic{"ledger_not_found", "Ledger was not found"}, "ledger"};
}
/// Admit one supplied ledger file; genuine absence keeps the family error and every other refusal passes through.
std::expected<void, registration_error> admit_ledger(profile_capture& capture, std::string_view ledger) {
  auto admitted = capture.admit_file("ledger", ledger, admission_file_role::ledger);
  if (admitted)
    return {};
  if (capture.last_admission_refusal_was_absent())
    return std::unexpected(admitted_ledger_absent());
  return std::unexpected(from_profile_error(std::move(admitted.error())));
}
/// The directory of an ID=DIR mapping, or nothing when the token has no separator; dispatch refuses such a token.
std::optional<std::string_view> mapping_directory(std::string_view mapping) {
  const auto at = mapping.find('=');
  if (at == std::string_view::npos)
    return std::nullopt;
  return mapping.substr(at + 1);
}
/// Metadata-only admission of every supplied slot of the family, after capture and before registration.
std::expected<void, registration_error> admit_family_slots(profile_capture& capture, const command_options& family) {
  struct visitor {
    profile_capture&                        capture;
    std::expected<void, registration_error> operator()(const command_render_options& o) const {
      const auto host = std::filesystem::path(o.workspace).parent_path().string();
      if (auto admitted = capture.admit_root("workspace", host); !admitted)
        return std::unexpected(from_profile_error(std::move(admitted.error())));
      if (auto admitted = capture.admit_file("declaration", o.workspace, admission_file_role::declaration); !admitted)
        return std::unexpected(registration_error{
            acceptance::known_declaration_error(admitted.error().detail, capture.last_admission_refusal_was_absent()).detail,
            "profile"});
      if (auto admitted = capture.admit_file("registry", o.registry, admission_file_role::registry); !admitted)
        return std::unexpected(from_profile_error(std::move(admitted.error())));
      if (auto directory = mapping_directory(o.root))
        if (auto admitted = capture.admit_root("source", *directory); !admitted)
          return std::unexpected(from_profile_error(std::move(admitted.error())));
      if (o.base_root)
        if (auto directory = mapping_directory(*o.base_root))
          if (auto admitted = capture.admit_root("base", *directory); !admitted)
            return std::unexpected(from_profile_error(std::move(admitted.error())));
      if (o.observations)
        if (auto admitted = capture.admit_file("observations", *o.observations, admission_file_role::observation); !admitted)
          return std::unexpected(from_profile_error(std::move(admitted.error())));
      return {};
    }
    std::expected<void, registration_error> operator()(const command_init_options& o) const {
      // Alias-only: a new target has nothing to alias, and no containment refusal applies to it.
      std::error_code ec;
      const auto      status = std::filesystem::symlink_status(o.ledger, ec);
      if (!ec && status.type() == std::filesystem::file_type::not_found)
        return {};
      if (ec && ec == std::errc::no_such_file_or_directory)
        return {};
      return admit_ledger(capture, o.ledger);
    }
    std::expected<void, registration_error> operator()(const command_history_options& o) const {
      return admit_ledger(capture, o.ledger);
    }
    std::expected<void, registration_error> operator()(const command_export_options& o) const {
      if (auto admitted = admit_ledger(capture, o.ledger); !admitted)
        return admitted;
      if (auto admitted = capture.admit_target("target", o.target); !admitted)
        return std::unexpected(from_profile_error(std::move(admitted.error())));
      return {};
    }
    std::expected<void, registration_error> operator()(const command_load_options&) const {
      return std::unexpected(profile_owner_protocol_error());
    }
  };
  return std::visit(visitor{capture}, family);
}
/// Exact field-by-field equality of two option sets of the same family.
bool same_options(const command_options& a, const command_options& b) {
  if (a.index() != b.index())
    return false;
  struct visitor {
    const command_options& other;
    bool                   operator()(const command_render_options& x) const {
      const auto& y = std::get<command_render_options>(other);
      return x.workspace == y.workspace && x.registry == y.registry && x.root == y.root && x.job == y.job && x.role == y.role &&
             x.base_root == y.base_root && x.observations == y.observations && x.budget_bytes == y.budget_bytes &&
             x.capture_outputs == y.capture_outputs;
    }
    bool operator()(const command_init_options& x) const {
      const auto& y = std::get<command_init_options>(other);
      return x.ledger == y.ledger && x.workspace_id == y.workspace_id;
    }
    bool operator()(const command_history_options& x) const {
      const auto& y = std::get<command_history_options>(other);
      return x.ledger == y.ledger && x.revision == y.revision && x.at_event == y.at_event && x.after_event == y.after_event &&
             x.request_id == y.request_id && x.limit == y.limit;
    }
    bool operator()(const command_export_options& x) const {
      const auto& y = std::get<command_export_options>(other);
      return x.ledger == y.ledger && x.target == y.target && x.revision == y.revision;
    }
    bool operator()(const command_load_options& x) const {
      const auto& y = std::get<command_load_options>(other);
      return x.ledger == y.ledger && x.workspace == y.workspace && x.registry == y.registry &&
             x.expect_revision == y.expect_revision && x.request_id == y.request_id && x.actor == y.actor &&
             x.expected_head == y.expected_head;
    }
  };
  return std::visit(visitor{b}, a);
}
} // namespace
profile_invocation::profile_invocation(std::unique_ptr<implementation> owner) : _owner(std::move(owner)) {
}
profile_invocation::profile_invocation(profile_invocation&& other) noexcept = default;
profile_invocation& profile_invocation::operator=(profile_invocation&& other) noexcept {
  if (this != &other)
    _owner = std::move(other._owner);
  return *this;
}
profile_invocation::~profile_invocation() = default;
std::expected<profile_invocation, registration_error>
profile_invocation::open_known_load(const acceptance::request_intent& request_intent, std::string_view ledger_path) {
  try {
    auto data = request_intent.data();
    if (!data || !*data || !(*data)->version2 || !(*data)->version2->profile_argument)
      return std::unexpected(profile_owner_protocol_error());
    const auto& v2      = *(*data)->version2;
    auto        budget  = std::make_shared<factory_budget>();
    auto        capture = profile_capture::open(*v2.profile_argument, budget);
    if (!capture)
      return std::unexpected(from_profile_error(std::move(capture.error())));
    // Metadata-only admissions precede registration; the physical parent is not normalized.
    const auto host = std::filesystem::path(v2.workspace_argument).parent_path().string();
    if (auto admitted = capture->admit_root("workspace", host); !admitted)
      return std::unexpected(from_profile_error(std::move(admitted.error())));
    if (auto admitted = capture->admit_file("declaration", v2.workspace_argument, admission_file_role::declaration); !admitted)
      return std::unexpected(registration_error{
          acceptance::known_declaration_error(admitted.error().detail, capture->last_admission_refusal_was_absent()).detail,
          "profile"});
    if (auto admitted = capture->admit_file("registry", v2.registry_argument, admission_file_role::registry); !admitted)
      return std::unexpected(from_profile_error(std::move(admitted.error())));
    // Ledger-contract load steps 3 to 5: the ledger is the store compared by the CAS (step 4 revision_conflict
    // precedes the step-5 receipt recheck), so a winner's commit must not fail receipts; its identity and mode
    // remain checked (step 5).
    if (auto admitted = capture->admit_file("ledger", ledger_path, admission_file_role::load_ledger); !admitted)
      return std::unexpected(from_profile_error(std::move(admitted.error())));
    auto root = capture->retain_root("workspace");
    if (!root)
      return std::unexpected(from_profile_error(std::move(root.error())));
    auto declaration = capture->retain_file("declaration");
    if (!declaration)
      return std::unexpected(from_profile_error(std::move(declaration.error())));
    auto registry = capture->retain_file("registry");
    if (!registry)
      return std::unexpected(from_profile_error(std::move(registry.error())));
    auto vm = extension_invocation::register_vm(*capture);
    if (!vm)
      return std::unexpected(std::move(vm.error()));
    attach_budget(budget, vm->_owner->runtime);
    const auto charged_before = vm->_owner->runtime.native_bytes;
    charge(vm->_owner->runtime, sizeof(implementation) + 64);
    const auto charged         = vm->_owner->runtime.native_bytes - charged_before;
    auto       owner           = std::make_unique<implementation>(budget, std::move(*capture), std::move(*vm));
    owner->owner_charge        = charged;
    owner->root_receipt        = std::move(*root);
    owner->declaration_receipt = std::move(*declaration);
    owner->registry_receipt    = std::move(*registry);
    owner->known_load          = true;
    owner->bound_intent        = *data;
    if (auto verified = owner->capture.verify(); !verified)
      return std::unexpected(from_profile_error(std::move(verified.error())));
    return profile_invocation(std::move(owner));
  } catch (const diagnostic& error) {
    return std::unexpected(registration_error{error, "profile"});
  } catch (const std::bad_alloc&) {
    return std::unexpected(registration_error{external_resource(), "profile"});
  } catch (const std::length_error&) {
    return std::unexpected(registration_error{external_resource(), "profile"});
  }
}
std::expected<profile_invocation, registration_error> profile_invocation::open_opaque(std::string_view profile_selector) {
  try {
    auto budget  = std::make_shared<factory_budget>();
    auto capture = profile_capture::open(profile_selector, budget);
    if (!capture)
      return std::unexpected(from_profile_error(std::move(capture.error())));
    auto vm = extension_invocation::register_vm(*capture);
    if (!vm)
      return std::unexpected(std::move(vm.error()));
    attach_budget(budget, vm->_owner->runtime);
    const auto charged_before = vm->_owner->runtime.native_bytes;
    charge(vm->_owner->runtime, sizeof(implementation) + 64);
    const auto charged  = vm->_owner->runtime.native_bytes - charged_before;
    auto       owner    = std::make_unique<implementation>(budget, std::move(*capture), std::move(*vm));
    owner->owner_charge = charged;
    if (auto verified = owner->capture.verify(); !verified)
      return std::unexpected(from_profile_error(std::move(verified.error())));
    return profile_invocation(std::move(owner));
  } catch (const diagnostic& error) {
    return std::unexpected(registration_error{error, "profile"});
  } catch (const std::bad_alloc&) {
    return std::unexpected(registration_error{external_resource(), "profile"});
  } catch (const std::length_error&) {
    return std::unexpected(registration_error{external_resource(), "profile"});
  }
}
std::expected<profile_invocation, registration_error> profile_invocation::open_admitted(std::string_view       profile_selector,
                                                                                        const command_options& family) {
  try {
    auto budget  = std::make_shared<factory_budget>();
    auto capture = profile_capture::open(profile_selector, budget);
    if (!capture)
      return std::unexpected(from_profile_error(std::move(capture.error())));
    // Metadata-only admissions precede the single registration.
    if (auto admitted = admit_family_slots(*capture, family); !admitted)
      return std::unexpected(std::move(admitted.error()));
    auto vm = extension_invocation::register_vm(*capture);
    if (!vm)
      return std::unexpected(std::move(vm.error()));
    attach_budget(budget, vm->_owner->runtime);
    const auto charged_before = vm->_owner->runtime.native_bytes;
    charge(vm->_owner->runtime, sizeof(implementation) + 64);
    const auto charged  = vm->_owner->runtime.native_bytes - charged_before;
    auto       owner    = std::make_unique<implementation>(budget, std::move(*capture), std::move(*vm));
    owner->owner_charge = charged;
    owner->admitted     = family;
    if (auto verified = owner->capture.verify(); !verified)
      return std::unexpected(from_profile_error(std::move(verified.error())));
    return profile_invocation(std::move(owner));
  } catch (const diagnostic& error) {
    return std::unexpected(registration_error{error, "profile"});
  } catch (const std::bad_alloc&) {
    return std::unexpected(registration_error{external_resource(), "profile"});
  } catch (const std::length_error&) {
    return std::unexpected(registration_error{external_resource(), "profile"});
  }
}
std::expected<profile_invocation, registration_error> profile_invocation::open_with_profile(std::string_view profile_selector,
                                                                                            const command_options& family) {
  if (std::holds_alternative<command_load_options>(family))
    return std::unexpected(profile_owner_protocol_error());
  return open_admitted(profile_selector, family);
}
std::expected<profile_invocation, registration_error>
profile_invocation::open_with_profile(const acceptance::request_intent& request_intent, std::string_view ledger_path) {
  return open_known_load(request_intent, ledger_path);
}
std::expected<value_ptr, diagnostic> profile_invocation::run_known(command_options options, command_services& services) && {
  // One check covers moved-from, consumed, failed, frame-begun and every owner without retained admitted options.
  if (!_owner || _owner->consumed || _owner->first_failure || _owner->frame_begun || !_owner->admitted)
    return std::unexpected(profile_owner_protocol_error().detail);
  if (!same_options(*_owner->admitted, options))
    return std::unexpected(profile_owner_protocol_error().detail);
  try {
    // Consume before any fallible work: a failure below never allows a retry.
    _owner->registrations_at_adoption = _owner->capture.registration_count();
    _owner->consumed                  = true;
    if (auto verified = _owner->capture.verify(); !verified)
      return std::unexpected(std::move(verified.error().detail));
    auto* vm = _owner->vm._owner.get();
    if (!vm)
      return std::unexpected(profile_owner_protocol_error().detail);
    return invoke_known(_owner->vm, *vm, std::move(options), services);
  } catch (const diagnostic& error) {
    return std::unexpected(error);
  } catch (const std::bad_alloc&) {
    return std::unexpected(external_resource());
  } catch (const std::length_error&) {
    return std::unexpected(external_resource());
  }
}
value_ptr profile_invocation::catalog() const {
  return _owner && !_owner->consumed && !_owner->first_failure ? _owner->vm.catalog() : value_ptr{};
}
std::string_view profile_invocation::effective_catalog() const {
  return _owner && !_owner->consumed && !_owner->first_failure ? _owner->vm.effective_catalog() : std::string_view{};
}
std::expected<void, registration_error> profile_invocation::fail(implementation& owner, registration_error error) {
  if (!owner.first_failure) {
    if (owner.vm._owner && owner.vm._owner->command.shape_failure)
      owner.first_failure = std::move(owner.vm._owner->command.shape_failure);
    else
      owner.first_failure = std::move(error);
  }
  return std::unexpected(*owner.first_failure);
}
std::expected<void, registration_error> profile_invocation::begin_frame(implementation& owner) {
  if (owner.consumed || owner.first_failure || owner.frame_begun)
    return std::unexpected(profile_owner_protocol_error());
  try {
    auto* vm = owner.vm._owner.get();
    if (!vm)
      return fail(owner, profile_owner_protocol_error());
    auto& s = vm->runtime;
    auto& c = vm->command;
    if (!c.profile || c.registering || c.shape_failure || s.seal_failed || c.invoked || exhausted(s))
      return fail(owner, profile_owner_protocol_error());
    begin_invocation(s, c);
    c.invoked             = true;
    c.profile_frame_owner = &owner;
    owner.frame_begun     = true;
    return {};
  } catch (const diagnostic& error) {
    return fail(owner, registration_error{error, "profile"});
  } catch (...) {
    return fail(owner, registration_error{external_resource(), "profile"});
  }
}
std::expected<extension_invocation, registration_error> profile_invocation::into_extension() && {
  // Known-load owners are refused without being consumed: adoption cannot change mode.
  if (!_owner || _owner->consumed || _owner->first_failure || _owner->frame_begun || _owner->known_load)
    return std::unexpected(profile_owner_protocol_error());
  try {
    extension_invocation adopted(std::move(_owner->capture), std::move(_owner->vm));
    _owner->registrations_at_adoption = adopted._owner->capture.registration_count();
    _owner->consumed                  = true;
    return adopted;
  } catch (const diagnostic& error) {
    return std::unexpected(registration_error{error, "profile"});
  } catch (...) {
    return std::unexpected(registration_error{external_resource(), "profile"});
  }
}
namespace {
/// Written by the frame cleanup after it has released nested temporaries; observation only.
struct cleanup_report {
  bool        ran   = false; ///< Cleanup executed.
  std::size_t peak  = 0;     ///< Temporary registry entries just before the release.
  std::size_t after = 0;     ///< Temporary registry entries after the release.
};
} // namespace
/// Private stage state: the consumed owner (VM, original capture and receipts), the sealed source capture, the normalized
/// bundle B (field bundle of S) and the finalized model S. Metric: the allocation is charged once as
/// sizeof(implementation)+64 to the cumulative native_bytes before it is made, never refunded.
struct profile_load_stage::implementation {
  std::unique_ptr<profile_invocation::implementation> owner;  ///< Original owner, frame begun, counters retained.
  acceptance::source_capture                          source; ///< Sealed bound capture; destroyed before the owner.
  value_ptr                                           bundle; ///< B: normalized bundle, never exposed.
  value_ptr                                           model;  ///< S: finalized model, never exposed.
  cleanup_report                                      report; ///< Result of the frame cleanup that ran before staging.
  bool baseline_before_route = false;             ///< Observation-only: shape baseline existed just before the route call.
  std::array<std::size_t, 4> instruction_marks{}; ///< Observation-only counter marks.
  std::array<std::size_t, 4> native_byte_marks{}; ///< Observation-only counter marks.
  /// @param o Consumed owner.
  /// @param captured Sealed capture.
  /// @param b Normalized bundle.
  /// @param m Finalized model.
  implementation(std::unique_ptr<profile_invocation::implementation> o, acceptance::source_capture captured, value_ptr b,
                 value_ptr m)
      : owner(std::move(o)), source(std::move(captured)), bundle(std::move(b)), model(std::move(m)) {
  }
};
profile_load_stage::profile_load_stage(std::unique_ptr<implementation> state) : _impl(std::move(state)) {
}
profile_load_stage::profile_load_stage(profile_load_stage&& other) noexcept = default;
profile_load_stage& profile_load_stage::operator=(profile_load_stage&& other) noexcept {
  if (this != &other)
    _impl = std::move(other._impl);
  return *this;
}
profile_load_stage::~profile_load_stage() = default;
/// Private prepared state: the retained internal stage (owner, sealed source, B and S) and the immutable derived data.
struct prepared_profile::implementation {
  profile_load_stage                             stage; ///< Retained internal step; owns the VM and every native receipt.
  std::shared_ptr<const profile_acceptance_data> data;  ///< Immutable owned values; no aliases into the stage.
  /// @param s Retained stage.
  /// @param d Derived data.
  implementation(profile_load_stage s, std::shared_ptr<const profile_acceptance_data> d)
      : stage(std::move(s)), data(std::move(d)) {
  }
};
prepared_profile::prepared_profile(std::unique_ptr<implementation> state) : _impl(std::move(state)) {
}
prepared_profile::prepared_profile(prepared_profile&& other) noexcept = default;
prepared_profile& prepared_profile::operator=(prepared_profile&& other) noexcept {
  if (this != &other)
    _impl = std::move(other._impl);
  return *this;
}
prepared_profile::~prepared_profile() = default;
std::expected<std::shared_ptr<const profile_acceptance_data>, registration_error> prepared_profile::data() const {
  if (!_impl)
    return std::unexpected(registration_error{external_protocol(), "profile"});
  return _impl->data;
}
std::shared_ptr<const acceptance::request_data> prepared_profile::bound_intent() const {
  return _impl && _impl->data ? _impl->data->bound_intent : nullptr;
}
std::expected<void, diagnostic> prepared_profile::verify_native_receipts() const {
  const auto changed = diagnostic{"capture_changed", "Captured inputs changed."};
  if (!_impl || !_impl->stage._impl || !_impl->stage._impl->owner)
    return std::unexpected(changed);
  auto& stage = *_impl->stage._impl;
  if (auto verified = stage.source.verify(); !verified) {
    if (verified.error().code == acceptance::source_error_code::capture ||
        verified.error().code == acceptance::source_error_code::resource_limit)
      return std::unexpected(std::move(verified.error().detail));
    return std::unexpected(changed);
  }
  if (auto verified = stage.owner->capture.verify(); !verified)
    return std::unexpected(std::move(verified.error().detail));
  return {};
}
profile_load_stage_observation prepared_profile::observe() const noexcept {
  return _impl ? _impl->stage.observe() : profile_load_stage_observation{};
}
profile_load_stage_observation profile_load_stage::observe() const noexcept {
  profile_load_stage_observation out;
  if (!_impl || !_impl->owner)
    return out;
  out.live            = true;
  out.frame_begun     = _impl->owner->frame_begun;
  out.sealed          = _impl->source.sealed_ready();
  out.bundle_retained = static_cast<bool>(_impl->bundle);
  out.model_retained  = static_cast<bool>(_impl->model);
  if (_impl->model) {
    if (auto it = _impl->model->fields.find("jobs"); it != _impl->model->fields.end() && it->second)
      out.jobs = it->second->items.size();
  }
  out.registrations = _impl->owner->capture.registration_count();
  if (const auto* vm = _impl->owner->vm._owner.get()) {
    out.instructions      = vm->runtime.instructions;
    out.native_bytes      = vm->runtime.native_bytes;
    out.shape_dispatches  = vm->command.shape_dispatch_count;
    const auto membership = measure_registry(vm->lua);
    out.tag_entries       = membership.tags;
    out.backing_entries   = membership.backings;
    out.temporary_entries = membership.temporaries;
  }
  out.cleanup_ran            = _impl->report.ran;
  out.peak_temporary_entries = _impl->report.peak;
  out.baseline_before_route  = _impl->baseline_before_route;
  out.instruction_marks      = _impl->instruction_marks;
  out.native_byte_marks      = _impl->native_byte_marks;
  return out;
}
namespace {
registration_error from_source_error(acceptance::source_error error) {
  if (error.code == acceptance::source_error_code::capture || error.code == acceptance::source_error_code::resource_limit)
    return {std::move(error.detail), "profile"};
  return profile_owner_protocol_error();
}
} // namespace
std::expected<prepared_profile, profile_preparation_error>
profile_invocation::prepare_load(const acceptance::request_intent& final_intent) && {
  cleanup_report report;
  auto           run = [&]() -> std::expected<prepared_profile, profile_preparation_error> {
    try {
      if (!_owner || _owner->consumed || _owner->first_failure || !_owner->known_load || _owner->frame_begun ||
          !_owner->bound_intent)
        return std::unexpected(profile_preparation_error{profile_owner_protocol_error(), {}});
      // Consume first: every later failure, including error-copy allocation, leaves the caller's owner moved-from.
      auto impl   = std::move(_owner);
      auto refuse = [&](registration_error error) -> std::unexpected<profile_preparation_error> {
        (void)fail(*impl, std::move(error));
        return std::unexpected(profile_preparation_error{*impl->first_failure, {}});
      };
      auto semantic = [&](value_ptr inner) -> std::unexpected<profile_preparation_error> {
        return std::unexpected(profile_preparation_error{registration_error{}, std::move(inner)});
      };
      auto final_data = final_intent.data();
      if (!final_data || !*final_data)
        return refuse(profile_owner_protocol_error());
      {
        const auto& now   = **final_data;
        const auto& bound = *impl->bound_intent;
        if (!now.version2 || !bound.version2 || now.request_id != bound.request_id || now.expected_head != bound.expected_head ||
            now.encoded != bound.encoded || now.version2->workspace_argument != bound.version2->workspace_argument ||
            now.version2->registry_argument != bound.version2->registry_argument ||
            now.version2->profile_argument != bound.version2->profile_argument)
          return refuse(profile_owner_protocol_error());
      }
      if (!impl->root_receipt || !impl->declaration_receipt || !impl->registry_receipt)
        return refuse(profile_owner_protocol_error());
      if (auto verified = impl->capture.verify(); !verified)
        return refuse(from_profile_error(std::move(verified.error())));
      const auto& bound_v2 = *impl->bound_intent->version2;
      auto source = acceptance::source_capture::open_bound(bound_v2.workspace_argument, bound_v2.registry_argument,
                                                           std::move(*impl->root_receipt), std::move(*impl->declaration_receipt),
                                                           std::move(*impl->registry_receipt), impl->budget);
      if (!source)
        return refuse(from_source_error(std::move(source.error())));
      if (auto begun = begin_frame(*impl); !begun)
        return std::unexpected(profile_preparation_error{std::move(begun.error()), {}});
      command_vm::active_frame frame;
      frame.profile_owner = impl.get();
      auto& vm            = *impl->vm._owner;
      auto& s             = vm.runtime;
      auto& c             = vm.command;
      // Declared after impl so it runs first on every exit: success, refusal, native failure and exception.
      // It never allocates or throws and is idempotent, so staging may run it early before ownership moves.
      struct frame_cleanup {
        lua_State*      lua;
        state*          runtime;
        cleanup_report* out;
        bool            done = false;
        void            run() noexcept {
          if (done)
            return;
          done                        = true;
          runtime->preserving_aliases = false;
          runtime->push_memo          = 0;
          runtime->input.reset();
          lua_settop(lua, 0);
          out->peak = measure_registry(lua).temporaries;
          release_shape_temporaries(lua);
          out->after = measure_registry(lua).temporaries;
          out->ran   = true;
        }
        ~frame_cleanup() {
          run();
        }
      } cleanup{vm.lua, &s, &report};
      std::array<std::size_t, 4> instruction_marks{}, native_byte_marks{};
      auto                       mark = [&](std::size_t index) {
        instruction_marks[index] = s.instructions;
        native_byte_marks[index] = s.native_bytes;
      };
      mark(0);
      // The nested calls below never take the baseline themselves, so establish it before any temporary is created.
      {
        lua_pushcfunction(vm.lua, ensure_shape_baseline_protected);
        const int status = lua_pcall(vm.lua, 0, 0, 0);
        lua_settop(vm.lua, 0);
        if (status == LUA_ERRMEM || exhausted(s))
          return refuse(registration_error{command_resource(s), "profile"});
        if (status || s.stack_exhausted || !c.shape_baseline_ready)
          return refuse(registration_error{external_resource(), "profile"});
      }
      auto record = [&](std::initializer_list<std::pair<const std::string, value_ptr>> fields) {
        visit(s, 1);
        for (const auto& field : fields)
          charge(s, 128 + field.first.size());
        return command_record(std::map<std::string, value_ptr>(fields));
      };
      auto text = [&](std::string_view bytes) {
        visit(s, 1);
        charge(s, bytes.size() + 1);
        return command_text(std::string(bytes));
      };
      // Strict envelope: exactly one of value or error.
      auto envelope_ok = [](const value_ptr& envelope) {
        return envelope && envelope->kind == value_kind::record && envelope->fields.size() == 1 &&
               envelope->fields.contains("error") != envelope->fields.contains("value");
      };
      auto routed_input = source->workspace();
      if (!routed_input)
        return refuse(from_source_error(std::move(routed_input.error())));
      if (!*routed_input || (*routed_input)->kind != value_kind::record || !(*routed_input)->fields.contains("file") ||
          !(*routed_input)->fields.contains("workspace"))
        return refuse(profile_owner_protocol_error());
      auto route_input =
          record({{"file", (*routed_input)->fields.at("file")}, {"workspace", (*routed_input)->fields.at("workspace")}});
      const bool baseline_before_route = c.shape_baseline_ready;
      auto       routed                = call_retained_value(vm.lua, c.shape_phases[8], route_input);
      if (c.bridge_error)
        return refuse(registration_error{*c.bridge_error, "profile"});
      if (exhausted(s))
        return refuse(registration_error{command_resource(s), "profile"});
      mark(1);
      if (!routed)
        return refuse(registration_error{
            routed.error().code == "resource_limit" ? std::move(routed.error()) : external_protocol(), "profile"});
      if (!envelope_ok(*routed))
        return refuse(profile_owner_protocol_error());
      if ((*routed)->fields.contains("error"))
        return semantic((*routed)->fields.at("error"));
      auto bundle = source->capture((*routed)->fields.at("value"));
      if (!bundle)
        return refuse(from_source_error(std::move(bundle.error())));
      if (auto verified = source->verify(); !verified)
        return refuse(from_source_error(std::move(verified.error())));
      if (auto verified = impl->capture.verify(); !verified)
        return refuse(from_profile_error(std::move(verified.error())));
      // Capture-stage storage is charged inside the native and source layers to the one factory sink.
      if (!source->sealed())
        return refuse(profile_owner_protocol_error());
      auto validation = record({{"bundle", *bundle}, {"capture", record({{"status", text("not_captured")}})}});
      mark(2);
      auto prepared = impl->vm.validate_profile_phases(validation, {}, {}, frame, nullptr);
      if (!prepared)
        return refuse(std::move(prepared.error()));
      mark(3);
      if (!envelope_ok(*prepared))
        return refuse(profile_owner_protocol_error());
      if ((*prepared)->fields.contains("error"))
        return semantic((*prepared)->fields.at("error"));
      auto model = (*prepared)->fields.at("value");
      if (!model || model->kind != value_kind::record || !model->fields.contains("bundle") || !model->fields.contains("jobs") ||
          !model->fields.at("bundle") || !model->fields.at("jobs") || model->fields.at("jobs")->kind != value_kind::list)
        return refuse(profile_owner_protocol_error());
      auto retained_bundle = model->fields.at("bundle");
      charge(s, sizeof(profile_load_stage::implementation) + 64);
      auto staged = std::make_unique<profile_load_stage::implementation>(std::move(impl), std::move(*source),
                                                                         std::move(retained_bundle), std::move(model));
      // Ownership of the VM moved into the stage: release the nested temporaries now, while it is certainly alive.
      cleanup.run();
      staged->report                = report;
      staged->baseline_before_route = baseline_before_route;
      staged->instruction_marks     = instruction_marks;
      staged->native_byte_marks     = native_byte_marks;
      auto& owner_state             = *staged->owner;
      auto& stage_vm                = *owner_state.vm._owner;
      auto& st                      = stage_vm.runtime;
      auto& cm                      = stage_vm.command;
      auto  refuse_owner            = [&](registration_error error) -> std::unexpected<profile_preparation_error> {
        (void)fail(owner_state, std::move(error));
        return std::unexpected(profile_preparation_error{*owner_state.first_failure, {}});
      };
      auto protocol = [&]() -> std::unexpected<profile_preparation_error> {
        (void)fail(owner_state, profile_owner_protocol_error());
        return std::unexpected(profile_preparation_error{*owner_state.first_failure, {}});
      };
      auto native_mode = [](std::string_view text) -> std::optional<std::uint32_t> {
        if (text == "0644")
          return 0644u;
        if (text == "0755")
          return 0755u;
        return std::nullopt;
      };
      // Phase 4: reassemble the effective raw tree from the owner's own originals; every step is charged first.
      auto original_snapshot = owner_state.capture.snapshot();
      if (!original_snapshot || !*original_snapshot)
        return protocol();
      const auto& closure = **original_snapshot;
      {
        std::size_t total = cm.assembly.size() + cm.effective_catalog_bytes.size() + closure.profile.bytes.size();
        for (const auto& f : cm.originals)
          total += f.bytes.size();
        for (const auto& f : closure.modules)
          total += f.bytes.size();
        charge(st, total * 4 + (cm.originals.size() + closure.modules.size() + 16) * 4096);
      }
      auto embedded = history::prepare_stdlib(cm.originals, cm.separator, cm.assembly);
      if (!embedded)
        return embedded.error().code == history::provenance_error_code::resource_limit
                   ? refuse_owner(registration_error{external_resource(), "profile"})
                   : protocol();
      std::vector<history::tree_file_view> module_views;
      module_views.reserve(closure.modules.size());
      for (const auto& f : closure.modules) {
        auto m = native_mode(f.mode);
        if (!m)
          return protocol();
        module_views.push_back({f.path, f.bytes, *m});
      }
      auto profile_mode = native_mode(closure.profile.mode);
      if (!profile_mode)
        return protocol();
      auto effective = history::prepare_effective_stdlib(
          {embedded->files, closure.profile.bytes, *profile_mode, module_views, cm.effective_catalog_bytes});
      if (!effective)
        return effective.error().code == history::effective_error_code::resource_limit
                   ? refuse_owner(registration_error{external_resource(), "profile"})
                   : protocol();
      auto verified_tree = history::verify_effective_stdlib(effective->files);
      if (!verified_tree)
        return verified_tree.error().code == history::effective_error_code::resource_limit
                   ? refuse_owner(registration_error{external_resource(), "profile"})
                   : protocol();
      {
        std::string digest_text;
        for (auto b : verified_tree->digest)
          digest_text += std::format("{:02x}", b);
        const auto same = [](const auto& a, const auto& b) {
          return a.size() == b.size() && std::ranges::equal(a, b, [](const auto& x, const auto& y) {
                   return x.path == y.path && x.payload == y.payload && x.mode == y.mode;
                 });
        };
        if (digest_text != cm.effective_digest_bytes || !same(verified_tree->files, cm.provenance))
          return protocol();
      }
      // Phase 5: shared native correspondence of B against the sealed source capture.
      auto paths = acceptance::verify_load_correspondence(staged->bundle, staged->source);
      if (!paths) {
        if (paths.error().code == acceptance::correspondence_error_code::capture)
          return refuse_owner(registration_error{std::move(paths.error().detail), "profile"});
        return protocol();
      }
      // The v2 encoder derives E and V from B, S and the verified raw tree; nothing from Lua or a caller is trusted.
      {
        std::size_t total = 0;
        for (const auto& f : verified_tree->files)
          total += f.payload.size();
        charge(st, total * 2 + verified_tree->files.size() * 1024 + 65536);
      }
      // The recorded semantic model is exactly dependencies and jobs; the validation envelope's bundle and capture stay out.
      if (!staged->model->fields.contains("dependencies") || !staged->model->fields.at("dependencies"))
        return protocol();
      const auto semantic_model =
          record({{"dependencies", staged->model->fields.at("dependencies")}, {"jobs", staged->model->fields.at("jobs")}});
      auto interpretation = history::encode_interpretation_v2(staged->bundle, semantic_model, verified_tree->files);
      if (!interpretation)
        return interpretation.error().code == history::interpretation_v2_error_code::resource_limit
                   ? refuse_owner(registration_error{external_resource(), "profile"})
                   : protocol();
      constexpr std::size_t interpretation_bound = 16u * 1024u * 1024u;
      if (interpretation->encoded.size() > interpretation_bound || interpretation->versions_encoded.size() > interpretation_bound)
        return refuse_owner(registration_error{external_resource(), "profile"});
      charge(st, (interpretation->encoded.size() + interpretation->versions_encoded.size()) * 4);
      std::string workspace_id;
      {
        const auto* node = interpretation->interpretation.get();
        for (const char* name : {"bundle", "workspace", "id"}) {
          if (!node || node->kind != value_kind::record)
            return protocol();
          auto it = node->fields.find(name);
          if (it == node->fields.end())
            return protocol();
          node = it->second.get();
        }
        if (!node || node->kind != value_kind::string)
          return protocol();
        workspace_id = node->string;
      }
      // Source tree from the sealed native snapshot; registry is exactly input.json.
      if (!staged->source.sealed())
        return protocol();
      auto sealed_snapshot = staged->source.sealed()->snapshot();
      if (!sealed_snapshot || !*sealed_snapshot)
        return protocol();
      const std::set<std::string_view> selected(paths->begin(), paths->end());
      std::vector<history::tree_file>  source_files;
      const captured_file*             registry_file = nullptr;
      std::size_t                      source_bytes  = 0;
      for (const auto& f : **sealed_snapshot)
        source_bytes += f.bytes.size();
      charge(st, source_bytes * 3 + (**sealed_snapshot).size() * 4096);
      for (const auto& f : **sealed_snapshot) {
        if (f.root == "workspace" && selected.contains(f.path)) {
          auto m = native_mode(f.mode);
          if (!m)
            return protocol();
          source_files.push_back({f.path, f.bytes, *m});
        } else if (f.root == "registry") {
          if (registry_file || f.path != "input.json")
            return protocol();
          registry_file = &f;
        }
      }
      if (!registry_file || source_files.size() != selected.size())
        return protocol();
      std::ranges::sort(source_files, {}, &history::tree_file::path);
      std::vector<history::tree_file_view> source_views;
      source_views.reserve(source_files.size());
      for (const auto& f : source_files)
        source_views.push_back({f.path, f.payload, f.mode});
      auto source_digest = history::digest_tree(source_views, history::tree_profile::source);
      if (!source_digest)
        return source_digest.error().code == history::tree_error_code::resource_limit
                   ? refuse_owner(registration_error{external_resource(), "profile"})
                   : protocol();
      auto registry_mode = native_mode(registry_file->mode);
      if (!registry_mode)
        return protocol();
      auto registry = history::prepare_registry(registry_file->bytes, *registry_mode);
      if (!registry)
        return registry.error().code == history::provenance_error_code::resource_limit
                   ? refuse_owner(registration_error{external_resource(), "profile"})
                   : protocol();
      // The shared immutable data is built privately; no pointer into the VM, B, S or capture survives in it.
      charge(st, sizeof(profile_acceptance_data) + 64);
      auto data            = std::make_shared<profile_acceptance_data>();
      data->source         = std::move(source_files);
      data->source_digest  = *source_digest;
      data->registry       = std::move(*registry);
      data->effective      = std::move(*verified_tree);
      data->interpretation = std::move(*interpretation);
      data->workspace_id   = std::move(workspace_id);
      data->bound_intent   = owner_state.bound_intent;
      // Phase 6: reverify every original receipt after derivation, before anything is published.
      if (auto verified = staged->source.verify(); !verified) {
        (void)fail(owner_state, from_source_error(std::move(verified.error())));
        return std::unexpected(profile_preparation_error{*owner_state.first_failure, {}});
      }
      if (auto verified = owner_state.capture.verify(); !verified) {
        (void)fail(owner_state, from_profile_error(std::move(verified.error())));
        return std::unexpected(profile_preparation_error{*owner_state.first_failure, {}});
      }
      charge(st, sizeof(prepared_profile::implementation) + 64);
      // Consumed before any fallible outward copy: a later failure never leaves a retryable owner.
      owner_state.consumed                                     = true;
      std::shared_ptr<const profile_acceptance_data> published = std::move(data);
      auto                                           prepared_state =
          std::make_unique<prepared_profile::implementation>(profile_load_stage(std::move(staged)), std::move(published));
      return prepared_profile(std::move(prepared_state));
    } catch (const diagnostic& error) {
      return std::unexpected(profile_preparation_error{registration_error{error, "profile"}, {}});
    } catch (const std::bad_alloc&) {
      return std::unexpected(profile_preparation_error{registration_error{external_resource(), "profile"}, {}});
    } catch (const std::length_error&) {
      return std::unexpected(profile_preparation_error{registration_error{external_resource(), "profile"}, {}});
    } catch (...) {
      return std::unexpected(profile_preparation_error{profile_owner_protocol_error(), {}});
    }
  };
  auto result = run();
  if (!result) {
    result.error().cleanup_ran                     = report.ran;
    result.error().peak_temporary_entries          = report.peak;
    result.error().temporary_entries_after_release = report.after;
  }
  return result;
}
profile_invocation_observation profile_invocation::observe() const noexcept {
  profile_invocation_observation out;
  if (!_owner)
    return out;
  out.consumed      = _owner->consumed;
  out.known_load    = _owner->known_load;
  out.frame_begun   = _owner->frame_begun;
  out.live          = !_owner->consumed && !_owner->first_failure;
  out.owner_charge  = _owner->owner_charge;
  out.registrations = _owner->consumed ? _owner->registrations_at_adoption : _owner->capture.registration_count();
  if (const auto* vm = _owner->vm._owner.get(); vm && vm->lua) {
    out.instructions      = vm->runtime.instructions;
    out.native_bytes      = vm->runtime.native_bytes;
    const auto membership = measure_registry(vm->lua);
    out.tag_entries       = membership.tags;
    out.backing_entries   = membership.backings;
    out.temporary_entries = membership.temporaries;
  }
  return out;
}
std::string_view extension_invocation::effective_catalog() const {
  return _owner && !_owner->first_failure && !_owner->history_failure && !_owner->assembly_failure
             ? _owner->vm.effective_catalog()
             : std::string_view{};
}
std::expected<void, registration_error> extension_invocation::fail(registration_error error) {
  if (!_owner)
    return std::unexpected(std::move(error));
  if (_owner->history_failure)
    if (auto original = std::get_if<registration_error>(&*_owner->history_failure))
      return std::unexpected(*original);
  if (!_owner->first_failure) {
    if (_owner->vm._owner->command.shape_failure)
      _owner->first_failure = std::move(_owner->vm._owner->command.shape_failure);
    else
      _owner->first_failure = std::move(error);
  }
  return std::unexpected(*_owner->first_failure);
}
std::expected<void, registration_error> extension_invocation::admit(std::string_view                         name,
                                                                    std::span<const extension_parsed_option> options,
                                                                    std::optional<std::string_view>          explicit_cwd,
                                                                    std::optional<std::string_view> workspace_directory) {
  try {
    if (!_owner)
      return fail(extension_protocol_error());
    if (_owner->first_failure || _owner->history_failure || _owner->assembly_failure)
      return fail(extension_protocol_error());
    if (_owner->attempted)
      return fail(extension_protocol_error());
    _owner->attempted = true;
    auto prepared     = _owner->vm.prepare_extension_invocation(name, options);
    if (!prepared)
      return fail(std::move(prepared.error().failure));
    _owner->prepared = std::move(*prepared);
    auto& pending    = _owner->prepared->pending_hosts;
    auto& runtime    = _owner->vm._owner->runtime;
    charge(runtime, pending.size() * sizeof(prepared_host_path));
    _owner->paths.reserve(pending.size());
    // All scalar and normalization checks finish before the first metadata action.
    for (auto& slot : pending) {
      std::string_view argument = slot.selector;
      std::string      expanded;
      if (slot.kind == extension_host_kind::root || slot.kind == extension_host_kind::base_root)
        argument = argument.substr(argument.find('=') + 1);
      if (slot.kind == extension_host_kind::workspace) {
        auto named = workspace_is_name(argument);
        if (!named)
          return fail({named.error(), "profile"});
        auto checked = *named ? validate_workspace_directory(workspace_directory) : validate_host_path(argument);
        if (!checked)
          return fail({checked.error(), "profile"});
        charge_extension_path(runtime, argument.size() + 16, *named && workspace_directory ? workspace_directory->size() : 0);
        auto result = expand_workspace(argument, workspace_directory);
        if (!result)
          return fail({result.error(), "profile"});
        expanded = std::move(*result);
        argument = expanded;
      }
      if (auto checked = validate_host_path_inputs(argument, explicit_cwd); !checked)
        return fail({checked.error(), "profile"});
      const auto cwd_bytes = !argument.starts_with('/') && explicit_cwd ? explicit_cwd->size() : 0;
      if (cwd_bytes)
        charge_extension_path(runtime, cwd_bytes);
      charge_extension_path(runtime, argument.size() + 1, cwd_bytes);
      auto path = prepare_host_path(argument, explicit_cwd);
      if (!path)
        return fail({path.error(), "profile"});
      if (slot.kind == extension_host_kind::workspace) {
        const auto parent = std::string_view(path->spelling).substr(0, path->spelling.rfind('/') + 1);
        charge(runtime, sizeof(std::string) + parent.size() + 1);
        _owner->workspace_parent = parent;
      }
      _owner->paths.push_back(std::move(*path));
    }
    constexpr std::string_view keys[]{"extension.workspace", "extension.registry",     "extension.root",
                                      "extension.base",      "extension.observations", "extension.ledger"};
    for (std::size_t i = 0; i < pending.size(); ++i) {
      auto                               kind = pending[i].kind;
      auto                               key  = keys[static_cast<std::size_t>(kind)];
      auto&                              path = _owner->paths[i].spelling;
      std::expected<void, profile_error> checked;
      if (kind == extension_host_kind::workspace) {
        checked = _owner->capture.admit_root("extension.workspace.root", _owner->workspace_parent);
        if (checked)
          checked = _owner->capture.admit_file(key, path, admission_file_role::declaration);
      } else if (kind == extension_host_kind::root || kind == extension_host_kind::base_root) {
        checked = _owner->capture.admit_root(key, path);
      } else {
        auto role = kind == extension_host_kind::registry       ? admission_file_role::registry
                    : kind == extension_host_kind::observations ? admission_file_role::observation
                                                                : admission_file_role::ledger;
        checked   = _owner->capture.admit_file(key, path, role);
      }
      if (!checked)
        return fail({checked.error().detail, checked.error().field});
    }
    auto verified = _owner->capture.verify();
    if (!verified)
      return fail({verified.error().detail, verified.error().field});
    _owner->admitted = true;
    return {};
  } catch (const diagnostic& error) {
    return fail({error, "profile"});
  } catch (const std::bad_alloc&) {
    return fail({external_resource(), "profile"});
  } catch (const std::length_error&) {
    return fail({external_resource(), "profile"});
  }
}
namespace {
using native_history_error = std::variant<registration_error, ledger::format_error, history::history_read_error>;
/// Borrowing adapter exists only within synchronous preparation; its duplicate never reopens the pathname.
class original_history_guard final : public ledger::format_read_guard {
public:
  explicit original_history_guard(profile_capture& capture) : _capture(capture) {
  }
  ~original_history_guard() override {
    if (_descriptor >= 0)
      ::close(_descriptor);
  }
  ledger::read_gate_status bind(int descriptor) override {
    try {
      return bind_impl(descriptor);
    } catch (const std::bad_alloc&) {
      return _status = ledger::read_gate_status::resource;
    } catch (const std::length_error&) {
      return _status = ledger::read_gate_status::resource;
    }
  }
  ledger::read_gate_status bind_impl(int descriptor) {
    if (_bound || _status != ledger::read_gate_status::ready)
      return _status = ledger::read_gate_status::refused;
    _bound       = true;
    auto checked = _capture.verify_file("extension.ledger", descriptor);
    if (!checked)
      return retain(std::move(checked.error()));
    _descriptor = ::fcntl(descriptor, F_DUPFD_CLOEXEC, 0);
    if (_descriptor < 0)
      return _status = ledger::read_gate_status::resource;
    return verify();
  }
  ledger::read_gate_status verify() override {
    try {
      return verify_impl();
    } catch (const std::bad_alloc&) {
      return _status = ledger::read_gate_status::resource;
    } catch (const std::length_error&) {
      return _status = ledger::read_gate_status::resource;
    }
  }
  bool bound() const noexcept {
    return _bound;
  }
  ledger::read_gate_status verify_impl() {
    if (_status != ledger::read_gate_status::ready)
      return _status;
    if (_descriptor < 0)
      return _status = ledger::read_gate_status::refused;
    auto checked = _capture.verify_file("extension.ledger", _descriptor);
    if (checked)
      checked = _capture.verify();
    if (!checked)
      return retain(std::move(checked.error()));
    return _status;
  }
  std::optional<profile_error> original;

private:
  ledger::read_gate_status retain(profile_error error) {
    original.emplace(std::move(error));
    return _status =
               original->detail.code == "resource_limit" ? ledger::read_gate_status::resource : ledger::read_gate_status::refused;
  }
  profile_capture&         _capture;
  int                      _descriptor = -1;
  bool                     _bound      = false;
  ledger::read_gate_status _status     = ledger::read_gate_status::ready;
};
} // namespace
std::expected<void, std::variant<registration_error, ledger::format_error, history::history_read_error>>
extension_invocation::prepare_history() {
  auto terminal = [&](native_history_error error) -> std::expected<void, native_history_error> {
    if (!_owner)
      return std::unexpected(std::move(error));
    if (!_owner->history_failure)
      _owner->history_failure.emplace(std::move(error));
    return std::unexpected(*_owner->history_failure);
  };
  try {
    if (!_owner)
      return terminal(extension_protocol_error());
    if (_owner->history_failure)
      return std::unexpected(*_owner->history_failure);
    if (_owner->assembly_failure)
      return std::unexpected(extension_protocol_error());
    if (_owner->first_failure) {
      // Install ownership before the first fallible error copy; later APIs borrow this same original.
      _owner->history_failure.emplace(std::in_place_type<registration_error>, std::move(*_owner->first_failure));
      _owner->first_failure.reset();
      return std::unexpected(*_owner->history_failure);
    }
    if (!_owner->admitted || (_owner->executed && _owner->vm._owner->command.extension_history.owner != this) ||
        _owner->history_attempted || !_owner->prepared->history_query)
      return terminal(extension_protocol_error());
    _owner->history_attempted = true;
    auto verified             = _owner->capture.verify();
    if (!verified)
      return terminal(registration_error{std::move(verified.error().detail), std::move(verified.error().field)});
    auto&                  selector = *_owner->prepared->history_query;
    history::history_query query;
    query.selector    = selector.selector == "revision"  ? history::history_selector::revision
                        : selector.selector == "state"   ? history::history_selector::state
                        : selector.selector == "request" ? history::history_selector::request
                                                         : history::history_selector::page;
    const auto parsed = std::from_chars(selector.id.data(), selector.id.data() + selector.id.size(), query.id);
    if (parsed.ec != std::errc{} || parsed.ptr != selector.id.data() + selector.id.size())
      return terminal(extension_protocol_error());
    query.limit                    = static_cast<std::size_t>(selector.limit);
    query.request_id               = selector.request_id;
    const prepared_host_path* path = nullptr;
    for (std::size_t i = 0; i < _owner->prepared->pending_hosts.size(); ++i)
      if (_owner->prepared->pending_hosts[i].kind == extension_host_kind::ledger)
        path = &_owner->paths[i];
    if (!path)
      return terminal(extension_protocol_error());
    auto                     guard  = std::make_shared<original_history_guard>(_owner->capture);
    ledger::read_gate_status status = ledger::read_gate_status::ready;
    // Every format/statement/transaction dies inside this scope, including on exceptions.
    auto read = [&]() -> std::expected<value_ptr, native_history_error> {
      try {
        auto file = ledger::open_existing_gated(path->spelling, guard);
        if (!file)
          return std::unexpected(std::move(file.error()));
        auto result = history::read_history(*file, query);
        status      = file->gate_status();
        if (!result)
          return std::unexpected(std::move(result.error()));
        return std::move(*result);
      } catch (const std::bad_alloc&) {
        return std::unexpected(registration_error{external_resource(), "profile"});
      } catch (const std::length_error&) {
        return std::unexpected(registration_error{external_resource(), "profile"});
      } catch (...) {
        return std::unexpected(extension_protocol_error());
      }
    }();
    // Original receipts are reverified after success AND failure, with no live SQLite resource.
    try {
      verified = _owner->capture.verify();
    } catch (const std::bad_alloc&) {
      if (guard->original)
        return terminal(registration_error{std::move(guard->original->detail), std::move(guard->original->field)});
      return terminal(registration_error{external_resource(), "profile"});
    } catch (const std::length_error&) {
      if (guard->original)
        return terminal(registration_error{std::move(guard->original->detail), std::move(guard->original->field)});
      return terminal(registration_error{external_resource(), "profile"});
    }
    if (guard->original)
      return terminal(registration_error{std::move(guard->original->detail), std::move(guard->original->field)});
    if (!verified)
      return terminal(registration_error{std::move(verified.error().detail), std::move(verified.error().field)});
    auto final_status = guard->bound() ? guard->verify() : ledger::read_gate_status::ready;
    if (guard->original)
      return terminal(registration_error{std::move(guard->original->detail), std::move(guard->original->field)});
    if (status != ledger::read_gate_status::ready || final_status != ledger::read_gate_status::ready) {
      const auto failure = status != ledger::read_gate_status::ready ? status : final_status;
      return terminal(registration_error{
          failure == ledger::read_gate_status::resource ? external_resource() : external_protocol(), "profile"});
    }
    if (!read)
      return terminal(std::move(read.error()));
    _owner->authenticated_history = std::move(*read);
    return {};
  } catch (const std::bad_alloc&) {
    return terminal(registration_error{external_resource(), "profile"});
  } catch (const std::length_error&) {
    return terminal(registration_error{external_resource(), "profile"});
  }
}
const value* extension_invocation::prepare_role() {
  auto& owner = *_owner;
  auto& vm    = *owner.vm._owner;
  auto& s     = vm.runtime;
  auto& c     = vm.command;
  if (owner.role_attempted || owner.first_failure || owner.history_failure || owner.assembly_failure || !owner.admitted ||
      !owner.executed || c.extension_history.owner != this || !c.extension_history.role_enabled ||
      !c.extension_history.context_identity || !c.extension_execution)
    return nullptr;
  owner.role_attempted = true;
  capture_session session(s.budget);
  auto            require = []<class T, class Error>(std::expected<T, Error> result) -> T {
    if (!result) {
      if constexpr (std::is_same_v<Error, diagnostic>)
        throw std::move(result.error());
      else
        throw registration_error{std::move(result.error().detail), std::move(result.error().field)};
    }
    if constexpr (!std::is_void_v<T>)
      return std::move(*result);
  };
  auto record = [&](std::initializer_list<std::pair<const std::string, value_ptr>> fields) {
    visit(s, 1);
    for (const auto& [key, child] : fields)
      charge(s, 256 + key.size() + 1);
    return command_record(std::map<std::string, value_ptr>(fields));
  };
  auto text = [&](std::string_view bytes) {
    visit(s, 1);
    charge(s, bytes.size() + 1);
    return command_text(std::string(bytes));
  };
  auto array = [&](std::vector<value_ptr> items) {
    visit(s, 1);
    charge(s, items.size() * sizeof(value_ptr) * 2);
    return command_array(std::move(items));
  };
  auto path = [&](extension_host_kind kind) -> const std::string& {
    for (std::size_t i = 0; i < owner.prepared->pending_hosts.size(); ++i)
      if (owner.prepared->pending_hosts[i].kind == kind)
        return owner.paths[i].spelling;
    throw external_protocol();
  };
  auto option = [&](std::string_view name) -> const extension_prepared_option& {
    for (const auto& entry : owner.prepared->options)
      if (entry.name == name)
        return entry;
    throw external_protocol();
  };
  auto seal = [&] {
    if (!owner.role_capture)
      owner.role_capture.emplace(require(session.finish_for_acceptance()));
  };
  auto semantic = [&](const value_ptr& envelope) {
    if (!envelope || envelope->kind != value_kind::record || envelope->fields.size() != 1 ||
        (envelope->fields.contains("error") == envelope->fields.contains("value")))
      throw external_protocol();
    if (envelope->fields.contains("error")) {
      owner.assembly_failure.emplace(assembly_error{envelope->fields.at("error")});
      return false;
    }
    return true;
  };
  try {
    require(owner.capture.verify());
    require(
        session.add_root("workspace", owner.workspace_parent, require(owner.capture.retain_root("extension.workspace.root"))));
    const auto& workspace_path = path(extension_host_kind::workspace);
    charge(s, workspace_path.size() + 1);
    auto file = workspace_path.substr(workspace_path.rfind('/') + 1);
    // Explicit selection establishes exact same-root/path correspondence before load.
    require(session.read("workspace", file, require(owner.capture.retain_file("extension.workspace"))));
    auto workspace = require(session.load(file)).root;
    auto routed =
        require(call_retained_value(vm.lua, c.shape_phases[8], record({{"file", text(file)}, {"workspace", workspace}})));
    if (semantic(routed)) {
      const auto& route     = routed->fields.at("value");
      const auto& source_id = route->fields.at("root")->fields.at("id")->string;
      const auto& mapping =
          owner.prepared->pending_hosts[std::get<extension_slot_index>(option("root").value).index.value()].selector;
      if (std::string_view(mapping).substr(0, mapping.find('=')) != source_id) {
        owner.assembly_failure.emplace(
            assembly_error{record({{"code", text("reference")},
                                   {"field", text("root")},
                                   {"message", text("Root mapping must match workspace source ID")}})});
      } else {
        auto spec_path = route->fields.at("spec");
        auto spec      = require(session.load(spec_path->string));
        charge(s, route->fields.at("jobs")->items.size() * sizeof(value_ptr) * 2);
        std::vector<value_ptr> jobs;
        jobs.reserve(route->fields.at("jobs")->items.size());
        for (const auto& job : route->fields.at("jobs")->items) {
          auto job_path = job->fields.at("file");
          auto loaded   = require(session.load(job_path->string));
          jobs.push_back(record({{"file", job_path}, {"value", loaded.root}}));
        }
        require(session.add_input("registry", path(extension_host_kind::registry),
                                  require(owner.capture.retain_file("extension.registry"))));
        auto registry = require(session.read("registry", "input.json"));
        if (option("observations").present) {
          require(session.add_input("observations", path(extension_host_kind::observations),
                                    require(owner.capture.retain_file("extension.observations"))));
          require(session.read("observations", "input.json"));
        }
        charge(s, 2 * (source_id.size() + 16));
        auto current = "current:" + source_id, base = "base:" + source_id;
        require(session.add_root(current, path(extension_host_kind::root), require(owner.capture.retain_root("extension.root"))));
        bool outputs = std::get<bool>(option("capture-outputs").value);
        if (outputs) {
          require(
              session.add_root(base, path(extension_host_kind::base_root), require(owner.capture.retain_root("extension.base"))));
          const auto& excluded = route->fields.at("root")->fields.at("excluded")->items;
          charge(s, 2 * excluded.size() * sizeof(std::string));
          std::vector<std::string> exclusions;
          exclusions.reserve(excluded.size());
          for (const auto& entry : excluded) {
            charge(s, 2 * (entry->string.size() + 1));
            exclusions.push_back(entry->string);
          }
          require(session.tree(current, exclusions));
          require(session.tree(base, std::move(exclusions)));
        }
        seal(); // All bytes/topology sealed before semantic consumers or custom F.
        auto snapshot    = require(owner.role_capture->snapshot());
        auto directories = require(owner.role_capture->directories());
        charge(s, snapshot->size() * sizeof(value_ptr) * 6 + directories->size() * sizeof(value_ptr) * 2);
        std::vector<value_ptr> retained, workspace_files, source_files, source_directories;
        retained.reserve(snapshot->size());
        workspace_files.reserve(snapshot->size());
        source_files.reserve(snapshot->size());
        source_directories.reserve(directories->size());
        for (const auto& captured : *snapshot) {
          retained.push_back(record({{"root", text(captured.root)},
                                     {"path", text(captured.path)},
                                     {"mode", text(captured.mode)},
                                     {"bytes", text(captured.bytes)}}));
          if (captured.root == "workspace")
            workspace_files.push_back(record({{"path", text(captured.path)}, {"bytes", text(captured.bytes)}}));
          if (captured.root.starts_with("current:") || captured.root.starts_with("base:"))
            source_files.push_back(record({{"root", text(captured.root)}, {"path", text(captured.path)}}));
        }
        for (const auto& directory : *directories)
          source_directories.push_back(record({{"root", text(directory.root)}, {"path", text(directory.path)}}));
        auto bundle  = record({{"workspace", record({{"file", text(file)}, {"value", workspace}})},
                               {"spec", record({{"file", spec_path}, {"value", spec.root}})},
                               {"jobs", array(std::move(jobs))},
                               {"registry", record({{"file", text("input.json")}, {"bytes", text(registry.bytes)}})},
                               {"files", array(std::move(workspace_files))}});
        auto capture = outputs ? record({{"status", text("captured")},
                                         {"files", array(std::move(source_files))},
                                         {"directories", array(std::move(source_directories))}})
                               : record({{"status", text("not_captured")}});
        charge(s, 6 * 320 + sizeof(value));
        std::map<std::string, value_ptr> fields{{"bundle", bundle},
                                                {"capture", capture},
                                                {"files", array(std::move(retained))},
                                                {"job", text(std::get<std::string>(option("job").value))},
                                                {"role", text(std::get<std::string>(option("role").value))}};
        if (option("budget-bytes").present) {
          visit(s, 1);
          fields.emplace("budget_bytes", command_integer(std::get<std::int32_t>(option("budget-bytes").value)));
        }
        owner.role_request = command_record(std::move(fields));
        auto prepared      = owner.vm.validate_profile_impl({}, {}, owner.role_request, this, &owner.role_basis);
        if (!prepared) {
          if (!owner.first_failure)
            owner.first_failure.emplace(std::move(prepared.error()));
        } else if (semantic(*prepared))
          owner.authenticated_role = (*prepared)->fields.at("value");
      }
    }
  } catch (registration_error& error) {
    if (!owner.first_failure && !owner.assembly_failure)
      owner.first_failure.emplace(std::move(error));
  } catch (diagnostic& error) {
    if (!owner.first_failure && !owner.assembly_failure)
      owner.first_failure.emplace(registration_error{std::move(error), "profile"});
  } catch (const std::bad_alloc&) {
    s.native_memory_failed = true;
    if (!owner.first_failure && !owner.assembly_failure)
      owner.first_failure.emplace(registration_error{external_resource(), "profile"});
  } catch (const std::length_error&) {
    s.native_memory_failed = true;
    if (!owner.first_failure && !owner.assembly_failure)
      owner.first_failure.emplace(registration_error{external_resource(), "profile"});
  }
  // Seal and verify even a routed semantic refusal. An earlier owned failure wins.
  try {
    seal();
    require(owner.role_capture->verify());
    require(owner.capture.verify());
  } catch (registration_error& error) {
    if (!owner.first_failure && !owner.assembly_failure)
      owner.first_failure.emplace(std::move(error));
  } catch (diagnostic& error) {
    if (!owner.first_failure && !owner.assembly_failure)
      owner.first_failure.emplace(registration_error{std::move(error), "profile"});
  }
  if (owner.first_failure || owner.assembly_failure || !owner.authenticated_role)
    return nullptr;
  validate(s, owner.authenticated_role, 1);
  return owner.authenticated_role.get();
}
std::expected<value_ptr, extension_execution_error> extension_invocation::execute() {
  auto failure = [&](diagnostic detail) -> std::expected<value_ptr, extension_execution_error> {
    if (!_owner)
      return std::unexpected(registration_error{std::move(detail), "catalog.commands"});
    if (_owner->history_failure)
      return std::visit(
          [](const auto& error) -> std::expected<value_ptr, extension_execution_error> { return std::unexpected(error); },
          *_owner->history_failure);
    if (_owner->assembly_failure)
      return std::unexpected(*_owner->assembly_failure);
    if (!_owner->first_failure) {
      auto& c = _owner->vm._owner->command;
      if (!c.shape_failure)
        c.shape_failure = registration_error{std::move(detail), std::move(c.registration_field)};
      _owner->first_failure = std::move(c.shape_failure);
    }
    return std::unexpected(*_owner->first_failure);
  };
  auto allocation_failure = [&]() -> std::expected<value_ptr, extension_execution_error> {
    if (_owner) {
      auto& s = _owner->vm._owner->runtime;
      auto& c = _owner->vm._owner->command;
      if (c.shape_failure || _owner->first_failure)
        return failure({});
      if (exhausted(s))
        return failure(command_resource(s));
      if (c.bridge_error)
        return failure(*c.bridge_error);
    }
    return failure(external_resource());
  };
  try {
    try {
      if (!_owner || _owner->first_failure || _owner->history_failure)
        return failure(external_protocol());
      auto& vm = *_owner->vm._owner;
      auto& c  = vm.command;
      auto& s  = vm.runtime;
      if (!_owner->executed && !c.shape_failure) {
        charge(s, 64);
        c.registration_field = "catalog.commands";
      }
      if (!_owner->admitted || _owner->executed || _owner->history_attempted)
        return failure(external_protocol());
      if (c.shape_failure || c.registering || c.invoked || s.seal_failed)
        return failure(external_protocol());
      const external_entry* selected       = nullptr;
      std::size_t           original_index = command_catalog_size;
      for (auto& entry : c.external_entries) {
        if (entry.kind != 0)
          continue;
        ++original_index;
        if (text_field(entry.descriptor, "name") == _owner->prepared->command) {
          selected = &entry;
          break;
        }
      }
      if (!selected)
        return failure(external_protocol());
      charge(s, 128);
      c.registration_field       = std::format("catalog.commands[{}]", original_index);
      _owner->executed           = true;
      const auto& capabilities   = selected->descriptor->fields.at("capabilities")->items;
      const auto  has_capability = [&](std::string_view name) {
        return std::ranges::any_of(capabilities, [&](const auto& v) { return v->string == name; });
      };
      const bool role_enabled       = has_capability("role_context_read_v1");
      const bool history_enabled    = has_capability("history_read_v1");
      const bool projection_enabled = has_capability("projection_apply_v1");
      if ((role_enabled && history_enabled) ||
          capabilities.size() != static_cast<std::size_t>(role_enabled) + static_cast<std::size_t>(history_enabled) +
                                     static_cast<std::size_t>(projection_enabled) ||
          (projection_enabled && !role_enabled && !history_enabled))
        return failure(external_protocol());
      if ((!capabilities.empty() && !history_enabled && !role_enabled) || selected->function_reference == LUA_NOREF)
        return failure(external_protocol());
      auto verified = verify();
      if (!verified)
        return std::unexpected(std::move(verified.error()));
      if (exhausted(s))
        return failure(command_resource(s));
      // Every constructed map/node/text is reserved before allocation. Input
      // transport and output conversion additionally charge their complete passes.
      auto record = [&](std::map<std::string, value_ptr> fields) { return command_record(std::move(fields)); };
      auto text   = [&](std::string_view bytes) {
        visit(s, 1);
        charge(s, bytes.size() + 1);
        return command_text(std::string(bytes));
      };
      auto scalar = [&](const extension_option_value& option) -> value_ptr {
        return std::visit(
            [&](const auto& value) -> value_ptr {
              using T = std::decay_t<decltype(value)>;
              if constexpr (std::is_same_v<T, std::string>)
                return text(value);
              else if constexpr (std::is_same_v<T, std::int32_t>) {
                visit(s, 1);
                return command_integer(value);
              } else if constexpr (std::is_same_v<T, bool>) {
                visit(s, 1);
                return command_boolean(value);
              } else if constexpr (std::is_same_v<T, extension_slot_index>) {
                if (!history_enabled && !role_enabled)
                  throw external_protocol();
                visit(s, 1);
                return command_boolean(value.index.has_value()); // Present host slots become opaque userdata before freezing.
              } else
                throw external_protocol();
            },
            option);
      };
      // Six records and their fixed map slots; each option has two slots and
      // two key copies. This conservative reservation precedes even map staging.
      charge(s, 6 * (sizeof(value) + 512) + 12 * 256);
      std::map<std::string, value_ptr> options, present;
      for (auto& option : _owner->prepared->options) {
        charge(s, 2 * (256 + option.name.size() + 1));
        options.emplace(option.name, scalar(option.value));
        visit(s, 1);
        present.emplace(option.name, command_boolean(option.present));
      }
      visit(s, 1);
      auto version = command_integer(1);
      charge(s, _owner->prepared->command.size() + 1);
      c.name          = _owner->prepared->command;
      auto invocation = record({{"version", std::move(version)},
                                {"command", text(c.name)},
                                {"options", record(std::move(options))},
                                {"present", record(std::move(present))}});
      s.input         = record({{"invocation", std::move(invocation)}, {"context", record({})}});
      validate(s, s.input, 1);
      begin_invocation(s, c);
      c.invoked                              = true;
      c.extension_execution                  = true;
      c.shape_function                       = selected->function_reference;
      c.extension_history.enabled            = history_enabled || role_enabled;
      c.extension_history.role_enabled       = role_enabled;
      c.extension_history.projection_enabled = projection_enabled;
      c.extension_history.project            = [](extension_invocation& original, std::string_view id) -> const value* {
        auto& owner = *original._owner;
        auto& vm    = *owner.vm._owner;
        auto& c     = vm.command;
        auto& s     = vm.runtime;
        if (owner.first_failure || owner.history_failure || owner.assembly_failure || exhausted(s) || c.bridge_error)
          return nullptr;
        const external_entry* selected = nullptr;
        std::size_t           ordinal  = 0;
        for (auto& entry : c.external_entries) {
          if (entry.kind != 2)
            continue;
          ++ordinal;
          if (text_field(entry.descriptor, "id") == id) {
            selected = &entry;
            break;
          }
        }
        const bool  role          = c.extension_history.role_enabled;
        const auto& authenticated = role ? owner.authenticated_role : owner.authenticated_history;
        if (!selected || text_field(selected->descriptor, "input") != (role ? "role_context_v1" : "history_snapshot_v1") ||
            selected->function_reference == LUA_NOREF || !authenticated || (role && !owner.role_basis))
          throw external_protocol();
        // Stage owning fallbacks before entering the selected projection phase.
        // The fixed allowance covers both diagnostics, the location and pinned
        // string capacity/terminators. It is cumulative reservation, not a quota.
        charge(s, sizeof(registration_error) + sizeof(diagnostic) + 4 * 128);
        registration_error projection_failure{external_resource(), std::format("catalog.projections[{}]", ordinal)};
        diagnostic         projection_protocol = external_protocol();
        std::optional<projection_definition> definition;
        value_ptr                            custom;
        try {
          auto resolved = owner.vm.resolve_projection_definition_impl(id, ordinal, &original, &projection_failure);
          if (!resolved) {
            if (!c.shape_failure)
              c.shape_failure.emplace(std::move(resolved.error()));
            return nullptr;
          }
          definition.emplace(std::move(*resolved));
          auto projected = call_retained_projection(vm.lua, selected->function_reference, authenticated);
          if (!projected) {
            if (!c.shape_failure)
              c.shape_failure.emplace(registration_error{std::move(projected.error()), std::move(projection_failure.field)});
            return nullptr;
          }
          custom = std::move(*projected);
        } catch (const std::bad_alloc&) {
          s.native_memory_failed = true;
          if (!c.shape_failure) {
            if (c.bridge_error)
              projection_failure.detail = std::move(*c.bridge_error);
            else if (s.conversion_failure)
              projection_failure.detail = std::move(*s.conversion_failure);
            c.shape_failure.emplace(std::move(projection_failure));
          }
          return nullptr;
        } catch (const std::length_error&) {
          s.native_memory_failed = true;
          if (!c.shape_failure) {
            if (c.bridge_error)
              projection_failure.detail = std::move(*c.bridge_error);
            else if (s.conversion_failure)
              projection_failure.detail = std::move(*s.conversion_failure);
            c.shape_failure.emplace(std::move(projection_failure));
          }
          return nullptr;
        } catch (diagnostic& error) {
          if (!c.shape_failure)
            c.shape_failure.emplace(
                registration_error{error.code == "resource_limit" ? std::move(error) : std::move(projection_protocol),
                                   std::move(projection_failure.field)});
          return nullptr;
        } catch (...) {
          if (!c.shape_failure)
            c.shape_failure.emplace(registration_error{std::move(projection_protocol), std::move(projection_failure.field)});
          return nullptr;
        }
        value_ptr base_sections, budget;
        if (role) {
          // Trusted base generation is outside selected-F attribution. It uses
          // only the original private basis and shares cumulative nested limits.
          auto prepared = call_retained_value(vm.lua, c.shape_phases[9], owner.role_basis);
          if (!prepared)
            throw prepared.error();
          if ((*prepared)->fields.contains("error")) {
            owner.assembly_failure.emplace(assembly_error{(*prepared)->fields.at("error")});
            return nullptr;
          }
          const auto& material = (*prepared)->fields.at("value");
          base_sections        = material->fields.at("base");
          budget               = material->fields.at("budget_bytes");
        } else {
          const auto& ledger_id  = text_field(owner.authenticated_history, "ledger");
          const auto& last_event = text_field(owner.authenticated_history, "last_event");
          charge(s, 8192 + ledger_id.size() + last_event.size());
          auto base_text = std::format(
              "## recorded_history\n\nRecorded history; ledger {}; last event {}. No current-policy reinterpretation.\n",
              ledger_id, last_event);
          base_sections = command_array({command_record({{"id", command_text("recorded_history")},
                                                         {"text", command_text(std::move(base_text))},
                                                         {"mandatory", command_boolean(true)}})});
          budget        = command_integer(1048576);
        }
        charge(s, 8192 + definition->id.size() + definition->revision.size() + definition->digest.size());
        auto input = command_record(
            {{"base", std::move(base_sections)}, {"custom", custom->fields.at("sections")}, {"budget_bytes", std::move(budget)}});
        validate(s, input, 1);
        auto assembled = call_retained_assembly(vm.lua, c.shape_phases[6], input);
        if (!assembled)
          throw assembled.error();
        if ((*assembled)->fields.contains("error")) {
          owner.assembly_failure.emplace(assembly_error{(*assembled)->fields.at("error")});
          return nullptr;
        }
        const auto& output             = (*assembled)->fields.at("value");
        owner.authenticated_projection = command_record({{"type", command_text("projection")},
                                                         {"id", command_text(std::move(definition->id))},
                                                         {"revision", command_text(std::move(definition->revision))},
                                                         {"definition", command_text(std::move(definition->digest))},
                                                         {"text", output->fields.at("text")},
                                                         {"sections", output->fields.at("sections")},
                                                         {"omitted", output->fields.at("omitted")}});
        validate(s, owner.authenticated_projection, 1);
        return owner.authenticated_projection.get();
      };
      c.extension_history.owner = this;
      c.extension_history.read  = [](extension_invocation& original) -> const value* {
        if (original._owner->vm._owner->command.extension_history.role_enabled)
          return original.prepare_role();
        auto prepared = original.prepare_history();
        if (!prepared)
          return nullptr;
        auto& owner = *original._owner;
        validate(owner.vm._owner->runtime, owner.authenticated_history, 1);
        return owner.authenticated_history.get();
      };
      struct cleanup {
        command_vm::implementation& owner;
        ~cleanup() {
          owner.command.extension_history  = {};
          owner.runtime.preserving_aliases = false;
          owner.runtime.push_memo          = 0;
          owner.runtime.input.reset();
          release_shape_temporaries(owner.lua);
        }
      } release{vm};
      auto l = vm.lua;
      lua_pushcfunction(l, execute_shape);
      auto status = lua_pcall(l, 0, LUA_MULTRET, 0);
      if (role_enabled) {
        // Verification runs after either handler branch, including caught failures.
        // Existing owned failures retain chronological precedence over a later seal refusal.
        auto retain = [&](diagnostic detail) {
          if (!_owner->first_failure && !_owner->assembly_failure && !c.shape_failure && !c.extension_history.refused)
            _owner->first_failure.emplace(registration_error{std::move(detail), "profile"});
        };
        auto receipts = _owner->capture.verify();
        if (!receipts)
          retain(std::move(receipts.error().detail));
        if (_owner->role_capture) {
          auto captured = _owner->role_capture->verify();
          if (!captured)
            retain(std::move(captured.error()));
        }
      }
      if (_owner->history_failure)
        return std::visit(
            [](const auto& error) -> std::expected<value_ptr, extension_execution_error> { return std::unexpected(error); },
            *_owner->history_failure);
      if (_owner->assembly_failure)
        return std::unexpected(*_owner->assembly_failure);
      if (_owner->first_failure)
        return failure({});
      if (c.bridge_error)
        return failure(*c.bridge_error);
      if (c.extension_history.refused)
        return failure(c.extension_history.resource_refusal ? command_resource(s) : external_protocol());
      if (exhausted(s) || status == LUA_ERRMEM)
        return failure(command_resource(s));
      if (status || lua_gettop(l) != 1)
        return failure(external_protocol());
      // The original return remains rooted while raw validation, bounded owned
      // conversion and final reserved Glaze encoding inspect its aliases/backings.
      lua_pushcfunction(l, scan_extension_response);
      lua_pushvalue(l, 1);
      status = lua_pcall(l, 1, 1, 0);
      if (exhausted(s) || status == LUA_ERRMEM)
        return failure(command_resource(s));
      if (status || !lua_toboolean(l, -1))
        return failure(external_protocol());
      if (lua_isinteger(l, -1) && lua_tointeger(l, -1) == 2)
        return failure(external_resource());
      lua_pop(l, 1);
      auto response = convert(l, 1, 1);
      auto encoded  = codec_encode_reserved(
          response, {&s, [](void* owner, const codec_reservation& reservation) -> std::expected<void, diagnostic> {
                       try {
                         charge(*static_cast<state*>(owner), reservation.total_bytes);
                         return {};
                       } catch (diagnostic& error) {
                         return std::unexpected(std::move(error));
                       }
                     }});
      if (!encoded)
        return failure(encoded.error().detail.code == "resource_limit" ? std::move(encoded.error().detail) : external_protocol());
      verified = verify();
      if (!verified)
        return std::unexpected(std::move(verified.error()));
      return response;
    } catch (diagnostic& error) {
      return failure(error.code == "resource_limit" ? std::move(error) : external_protocol());
    } catch (const std::bad_alloc&) {
      return allocation_failure();
    } catch (const std::length_error&) {
      return allocation_failure();
    } catch (...) {
      return failure(external_protocol());
    }
  } catch (const std::bad_alloc&) {
    return allocation_failure();
  } catch (const std::length_error&) {
    return allocation_failure();
  }
}

std::expected<void, registration_error> extension_invocation::verify() {
  try {
    if (!_owner || _owner->first_failure || _owner->history_failure || _owner->assembly_failure || !_owner->admitted)
      return fail(extension_protocol_error());
    auto checked = _owner->capture.verify();
    if (!checked)
      return fail({checked.error().detail, checked.error().field});
    if (_owner->role_capture) {
      auto role = _owner->role_capture->verify();
      if (!role)
        return fail({std::move(role.error()), "profile"});
    }
    return {};
  } catch (const std::bad_alloc&) {
    return fail({external_resource(), "profile"});
  } catch (const std::length_error&) {
    return fail({external_resource(), "profile"});
  }
}
} // namespace planar::engine
