"""Private actual-runtime observers; isolate every compiled object and module dependency."""
from pathlib import Path
import hashlib,json,shlex,shutil,subprocess,sys,time,os
sys.dont_write_bytecode=True
R=Path(__file__).resolve().parents[3];B=R/'build/rewrite-debug';S=Path(__file__).parent;E=R/'tests/native/v5-02-x05-c2-b2-evidence';P=E/sys.argv[1];P.mkdir();commands=[]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def write(p,v):p.write_text(json.dumps(v,sort_keys=True,indent=2)+'\n')
def run(label,args,mode=''):
 start=time.time();p=subprocess.run(list(map(str,args)),cwd=B,capture_output=True,text=True,env=dict(os.environ,C2B2_MODE=mode));(P/(label+'.log')).write_text(p.stdout+p.stderr)
 commands.append(dict(label=label,argv=list(map(str,args)),cwd=str(B),mode=mode,exit=p.returncode,seconds=time.time()-start));write(P/'commands.json',commands);print(label,p.returncode,flush=True)
 assert p.returncode==0,(label,(p.stdout+p.stderr)[-4000:]);return p.stdout
def compiled():return {str(p.relative_to(R)):sha(p) for p in B.rglob('*') if p.is_file() and p.suffix in {'.o','.pcm','.a'}}
original=compiled();write(P/'shared-compiled-before.json',original);copied={}
for relative,digest in original.items():
 src=R/relative;dst=P/'private'/src.relative_to(B);dst.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(src,dst);assert sha(dst)==digest;copied[str(dst.relative_to(E))]=digest
write(P/'copied-compiled.json',copied)
entries=json.loads((B/'compile_commands.json').read_text())
def expand(args):
 result=[]
 for a in args:
  if a.startswith('@'):result.extend(expand(shlex.split((B/a[1:]).read_text())))
  else:result.append(a)
 return result
def compile(label,suffix,source):
 base=expand(shlex.split(next(e['command'] for e in entries if e['file'].endswith(suffix))))
 for i,a in enumerate(base):
  if a.startswith('-fmodule-file='):
   key,path=a[len('-fmodule-file='):].split('=',1);base[i]='-fmodule-file='+key+'='+str(P/'private'/(B/path).resolve().relative_to(B))
  if a.startswith('-fmodule-output='):base[i]='-fmodule-output='+str(P/(label+'.pcm'))
 base+=['-fno-implicit-modules','-fno-implicit-module-maps'];base[base.index('-c')+1]=str(source);base[base.index('-o')+1]=str(P/(label+'.o'))
 for flag in ['-MF','-MT']:
  if flag in base:base[base.index(flag)+1]=str(P/(label+('.d' if flag=='-MF' else '.o')))
 run(label+'-compile',base);return P/(label+'.o')
base=(R/'src/engine/runtime/runtime.cpp').read_text();text=base;subs=[]
def replace(a,b):
 global text
 assert text.count(a)==1,(a,text.count(a));text=text.replace(a,b);subs.append((a,b))
replace('void* allocate(void* ud, void* p, std::size_t old, std::size_t size) {', 'bool c2_fail_lua=false;\nvoid* allocate(void* ud, void* p, std::size_t old, std::size_t size) {\n  if(c2_fail_lua && size) {static_cast<state*>(ud)->exhausted=true;return nullptr;}')
replace('#include <cstdlib>','#include <cstdlib>\nextern "C" void c2_event(const char*, std::size_t, std::size_t);')
replace('  lua_pushvalue(l, input);\n  lua_call(l, 1, LUA_MULTRET);','  lua_pushvalue(l, input);\n  c2_event("call", context(l).nodes, context(l).native_bytes);\n  lua_call(l, 1, LUA_MULTRET);')
replace('      projection_scan scanned;','      c2_event("scan",s.nodes,s.native_bytes);\n      projection_scan scanned;')
replace('      auto counted = codec_projection_count(','''      lua_gc(l,LUA_GCCOLLECT); // Real rooted borrowed-view GC boundary.
      if(auto mode=std::getenv("C2B2_MODE")) {
        if(std::string_view(mode)=="remaining")s.native_bytes=memory_max;
      }
      auto counted = codec_projection_count(''')
# Unique projection callback charge, preserving the rest of the bridge.
needle='charge(*static_cast<state*>(owner), reservation.total_bytes);';a=text.index('command_vm::invoke_projection');at=text.index(needle,a);text=text[:at]+text[at:].replace(needle,needle+'\n               c2_event("reserved",reservation.total_bytes,static_cast<state*>(owner)->native_bytes);',1);subs.append(('PROJECTION_ONLY:'+needle,needle+' EVENT'))
needle='\n      auto result = convert(l, 1, 1);';assert text.count(needle)==1
replace(needle,'\n      c2_event("copy",s.nodes,s.native_bytes);'+needle)
replace('      charge(s, sizeof(projection_scan));','''      if(auto mode=std::getenv("C2B2_MODE")) {
        if(std::string_view(mode)=="workspace")s.native_bytes=memory_max;
        if(std::string_view(mode)=="nodes")s.nodes=node_max;
        if(std::string_view(mode)=="instructions") { c.invocation_started=true;s.instruction_limit=s.instructions; }
        if(std::string_view(mode)=="lua") {c.invocation_started=true;s.memory_limit=s.memory;}
      }
      charge(s, sizeof(projection_scan));''')
