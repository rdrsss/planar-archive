"""Freeze exact D7-A candidate and audit the explicitly historical protection basis."""
from pathlib import Path
import difflib,hashlib,json,os,xml.etree.ElementTree as ET
R=Path(__file__).resolve().parents[3];E=R/'tests/native/r14-d7-a-evidence'
sha=lambda b:hashlib.sha256(b).hexdigest()
baseline=json.loads((E/'baseline.json').read_text())
owned=[row['path'] for row in baseline]
owned += [p.relative_to(R).as_posix() for p in sorted((R/'tests/native/r14-d7-a-support').rglob('*')) if p.is_file()]
files={};diff=[]
for p in sorted(owned):
 before=E/'baseline'/p;old=before.read_bytes() if before.exists() else b''
 if before.exists():assert sha(old)==next(row['sha256'] for row in baseline if row['path']==p)
 data=(R/p).read_bytes();files[p]=sha(data)
 target=E/'candidate'/p;target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data)
 diff.extend(difflib.unified_diff(old.decode().splitlines(True),data.decode().splitlines(True),fromfile='baseline/'+p,tofile='candidate/'+p))
(E/'candidate.json').write_text(json.dumps({'path_base':'repository root; snapshots under tests/native/r14-d7-a-evidence/candidate','files':files},indent=2)+'\n')
(E/'owned.diff').write_text(''.join(diff))
C=R/'tests/native/r14-d6-c01-evidence';D=R/'tests/native/r14-d6-evidence'
# Verify all retained frozen evidence, without expecting superseded current product files to remain unchanged.
inherited={}
for name,directory in [('d6',D),('c01',C)]:
 candidate=json.loads((directory/'candidate.json').read_text())['files']
 artifacts=json.loads((directory/'artifacts.json').read_text())['files']
 for p,h in candidate.items():assert sha((directory/'candidate'/p).read_bytes())==h,(name,p)
 for p,h in artifacts.items():assert sha((directory/p).read_bytes())==h,(name,p)
 inherited[name]={'frozen_candidate_files':len(candidate),'artifacts':len(artifacts)}
 for p,h in json.loads((directory/'handoff-identities.json').read_text()).items():assert sha((directory/p).read_bytes())==h,(name,p)
# This protection list predates D7-A; do not describe it as a contemporaneous starting inventory.
historical=json.loads((C/'baseline.json').read_text())['files']
historical.update(json.loads((C/'candidate.json').read_text())['files'])
coordinator={'CLAUDE.md','docs/vernacular/implementation.md','docs/vernacular/roadmap.md',
 'docs/vernacular/v5-alignment.md','docs/vernacular/acceptance-matrix.md','docs/vernacular/load-command-contract.md',
 'docs/vernacular/rounds/R14-D6-C01.md','docs/vernacular/rounds/R14-D6-C01-review.md',
 'docs/vernacular/rounds/R14-D7.md','docs/vernacular/rounds/R14-D7-A.md'}
changed=[];exceptions=[]
for p,h in historical.items():
 if p in owned:continue
 if not (R/p).is_file() or sha((R/p).read_bytes())!=h:
  (exceptions if p in coordinator else changed).append(p)
assert not changed,changed
assert (R/'AGENTS.md').is_symlink() and os.readlink(R/'AGENTS.md')=='CLAUDE.md'
contract=sha((R/'docs/vernacular/load-command-contract.md').read_bytes().split(b'## Fixed catalog',1)[1])
assert contract==(C/'contract-body.sha256').read_text().strip()
before={t['name'] for t in json.loads((C/'final-inventory.log').read_text())['tests']}
after={t['name'] for t in json.loads((E/'inventory.log').read_text())['tests']}
assert len(before)==391 and len(after)==396 and before<=after
cases=ET.parse(E/'focused-final.xml').findall('.//testcase')
assert len(cases)==188
assert not any(c.find('failure') is not None or c.find('skipped') is not None or c.get('status') in {'notrun','disabled'} for c in cases)
assert len([c for c in cases if c.get('name','').endswith('cli')])==6
assert 'ninja: no work to do.' in (E/'no-work.log').read_text()
checks=json.loads((E/'commands.json').read_text())
assert [(c['name'],c['exit']) for c in checks if c['exit']]==[('build',1)]
serialized=[c for c in checks if c['argv'][0] in {'cmake','ctest'}]
for a,b in zip(serialized,serialized[1:]):assert a['end']<=b['start']
report={'protection_basis':'historical C01 baseline plus approved frozen candidate; no full-tree contemporaneous D7-A baseline exists',
 'contemporaneous_owned_snapshots':len(baseline),'historical_protected_paths':len(historical),'changed_outside_owned_or_coordinator':changed,
 'coordinator_exceptions_changed':exceptions,'new_paths_audit':'not claimed: no contemporaneous full-tree D7-A inventory',
 'inherited_evidence':inherited,'contract_body_sha256':contract,'symlink_preserved':True,
 'baseline_names':len(before),'current_names':len(after),'preserved_names':True,'new_names':sorted(after-before),
 'focused_cases':len(cases),'cli_suites':6,'failures':0,'skips':0,'build_test_serialized':True,
 'limitations':['macOS ARM64 debug only','no deterministic C++ allocator-failure injection','no full R14/debug-release milestone','public provider/CLI/transport remain D7-B']}
(E/'audit.json').write_text(json.dumps(report,indent=2)+'\n')
artifacts={p.relative_to(E).as_posix():sha(p.read_bytes()) for p in sorted(E.rglob('*')) if p.is_file() and p.name not in {'artifacts.json','handoff-identities.json'}}
(E/'artifacts.json').write_text(json.dumps({'path_base':'tests/native/r14-d7-a-evidence','files':artifacts},indent=2)+'\n')
identities={p:sha((E/p).read_bytes()) for p in ['candidate.json','owned.diff','artifacts.json']}
(E/'handoff-identities.json').write_text(json.dumps(identities,indent=2)+'\n');print(json.dumps(identities,indent=2))
