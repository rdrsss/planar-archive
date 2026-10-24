from pathlib import Path
import hashlib,json,os,shlex,subprocess,sys,time
R=Path.cwd(); B=R/'build/rewrite-debug'; O=R/'tests/native/v5-02-x05-a3-c05-evidence/red-reference5';O.mkdir(exist_ok=False);rows=[]
def run(label,args,mode=None):
 env=os.environ.copy();env['REVIEW_REF_MODE']=mode or '';t=time.monotonic();p=subprocess.run(list(map(str,args)),cwd=B,env=env,text=True,capture_output=True);(O/(label+'.log')).write_text(p.stdout+p.stderr);rows.append(dict(label=label,argv=list(map(str,args)),mode=mode,exit_code=p.returncode,seconds=time.monotonic()-t));(O/'commands.json').write_text(json.dumps(rows,indent=2)+'\n');print(label,p.returncode,flush=True);return p.returncode
entries=json.loads((B/'compile_commands.json').read_text())
def compile(label,suffix,text):
 p=O/(label+'.cpp');p.write_text(text);a=shlex.split(next(e['command']for e in entries if e['file'].endswith(suffix)));a[a.index('-c')+1]=str(p);a[a.index('-o')+1]=str(O/(label+'.o'));assert run('compile-'+label,a)==0;return O/(label+'.o')
base=(R/'src/engine/runtime/runtime.cpp').read_text()
subs=[('#include <cstdlib>','#include <cstdlib>\nextern "C" bool review_ref_fail(std::size_t);\nextern "C" void review_ref_arm(bool);\nextern "C" void review_ref_status(int,bool,bool);\nextern "C" void review_ref_position(std::size_t,int);'),('  auto result = std::realloc(p, size);','  if(size>old && review_ref_fail(size))return nullptr;\n  auto result = std::realloc(p, size);'),('  int status = lua_pcall(l, lua_gettop(l) - 2, LUA_MULTRET, 0);','  int status = lua_pcall(l, lua_gettop(l) - 2, LUA_MULTRET, 0);\n  review_ref_arm(false);\n  if(status==LUA_ERRMEM)review_ref_status(status,context(l).exhausted,context(l).conversion_failure.has_value());'),('  // Vector ownership is installed before Lua allocation can longjmp.','  // Vector ownership is installed before Lua allocation can longjmp.\n  review_ref_position(commands(l).pending_seal_location.index,1);\n  review_ref_arm(true);'),('  lua_pushvalue(l, 2);\n  commands(l).external_entries.back().function_reference = luaL_ref(l, LUA_REGISTRYINDEX);','  review_ref_position(commands(l).pending_seal_location.index,2);\n  lua_pushvalue(l, 2);\n  commands(l).external_entries.back().function_reference = luaL_ref(l, LUA_REGISTRYINDEX);'),('  commands(l).pending_seal_location                      = {};','  review_ref_arm(false);\n  commands(l).pending_seal_location                      = {};'),('    if (lua)\n      lua_close(lua);','    if (lua) {lua_close(lua);if(runtime.memory!=0)std::abort();std::println("Lua cleanup live=0");}')]
text=base
for a,b in subs:assert text.count(a)==1,(a,text.count(a));text=text.replace(a,b)
rev=text
for a,b in reversed(subs):assert rev.count(b)==1;rev=rev.replace(b,a)
assert rev==base
(O/'delta.json').write_text(json.dumps(dict(original_sha256=hashlib.sha256(base.encode()).hexdigest(),copy_sha256=hashlib.sha256(text.encode()).hexdigest(),reverse_identical=True,substitutions=[dict(before=a,after=b,count=1)for a,b in subs],policy='Actual Lua growth requests during native reference retention may return nullptr with no artificial exhausted flag. Fault window resets after pcall catches or successful ref retention. This is controlled allocator refusal, not natural host exhaustion.'),indent=2)+'\n')
main=r'''import std;
import planar.engine.runtime;
using namespace planar::engine;
bool armed=false;std::size_t attempts=0,refusals=0,bytes=0,index=0,first_index=0,first_bytes=0,second_bytes=0;int phase=0,first_phase=0,caught_memory=0;bool caught_exhausted=false,caught_conversion=false;
extern "C" void review_ref_arm(bool value){armed=value;}
extern "C" void review_ref_status(int,bool exhausted,bool conversion){++caught_memory;caught_exhausted|=exhausted;caught_conversion|=conversion;}
extern "C" void review_ref_position(std::size_t i,int p){index=i;phase=p;}
extern "C" bool review_ref_fail(std::size_t n){if(!armed)return false;++attempts;bytes=n;auto mode=std::string_view(std::getenv("REVIEW_REF_MODE"));if(mode=="control")return false;if(mode=="function"&&phase!=2)return false;if(mode=="descriptor"&&phase!=1)return false;if(mode.starts_with("one")&&refusals>=2)return false;if(!refusals){first_index=index;first_phase=phase;}++refusals;if(refusals==1)first_bytes=n;if(refusals==2)second_bytes=n;return true;}
int main(){auto p=std::make_shared<profile_closure>();p->profile={"profile","profile.json","0644",R"({"version":1,"modules":["one.lua"]})"};std::string source;
for(int i=1;i<=64;++i)source+=std::format("pcall(registration.projection,{{id='p{}',abi='projection_v1',revision='v1',input='role_context_v1',help=''}},function()end);",i);
if(std::string_view(std::getenv("REVIEW_REF_MODE"))=="one-later")source+="error(\"private later error\")";p->modules.push_back({"modules","one.lua","0644",source});auto vm=command_vm::create(p);std::println("attempts={} refusals={} bytes={} success={} first_index={} first_phase={} caught_memory={} caught_exhausted={} caught_conversion={} last_index={} last_phase={} first_refused_bytes={} second_refused_bytes={}",attempts,refusals,bytes,vm.has_value(),first_index,first_phase,caught_memory,caught_exhausted,caught_conversion,index,phase,first_bytes,second_bytes);if(!vm)std::println("{} / {} / {}",vm.error().detail.code,vm.error().detail.message,vm.error().field);if(std::string_view(std::getenv("REVIEW_REF_MODE")).starts_with("one"))return !vm&&vm.error().detail.code=="resource_limit"&&vm.error().field=="catalog.projections[2]"?0:1;return 0;}
'''
po=compile('main','/runtime/commands.t.cpp',main);ro=compile('runtime','/runtime/runtime.cpp',text)
a=shlex.split(subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=B,text=True).splitlines()[-1].split(' && ')[1]);libs=[x for x in a[a.index('src/engine/libplanar_runtime.a'):]if'catch2'not in x and x!='src/engine/libplanar_runtime.a'];binary=O/'probe';assert run('link',[a[0],'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++',po,ro,*libs,'-o',binary])==0
for mode in ['control','function','one','one-later']:assert run('run-'+mode,[binary],mode)==(1 if mode.startswith('one') else 0)
