"""Controlled actual pre-root guard, partial freezer and union failures; not natural exhaustion."""
from pathlib import Path
import json,os,shlex,subprocess,sys,time,hashlib
R=Path.cwd();B=R/'build/rewrite-debug';O=R/'tests/native/v5-02-x05-a3-c03-evidence'/sys.argv[1];O.mkdir(exist_ok=False);commands=[]
def run(label,args,mode=None):
 env=os.environ.copy()
 if mode:env['C02_LOCATION']=mode
 t=time.monotonic();p=subprocess.run(list(map(str,args)),cwd=B,env=env,capture_output=True,text=True);(O/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=list(map(str,args)),mode=mode,exit_code=p.returncode,seconds=time.monotonic()-t));(O/'commands.json').write_text(json.dumps(commands,indent=2)+'\n');assert p.returncode==0,(label,p.stdout+p.stderr)
entries=json.loads((B/'compile_commands.json').read_text())
def compile(label,suffix,text):
 p=O/(label+'.cpp');p.write_text(text);e=next(e for e in entries if e['file'].endswith(suffix));args=shlex.split(e['command']);args[args.index('-c')+1]=str(p);args[args.index('-o')+1]=str(O/(label+'.o'));run('compile-'+label,args);return O/(label+'.o')
base=(R/'src/engine/runtime/runtime.cpp').read_text();subs=[]
subs.append(('int seal_profile_module(lua_State* l) {','''bool c02_mode(std::string_view value) {return value==std::getenv("C02_LOCATION");}
int seal_profile_module(lua_State* l) {'''))
# Erase only the native catalog, after ordinary initial environment sealing. The real
# guard preflight refuses before selecting any root; this is a controlled protocol fault.
subs.append(('  std::array<int, 384> roots{};','''  if(c02_mode("pre-root")) {lua_pushnil(l);lua_rawseti(l,LUA_REGISTRYINDEX,native_catalog_index);}
  std::array<int, 384> roots{};'''))
subs.append(('      lua_setmetatable(l, object);','''      lua_setmetatable(l, object);
      if(commands(l).active_module && commands(l).pending_seal_location.kind==1 && commands(l).pending_seal_location.index==1 && c02_mode_freeze())
        luaL_error(l,"c02_partial_freeze");'''))
# Freeze helper is declared before freezer; no nontrivial state crosses longjmp.
subs.append(('void freeze_graph(lua_State* l, int root, int memo, int backings, int sealed) {','''bool c02_mode_freeze() {return std::string_view(std::getenv("C02_LOCATION"))=="freeze";}
void freeze_graph(lua_State* l, int root, int memo, int backings, int sealed) {'''))
subs.append(('    lua_rawset(l, memo);\n    lua_pop(l, 1);\n  }\n  lua_pushvalue(l, memo);','''    lua_rawset(l, memo);
    if(module_roots && std::string_view(std::getenv("C02_LOCATION"))=="union")luaL_error(l,"c02_union");
    lua_pop(l, 1);
  }
  lua_pushvalue(l, memo);'''))
subs.append(('      lua_pushcfunction(l, fn);\n      auto status = lua_pcall(l, 0, 0, 0);','''    lua_pushcfunction(l, fn);
    lua_rawgeti(l,LUA_REGISTRYINDEX,sealed_index);const void* before=lua_topointer(l,-1);lua_pop(l,1);
    auto status = lua_pcall(l, 0, 0, 0);
    if(fn==seal_profile_module && !c02_mode("success")) {
      if(!s.seal_failed)throw std::runtime_error("failed owner not terminal");
      lua_rawgeti(l,LUA_REGISTRYINDEX,sealed_index);bool unchanged=before==lua_topointer(l,-1);lua_pop(l,1);
      if(!unchanged)throw std::runtime_error("failed boundary published identity set");
      int old_top=lua_gettop(l);lua_pushcfunction(l,seal_profile_module);int retry=lua_pcall(l,0,0,0);lua_settop(l,old_top);
      if(retry==LUA_OK || !s.seal_failed)throw std::runtime_error("failed owner reused");
      std::println("terminal retry refused; successful identity set unchanged");
    }'''))
subs.append(('    if (lua)\n      lua_close(lua);','''    if (lua) {lua_close(lua);if(runtime.memory!=0)std::abort();std::println("Lua cleanup live=0");}'''))
text=base
for a,b in subs:assert text.count(a)==1,(a,text.count(a));text=text.replace(a,b)
reverse=text
for a,b in reversed(subs):assert reverse.count(b)==1;reverse=reverse.replace(b,a)
assert reverse==base
(O/'transformations.json').write_text(json.dumps(dict(source_sha256=hashlib.sha256(base.encode()).hexdigest(),candidate_sha256=hashlib.sha256(text.encode()).hexdigest(),reverse_identical=True,substitutions=[dict(before=a,after=b,count=1) for a,b in subs]),indent=2)+'\n')
main=r'''import std;
import planar.engine.runtime;
using namespace planar::engine;
int main(){auto p=std::make_shared<profile_closure>();p->profile={"profile","profile.json","0644",R"({"version":1,"modules":["one.lua"]})"};p->modules.push_back({"modules","one.lua","0644",R"(registration.shape({id='z',abi='shape_v1',revision='v1',stages='optional',help=''},function()end);registration.projection({id='a',abi='projection_v1',revision='v1',input='role_context_v1',help=''},function()end))"});auto vm=command_vm::create(p);auto mode=std::string_view(std::getenv("C02_LOCATION"));if(mode=="success")return vm?0:1;if(vm)return 1;auto field=mode=="freeze"?"catalog.shapes[1]":"profile.modules[1]";std::println("{}: {} / {} / {}",mode,vm.error().detail.code,vm.error().detail.message,vm.error().field);return vm.error().detail.code=="internal"&&vm.error().detail.message=="Invalid external profile protocol"&&vm.error().field==field?0:1;}
'''
po=compile('main','/runtime/commands.t.cpp',main);ro=compile('locations-runtime','/runtime/runtime.cpp',text)
link=shlex.split(subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=B,text=True).splitlines()[-1].split(' && ')[1]);start=link.index('src/engine/libplanar_runtime.a');libs=[a for a in link[start:] if 'catch2' not in a and a!='src/engine/libplanar_runtime.a'];binary=O/'locations'
run('link',[link[0],'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++',po,ro,*libs,'-o',binary])
for mode in ['success','pre-root','freeze','union']:run('run-'+mode,[binary],mode)
print('Controlled actual pre-root, partial-freeze and union paths preserve truthful location, terminal owner, unpublished identity set, and cleanup')
