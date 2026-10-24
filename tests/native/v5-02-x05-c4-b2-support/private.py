"""Private copied-dependency production instrumentation; no shared input is linked or changed."""
from pathlib import Path
import json,shlex,shutil,subprocess,time,hashlib,stat
R=Path(__file__).resolve().parents[3];B=R/'build/rewrite-debug';S=Path(__file__).parent;E=R/'tests/native/v5-02-x05-c4-b2-evidence';P=E/'private';D=E/'dependencies';P.mkdir(exist_ok=True)
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
interface=(R/'src/engine/runtime/runtime.cppm').read_text().replace('  /// @brief Execute the admitted zero-capability','  value_ptr private_history() const;\n  /// @brief Execute the admitted zero-capability',1)
(P/'runtime.cppm').write_text(interface);run('interface',redirect(iface,P/'runtime.cppm','interface'))
s=(R/'src/engine/runtime/runtime.cpp').read_text().replace('module planar.engine.runtime;','extern "C" void c4_stage(int);\nmodule planar.engine.runtime;',1)
s=s.replace('    auto checked = _capture.verify_file("extension.ledger", descriptor);','    c4_stage(1);\n    auto checked = _capture.verify_file("extension.ledger", descriptor);')
s=s.replace('    _descriptor = ::fcntl(descriptor, F_DUPFD_CLOEXEC, 0);','    c4_stage(2);\n    _descriptor = ::fcntl(descriptor, F_DUPFD_CLOEXEC, 0);')
s=s.replace('    auto checked = _capture.verify_file("extension.ledger", _descriptor);','    c4_stage(3);\n    auto checked = _capture.verify_file("extension.ledger", _descriptor);')
s=s.replace('    auto                     guard  = std::make_shared<original_history_guard>(_owner->capture);','    c4_stage(10);\n    auto                     guard  = std::make_shared<original_history_guard>(_owner->capture);')
s=s.replace('    // Original receipts are reverified after success AND failure,','    c4_stage(4);\n    // Original receipts are reverified after success AND failure,')
assert all(('c4_stage('+str(n)+');') in s for n in [1,2,3,4,10])
s+='\nnamespace planar::engine { value_ptr extension_invocation::private_history() const {return _owner->authenticated_history;} }\n'
(P/'runtime.cpp').write_text(s);run('runtime',redirect(runtime,P/'runtime.cpp','runtime',True))
s=(R/'src/engine/ledger/format.cpp').read_text();s=s.replace('module planar.engine.ledger.format;', '''extern "C" { int c4_header_reads = 0; }
static ssize_t c4_pread(int fd, void* p, size_t n, off_t o) { ++c4_header_reads; return ::pread(fd,p,n,o); }
module planar.engine.ledger.format;''',1).replace('::pread(', 'c4_pread(')
s=s.replace('return c4_pread(fd,p,n,o);','return ::pread(fd,p,n,o);')
(P/'format-green.cpp').write_text(s);run('format-green',redirect(fmt,P/'format-green.cpp','format-green'))
a=s.index('  if (guard) {\n    // Bind the actual descriptor');b=s.index('  auto checked = header(held.fd);',a)
red=s[:a]+s[b:];(P/'format-red.cpp').write_text(red);write(P/'red-control.json',dict(kind='deliberately removed new pre-header bind as test sensitivity control; not a historical bug reproduction',removed=s[a:b]))
run('format-red',redirect(fmt,P/'format-red.cpp','format-red'))
run('probe',redirect(test,S/'private-probe.cpp','probe',True))
libs=[str(dest(B/a)) if a.endswith(('.o','.a','.pcm','.dylib')) else a for a in libs if a not in {'src/engine/libplanar_runtime.a','src/engine/libplanar_ledger_format.a'}]
common=[link[0],'-stdlib=libc++','-arch','arm64','-L'+str(dest('/opt/homebrew/opt/llvm/lib/c++/libc++.1.dylib').parent),'-Wl,-rpath,'+str(dest('/opt/homebrew/opt/llvm/lib/c++/libc++.1.dylib').parent)]
for label in ['red','green']:
 run(label+'-link',common+[P/'probe.o',P/'interface.o',P/'runtime.o',P/('format-'+label+'.o'),dest(B/'src/engine/CMakeFiles/planar_ledger_format.dir/ledger/format.cppm.o'),*libs,'-o',P/label])
 run(label+'-header',[P/label,'[c4b2-header]','--reporter','junit','--out',P/(label+'-header.xml')],42 if label=='red' else 0)
run('green-native',[P/'green','[c4b2-native]','--reporter','junit','--out',P/'green-native.xml'])
assert compiled()==before
assert all(entry(Path(k))==v and entry(dest(k))==v for k,v in members.items())
assert {str(p) for p in D.rglob('*') if p.is_file()}=={str(dest(k)) for k in members}
write(P/'dependency-final.json',dict(members=len(members),shared_and_copied_unchanged=True,copied_membership_exact=True))
