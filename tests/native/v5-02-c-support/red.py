"""Preimplementation regression: a valid name must use the explicit environment."""
from pathlib import Path
import os,subprocess,tempfile
root=Path(__file__).resolve().parents[3]
with tempfile.TemporaryDirectory(prefix='planar-c-red-') as temporary:
 t=Path(temporary);(t/'named/demo').mkdir(parents=True)
 # Missing selected workspace must report io, but invalid selectors are usage.
 env=dict(os.environ,PLANAR_WORKSPACE_DIR=str(t/'named'))
 args=[str(root/'build/rewrite-debug/bin/planar'),'render','--workspace','bad.name','--registry','missing.json','--root','source=missing','--job','combined','--role','builder']
 p=subprocess.run(args,env=env,capture_output=True,text=True,cwd=t)
 print('status',p.returncode,'stderr',p.stderr,flush=True)
 assert p.returncode==2, 'invalid bare selector must be usage before filesystem access'
