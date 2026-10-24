import std;
import planar.kernel.runtime;
int main(int argc, char** argv) {
 using namespace planar::kernel;
 auto v=std::make_shared<value>(); v->kind=value_kind::record;
 auto r=run_trusted(argc > 1 ? argv[1] : "caught", v);
 std::println("accepted={}", bool(r));
 if (r) std::println("boolean={}", (*r)->boolean);
 else std::println("code={} message={} limit={}",r.error().code,r.error().message,r.error().limit);
}
