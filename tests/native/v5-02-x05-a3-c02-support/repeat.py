"""Reversible private production seam probes; no public runtime interface changes."""
from pathlib import Path
import json,os,shlex,subprocess,sys,time,hashlib
root=Path.cwd();build=root/'build/rewrite-debug';out=root/'tests/native/v5-02-x05-a3-c02-evidence'/sys.argv[1];out.mkdir(exist_ok=False)
entries=json.loads((build/'compile_commands.json').read_text());commands=[]
def sha(s):return hashlib.sha256(s.encode()).hexdigest()
def run(label,args,mode=None,expect=0):
 start=time.monotonic();env=os.environ.copy()
 if mode:env['X05_A2_CASE']=mode
 r=subprocess.run(list(map(str,args)),cwd=build,capture_output=True,text=True,env=env);log=r.stdout+r.stderr;(out/(label+'.log')).write_text(log)
 commands.append(dict(label=label,argv=list(map(str,args)),case=mode,exit_code=r.returncode,seconds=time.monotonic()-start));(out/'commands.json').write_text(json.dumps(commands,indent=2)+'\n')
 assert r.returncode==expect,(label,r.returncode,log[-4000:]);return log
def compile_copy(label,suffix,text):
 e=next(e for e in entries if e['file'].endswith(suffix));p=out/(label+'.cpp');p.write_text(text);cmd=shlex.split(e['command']);cmd[cmd.index('-c')+1]=str(p);cmd[cmd.index('-o')+1]=str(out/(label+'.o'));run('compile-'+label,cmd);return str(out/(label+'.o'))
