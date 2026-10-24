from pathlib import Path
import os,stat,hashlib,json,sys,shutil
R=Path(__file__).resolve().parents[3]
O=R/'tests/native/v5-02-x06-a4-c01-evidence'
DOCS=['CLAUDE.md','docs/architecture.md','docs/testing.md']+['docs/vernacular/'+x+'.md' for x in ['implementation','roadmap','acceptance-matrix','v5-alignment','external-profile-implementation','x06-implementation']]
OWN=DOCS+['docs/vernacular/profile-preparation-architecture.md']
EX=['.git','tests/native/v5-02-x06-a4-c01-evidence','tests/native/v5-02-x06-a4-c01-support','docs/vernacular/rounds/V5-02-X06-A4-C01.md']
def inv():
 d={}
 def visit(p):
  r=p.relative_to(R).as_posix()
  if r in EX:return
  s=p.lstat();e=dict(mode=s.st_mode,device=s.st_dev,inode=s.st_ino,ctime_ns=s.st_ctime_ns,mtime_ns=s.st_mtime_ns,size=s.st_size)
  if p.is_symlink():e.update(type='link',target=os.readlink(p))
  elif p.is_dir():
   e.update(type='directory');d[r]=e
   for c in sorted(p.iterdir()):visit(c)
   return
  elif p.is_file():
   h=hashlib.sha256()
   with p.open('rb') as f:
    for b in iter(lambda:f.read(1048576),b''):h.update(b)
   e.update(type='file',sha256=h.hexdigest())
  else:e.update(type='other')
  d[r]=e
 for p in sorted(R.iterdir()):visit(p)
 return d
mode=sys.argv[1]; out=O/(mode+'.json');assert not out.exists()
if mode=='baseline':
 for n in OWN:
  p=R/n
  if p.exists():
   q=O/'preedit'/n;q.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(p,q)
d=inv();out.write_text(json.dumps(d,sort_keys=True,indent=2)+'\n');print(mode,len(d))
