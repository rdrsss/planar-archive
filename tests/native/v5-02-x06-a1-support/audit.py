from pathlib import Path
import hashlib,json,difflib,re,os
R=Path(__file__).resolve().parents[3];O=R/'tests/native/v5-02-x06-a1-evidence';base=json.loads((O/'baseline.json').read_text())
new=['src/engine/history/interpretation_v2_codec.'+s for s in ['cppm','cpp','t.cpp']]
owned=['CLAUDE.md','docs/architecture.md','docs/testing.md']+['docs/vernacular/'+s+'.md' for s in ['implementation','roadmap','acceptance-matrix','v5-alignment','external-profile-implementation','x06-implementation']]+['src/engine/CMakeLists.txt','src/engine/history/effective_provenance.cpp','src/engine/history/provenance_codec.cpp']+new
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
for p in ['src/engine/history/interpretation_codec.cpp','src/engine/history/interpretation_codec.cppm','src/engine/history/interpretation_codec.t.cpp','docs/vernacular/external-profile-contract.md','src/engine/runtime/runtime.cpp','src/stdlib/planar/semantic.lua']:
 assert sha(R/p)==base[p]['sha256'],p
old=(O/'preedit/src/engine/CMakeLists.txt').read_text();assert (R/'src/engine/CMakeLists.txt').read_text().startswith(old)
for p in owned[:8]:
 a=(O/'preedit'/p).read_text();b=(R/p).read_text();marker='The checkpoints below';assert a[a.index(marker):]==b[b.index(marker):],p
assert os.readlink(R/'AGENTS.md')=='CLAUDE.md';assert os.readlink(R/'compile_commands.json')==base['compile_commands.json']['target']
cpp=(R/new[1]).read_text();interface=(R/new[0]).read_text()
assert 'verify_effective_stdlib(files)' in cpp and 'decode_catalog(' in cpp and 'shape_definition_v1' in cpp
assert 'owner' in cpp and 'module_bytes' in cpp and 'hash_value(' in cpp
assert 'std::span<const tree_file> raw_files' in interface
for line in cpp.splitlines():
 if line.startswith('import '):assert not any(s in line for s in ['runtime','ledger','capture.native','lua','source_export']),line
inventory=json.loads((O/'inventory.json').read_text());prior=json.loads((R/'tests/native/v5-02-x05-c5-evidence/debug/inventory.log').read_text());names={t['name'] for t in inventory['tests']};previous={t['name'] for t in prior['tests']};assert len(previous)==876 and previous<=names and len(names)==892
(O/'inventory-preservation.json').write_text(json.dumps(dict(prior=len(previous),current=len(names),added=sorted(names-previous),missing=sorted(previous-names)),indent=2)+'\n')
diff=[]
for p in owned:
 a=O/'preedit'/p;before=a.read_text() if a.exists() else ''
 diff+=difflib.unified_diff(before.splitlines(True),(R/p).read_text().splitlines(True),fromfile='preedit/'+p,tofile=p)
(O/'candidate.diff').write_text(''.join(diff))
candidate={p:sha(R/p) for p in owned}
for p in (R/'tests/vectors/interpretation-v2').iterdir():
 if p.is_file():candidate[p.relative_to(R).as_posix()]=sha(p)
(O/'candidate.json').write_text(json.dumps(dict(files=candidate,diff_sha256=sha(O/'candidate.diff')),sort_keys=True,indent=2)+'\n')
(O/'source-audit.json').write_text(json.dumps(dict(owned_files=len(owned),unchanged_v1_codec=True,old_contract_runtime_and_Lua_unchanged=True,CMake_append_only=True,historical_status_bodies_unchanged=8,mandatory_rawtree_both_APIs=True,pure_imports=True,AGENTS_symlink='CLAUDE.md',previous_names_preserved=876,new_cases=16),indent=2)+'\n')
print('source audit and candidate snapshot pass; 876 names preserved plus 16')
