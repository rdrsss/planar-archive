from pathlib import Path
import hashlib,json,os,shlex,subprocess,sys,time
R=Path.cwd(); B=R/'build/rewrite-debug'; O=R/'tests/native/v5-02-x05-a3-c05-evidence'/sys.argv[1];O.mkdir(exist_ok=False);rows=[]
def run(label,args,mode=None):
 env=os.environ.copy();env['REVIEW_REF_MODE']=mode or '';t=time.monotonic();p=subprocess.run(list(map(str,args)),cwd=B,env=env,text=True,capture_output=True);(O/(label+'.log')).write_text(p.stdout+p.stderr);rows.append(dict(label=label,argv=list(map(str,args)),mode=mode,exit_code=p.returncode,seconds=time.monotonic()-t));(O/'commands.json').write_text(json.dumps(rows,indent=2)+'\n');print(label,p.returncode,flush=True);return p.returncode
entries=json.loads((B/'compile_commands.json').read_text())
def compile(label,suffix,text):
 p=O/(label+'.cpp');p.write_text(text);a=shlex.split(next(e['command']for e in entries if e['file'].endswith(suffix)));a[a.index('-c')+1]=str(p);a[a.index('-o')+1]=str(O/(label+'.o'));assert run('compile-'+label,a)==0;return O/(label+'.o')
