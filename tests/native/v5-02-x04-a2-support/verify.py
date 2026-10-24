"""Source-frozen serialized X04-A2 debug gate; each invocation creates a unique pass."""
from pathlib import Path
import datetime,hashlib,json,shutil,subprocess,sys,xml.etree.ElementTree as ET
from protect import ROOT,OUT,OWNED,sha
PASS=OUT/'passes'/sys.argv[1]
PASS.mkdir(parents=True,exist_ok=False)
def write(n,v):(PASS/n).write_text(json.dumps(v,indent=2,sort_keys=True)+'\n')
owned=OWNED+[str(p.relative_to(ROOT)) for p in sorted((ROOT/'tests/native/v5-02-x04-a2-support').glob('*.py'))]
freeze={n:sha(ROOT/n) for n in owned if (ROOT/n).exists()}
write('source-freeze.json',freeze)
for n in freeze:
 target=PASS/'candidate-source'/n;target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(ROOT/n,target)
rows=[]
def run(label,args):
 start=datetime.datetime.now(datetime.timezone.utc)
 with (PASS/(label+'.log')).open('w') as stream:
  result=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=stream,stderr=subprocess.STDOUT)
 rows.append(dict(label=label,argv=list(map(str,args)),exit_code=result.returncode,start=start.isoformat(),seconds=(datetime.datetime.now(datetime.timezone.utc)-start).total_seconds()))
 write('commands.json',rows)
 assert {n:sha(ROOT/n) for n in freeze}==freeze,('source changed',label)
 if result.returncode:raise SystemExit((label,result.returncode))
 return (PASS/(label+'.log')).read_text()
BUILD=ROOT/'build/rewrite-debug'
run('debug-build',['cmake','--build',BUILD,'--target','all','planar_tests'])
listing=json.loads(run('debug-inventory',['ctest','--test-dir',BUILD,'--show-only=json-v1']))
old=json.loads((ROOT/'tests/native/v5-02-x04-a1-evidence/debug-inventory.log').read_text())
names={t['name'] for t in listing['tests']};prior={t['name'] for t in old['tests']}
assert len(prior)==451 and prior<=names
write('inventory-preservation.json',dict(previous=len(prior),current=len(names),missing=sorted(prior-names),added=sorted(names-prior)))
run('debug-focused',['ctest','--test-dir',BUILD,'-R','replacement.*(profile|capture|path|acceptance)','--output-on-failure','--output-junit',PASS/'debug-focused.xml'])
cases=ET.parse(PASS/'debug-focused.xml').findall('.//testcase')
assert cases and not any(c.find('skipped') is not None or c.find('failure') is not None or c.find('error') is not None for c in cases)
write('focused-results.json',dict(tests=len(cases),failures=0,errors=0,skips=0,names=[c.get('name') for c in cases]))
for label,script in [('vectors','tests/vectors/generate.py'),('record-vectors','tests/vectors/records/generate.py'),('request-v2-vectors','tests/vectors/records/generate-request-v2.py')]:run(label,[sys.executable,script,'--check'])
for label,script in [('historical','v5-02-b2-support/audit-variants.py'),('d6','r14-d6-support/audit.py'),('d6-c01','r14-d6-c01-support/audit.py'),('d7-b','r14-d7-b-support/audit.py')]:
 run(label+'-audit',[sys.executable,ROOT/'tests/native'/script,BUILD,PASS/(label+'-audit-report.json')])
variants={}
for manifest in (BUILD/'src/engine').glob('*-generated/generation.json'):
 data=json.loads(manifest.read_text());data=data.get('files',data)
 for name,row in data.items():
  original=ROOT/'src/engine'/('export' if name.startswith('source_export') else 'ledger')/name;generated=manifest.parent/name
  assert sha(original)==row['source_sha256'] and sha(generated)==row['generated_sha256'] and row['reverse_identical']
  text=original.read_text()
  for sub in row['substitutions']:
   assert text.count(sub['before'])==sub['count'];text=text.replace(sub['before'],sub['after'])
  assert text==generated.read_text()
  for sub in reversed(row['substitutions']):text=text.replace(sub['after'],sub['before'])
  assert text==original.read_text()
 variants[manifest.parent.name]={n:row['generated_sha256'] for n,row in data.items()}
assert len(variants)==5;write('five-storage-export-variants.json',variants)
run('pinned-lint',[sys.executable,'scripts/rewrite-check.py','lint','--build-dir',BUILD])
text=run('debug-no-work',['cmake','--build',BUILD,'--target','all','planar_tests']);assert 'ninja: no work to do.' in text
write('source-stability.json',dict(unchanged=True,files=freeze))
write('debug-build-identities.json',{str(p.relative_to(ROOT)):sha(p) for p in [BUILD/'bin/planar',BUILD/'src/engine/planar_capture_profile_tests',BUILD/'src/engine/planar_capture_native_tests',BUILD/'src/engine/libplanar_admission_syscalls.dylib',BUILD/'src/engine/CMakeFiles/planar_capture_profile.dir/capture/profile.cpp.o',BUILD/'src/engine/CMakeFiles/planar_capture_native.dir/capture/native.cpp.o']})
write('summary.json',dict(inventory=len(names),preserved=len(prior),focused=len(cases),skips=0,source_unchanged=True))
write('pass-manifest.json',{str(p.relative_to(PASS)):sha(p) for p in sorted(PASS.rglob('*')) if p.is_file() and p.name!='pass-manifest.json'})
print((PASS/'summary.json').read_text())
