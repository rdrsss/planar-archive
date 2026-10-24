"""Serialized source-frozen focused B2 verification; unique attempt directories."""
from pathlib import Path
import datetime,hashlib,json,shutil,subprocess,sys,xml.etree.ElementTree as ET
from protect import ROOT,OUT,filesystem,sha
attempt=OUT/sys.argv[1];attempt.mkdir(exist_ok=False)
frozen=filesystem()
def write(p,v):p.write_text(json.dumps(v,indent=2,sort_keys=True)+'\n')
write(attempt/'source-freeze.json',frozen)
for name,row in frozen.items():
 if name.startswith(('src/','cmake/','scripts/','tests/native/v5-02-x04-b2-support/')) or name in ['CMakeLists.txt','CMakePresets.json','Makefile']:
  target=attempt/'candidate-source'/name;target.parent.mkdir(parents=True,exist_ok=True)
  if 'symlink' in row:target.symlink_to(row['symlink'])
  else:shutil.copy2(ROOT/name,target)
rows=[]
def run(label,args):
 assert filesystem()==frozen,'source changed before '+label
 start=datetime.datetime.now(datetime.timezone.utc)
 with (attempt/(label+'.log')).open('w') as f:r=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=f,stderr=subprocess.STDOUT)
 rows.append(dict(label=label,argv=list(map(str,args)),start=start.isoformat(),end=datetime.datetime.now(datetime.timezone.utc).isoformat(),exit_code=r.returncode));write(attempt/'commands.json',rows)
 print(label,r.returncode,flush=True)
 assert filesystem()==frozen,'source changed after '+label
 if r.returncode:raise SystemExit(label+' failed')
 return (attempt/(label+'.log')).read_text()
build=ROOT/'build/rewrite-debug'
run('configure',['cmake','--preset','debug'])
run('build',['cmake','--build',build,'--target','all','planar_tests'])
listing=json.loads(run('inventory',['ctest','--test-dir',build,'--show-only=json-v1']))
names=[t['name'] for t in listing['tests']]
prior=json.loads((ROOT/'tests/native/v5-02-x04-b1-evidence/attempt02-final-docs/inventory.log').read_text())
prior=[t['name'] for t in prior['tests']]
assert len(prior)==497 and set(prior)<=set(names) and len(names)==len(set(names))
write(attempt/'inventory-preservation.json',dict(prior=497,current=len(names),missing=sorted(set(prior)-set(names)),added=sorted(set(names)-set(prior))))
run('focused',['ctest','--test-dir',build,'-R','replacement.*(effective catalog|profile codec|provenance|command|acceptance)','--output-on-failure','--output-junit',attempt/'focused.xml'])
for n in ['LastTest.log','CTestCostData.txt']:shutil.copy2(build/'Testing/Temporary'/n,attempt/n)
cases=ET.parse(attempt/'focused.xml').getroot().findall('.//testcase');assert cases and all(c.get('status')=='run' and all(c.find(t) is None for t in ['failure','error','skipped']) for c in cases)
write(attempt/'focused-summary.json',dict(executed=len(cases),failures=0,errors=0,skips=0,names=[c.get('name') for c in cases]))
for label,script in [('canonical-vectors','tests/vectors/generate.py'),('record-vectors','tests/vectors/records/generate.py'),('request-vectors','tests/vectors/records/generate-request-v2.py')]:run(label,[sys.executable,script,'--check'])
for label,script in [('historical','v5-02-b2-support/audit-variants.py'),('d6','r14-d6-support/audit.py'),('d6-c01','r14-d6-c01-support/audit.py'),('d7-b','r14-d7-b-support/audit.py')]:run(label+'-audit',[sys.executable,ROOT/'tests/native'/script,build,attempt/(label+'-report.json')])
run('lint',[sys.executable,'scripts/rewrite-check.py','lint','--build-dir',build])
assert 'ninja: no work to do.' in run('no-work',['cmake','--build',build,'--target','all','planar_tests'])
write(attempt/'stability.json',dict(unchanged=True,freeze_sha256=sha(attempt/'source-freeze.json')))
write(attempt/'manifest.json',{str(p.relative_to(attempt)):sha(p) for p in attempt.rglob('*') if p.is_file() and p.name!='manifest.json'})