base=(root/'src/engine/runtime/runtime.cpp').read_text();subs=[]
injection=r'''
bool a2_mode(const char* value) { return std::string_view(std::getenv("X05_A2_CASE"))==value; }
int a2_load(lua_State* l) {
 const char* source=lua_tostring(l,1);
 if(luaL_loadbufferx(l,source,std::char_traits<char>::length(source),"a2-new-root","t")) return lua_error(l);
 lua_call(l,0,1);return 1;
}
void* a2_refuse_growth(void* ud,void* p,std::size_t old,std::size_t size) {
 if(size>old) {std::println("denied Lua growth {} -> {}",old,size);return nullptr;}
 return allocate(ud,p,old,size);
}
int a2_gc(lua_State* l) { lua_gc(l,LUA_GCCOLLECT);return 0; }
int a2_weak(lua_State* l) {
 lua_newtable(l);lua_newtable(l);lua_pushstring(l,"v");lua_setfield(l,-2,"__mode");lua_setmetatable(l,-2);
 lua_pushvalue(l,1);lua_rawseti(l,-2,1);return 1;
}
bool a2_load_root(lua_State* l,const char* text) {
 lua_pushcfunction(l,a2_load);lua_pushstring(l,text);
 return lua_pcall(l,1,1,0)==LUA_OK;
}
bool a2_run(lua_State* l) {
 const char* text=R"lua(local alias=string;local iterator,state=pairs(alias);assert(iterator==next and state==alias);
 local t={a={v=3}};t.b=t.a;local shared=engine;
 return function() local n=0;local function inc() n=n+1 end;inc();
 assert(t.a==t.b and t.a.v==3 and shared==engine);
 assert(rawget(alias,'len')==alias.len and alias.len('abc')==3);
 assert(not pcall(function() alias.len=false end));
 assert(not pcall(function() t.a.v=4 end));
 local k,v=iterator(alias,nil);assert(k~=nil and v~=nil);return alias.len('abc')+n end)lua";
 if(a2_mode("writer")) text="local n=0;return function() n=n+1 end";
 if(a2_mode("native")) text="return function(f) return function() return f() end end";
 if(a2_mode("direct-native")) text="return next";
 if(!a2_load_root(l,text)) {std::println("load failed {}",lua_tostring(l,-1));return false;}
 if(a2_mode("native")) {lua_pushinteger(l,99);lua_pushcclosure(l,digest,1);if(lua_pcall(l,1,1,0))return false;}
 int reference=luaL_ref(l,LUA_REGISTRYINDEX);
 std::size_t before=context(l).native_bytes;
 if(a2_mode("exhausted")) context(l).native_bytes=memory_max;
 if(a2_mode("reservation")||a2_mode("luaalloc")) {
  lua_pushcfunction(l,a2_gc);if(lua_pcall(l,0,0,0))return false;
  lua_setallocf(l,a2_refuse_growth,&context(l));
 }
 lua_pushcfunction(l,seal_new_roots);lua_rawgeti(l,LUA_REGISTRYINDEX,reference);
 int status=lua_pcall(l,1,0,0);
 if(a2_mode("reservation")||a2_mode("luaalloc"))lua_setallocf(l,allocate,&context(l));
 std::println("boundary status={} poisoned={} charged-before={} charged-after={} inspection-edges={}",status,context(l).seal_failed,before,context(l).native_bytes,context(l).seal_inspections.edges);
 bool success=a2_mode("success")||a2_mode("red");
 if(!success) {
  if(!context(l).seal_failed)throw std::runtime_error("failure owner was not terminal");
  lua_rawgeti(l,LUA_REGISTRYINDEX,sealed_index);bool unchanged=lua_topointer(l,-1)==sealed_before;lua_pop(l,1);
  if(!unchanged)throw std::runtime_error("failed boundary published identities");
  if(status)lua_pop(l,1);
  lua_pushcfunction(l,seal_new_roots);lua_rawgeti(l,LUA_REGISTRYINDEX,reference);
  int retry=lua_pcall(l,1,0,0);std::println("same-owner retry status={} poisoned={}",retry,context(l).seal_failed);
  if(retry==LUA_OK)throw std::runtime_error("failed owner reused");
  lua_pop(l,1);
  if(a2_mode("writer")||a2_mode("native")||a2_mode("direct-native")) {
   if(!commands(l).bridge_error||commands(l).bridge_error->code!="internal")throw std::runtime_error("expected internal");
  } else if(a2_mode("exhausted")||a2_mode("badalloc")) {
   if(!commands(l).bridge_error||commands(l).bridge_error->code!="resource_limit")throw std::runtime_error("expected resource_limit");
  } else if(a2_mode("reservation")) {
   if(!context(l).stack_exhausted)throw std::runtime_error("expected stack failure");
  } else if(a2_mode("luaalloc")) {
   if(status!=LUA_ERRMEM)throw std::runtime_error("expected Lua memory failure");
  } else if(a2_mode("partial")||a2_mode("publication")) {
   if(status==LUA_OK)throw std::runtime_error("expected partial freeze failure");
  }
  std::println("failure classification and terminal reuse checks passed");return true;
 }
 if(status||context(l).seal_failed||context(l).native_bytes<=before)return false;
 lua_pushcfunction(l,a2_weak);lua_rawgeti(l,LUA_REGISTRYINDEX,reference);
 if(lua_pcall(l,1,1,0))return false;
 int weak=luaL_ref(l,LUA_REGISTRYINDEX);luaL_unref(l,LUA_REGISTRYINDEX,reference);
 lua_pushcfunction(l,a2_gc);if(lua_pcall(l,0,0,0))return false;
 lua_rawgeti(l,LUA_REGISTRYINDEX,weak);lua_rawgeti(l,-1,1);
 if(!lua_isfunction(l,-1))throw std::runtime_error("successful identity was not strongly retained");
 reference=luaL_ref(l,LUA_REGISTRYINDEX);lua_pop(l,1);
 // Third boundary seals a fresh closure retaining the second closure and prior aliases.
 lua_pushcfunction(l,a2_load);lua_pushstring(l,"return function(previous) return function() return previous() end end");
 if(lua_pcall(l,1,1,0))return false;
 lua_rawgeti(l,LUA_REGISTRYINDEX,reference);if(lua_pcall(l,1,1,0))return false;
 int third=luaL_ref(l,LUA_REGISTRYINDEX);
 before=context(l).native_bytes;
 lua_pushcfunction(l,seal_new_roots);lua_rawgeti(l,LUA_REGISTRYINDEX,third);
 if(lua_pcall(l,1,0,0)||context(l).seal_failed||context(l).native_bytes<=before)return false;
 std::println("third boundary charged-before={} charged-after={}",before,context(l).native_bytes);
 lua_gc(l,LUA_GCCOLLECT);
 for(int i=0;i<3;++i) {
  lua_rawgeti(l,LUA_REGISTRYINDEX,third);
  int call=lua_pcall(l,0,1,0);
  if(call||lua_tointeger(l,-1)!=4) {std::println("invocation failed status={} result={}",call,lua_tostring(l,-1));return false;}
  lua_pop(l,1);
 }
 std::println("three boundaries, exact iterator GC, proxy contents aliases writes and scratch passed");return true;
}
'''
# The exact helper lives in the same private namespace. Only the fixture calls it after normal creation.
subs.append(('// Bound C-library multi-result growth before it can reserve an arbitrary stack.',injection+'\n// Bound C-library multi-result growth before it can reserve an arbitrary stack.'))
subs.append(('    return command_vm(std::move(owner));','    if (!a2_run(l)) throw diagnostic{"internal","A2 probe failed"};\n    return command_vm(std::move(owner));'))
# Faults occur only after the normal embedded seal has completed, during repeated roots.
subs.append(('    return std::pmr::new_delete_resource()->allocate(bytes, alignment);','    if (context_probe_badalloc(_owner)) throw std::bad_alloc();\n    return std::pmr::new_delete_resource()->allocate(bytes, alignment);'))
subs.append(('int seal_roots(lua_State* l, std::span<const int> roots, bool cleanup_registration, bool module_roots = false) {', 'bool a2_mode_publication() { return std::string_view(std::getenv("X05_A2_CASE"))=="publication"; }\nint seal_roots(lua_State* l, std::span<const int> roots, bool cleanup_registration, bool module_roots = false) {'))
subs.append(('  closure_guard::snapshot_curated_natives(l, native_catalog_index);', '''  closure_guard::snapshot_curated_natives(l, native_catalog_index);
  constexpr const char* a2_preflight="local t={answer=7};local it,s,k=pairs(t);assert(it==next and s==t and k==nil);local key,v=it(s,k);assert(key=='answer' and v==7 and rawget(t,key)==7)";
  if(luaL_loadbufferx(l,a2_preflight,std::char_traits<char>::length(a2_preflight),"a2-unsealed","t"))return lua_error(l);
  lua_call(l,0,0);'''))
