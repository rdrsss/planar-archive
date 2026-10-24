"""Bind final D4 source, evidence, exact exemption and immutable D3 baseline."""
from pathlib import Path
import hashlib,json,xml.etree.ElementTree as ET
from freeze import R,E,OWNED,ROUND_EXEMPT,D3_ROUND,exemptions,entry,candidate,write
assert ROUND_EXEMPT=='docs/vernacular/rounds/V5-02-X05-C4-D4.md'
assert exemptions()==set(OWNED)|{'docs/vernacular/rounds/V5-02-X05-C4-D4.md'}
assert D3_ROUND not in exemptions()
baseline=json.loads((E/'baseline.json').read_text())
assert D3_ROUND in baseline and baseline[D3_ROUND]==entry(R/D3_ROUND)
assert baseline[D3_ROUND]['sha256']=='5b810e19da5235b44a26080c36fd519ea72623585089c95ed6a959e5ceb0a5eb'
assert candidate()==json.loads((E/'frozen-candidate.json').read_text())
protection=json.loads((E/'final-protection.json').read_text());assert protection['changed']==[] and protection['release_absent']
assert not (R/'build/rewrite-release').exists() and (R/'AGENTS.md').is_symlink()
assert not (E/'dependencies').exists()
cleanup=json.loads((E/'cleanup.json').read_text());assert cleanup['shared_inputs_unchanged'] and cleanup['copied_membership_modes_bytes_verified']
for name,count in [('focused.xml',536),('private/private.xml',10)]:
 rows=ET.parse(E/name).findall('.//testcase');assert len(rows)==count
 assert all(not any(c.find(t)is not None for t in ['failure','error','skipped']) for c in rows)
old=json.loads((E/'development-private-attempt1/frozen-candidate.json').read_text());now=candidate()
changed=sorted(n for n in now if old.get(n)!=now[n]);assert changed==['tests/native/v5-02-x05-c4-d4-support/finalize.py','tests/native/v5-02-x05-c4-d4-support/freeze.py','tests/native/v5-02-x05-c4-d4-support/methods.inc','tests/native/v5-02-x05-c4-d4-support/private-tests.inc']
write(E/'product-stability-after-private-fixture-correction.json',dict(changed=changed,all_product_tests_reference_and_other_support_bytes_unchanged=True,broad_focused_passed=536,final_private_passed=10,assembly_instruction_control='controlled already-exhausted state; no actual instruction-crossing claim',protection_control='exact D4 round only; D3 remained in original baseline and is equal'))
write(E/'exemption-proof.json',dict(current_exact_round_exemption=ROUND_EXEMPT,baseline_d3=baseline[D3_ROUND],current_d3=entry(R/D3_ROUND),baseline_always_captured_d3=True,original_capture_excluded_only_lowercase_d4_test_prefix=True,baseline_and_finalizer_validate_current_exact_exemption=True))
write(E/'verification-summary.json',dict(focused=536,private=10,current_inventory=876,baseline_inventory=865,preserved_all_with_explicit_rename=True,added=11,vectors=6,audits=7,failures=0,errors=0,skips=0,disabled=0,full_gate=False,release_gate=False,initial_focused_passed=534,initial_focused=535,initial_private_passed=9,initial_private=10,corrected_private_only_after_fixture_change=True))
commands=json.loads((E/'gate-results.json').read_text());private=json.loads((E/'private/commands.json').read_text())
for command in commands:
 if command['label']=='private-corrected':command['seconds']=sum(row['seconds'] for row in private)
commands.append(dict(label='protection-corrected',argv=['python3','-B','tests/native/v5-02-x05-c4-d4-support/freeze.py','protect'],exit=0,note='Exact D4 exception validated; all D3/prior baseline entries protected'))
write(E/'gate-results.json',commands)
with (E/'final-gate.log').open('a')as f:f.write('\nExact D4-only exemption: protection-corrected 0; D3 original baseline mode/hash unchanged.\n')
def artifact():return {str(p.relative_to(E)):{**entry(p),'bytes':p.stat().st_size} for p in sorted(E.rglob('*')) if p.is_file() and p.name not in {'artifact-manifest.json','finalization.json'}}
manifest=artifact();write(E/'artifact-manifest.json',manifest)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
write(E/'finalization.json',dict(candidate_entries=len(candidate()),candidate_sha256=sha(E/'frozen-candidate.json'),artifact_entries=len(manifest),artifact_manifest_sha256=sha(E/'artifact-manifest.json'),report_sha256=sha(E/'REPORT.md'),protection_entries=protection['entries'],exact_round_exemption=ROUND_EXEMPT,d3_original_baseline_sha256=baseline[D3_ROUND]['sha256'],debug_and_owned_files_released=True,independent_review_pending=True))
assert artifact()==manifest and candidate()==json.loads((E/'frozen-candidate.json').read_text())
print((E/'finalization.json').read_text())
