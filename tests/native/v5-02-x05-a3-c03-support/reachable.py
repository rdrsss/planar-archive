"""Reversible production-copy retained callable, lifetime and resource probes."""
from pathlib import Path
import json,os,shlex,subprocess,sys,time,hashlib
root=Path.cwd();build=root/'build/rewrite-debug';out=root/'tests/native/v5-02-x05-a3-c03-evidence'/sys.argv[1];out.mkdir(exist_ok=False)
entries=json.loads((build/'compile_commands.json').read_text());commands=[]
def run(label,args,mode=None,expect=0):
 start=time.monotonic();env=os.environ.copy()
 if mode:env['X05_A3_CASE']=mode
 r=subprocess.run(list(map(str,args)),cwd=build,capture_output=True,text=True,env=env);(out/(label+'.log')).write_text(r.stdout+r.stderr)
 commands.append(dict(label=label,argv=list(map(str,args)),case=mode,exit_code=r.returncode,seconds=time.monotonic()-start));(out/'commands.json').write_text(json.dumps(commands,indent=2)+'\n')
 assert r.returncode==expect,(label,r.returncode,(r.stdout+r.stderr)[-4000:]);return r.stdout+r.stderr
def compile_copy(label,suffix,text):
 e=next(e for e in entries if e['file'].endswith(suffix));p=out/(label+'.cpp');p.write_text(text);cmd=shlex.split(e['command']);cmd[cmd.index('-c')+1]=str(p);cmd[cmd.index('-o')+1]=str(out/(label+'.o'));run('compile-'+label,cmd);return str(out/(label+'.o'))
base=(root/'src/engine/runtime/runtime.cpp').read_text()
injection=r'''
bool a3_mode(std::string_view mode) { return mode==std::getenv("X05_A3_CASE"); }
void a3_dispatch(command_vm::implementation& owner) {
 auto l=owner.lua;auto& c=owner.command;
 auto verified=history::verify_effective_stdlib(c.provenance);
 if(!verified) throw external_protocol();
 std::string digest;for(auto b:verified->digest)digest+=std::format("{:02x}",b);
 if(digest!=c.effective_digest_bytes)throw external_protocol();
 std::vector<std::string_view> shipped,external;
 for(auto& f:embedded::originals())shipped.push_back(f.path);
 for(auto& f:c.profile->modules)external.push_back(f.path);
 if(!history::decode_catalog(c.effective_catalog_bytes,shipped,external))throw external_protocol();
 lua_gc(l,LUA_GCCOLLECT);
 for(auto& e:c.external_entries) {
  lua_rawgeti(l,LUA_REGISTRYINDEX,e.function_reference);
  lua_pushstring(l,a3_mode("saved")?"saved":"dispatch");
  auto status=lua_pcall(l,1,1,0);
  if(a3_mode("saved")) {
   if(!status||!c.bridge_error||c.bridge_error->message!="Invalid external profile protocol")throw std::runtime_error("saved callback was not inactive");
   std::println("saved callback refused with sticky error outside registration");throw external_protocol();
  }
  if(status||lua_tointeger(l,-1)!=42)throw std::runtime_error("retained captured callable mismatch");
  lua_pop(l,1);
 }
 if(c.external_entries.size()!=3)throw std::runtime_error("three callable kinds missing");
 std::println("same-VM three actual retained callables execute final D and shared frozen aliases after GC; exact catalog/provenance verify; instructions={} native={}",owner.runtime.instructions,owner.runtime.native_bytes);
}
'''
subs=[('    if (c.profile)\n      register_profile(*owner);','''    if (c.profile) {
      if(std::string_view(std::getenv("X05_A3_CASE"))=="unexpected")throw std::runtime_error("private unexpected");
      if(std::string_view(std::getenv("X05_A3_CASE"))=="badalloc")throw std::bad_alloc();
      register_profile(*owner);
    }'''),('    auto  kind = static_cast<std::size_t>(lua_tointeger(l, lua_upvalueindex(1)));','''    if(std::string_view(std::getenv("X05_A3_CASE"))=="callback-unexpected")throw std::runtime_error("private callback");
    if(std::string_view(std::getenv("X05_A3_CASE"))=="callback-badalloc")throw std::bad_alloc();
    auto  kind = static_cast<std::size_t>(lua_tointeger(l, lua_upvalueindex(1)));'''),('void register_profile(command_vm::implementation& owner) {',injection+'\nvoid register_profile(command_vm::implementation& owner) {'),('  c.seal       = c.effective_catalog_bytes + c.effective_digest_bytes;','  a3_dispatch(owner);\n  c.seal       = c.effective_catalog_bytes + c.effective_digest_bytes;'),('  // Vector ownership is installed before Lua allocation can longjmp.','''  if (std::string_view(std::getenv("X05_A3_CASE"))=="unknown") {
    constexpr const char* factory="return function(f) return function() return f() end end";
    if(luaL_loadbufferx(l,factory,std::char_traits<char>::length(factory),"a3-private-native-factory","t"))return lua_error(l);
    lua_call(l,0,1);lua_pushinteger(l,99);lua_pushcclosure(l,digest,1);lua_call(l,1,1);lua_replace(l,2);
  }
  // Vector ownership is installed before Lua allocation can longjmp.'''),('int seal_profile_module(lua_State* l) {','''int seal_profile_module(lua_State* l) {
  if(std::string_view(std::getenv("X05_A3_CASE"))=="native")context(l).native_bytes=memory_max;'''),('    if (lua)\n      lua_close(lua);','''    if (lua) {
      lua_close(lua);
      if(runtime.memory!=0)std::abort();
    }''')]
