"""Final documentation/support freeze; verify all prior gates still bind unchanged product and fixtures."""
from pathlib import Path
import datetime,json,subprocess,sys
from protect import ROOT,OUT,filesystem,sha
attempt=OUT/'attempt06-final';attempt.mkdir(exist_ok=False)
frozen=filesystem();(attempt/'source-freeze.json').write_text(json.dumps(frozen,indent=2,sort_keys=True)+'\n')
rows=[]
for label,args in [('scope-audit',[sys.executable,'-B','tests/native/v5-02-x04-b3-support/audit.py']),('effective-vectors',[sys.executable,'-B','tests/vectors/effective-provenance/generate.py','--check']),('no-work',['cmake','--build','build/rewrite-debug','--target','all','planar_tests'])]:
 assert filesystem()==frozen,label+' pre-command source changed'
 start=datetime.datetime.now(datetime.timezone.utc).isoformat()
 with (attempt/(label+'.log')).open('w') as f:r=subprocess.run(args,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT)
 rows.append(dict(label=label,argv=args,start=start,end=datetime.datetime.now(datetime.timezone.utc).isoformat(),exit_code=r.returncode));(attempt/'commands.json').write_text(json.dumps(rows,indent=2)+'\n')
 assert r.returncode==0,label
 assert filesystem()==frozen,label+' post-command source changed'
 if label=='no-work':assert 'ninja: no work to do.' in (attempt/(label+'.log')).read_text()
 print(label,r.returncode,flush=True)
(attempt/'stability.json').write_text(json.dumps(dict(unchanged=True,source_freeze_sha256=sha(attempt/'source-freeze.json')),indent=2)+'\n')
