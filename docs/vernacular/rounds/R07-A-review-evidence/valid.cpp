import std;
import planar.kernel.runtime;
int main() {
 using namespace planar::kernel;
 auto v=std::make_shared<value>(); v->kind=value_kind::record;
 auto r=run_trusted("copy",v);
 std::println("returned={}",bool(r));
 if (!r) std::println("code={} message={} limit={}",r.error().code,r.error().message,r.error().limit);
}