text=base
for a,b in subs:assert text.count(a)==1,(a,text.count(a));text=text.replace(a,b)
reverse=text
for a,b in reversed(subs):assert reverse.count(b)==1;reverse=reverse.replace(b,a)
assert reverse==base
(out/'original-runtime.cpp').write_text(base)
(out/'transformations.json').write_text(json.dumps(dict(original_sha256=hashlib.sha256(base.encode()).hexdigest(),candidate_sha256=hashlib.sha256(text.encode()).hexdigest(),reverse_identical=True,substitutions=[dict(before=a,after=b,count=1) for a,b in subs]),indent=2)+'\n')
main=r'''import std;
import planar.engine.runtime;
using namespace planar::engine;
int main(){
 auto p=std::make_shared<profile_closure>();
 p->profile={"profile","profile.json","0644",R"json({"version":1,"modules":["one.lua","two.lua"]})json"};
 p->modules.push_back({"modules","one.lua","0644",R"lua(
 assert(_G==_G._G and not engine.hash_value and not engine.json_encode);
 local saved=registration.add;local t={answer=41};local alias=t;
 local D={name='custom',family='extension_read_v1',handler_id='custom.v1',options=engine.array{},help='old',response_kind='extension_v1',capabilities=engine.array{}}
 local F=function(mode)
   if mode=='saved' then return saved(D,function()end) end
   assert(t==alias and t.answer==41 and D.help=='final');
   assert(not pcall(function() t.answer=9 end));assert(not pcall(function() D.help='changed' end));
   assert(not pcall(function() _G.escape=1 end));assert(not engine.hash_value);
   local n=0;local function inc()n=n+1 end;inc();return t.answer+n
 end
 assert(pcall(saved,D,F));D.help='final'
 registration.shape({id='custom',abi='shape_v1',revision='v1',stages='optional',help=''},F)
 )lua"});
 p->modules.push_back({"modules","two.lua","0755",R"lua(
 assert(_G==_G._G and not _G.escape and not engine.hash_value);local t={answer=42};
 registration.projection({id='custom',abi='projection_v1',revision='v1',input='role_context_v1',help=''},function()return t.answer end)
 )lua"});
 auto vm=command_vm::create(p);auto mode=std::string_view(std::getenv("X05_A3_CASE"));
 if(mode=="success") {if(!vm){std::println("{} {} {}",vm.error().detail.code,vm.error().detail.message,vm.error().field);return 1;}p->modules.clear();p->profile.bytes="changed";return 0;}
 if(vm)return 1;
 std::println("refusal {} {} {}",vm.error().detail.code,vm.error().detail.message,vm.error().field);
 return vm.error().detail.code==((mode=="native"||mode=="badalloc"||mode=="callback-badalloc")?"resource_limit":"internal")?0:1;
}
'''
po=compile_copy('main','/runtime/commands.t.cpp',main);ro=compile_copy('probe-runtime','/runtime/runtime.cpp',text)
link=shlex.split(subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=build,text=True).splitlines()[-1].split(' && ')[1]);compiler=link[0];start=link.index('src/engine/libplanar_runtime.a');libs=[a for a in link[start:] if 'catch2' not in a and a!='src/engine/libplanar_runtime.a']
binary=out/'probe';run('link',[compiler,'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++',po,ro,*libs,'-o',binary])
for mode in ['success','saved','unknown','native','unexpected','badalloc','callback-unexpected','callback-badalloc']:run('run-'+mode,[binary],mode)
# Controlled red: revert only the catch-all policy; the non-allocation detector fails.
a='profile ? external_protocol() : diagnostic{"internal", "Command registration failed"}'
b='profile ? external_resource() : diagnostic{"internal", "Command registration failed"}'
assert text.count(a)==1
red=text.replace(a,b);redobj=compile_copy('red-runtime','/runtime/runtime.cpp',red);redbin=out/'red'
run('link-red',[compiler,'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++',po,redobj,*libs,'-o',redbin])
run('run-red',[redbin],'unexpected',1)
(out/'red-transformation.json').write_text(json.dumps(dict(before=a,after=b,reverse_identical=red.replace(b,a)==text),indent=2)+'\n')
print('Eight real production retained-dispatch/lifetime/resource probes plus catch-all red control passed')
