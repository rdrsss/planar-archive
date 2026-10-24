"""Compile private fault/read checks with copied dependencies and no implicit modules."""
from pathlib import Path
import hashlib,json,os,shlex,shutil,stat,subprocess,time,xml.etree.ElementTree as ET
import sys
sys.dont_write_bytecode = True
from freeze import ROOT as R,EVIDENCE as E,verify_candidate,write
B=R/'build/rewrite-debug';S=Path(__file__).parent;P=E/'private-dependencies';O=E/'private-probes';O.mkdir();commands=[]
def entry(p):
 return dict(mode=stat.S_IMODE(p.stat().st_mode),size=p.stat().st_size,sha256=hashlib.sha256(p.read_bytes()).hexdigest())
def compiled():
 return {str(p.resolve()):entry(p) for p in B.rglob('*') if p.is_file() and p.suffix in {'.o','.pcm','.a','.dylib'}}
def run(label,args,cwd=R,expected=0):
 verify_candidate();start=time.time();p=subprocess.run(list(map(str,args)),cwd=cwd,capture_output=True,text=True)
 (O/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=list(map(str,args)),cwd=str(cwd),exit=p.returncode,seconds=time.time()-start));write(O/'commands.json',commands)
 print(label,p.returncode,flush=True)
 assert p.returncode==expected,(label,(p.stdout+p.stderr)[-3000:]);verify_candidate();return p.stdout
before=compiled()
entries=json.loads((B/'compile_commands.json').read_text())
def expand(args):
 result=[]
 for a in args:
  if a.startswith('@'):result.extend(expand(shlex.split((B/a[1:]).read_text())))
  else:result.append(a)
 return result
args=expand(shlex.split(next(e['command'] for e in entries if e['file'].endswith('/runtime/external_registration.t.cpp'))))
implargs=expand(shlex.split(next(e['command'] for e in entries if e['file']==str(R/'src/engine/runtime/runtime.cpp'))))
for a in args+implargs:
 if a.startswith('-fmodule-file='):
  path=Path(a[len('-fmodule-file='):].split('=',1)[1]);path=(B/path).resolve()
  before[str(path)]=entry(path)
for p in Path('/opt/homebrew/opt/llvm/lib/c++').glob('*.dylib'):
 before[str(p.resolve())]=entry(p)
write(O/'compiled-before.json',before)
def destination(path):
 path=Path(path).resolve()
 return P/str(path).lstrip('/')
for name,identity in before.items():
 target=destination(name);target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(name,target);assert entry(target)==identity
write(O/'copied-before.json',{str(destination(name)):entry(destination(name)) for name in before})
def redirect(arguments,source,object_name):
 result=list(arguments)
 for i,a in enumerate(result):
  if a.startswith('-fmodule-file='):
   key,path=a[len('-fmodule-file='):].split('=',1);result[i]='-fmodule-file='+key+'='+str(destination(B/path))
  elif a.startswith('-fmodule-output='):result[i]='-fmodule-output='+str(O/(object_name+'.pcm'))
 for flag,suffix in [('-o','.o'),('-MF','.d'),('-MT','.o')]:
  if flag in result:result[result.index(flag)+1]=str(O/(object_name+suffix))
 result[result.index('-c')+1]=str(source)
 result+=['-fno-implicit-modules','-fno-implicit-module-maps']
 return result
source=O/'combined.cpp';source.write_text((R/'src/engine/runtime/external_registration.t.cpp').read_text()+'\n'+(S/'allocator-probes.inc').read_text())
run('compile',redirect(args,source,'probes'),B)
linkline=subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=B,text=True).splitlines()[-1]
link=shlex.split(linkline.split(' && ')[1]);libs=link[link.index('src/engine/libplanar_runtime.a'):]
libs=[str(destination(B/a)) if a.endswith(('.a','.o','.dylib')) else a for a in libs]
libs.append(str(destination(B/'src/engine/libplanar_admission_syscalls.dylib')))
common=[link[0],'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,'+str(destination(B/'src/engine/libplanar_admission_syscalls.dylib').parent)]
run('link',common+[O/'probes.o',*libs,'-o',O/'probes'],B)
run('probes',[O/'probes','[c3-b2-private]','--reporter','junit','--out',O/'probes.xml'])
cases=ET.parse(O/'probes.xml').findall('.//testcase');assert len(cases)==4 and all(not any(c.find(t) is not None for t in ['failure','error','skipped']) for c in cases)
# Reproduce the audited pre-hardening adapter with current interfaces and copied objects.
pre=(R/'src/engine/runtime/runtime.cpp').read_text()
new='''  if (!_owner->first_failure) {
    if (_owner->vm._owner->command.shape_failure)
      _owner->first_failure = std::move(_owner->vm._owner->command.shape_failure);
    else
      _owner->first_failure = std::move(error);
  }'''
assert new in pre
pre=pre.replace(new,'''  if (!_owner->first_failure)
    _owner->first_failure = std::move(error);''')
prepath=O/'pre-hardening-runtime.cpp';prepath.write_text(pre)
run('compile-pre-hardening',redirect(implargs,prepath,'pre-hardening-runtime'),B)
archive=O/'libpre-hardening-runtime.a';copied_runtime=destination(B/'src/engine/libplanar_runtime.a')
interface=destination(B/'src/engine/CMakeFiles/planar_runtime.dir/runtime/runtime.cppm.o')
run('archive-pre-hardening',['/opt/homebrew/opt/llvm/bin/llvm-ar','qc',archive,interface,O/'pre-hardening-runtime.o'],B)
oldlibs=[str(archive) if a==str(copied_runtime) else a for a in libs]
run('link-pre-hardening',common+[O/'probes.o',*oldlibs,'-o',O/'pre-hardening-probes'],B)
run('red-pre-hardening',[O/'pre-hardening-probes','C3 B2 private published preparation error*','--reporter','junit','--out',O/'pre-hardening.xml'],expected=42)
red=ET.parse(O/'pre-hardening.xml');assert red.findall('.//failure')
assert any('field' in f.get('message','') for f in red.findall('.//failure'))
assert compiled()=={k:v for k,v in before.items() if Path(k).is_relative_to(B)}
assert all(entry(Path(k))==v and entry(destination(k))==v for k,v in before.items())
write(O/'compiled-final.json',dict(entries=len(before),shared_and_copied_unchanged=True,all_responses_expanded=True,implicit_modules_disabled=True,cases=4,pre_hardening_red=True,final_product_probes_green=True))
# Disposable compiled copies are removed only after every comparison; manifests/sources/logs remain.
shutil.rmtree(P)
for p in O.iterdir():
 if p.suffix in {'.o','.a','.pcm'} or p.name in {'probes','pre-hardening-probes'}:p.unlink()
print('PRIVATE PROBES COMPLETE',flush=True)
