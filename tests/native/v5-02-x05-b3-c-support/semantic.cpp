// Private observer drives the actual captured-profile semantic operation.
#include "../../../src/engine/runtime/external_registration.t.cpp"
#include <cstdlib>
#include <new>
namespace {
bool        hash_observing = false;
std::size_t hash_actual = 0, hash_reserved = 0;
struct item {
  int         type;
  const void* identity;
  long long   number;
  bool        operator==(const item&) const = default;
};
using members = std::map<const void*, item>;
std::array<members, 2>     baseline_members, current_members;
std::map<int, item>        baseline_refs, current_refs;
std::array<std::size_t, 5> dispatches{};
std::size_t                boundaries = 0, operations = 0;
std::size_t                last_nodes = 0, last_native = 0, last_instructions = 0, settled_heap = 0;
std::array<std::size_t, 5> staging_allocations{}, staging_sizes{};
int                        watched_stage = 0;
std::size_t                watched_bytes = 0, watched_requests = 0;
std::string_view           mode() {
  return std::getenv("R01_MODE");
}
} // namespace
void* operator new(std::size_t bytes) {
  if (hash_observing)
    hash_actual += bytes;
  if (watched_stage) {
    if (bytes != watched_bytes)
      std::abort();
    ++watched_requests;
    ++staging_allocations[watched_stage];
  }
  if (auto p = std::malloc(bytes ? bytes : 1))
    return p;
  throw std::bad_alloc{};
}
void operator delete(void* p) noexcept {
  std::free(p);
}
void* operator new[](std::size_t bytes) {
  return ::operator new(bytes);
}
void operator delete[](void* p) noexcept {
  ::operator delete(p);
}
extern "C" void r01_watch(int stage, std::size_t bytes, bool begin) {
  if (begin) {
    if (watched_stage)
      std::abort();
    if (bytes) {
      if (staging_sizes[stage] && staging_sizes[stage] != bytes)
        std::abort();
      staging_sizes[stage] = bytes;
    }
    watched_stage    = stage;
    watched_bytes    = bytes;
    watched_requests = 0;
  } else {
    if (watched_requests != (bytes ? 1 : 0))
      std::abort();
    watched_stage = 0;
  }
}
namespace {
std::pair<std::size_t, std::size_t> independent_hash_size(const value* v) {
  std::size_t nodes = 1, bytes = v->kind == value_kind::boolean ? 1 : 9;
  if (v->kind == value_kind::string)
    bytes += v->string.size();
  for (auto& item : v->items) {
    auto [n, b] = independent_hash_size(item.get());
    nodes += n;
    bytes += b;
  }
  for (auto& [key, item] : v->fields) {
    auto [n, b] = independent_hash_size(item.get());
    nodes += 1 + n;
    bytes += 9 + key.size() + b;
  }
  return {nodes, bytes};
}
} // namespace
extern "C" void r01_hash(int op, std::size_t before, std::size_t after, const void* input, std::size_t n) {
  std::size_t           expected;
  constexpr std::size_t alignment = 16, digest_storage = 48;
  auto                  frame = [&](std::size_t payload, std::size_t domain) {
    return 4 * (payload + 54 + domain) + 4 * alignment + 2 * (18 + domain) + 3 * alignment + digest_storage;
  };
  if (op == 0) {
    auto [nodes, bytes] = independent_hash_size(static_cast<const value*>(input));
    expected            = (nodes + 4) * 512 + frame(bytes, n);
  } else {
    auto domain = op == 1 ? 13u : 11u;
    expected    = 6 * 512 + 2 * n + alignment + frame(9 + n, domain);
  }
  if (after - before != expected)
    std::abort();
  hash_reserved  = expected;
  hash_actual    = 0;
  hash_observing = true;
}
extern "C" void r01_hash_end() {
  hash_observing = false;
  if (hash_actual > hash_reserved)
    std::abort();
  std::println("hash-allocation actual={} reserved={} verified", hash_actual, hash_reserved);
}
extern "C" bool r01_direct_hash(const value_ptr* input);
TEST_CASE("B3 C pinned aggregate hash allocation bound covers long suffix deep and wide inputs", "[b3c-hash]") {
  for (std::size_t n : {22, 23, 24, 31, 32, 47, 48, 63, 64, 255, 256, 4095, 4096, 65535, 65536, 1048576}) {
    auto input =
        record_value({{"a", string_value(std::string(n, 'a'))}, {"z", array_value({string_value(std::string(257, 'z'))})}});
    REQUIRE(r01_direct_hash(&input));
  }
  value_ptr deep = string_value("leaf");
  for (int i = 0; i < 250; ++i)
    deep = array_value({deep});
  REQUIRE(r01_direct_hash(&deep));
  std::map<std::string, value_ptr> fields;
  for (int i = 0; i < 8192; ++i)
    fields.emplace(std::format("key{:05}", i), string_value(std::string(31, 'w')));
  auto wide = record_value(std::move(fields));
  REQUIRE(r01_direct_hash(&wide));
}

