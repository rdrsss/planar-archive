"""Authorized same-round removal of verified generated private build products only."""
from pathlib import Path
import json,shutil
from freeze import R,E,entry,write
manifest=json.loads((E/'artifact-manifest.json').read_text());final=json.loads((E/'finalization.json').read_text())
assert entry(E/'artifact-manifest.json')==final['manifest']
assert entry(E/'frozen-candidate.json')==final['candidate']
assert entry(E/'REPORT.md')==final['report']
for name,identity in manifest['entries'].items():assert entry(R/name)==identity,name
compiled=json.loads((E/'private/compiled-before.json').read_text())
for name,identity in compiled.items():assert entry(Path(name))==identity,name
removed={}
private_names={'private',*(f'private-attempt{i}' for i in range(1,6))}
for p in E.rglob('*'):
 if not p.is_file():continue
 relative=p.relative_to(E);root=relative.parts[0]
 if (root=='dependencies' or root in private_names) and (p.suffix in {'.o','.pcm','.a','.dylib'} or (root in private_names and p.name in {'red','green'})):
  assert not p.is_symlink() and p.resolve().is_relative_to(E.resolve())
  name=str(p.relative_to(R));assert name in manifest['entries']
  identity=entry(p);assert identity==manifest['entries'][name]
  removed[name]={**identity,'bytes':p.stat().st_size}
for name in ['artifact-manifest.json','finalization.json','frozen-candidate.json','REPORT.md']:
 target=E/('pre-cleanup-'+name);assert not target.exists();shutil.copy2(E/name,target)
write(E/'removed-private-build-products.json',{'schema':1,'entries':removed,'files':len(removed),'bytes':sum(x['bytes'] for x in removed.values()),'prior_finalization':final,'shared_dependencies_verified':len(compiled)})
for name in removed:(R/name).unlink()
# Remove only newly empty directories inside this round's own disposable dependency tree.
dep=E/'dependencies'
if dep.exists():
 for p in sorted((p for p in dep.rglob('*') if p.is_dir()),key=lambda p:len(p.parts),reverse=True):p.rmdir()
 dep.rmdir()
assert all(not(R/name).exists() for name in removed)
for name,identity in compiled.items():assert entry(Path(name))==identity,name
write(E/'cleanup-final.json',{'schema':1,'removed_files':len(removed),'removed_bytes':sum(x['bytes'] for x in removed.values()),'all_planned_removed':True,'shared_dependencies':len(compiled),'shared_unchanged':True,'product_tests_rerun':False})
print(json.dumps(json.loads((E/'cleanup-final.json').read_text()),indent=2))
