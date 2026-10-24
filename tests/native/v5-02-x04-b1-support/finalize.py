"""Freeze final B1 candidate, ownership proof and complete author evidence."""
from pathlib import Path
import difflib,json,os,shutil,subprocess
from protect import ROOT,OUT,OWNED,filesystem,sha

def write(p,v):p.write_text(json.dumps(v,indent=2,sort_keys=True)+'\n')
subprocess.run(['python3','-B',ROOT/'tests/native/v5-02-x04-b1-support/protect.py','final'],check=True,cwd=ROOT)
base=json.loads((OUT/'baseline-filesystem.json').read_text());final=json.loads((OUT/'final-filesystem.json').read_text())
changed=sorted(k for k in base.keys()|final.keys() if base.get(k)!=final.get(k))
assert all(k in OWNED or k.startswith('tests/native/v5-02-x04-b1-support/') for k in changed),changed
assert final['AGENTS.md']['symlink']=='CLAUDE.md'
write(OUT/'ownership-audit.json',dict(changed=changed,unowned_changes=[],symlink_preserved=True,root_only_exclusions=['.git','build']))
candidate={};diff=[]
for name in OWNED:
 p=ROOT/name
 if not p.exists():continue
 candidate[name]=sha(p)
 old=OUT/'baseline-owned'/name
 a=old.read_text().splitlines(keepends=True) if old.exists() else []
 diff.extend(difflib.unified_diff(a,p.read_text().splitlines(keepends=True),fromfile='baseline/'+name,tofile='candidate/'+name))
for p in sorted((ROOT/'tests/native/v5-02-x04-b1-support').glob('*.py')):
 name=str(p.relative_to(ROOT));candidate[name]=sha(p);target=OUT/'final-owned'/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,target)
write(OUT/'candidate-manifest.json',candidate);(OUT/'owned.diff').write_text(''.join(diff))
freeze=json.loads((OUT/'attempt02-final-docs/source-freeze.json').read_text())
for name in OWNED:assert final.get(name)==freeze.get(name),(name,'changed since final gate')
write(OUT/'final-gate-stability.json',dict(candidate_matches_final_gate=True,source_freeze_sha256=sha(OUT/'attempt02-final-docs/source-freeze.json')))
build=ROOT/'build/rewrite-debug'
write(OUT/'debug-build-identities.json',{str(p.relative_to(ROOT)):sha(p) for p in sorted(build.rglob('*')) if p.is_file() and not p.is_symlink() and (os.access(p,os.X_OK) or p.suffix in ('.o','.pcm') or p.name in ('CMakeCache.txt','compile_commands.json','build.ninja'))})
write(OUT/'identity.json',dict(head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),candidate_manifest_sha256=sha(OUT/'candidate-manifest.json'),owned_diff_sha256=sha(OUT/'owned.diff'),status='candidate awaiting independent review',build_ownership='released after final artifact manifest; zero further author edits'))
write(OUT/'artifact-manifest.json',{str(p.relative_to(OUT)):sha(p) for p in sorted(OUT.rglob('*')) if p.is_file() and p!=OUT/'artifact-manifest.json'})
print(json.dumps(dict(candidate_manifest_sha256=sha(OUT/'candidate-manifest.json'),owned_diff_sha256=sha(OUT/'owned.diff'),artifact_manifest_sha256=sha(OUT/'artifact-manifest.json'),changed=len(changed))))
