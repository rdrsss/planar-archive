"""Reversible diagnostic class probe: removed allocation windows and actual outer new refusal."""
from pathlib import Path
import hashlib,json,os,shlex,subprocess,sys,time
R=Path.cwd();B=R/'build/rewrite-debug';O=R/'tests/native/v5-02-x05-a3-c04-evidence'/sys.argv[1];O.mkdir(exist_ok=False);rows=[]
def run(label,args,mode=None,kind=2,index=10):
 env=os.environ.copy();env.update(C04_MODE=mode or '',C04_KIND=str(kind),C04_INDEX=str(index))
 t=time.monotonic();p=subprocess.run(list(map(str,args)),cwd=B,env=env,text=True,capture_output=True);(O/(label+'.log')).write_text(p.stdout+p.stderr);rows.append(dict(label=label,argv=list(map(str,args)),mode=mode,kind=kind,index=index,exit_code=p.returncode,seconds=time.monotonic()-t));(O/'commands.json').write_text(json.dumps(rows,indent=2)+'\n');print(label,p.returncode,flush=True);assert p.returncode==0,(label,p.returncode,p.stdout+p.stderr)
entries=json.loads((B/'compile_commands.json').read_text())
def compile(label,suffix,text):
 p=O/(label+'.cpp');p.write_text(text);a=shlex.split(next(e['command']for e in entries if e['file'].endswith(suffix)));a[a.index('-c')+1]=str(p);a[a.index('-o')+1]=str(O/(label+'.o'));run('compile-'+label,a);return O/(label+'.o')
