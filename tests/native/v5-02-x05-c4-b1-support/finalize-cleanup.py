"""Bind the smaller permanent evidence tree after authorized disposable-output cleanup."""
import json
from freeze import R,S,E,OWNED,candidate,entry,write,verify,protection
before=json.loads((E/'pre-cleanup-frozen-candidate.json').read_text())['entries'];now=candidate()
for name in OWNED:assert before[name]==now[name],name
changed={name for name in before.keys()|now.keys() if before.get(name)!=now.get(name)}
assert changed=={str((E/'REPORT.md').relative_to(R)),str((S/'cleanup.py').relative_to(R)),str((S/'finalize-cleanup.py').relative_to(R))},changed
write(E/'cleanup-candidate-delta.json',{'schema':1,'changed':sorted(changed),'owned_product_test_reference_entries_unchanged':len(OWNED),'tested_candidate':entry(E/'pre-cleanup-frozen-candidate.json')})
write(E/'frozen-candidate.json',{'schema':1,'entries':now});verify();protection()
files={str(p.relative_to(R)):entry(p) for p in E.rglob('*') if p.name not in {'artifact-manifest.json','finalization.json'}}
write(E/'artifact-manifest.json',{'schema':1,'entries':files})
write(E/'finalization.json',{'schema':1,'manifest':entry(E/'artifact-manifest.json'),'candidate':entry(E/'frozen-candidate.json'),'report':entry(E/'REPORT.md'),'artifact_entries':len(files),'candidate_entries':len(now),'cleanup':entry(E/'removed-private-build-products.json'),'tested_candidate':entry(E/'pre-cleanup-frozen-candidate.json'),'product_tests_unchanged':True})
verify()
