from pathlib import Path
import hashlib,json,stat,shutil,os
R=Path.cwd();E=R/'tests/native/v5-02-x05-c4-d2-b1-evidence';P=E/'private';D=E/'dependencies';B=R/'build/rewrite-debug'
def file(p):return dict(mode=stat.S_IMODE(p.stat().st_mode),sha256=hashlib.file_digest(p.open('rb'),'sha256').hexdigest(),bytes=p.stat().st_size,mtime_ns=p.stat().st_mtime_ns)
def tree(p):
 d={}
 for q in [p,*p.rglob('*')]:
  assert not q.is_symlink(),q
  d[str(q.relative_to(E))]=dict(kind='directory',mode=stat.S_IMODE(q.stat().st_mode)) if q.is_dir() else dict(kind='file',**file(q))
 return d
members=json.loads((P/'compiled-before.json').read_text())
dest=lambda p:D/str(Path(p).resolve()).lstrip('/')
assert {str(q) for q in D.rglob('*') if q.is_file()}=={str(dest(k)) for k in members}
assert all(file(Path(k))==v and file(dest(k))==v for k,v in members.items())
remove=tree(D)
products=[q for root in [P,*E.glob('development-private-*')] if root.is_dir() for q in root.iterdir() if q.suffix in {'.o','.pcm','.d','.dylib'} or q.name in {'baseline','green','probe'}]
for q in products:assert q.is_file() and not q.is_symlink();remove[str(q.relative_to(E))]=dict(kind='file',**file(q))
(E/'removal-inventory.json').write_text(json.dumps(remove,indent=2,sort_keys=True)+'\n')
assert tree(D)=={k:v for k,v in remove.items() if k=='dependencies' or k.startswith('dependencies/')}
for q in products:assert dict(kind='file',**file(q))==remove[str(q.relative_to(E))]
assert D==R/'tests/native/v5-02-x05-c4-d2-b1-evidence/dependencies'
for q in products:q.unlink()
shutil.rmtree(D)
assert not D.exists() and all(not p.exists() for p in products)
(E/'cleanup.json').write_text(json.dumps(dict(copied_dependency_files=len(members),removed_files=sum(v['kind']=='file' for v in remove.values()),removed_bytes=sum(v.get('bytes',0) for v in remove.values()),shared_inputs_unchanged=True,copied_membership_modes_bytes_verified=True,only_coder_owned_disposable_inputs_and_products_removed=True),indent=2)+'\n')
print((E/'cleanup.json').read_text())
