// Private observer drives the actual captured-profile semantic operation.
#include "../../../src/engine/runtime/external_registration.t.cpp"
#include <cstdlib>
#include <new>
namespace {
struct item {
  int         type;
  const void* identity;
  long long   number;
  bool        operator==(const item&) const = default;
};
using members = std::map<const void*, item>;
std::array<members, 2>     baseline_members, current_members;
std::map<int, item>        baseline_refs, current_refs;
std::array<std::size_t, 4> dispatches{};
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
  REQUIRE(baseline_refs.size() == 16); // Five shipped, three phases, and all four external D/F pairs.
  auto input = semantic_validation(f.bundle);
  if (mode().starts_with("error"))
    input =
        semantic_validation(changed(f.bundle, "registry", changed(f.bundle->fields.at("registry"), "bytes", string_value("{"))));
  int  repeats = (mode() == "success" ? 25 : mode() == "instructions" ? 60 : mode() == "native" ? 200 : 1);
  bool refused = false;
  for (int i = 0; i < repeats; ++i) {
    auto result = vm->validate_profile(input);
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
      auto retry = vm->validate_profile({});
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
  std::println("staging counts node={} map={} string={} wrapper={}; request bytes node={} map={} string={} wrapper={}",
               staging_allocations[1], staging_allocations[2], staging_allocations[3], staging_allocations[4], staging_sizes[1],
               staging_sizes[2], staging_sizes[3], staging_sizes[4]);
  if (mode() == "wrapper")
    CHECK(dispatches[3] == 1);
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
    CHECK(dispatches[3] == 0);
}
