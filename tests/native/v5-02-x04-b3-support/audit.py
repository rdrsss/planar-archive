"""Audit exact B3 ownership, pure imports and composed passing evidence without mutating product inputs."""
from pathlib import Path
import json,re,xml.etree.ElementTree as ET
from protect import ROOT,OUT,OWNED,filesystem,sha
base=json.loads((OUT/'baseline-filesystem.json').read_text());now=filesystem()
changed=sorted(k for k in base.keys()|now.keys() if base.get(k)!=now.get(k))
def allowed(n):return n in OWNED or n.startswith(('tests/native/v5-02-x04-b3-support/','tests/vectors/effective-provenance/'))
assert all(allowed(k) for k in changed),[k for k in changed if not allowed(k)]
assert now['AGENTS.md']['symlink']=='CLAUDE.md'
pure=(ROOT/'src/engine/history/effective_provenance.cpp').read_text();interface=(ROOT/'src/engine/history/effective_provenance.cppm').read_text()
imports=re.findall(r'^(?:export )?import ([^;]+);',pure,re.M)
assert set(imports)=={'std','planar.engine.capture.profile_codec','planar.engine.history.effective_catalog','planar.engine.canonical'}
assert set(re.findall(r'^(?:export )?import ([^;]+);',interface,re.M))=={'std','planar.engine.history.provenance_codec'}
assert 'tree_profile::effective_provenance' in pure and 'verify_stdlib(input.embedded)' in pure
assert not re.search(r'filesystem|lua_|sqlite|intern\(',pure)
paths=['src/engine/capture/profile_codec.'+e for e in ['cppm','cpp']]+['src/engine/history/'+n+'.'+e for n in ['effective_catalog','provenance_codec','tree_core','record'] for e in ['cppm','cpp']]+['src/engine/runtime/runtime.cpp','src/engine/runtime/runtime.cppm','src/stdlib/planar/commands.lua']
assert all(base[n]==now[n] for n in paths)
a=ET.parse(OUT/'attempt02/focused.xml').getroot().findall('.//testcase')
b=ET.parse(OUT/'attempt05-oracle-tests/focused.xml').getroot().findall('.//testcase')
isnew=lambda c:c.get('name').startswith('replacement.replacement.effective provenance')
inherited=[c for c in a if not isnew(c)]
assert len(a)==98 and len(inherited)==86 and len(b)==12
for c in inherited+b:assert c.get('status')=='run' and all(c.find(t) is None for t in ['failure','error','skipped'])
assert set(c.get('name') for c in a)==set(c.get('name') for c in inherited+b)
freeze=json.loads((OUT/'attempt05-oracle-tests/source-freeze.json').read_text())
for n in ['src/engine/history/effective_provenance.'+e for e in ['cppm','cpp','t.cpp']]+['src/engine/CMakeLists.txt']:
 assert now[n]==freeze[n]
# Product implementation has been stable throughout the failed oracle/test setup attempts.
first=json.loads((OUT/'attempt01/source-freeze.json').read_text())
for e in ['cppm','cpp']:assert now['src/engine/history/effective_provenance.'+e]==first['src/engine/history/effective_provenance.'+e]
report=dict(changed=changed,unowned_changes=[],root_only_exclusions=['.git','build'],symlink_preserved=True,pure_imports=imports,unchanged_component_sources={n:sha(ROOT/n) for n in paths},implementation_unchanged_since_attempt01=True,coverage=dict(inherited_passing=86,new_passing=12,distinct=98,single_full_selection_pass=False,notes='Composed focused evidence only: attempt02 inherited cases and attempt05 exact new target. No full520-suite or release/Linux result.'))
(OUT/'source-audit.json').write_text(json.dumps(report,indent=2,sort_keys=True)+'\n')
print('Owned scope, unchanged components, pure imports and86 inherited+12 new passing cases verified')
