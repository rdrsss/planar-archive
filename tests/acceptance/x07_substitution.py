"""X07 substitution evidence: with-profile init, history and export run the sealed shipped callable, not native code."""
from pathlib import Path
import json,os,subprocess,sys,tempfile
variant,case,planar=sys.argv[1],sys.argv[2],str(Path(sys.argv[3]).resolve())
variant=str(Path(variant).resolve())
# The substituted callables return a distinguishable typed refusal before any native service call, so a native
# dispatch would instead create the ledger, read history or publish the target.
SUBSTITUTED={name:{"version":1,"kind":"error","error":{"code":"schema","message":f"X07 substituted {name.rsplit('_',1)[1]} callable","field":""}} for name in ('x07.substitute_init','x07.substitute_history','x07.substitute_export')}
assert case in SUBSTITUTED,case
expected=(json.dumps(SUBSTITUTED[case],sort_keys=True,separators=(',',':'))+'\n').encode()
with tempfile.TemporaryDirectory(prefix='planar-x07-sub-') as temp:
 root=Path(temp).resolve();work=root/'work';trusted=root/'trusted';work.mkdir();trusted.mkdir()
 env={**os.environ,'HOME':str(root),'PLANAR_DB':str(root/'forbidden-legacy.db')}
 env.pop('PLANAR_WORKSPACE_DIR',None)
 (trusted/'p.json').write_text('{"version":1,"modules":[]}')
 def run(exe,args):
  return subprocess.run([exe,*args],capture_output=True,env=env,cwd=work,timeout=60)
 ledger=work/'real.db'
 made=run(planar,['init','--ledger',str(ledger),'--workspace-id','quota'])
 assert made.returncode==0 and not made.stderr,made
 family=case.rsplit('_',1)[1]
 if family=='init':
  tail=['init','--ledger','native.db','--workspace-id','quota']
 elif family=='history':
  tail=['history','--ledger','real.db']
 else:
  tail=['export','--ledger','real.db','--revision','1','--target','out']
 # The generated variant must itself be valid: its embedded (profile-free) run yields exactly the substituted bytes.
 embedded=run(variant,tail if family!='init' else ['init','--ledger','embedded.db','--workspace-id','quota'])
 assert embedded.returncode==3 and embedded.stderr==expected and not embedded.stdout,embedded
 for leftover in ('embedded.db','out'):
  if (work/leftover).exists():
   if (work/leftover).is_dir():
    import shutil;shutil.rmtree(work/leftover)
   else:(work/leftover).unlink()
 # The same family under with-profile, after identical CLI11 parsing, must give the substituted bytes and no native effect.
 profiled=run(variant,['with-profile','--stdlib-profile','../trusted/p.json',*tail])
 assert profiled.returncode==3,(profiled.returncode,profiled.stdout,profiled.stderr)
 assert profiled.stderr==expected,profiled.stderr
 assert not profiled.stdout,profiled.stdout
 if family=='init':assert not (work/'native.db').exists(),'init ran natively'
 if family=='export':assert not (work/'out').exists(),'export published natively'
 print(f'{case} passed')