base=(R/'src/engine/runtime/runtime.cpp').read_text()
subs=[('#include <cstdlib>','#include <cstdlib>\nextern "C" void c04_window(const char*,std::size_t,std::size_t);\nextern "C" void c04_disarm();\nextern "C" bool c04_phase(const char*);'),
('    c.pending_seal_location = {kind, c.registration_attempts[kind] + 1};','''    c04_window("register",kind,c.registration_attempts[kind]+1);
    c.pending_seal_location = {kind, c.registration_attempts[kind] + 1};
    c04_disarm();
    if(c04_phase("register-failure") && kind==static_cast<std::size_t>(std::atoi(std::getenv("C04_KIND"))) && c.pending_seal_location.index==static_cast<std::size_t>(std::atoi(std::getenv("C04_INDEX")))) context(l).native_bytes=memory_max;'''),
('      c.pending_seal_location = {e.kind, indices[e.kind]};','''      c04_window("collect",e.kind,indices[e.kind]);
      c.pending_seal_location = {e.kind, indices[e.kind]};
      c04_disarm();'''),
('  auto checked = history::validate_catalog(catalog, shipped, external);','''  auto checked = history::validate_catalog(catalog, shipped, external);
  if(c.active_module==2 && c04_phase("unmatched")) checked=std::unexpected(history::catalog_error{history::catalog_error_code::invalid_catalog,"catalog.commands[3]"});
  if(c.active_module==2 && c04_phase("pure-global")) checked=std::unexpected(history::catalog_error{history::catalog_error_code::resource_limit,"catalog"});'''),
('  lua_pushvalue(l, 2);\n  commands(l).external_entries.back().function_reference = luaL_ref(l, LUA_REGISTRYINDEX);','''  if(c04_phase("raw-reference") && commands(l).pending_seal_location.kind==2 && commands(l).pending_seal_location.index==10) {
    context(l).exhausted=true;
    return luaL_error(l,"controlled_reference_status");
  }
  lua_pushvalue(l, 2);
  commands(l).external_entries.back().function_reference = luaL_ref(l, LUA_REGISTRYINDEX);'''),
('    auto code = checked.error().code;','''    c04_window("remap",static_cast<std::size_t>(std::atoi(std::getenv("C04_KIND"))),static_cast<std::size_t>(std::atoi(std::getenv("C04_INDEX"))));
    auto code = checked.error().code;'''),
('    if (code == history::catalog_error_code::identity_conflict)','''    c04_disarm();
    if (code == history::catalog_error_code::identity_conflict)'''),
('  auto field = std::format("catalog.{}[{}]",','''  c04_window("outer",location.kind,location.index);
  auto field = std::format("catalog.{}[{}]",'''),
('  c.registration_field.swap(field);','  c04_disarm();\n  c.registration_field.swap(field);'),
('    c.external_entries.push_back({LUA_NOREF, LUA_NOREF, kind, c.active_module, {}});','''    if(c04_phase("sticky") && kind==2 && c.pending_seal_location.index==10) {
      c.external_entries.shrink_to_fit();
      c04_window("sticky",kind,c.pending_seal_location.index);
    }
    c.external_entries.push_back({LUA_NOREF, LUA_NOREF, kind, c.active_module, {}});
    c04_disarm();'''),
('  c.registration_field.swap(catalog_location);','''  c04_window("global-install",2,10);
  c.registration_field.swap(catalog_location);
  c04_disarm();'''),
('  auto embedded = history::prepare_stdlib(c.originals, c.separator, c.assembly);','''  c04_window("global",2,10);
  auto embedded = history::prepare_stdlib(c.originals, c.separator, c.assembly);
  c04_disarm();'''),
('  auto catalog = profile_catalog(c);','''  if(c.active_module==2 && c04_phase("prevalidation")) c04_window("prevalidation",2,10);
  auto catalog = profile_catalog(c);
  c04_disarm();'''),
('    if (lua)\n      lua_close(lua);','    if (lua) {lua_close(lua);if(runtime.memory!=0)std::abort();std::println("Lua cleanup live=0");}')]
text=base
for a,b in subs:assert text.count(a)==1,(a,text.count(a));text=text.replace(a,b)
rev=text
for a,b in reversed(subs):assert rev.count(b)==1;rev=rev.replace(b,a)
assert rev==base
(O/'delta.json').write_text(json.dumps(dict(original_sha256=hashlib.sha256(base.encode()).hexdigest(),reverse_identical=True,substitutions=[dict(before=a,after=b,count=1)for a,b in subs],policy='Removed register/collect/remap windows arm global new around production operations and require zero actual requests. Outer/prevalidation windows refuse the next actual request. register-failure tightens existing native counter to exercise real charge refusal, not natural exhaustion. Sticky probes tighten private vector capacity before real new refusal. Raw-reference modes are controlled Lua status/exhaustion adapters after descriptor ref retention; unmatched/global pure results are controlled adapter probes, not natural validator failures.'),indent=2)+'\n')
main=r'''import std;
import planar.engine.runtime;
using namespace planar::engine;
bool armed=false;std::size_t refused=0,requests=0,arms=0;
std::string_view mode(){return std::getenv("C04_MODE");}
extern "C" bool c04_phase(const char* phase){return mode().find(phase)!=std::string_view::npos;}
extern "C" void c04_window(const char* site,std::size_t kind,std::size_t index){if(kind!=static_cast<std::size_t>(std::atoi(std::getenv("C04_KIND")))||index!=static_cast<std::size_t>(std::atoi(std::getenv("C04_INDEX"))))return;auto s=std::string_view(site);if((mode().starts_with("removed-")&&mode().substr(8)==s)||(mode().starts_with("outer-")&&s=="outer")||(mode()=="prevalidation"&&s==mode())||(c04_phase("sticky")&&s=="sticky")||(mode()==s&&(s=="global"||s=="global-install"))){armed=true;++arms;}}
extern "C" void c04_disarm(){armed=false;}
void* operator new(std::size_t n){if(armed){armed=false;++requests;refused=n;throw std::bad_alloc();}if(auto p=std::malloc(n))return p;throw std::bad_alloc();}
void operator delete(void* p) noexcept{std::free(p);}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete[](void* p) noexcept{::operator delete(p);}
int main(){int kind=std::atoi(std::getenv("C04_KIND")),index=std::atoi(std::getenv("C04_INDEX"));auto p=std::make_shared<profile_closure>();p->profile={"profile","profile.json","0644",R"({"version":1,"modules":["one.lua","two.lua"]})"};
auto entry=[](int k,int i,bool bad,bool missing){auto d=k==0?std::format("name='c{}',family='{}',handler_id='custom.c{}',options=engine.array{{}},help='',response_kind='extension_v1',capabilities=engine.array{{}}",i,bad?"bad":"extension_read_v1",i):k==1?std::format("id='s{}',abi='{}',revision='v1',stages='optional',help=''",i,bad?"bad":"shape_v1"):std::format("id='p{}',abi='{}',revision='v1',input='role_context_v1',help=''",i,bad?"bad":"projection_v1");if(missing)d.erase(0,d.find(',')+1);return std::format("registration.{}({{{}}},function()end);",k==0?"add":k==1?"shape":"projection",d);};
std::string first;for(int k=0;k<3;++k)first+=entry(k,1,false,false);p->modules.push_back({"modules","one.lua","0644",first});std::string source=entry((kind+1)%3,2,false,false);int total=index-(kind==0?5:0);bool remap=c04_phase("remap"),collect=c04_phase("collect")&&!mode().starts_with("removed-");for(int i=2;i<=total+(remap&&index<64?1:0);++i){auto e=entry(kind,i,remap&&i==total,collect&&i==total);if(c04_phase("sticky")&&i==total){e.replace(0,24,"pcall(registration.projection,");e+="pcall(registration.shape,{},function()end);error('private later failure');";}source+=e;}p->modules.push_back({"modules","two.lua","0644",source});auto vm=command_vm::create(p);std::println("mode={} kind={} index={} arms={} actual_new_requests={} refused_bytes={}",mode(),kind,index,arms,requests,refused);
if(mode()=="success"||mode()=="removed-register"||mode()=="removed-collect"||mode()=="global-install")return vm&&requests==0&&(mode()=="success"||arms==1)?0:1;
if(vm)return 2;std::println("{} / {} / {}",vm.error().detail.code,vm.error().detail.message,vm.error().field);
if(mode()=="unmatched")return requests==0&&vm.error().detail.code=="internal"&&vm.error().field=="profile.modules[2]"?0:10;
if(mode()=="pure-global")return requests==0&&vm.error().detail.code=="resource_limit"&&vm.error().field=="catalog"?0:11;
if(mode()=="raw-reference")return requests==0&&vm.error().detail.code=="resource_limit"&&vm.error().detail.message=="Lua allocation exceeded"&&vm.error().field=="catalog.projections[10]"?0:12;
if(mode()=="outer-raw-reference")return requests==1&&refused==32&&vm.error().detail.code=="resource_limit"&&vm.error().field=="profile.modules[2]"?0:13;
if(mode()=="global")return requests==1&&refused>0&&vm.error().detail.code=="resource_limit"&&vm.error().field=="catalog"?0:7;
if(mode()=="sticky")return requests==1&&refused>0&&vm.error().detail.code=="resource_limit"&&vm.error().field=="catalog.projections[10]"?0:8;
if(mode()=="outer-sticky")return requests==2&&refused==32&&vm.error().detail.code=="resource_limit"&&vm.error().field=="profile.modules[2]"?0:9;
if(mode()=="removed-remap")return arms==1&&requests==0&&vm.error().detail.code=="internal"&&vm.error().field==std::format("catalog.{}[{}]",kind==0?"commands":kind==1?"shapes":"projections",index)?0:3;
if(mode().starts_with("outer-")||mode()=="prevalidation"){
 if(requests==0&&mode().starts_with("outer-")&&kind!=2)return arms==1&&vm.error().field==std::format("catalog.{}[{}]",kind==0?"commands":"shapes",index)?0:6;
 return arms==1&&requests==1&&refused>0&&vm.error().detail.code=="resource_limit"&&vm.error().detail.message=="External profile resource limit exceeded"&&vm.error().field=="profile.modules[2]"?0:4;}
auto field=std::format("catalog.{}[{}]",kind==0?"commands":kind==1?"shapes":"projections",index);return requests==0&&vm.error().field==field?0:5;}
'''
po=compile('main','/runtime/commands.t.cpp',main);ro=compile('runtime','/runtime/runtime.cpp',text)
a=shlex.split(subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=B,text=True).splitlines()[-1].split(' && ')[1]);libs=[x for x in a[a.index('src/engine/libplanar_runtime.a'):]if'catch2'not in x and x!='src/engine/libplanar_runtime.a'];binary=O/'probe';run('link',[a[0],'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++',po,ro,*libs,'-o',binary])
for kind in range(3):
 for index in [10,64]:
  for mode in ['success','removed-register','removed-collect','removed-remap','control-remap','control-collect','control-register-failure','outer-remap','outer-collect','outer-register-failure']:
   run(f'run-{kind}-{index}-{mode}',[binary],mode,kind,index)
for mode in ['prevalidation','global-install','global','sticky','outer-sticky','unmatched','pure-global','raw-reference','outer-raw-reference']:run('run-'+mode,[binary],mode)
print('69 class scenarios pass; removed windows make zero new requests; remaining projection render refusals use real requests and module fallback; short command/shape render windows honestly record zero requests')
