"""Private copied-dependency production instrumentation; no shared input is linked or changed."""
from pathlib import Path
import json,shlex,shutil,subprocess,time,hashlib,stat
R=Path(__file__).resolve().parents[3];B=R/'build/rewrite-debug';S=Path(__file__).parent;E=R/'tests/native/v5-02-x05-c4-c2-evidence';P=E/'private';D=E/'dependencies';P.mkdir(exist_ok=True)
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
iface=arguments('runtime/runtime.cppm');runtime=arguments('runtime/runtime.cpp');fmt=arguments('ledger/format.cpp');test=arguments('runtime/runtime.t.cpp')
line=subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=B,text=True).splitlines()[-1]
link=expand(shlex.split(line.split(' && ')[1]));libs=link[link.index('src/engine/libplanar_runtime.a'):]
before=compiled();members=dict(before)
journals={n:entry(B/n) for n in ['.ninja_log','.ninja_deps']};write(P/'journals-before.json',journals)
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
 header=(E/'preedit-src_engine_runtime_runtime.cppm' if base else R/'src/engine/runtime/runtime.cppm').read_text()
 header=header.replace('  /// @brief Transfer the complete owner;', '  std::array<std::size_t,19> private_probe() const;\n  /// @brief Transfer the complete owner;',1)
 (P/(variant+'-runtime.cppm')).write_text(header)
 (P/'runtime.cppm').write_text(header);run(variant+'-interface',redirect(iface,P/'runtime.cppm','interface'))
 source=(E/'preedit-src_engine_runtime_runtime.cpp' if base else R/'src/engine/runtime/runtime.cpp').read_text()
 source=source.replace('module planar.engine.runtime;', 'extern "C" { void c4_stage(int); extern bool c4_fail_lua; extern bool c4_one_shot; extern int c4_failed_lua; extern std::size_t c4_lua_allocations; }\nmodule planar.engine.runtime;',1)
 source=source.replace('  auto result = std::realloc(p, size);','  ++c4_lua_allocations; if (c4_fail_lua) { if(c4_one_shot)c4_fail_lua=false; ++c4_failed_lua; return nullptr; }\n  auto result = std::realloc(p, size);',1)
 if not base:
  source=source.replace('          auto projected = call_retained_projection(', '          c4_stage(10);\n          auto projected = call_retained_projection(',1)
  source=source.replace('          auto resolved = owner.vm.resolve_projection_definition_impl(', '          c4_stage(9);\n          auto resolved = owner.vm.resolve_projection_definition_impl(',1)
  source=source.replace('          if (!projected) {', '          c4_stage(11);\n          if (!projected) {',1)
  a=source.index('std::expected<value_ptr, extension_execution_error> extension_invocation::execute()');b=source.index('  auto allocation_failure',a)
  section=source[a:b].replace('    return std::unexpected(*_owner->first_failure);','    c4_stage(34);\n    return std::unexpected(*_owner->first_failure);',1);assert 'c4_stage(34)' in section;source=source[:a]+section+source[b:]
  source=source.replace('        auto assembled = call_retained_assembly(', '        c4_stage(20);\n        auto assembled = call_retained_assembly(',1)
  source=source.replace('        if (!assembled)\n', '        c4_stage(21);\n        if (!assembled)\n',1)
  a=source.index('std::expected<value_ptr, diagnostic> call_retained_assembly');b=source.index('// Compare raw/proxy',a)
  section=source[a:b].replace('  auto result  = convert(l, base + 1, 1);','  c4_stage(31);\n  auto result  = convert(l, base + 1, 1);',1);assert 'c4_stage(31)' in section;source=source[:a]+section+source[b:]
  source=source.replace('owner.assembly_failure.emplace(assembly_error{(*assembled)->fields.at("error")});','owner.assembly_failure.emplace(assembly_error{(*assembled)->fields.at("error")});\n          c4_stage(32);',1)
  source=source.replace('  auto result = convert(l, base + 1, 1);','  c4_stage(12);\n  auto result = convert(l, base + 1, 1);',1)
  source=source.replace('int scan_projection(lua_State* l) {','int scan_projection(lua_State* l) {\n lua_gc(l,LUA_GCCOLLECT);',1)
  source=source.replace('int scan_assembly_response(lua_State* l) {','int scan_assembly_response(lua_State* l) {\n lua_gc(l,LUA_GCCOLLECT);',1)
 if not base:
  source=source.replace('      if (_owner->history_failure)\n        return std::visit', '      std::cerr << "outer status " << status << " refused " << c.extension_history.refused << " result " << (void*)c.extension_history.projection_result << " lua " << (lua_type(l,-1)==LUA_TSTRING?lua_tostring(l,-1):"table") << "\\n";\n      if (_owner->history_failure)\n        return std::visit',1)
 if not base:
  for n in [9,10,11,12,20,21,31,32,34]:assert 'c4_stage('+str(n)+')' in source,n
 methods=(S/'methods.inc').read_text()
 (P/(variant+'-runtime.cpp')).write_text(source+methods)
 (P/'runtime.cpp').write_text(source+methods);run(variant+'-runtime',redirect(runtime,P/'runtime.cpp','runtime',True))
 run(variant+'-probe',redirect(test,S/'private-probe.cpp','probe',True))
 redirected=[str(dest(B/a)) if a.endswith(('.o','.a','.pcm','.dylib')) else a for a in libs if a!='src/engine/libplanar_runtime.a']
 common=[link[0],'-stdlib=libc++','-arch','arm64','-L'+str(dest('/opt/homebrew/opt/llvm/lib/c++/libc++.1.dylib').parent),'-Wl,-rpath,'+str(dest('/opt/homebrew/opt/llvm/lib/c++/libc++.1.dylib').parent)]
 run(variant+'-link',common+[P/'probe.o',P/'interface.o',P/'runtime.o',*redirected,'-o',P/variant])
 run(variant+'-layout',[P/variant,'[c4c2-layout]','--reporter','junit','--out',P/(variant+'-layout.xml')])
 shutil.copy2(B/'LAYOUT.json',P/(variant+'-layout.json'));(B/'LAYOUT.json').unlink()
 if base:run('baseline-service-refusal',[P/variant,'C4C2 actual service protects outer frame across projection and assembly','--reporter','junit','--out',P/'baseline-service-refusal.xml'],42)
 else:run('green-private',[P/variant,'[c4c2-private]','--reporter','junit','--out',P/'green-private.xml'])
 assert compiled()==before
 assert {n:entry(B/n) for n in journals}==journals
 assert all(entry(Path(k))==v and entry(dest(k))==v for k,v in members.items())
assert {str(p) for p in D.rglob('*') if p.is_file()}=={str(dest(k)) for k in members}
write(P/'journals-final.json',dict(entries=journals,unchanged=True))
write(P/'dependency-final.json',dict(members=len(members),shared_and_copied_unchanged=True,copied_membership_exact=True))
a=json.loads((P/'baseline-layout.json').read_text());b=json.loads((P/'green-layout.json').read_text())
write(P/'layout-comparison.json',dict(baseline=a,candidate=b,deltas=[y-x for x,y in zip(a,b)]))