base=(R/'src/engine/runtime/runtime.cpp').read_text()
subs=[('#include <cstdlib>','#include <cstdlib>\nextern "C" bool review_ref_fail(std::size_t);\nextern "C" void review_ref_arm(bool);\nextern "C" void review_ref_status(int,bool,bool,bool);\nextern "C" void review_ref_position(std::size_t,int);'),('  auto result = std::realloc(p, size);','  if(size>old && review_ref_fail(size))return nullptr;\n  auto result = std::realloc(p, size);'),('  int status = lua_pcall(l, lua_gettop(l) - 2, LUA_MULTRET, 0);','  int status = lua_pcall(l, lua_gettop(l) - 2, LUA_MULTRET, 0);\n  review_ref_arm(false);\n  review_ref_status(status,context(l).exhausted,context(l).conversion_failure.has_value(),context(l).lua_memory_failed);'),('  // Vector ownership is installed before Lua allocation can longjmp.','  // Vector ownership is installed before Lua allocation can longjmp.\n  review_ref_position(commands(l).pending_seal_location.index,1);\n  review_ref_arm(true);'),('  lua_pushvalue(l, 2);\n  commands(l).external_entries.back().function_reference = luaL_ref(l, LUA_REGISTRYINDEX);','  review_ref_position(commands(l).pending_seal_location.index,2);\n  lua_pushvalue(l, 2);\n  commands(l).external_entries.back().function_reference = luaL_ref(l, LUA_REGISTRYINDEX);'),('  commands(l).pending_seal_location                      = {};','  review_ref_arm(false);\n  commands(l).pending_seal_location                      = {};'),('    if (lua)\n      lua_close(lua);','    if (lua) {lua_close(lua);if(runtime.memory!=0)std::abort();std::println("Lua cleanup live=0");}')]
text=base
for a,b in subs:assert text.count(a)==1,(a,text.count(a));text=text.replace(a,b)
rev=text
for a,b in reversed(subs):assert rev.count(b)==1;rev=rev.replace(b,a)
assert rev==base
(O/'delta.json').write_text(json.dumps(dict(original_sha256=hashlib.sha256(base.encode()).hexdigest(),copy_sha256=hashlib.sha256(text.encode()).hexdigest(),reverse_identical=True,substitutions=[dict(before=a,after=b,count=1)for a,b in subs],policy='Actual Lua growth requests during native reference retention may return nullptr with no artificial exhausted flag. Fault window resets after pcall catches or successful ref retention. This is controlled allocator refusal, not natural host exhaustion.'),indent=2)+'\n')
main=r'''import std;
import planar.engine.runtime;
using namespace planar::engine;
bool armed=false;std::size_t attempts=0,refusals=0,bytes=0,index=0,first_index=0,first_bytes=0,second_bytes=0;int phase=0,first_phase=0,caught_memory=0;bool caught_exhausted=false,caught_conversion=false;int outer_sticky=0;std::size_t function_requests=0;
struct request {std::size_t index,bytes;int phase;bool refused;};std::array<request,1024> requests{};std::size_t request_count=0;
std::string_view mode(){return std::getenv("REVIEW_REF_MODE");}
extern "C" void review_ref_arm(bool value){armed=value;}
extern "C" void review_ref_status(int status,bool exhausted,bool conversion,bool sticky){if(status==4){++caught_memory;caught_exhausted|=exhausted;caught_conversion|=conversion;}if(sticky)++outer_sticky;if(status)std::println("nested real status={} quota={} conversion={} sticky={} index={} phase={}",status,exhausted,conversion,sticky,index,phase);}
extern "C" void review_ref_position(std::size_t i,int p){index=i;phase=p;}
extern "C" bool review_ref_fail(std::size_t n){if(!armed)return false;++attempts;if(phase==2)++function_requests;bytes=n;bool fail=true;
if(mode()=="control"||mode().starts_with("ordinary"))fail=false;
if(mode()=="function"&&phase!=2)fail=false;
if(refusals>=(mode()=="recovery"?1u:2u))fail=false;
if(request_count>=requests.size())std::abort();requests[request_count++]={index,n,phase,fail};
if(!fail)return false;if(!refusals){first_index=index;first_phase=phase;}++refusals;if(refusals==1)first_bytes=n;if(refusals==2)second_bytes=n;return true;}
int main(){auto p=std::make_shared<profile_closure>();p->profile={"profile","profile.json","0644",R"({"version":1,"modules":["one.lua"]})"};std::string source;
if(mode()=="ordinary-caught")source+="local ok=pcall(function() error('private ordinary') end);assert(not ok);";
for(int i=1;i<=64;++i){auto registration=std::format("registration.projection({{id='p{}',abi='projection_v1',revision='v1',input='role_context_v1',help=''}},function()end)",i);
if(mode()=="host")source+=registration+";";
else if(mode()=="nested3"||mode()=="nested3-later")source+="pcall(function() pcall(function() pcall(function() "+registration+" end) end) end);";
else source+="pcall(function() "+registration+" end);";}
if(mode()=="one-later"||mode()=="nested3-later"||mode()=="ordinary-later")source+="error('private later error')";
p->modules.push_back({"modules","one.lua","0644",source});auto vm=command_vm::create(p);
std::println("attempts={} refusals={} bytes={} success={} first_index={} first_phase={} caught_memory={} caught_exhausted={} caught_conversion={} last_index={} last_phase={} first_refused_bytes={} second_refused_bytes={}",attempts,refusals,bytes,vm.has_value(),first_index,first_phase,caught_memory,caught_exhausted,caught_conversion,index,phase,first_bytes,second_bytes);
std::println("function growth requests={} outer sticky statuses={}",function_requests,outer_sticky);
for(std::size_t i=0;i<request_count;++i)if(requests[i].index==first_index)std::println("actual growth index={} phase={} bytes={} refused={}",requests[i].index,requests[i].phase,requests[i].bytes,requests[i].refused);
if(!vm)std::println("{} / {} / {} / {} / {}",vm.error().detail.code,vm.error().detail.message,vm.error().field,vm.error().detail.limit,vm.error().detail.maximum);
bool valid=false;
if(mode()=="control"||mode()=="function"||mode()=="ordinary-caught")valid=vm.has_value()&&refusals==0;
else if(mode()=="recovery")valid=vm.has_value()&&refusals==1&&caught_memory==0&&first_index==2&&first_phase==1;
else if(mode()=="ordinary-later")valid=!vm&&vm.error().detail.code=="internal"&&vm.error().field=="profile.modules[1]"&&refusals==0;
else valid=!vm&&vm.error().detail.code=="resource_limit"&&vm.error().detail.message=="Lua allocation exceeded"&&vm.error().field=="catalog.projections[2]"&&refusals==2&&first_index==2&&first_phase==1&&index==2&&phase==1&&caught_memory==(mode()=="host"?0:1)&&!caught_exhausted&&!caught_conversion;
if(mode()=="function")valid=valid&&function_requests==0;if(mode().starts_with("nested3"))valid=valid&&outer_sticky==2;return valid?0:1;}
'''
po=compile('main','/runtime/commands.t.cpp',main);ro=compile('runtime','/runtime/runtime.cpp',text)
a=shlex.split(subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=B,text=True).splitlines()[-1].split(' && ')[1]);libs=[x for x in a[a.index('src/engine/libplanar_runtime.a'):]if'catch2'not in x and x!='src/engine/libplanar_runtime.a'];binary=O/'probe';assert run('link',[a[0],'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++',po,ro,*libs,'-o',binary])==0
for mode in ['control','function','one','one-later','nested3','nested3-later','recovery','host','ordinary-caught','ordinary-later']:assert run('run-'+mode,[binary],mode)==0
