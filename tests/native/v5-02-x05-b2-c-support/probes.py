"""Production allocation sweep and reversible runtime-copy lifetime/resource controls."""
from pathlib import Path
import json,shlex,subprocess,sys,os,hashlib,time
root=Path(__file__).resolve().parents[3];build=root/'build/rewrite-debug';out=root/'tests/native/v5-02-x05-b2-c-evidence'/sys.argv[1];out.mkdir(exist_ok=False)
entries=json.loads((build/'compile_commands.json').read_text());commands=[]
def run(label,args,mode=None):
 env=os.environ.copy()
 if mode:env['B2C_MODE']=mode
 start=time.time();p=subprocess.run(list(map(str,args)),cwd=build,capture_output=True,text=True,env=env)
 (out/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=list(map(str,args)),mode=mode,exit_code=p.returncode,seconds=time.time()-start));(out/'commands.json').write_text(json.dumps(commands,indent=2)+'\n');print(label,p.returncode,flush=True);assert p.returncode==0,(label,(p.stdout+p.stderr)[-4000:])
def compile(label,suffix,text):
 path=out/(label+'.cpp');path.write_text(text);e=next(e for e in entries if e['file'].endswith(suffix));cmd=shlex.split(e['command']);cmd[cmd.index('-c')+1]=str(path);cmd[cmd.index('-o')+1]=str(out/(label+'.o'));run(label+'-compile',cmd);return out/(label+'.o')
link=shlex.split(subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=build,text=True).splitlines()[-1].split(' && ')[1]);start=link.index('src/engine/libplanar_runtime.a');libs=link[start:]
allocation=compile('allocation','/runtime/external_registration.t.cpp',(root/'tests/native/v5-02-x05-b2-c-support/allocation.cpp').read_text());run('allocation-link',[link[0],'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++',allocation,*libs,'-o',out/'allocation']);run('allocation',[out/'allocation','--reporter','junit','--out',out/'allocation.xml'])
base=(root/'src/engine/runtime/runtime.cpp').read_text()
helper=r'''
std::size_t probe_count(lua_State* l,int slot) {
 int top=lua_gettop(l);std::size_t n=0;lua_rawgeti(l,LUA_REGISTRYINDEX,slot);lua_pushnil(l);
 while(lua_next(l,-2)){++n;lua_pop(l,1);}lua_settop(l,top);return n;
}
void probe_lifetime(lua_State* l) {
 auto tags=probe_count(l,tags_index), backs=probe_count(l,backing_index);
 if(tags!=probe_count(l,shape_tags_index)||backs!=probe_count(l,shape_backings_index))std::abort();
 auto& s=context(l);auto& c=commands(l);
 if(std::string_view(std::getenv("B2C_MODE"))=="success") {
  if(c.calls && (s.instructions<static_cast<std::size_t>(c.owner_first)||s.nodes<=static_cast<std::size_t>(c.owner_last)))std::abort();
  c.owner_first=static_cast<int>(s.instructions);c.owner_last=static_cast<int>(s.nodes);++c.calls;
  lua_gc(l,LUA_GCCOLLECT);
  std::println("call={} tags={} backings={} sealed={} instructions={} nodes={} native={} live_lua={}",c.calls,tags,backs,probe_count(l,sealed_index),s.instructions,s.nodes,s.native_bytes,s.memory);
 }
}
'''
subs=[('#include <cstdlib>', '#include <cstdlib>\nextern \"C\" void b2c_probe_arm();'),('    if (c.bridge_error)\n      return failure(*c.bridge_error);', '    if(c.bridge_error && std::string_view(std::getenv(\"B2C_MODE\"))==\"bridge\" && s.entry.empty()){s.entry=\"armed\";b2c_probe_arm();}\n    if (c.bridge_error)\n      return failure(*c.bridge_error);'),('void release_shape_temporaries(lua_State* l) {',helper+'\nvoid release_shape_temporaries(lua_State* l) {'),('        release_shape_temporaries(owner.lua);','        release_shape_temporaries(owner.lua);\n        if(owner.command.shape_baseline_ready)probe_lifetime(owner.lua);'),('                     charge(*static_cast<state*>(owner), reservation.total_bytes);','''                     if(std::string_view(std::getenv("B2C_MODE"))=="codec")static_cast<state*>(owner)->native_bytes=memory_max;
                     charge(*static_cast<state*>(owner), reservation.total_bytes);'''),('  auto result = std::realloc(p, size);', '  if(s.command && s.instruction_limit==instruction_max && std::string_view(std::getenv("B2C_MODE"))=="lua" && size>old)return nullptr;\n  auto result = std::realloc(p, size);'),('    auto result  = convert(l, 1, 1);', '    if(std::string_view(std::getenv("B2C_MODE"))=="nodes")s.nodes=node_max;\n    auto result  = convert(l, 1, 1);'),('    if (lua)\n      lua_close(lua);','    if (lua) {lua_close(lua);if(runtime.memory!=0)std::abort();}')]
text=base
for a,b in subs:assert text.count(a)==1,(a,text.count(a));text=text.replace(a,b)
reverse=text
for a,b in reversed(subs):reverse=reverse.replace(b,a)
assert reverse==base
(out/'transformations.json').write_text(json.dumps(dict(original_sha256=hashlib.sha256(base.encode()).hexdigest(),copy_sha256=hashlib.sha256(text.encode()).hexdigest(),reverse_identical=True,substitutions=[dict(before=a,after=b) for a,b in subs]),indent=2)+'\n')
runtime=compile('runtime','/runtime/runtime.cpp',text)
main=(root/'tests/native/v5-02-x05-b2-c-support/lifetime.cpp').read_text();obj=compile('lifetime','/runtime/external_registration.t.cpp',main)
libs=[a for a in libs if 'catch2' not in a and a!='src/engine/libplanar_runtime.a'];run('lifetime-link',[link[0],'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++',obj,runtime,*libs,'-o',out/'lifetime'])
for mode in ['success','codec','instructions','lua','nodes','bridge']:run('lifetime-'+mode,[out/'lifetime'],mode)
