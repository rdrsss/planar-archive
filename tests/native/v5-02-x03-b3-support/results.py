"""Summarize retained full gates and exact preservation with no suite rerun."""
from pathlib import Path
import hashlib,json,re,sys
ROOT=Path(__file__).resolve().parents[3];OUT=ROOT/'tests/native/v5-02-x03-b3-evidence'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text())
faults={}
configurations={}
for preset in ('debug','release'):
 cache=(ROOT/'build'/('rewrite-'+preset)/'CMakeCache.txt').read_text()
 expected='Debug' if preset=='debug' else 'RelWithDebInfo'
 assert 'CMAKE_BUILD_TYPE:STRING='+expected in cache
 configurations[preset]=expected
 raw=(OUT/(preset+'-LastTest.log')).read_text();formats={}
 for fmt in (1,2):
  data={}
  for mode,count in [('true',38),('false',15)]:
   rows=[(int(a),int(b),c=='true') for a,b,c in re.findall('D8 SQLITE_FULL format='+str(fmt)+' journal='+mode+r' ordinal=(\d+) hits=(\d+) success=(true|false)',raw)]
   assert len(rows)==count+1 and [r[0] for r in rows]==list(range(count+1))
   assert all(h>0 and not ok for _,h,ok in rows[:-1]) and rows[-1]==(count,0,True)
   data['journal' if mode=='true' else 'database']={'faults':count,'rows':rows}
  boundaries=sorted(set(map(int,re.findall('D8 process format='+str(fmt)+r' boundary=(\d+)',raw))))
  assert boundaries==list(range(5));data['unique_process_boundaries']=boundaries
  formats[str(fmt)]=data
 faults[preset]=formats
prior=read(ROOT/'tests/native/v5-02-x03-a-evidence/baseline-manifest.json')['sha256']
protected=[n for n in prior if n.startswith('fixtures/') or n.startswith('src/stdlib/') or n in ('src/engine/history/record.cpp','src/engine/history/record.cppm','tests/vectors/records/records.tsv')]
assert protected and all(sha(ROOT/n)==prior[n] for n in protected)
old=read(ROOT/'tests/native/v5-02-x01-review-evidence/inventory.log');current=read(OUT/'debug-inventory.log')
old_names={r['name'] for r in old['tests']};names={r['name'] for r in current['tests']}
assert len(old_names)==405 and old_names<=names
approved={}
for label in ('b2-b','b2-b-c01'):
 p=ROOT/('tests/native/v5-02-x03-'+label+'-evidence/candidate-manifest.json');d=read(p)
 d={r['path']:r['sha256'] for r in d['files']} if isinstance(d.get('files'),list) else d
 paths={n:h for n,h in d.items() if n.startswith(('src/','tests/'))}
 for n,h in paths.items():
  if label=='b2-b' and n in ('src/engine/ledger/format.cpp','src/engine/CMakeLists.txt'):continue
  assert sha(ROOT/n)==h,n
 approved[label]={'manifest_sha256':sha(p),'source_members':paths}
report=dict(configurations=configurations,faults=faults,protected_v1_W_fixtures=dict(path_base=str(ROOT),source_manifest='tests/native/v5-02-x03-a-evidence/baseline-manifest.json',files={n:sha(ROOT/n) for n in sorted(protected)}),baseline405=dict(retained=len(old_names),missing=sorted(old_names-names)),approved_candidate_sources=approved)
(OUT/'results-summary.json').write_text(json.dumps(report,indent=2,sort_keys=True)+'\n')
print('Full gate raw fault matrices, 405 preservation, old codec/W/fixtures and approved combined sources verified')
