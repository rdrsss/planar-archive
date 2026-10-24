"""C03 reversible production copies: real native refusal, real Lua allocation/
stack refusal under controlled limits, and explicit throwing allocator adaptation.
No claimed natural exhaustion or exception at a nonallocating selector operation.
"""
from pathlib import Path
import hashlib,json,os,shlex,subprocess,sys,time
R=Path.cwd();B=R/'build/rewrite-debug';O=R/'tests/native/v5-02-x05-a3-c03-evidence'/sys.argv[1];O.mkdir(exist_ok=False);rows=[]
def run(label,args,mode=None):
 env=os.environ.copy()
 if mode:env['C03_MODE']=mode
 t=time.monotonic();p=subprocess.run(list(map(str,args)),cwd=B,env=env,text=True,capture_output=True);(O/(label+'.log')).write_text(p.stdout+p.stderr);rows.append(dict(label=label,argv=list(map(str,args)),mode=mode,exit_code=p.returncode,seconds=time.monotonic()-t));(O/'commands.json').write_text(json.dumps(rows,indent=2)+'\n');assert p.returncode==0,(label,p.returncode,p.stdout+p.stderr)
entries=json.loads((B/'compile_commands.json').read_text())
def compile(label,suffix,text):
 p=O/(label+'.cpp');p.write_text(text);a=shlex.split(next(e['command']for e in entries if e['file'].endswith(suffix)));a[a.index('-c')+1]=str(p);a[a.index('-o')+1]=str(O/(label+'.o'));run('compile-'+label,a);return O/(label+'.o')
base=(R/'src/engine/runtime/runtime.cpp').read_text();subs=[]
subs.append(('#include <cstdlib>','#include <cstdlib>\nextern "C" void c03_arm();\nextern "C" void c03_disarm();\nextern "C" void c03_lua_allocation(std::size_t);'))
subs.append(('void* allocate(void* ud, void* p, std::size_t old, std::size_t size) {','''bool c03_lua_target(state& s);
void* allocate(void* ud, void* p, std::size_t old, std::size_t size) {'''))
subs.append(('  auto result = std::realloc(p, size);','''  if(size>old && c03_lua_target(s)) {
    c03_lua_allocation(size);
    if(std::string_view(std::getenv("C03_MODE"))=="memory") {s.exhausted=true;return nullptr;}
  }
  auto result = std::realloc(p, size);'''))
subs.append(('command_state& commands(lua_State* l) {','''bool c03_lua_target(state& s) {
  return s.command && s.command->active_module==2 && s.command->pending_seal_location.kind==2 && s.command->pending_seal_location.index==10;
}
command_state& commands(lua_State* l) {'''))
subs.append(('    c.pending_seal_location = {e.kind, indices[e.kind]};','''    if(e.kind==2 && indices[e.kind]==10 && std::string_view(std::getenv("C03_MODE"))=="removed")c03_arm();
    c.pending_seal_location = {e.kind, indices[e.kind]};
    c03_disarm();'''))
# Real remaining production string allocation. Disabled fault twin uses same writer.
subs.append(('  auto field = std::format("catalog.{}[{}]",','''  if(location.kind==2 && location.index==10 && std::string_view(std::getenv("C03_MODE"))=="outer")c03_arm();
  auto field = std::format("catalog.{}[{}]",'''))
subs.append(('  c.registration_field.swap(field);','  c03_disarm();\n  c.registration_field.swap(field);'))
subs.append(('  locations.reserve(c.profile->modules.size());','''  if(std::string_view(std::getenv("C03_MODE"))=="staging")c03_arm();
  locations.reserve(c.profile->modules.size());
  c03_disarm();'''))
# A real active-frame overflow check, and a controlled Lua error after partial
# table freezing. These are bounded protocol/resource probes, not host failures.
subs.append(('  freeze_stack(l, 12);\n  lua_pushvalue(l, root);','''  if(c03_lua_target(context(l)) && std::string_view(std::getenv("C03_MODE"))=="stack") {
    if(!lua_checkstack(l,2048))std::abort();lua_settop(l,2048);
  }
  freeze_stack(l, 12);
  lua_pushvalue(l, root);'''))
subs.append(('      lua_setmetatable(l, object);','''      lua_setmetatable(l, object);
      if(c03_lua_target(context(l)) && std::string_view(std::getenv("C03_MODE"))=="status")luaL_error(l,"controlled_c03_partial_freeze");'''))
# Inspect first failure without consuming pending token; retry actual private
# protected module entry before outer adaptation. Verify no publication occurred.
subs.append(('      lua_pushcfunction(l, fn);\n      auto status = lua_pcall(l, 0, 0, 0);','''      lua_rawgeti(l,LUA_REGISTRYINDEX,sealed_index);const void* before=lua_topointer(l,-1);lua_pop(l,1);
      lua_pushcfunction(l, fn);
      auto status = lua_pcall(l, 0, 0, 0);
      if(sealing && c.active_module==2 && s.seal_failed) {
        lua_rawgeti(l,LUA_REGISTRYINDEX,sealed_index);bool unchanged=before==lua_topointer(l,-1);lua_pop(l,1);
        if(!unchanged)std::abort();auto token=c.pending_seal_location;
        int old_top=lua_gettop(l);lua_pushcfunction(l,seal_profile_module);int retry=lua_pcall(l,0,0,0);lua_settop(l,old_top);
        if(retry==LUA_OK || !s.seal_failed || c.pending_seal_location.kind!=token.kind || c.pending_seal_location.index!=token.index)std::abort();
        std::println("terminal retry retains token; identity set unchanged");
      }'''))
