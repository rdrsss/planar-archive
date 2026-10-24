"""Compile an independent allocation observer against the unchanged production codec."""
from pathlib import Path
import json,shlex,subprocess,sys,time
root=Path(__file__).resolve().parents[3];build=root/'build/rewrite-debug'
out=root/'tests/native/v5-02-x05-b2-b-evidence'/sys.argv[1];out.mkdir(exist_ok=False)
commands=[]
def run(label,args):
 start=time.time()
 with (out/(label+'.log')).open('w') as stream:p=subprocess.run([str(x) for x in args],cwd=build,stdout=stream,stderr=subprocess.STDOUT)
 commands.append({'label':label,'argv':[str(x) for x in args],'exit':p.returncode,'seconds':round(time.time()-start,3)})
 (out/'commands.json').write_text(json.dumps(commands,indent=2)+'\n');print(label,p.returncode,flush=True);assert p.returncode==0
cc=json.loads((build/'compile_commands.json').read_text())
cmd=shlex.split(next(r['command'] for r in cc if r['file'].endswith('json_codec/codec.t.cpp')))
cmd[cmd.index('-o')+1]=str(out/'phase.o');cmd[-1]=str(root/'tests/native/v5-02-x05-b2-b-support/phase.cpp')
run('compile',cmd)
cmd=json.loads((root/'tests/native/v5-02-x05-b2-a-review-evidence/probe-commands.json').read_text())[1]['argv']
cmd=[x.replace(str(root/'tests/native/v5-02-x05-b2-a-review-evidence/adversarial'),str(out/'phase')) for x in cmd]
run('link',cmd)
run('phase',[out/'phase','--reporter','junit','--out',out/'phase.xml'])
