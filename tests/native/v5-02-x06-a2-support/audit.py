from pathlib import Path
import hashlib,json,difflib,subprocess,re,os
R=Path(__file__).resolve().parents[3];O=R/'tests/native/v5-02-x06-a2-evidence';base=json.loads((O/'baseline.json').read_text())
DOCS=['CLAUDE.md','docs/architecture.md','docs/testing.md']+['docs/vernacular/'+x+'.md' for x in ['implementation','roadmap','acceptance-matrix','v5-alignment','external-profile-implementation','x06-implementation']]
new=['src/engine/history/recorded_interpretation.'+s for s in ['cppm','cpp','t.cpp']]
owned=DOCS+['src/engine/CMakeLists.txt','src/engine/history/interpretation_codec.cpp']+new
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
for p in ['src/engine/history/interpretation_codec.cppm','src/engine/history/interpretation_codec.t.cpp','src/engine/history/interpretation_v2_codec.cpp','src/engine/history/interpretation_v2_codec.cppm','src/engine/history/effective_provenance.cpp','src/engine/history/provenance_codec.cpp','docs/vernacular/external-profile-contract.md','src/engine/ledger/format.cpp','src/engine/history/read.cpp','src/engine/export/source.cpp','src/engine/runtime/runtime.cpp']:
 if p in base:assert sha(R/p)==base[p]['sha256'],p
old=(O/'preedit/src/engine/history/interpretation_codec.cpp').read_text()
expected=old.replace('  if (!embedded_versions || *embedded_versions != versions_encoded)\n','  if (!embedded_versions)\n    return std::unexpected(codec_failure(embedded_versions.error()));\n  if (*embedded_versions != versions_encoded)\n')
assert (R/'src/engine/history/interpretation_codec.cpp').read_text()==expected
assert (R/'src/engine/CMakeLists.txt').read_text().startswith((O/'preedit/src/engine/CMakeLists.txt').read_text())
for p in DOCS[:-1]:
 a=(O/'preedit'/p).read_text();b=(R/p).read_text();marker='The checkpoints below';assert a[a.index(marker):]==b[b.index(marker):],p
p=DOCS[-1];a=(O/'preedit'/p).read_text();b=(R/p).read_text();marker='## Existing boundaries';assert a[a.index(marker):]==b[b.index(marker):]
assert os.readlink(R/'AGENTS.md')=='CLAUDE.md'
assert os.readlink(R/'compile_commands.json')==base['compile_commands.json']['target']
for p in new[:2]:
 for line in (R/p).read_text().splitlines():
  if line.startswith(('import ','export import ')):assert not any(t in line for t in ['runtime','ledger','capture.native','lua','source_export']),line
inventory=json.loads((O/'inventory.json').read_text());prior=json.loads((R/'tests/native/v5-02-x06-a1-evidence/inventory.json').read_text());names={t['name'] for t in inventory['tests']};previous={t['name'] for t in prior['tests']};assert len(previous)==892 and previous<=names and len(names)==910
commands=subprocess.run(['ninja','-C',str(R/'build/rewrite-debug'),'-t','commands','planar_history_recorded_interpretation_tests'],capture_output=True,text=True,check=True).stdout
link=next(l for l in commands.splitlines() if ' -o src/engine/planar_history_recorded_interpretation_tests ' in l)
assert not any(x in link for x in ['libplanar_lua','libplanar_runtime','libplanar_ledger_format','libplanar_ledger_sqlite','libplanar_capture_native','libplanar_history_read'])
nm=subprocess.run(['nm','-u',str(R/'build/rewrite-debug/src/engine/planar_history_recorded_interpretation_tests')],capture_output=True,text=True,check=True)
assert not re.search(r'\b_?lua(?:L)?_',nm.stdout)
(O/'undefined-symbols.log').write_text(nm.stdout+nm.stderr)
(O/'no-lua-proof.json').write_text(json.dumps(dict(no_lua_symbols=True,no_runtime_storage_nativecapture_link=True,pure_ledger_types_transitive=True,link=link),indent=2)+'\n')
diff=[]
for p in owned:
 a=O/'preedit'/p;before=a.read_text() if a.exists() else ''
 diff+=difflib.unified_diff(before.splitlines(True),(R/p).read_text().splitlines(True),fromfile='preedit/'+p,tofile=p)
(O/'candidate.diff').write_text(''.join(diff));candidate={p:sha(R/p) for p in owned}
(O/'candidate.json').write_text(json.dumps(dict(files=candidate,diff_sha256=sha(O/'candidate.diff')),sort_keys=True,indent=2)+'\n')
(O/'source-audit.json').write_text(json.dumps(dict(owned_files=len(owned),old_codec_single_resource_adapter=True,old_v1_interface_tests_unchanged=True,approved_A1_and_provenance_unchanged=True,storage_read_export_contract_runtime_Lua_unchanged=True,CMake_append_only=True,historical_status_bodies_unchanged=8,x06_historical_body_unchanged=True,pure_imports=True,AGENTS_symlink='CLAUDE.md',previous_names_preserved=892,new_cases=18),indent=2)+'\n')
print('source and no-Lua audit pass; 892 names preserved plus18')
