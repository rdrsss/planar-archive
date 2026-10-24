"""Private copied-dependency production instrumentation; no shared input is linked or changed."""
from pathlib import Path
import json,shlex,shutil,subprocess,time,hashlib,stat
R=Path(__file__).resolve().parents[3];B=R/'build/rewrite-debug';S=Path(__file__).parent;A=R/'tests/native/v5-02-x05-c4-b3-evidence';E=R/'tests/native/v5-02-x05-c4-b3-c01-evidence';P=E/'private';D=E/'dependencies';P.mkdir(exist_ok=True)
def write(p,x):p.write_text(json.dumps(x,sort_keys=True,indent=2)+'\n')
def entry(p):return dict(mode=stat.S_IMODE(p.stat().st_mode),sha256=hashlib.sha256(p.read_bytes()).hexdigest(),bytes=p.stat().st_size)
def compiled():return {str(p.resolve()):entry(p) for p in B.rglob('*') if p.is_file() and p.suffix in {'.o','.pcm','.a','.dylib'}}
def expand(args):
 out=[]
 for a in args:
  if a.startswith('@'):out+=expand(shlex.split((B/a[1:]).read_text()))
  else:out.append(a)
 return out
cc=json.loads((B/'compile_commands.json').read_text())
def arguments(tail):return expand(shlex.split(next(x['command'] for x in cc if x['file'].endswith(tail))))
iface=arguments('runtime/runtime.cppm');runtime=arguments('runtime/runtime.cpp');fmt=arguments('ledger/format.cpp');test=arguments('runtime/runtime.t.cpp')
line=subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=B,text=True).splitlines()[-1]
link=expand(shlex.split(line.split(' && ')[1]));libs=link[link.index('src/engine/libplanar_runtime.a'):]
before=compiled();members=dict(before)
for a in iface+runtime+fmt+test:
 if a.startswith('-fmodule-file='):
  path=(B/a[len('-fmodule-file='):].split('=',1)[1]).resolve();members[str(path)]=entry(path)
for p in Path('/opt/homebrew/opt/llvm/lib/c++').glob('*.dylib'):members[str(p.resolve())]=entry(p.resolve())
def old_dest(p):return D/str(Path(p).resolve()).lstrip('/')
if D.exists():
 prior=json.loads((P/'compiled-before.json').read_text())
 assert {str(p) for p in D.rglob('*') if p.is_file()}=={str(old_dest(k)) for k in prior}
 assert all(entry(old_dest(k))==v for k,v in prior.items())
 assert D==E/'dependencies'
 shutil.rmtree(D)
write(P/'compiled-before.json',members)
def dest(p):return D/str(Path(p).resolve()).lstrip('/')
for k,v in members.items():
 p=dest(k);p.parent.mkdir(parents=True,exist_ok=True)
 if not p.exists() or entry(p)!=v:shutil.copy2(k,p)
 assert entry(p)==v
commands=[]
def run(label,args,expected=0):
 start=time.monotonic();p=subprocess.run(list(map(str,args)),cwd=B,capture_output=True,text=True)
 (P/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=list(map(str,args)),cwd=str(B),exit=p.returncode,seconds=time.monotonic()-start));write(P/'commands.json',commands);print(label,p.returncode,flush=True)
 assert p.returncode==expected,(p.stdout+p.stderr)[-5000:]
def redirect(args,source,out,custom=False):
 a=list(args)
 for i,v in enumerate(a):
  if v.startswith('-fmodule-file='):
   k,p=v[len('-fmodule-file='):].split('=',1);a[i]='-fmodule-file='+k+'='+str(P/'runtime.pcm' if custom and k=='planar.engine.runtime' else dest(B/p))
  elif v.startswith('-fmodule-output='):a[i]='-fmodule-output='+str(P/'runtime.pcm')
 for flag,extension in [('-o','.o'),('-MF','.d'),('-MT','.o')]:
  if flag in a:a[a.index(flag)+1]=str(P/(out+extension))
 a[a.index('-c')+1]=str(source);a+=['-fno-implicit-modules','-fno-implicit-module-maps','-I'+str(S)]
 return a

