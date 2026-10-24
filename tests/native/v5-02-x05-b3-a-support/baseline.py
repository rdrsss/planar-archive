"""Compile exact prechange embedded Lua privately; compare real native output bytes."""
from pathlib import Path
import subprocess,json,shlex,hashlib,os,sys
root=Path(__file__).resolve().parents[3]; build=root/'build/rewrite-debug'; out=root/'tests/native/v5-02-x05-b3-a-evidence/baseline-comparison';out.mkdir()
entries=json.loads((build/'compile_commands.json').read_text());commands=[]
def run(label,args,env=None):
 p=subprocess.run([str(x) for x in args],cwd=build,capture_output=True,text=True,env=env)
 (out/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=[str(a) for a in args],exit=p.returncode));(out/'commands.json').write_text(json.dumps(commands,indent=2)+'\n');assert p.returncode==0,label
# Recreate module with exact originals in original order, replacing only owned
# schema/semantic/bootstrap with archived prechange bytes.
import re
module=(build/'src/engine/planar_runtime_source.cppm').read_text()
arrays={name:bytes(int(x,16) for x in re.findall(r'0x([0-9a-f]{2})',body)) for name,body in re.findall(r'constexpr unsigned char (\w+)\[\] = \{([^}]*)\};',module)}
parts=[];substitutions=[]
for logical,name in re.findall(r'\{"([^"]+)", \{reinterpret_cast<const char\*>\((original_\d+)\)',module):
 current=arrays[name]; baseline=root/'tests/native/v5-02-x05-b3-a-evidence/baseline/src/stdlib'/logical
 original=baseline.read_bytes() if baseline.exists() else current;parts.append(original+b'\n')
 old='constexpr unsigned char '+name+'[] = {'+','.join(f'0x{x:02x}' for x in current)+',0};'
 new='constexpr unsigned char '+name+'[] = {'+','.join(f'0x{x:02x}' for x in original)+',0};'
 assert module.count(old)==1;module=module.replace(old,new)
 substitutions.append(dict(path=logical,current=hashlib.sha256(current).hexdigest(),baseline=hashlib.sha256(original).hexdigest()))
module=re.sub(r'constexpr unsigned char source_bytes\[\] = \{[^}]*\};','constexpr unsigned char source_bytes[] = {'+','.join(f'0x{x:02x}' for x in b''.join(parts))+',0};',module)
(out/'baseline-source.cppm').write_text(module);(out/'source-identities.json').write_text(json.dumps(substitutions,indent=2)+'\n')
def compile(label,suffix,path):
 cmd=shlex.split(next(e['command'] for e in entries if e['file'].endswith(suffix)))
 # CMake puts the module output inside a response file. Expand before redirecting
 # so private compiler outputs can never replace shared build graph artifacts.
 def expand(args):
  result=[]
  for arg in args:
   result.extend(expand(shlex.split((build/arg[1:]).read_text()))) if arg.startswith('@') else result.append(arg)
  return result
 cmd=expand(cmd)
 cmd[cmd.index('-c')+1]=str(path);cmd[cmd.index('-o')+1]=str(out/(label+'.o'))
 cmd=[('-fmodule-output='+str(out/'baseline-source.pcm')) if a.startswith('-fmodule-output=') else a for a in cmd]
 if suffix.endswith('.cppm'):
  assert any(a.startswith('-fmodule-output=') for a in cmd),cmd
  assert all(not a.startswith('@') for a in cmd)
  assert next(a for a in cmd if a.startswith('-fmodule-output='))=='-fmodule-output='+str(out/'baseline-source.pcm')
 # Verify all shared object/module/library bytes before and after each compile.
 shared={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in (build/'src').rglob('*') if p.is_file() and p.suffix in {'.pcm','.o','.a'}}
 run(label,cmd)
 assert all(Path(name).is_file() and hashlib.sha256(Path(name).read_bytes()).hexdigest()==digest for name,digest in shared.items())
 (out/(label+'-shared-stability.json')).write_text(json.dumps(dict(entries=len(shared),unchanged=True),indent=2)+'\n')
 return out/(label+'.o')
source=compile('source','/planar_runtime_source.cppm',out/'baseline-source.cppm')
main=compile('main','/runtime/shapes.t.cpp',root/'tests/native/v5-02-x05-b3-a-support/baseline.cpp')
link=shlex.split(subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=build,text=True).splitlines()[-1].split(' && ')[1]);libs=link[link.index('src/engine/libplanar_runtime.a'):]
common=[link[0],'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++',main]
for label in ['current','baseline']:
 selected=[a for a in libs if label=='current' or a!='src/engine/libplanar_runtime_source.a']
 run(label+'-link',common+([source] if label=='baseline' else [])+selected+['-o',out/label])
 run(label,[out/label,'[baseline-compare]'],dict(os.environ,B3_BASELINE_OUTPUT=str(out/(label+'.jsonl'))))
assert (out/'current.jsonl').read_bytes()==(out/'baseline.jsonl').read_bytes()
(out/'comparison.json').write_text(json.dumps(dict(cases=len((out/'current.jsonl').read_text().splitlines()),byte_equal=True,sha256=hashlib.sha256((out/'current.jsonl').read_bytes()).hexdigest()),indent=2)+'\n')
print('Exact prechange/current production embedded semantic outputs match')
