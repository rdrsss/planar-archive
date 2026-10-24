"""Copy all compiled inputs, recursively expand response files, and run isolated native probes."""
from pathlib import Path
import json,shlex,shutil,subprocess,time,hashlib,stat
R=Path(__file__).resolve().parents[3];B=R/'build/rewrite-debug';S=Path(__file__).parent;E=R/'tests/native/v5-02-x05-c4-d2-a-evidence';P=E/'private';D=E/'dependencies';P.mkdir(exist_ok=True)
def write(p,x):p.write_text(json.dumps(x,sort_keys=True,indent=2)+'\n')
def entry(p):return dict(mode=stat.S_IMODE(p.stat().st_mode),sha256=hashlib.file_digest(p.open('rb'),'sha256').hexdigest(),bytes=p.stat().st_size,mtime_ns=p.stat().st_mtime_ns)
def compiled():return {str(p.resolve()):entry(p) for p in B.rglob('*') if p.is_file() and p.suffix in {'.o','.pcm','.a','.dylib'}}
def expand(args):
 out=[]
 for a in args:
  if a.startswith('@'):out+=expand(shlex.split((B/a[1:]).read_text()))
  else:out.append(a)
 return out
cc=json.loads((B/'compile_commands.json').read_text())
a=expand(shlex.split(next(x['command'] for x in cc if x['file'].endswith('capture/profile.t.cpp'))))
line=subprocess.check_output(['ninja','-t','commands','planar_capture_profile_tests'],cwd=B,text=True).splitlines()[-1]
link=expand(shlex.split(line.split(' && ')[1]));libs=link[link.index('src/engine/libplanar_capture_profile.a'):]
before=compiled();members=dict(before);journals={n:entry(B/n) for n in ['.ninja_log','.ninja_deps']}
write(P/'journals-before.json',journals)
for v in a:
 if v.startswith('-fmodule-file='):
  p=(B/v[len('-fmodule-file='):].split('=',1)[1]).resolve();members[str(p)]=entry(p)
for p in Path('/opt/homebrew/opt/llvm/lib/c++').glob('*.dylib'):members[str(p.resolve())]=entry(p.resolve())
def dest(p):return D/str(Path(p).resolve()).lstrip('/')
assert not D.exists(), 'preserve previous attempt; use fresh evidence prefix'
write(P/'compiled-before.json',members)
for k,v in members.items():
 p=dest(k);p.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(k,p);assert entry(p)==v
for source in ['private.py','private-probe.cpp','syscalls.cpp','baseline-gap.cpp']:(P/source).write_bytes((S/source).read_bytes())
commands=[]
def run(label,args,expected=0):
 started=time.monotonic();p=subprocess.run(list(map(str,args)),cwd=B,capture_output=True,text=True)
 (P/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=list(map(str,args)),cwd=str(B),exit=p.returncode,seconds=time.monotonic()-started));write(P/'commands.json',commands);print(label,p.returncode,flush=True);assert p.returncode==expected,(p.stdout+p.stderr)[-5000:]
for i,v in enumerate(a):
 if v.startswith('-fmodule-file='):
  k,p=v[len('-fmodule-file='):].split('=',1);a[i]='-fmodule-file='+k+'='+str(dest(B/p))
for flag,ext in [('-o','.o'),('-MF','.d'),('-MT','.o')]:
 if flag in a:a[a.index(flag)+1]=str(P/('probe'+ext))
