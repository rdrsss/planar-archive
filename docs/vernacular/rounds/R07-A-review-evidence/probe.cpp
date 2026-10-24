import std;
import planar.kernel.runtime;
int main(int argc, char**) {
  using namespace planar::kernel;
  auto v=std::make_shared<value>(); v->kind=value_kind::immaterial;
  auto child=std::make_shared<value>(); child->kind=value_kind::boolean; child->boolean=true;
  v->items.push_back(child);
  auto r=run_trusted("copy",v);
  std::println("accepted_nontext_immaterial={}",bool(r));
  if (!r) std::println("{} {}",r.error().code,r.error().message);
  return argc == 0;
}
