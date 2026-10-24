"""C5 identity and protection guard; immutable initial capture."""
from pathlib import Path
import os,stat,hashlib,json,sys,shutil
R=Path(__file__).resolve().parents[3];S=Path(__file__).parent;E=R/'tests/native/v5-02-x05-c5-evidence'
STATUS=['CLAUDE.md','docs/architecture.md','docs/testing.md']+['docs/vernacular/'+n+'.md' for n in ['implementation','roadmap','acceptance-matrix','v5-alignment','external-profile-implementation']]
ROUND='docs/vernacular/rounds/V5-02-X05-C5.md';DOC='docs/vernacular/x05-projection-evidence.md'
def write(p,x):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(x,indent=2,sort_keys=True)+'\n')
_cache={}
def entry(p):
 st=p.lstat();v={'mode':stat.S_IMODE(st.st_mode)}
 if p.is_symlink():v['link']=os.readlink(p)
 elif p.is_file():
  key=(st.st_dev,st.st_ino,st.st_ctime_ns,st.st_mtime_ns,st.st_size,st.st_mode)
  previous=_cache.get(str(p))
  if previous and previous[0]==key:digest=previous[1]
  else:
   with p.open('rb') as f:digest=hashlib.file_digest(f,'sha256').hexdigest()
   _cache[str(p)]=(key,digest)
  v.update(sha256=digest,bytes=st.st_size)
 else:v['directory']=True
 return v
def snapshot():
 rows={}
 for top in ['src','vendor','cmake','scripts','fixtures','docs','tests','build/install-release','CLAUDE.md','AGENTS.md','CMakeLists.txt','CMakePresets.json','Makefile']:
  p=R/top
  if not p.exists():rows[top]={'absent':True};continue
  for root,ds,fs in os.walk(p,followlinks=False) if p.is_dir() else []:
   base=Path(root);ds[:]=[n for n in ds if base/n not in [S,E]]
   for q in [base,*[base/n for n in ds+fs]]:rows[str(q.relative_to(R))]=entry(q)
  if not p.is_dir():rows[top]=entry(p)
 return rows
def exemptions():
 x=set(STATUS+[ROUND,DOC]);assert {n for n in x if n.startswith('docs/vernacular/rounds/')}=={ROUND}
 assert all('D4' not in n and 'D3' not in n for n in x)
 return x
def stable(final=False):
 a=json.loads((E/'baseline.json').read_text());b=snapshot();excluded=exemptions() if final else {ROUND}
 changes=sorted(k for k in a.keys()|b.keys() if k not in excluded and a.get(k)!=b.get(k))
 assert not changes,changes
 return {'entries':len(a),'unauthorized_changes':changes,'coordinator_round_exemption':ROUND,'product_test_vendor_tooling_unchanged':True}
if __name__=='__main__':
 if sys.argv[1]=='baseline':
  assert not (E/'baseline.json').exists();assert not (R/'build/rewrite-release').exists()
  write(E/'release-precreation.json',{'path':'build/rewrite-release','absent':True});a=snapshot();exemptions()
  assert 'docs/vernacular/rounds/V5-02-X05-C4-D4-C01.md' in a
  write(E/'baseline.json',a)
  for n in STATUS+[DOC]:
   if (R/n).exists():p=E/'preedit'/n;p.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(R/n,p)
  print('baseline entries',len(a),flush=True)
 elif sys.argv[1]=='protect':write(E/'final-protection.json',stable(True))
