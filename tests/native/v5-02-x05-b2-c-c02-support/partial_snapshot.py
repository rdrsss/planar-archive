"""Reversible observations only; optional controlled codec refusal is explicit."""
from pathlib import Path
import subprocess,json,shlex,hashlib,os,sys,time
root=Path(__file__).resolve().parents[3];build=root/'build/rewrite-debug';out=root/'tests/native/v5-02-x05-b2-c-c02-evidence'/sys.argv[1];out.mkdir();commands=[]
def run(label,args,mode=None):
 env=os.environ.copy()
 if mode:env['C01_MODE']=mode
 t=time.time();p=subprocess.run(list(map(str,args)),cwd=build,capture_output=True,text=True,env=env);(out/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=list(map(str,args)),mode=mode,exit=p.returncode,seconds=time.time()-t));(out/'commands.json').write_text(json.dumps(commands,indent=2)+'\n');print(label,p.returncode,flush=True);assert p.returncode==0,label
entries=json.loads((build/'compile_commands.json').read_text())
def compile(label,suffix,path):
 cmd=shlex.split(next(e['command'] for e in entries if e['file'].endswith(suffix)));cmd[cmd.index('-c')+1]=str(path);cmd[cmd.index('-o')+1]=str(out/(label+'.o'));run(label+'-compile',cmd);return out/(label+'.o')
base=(root/'src/engine/runtime/runtime.cpp').read_text()
helper='''
void c01_observe(lua_State* l) {
 int top=lua_gettop(l);
 for(auto& e:commands(l).entries) {lua_rawgeti(l,LUA_REGISTRYINDEX,e.reference);c01_reference(e.reference,lua_type(l,-1),lua_topointer(l,-1));lua_pop(l,1);}
 for(auto& e:commands(l).external_entries) {for(int ref:{e.descriptor_reference,e.function_reference}){lua_rawgeti(l,LUA_REGISTRYINDEX,ref);c01_reference(ref,lua_type(l,-1),lua_topointer(l,-1));lua_pop(l,1);}}
 lua_settop(l,top);c01_boundary();
 if(std::string_view(std::getenv("C01_MODE"))=="snapshot") {std::println("snapshot_state invocation={} ready={} exhausted={} live_lua={} limit={}",commands(l).invocation_started,commands(l).shape_baseline_ready,context(l).exhausted,context(l).memory,context(l).memory_limit);if(commands(l).invocation_started && (!context(l).exhausted || commands(l).shape_baseline_ready))std::abort();}
}
'''
subs=[('#include <cstdlib>','#include <cstdlib>\nextern "C" void c01_reference(int,int,const void*);\nextern "C" void c01_boundary();'),('int execute_shape(lua_State* l) {',helper+'\nint execute_shape(lua_State* l) {'),('    return command_vm(std::move(owner));','    c01_observe(owner->lua);\n    return command_vm(std::move(owner));'),('        release_shape_temporaries(owner.lua);','        release_shape_temporaries(owner.lua);\n        lua_gc(owner.lua,LUA_GCCOLLECT);\n        c01_observe(owner.lua);'),('                     charge(*static_cast<state*>(owner), reservation.total_bytes);','                     if(std::string_view(std::getenv("C01_MODE"))=="resource")static_cast<state*>(owner)->native_bytes=memory_max;\n                     charge(*static_cast<state*>(owner), reservation.total_bytes);'),('    if (lua)\n      lua_close(lua);','    if (lua) {lua_close(lua);if(runtime.memory!=0)std::abort();}')]
subs.append(('      lua_rawsetp(l, LUA_REGISTRYINDEX, slot == tags_index ? &shape_tags_key : &shape_backings_key);', '      lua_rawsetp(l, LUA_REGISTRYINDEX, slot == tags_index ? &shape_tags_key : &shape_backings_key);\n      if(slot==tags_index && std::string_view(std::getenv("C01_MODE"))=="snapshot")context(l).memory_limit=context(l).memory;'))
subs.append(('  auto result = std::realloc(p, size);', '  if(s.command && s.instruction_limit==instruction_max && s.memory_limit!=memory_max && std::string_view(std::getenv("C01_MODE"))=="snapshot" && size>old){s.exhausted=true;return nullptr;}\n  auto result = std::realloc(p, size);'))
variant=base
for a,b in subs:assert variant.count(a)==1,(a,variant.count(a));variant=variant.replace(a,b)
reverse=variant
for a,b in reversed(subs):reverse=reverse.replace(b,a)
assert reverse==base
(out/'runtime.cpp').write_text(variant);(out/'transform.json').write_text(json.dumps(dict(base_sha256=hashlib.sha256(base.encode()).hexdigest(),variant_sha256=hashlib.sha256(variant.encode()).hexdigest(),reverse_identical=True,substitutions=[dict(before=a,after=b)for a,b in subs]),indent=2)+'\n')
runtime=compile('runtime','/runtime/runtime.cpp',out/'runtime.cpp');main=compile('main','/runtime/external_registration.t.cpp',root/'tests/native/v5-02-x05-b2-c-c02-support/registry.cpp')
link=shlex.split(subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=build,text=True).splitlines()[-1].split(' && ')[1]);libs=link[link.index('src/engine/libplanar_runtime.a'):];libs=[a for a in libs if 'catch2' not in a and a!='src/engine/libplanar_runtime.a'];run('link',[link[0],'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++',main,runtime,*libs,'-o',out/'registry'])
for mode in ['embedded','success','execution','resource','snapshot']:run(mode,[out/'registry'],mode)
