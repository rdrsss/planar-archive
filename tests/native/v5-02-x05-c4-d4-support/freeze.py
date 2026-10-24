from pathlib import Path
import hashlib,json,stat,os,sys,difflib
R=Path(__file__).resolve().parents[3];S=Path(__file__).parent;E=R/'tests/native/v5-02-x05-c4-d4-evidence'
OWNED=['src/engine/runtime/'+x for x in ['runtime.cpp','external_registration.t.cpp']]+['src/stdlib/planar/render.lua','src/stdlib/planar/bootstrap.lua','CLAUDE.md','docs/architecture.md','docs/testing.md','docs/vernacular/implementation.md','docs/vernacular/roadmap.md','docs/vernacular/acceptance-matrix.md','docs/vernacular/v5-alignment.md','docs/vernacular/external-profile-implementation.md']
ROUND_EXEMPT='docs/vernacular/rounds/V5-02-X05-C4-D4.md'
D3_ROUND='docs/vernacular/rounds/V5-02-X05-C4-D3.md'
def exemptions():
 assert ROUND_EXEMPT=='docs/vernacular/rounds/V5-02-X05-C4-D4.md'
 result=set(OWNED)|{ROUND_EXEMPT}
 assert D3_ROUND not in result
 assert {n for n in result if n.startswith('docs/vernacular/rounds/')}=={ROUND_EXEMPT}
 return result
exemptions()
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
   if name.startswith('tests/native/v5-02-x05-c4-d4-'):continue
   current[name]=entry(q)
 return current
def candidate():
 return {str(p.relative_to(R)):entry(p) for p in [*(R/n for n in OWNED),*S.rglob('*')] if p.is_file()}
if __name__=='__main__':
 if sys.argv[1]=='baseline':
  exemptions();snapshot=protected();assert D3_ROUND in snapshot;write(E/'baseline.json',snapshot)
  for n in OWNED:
   p=R/n
   if p.exists():(E/('preedit-'+n.replace('/','_'))).write_bytes(p.read_bytes())
 elif sys.argv[1]=='freeze':
  write(E/'frozen-candidate.json',candidate())
  diff=[]
  for n in OWNED:
   old=(E/('preedit-'+n.replace('/','_'))).read_text().splitlines(keepends=True)
   new=(R/n).read_text().splitlines(keepends=True)
   diff.extend(difflib.unified_diff(old,new,fromfile='preedit/'+n,tofile='candidate/'+n))
   target=E/'candidate-source'/n;target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes((R/n).read_bytes())
  (E/'candidate.diff').write_text(''.join(diff))
 elif sys.argv[1]=='protect':
  a=json.loads((E/'baseline.json').read_text());b=protected();excluded=exemptions();assert D3_ROUND in a and a[D3_ROUND]==b[D3_ROUND]
  changed=sorted(n for n in a.keys()|b.keys() if n not in excluded and a.get(n)!=b.get(n));write(E/'final-protection.json',{'changed':changed,'entries':len(a),'release_absent':not(R/'build/rewrite-release').exists()});assert not changed,changed
