from pathlib import Path
import json,shlex,shutil,subprocess,time,sys
from protect import R,O,entry,write
freeze=json.loads((O/'frozen-support.json').read_text());assert {str(p.relative_to(R)):entry(p) for p in Path(__file__).parent.rglob('*')}==freeze
B=R/'build/rewrite-debug';S=Path(__file__).parent;D=O/'dependencies';P=O/'private';P.mkdir(exist_ok=True);commands=[]
def compiled():return {str(p.resolve()):entry(p) for p in B.rglob('*') if p.is_file() and p.suffix in {'.o','.pcm','.a','.dylib'}}
def expand(args):
 out=[]
 for a in args:
  if a.startswith('@'):out+=expand(shlex.split((B/a[1:]).read_text()))
  else:out.append(a)
 return out
cc=json.loads((B/'compile_commands.json').read_text());args=expand(shlex.split(next(x['command'] for x in cc if x['file'].endswith('/capture/profile.t.cpp'))))
line=subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=B,text=True).splitlines()[-1]
link=expand(shlex.split(line.split(' && ')[1]));libs=link[link.index('src/engine/libplanar_runtime.a'):];libs+=[str(B/'libplanar_sqlite.a'),str(B/'src/engine/libplanar_admission_syscalls.dylib')]
before=compiled();members=dict(before)
for a in args:
 if a.startswith('-fmodule-file='):
  p=(B/a[len('-fmodule-file='):].split('=',1)[1]).resolve();members[str(p)]=entry(p)
for a in libs:
 if a.endswith(('.o','.pcm','.a','.dylib')):
  p=(B/a).resolve();members[str(p)]=entry(p)
for p in Path('/opt/homebrew/opt/llvm/lib/c++').glob('*.dylib'):members[str(p.resolve())]=entry(p.resolve())
write(O/'compiled-before.json',members)
def dest(p):return D/str(Path(p).resolve()).lstrip('/')
for k,v in members.items():
 p=dest(k);p.parent.mkdir(parents=True,exist_ok=True)
 if p.exists():p.unlink()
 shutil.copy2(k,p);assert entry(p)==v
write(O/'copied-before.json',{str(dest(k)):entry(dest(k)) for k in members})
def run(label,a):
 start=time.monotonic();p=subprocess.run([str(x) for x in a],cwd=B,capture_output=True,text=True);(P/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=[str(x) for x in a],cwd=str(B),exit=p.returncode,seconds=time.monotonic()-start));write(P/'commands.json',commands);print(label,p.returncode,flush=True);assert p.returncode==0,(p.stdout[-1000:],p.stderr[-4000:])
for i,a in enumerate(args):
 if a.startswith('-fmodule-file='):
  k,p=a[len('-fmodule-file='):].split('=',1);args[i]='-fmodule-file='+k+'='+str(dest(B/p))
 elif a.startswith('-fmodule-output='):args[i]='-fmodule-output='+str(P/'probe.pcm')
for flag,suffix in [('-o','.o'),('-MF','.d'),('-MT','.o')]:
 if flag in args:args[args.index(flag)+1]=str(P/('probe'+suffix))
args[args.index('-c')+1]=str(S/'probe.cpp');args+=['-fno-implicit-modules','-fno-implicit-module-maps','-I'+str(R/'vendor/sqlite/b9c7')]
run('compile',args)
common=[link[0],'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,'+str(dest(B/'src/engine/libplanar_admission_syscalls.dylib').parent)]
libs=[str(dest(B/a)) if a.endswith(('.o','.a','.pcm','.dylib')) else a for a in libs]
run('link',common+[P/'probe.o',*libs,'-o',P/'probe'])
run('probe',[P/'probe','[c4b0]','--reporter','junit','--out',P/'probe.xml'])
assert compiled()==before
assert all(entry(Path(k))==v and entry(dest(k))==v for k,v in members.items())
write(O/'compiled-final.json',dict(entries=len(members),shared_and_copied_unchanged=True))

assert {str(p.relative_to(R)):entry(p) for p in S.rglob('*')}==freeze
assert {str(p) for p in D.rglob('*') if p.is_file()}=={str(dest(k)) for k in members}
write(O/'source-bound-result.json',{'support_unchanged':True,'compiled_members':len(members),'copied_membership_exact':True})
