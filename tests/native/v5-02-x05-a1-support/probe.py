"""Production-specific reversible probes linked against the exact candidate modules."""
from pathlib import Path
import hashlib,json,shlex,subprocess,sys,time
root=Path.cwd();build=root/'build/rewrite-debug';out=root/'tests/native/v5-02-x05-a1-evidence'/sys.argv[1];out.mkdir(exist_ok=False)
entries=json.loads((build/'compile_commands.json').read_text());commands=[];manifests={}
def sha(data):return hashlib.sha256(data.encode()).hexdigest()
def run(label,args):
 start=time.monotonic();r=subprocess.run(list(map(str,args)),cwd=build,capture_output=True,text=True);log=r.stdout+r.stderr
 (out/(label+'.log')).write_text(log);commands.append(dict(label=label,argv=list(map(str,args)),cwd=str(build),exit_code=r.returncode,seconds=time.monotonic()-start));(out/'commands.json').write_text(json.dumps(commands,indent=2)+'\n')
 assert r.returncode==0,(label,log[-4000:]);return log
def entry(suffix):return next(e for e in entries if e['file'].endswith(suffix))
def compile_copy(label,e,source,extra=[]):
 p=out/(label+'.cpp');p.write_text(source);cmd=shlex.split(e['command']);cmd[cmd.index('-o')+1]=str(out/(label+'.o'));cmd[cmd.index('-c')+1]=str(p);run('compile-'+label,cmd+extra);return str(out/(label+'.o'))
def change(source,subs,label):
 result=source
 for before,after in subs:
  assert result.count(before)==1,(label,before,result.count(before));result=result.replace(before,after)
 reverse=result
 for before,after in reversed(subs):
  assert reverse.count(after)==1;reverse=reverse.replace(after,before)
 assert reverse==source
 manifests[label]=dict(source_sha256=sha(source),candidate_sha256=sha(result),reverse_identical=True,substitutions=[dict(before=a,after=b,count=1) for a,b in subs]);return result
link=shlex.split(subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=build,text=True).splitlines()[-1].split(' && ')[1]);compiler=link[0];start=link.index('src/engine/libplanar_runtime.a');libs=[a for a in link[start:] if 'catch2' not in a]
base=(root/'src/engine/runtime/runtime.cpp').read_text();guard=(root/'src/engine/runtime/closure_guard/closure_guard.cpp').read_text();re=entry('/runtime/runtime.cpp');ge=entry('/closure_guard/closure_guard.cpp');pe=entry('/runtime/commands.t.cpp')
probe='''import std;
import planar.engine.runtime;
int main() {
 auto vm=planar::engine::command_vm::create();
 std::println("actual production create accepted={}", vm.has_value());
 if (!vm) std::println("diagnostic code={} message={}",vm.error().code,vm.error().message);
 return 0;
}
'''
po=compile_copy('probe',pe,probe)
# Pinned production source assembly variants: exact shipped originals except commands.lua.
source=(build/'src/engine/planar_runtime_source.cppm').read_text();original=(root/'src/stdlib/planar/commands.lua').read_text();needle='  local function init_command(invocation,context)'
source_objects={};source_options={}
for variant,after in [('writer','  local x05_a1_scalar=0\n'+needle+'\n    x05_a1_scalar=x05_a1_scalar+1'),('scratch',needle+'\n    local n=0;local function inc() n=n+1 end;inc();if n~=1 then error("scratch") end')]:
 assert original.count(needle)==1;altered=original.replace(needle,after);before=','.join('0x%02x'%x for x in original.encode())+',';new=','.join('0x%02x'%x for x in altered.encode())+','
 assert source.count(before)==2;module='planar.engine.x05_a1_'+variant+'_source';text=source.replace(before,new).replace('planar.engine.runtime_source',module)
 assert text.replace(new,before).replace(module,'planar.engine.runtime_source')==source
 manifests[variant+'-source']=dict(source_sha256=sha(source),candidate_sha256=sha(text),commands_before=needle,commands_after=after,commands_reverse_identical=altered.replace(after,needle)==original,assembly_reverse_identical=True)
 p=out/(variant+'_source.cppm');p.write_text(text);e=entry('/planar_runtime_source.cppm');cmd=shlex.split(e['command']);cmd[cmd.index('-c')+1]=str(p);cmd[cmd.index('-o')+1]=str(out/(variant+'_source.o'))
 cmd=[a for a in cmd if not a.startswith('-fmodule-output=')];cmd+=['-fmodule-output='+str(out/(variant+'_source.pcm'))];run('compile-'+variant+'-source',cmd)
 source_objects[variant]=str(out/(variant+'_source.o'));source_options[variant]=['-fmodule-file='+module+'='+str(out/(variant+'_source.pcm'))]
