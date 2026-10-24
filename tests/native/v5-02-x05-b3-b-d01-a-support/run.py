"""Private feasibility compile; never writes shared object/PCM outputs."""
from pathlib import Path
import hashlib, json, shlex, subprocess, sys, time
root=Path(__file__).resolve().parents[3]
build=root/'build/rewrite-debug'
out=root/'tests/native/v5-02-x05-b3-b-d01-a-evidence'/sys.argv[1]
out.mkdir()
commands=[]
(out/"prototype.cpp").write_bytes((root/"tests/native/v5-02-x05-b3-b-d01-a-support/prototype.cpp").read_bytes())
def snapshot():
 return {str(p.relative_to(build)):hashlib.sha256(p.read_bytes()).hexdigest() for p in build.rglob('*') if p.is_file() and p.suffix in {'.pcm','.o','.a'}}
def run(label,args):
 start=time.time(); p=subprocess.run(list(map(str,args)),cwd=build,capture_output=True,text=True)
 (out/(label+'.log')).write_text(p.stdout+p.stderr)
 commands.append(dict(label=label,argv=list(map(str,args)),cwd=str(build),exit=p.returncode,seconds=time.time()-start))
 (out/'commands.json').write_text(json.dumps(commands,indent=2)+'\n')
 print(label,p.returncode,flush=True)
 if p.returncode: raise SystemExit(p.returncode)
before=snapshot(); (out/'shared-before.json').write_text(json.dumps(before,indent=2)+'\n')
entries=json.loads((build/'compile_commands.json').read_text())
cmd=shlex.split(next(e['command'] for e in entries if e['file'].endswith('json_codec/codec.t.cpp')))
def expand(args):
 result=[]
 for a in args:
  result.extend(expand(shlex.split((build/a[1:]).read_text()))) if a.startswith('@') else result.append(a)
 return result
cmd=expand(cmd)
cmd[cmd.index('-c')+1]=str(root/'tests/native/v5-02-x05-b3-b-d01-a-support/prototype.cpp')
cmd[cmd.index('-o')+1]=str(out/'prototype.o')
cmd=[('-fmodule-output='+str(out/'prototype.pcm')) if a.startswith('-fmodule-output=') else a for a in cmd]
if '-MF' in cmd: cmd[cmd.index('-MF')+1]=str(out/'prototype.d')
cmd += ['-isystem',str(root/'vendor/glaze/a4a7/include')]
assert not any(a.startswith('@') for a in cmd)
run('compile',cmd)
run('link',[cmd[0],'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++',out/'prototype.o','-o',out/'prototype','src/engine/libplanar_json_codec.a','vendor/catch2/src/libCatch2Maind.a','src/engine/lib@cmake_cxx_std.a','src/engine/libplanar_value.a','vendor/catch2/src/libCatch2d.a'])
run('prototype',[out/'prototype','--reporter','junit','--out',out/'prototype.xml'])
after=snapshot(); assert before==after
(out/'shared-stability.json').write_text(json.dumps(dict(entries=len(before),membership_and_hashes_unchanged=True),indent=2)+'\n')
