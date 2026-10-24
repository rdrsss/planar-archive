"""Private copied-dependency production instrumentation; no shared input is linked or changed."""
from pathlib import Path
import json,shlex,shutil,subprocess,time,hashlib,stat,re
R=Path(__file__).resolve().parents[3];B=R/'build/rewrite-debug';S=Path(__file__).parent;E=R/'tests/native/v5-02-x05-c4-d4-evidence';P=E/'private';D=E/'dependencies';P.mkdir(exist_ok=True)
def write(p,x):p.write_text(json.dumps(x,sort_keys=True,indent=2)+'\n')
def entry(p):return dict(mode=stat.S_IMODE(p.stat().st_mode),sha256=hashlib.sha256(p.read_bytes()).hexdigest(),bytes=p.stat().st_size,mtime_ns=p.stat().st_mtime_ns)
def compiled():return {str(p.resolve()):entry(p) for p in B.rglob('*') if p.is_file() and p.suffix in {'.o','.pcm','.a','.dylib'}}
def expand(args):
 out=[]
 for a in args:
  if a.startswith('@'):out+=expand(shlex.split((B/a[1:]).read_text()))
  else:out.append(a)
 return out
cc=json.loads((B/'compile_commands.json').read_text())
def arguments(tail):return expand(shlex.split(next(x['command'] for x in cc if x['file'].endswith(tail))))
iface=arguments('runtime/runtime.cppm');runtime=arguments('runtime/runtime.cpp');test=arguments('runtime/external_registration.t.cpp')
line=subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=B,text=True).splitlines()[-1]
link=expand(shlex.split(line.split(' && ')[1]));libs=link[link.index('src/engine/libplanar_runtime.a'):]
before=compiled();members=dict(before)
journals={n:entry(B/n) for n in ['.ninja_log','.ninja_deps']};write(P/'journals-before.json',journals)
for a in iface+runtime+test:
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

header=(R/'src/engine/runtime/runtime.cppm').read_text().replace('  /// @brief Transfer the complete owner;', '  std::array<std::size_t,23> private_probe() const;\n  void private_limit(int metric);\n  /// @brief Transfer the complete owner;',1)
(P/'runtime.cppm').write_text(header);run('interface',redirect(iface,P/'runtime.cppm','interface'))
source=(R/'src/engine/runtime/runtime.cpp').read_text()
source=source.replace('module planar.engine.runtime;', 'extern "C" { void d3_stage(int); extern bool d3_fail_lua; extern bool d3_one_shot; extern int d3_lua_refusals; }\nmodule planar.engine.runtime;',1)
source=source.replace('  auto result = std::realloc(p, size);','  if(d3_fail_lua){if(d3_one_shot)d3_fail_lua=false;++d3_lua_refusals;return nullptr;}\n  auto result = std::realloc(p, size);',1)
source=source.replace('  owner.role_attempted = true;', '  owner.role_attempted = true; d3_stage(1);',1)
source=source.replace('seal(); // All bytes/topology', 'seal(); d3_stage(2); // All bytes/topology',1)
source=re.sub(r'auto prepared\s*= owner.vm.validate_profile_impl', 'd3_stage(3);\n        auto prepared = owner.vm.validate_profile_impl', source, count=1)
source=source.replace('owner.authenticated_role = (*prepared)->fields.at("value");','owner.authenticated_role = (*prepared)->fields.at("value");\n        d3_stage(4);',1)
source=source.replace('int execute_shape(lua_State* l) {','int execute_shape(lua_State* l) {\n if(commands(l).extension_history.role_enabled)lua_gc(l,LUA_GCCOLLECT);',1)
source=source.replace('auto result = call_retained_value(_owner->lua, selected->function_reference, input);', 'd3_stage(5);\n        auto result = call_retained_value(_owner->lua, selected->function_reference, input);',1)
for n in [1,2,3,4,5]:assert 'd3_stage('+str(n)+')' in source,n
source=source.replace('auto projected = call_retained_projection(', 'd3_stage(6);\n          auto projected = call_retained_projection(',1)
source=source.replace('custom = std::move(*projected);', 'custom = std::move(*projected); d3_stage(7);',1)
source=source.replace('auto prepared = call_retained_value(vm.lua, c.shape_phases[9], owner.role_basis);', 'd3_stage(8);\n          auto prepared = call_retained_value(vm.lua, c.shape_phases[9], owner.role_basis); d3_stage(9);',1)
source=source.replace('auto assembled = call_retained_assembly(vm.lua, c.shape_phases[6], input);', 'd3_stage(10);\n        auto assembled = call_retained_assembly(vm.lua, c.shape_phases[6], input); d3_stage(11);',1)
for n in range(6,12): assert 'd3_stage('+str(n)+')' in source,n
(P/'runtime.cpp').write_text(source+(S/'methods.inc').read_text());run('runtime',redirect(runtime,P/'runtime.cpp','runtime',True))
(P/'probe.cpp').write_text((R/'src/engine/runtime/external_registration.t.cpp').read_text()+(S/'private-tests.inc').read_text());run('probe',redirect(test,P/'probe.cpp','probe',True))
redirected=[str(dest(B/a)) if a.endswith(('.o','.a','.pcm','.dylib')) else a for a in libs if a!='src/engine/libplanar_runtime.a']
common=[link[0],'-stdlib=libc++','-arch','arm64','-L'+str(dest('/opt/homebrew/opt/llvm/lib/c++/libc++.1.dylib').parent),'-Wl,-rpath,'+str(dest('/opt/homebrew/opt/llvm/lib/c++/libc++.1.dylib').parent),'-Wl,-rpath,'+str(dest(B/'src/engine'))]
run('link',common+[P/'probe.o',P/'interface.o',P/'runtime.o',*redirected,'-o',P/'green'])
run('loader-inspection',['otool','-L',P/'green'])
run('private',['env','DYLD_PRINT_LIBRARIES=1',P/'green','[d3-private]','--reporter','junit','--out',P/'private.xml'])
loader=(P/'private.log').read_text();assert str(dest(B/'src/engine/libplanar_admission_syscalls.dylib')) in loader
write(P/'loader-proof.json',dict(copied_helper_path=str(dest(B/'src/engine/libplanar_admission_syscalls.dylib')),actual_helper_load_logged=True,libcxx_install_name='/usr/lib/libc++.1.dylib',copied_libcxx_load_claim=False))
shutil.copy2(B/'D4-LAYOUT.json',P/'layout.json');(B/'D4-LAYOUT.json').unlink()
assert compiled()==before
assert {n:entry(B/n) for n in journals}==journals
assert all(entry(Path(k))==v and entry(dest(k))==v for k,v in members.items())
assert {str(p) for p in D.rglob('*') if p.is_file()}=={str(dest(k)) for k in members}
write(P/'journals-final.json',dict(entries=journals,unchanged=True))
write(P/'dependency-final.json',dict(members=len(members),shared_and_copied_unchanged=True,copied_membership_exact=True))
