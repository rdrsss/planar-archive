"""Bind exact combined original B3-B plus R01 candidate after the frozen focused gate."""
from pathlib import Path
import hashlib,json,difflib,os,xml.etree.ElementTree as ET
root=Path(__file__).resolve().parents[3];out=root/'tests/native/v5-02-x05-b3-b-r01-evidence'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def meta(p):return dict(sha256=sha(p),bytes=p.stat().st_size)
def dump(name,data):(out/name).write_text(json.dumps(data,indent=2)+'\n')
frozen=json.loads((out/'frozen-source.json').read_text())
for p,h in frozen.items():assert sha(root/p)==h,p
owned=['src/engine/runtime/runtime.cpp','src/engine/runtime/external_registration.t.cpp','CLAUDE.md','docs/architecture.md','docs/testing.md','docs/vernacular/implementation.md','docs/vernacular/roadmap.md','docs/vernacular/external-profile-implementation.md']
baseline=json.loads((out/'baseline.json').read_text())
for p,h in baseline.items():
 if p not in owned:assert sha(root/p)==h,p
assert os.readlink(root/'AGENTS.md')=='CLAUDE.md'
original=root/'tests/native/v5-02-x05-b3-b-evidence'
paths=sorted(set(owned+['src/engine/runtime/runtime.cppm']+[str(p.relative_to(root)) for d in ['v5-02-x05-b3-b-support','v5-02-x05-b3-b-r01-support'] for p in (root/'tests/native'/d).glob('*') if p.is_file()]))
dump('candidate-manifest.json',{p:meta(root/p) for p in paths})
for label,oldroot in [('candidate',original/'baseline'),('r01',out/'baseline')]:
 diff=''
 for p in paths:
  prior=oldroot/p
  if label=='r01' and not prior.exists() and 'r01-support' not in p:continue
  diff+=''.join(difflib.unified_diff(prior.read_text().splitlines(keepends=True) if prior.exists() else [],(root/p).read_text().splitlines(keepends=True),fromfile='a/'+p,tofile='b/'+p))
 (out/(label+'.diff')).write_text(diff)
checks={}
for p,count in [('final-gate/focused.xml',376),('final-gate/history-codec.xml',11),('final-gate/production-semantics.xml',11),('final-allocation/allocation.xml',1),('final-request423/checkpoint.xml',1),('final-request423/green.xml',1)]+[(f'final-probes/{mode}.xml',1)for mode in ['success','semantic','execution','parser','conversion','instructions','snapshot','lua','nodes','native','error','error-node','error-node-exact','error-map','error-string','wrapper']]:
 cases=ET.parse(out/p).findall('.//testcase');assert len(cases)==count,p
 assert all(not any(c.find(t)is not None for t in ['failure','error','skipped'])for c in cases),p
 checks[p]=dict(cases=count,failures=0,errors=0,skips=0)
current={t['name'] for t in json.loads((out/'final-gate/inventory.json').read_text())['tests']}
prior={t['name']for t in json.loads((out/'inventory-before.json').read_text())['tests']}
assert len(current)==642 and len(prior)==641 and prior<=current
assert 'ninja: no work to do.' in (out/'final-gate/no-work.log').read_text()
dump('checks.json',dict(checks=checks,inventory=642,prior_preserved=641,full_milestone=False,independent_review='pending',codec_and_historical_hashes_unchanged=True))
for p,h in frozen.items():assert sha(root/p)==h,p
excluded={'artifact-manifest.json','finalization.json'}
dump('artifact-manifest.json',{str(p.relative_to(out)):meta(p)for p in sorted(out.rglob('*'))if p.is_file() and str(p.relative_to(out)) not in excluded})
dump('finalization.json',dict(status='candidate awaiting independent combined original B3-B plus R01 review',debug_ownership_released=True,no_further_edits=True,source_entries=len(frozen),source_rehash_equal=True,candidate_manifest_sha256=sha(out/'candidate-manifest.json'),candidate_diff_sha256=sha(out/'candidate.diff'),r01_diff_sha256=sha(out/'r01.diff'),artifact_manifest_sha256=sha(out/'artifact-manifest.json'),report_sha256=sha(out/'REPORT.md')))
print((out/'finalization.json').read_text())
