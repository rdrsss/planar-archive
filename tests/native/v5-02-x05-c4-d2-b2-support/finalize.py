"""Bind the frozen candidate and exact preserved evidence after successful gates."""
from pathlib import Path
import hashlib,json,stat,ast
from freeze import R,S,E,entry,write,candidate
# Only this round is exempt from unrelated-document protection. Parse the
# actual helper's exclusion assignment so inherited historical exclusions refuse.
expected_round='docs/vernacular/rounds/V5-02-X05-C4-D2-B2.md'
assignments=[n for n in ast.walk(ast.parse((S/'freeze.py').read_text())) if isinstance(n,ast.Assign) and any(isinstance(t,ast.Name) and t.id=='excluded' for t in n.targets)]
assert len(assignments)==1
exclusions=ast.literal_eval(assignments[0].value.right)
assert exclusions=={expected_round},exclusions
assert 'docs/vernacular/rounds/V5-02-X05-C4-D1.md' not in exclusions
assert 'docs/vernacular/rounds/V5-02-X05-C4-D2-B1.md' not in exclusions
assert json.loads((E/'bookkeeping-identity.json').read_text())['owned_product_test_prose_unchanged']
assert candidate()==json.loads((E/'frozen-candidate.json').read_text())
assert not json.loads((E/'final-protection.json').read_text())['changed']
paths=[p for p in E.rglob('*') if p.is_file() and p.name not in {'artifact-manifest.json','finalization.json'}]
artifacts={str(p.relative_to(E)):dict(mode=stat.S_IMODE(p.stat().st_mode),bytes=p.stat().st_size,sha256=hashlib.sha256(p.read_bytes()).hexdigest()) for p in paths}
write(E/'artifact-manifest.json',dict(schema=1,root=str(E.relative_to(R)),entries=artifacts))
write(E/'finalization.json',dict(schema=1,candidate_sha256=hashlib.sha256((E/'frozen-candidate.json').read_bytes()).hexdigest(),artifact_manifest_sha256=hashlib.sha256((E/'artifact-manifest.json').read_bytes()).hexdigest(),report_sha256=hashlib.sha256((E/'REPORT.md').read_bytes()).hexdigest(),artifact_entries=len(artifacts),candidate_entries=len(candidate()),full_gate_claimed=False,release_gate_claimed=False,review='pending',debug_released=True,coordinator_round_exclusion_checked=expected_round,tested_vs_final_identity='bookkeeping-identity.json',copied_runtime_link_load_selection_proved=False))
print((E/'finalization.json').read_text())
