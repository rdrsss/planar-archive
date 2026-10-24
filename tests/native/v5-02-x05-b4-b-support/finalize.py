"""Verify frozen/protected bytes and identify exact owned diff and artifacts."""
from pathlib import Path
import hashlib,json,difflib,subprocess
root=Path(__file__).resolve().parents[3];e=root/'tests/native/v5-02-x05-b4-b-evidence'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
checks={}
for name in ['frozen-source.json','protected-start.json','protected-unowned.json']:
 rows=json.loads((e/name).read_text())
 for p,digest in rows.items():assert sha(root/p)==digest,p
 checks[name]={'entries':len(rows),'all_unchanged':True}
assert (root/'AGENTS.md').is_symlink()
assert (root/'AGENTS.md').readlink()==Path('CLAUDE.md')
patch=''
owned=json.loads((e/'baseline.json').read_text())['sha256']
for name in owned:
 before=(e/'baseline'/name).read_text().splitlines(keepends=True);after=(root/name).read_text().splitlines(keepends=True)
 patch+=''.join(difflib.unified_diff(before,after,fromfile='a/'+name,tofile='b/'+name))
(e/'candidate.diff').write_text(patch)
frozen=json.loads((e/'frozen-source.json').read_text())
(e/'candidate-manifest.json').write_text(json.dumps(dict(sha256=frozen,owned_diff_sha256=sha(e/'candidate.diff'),head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip()),indent=2)+'\n')
(e/'checks.json').write_text(json.dumps(dict(checks=checks,agents_symlink_preserved=True,debug_released=True,independent_review_required=True),indent=2)+'\n')
artifacts={str(p.relative_to(root)):sha(p) for p in sorted(e.rglob('*')) if p.is_file() and p.name!='artifact-manifest.json' and '__pycache__' not in p.parts}
(e/'artifact-manifest.json').write_text(json.dumps(artifacts,indent=2)+'\n')
print(json.dumps(dict(protected=checks,diff_sha256=sha(e/'candidate.diff'),manifest_sha256=sha(e/'candidate-manifest.json'),artifacts=len(artifacts))))
