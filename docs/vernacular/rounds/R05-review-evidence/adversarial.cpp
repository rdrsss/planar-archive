#include <sys/stat.h>
#include <unistd.h>
import std;
import planar.kernel.capture;
using namespace planar::kernel;
int main() {
int checks=0,failures=0;
auto check=[&](bool ok,std::string name){++checks;if(!ok){++failures;std::cout<<"FAIL "<<name<<'\n';}};
for(auto source:{"return planar.spec{a=planar.file(planar.md('x'))}","return planar.spec{a=planar.md(planar.file('x'))}","return planar.spec{a=planar.md(planar.md('x'))}","return planar.spec{a=planar.file(planar.file('x'))}","return planar.spec(nil)","return planar.md()","return planar.list(1)","return planar.immaterial({})"}) {auto r=load_declaration(source);check(!r&&r.error().code=="schema",source);}
for(auto source:{"local x=planar.spec{a={b=1}}; x.a.b=2; return x","local x=planar.spec{a=1}; planar.spec=x; return x","_ENV={}; return planar.spec{}"}) {auto r=load_declaration(source);check(!r&&r.error().code=="capability_denied",source);}
for(auto source:{"return planar.spec{s='\\192\\128'}","return planar.spec{s='\\237\\160\\128'}","return planar.spec{s='\\244\\144\\128\\128'}","return planar.spec{s='\\128'}","return planar.spec{s='\\000'}"}) {auto r=load_declaration(source);check(!r&&r.error().code=="schema",source);}
char pattern[]="/tmp/planar-r05-adv-XXXXXX";auto p=std::filesystem::canonical(::mkdtemp(pattern));
for(bool text:{false,true}) {std::ofstream(p/"a")<<"old";capture_session c;check(bool(c.add_root("r",p)),"register root");check(bool(c.read("r","a",text)),"initial retained read");c.before_verification([&]{std::ofstream(p/"a")<<std::string((text?1048576:16777216)+1,'x');});auto r=c.finish();check(!r&&r.error().code=="capture_changed",std::string(text?"text":"binary")+" quota-crossing change must be capture_changed (actual "+(r?"success":r.error().code+"/"+r.error().limit)+")");}
std::filesystem::remove_all(p);std::cout<<checks<<" executed; "<<failures<<" failed; 0 skipped\n";return failures?1:0;
}
