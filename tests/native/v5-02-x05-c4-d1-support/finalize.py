"""Bind the frozen candidate and exact preserved evidence after successful gates."""
from pathlib import Path
import hashlib,json,stat
from freeze import R,S,E,entry,write,candidate
assert candidate()==json.loads((E/'frozen-candidate.json').read_text())
assert not json.loads((E/'final-protection.json').read_text())['changed']
paths=[p for p in E.rglob('*') if p.is_file() and p.name not in {'artifact-manifest.json','finalization.json'}]
artifacts={str(p.relative_to(E)):dict(mode=stat.S_IMODE(p.stat().st_mode),bytes=p.stat().st_size,sha256=hashlib.sha256(p.read_bytes()).hexdigest()) for p in paths}
write(E/'artifact-manifest.json',dict(schema=1,root=str(E.relative_to(R)),entries=artifacts))
write(E/'finalization.json',dict(schema=1,candidate_sha256=hashlib.sha256((E/'frozen-candidate.json').read_bytes()).hexdigest(),artifact_manifest_sha256=hashlib.sha256((E/'artifact-manifest.json').read_bytes()).hexdigest(),report_sha256=hashlib.sha256((E/'REPORT.md').read_bytes()).hexdigest(),artifact_entries=len(artifacts),candidate_entries=len(candidate()),full_gate_claimed=False,release_gate_claimed=False,review='pending',debug_released=True))
print((E/'finalization.json').read_text())