subs.append(('    if (lua)\n      lua_close(lua);','''    if (lua) {lua_close(lua);if(runtime.memory!=0)std::abort();std::println("Lua cleanup live=0");}'''))
text=base
for a,b in subs:assert text.count(a)==1,(a,text.count(a));text=text.replace(a,b)
rev=text
for a,b in reversed(subs):assert rev.count(b)==1;rev=rev.replace(b,a)
assert rev==base
(O/'delta.json').write_text(json.dumps(dict(original_sha256=hashlib.sha256(base.encode()).hexdigest(),copy_sha256=hashlib.sha256(text.encode()).hexdigest(),reverse_identical=True,substitutions=[dict(before=a,after=b,count=1)for a,b in subs],policy='outer/staging use actual global new refusal; removed arms only the trivial token assignment and observes zero requests; memory refuses real Lua growth; stack fills active frame to trigger real check; status is controlled partial-freeze Lua protocol failure; native modes use controlled throwing allocator on a real Lua growth request, not a claim that realloc throws C++ exceptions'),indent=2)+'\n')
main=r'''import std;
import planar.engine.runtime;
using namespace planar::engine;
bool armed=false;std::size_t refused=0,attempted=0,lua_attempted=0,arms=0;
std::string_view mode(){return std::getenv("C03_MODE");}
extern "C" void c03_arm(){armed=true;++arms;}
extern "C" void c03_disarm(){armed=false;}
extern "C" void c03_lua_allocation(std::size_t n){if(mode()=="memory"||mode()=="native-badalloc"||mode()=="native-unexpected")lua_attempted=n;if(mode()=="native-badalloc")throw std::bad_alloc();if(mode()=="native-unexpected")throw std::runtime_error("private allocator exception");}
void* operator new(std::size_t n){if(armed){armed=false;++attempted;refused=n;throw std::bad_alloc();}if(auto p=std::malloc(n))return p;throw std::bad_alloc();}
void operator delete(void* p) noexcept{std::free(p);}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete[](void* p) noexcept{::operator delete(p);}
int main(){auto p=std::make_shared<profile_closure>();p->profile={"profile","profile.json","0644",R"({"version":1,"modules":["one.lua","two.lua"]})"};
auto entry=[](int i,std::string_view f){return std::format("registration.projection({{id='p{}',abi='projection_v1',revision='v1',input='role_context_v1',help=''}},{});",i,f);};p->modules.push_back({"modules","one.lua","0644",entry(1,"function()end")});std::string source="local n=0;";for(int i=2;i<=10;++i)source+=entry(i,i==10&&(mode()=="outer"||mode()=="outer-control")?"function() n=n+1 end":"function()end");p->modules.push_back({"modules","two.lua","0644",source});
auto vm=command_vm::create(p);std::println("mode={} arms={} actual_new_requests={} refused_bytes={} lua_growth_bytes={}",mode(),arms,attempted,refused,lua_attempted);
if(mode()=="success"||mode()=="removed")return vm&&attempted==0&&refused==0&&(mode()!="removed"||arms==2)?0:1;
if(vm)return 2;std::println("{} / {} / {}",vm.error().detail.code,vm.error().detail.message,vm.error().field);
auto code=mode()=="outer-control"||mode()=="status"||mode()=="native-unexpected"?"internal":"resource_limit";
auto message=mode()=="memory"?"Lua allocation exceeded":mode()=="stack"?"Lua stack exceeded":code==std::string_view("internal")?"Invalid external profile protocol":"External profile resource limit exceeded";
auto field=mode()=="staging"?"profile":mode()=="outer"?"profile.modules[2]":"catalog.projections[10]";
return vm.error().detail.code==code&&vm.error().detail.message==message&&vm.error().field==field&&(mode()!="outer"||refused==32)&&(mode()!="staging"||refused>0)&&((mode()!="memory"&&mode()!="native-badalloc"&&mode()!="native-unexpected")||lua_attempted>0)?0:1;}
'''
po=compile('main','/runtime/commands.t.cpp',main);ro=compile('runtime','/runtime/runtime.cpp',text)
a=shlex.split(subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=B,text=True).splitlines()[-1].split(' && ')[1]);libs=[x for x in a[a.index('src/engine/libplanar_runtime.a'):]if'catch2'not in x and x!='src/engine/libplanar_runtime.a'];binary=O/'probe';run('link',[a[0],'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++',po,ro,*libs,'-o',binary])
for mode in ['success','removed','outer-control','outer','staging','memory','stack','status','native-badalloc','native-unexpected']:run('run-'+mode,[binary],mode)
print('All ten allocation/location scenarios pass; cleanup zero; actual and controlled policies recorded separately')