# Every product guard push retains its own require_stack check; independent graph entry invariant.
checked_guard=change(guard,[('    charge(result.edges);','''    if (l->top.p > l->ci->top.p) {
      std::println("DETECTED active frame overflow");
      throw failure::invalid_metadata;
    }
    charge(result.edges);''')], 'frame-invariant')
go=compile_copy('guard-instrumented',ge,checked_guard)
for mode in ['red-writer','writer','scratch','shipped-invariant','unknown-native','cumulative-native','native-badalloc','reservation-failure']:
 runtime=base;subs=[];objects=[];extra=[];expected='true' if mode in ['red-writer','scratch','shipped-invariant'] else 'false'
 if mode=='red-writer':runtime=(root/'tests/native/v5-02-x05-a1-evidence/original-source/src/engine/runtime/runtime.cpp').read_text()
 if mode in ['red-writer','writer','scratch']:
  variant='scratch' if mode=='scratch' else 'writer';subs.append(('import planar.engine.runtime_source;','import planar.engine.x05_a1_'+variant+'_source;'));extra=source_options[variant];objects.append(source_objects[variant])
 if mode=='unknown-native':subs.append(('  closure_guard::snapshot_curated_natives(l, native_catalog_index);','''  closure_guard::snapshot_curated_natives(l, native_catalog_index);
  lua_pushinteger(l,99);
  lua_pushcclosure(l,digest,1);
  lua_setglobal(l,"unknown_native_probe");'''))
 if mode=='cumulative-native':subs.append(('  if (!audit_command_roots(l))','  context(l).native_bytes=memory_max;\n  if (!audit_command_roots(l))'))
 if mode=='native-badalloc':subs.append(('    return std::pmr::new_delete_resource()->allocate(bytes, alignment);','    (void)alignment;\n    throw std::bad_alloc();'))
 if mode=='reservation-failure':
  subs += [('    if (!lua_checkstack(l, 2048)) {','    if (!lua_checkstack(l, 20)) {'),('int seal_environment(lua_State* l) {','''void* probe_refuse_growth(void* ud,void* p,std::size_t old,std::size_t size) {
 if(size>old) { std::println("denied real growth {} -> {}",old,size);return nullptr; }
 return allocate(ud,p,old,size);
}
int seal_environment(lua_State* l) {
 int gc=lua_gc(l,LUA_GCCOLLECT);
 std::println("supported GC={} entry top={} headroom={}",gc,lua_gettop(l),l->ci->top.p-l->top.p);
 lua_setallocf(l,probe_refuse_growth,&context(l));'''),('  if (!audit_command_roots(l))','  std::println("UNEXPECTED AUDIT");\n  if (!audit_command_roots(l))')]
 runtime=change(runtime,subs,mode);ro=compile_copy(mode,re,runtime,extra);selected=[a for a in libs if a!='src/engine/libplanar_runtime.a']
 if mode in ['red-writer','writer','scratch']:selected=[a for a in selected if a!='src/engine/libplanar_runtime_source.a']
 if mode=='shipped-invariant':selected=[a for a in selected if a!='src/engine/libplanar_closure_guard.a'];objects.append(go)
 binary=out/mode;run('link-'+mode,[compiler,'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++',po,ro,*objects,*selected,'-o',binary]);log=run('run-'+mode,[binary]);assert 'accepted='+expected in log,(mode,log)
 if mode in ['writer','unknown-native']:assert 'diagnostic code=internal' in log
 if mode in ['cumulative-native','native-badalloc','reservation-failure']:assert 'diagnostic code=resource_limit' in log
 if mode=='shipped-invariant':assert 'DETECTED' not in log
 if mode=='reservation-failure':assert 'denied real growth' in log and 'UNEXPECTED AUDIT' not in log
(out/'transformations.json').write_text(json.dumps(manifests,indent=2)+'\n')
print('Eight production-specific reversible probes passed')
