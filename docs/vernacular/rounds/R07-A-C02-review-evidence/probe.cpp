import std;
import planar.kernel.runtime;
int main(int argc,char** argv) { using namespace planar::kernel; auto v=std::make_shared<value>(); v->kind=value_kind::record; auto r=run_trusted(argc>1?argv[1]:"ordinary",v); std::println("accepted={}",bool(r)); if(r) std::println("kind={} boolean={} string={}",int((*r)->kind),(*r)->boolean,(*r)->string); else std::println("code={} limit={} message={}",r.error().code,r.error().limit,r.error().message); }
