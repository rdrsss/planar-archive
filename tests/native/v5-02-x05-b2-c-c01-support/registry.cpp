// Private reversible observer: exact retained references across owner lifetime.
#include <cstdlib>
import std;
import planar.engine.runtime;
using namespace planar::engine;
struct observation {
  int         reference, type;
  const void* identity;
};
std::array<observation, 64> baseline{};
std::size_t                 count = 0, cursor = 0, boundaries = 0;
extern "C" void             c01_reference(int reference, int type, const void* identity) {
  if (!boundaries)
    baseline[count++] = {reference, type, identity};
  else {
    if (cursor >= count || baseline[cursor].reference != reference || baseline[cursor].type != type ||
        baseline[cursor].identity != identity)
      std::abort();
    ++cursor;
  }
  if (!identity || (type != 5 && type != 6))
    std::abort();
}
extern "C" void c01_boundary() {
  if (boundaries && cursor != count)
    std::abort();
  ++boundaries;
  cursor = 0;
  std::println("boundary={} exact_retained_references={}", boundaries, count);
}
int main() {
  auto mode  = std::string_view(std::getenv("C01_MODE"));
  auto p     = std::make_shared<profile_closure>();
  p->profile = {"profile", "profile.json", "0644", R"({"version":1,"modules":["one.lua","two.lua"]})"};
  auto shape = [](std::string id, std::string body) {
    return "local D={id='" + id +
           "',abi='shape_v1',revision='v1',stages='optional',help=''};registration.shape(D,function(input) " + body + " end);";
  };
  std::string command = "registration.add({name='inspect',family='extension_read_v1',handler_id='custom.inspect',options=engine."
                        "array{},help='',response_kind='extension_v1',capabilities=engine.array{}},function()return {} end);";
  std::string projection = "registration.projection({id='view',abi='projection_v1',revision='v1',input='role_context_v1',help=''}"
                           ",function()return {} end);";
  p->modules.push_back({"modules", "one.lua", "0644", command + shape("unselected", "return input.job") + projection});
  p->modules.push_back(
      {"modules", "two.lua", "0644",
       shape("selected",
             mode == "execution"
                 ? "error('ordinary failure')"
                 : "assert(input.job==input.spec);local s=engine.array{1,2};return engine.object{alias=input.job,scratch=s}") +
           shape("other", "return {}")});
  if (mode == "embedded") {
    auto vm = command_vm::create();
    if (!vm || count != 5)
      return 1;
    return 0;
  }
  auto vm = command_vm::create(p);
  if (!vm || count != 15)
    return 2;
  auto v        = std::make_shared<value>();
  v->kind       = value_kind::integer;
  v->integer    = 1;
  auto empty    = std::make_shared<value>();
  empty->kind   = value_kind::record;
  auto input    = std::make_shared<value>();
  input->kind   = value_kind::record;
  input->fields = {{"version", v}, {"job", empty}, {"spec", empty}, {"assignments", empty}, {"reviewer", empty}};
  int calls     = mode == "success" ? 25 : 1;
  for (int i = 0; i < calls; ++i) {
    auto r = vm->invoke_shape("selected", input);
    if (mode == "success") {
      if (!r)
        return 3;
    } else {
      if (r || r.error().detail.code != (mode == "execution" ? "internal" : "resource_limit"))
        return 4;
      auto again = vm->invoke_shape("selected", input);
      if (again || again.error().field != r.error().field)
        return 5;
    }
  }
  if (boundaries != static_cast<std::size_t>(calls + 1))
    return 6;
}
