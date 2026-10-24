from pathlib import Path
import hashlib,json,stat,os,sys
R=Path(__file__).resolve().parents[3];S=Path(__file__).parent;E=R/'tests/native/v5-02-x05-c4-b3-c01-evidence'
OWNED=['src/engine/runtime/runtime.cpp','src/engine/runtime/external_registration.t.cpp','CLAUDE.md','docs/architecture.md','docs/testing.md','docs/vernacular/implementation.md','docs/vernacular/roadmap.md','docs/vernacular/acceptance-matrix.md','docs/vernacular/external-profile-implementation.md']
def entry(p):
 d={'mode':stat.S_IMODE(p.lstat().st_mode)}
 if p.is_symlink():d['link']=os.readlink(p)
 elif p.is_file():d['sha256']=hashlib.sha256(p.read_bytes()).hexdigest()
 else:d['directory']=True
 return d
def write(p,d):p.write_text(json.dumps(d,sort_keys=True,indent=2)+'\n')
def protected():
 current={}
 for top in ['src','vendor','cmake','scripts','fixtures','docs','tests','build/rewrite-release','build/install-release','CLAUDE.md','AGENTS.md','CMakeLists.txt','CMakePresets.json','Makefile']:
  p=R/top
  if not p.exists():current[top]={'absent':True};continue
  for q in [p,*p.rglob('*')] if p.is_dir() else [p]:
   name=str(q.relative_to(R))
   if name.startswith('tests/native/v5-02-x05-c4-b3-c01-'):continue
   current[name]=entry(q)
 return current
def candidate():return {str(p.relative_to(R)):entry(p) for p in [*(R/n for n in OWNED),E/'REPORT.md',*S.rglob('*')]}
if __name__=='__main__':
 if sys.argv[1]=='baseline':
  write(E/'baseline.json',protected())
  for n in OWNED:
   p=R/n
   if p.exists():(E/('preedit-'+n.replace('/','_'))).write_bytes(p.read_bytes())
 elif sys.argv[1]=='freeze':write(E/'frozen-candidate.json',candidate())
 elif sys.argv[1]=='protect':
  a=json.loads((E/'baseline.json').read_text());b=protected();excluded=set(OWNED)|{'docs/vernacular/rounds/V5-02-X05-C4-B3-C01.md'}
  changed=sorted(n for n in a.keys()|b.keys() if n not in excluded and a.get(n)!=b.get(n));write(E/'final-protection.json',{'changed':changed,'entries':len(a),'release_absent':not(R/'build/rewrite-release').exists()});assert not changed,changed
