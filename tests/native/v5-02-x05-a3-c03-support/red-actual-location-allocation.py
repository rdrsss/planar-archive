from pathlib import Path
import hashlib,json,os,shlex,subprocess,time
R=Path.cwd();B=R/'build/rewrite-debug';O=R/'tests/native/v5-02-x05-a3-c03-evidence/red-actual-location-allocation';O.mkdir(exist_ok=False);rows=[]
def run(label,args,mode=None,expect=0):
 env=os.environ.copy()
 if mode:env['C02_ALLOC']=mode
 t=time.monotonic();p=subprocess.run(list(map(str,args)),cwd=B,env=env,text=True,capture_output=True);(O/(label+'.log')).write_text(p.stdout+p.stderr);rows.append(dict(label=label,argv=list(map(str,args)),mode=mode,exit_code=p.returncode,expected_exit=expect,seconds=time.monotonic()-t));(O/'commands.json').write_text(json.dumps(rows,indent=2)+'\n');assert p.returncode==expect,(label,p.returncode,p.stdout+p.stderr)
entries=json.loads((B/'compile_commands.json').read_text())
def compile(label,suffix,text):
 p=O/(label+'.cpp');p.write_text(text);args=shlex.split(next(e['command']for e in entries if e['file'].endswith(suffix)));args[args.index('-c')+1]=str(p);args[args.index('-o')+1]=str(O/(label+'.o'));run('compile-'+label,args);return O/(label+'.o')
base=(R/'src/engine/runtime/runtime.cpp').read_text();subs=[]
subs.append(('#include <cstdlib>','#include <cstdlib>\nextern "C" void c02_allocation_arm();\nextern "C" void c02_allocation_disarm();'))
a='''      c.registration_field = std::format("catalog.{}[{}]",
                                         e.kind == 0   ? "commands"
                                         : e.kind == 1 ? "shapes"
                                                       : "projections",
                                         indices[e.kind]);'''
b='''      if(e.kind==2 && indices[e.kind]==10)c02_allocation_arm();
'''+a+'''
      c02_allocation_disarm();'''
assert base.count(a)==2 # mapping and old final descriptor collection; instrument mapping only
start=base.index('bool module_seal_location(');end=base.index('// Audit raw roots',start);part=base[start:end];assert part.count(a)==1
text=base[:start]+part.replace(a,b)+base[end:]
c='    if (lua)\n      lua_close(lua);';d='    if (lua) {lua_close(lua);if(runtime.memory!=0)std::abort();std::println("Lua cleanup live=0");}'
assert text.count(c)==1;text=text.replace(c,d)
for x,y in subs:assert text.count(x)==1;text=text.replace(x,y)
reverse=text
for x,y in reversed(subs):reverse=reverse.replace(y,x)
assert reverse.replace(d,c).replace(b,a)==base
(O/'delta.json').write_text(json.dumps(dict(source_sha256=hashlib.sha256(base.encode()).hexdigest(),copy_sha256=hashlib.sha256(text.encode()).hexdigest(),reverse_identical=True,substitutions=[dict(before=x,after=y)for x,y in subs]+[dict(before=a,after=b,count=1,scope='module_seal_location only'),dict(before=c,after=d)],fault='Actual next global operator new refusal, armed immediately before production projection[10] location format/assignment; disarmed on successful assignment. No exception directly inserted.'),indent=2)+'\n')
main=r'''import std;
import planar.engine.runtime;
using namespace planar::engine;
bool allocation_armed=false;std::size_t allocation_refused=0;
extern "C" void c02_allocation_arm(){allocation_armed=std::string_view(std::getenv("C02_ALLOC"))=="actual";}
extern "C" void c02_allocation_disarm(){allocation_armed=false;}
void* operator new(std::size_t n){if(allocation_armed){allocation_armed=false;allocation_refused=n;throw std::bad_alloc();}if(auto p=std::malloc(n))return p;throw std::bad_alloc();}
void operator delete(void* p) noexcept{std::free(p);}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete[](void* p) noexcept{::operator delete(p);}
int main(){auto p=std::make_shared<profile_closure>();p->profile={"profile","profile.json","0644",R"({"version":1,"modules":["one.lua"]})"};std::string source;for(int i=1;i<=10;++i)source+=std::format("registration.projection({{id='p{}',abi='projection_v1',revision='v1',input='role_context_v1',help=''}},function()end);",i);p->modules.push_back({"modules","one.lua","0644",source});auto vm=command_vm::create(p);auto mode=std::string_view(std::getenv("C02_ALLOC"));if(mode=="success"){std::println("success {} refused_bytes={}",bool(vm),allocation_refused);return vm&&allocation_refused==0?0:1;}if(vm){std::println("unexpected success refused_bytes={}",allocation_refused);return 2;}std::println("{}: {} / {} / {}; refused_bytes={}; expected catalog.projections[10]",mode,vm.error().detail.code,vm.error().detail.message,vm.error().field,allocation_refused);return allocation_refused>0&&vm.error().detail.code=="resource_limit"&&vm.error().detail.message=="External profile resource limit exceeded"&&vm.error().field=="catalog.projections[10]"?0:1;}
'''
po=compile('main','/runtime/commands.t.cpp',main);ro=compile('runtime','/runtime/runtime.cpp',text)
link=shlex.split(subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=B,text=True).splitlines()[-1].split(' && ')[1]);start=link.index('src/engine/libplanar_runtime.a');libs=[a for a in link[start:]if 'catch2'not in a and a!='src/engine/libplanar_runtime.a'];binary=O/'probe';run('link',[link[0],'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++',po,ro,*libs,'-o',binary]);run('success',[binary],'success');run('actual',[binary],'actual',1)
