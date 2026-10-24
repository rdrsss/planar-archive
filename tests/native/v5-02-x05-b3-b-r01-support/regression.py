"""Production allocation sampling; isolated outputs and shared membership/hash verification."""
from pathlib import Path
import hashlib,json,shlex,subprocess,sys,time,os
root=Path(__file__).resolve().parents[3];build=root/'build/rewrite-debug';out=root/'tests/native/v5-02-x05-b3-b-r01-evidence'/sys.argv[1];out.mkdir()
entries=json.loads((build/'compile_commands.json').read_text());commands=[]
def run(label,args,mode=None,expected=0):
 env=dict(os.environ,B3B_MODE=mode or 'success');start=time.time()
 p=subprocess.run(list(map(str,args)),cwd=build,capture_output=True,text=True,env=env)
 (out/(label+'.log')).write_text(p.stdout+p.stderr)
 commands.append(dict(label=label,argv=list(map(str,args)),mode=mode,exit=p.returncode,seconds=time.time()-start));(out/'commands.json').write_text(json.dumps(commands,indent=2)+'\n');print(label,p.returncode,flush=True)
 assert p.returncode==expected,(label,(p.stdout+p.stderr)[-2500:])
def snapshot():
 return {str(p.relative_to(build)):hashlib.sha256(p.read_bytes()).hexdigest() for p in (build/'src').rglob('*') if p.is_file() and p.suffix in {'.pcm','.o','.a'}}
def compile(label,suffix,path):
 cmd=shlex.split(next(e['command'] for e in entries if e['file'].endswith(suffix)))
 def expand(args):
  result=[]
  for arg in args:
   result.extend(expand(shlex.split((build/arg[1:]).read_text()))) if arg.startswith('@') else result.append(arg)
  return result
 cmd=expand(cmd);cmd[cmd.index('-c')+1]=str(path);cmd[cmd.index('-o')+1]=str(out/(label+'.o'))
 cmd=[('-fmodule-output='+str(out/(label+'.pcm'))) if a.startswith('-fmodule-output=') else a for a in cmd]
 # Object dependency output is private too.
 if '-MF' in cmd:cmd[cmd.index('-MF')+1]=str(out/(label+'.d'))
 assert all(not a.startswith('@') for a in cmd)
 before=snapshot();run(label+'-compile',cmd);after=snapshot();assert before==after
 (out/(label+'-shared-stability.json')).write_text(json.dumps(dict(entries=len(before),membership_and_hashes_unchanged=True),indent=2)+'\n');return out/(label+'.o')
link=shlex.split(subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=build,text=True).splitlines()[-1].split(' && ')[1]);libs=link[link.index('src/engine/libplanar_runtime.a'):]
common=[link[0],'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++']

base=(root/'tests/native/v5-02-x05-b3-b-r01-support/allocation.cpp').read_text()
assert base.count('  for (auto fault : samples) {')==1
text=base.replace('  for (auto fault : samples) {','  samples={423};\n  for (auto fault : samples) {')
(out/'single.cpp').write_text(text)
main=compile('single','/runtime/external_registration.t.cpp',out/'single.cpp')
runtime=compile('baseline-runtime','/runtime/runtime.cpp',root/'tests/native/v5-02-x05-b3-b-r01-evidence/baseline/src/engine/runtime/runtime.cpp')
run('checkpoint-link',common+[main,runtime,*libs,'-o',out/'checkpoint'])
run('checkpoint',[out/'checkpoint','[r01-allocation]','--reporter','junit','--out',out/'checkpoint.xml'],expected=0)
run('green-link',common+[main,*libs,'-o',out/'green'])
run('green',[out/'green','[r01-allocation]','--reporter','junit','--out',out/'green.xml'])
