"""Bind an incomplete WIP checkpoint; no approval or full gate is claimed."""
from pathlib import Path
import hashlib,json,os,difflib,xml.etree.ElementTree as ET
root=Path(__file__).resolve().parents[3];out=root/'tests/native/v5-02-x05-b3-b-evidence'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def meta(p):return dict(sha256=sha(p),bytes=p.stat().st_size,mode=p.stat().st_mode&0o777)
def dump(name,data):(out/name).write_text(json.dumps(data,indent=2)+'\n')
def snapshot():
 paths=[]
 for directory in ['src','cmake','scripts','fixtures','tests']:
  for p in (root/directory).rglob('*'):
   if any('evidence' in part or part in {'__pycache__','build'} for part in p.relative_to(root).parts):continue
   if p.is_file() or p.is_symlink():paths.append(p)
 paths += [root/f for f in ['AGENTS.md','CLAUDE.md','CMakeLists.txt','CMakePresets.json','Makefile','.clang-format','docs/architecture.md','docs/testing.md','docs/vernacular/implementation.md','docs/vernacular/roadmap.md','docs/vernacular/external-profile-implementation.md']]
 return {str(p.relative_to(root)):({'symlink':os.readlink(p)} if p.is_symlink() else meta(p)) for p in sorted(set(paths))}
freeze=snapshot();dump('source-snapshot.json',freeze)
baseline=json.loads((out/'baseline.json').read_text());assert all(sha(out/'baseline'/f)==h for f,h in baseline.items())
paths=sorted(list(baseline)+[str(p.relative_to(root)) for p in (root/'tests/native/v5-02-x05-b3-b-support').glob('*') if p.is_file()]+['tests/native/v5-02-x05-b3-b-evidence/REPORT.md'])
dump('candidate-manifest.json',{f:meta(root/f) for f in paths})
diff=''
for f in paths:
 prior=out/'baseline'/f
 diff+=''.join(difflib.unified_diff(prior.read_text().splitlines(keepends=True) if prior.exists() else [],(root/f).read_text().splitlines(keepends=True),fromfile='a/'+f,tofile='b/'+f))
(out/'candidate.diff').write_text(diff)
checks={}
for f,count in [('checkpoint-production.xml',10),('checkpoint-allocation/allocation.xml',1)]:
 cases=ET.parse(out/f).findall('.//testcase');assert len(cases)==count
 assert all(not any(c.find(tag)is not None for tag in ['failure','error','skipped']) for c in cases)
 checks[f]=dict(cases=count,failures=0,errors=0,skips=0)
current=json.loads((out/'checkpoint-inventory.json').read_text())['tests'];prior=json.loads((root/'tests/native/v5-02-x05-b3-a-evidence/final-gate/inventory.log').read_text())['tests']
assert len(current)==637 and len(prior)==627
assert {t['name'] for t in prior}<={t['name'] for t in current}
assert 'ninja: no work to do.' in (out/'checkpoint-no-work.log').read_text()
assert os.readlink(root/'AGENTS.md')=='CLAUDE.md'
dump('checkpoint-checks.json',dict(checks=checks,inventory=637,prior_names_preserved=627,full_gate=False,approval=False,decoder_accounting='pending B3-B-D01',remaining_proofs=['complete semantic registry/live-GC stability','partial setup/resource lifetime probes','frozen focused gate and independent review']))
excluded={'artifact-manifest.json','finalization.json'}
dump('artifact-manifest.json',{str(p.relative_to(out)):meta(p) for p in sorted(out.rglob('*')) if p.is_file() and str(p.relative_to(out)) not in excluded})
assert snapshot()==freeze
dump('finalization.json',dict(status='incomplete prerequisite checkpoint',debug_ownership_released=True,no_further_edits=True,source_snapshot_entries=len(freeze),source_rehash_equal=True,candidate_manifest_sha256=sha(out/'candidate-manifest.json'),candidate_diff_sha256=sha(out/'candidate.diff'),artifact_manifest_sha256=sha(out/'artifact-manifest.json'),report_sha256=sha(out/'REPORT.md')))
print((out/'finalization.json').read_text())
