// Controlled reversible-copy observers, never alternate production dispatch.
#include <cstdlib>
#include <new>
import std;
import planar.engine.runtime;
using namespace planar::engine;
namespace {
thread_local bool fail_next_allocation = false;
}
extern "C" void b2c_probe_arm() {
  fail_next_allocation = true;
}
void* operator new(std::size_t size) {
  if (fail_next_allocation) {
    fail_next_allocation = false;
    throw std::bad_alloc{};
  }
  if (auto p = std::malloc(size ? size : 1))
    return p;
  throw std::bad_alloc{};
}
void operator delete(void* p) noexcept {
  std::free(p);
}
void* operator new[](std::size_t size) {
  return ::operator new(size);
}
void operator delete[](void* p) noexcept {
  ::operator delete(p);
}
int main() {
  auto mode        = std::string_view(std::getenv("B2C_MODE"));
  auto profile     = std::make_shared<profile_closure>();
  profile->profile = {"profile", "profile.json", "0644", R"({"version":1,"modules":["shape.lua"]})"};
  std::string body =
      mode == "bridge" ? "assert(not pcall(saved,D,function()end));return {}"
      : mode == "instructions"
          ? "pcall(function() pcall(function() while true do end end) end); return {}"
          : "local x=engine.array{}; for i=1,200 do x[i]=i end; assert(input.job==input.spec); assert(not pcall(function() "
            "D.help='mutated' end));return engine.object{data=x,alias=input.job,revision=D.revision}";
  profile->modules.push_back(
      {"modules", "shape.lua", "0644",
       "local saved=registration.shape; local D={id='custom',abi='shape_v1',revision='final',stages='optional',help='shape'}; "
       "registration.shape(D,function(input) " +
           body + " end)"});
  auto vm = command_vm::create(profile);
  if (!vm)
    return 1;
  auto version     = std::make_shared<value>();
  version->kind    = value_kind::integer;
  version->integer = 1;
  auto shared      = std::make_shared<value>();
  shared->kind     = value_kind::record;
  auto input       = std::make_shared<value>();
  input->kind      = value_kind::record;
  input->fields    = {{"version", version}, {"job", shared}, {"spec", shared}, {"assignments", shared}, {"reviewer", shared}};
  // Mutating the caller's original snapshot cannot alter retained source/F.
  profile->modules.clear();
  profile->profile.bytes.clear();
  for (int i = 0; i < (mode == "success" ? 100 : 1); ++i) {
    auto response = vm->invoke_shape("custom", input);
    if (mode == "success") {
      if (!response || response->result->fields.at("revision")->string != "final")
        return 2;
    } else {
      if (response || response.error().detail.code != (mode == "bridge" ? "internal" : "resource_limit") ||
          response.error().field != "catalog.shapes[1]")
        return 3;
      auto again = vm->invoke_shape("missing", input);
      if (again || again.error().field != response.error().field ||
          again.error().detail.message != response.error().detail.message)
        return 4;
      std::println("terminal {} {} {}", response.error().detail.code, response.error().detail.limit, response.error().field);
    }
  }
}
