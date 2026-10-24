"""Bind B2's pure boundary and retained exact native signature source evidence."""
from pathlib import Path
import json,re
from protect import ROOT,OUT,OWNED,filesystem,sha
baseline=json.loads((OUT/'baseline-filesystem.json').read_text());now=filesystem()
changed=sorted(k for k in baseline.keys()|now.keys() if baseline.get(k)!=now.get(k))
assert all(k in OWNED or k.startswith('tests/native/v5-02-x04-b2-support/') for k in changed),changed
assert now['AGENTS.md']['symlink']=='CLAUDE.md'
paths=['src/engine/runtime/runtime.cpp','src/engine/runtime/runtime.cppm','src/stdlib/planar/commands.lua','src/engine/capture/profile_codec.cpp','src/engine/capture/profile_codec.cppm','src/engine/canonical/canonical.cpp','src/engine/canonical/canonical.cppm','src/engine/history/provenance_codec.cpp','src/engine/history/provenance_codec.cppm']
assert all(baseline[n]==now[n] for n in paths)
runtime=(ROOT/paths[0]).read_text();pure=(ROOT/'src/engine/history/effective_catalog.cpp').read_text()
def tuples(s):return sorted(re.findall(r'\{"(--[a-z-]+)",\s*"([a-z_]+)",\s*(true|false)\}',s))
old=runtime[runtime.index('std::vector<signature_option> command_signature'):runtime.index('void help_text')]
role=pure[pure.index('std::vector<signature> role_signature'):pure.index('std::vector<signature> history_signature')]
history=pure[pure.index('std::vector<signature> history_signature'):pure.index('std::vector<signature> shipped_signature')].replace(', id,', ', "decimal",')
# role and history helpers implement exactly the two extracted native signatures.
other=pure[pure.index('std::vector<signature> shipped_signature'):pure.index('void validate(')]
assert tuples(old)==tuples(role+history+other),(tuples(old),tuples(role+history+other))
assert 'history_signature("decimal")' in other and 'history_signature("history_id")' in pure
imports=re.findall(r'^(?:export )?import ([^;]+);',pure,re.M)
assert set(imports)=={'std','planar.engine.canonical'}
interface=(ROOT/'src/engine/history/effective_catalog.cppm').read_text()
assert set(re.findall(r'^(?:export )?import ([^;]+);',interface,re.M))=={'std','planar.engine.value'}
assert '.depth = 7' in pure and '.nodes = 9 + 64 * (25 + 32 * 9 + 3) + 128 * 21' in pure
report={'baseline_preserved_except_owned':True,'symlink_preserved':True,'unchanged_sources':{n:sha(ROOT/n) for n in paths},'native_signature_option_tuples_equal':True,'pure_imports':imports,'changed':changed,'notes':'Historical source membership and native structural signatures only; no callability, runtime lookup, filesystem access, aggregate B3 bound, or execution claim.'}
(OUT/'source-audit.json').write_text(json.dumps(report,indent=2,sort_keys=True)+'\n')
print('B2 scope, exact old runtime/source preservation, signatures and pure imports pass')