for variant in ['baseline','green']:
 base=variant=='baseline'
 header=(R/'src/engine/runtime/runtime.cppm').read_text()
 header=header.replace('  /// @brief Transfer the complete owner;', '  std::array<std::size_t,9> private_probe() const;\n  /// @brief Transfer the complete owner;',1)
 (P/(variant+'-runtime.cppm')).write_text(header)
 (P/'runtime.cppm').write_text(header);run(variant+'-interface',redirect(iface,P/'runtime.cppm','interface'))
 source=(E/'preedit-src_engine_runtime_runtime.cpp' if base else R/'src/engine/runtime/runtime.cpp').read_text()
 if True:
  source=source.replace('module planar.engine.runtime;', 'extern "C" { void c4_stage(int); extern bool c4_fail_lua; extern int c4_failed_lua; extern std::size_t c4_lua_allocations; }\nmodule planar.engine.runtime;',1)
  source=source.replace('  auto result = std::realloc(p, size);','  ++c4_lua_allocations; if (c4_fail_lua) { ++c4_failed_lua; return nullptr; }\n  auto result = std::realloc(p, size);',1)
  source=source.replace('    _owner->history_attempted = true;','    c4_stage(1);\n    _owner->history_attempted = true;',1)
  source=source.replace('    auto                     guard  = std::make_shared<original_history_guard>(_owner->capture);','    c4_stage(2);\n    auto                     guard  = std::make_shared<original_history_guard>(_owner->capture);',1)
  source=source.replace('    // Original receipts are reverified after success AND failure,','    c4_stage(3);\n    // Original receipts are reverified after success AND failure,',1)
  source=source.replace('      bridge.result = bridge.read(*bridge.owner);','      bridge.result = bridge.read(*bridge.owner);\n      c4_stage(4);',1)
  source=source.replace('      if (_owner->history_failure)\n        return std::unexpected(*_owner->history_failure);','      c4_stage(5);\n      if (_owner->history_failure) { c4_stage(6);\n        return std::unexpected(*_owner->history_failure); }',1)
  source=source.replace('    auto checked = _capture.verify_file("extension.ledger", _descriptor);','    c4_stage(7);\n    auto checked = _capture.verify_file("extension.ledger", _descriptor);',1)
  if not base:
   source=source.replace('commands(l).extension_history.context_identity = lua_topointer(l, -1);', 'commands(l).extension_history.context_identity = lua_topointer(l, -1); c4_stage(8);',1)
   assert 'c4_stage(8);' in source
  for n in range(1,8):assert 'c4_stage('+str(n)+');' in source,n
 layout='\nnamespace planar::engine {\nstd::array<std::size_t,9> extension_invocation::private_probe() const {\n auto& vm=*_owner->vm._owner;\n return {sizeof(command_state),sizeof(command_vm::implementation),sizeof(implementation),BRIDGE,\n vm.runtime.native_bytes,OWNER,static_cast<std::size_t>(vm.runtime.push_memo),static_cast<std::size_t>(bool(vm.runtime.input)),IDENTITY};\n}\n}\n'
 layout=layout.replace('BRIDGE','sizeof(extension_history_bridge)').replace('OWNER','static_cast<std::size_t>(bool(vm.command.extension_history.owner))').replace('IDENTITY','0' if base else 'static_cast<std::size_t>(bool(vm.command.extension_history.context_identity))')
 (P/(variant+'-runtime.cpp')).write_text(source+layout)
 (P/'runtime.cpp').write_text(source+layout);run(variant+'-runtime',redirect(runtime,P/'runtime.cpp','runtime',True))
 test_args=redirect(test,S/'private-probe.cpp','probe',True)

 run(variant+'-probe',test_args)
 redirected=[str(dest(B/a)) if a.endswith(('.o','.a','.pcm','.dylib')) else a for a in libs if a!='src/engine/libplanar_runtime.a']
 common=[link[0],'-stdlib=libc++','-arch','arm64','-L'+str(dest('/opt/homebrew/opt/llvm/lib/c++/libc++.1.dylib').parent),'-Wl,-rpath,'+str(dest('/opt/homebrew/opt/llvm/lib/c++/libc++.1.dylib').parent)]
 run(variant+'-link',common+[P/'probe.o',P/'interface.o',P/'runtime.o',*redirected,'-o',P/variant])
 run(variant+'-red',[P/variant,'[c4b3-red]','--reporter','junit','--out',P/(variant+'-red.xml')],0)
 shutil.copy2(B/'BASE_LAYOUT.json',P/(variant+'-layout.json'));(B/'BASE_LAYOUT.json').unlink()
 if not base:
  run('green-private',[P/variant,'[c4b3-private]','--reporter','junit','--out',P/'green-private.xml'])
  shutil.copy2(B/'LAYOUT.json',P/'LAYOUT.json');(B/'LAYOUT.json').unlink()
  run('reviewer-private',[P/variant,'[c4b3-reviewer]','--reporter','junit','--out',P/'reviewer-private.xml'],0)
 else:
  run('baseline-reviewer',[P/variant,'[c4b3-reviewer]','--reporter','junit','--out',P/'baseline-reviewer.xml'],42)
 assert compiled()==before
 assert all(entry(Path(k))==v and entry(dest(k))==v for k,v in members.items())
assert {str(p) for p in D.rglob('*') if p.is_file()}=={str(dest(k)) for k in members}
write(P/'dependency-final.json',dict(members=len(members),shared_and_copied_unchanged=True,copied_membership_exact=True))

a=json.loads((P/'baseline-layout.json').read_text());b=json.loads((P/'green-layout.json').read_text());assert b['command']-a['command']==b['bridge']-a['bridge']==8;assert b['vm']-a['vm']==8;assert b['invocation']==a['invocation'];assert b['native_bytes']-a['native_bytes']==8
write(P/'layout-comparison.json',dict(baseline=a,candidate=b,exact_inline_growth_and_initial_charge=True))
