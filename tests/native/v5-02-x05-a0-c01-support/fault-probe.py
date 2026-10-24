"""Evidence-only source variants: original graph invariant and allocator refusal."""
from pathlib import Path
import json,shlex,subprocess,sys,time
root=Path.cwd();build=root/'build/rewrite-debug';mode=sys.argv[1];out=root/'tests/native/v5-02-x05-a0-c01-evidence'/mode;out.mkdir(exist_ok=False)
commands=[]
def run(name,args):
 start=time.monotonic();r=subprocess.run(args,cwd=build,capture_output=True,text=True);text=r.stdout+r.stderr
 (out/(name+'.log')).write_text(text);commands.append(dict(name=name,argv=list(map(str,args)),cwd=str(build),exit=r.returncode,seconds=time.monotonic()-start));(out/'commands.json').write_text(json.dumps(commands,indent=2)+'\n');print(name,r.returncode,text,flush=True);assert r.returncode==0;return text
entries=json.loads((build/'compile_commands.json').read_text())
def compile_copy(kind,source):
 e=next(e for e in entries if e['file'].endswith('x05_a0_0_runtime.cpp') if kind=='runtime') if kind=='runtime' else next(e for e in entries if e['file'].endswith('v5-02-x05-a0-support/audit.cpp'))
 p=out/(kind+'.cpp');p.write_text(source);cmd=shlex.split(e['command']);cmd[cmd.index('-o')+1]=str(out/(kind+'.o'));cmd[cmd.index('-c')+1]=str(p);run('compile-'+kind,cmd);return str(out/(kind+'.o'))
link=shlex.split(subprocess.check_output(['ninja','-t','commands','planar_x05_closure_audit_tests'],cwd=build,text=True).splitlines()[-1].split(' && ')[1]);start=link.index('src/engine/libplanar_x05_closure_audit.a');libs=[a for a in link[start:] if 'catch2' not in a]
compiler=link[0];obj=[]
if mode in ['red-exact','green-exact']:
 original=root/'tests/native/v5-02-x05-a0-c01-evidence/original-source'
 run('generate-original',[sys.executable,str(original/'generate.py'),str(root),str(out/'generated')])
 runtime=(out/'generated/x05_a0_0_runtime.cpp').read_text()
 audit=(original/'audit.cpp').read_text()
 if mode=='green-exact':
  runtime=(build/'src/engine/v5-02-x05-a0-generated/x05_a0_0_runtime.cpp').read_text()
  audit=(root/'tests/native/v5-02-x05-a0-support/audit.cpp').read_text()
 if '#include "lstate.h"' not in audit:audit=audit.replace('#include "lua.h"','#include "lua.h"\n#include "lstate.h"')
 needle='    charge(result.edges, bound.edges);';assert audit.count(needle)==1
 audit=audit.replace(needle,needle+'\n    if (l->top.p > l->ci->top.p) {\n      std::println("DETECTED current-frame overflow: exceeded_by={} physical_headroom={}", l->top.p-l->ci->top.p, l->stack_last.p-l->top.p);\n      throw failure::invalid_metadata;\n    }')
 obj=[compile_copy('runtime',runtime),compile_copy('audit',audit)]
 libs=[a for a in libs if a not in ['src/engine/libplanar_x05_runtime_0.a','src/engine/libplanar_x05_closure_audit.a']]
else:
 assert mode in ['reservation-failure','reservation-failure-final']
 runtime=(build/'src/engine/v5-02-x05-a0-generated/x05_a0_0_runtime.cpp').read_text()
 original_runtime=runtime;transformations=[]
 def transform(before,after):
  global runtime
  assert runtime.count(before)==1
  transformations.append(dict(before=before,after=after,count=1))
  runtime=runtime.replace(before,after)
 before='    if (!lua_checkstack(l, 2048)) {';assert runtime.count(before)==1
 transform(before,'    if (!lua_checkstack(l, 20)) {')
 before='int seal_environment(lua_State* l) {\n';assert runtime.count(before)==1
 transform(before,'''void* x05_refuse_growth(void* ud, void* p, std::size_t old, std::size_t size) {
  if (size > old) {
    std::println("reservation allocator refused growth: old={} new={}", old, size);
    return nullptr;
  }
  return allocate(ud, p, old, size);
}
int seal_environment(lua_State* l) {
  int x05_gc = lua_gc(l, LUA_GCCOLLECT);
  std::println("supported GC completed status={}", x05_gc);
  std::println("reservation entry top={} current_headroom={} physical_headroom={}", lua_gettop(l), l->ci->top.p-l->top.p, l->stack_last.p-l->top.p);
  lua_setallocf(l, x05_refuse_growth, &context(l));
''')
 before='    context(l).stack_exhausted = true;\n    return 0;';assert runtime.count(before)==1
 transform(before,'    context(l).stack_exhausted = true;\n    std::println("reservation failed safely top={} headroom={}", lua_gettop(l), l->ci->top.p-l->top.p);\n    return 0;')
 transform('  // X05-A0 only: inspect every original root before any proxy freezing.','  std::println("UNEXPECTED AUDIT ENTERED");\n  // X05-A0 only: inspect every original root before any proxy freezing.')
 before='    status = lua_pcall(l, 0, 0, 0);'
 transform(before,before+'\n    std::println("reservation protected return status={} top={} headroom={}", status, lua_gettop(l), l->ci->top.p-l->top.p);')
 reverse=runtime
 for change in reversed(transformations):
  assert reverse.count(change['after'])==1
  reverse=reverse.replace(change['after'],change['before'])
 assert reverse==original_runtime
 (out/'transformations.json').write_text(json.dumps(dict(substitutions=transformations,reverse_identical=True),indent=2)+'\n')
 obj=[compile_copy('runtime',runtime)];libs=[a for a in libs if a!='src/engine/libplanar_x05_runtime_0.a']
probe=out/'probe.cpp';probe.write_text('''import std;
import planar.engine.x05_a0_0;
int main() {
  auto vm=planar::engine::x05_a0_0::command_vm::create();
  std::println("actual generated shipped bundle accepted={}",vm.has_value());
  if (!vm) std::println("diagnostic code={}",vm.error().code);
  return 0;
}
''')
e=next(e for e in entries if e['file'].endswith('v5-02-x05-a0-support/shipped.t.cpp'));cmd=shlex.split(e['command']);cmd[cmd.index('-o')+1]=str(out/'probe.o');cmd[cmd.index('-c')+1]=str(probe);run('compile-probe',cmd)
run('link',[compiler,'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++',str(out/'probe.o'),*obj,*libs,'-o',str(out/'probe')])
result=run('run',[str(out/'probe')])
if mode=='green-exact':assert 'accepted=true' in result and 'DETECTED' not in result
elif mode=='red-exact':assert 'DETECTED current-frame overflow: exceeded_by=2' in result
else:assert 'accepted=false' in result and 'supported GC completed status=0' in result and 'reservation protected return status=0 top=0 headroom=20' in result and 'reservation allocator refused growth:' in result and 'reservation failed safely top=0 headroom=20' in result and 'diagnostic code=resource_limit' in result and 'UNEXPECTED AUDIT' not in result
