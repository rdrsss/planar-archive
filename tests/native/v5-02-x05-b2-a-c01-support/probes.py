"""Compile unchanged production-API allocation probes, preserving each run."""
from pathlib import Path
import json,shlex,subprocess,sys,time
root=Path(__file__).resolve().parents[3]; build=root/'build/rewrite-debug'
out=root/'tests/native/v5-02-x05-b2-a-c01-evidence'/sys.argv[1];out.mkdir(exist_ok=False)
commands=[]
def run(label,args,cwd):
 start=time.time()
 with (out/(label+'.log')).open('w') as stream: p=subprocess.run([str(x) for x in args],cwd=cwd,stdout=stream,stderr=subprocess.STDOUT)
 commands.append({'label':label,'argv':[str(x) for x in args],'cwd':str(cwd),'exit':p.returncode,'seconds':round(time.time()-start,3)})
 (out/'commands.json').write_text(json.dumps(commands,indent=2)+'\n');print(label,p.returncode,flush=True);return p.returncode
cc=json.loads((build/'compile_commands.json').read_text())
for name in sys.argv[2:]:
 source=root/'tests/native/v5-02-x05-b2-a-review-evidence'/(name+'.cpp') if name in ['adapter','adversarial'] else root/'tests/native/v5-02-x05-b2-a-c01-support'/(name+'.cpp')
 cmd=shlex.split(next(r['command'] for r in cc if r['file'].endswith('external_registration.t.cpp')));cmd[cmd.index('-o')+1]=str(out/(name+'.o'));cmd[-1]=str(source)
 assert run(name+'-compile',cmd,build)==0
 old=json.loads((root/'tests/native/v5-02-x05-b2-a-review-evidence/probe-commands.json').read_text())[1]['argv']
 old=[x.replace(str(root/'tests/native/v5-02-x05-b2-a-review-evidence/adversarial'),str(out/name)) for x in old]
 assert run(name+'-link',old,build)==0
 run(name,[out/name,'--reporter','junit','--out',out/(name+'.xml')],root)
