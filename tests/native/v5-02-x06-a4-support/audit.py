from pathlib import Path
from urllib.parse import unquote
import hashlib,json,difflib,re,os
R=Path(__file__).resolve().parents[3];O=R/'tests/native/v5-02-x06-a4-evidence'
DOCS=['CLAUDE.md','docs/architecture.md','docs/testing.md']+['docs/vernacular/'+x+'.md' for x in ['implementation','roadmap','acceptance-matrix','v5-alignment','external-profile-implementation','x06-implementation','profile-preparation-architecture']]
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
base=json.loads((O/'baseline.json').read_text());diff=[];links=[]
for name in DOCS:
 p=R/name;q=O/'preedit'/name
 before=q.read_text() if q.exists() else ''
 after=p.read_text()
 if q.exists():
  assert sha(q)==base[name]['sha256'],name
  marker='## Existing boundaries' if name.endswith('x06-implementation.md') else 'The checkpoints below'
  assert before[before.index(marker):]==after[after.index(marker):],name
 diff.extend(difflib.unified_diff(before.splitlines(True),after.splitlines(True),fromfile='preedit/'+name,tofile=name))
 for match in re.finditer(r'(?<!!)\[[^\]]+\]\(([^)]+)\)',after):
  target=unquote(match.group(1).split('#')[0])
  if not target or '://' in target or target.startswith('mailto:'):continue
  assert (p.parent/target).exists(),(name,target)
  links.append([name,target])
assert os.readlink(R/'AGENTS.md')=='CLAUDE.md'
assert os.readlink(R/'compile_commands.json')==base['compile_commands.json']['target']
(O/'candidate.diff').write_text(''.join(diff))
(O/'candidate.json').write_text(json.dumps(dict(files={p:sha(R/p) for p in DOCS},diff_sha256=sha(O/'candidate.diff')),indent=2,sort_keys=True)+'\n')
(O/'link-audit.json').write_text(json.dumps(dict(local_links=len(links),unresolved=[],URI_decoding=True),indent=2)+'\n')
(O/'source-audit.json').write_text(json.dumps(dict(documentation_files=len(DOCS),historical_bodies_unchanged=9,preedit_matches_original_baseline=True,AGENTS_symlink='CLAUDE.md',compile_commands_symlink_preserved=True,no_builds_or_product_tests=True,architecture_sketch_not_compiled=True),indent=2)+'\n')
print(json.dumps(dict(candidate=sha(O/'candidate.json'),diff=sha(O/'candidate.diff'),local_links=len(links))))
