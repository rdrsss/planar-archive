"""Retain independent reviewer instrumentation against real generated shipped path."""
from pathlib import Path
import subprocess,json,shlex,sys,time
root=Path.cwd();build=root/'build/rewrite-debug';phase=sys.argv[1]
out=root/'tests/native/v5-02-x05-a0-c01-evidence'/phase;out.mkdir(exist_ok=False)
source=(root/'tests/native/v5-02-x05-a0-support/audit.cpp').read_text()
if '#include "lstate.h"' not in source:source=source.replace('#include "lua.h"','#include "lua.h"\n#include "lstate.h"')
needle='    charge(result.edges, bound.edges);'
assert source.count(needle)==1
source=source.replace(needle,needle+'\n    if (l->top.p > l->ci->top.p) {\n      std::println("DETECTED current-frame overflow: exceeded_by={} physical_headroom={}", l->top.p-l->ci->top.p, l->stack_last.p-l->top.p);\n      throw failure::invalid_metadata;\n    }')
# A pre-operation invariant observes every audit push/read that may grow top, unchanged for red and green.
for call,amount in [('lua_pushvalue(l, index);',1),('lua_pushnil(l);',1),('while (lua_next(l, index)) {',1),('for (int n = 1; lua_getupvalue(l, index, n); ++n) {',1),('lua_rawgeti(l, LUA_REGISTRYINDEX, slot);',1)]:
 # Snapshot/installation paths are outside the destructor-bearing audit and not instrumented here.
 start=source.index('struct inspector')
 prefix,body=source[:start],source[start:]
 if call.startswith('while'):
  body=body.replace(call,'while ((x05_invariant(l, 1), lua_next(l, index))) {')
 elif call.startswith('for'):
  body=body.replace(call,'for (int n = 1; (x05_invariant(l, 1), lua_getupvalue(l, index, n)); ++n) {')
 else:
  # Only audit's registry read, not remember_installed_callback.
  if call.startswith('lua_rawgeti'):
   at=body.index('std::expected<report, failure> audit_retained');body=body[:at]+body[at:].replace(call,'x05_invariant(l, 1);\n    '+call)
  else:body=body.replace(call,'x05_invariant(l, 1);\n      '+call)
 source=prefix+body
helper='''void x05_invariant(lua_State* l, int extra) {
  if (l->top.p > l->ci->top.p || l->ci->top.p-l->top.p < extra) {
    std::println("DETECTED current-frame overflow before push: extra={} headroom={}", extra, l->ci->top.p-l->top.p);
    throw failure::invalid_metadata;
  }
}
'''
source=source.replace('struct inspector {',helper+'struct inspector {')
(out/'instrumented-audit.cpp').write_text(source)
(out/'shipped-probe.cpp').write_text((root/'tests/native/v5-02-x05-a0-review-evidence/shipped-probe.cpp').read_text())
compiler='/opt/homebrew/opt/llvm/bin/clang++'
common=['-stdlib=libc++','-nostdinc++','-isystem','/opt/homebrew/opt/llvm/include/c++/v1','-g','-std=c++26','-arch','arm64','-Wall','-Wextra','-Wpedantic','-Werror']
flags=common+['-isystem',str(root/'vendor/lua/0b10/src'),'-fmodule-file=std=src/engine/CMakeFiles/@cmake_cxx_std.dir/std.pcm','-fmodule-file=planar.engine.x05_closure_audit=src/engine/CMakeFiles/planar_x05_closure_audit.dir/planar.engine.x05_closure_audit.pcm']
link=subprocess.check_output(['ninja','-t','commands','planar_x05_closure_audit_tests'],cwd=build,text=True).splitlines()[-1].split(' && ')[1];original=shlex.split(link);start=original.index('src/engine/libplanar_x05_closure_audit.a')
libs=[a for a in original[start:] if 'catch2' not in a and 'libplanar_x05_closure_audit.a' not in a]
commands=[]
def run(name,args):
 t=time.monotonic();r=subprocess.run(args,cwd=build,capture_output=True,text=True);output=r.stdout+r.stderr
 (out/(name+'.log')).write_text(output);commands.append(dict(name=name,argv=args,cwd=str(build),seconds=time.monotonic()-t,exit=r.returncode));(out/'commands.json').write_text(json.dumps(commands,indent=2)+'\n');print(name,r.returncode,output,flush=True);assert r.returncode==0;return output
run('compile-probe',[compiler,*common,'@src/engine/CMakeFiles/planar_x05_closure_audit_tests.dir/__/__/tests/native/v5-02-x05-a0-support/shipped.t.cpp.o.modmap','-c',str(out/'shipped-probe.cpp'),'-o',str(out/'probe.o')])
run('compile-instrumented',[compiler,*flags,'-c',str(out/'instrumented-audit.cpp'),'-o',str(out/'audit.o')])
for kind,objs in [('original',['src/engine/libplanar_x05_closure_audit.a']),('instrumented',[str(out/'audit.o')])]:
 run('link-'+kind,[compiler,'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++',str(out/'probe.o'),*objs,*libs,'-o',str(out/kind)])
 result=run(kind,[str(out/kind)])
 if phase=='red':assert ('accepted=true' in result if kind=='original' else 'DETECTED current-frame overflow' in result and 'accepted=false' in result)
 else:assert 'accepted=true' in result and 'DETECTED' not in result

if phase!='red':
 deep=(root/'tests/native/v5-02-x05-a0-review-evidence/probe.cpp').read_text()
 deep=deep.replace('int audit_frame(lua_State* l) {','int audit_frame(lua_State* l) {\n  if (!lua_checkstack(l, audit_stack_slots)) return 0;')
 deep=deep.replace('for i=1,40 do','for i=1,100 do')
 (out/'deep-probe.cpp').write_text(deep)
 run('compile-deep',[compiler,*flags,'-c',str(out/'deep-probe.cpp'),'-o',str(out/'deep.o')])
 run('link-deep',[compiler,*common,str(out/'deep.o'),str(out/'audit.o'),'-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++','src/engine/lib@cmake_cxx_std.a','libplanar_lua.a','-o',str(out/'deep')])
 result=run('deep',[str(out/'deep')]);assert 'audit accepted=true' in result and 'DETECTED' not in result