extern "C" void r01_dispatch(int phase) {
  ++dispatches[phase];
}
extern "C" void r01_member(int slot, const void* key, int type, const void* identity, long long number) {
  if (!key || !current_members[slot].emplace(key, item{type, identity, number}).second)
    std::abort();
}
extern "C" void r01_reference(int ref, int type, const void* identity) {
  if (!identity || (type != 5 && type != 6) || !current_refs.emplace(ref, item{type, identity, 0}).second)
    std::abort();
}
extern "C" void r01_boundary(bool final, bool active, bool ready, std::size_t live, std::size_t instructions, std::size_t nodes,
                             std::size_t native) {
  if (!boundaries) {
    baseline_members = current_members;
    baseline_refs    = current_refs;
    std::println("initial live={} tags={} backings={} retained_refs={}", live, baseline_members[0].size(),
                 baseline_members[1].size(), baseline_refs.size());
  } else if (current_members != baseline_members || current_refs != baseline_refs)
    std::abort();
  ++boundaries;
  current_members = {};
  current_refs    = {};
  if (!final)
    return;
  if (active || instructions < last_instructions || nodes < last_nodes || native < last_native)
    std::abort();
  if (mode() == "success" && operations && (nodes <= last_nodes || native <= last_native))
    std::abort();
  if (mode() == "success" && operations >= 2) {
    if (!settled_heap)
      settled_heap = live;
    else if (live != settled_heap)
      std::abort();
  }
  last_nodes        = nodes;
  last_native       = native;
  last_instructions = instructions;
  ++operations;
  std::println(
      "operation={} exact_refs={} tags={} backings={} active={} baseline_ready={} live={} instructions={} nodes={} native={}",
      operations, baseline_refs.size(), baseline_members[0].size(), baseline_members[1].size(), active, ready, live, instructions,
      nodes, native);
}
TEST_CASE("R01 full semantic lifetimes preserve exact membership retained roots and live heap", "[r01-semantic]") {
  semantic_fixture f;
  auto             source = semantic_shape();
  if (mode() == "semantic" || mode() == "wrapper")
    source = callable_shape("return engine.object{ok=false,error=engine.object{code='schema',message='no',field='shape'}}");
  if (mode() == "execution")
    source = callable_shape("error('ordinary failure')");
  if (mode() == "instructions")
    source = semantic_shape("local n=0;for i=1,500000 do n=n+i end");
  auto vm =
      command_vm::create(profile({source, command() + ";registration.projection({id='unused_view',abi='projection_v1',revision='"
                                                      "v1',input='role_context_v1',help=''},function()return {} "
                                                      "end);registration.shape({id='unused_shape',abi='shape_v1',revision='v1',"
                                                      "stages='optional',help=''},function()error('must stay unselected')end)"}));
  REQUIRE(vm);
  REQUIRE(baseline_refs.size() == 17); // Five shipped, four phases, and all four external D/F pairs.
  auto input = semantic_validation(f.bundle);
  if (mode().starts_with("error"))
    input =
        semantic_validation(changed(f.bundle, "registry", changed(f.bundle->fields.at("registry"), "bytes", string_value("{"))));
  int  repeats = (mode() == "success" ? 25 : mode() == "instructions" ? 60 : mode() == "native" ? 200 : 1);
  bool refused = false;
  for (int i = 0; i < repeats; ++i) {
    auto result = vm->select_profile(selection_request(input));
    if (mode() == "success") {
      REQUIRE(result);
      REQUIRE((*result)->fields.contains("value"));
    } else if (mode() == "semantic" || mode() == "error") {
      REQUIRE(result);
      REQUIRE((*result)->fields.contains("error"));
    } else if ((mode() == "instructions" || mode() == "native") && result)
      continue;
    else {
      REQUIRE_FALSE(result);
      std::println("terminal code={} limit={} maximum={} field={}", result.error().detail.code, result.error().detail.limit,
                   result.error().detail.maximum, result.error().field);
      CHECK(result.error().detail.code == (mode() == "execution" ? "internal" : "resource_limit"));
      auto retry = vm->select_profile({});
      REQUIRE_FALSE(retry);
      CHECK(retry.error().detail.code == result.error().detail.code);
      CHECK(retry.error().detail.message == result.error().detail.message);
      CHECK(retry.error().detail.limit == result.error().detail.limit);
      CHECK(retry.error().detail.maximum == result.error().detail.maximum);
      CHECK(retry.error().field == result.error().field);
      refused = true;
      break;
    }
  }
  if (mode() != "success" && mode() != "semantic" && mode() != "error")
    CHECK(refused);
  CHECK(operations > 0);
  if (mode() == "success") {
    CHECK(dispatches[0] == 25);
    CHECK(dispatches[1] == 50);
    CHECK(dispatches[2] == 25);
    CHECK(dispatches[3] == 25);
    CHECK(dispatches[4] == 50);
  }
  std::println("staging counts node={} map={} string={} wrapper={}; request bytes node={} map={} string={} wrapper={}",
               staging_allocations[1], staging_allocations[2], staging_allocations[3], staging_allocations[4], staging_sizes[1],
               staging_sizes[2], staging_sizes[3], staging_sizes[4]);
  if (mode() == "wrapper")
    CHECK(dispatches[4] == 1);
  if (mode() == "error-node")
    CHECK(staging_allocations[1] == 0);
  if (mode() == "error-node-exact")
    CHECK(staging_allocations[1] == 1);
  if (mode() == "error-map")
    CHECK(staging_allocations[2] == 0);
  if (mode() == "error-string")
    CHECK(staging_allocations[3] == 0);
  if (mode() == "wrapper")
    CHECK(staging_allocations[4] == 0);
  if (mode() == "error") {
    CHECK(staging_allocations[1] == 6);
    CHECK(staging_allocations[2] == 5);
    CHECK(staging_allocations[3] == 2);
  }
  if (mode() == "semantic")
    CHECK(staging_allocations[4] == 1);
  if (mode() == "parser" || mode() == "conversion" || mode() == "snapshot" || mode() == "lua" || mode() == "nodes" ||
      mode().starts_with("error"))
    CHECK(dispatches[4] == 0);
}