# Exact original registry identities; the two inherited lazy-baseline bookkeeping keys are reported separately.
snapshot = r'''void c2_snapshot(lua_State* l) {
  std::size_t checksum=0,count=0;
  c2_event("snapshot_begin",0,0);
  for(int table : {LUA_REGISTRYINDEX,tags_index,backing_index,sealed_index}) {
    int index=LUA_REGISTRYINDEX;
    if(table!=LUA_REGISTRYINDEX) {lua_rawgeti(l,LUA_REGISTRYINDEX,table);index=lua_gettop(l);}
    lua_pushnil(l);
    while(lua_next(l,index)) {
      auto key=lua_isinteger(l,-2)?static_cast<std::size_t>(lua_tointeger(l,-2)):reinterpret_cast<std::size_t>(lua_topointer(l,-2));
      auto value=lua_isinteger(l,-1)?static_cast<std::size_t>(lua_tointeger(l,-1)):reinterpret_cast<std::size_t>(lua_topointer(l,-1));
      if(table==LUA_REGISTRYINDEX && (lua_touserdata(l,-2)==&shape_tags_key || lua_touserdata(l,-2)==&shape_backings_key))
        c2_event("bookkeeping",key,value);
      else {
        c2_event(table==LUA_REGISTRYINDEX?"registry":table==tags_index?"tags":table==backing_index?"backings":"sealed",key,value);
        checksum^=(key*1099511628211ULL)^(value*1469598103934665603ULL);++count;
      }
      lua_pop(l,1);
    }
    if(table!=LUA_REGISTRYINDEX)lua_pop(l,1);
  }
  c2_event("snapshot",checksum,count);
}
'''
replace('int execute_shape(lua_State* l) {',snapshot+'int execute_shape(lua_State* l) {\n  c2_snapshot(l);c2_event("before",0,0);')
replace('  if (!commands(l).shape_baseline_ready)\n    return;', '  if (!commands(l).shape_baseline_ready) { c2_snapshot(l);return; }')
replace('  lua_settop(l, 0);\n}\nvoid begin_invocation', '  lua_settop(l, 0);\n  c2_snapshot(l);\n}\nvoid begin_invocation')
replace('    for (int slot : {tags_index, backing_index}) {\n      lua_newtable(l);', '    for (int slot : {tags_index, backing_index}) {\n      if(auto mode=std::getenv("C2B2_MODE"))if(std::string_view(mode)=="partial" && slot==backing_index)c2_fail_lua=true;\n      lua_newtable(l);')
replace('      projection_scan scanned;', '      projection_scan scanned;') if False else None
replace('      status = lua_pcall(l, 2, 0, 0);', '      if(auto mode=std::getenv("C2B2_MODE"))if(std::string_view(mode)=="metatable") {lua_newtable(l);lua_setmetatable(l,1); }\n      status = lua_pcall(l, 2, 0, 0);')
(P/'runtime.cpp').write_text(text);write(P/'transformations.json',dict(original_sha256=sha(R/'src/engine/runtime/runtime.cpp'),private_sha256=sha(P/'runtime.cpp'),substitutions=subs,controlled_adapters=True))
runtime=compile('runtime','/runtime/runtime.cpp',P/'runtime.cpp');observer=compile('observer','/runtime/external_registration.t.cpp',S/'observer.cpp')
link=shlex.split(subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=B,text=True).splitlines()[-1].split(' && ')[1]);libs=link[link.index('src/engine/libplanar_runtime.a'):];libs=[str(P/'private'/a) if a.endswith('.a') else a for a in libs]
common=[link[0],'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++']
run('link',common+[observer,runtime,*libs,'-o',P/'observer'])
for label,tag,mode in [('observer','[c2-b2-observer]',''),('alloc','[c2-b2-alloc]',''),('metatable','[c2-b2-metatable]','metatable')]+[(m,'[c2-b2-quota]',m)for m in ['remaining','workspace','nodes','instructions','lua','partial']]:
 run(label,[P/'observer',tag,'--reporter','junit','--out',P/(label+'.xml')],mode)
assert compiled()==original
private={str(p.relative_to(E)):sha(p)for p in (P/'private').rglob('*')if p.is_file()};assert private==copied
write(P/'compiled-stability.json',dict(shared_entries=len(original),private_entries=len(copied),membership_and_bytes_unchanged=True,all_response_files_expanded=True,implicit_modules_disabled=True))
