"""Serialized source-frozen focused B1 verification; unique attempt directories."""
from pathlib import Path
import datetime,hashlib,json,shutil,subprocess,sys,xml.etree.ElementTree as ET
from protect import ROOT,OUT,filesystem,sha
attempt=OUT/sys.argv[1];attempt.mkdir(exist_ok=False)
frozen=filesystem()
def write(p,v):p.write_text(json.dumps(v,indent=2,sort_keys=True)+'\n')
write(attempt/'source-freeze.json',frozen)
for name,row in frozen.items():
 if name.startswith(('src/','cmake/','scripts/','tests/native/v5-02-x04-b1-support/')) or name in ['CMakeLists.txt','CMakePresets.json','Makefile']:
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
prior=json.loads((ROOT/'tests/native/v5-02-x04-a3-evidence/attempt01/debug/inventory.log').read_text())
prior=[t['name'] for t in prior['tests']]
assert len(prior)==489 and set(prior)<=set(names) and len(names)==len(set(names))
write(attempt/'inventory-preservation.json',dict(prior=489,current=len(names),missing=sorted(set(prior)-set(names)),added=sorted(set(names)-set(prior))))
previous=json.loads((OUT/'attempt01/source-freeze.json').read_text())
changed=sorted(k for k in previous.keys()|frozen.keys() if previous.get(k)!=frozen.get(k))
assert all(k.endswith('.md') or k.startswith('tests/native/v5-02-x04-b1-support/') for k in changed),changed
write(attempt/'post-test-stability.json',dict(product_and_tests_unchanged=True,changed=changed,passing_attempt='attempt01'))
variants={}
for manifest in (build/'src/engine').glob('*-generated/generation.json'):
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
 variants[manifest.parent.name]=data
assert len(variants)==5
write(attempt/'five-storage-export-variants.json',variants)
run('lint',[sys.executable,'scripts/rewrite-check.py','lint','--build-dir',build])
assert 'ninja: no work to do.' in run('no-work',['cmake','--build',build,'--target','all','planar_tests'])
write(attempt/'stability.json',dict(unchanged=True,freeze_sha256=sha(attempt/'source-freeze.json')))
write(attempt/'manifest.json',{str(p.relative_to(attempt)):sha(p) for p in attempt.rglob('*') if p.is_file() and p.name!='manifest.json'})
