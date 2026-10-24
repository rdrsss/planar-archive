"""Direct integrated gate retaining raw JUnit and LastTest before any later inventory."""
from pathlib import Path
import json,shutil,subprocess,sys,xml.etree.ElementTree as ET
root=Path(__file__).resolve().parents[3];build=Path(sys.argv[1]).resolve();output=Path(sys.argv[2]);output.mkdir(parents=True,exist_ok=True)
def run(args,**kwargs):
 print('+',' '.join(map(str,args)),flush=True)
 return subprocess.run(list(map(str,args)),cwd=root,check=True,**kwargs)
run(['cmake','--build',build,'--target','all','planar_tests'])
listing=run(['ctest','--test-dir',build,'--show-only=json-v1'],capture_output=True,text=True).stdout
(output/'inventory.json').write_text(listing)
names={t['name'] for t in json.loads(listing)['tests']}
baseline={t['name'] for t in json.loads((root/'tests/native/v5-02-b2-evidence/baseline-inventory.json').read_text())['tests']}
assert len(baseline)==358 and baseline<=names,(len(baseline),baseline-names)
assert len(names)>0
try:
 run(['ctest','--test-dir',build,'--output-on-failure','--output-junit',(output/'tests.xml').resolve()])
finally:
 shutil.copyfile(build/'Testing/Temporary/LastTest.log',output/'LastTest.log')
cases=ET.parse(output/'tests.xml').findall('.//testcase')
assert len(cases)==len(names) and all(c.find('skipped') is None and c.find('failure') is None and c.get('status') not in ['notrun','disabled'] for c in cases)
run([sys.executable,'scripts/rewrite-check.py','lint','--build-dir',build])
result=run(['cmake','--build',build,'--target','all','planar_tests'],capture_output=True,text=True)
(output/'no-work.log').write_text(result.stdout+result.stderr)
assert 'ninja: no work to do.' in result.stdout,result.stdout
summary={'tests':len(names),'skips':0,'preserved_baseline_names':len(baseline),'added_names':sorted(names-baseline),'zero_work':True}
(output/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
print(json.dumps(summary),flush=True)