a[a.index('-c')+1]=str(S/'private-probe.cpp');a+=['-fno-implicit-modules','-fno-implicit-module-maps']
assert not any(v.startswith('@') for v in a)
run('syscalls',[a[0],'-dynamiclib','-fno-implicit-modules','-fno-implicit-module-maps','-std=c++26','-stdlib=libc++',S/'syscalls.cpp','-install_name','@rpath/libd1_syscalls.dylib','-o',P/'libd1_syscalls.dylib'])
run('compile',a)
redirected=[str(dest(B/v)) if v.endswith(('.o','.a','.pcm','.dylib')) else v for v in libs if v!='src/engine/libplanar_admission_syscalls.dylib']
common=[link[0],'-stdlib=libc++','-arch','arm64','-L'+str(dest('/opt/homebrew/opt/llvm/lib/c++/libc++.1.dylib').parent),'-Wl,-rpath,'+str(dest('/opt/homebrew/opt/llvm/lib/c++/libc++.1.dylib').parent)]
run('link',common+['-Wl,-rpath,'+str(P),P/'probe.o',P/'libd1_syscalls.dylib',*redirected,'-o',P/'probe'])
# Preserve and execute the old borrowed-cache gap separately. It is not a claim
# that old borrowed behavior changed; new retained mode supplies the missing authority.
baseline=a.copy(); baseline[baseline.index('-c')+1]=str(S/'baseline-gap.cpp')
for flag,ext in [('-o','.o'),('-MF','.d'),('-MT','.o')]:
 if flag in baseline:baseline[baseline.index(flag)+1]=str(P/('baseline-probe'+ext))
run('baseline-probe-compile',baseline)
old=expand(shlex.split(next(x['command'] for x in cc if x['file'].endswith('capture/native.cpp'))))
for i,v in enumerate(old):
 if v.startswith('-fmodule-file='):
  k,p=v[len('-fmodule-file='):].split('=',1);old[i]='-fmodule-file='+k+'='+str(dest(B/p))
for flag,ext in [('-o','.o'),('-MF','.d'),('-MT','.o')]:
 if flag in old:old[old.index(flag)+1]=str(P/('baseline-native'+ext))
(P/'baseline-native.cpp').write_bytes((E/'preedit-src_engine_capture_native.cpp').read_bytes())
old[old.index('-c')+1]=str(P/'baseline-native.cpp');old+=['-fno-implicit-modules','-fno-implicit-module-maps']
run('baseline-native-compile',old)
layout=old.copy()
(P/'candidate-native.cpp').write_bytes((R/'src/engine/capture/native.cpp').read_bytes())
layout[layout.index('-c')+1]=str(P/'candidate-native.cpp')
for flag,ext in [('-o','.o'),('-MF','.d'),('-MT','.o')]:
 if flag in layout:layout[layout.index(flag)+1]=str(P/('layout-native'+ext))
run('native-layout',layout+['-Xclang','-fdump-record-layouts'])
admission=expand(shlex.split(next(x['command'] for x in cc if x['file'].endswith('capture/admission.cpp'))))
for i,v in enumerate(admission):
 if v.startswith('-fmodule-file='):
  k,p=v[len('-fmodule-file='):].split('=',1);admission[i]='-fmodule-file='+k+'='+str(dest(B/p))
for flag,ext in [('-o','.o'),('-MF','.d'),('-MT','.o')]:
 if flag in admission:admission[admission.index(flag)+1]=str(P/('layout-admission'+ext))
(P/'candidate-admission.cpp').write_bytes((R/'src/engine/capture/admission.cpp').read_bytes())
admission[admission.index('-c')+1]=str(P/'candidate-admission.cpp')
run('admission-layout',admission+['-fno-implicit-modules','-fno-implicit-module-maps','-Xclang','-fdump-record-layouts'])


run('baseline-link',common+[P/'baseline-probe.o',P/'baseline-native.o',*redirected,'-o',P/'baseline'])
run('baseline-gap',[P/'baseline','[d2-a-baseline]','--reporter','junit','--out',P/'baseline.xml'],42)
run('run',[P/'probe','[d2-a-private]','--reporter','junit','--out',P/'probe.xml'])
assert compiled()==before
assert {n:entry(B/n) for n in journals}==journals
assert all(entry(Path(k))==v and entry(dest(k))==v for k,v in members.items())
assert {str(p) for p in D.rglob('*') if p.is_file()}=={str(dest(k)) for k in members}
write(P/'journals-final.json',dict(entries=journals,unchanged=True))
write(P/'dependency-final.json',dict(members=len(members),shared_and_copied_unchanged=True,copied_membership_exact=True,recursive_responses=True,implicit_modules=False))