subs.append(('class closure_memory final', '''bool context_probe_badalloc(const state& s) {
 return s.seal_inspections.edges>0 && std::string_view(std::getenv("X05_A2_CASE"))=="badalloc";
}
class closure_memory final'''))
subs.append(('      lua_setmetatable(l, object);','''      lua_setmetatable(l, object);
      if (!cleanup_probe_initial(context(l)) && std::string_view(std::getenv("X05_A2_CASE"))=="partial")
        luaL_error(l,"a2_partial_freeze");'''))
subs.append(('void freeze_graph(lua_State* l, int root, int memo, int backings, int sealed) {','''bool cleanup_probe_initial(const state& s) { return s.seal_inspections.lua_closures==5; }
void freeze_graph(lua_State* l, int root, int memo, int backings, int sealed) {'''))
# Use explicit stage, independent of shipped graph counts.
subs[-1]=(subs[-1][0],subs[-1][1].replace('return s.seal_inspections.lua_closures==5;', 'return !s.pending.empty();'))
# pending has unrelated semantics, use fixture-only field instead.
subs[-1]=(subs[-1][0],subs[-1][1].replace('return !s.pending.empty();','return !s.a2_repeat;'))
subs.append(('    if (!lua_checkstack(l, 2048)) {','    if (!lua_checkstack(l, a2_mode("reservation") ? 20 : 2048)) {'))
subs.append(('    lua_rawset(l, memo);\n    lua_pop(l, 1);\n  }\n  lua_pushvalue(l, memo);','    lua_rawset(l, memo);\n    if (context(l).a2_repeat && a2_mode_publication()) luaL_error(l, \"a2_publication\");\n    lua_pop(l, 1);\n  }\n  lua_pushvalue(l, memo);'))
subs.append(('  closure_guard::report     seal_inspections;', '  bool a2_repeat=false;\n  closure_guard::report     seal_inspections;'))
injection_old=subs[0][1];injection_new=injection_old.replace(' std::size_t before=context(l).native_bytes;', ' lua_rawgeti(l,LUA_REGISTRYINDEX,sealed_index);const void* sealed_before=lua_topointer(l,-1);lua_pop(l,1);\n context(l).a2_repeat=true;\n std::size_t before=context(l).native_bytes;',1);subs[0]=(subs[0][0],injection_new)
text=base
for before,after in subs:
 assert text.count(before)==1,(before,text.count(before));text=text.replace(before,after)
rev=text
for before,after in reversed(subs):assert rev.count(after)==1;rev=rev.replace(after,before)
assert rev==base
# Probe main verifies create succeeds only when every internal assertion passed.
main='''import std;\nimport planar.engine.runtime;\nint main(){auto vm=planar::engine::command_vm::create();if(!vm)std::println("create failure {} {}",vm.error().code,vm.error().message);return vm?0:1;}\n'''
(out/'source-original.cpp').write_text(base)
(out/'transformations.json').write_text(json.dumps(dict(source_sha256=sha(base),candidate_sha256=sha(text),reverse_identical=True,substitutions=[dict(before=a,after=b,count=1) for a,b in subs]),indent=2)+'\n')
po=compile_copy('main','/runtime/commands.t.cpp',main)
ro=compile_copy('repeat-runtime','/runtime/runtime.cpp',text)
link=shlex.split(subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=build,text=True).splitlines()[-1].split(' && ')[1]);compiler=link[0];start=link.index('src/engine/libplanar_runtime.a');libs=[a for a in link[start:] if 'catch2' not in a and a!='src/engine/libplanar_runtime.a']
def link_run(label,obj):
 binary=out/label;run('link-'+label,[compiler,'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++',po,obj,*libs,'-o',binary]);return binary
binary=link_run('repeat',ro)
for mode in ['success','writer','native','direct-native','exhausted','badalloc','partial','publication','luaalloc','reservation']:run('run-'+mode,[binary],mode)
# Red control removes only prior-identity freezer skip; a second seal destroys proxy contents.
red_before='seen = seen || !lua_isnil(l, -1);';red_after='(void)sealed; // red control: prior proxies are erroneously frozen again'
assert text.count(red_before)==1;red=text.replace(red_before,red_after);redobj=compile_copy('red-runtime','/runtime/runtime.cpp',red);redbin=link_run('red',redobj);run('run-red',[redbin],'red',1)
(out/'transformations.json').write_text(json.dumps(dict(source_sha256=sha(base),candidate_sha256=sha(text),reverse_identical=True,substitutions=[dict(before=a,after=b,count=1) for a,b in subs],red=dict(before=red_before,after=red_after,reverse_identical=red.replace(red_after,red_before)==text)),indent=2)+'\n')
print('Ten real production repeat-boundary scenarios plus red control passed')
