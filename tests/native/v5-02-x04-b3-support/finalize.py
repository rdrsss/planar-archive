"""Finalize all B3 reports/copies/manifests before releasing ownership with zero further author edits."""
from pathlib import Path
import difflib,json,os,shutil,subprocess
from protect import ROOT,OUT,OWNED,filesystem,sha

def write(p,v):p.write_text(json.dumps(v,indent=2,sort_keys=True)+'\n')
subprocess.run(['python3','-B',ROOT/'tests/native/v5-02-x04-b3-support/audit.py'],check=True,cwd=ROOT)
variants={};build=ROOT/'build/rewrite-debug'
for manifest in (build/'src/engine').glob('*-generated/generation.json'):
 data=json.loads(manifest.read_text());data=data.get('files',data)
 for name,row in data.items():
  original=ROOT/'src/engine'/('export' if name.startswith('source_export') else 'ledger')/name;generated=manifest.parent/name
  assert sha(original)==row['source_sha256'] and sha(generated)==row['generated_sha256'] and row['reverse_identical']
  text=original.read_text()
  for sub in row['substitutions']:
   assert text.count(sub['before'])==sub['count'];text=text.replace(sub['before'],sub['after'])
  assert text==generated.read_text()
  for sub in reversed(row['substitutions']):text=text.replace(sub['after'],sub['before'])
  assert text==original.read_text()
 variants[manifest.parent.name]=data
assert len(variants)==5;write(OUT/'five-storage-export-variants.json',variants)
subprocess.run(['python3','-B',ROOT/'tests/native/v5-02-x04-b3-support/protect.py','final'],check=True,cwd=ROOT)
base=json.loads((OUT/'baseline-filesystem.json').read_text());final=json.loads((OUT/'final-filesystem.json').read_text())
changed=sorted(k for k in base.keys()|final.keys() if base.get(k)!=final.get(k))
assert all(k in OWNED or k.startswith(('tests/native/v5-02-x04-b3-support/','tests/vectors/effective-provenance/')) for k in changed)
assert final['AGENTS.md']['symlink']=='CLAUDE.md'
write(OUT/'ownership-audit.json',dict(changed=changed,unowned_changes=[],symlink_preserved=True,root_only_exclusions=['.git','build']))
names=sorted(set(OWNED)|{str(p.relative_to(ROOT)) for prefix in ['tests/native/v5-02-x04-b3-support','tests/vectors/effective-provenance'] for p in (ROOT/prefix).rglob('*') if p.is_file()})
candidate={};diff=[]
for name in names:
 p=ROOT/name;candidate[name]=sha(p);target=OUT/'final-owned'/name
 if not target.exists():target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,target)
 old=OUT/'baseline-owned'/name
 try:
  a=old.read_text().splitlines(keepends=True) if old.exists() else []
  diff.extend(difflib.unified_diff(a,p.read_text().splitlines(keepends=True),fromfile='baseline/'+name,tofile='candidate/'+name))
 except UnicodeError:diff.append('Binary candidate: '+name+' sha256:'+candidate[name]+'\n')
write(OUT/'candidate-manifest.json',candidate);(OUT/'owned.diff').write_text(''.join(diff))
freeze=json.loads((OUT/'attempt06-final/source-freeze.json').read_text())
for name in names:assert final.get(name)==freeze.get(name),(name,'changed after final gate')
write(OUT/'final-gate-stability.json',dict(candidate_matches_final_gate=True,source_freeze_sha256=sha(OUT/'attempt06-final/source-freeze.json')))
for attempt in sorted(OUT.glob('attempt*')):
 if attempt.is_dir():write(attempt/'manifest.json',{str(p.relative_to(attempt)):sha(p) for p in sorted(attempt.rglob('*')) if p.is_file() and p!=attempt/'manifest.json'})
write(OUT/'debug-build-identities.json',{str(p.relative_to(ROOT)):sha(p) for p in sorted(build.rglob('*')) if p.is_file() and not p.is_symlink() and (os.access(p,os.X_OK) or p.suffix in ('.o','.pcm') or p.name in ('CMakeCache.txt','compile_commands.json','build.ninja'))})
write(OUT/'identity.json',dict(head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),candidate_manifest_sha256=sha(OUT/'candidate-manifest.json'),owned_diff_sha256=sha(OUT/'owned.diff'),status='candidate awaiting independent review',build_ownership='released after final artifact manifest; zero further author edits'))
write(OUT/'artifact-manifest.json',{str(p.relative_to(OUT)):sha(p) for p in sorted(OUT.rglob('*')) if p.is_file() and p!=OUT/'artifact-manifest.json'})
print(json.dumps(dict(candidate_manifest_sha256=sha(OUT/'candidate-manifest.json'),owned_diff_sha256=sha(OUT/'owned.diff'),artifact_manifest_sha256=sha(OUT/'artifact-manifest.json'),changed=len(changed),candidate_paths=len(candidate))))
