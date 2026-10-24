#include <sys/stat.h>
#include <unistd.h>
import std;
import planar.kernel.capture;
using namespace planar::kernel;
void show(std::string label, const std::expected<void, diagnostic>& r) {std::cout << label << ": " << (r ? "success" : r.error().code + "/" + r.error().limit) << '\n';}
int main() {
char pattern[]="/tmp/planar-r05-probe-XXXXXX"; auto p=std::filesystem::canonical(::mkdtemp(pattern));
auto put=[&](std::string n,std::string s){std::ofstream(p/n,std::ios::binary)<<s;};
put("a","old");
{capture_session c;show("root",c.add_root("r",p));show("tree",c.tree("r"));c.before_verification([&]{put("a",std::string(16777217,'x'));});auto r=c.finish();std::cout<<"grew beyond quota final status: "<<(r?"success":r.error().code+"/"+r.error().limit)<<'\n';}
std::filesystem::remove(p/"a");
std::filesystem::create_symlink("/etc/passwd",p/"excluded");
{capture_session c;show("excluded link root",c.add_root("r",p));show("excluded link tree",c.tree("r",{"excluded"}));auto r=c.finish();std::cout<<"excluded link finish: "<<(r?"success":r.error().code)<<'\n';}
for(auto source:{"return planar.spec{a=planar.file(planar.md('x'))}","return planar.spec{a=planar.md(planar.file('x'))}","return planar.spec{a=planar.md(planar.md('x'))}","return planar.spec{a=planar.file(planar.file('x'))}"}) {auto r=load_declaration(source);std::cout<<source<<": "<<(r?"success":r.error().code)<<'\n';}
std::filesystem::remove_all(p);
}
