"""Bind the exact incremental B3-C candidate and raw verification artifacts."""
from pathlib import Path
import difflib,hashlib,json,xml.etree.ElementTree as ET
root=Path(__file__).resolve().parents[3];out=root/'tests/native/v5-02-x05-b3-c-evidence'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def dump(name,data):(out/name).write_text(json.dumps(data,indent=2)+'\n')
frozen=json.loads((out/'frozen-source.json').read_text())
for name,digest in frozen.items():assert sha(root/name)==digest,name
baseline=json.loads((out/'baseline.json').read_text());diff=[];candidate={}
for name in baseline:
 before=(out/'baseline'/name).read_text();after=(root/name).read_text()
 diff.extend(difflib.unified_diff(before.splitlines(True),after.splitlines(True),fromfile='a/'+name,tofile='b/'+name))
 candidate[name]=dict(sha256=sha(root/name),bytes=(root/name).stat().st_size)
for p in sorted((root/'tests/native/v5-02-x05-b3-c-support').rglob('*')):
 if not p.is_file() or '__pycache__'in p.parts:continue
 name=str(p.relative_to(root));candidate[name]=dict(sha256=sha(p),bytes=p.stat().st_size)
 diff.extend(difflib.unified_diff([],p.read_text().splitlines(True),fromfile='/dev/null',tofile='b/'+name))
(out/'candidate.diff').write_text(''.join(diff));dump('candidate-manifest.json',candidate)
checks={}
for path,count in [('final-gate/focused.xml',389),('final-gate/history-codec.xml',11),('final-gate/production-selection.xml',13),('final-gate/production-semantics.xml',11),('final-allocation/allocation.xml',1)]:
 cases=ET.parse(out/path).findall('.//testcase');assert len(cases)==count,(path,len(cases));assert not any(c.find(x)is not None for c in cases for x in ['failure','error','skipped']);checks[path]=len(cases)
for p in sorted((out/'final-probes').glob('*.xml')):
 cases=ET.parse(p).findall('.//testcase');assert len(cases)==1;assert not any(c.find(x)is not None for c in cases for x in ['failure','error','skipped']);checks[str(p.relative_to(out))]=len(cases)
assert len(list((out/'final-probes').glob('*.xml')))==20
assert (out/'final-gate/frozen-stability.json').is_file()
dump('checks.json',checks)
dump('finalization.json',dict(candidate_diff_sha256=sha(out/'candidate.diff'),frozen_entries=len(frozen),source_unchanged=True,inventory=655,prior_names_preserved=642,focused=389,supplemental=11,private_modes=19,independent_review_required=True,full_milestone_gate=False))
dump('artifact-manifest.json',{str(p.relative_to(out)):dict(sha256=sha(p),bytes=p.stat().st_size)for p in sorted(out.rglob('*'))if p.is_file() and p.name!='artifact-manifest.json'})
print('Candidate, incremental diff, exact source stability and raw gate artifacts bound; independent review required')
