from pathlib import Path
import hashlib,json,stat,os,sys,difflib
R=Path(__file__).resolve().parents[3];S=Path(__file__).parent;E=R/'tests/native/v5-02-x05-c4-d2-b2-c01-evidence'
OWNED=['src/engine/capture/native.cpp','src/engine/capture/native.t.cpp','CLAUDE.md','docs/architecture.md','docs/testing.md','docs/vernacular/implementation.md','docs/vernacular/roadmap.md','docs/vernacular/acceptance-matrix.md','docs/vernacular/v5-alignment.md','docs/vernacular/external-profile-implementation.md']
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
   if name.startswith('tests/native/v5-02-x05-c4-d2-b2-c01-'):continue
   current[name]=entry(q)
 return current
def candidate():
 original=json.loads((R/'tests/native/v5-02-x05-c4-d2-b2-evidence/frozen-candidate.json').read_text())
 product=[R/n for n in original if n.startswith('src/')]
 return {str(p.relative_to(R)):entry(p) for p in [*(R/n for n in OWNED),*product,*S.rglob('*')]}
if __name__=='__main__':
 if sys.argv[1]=='baseline':
  assert "excluded=set(OWNED)|{'docs/vernacular/rounds/V5-02-X05-C4-D2-B2-C01.md'}" in Path(__file__).read_text()
  assert 'docs/vernacular/rounds/V5-02-X05-C4-D2-B2.md' in protected()
  write(E/'baseline.json',protected())
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
  original=R/'tests/native/v5-02-x05-c4-d2-b2-evidence'
  combined=[]
  for n in candidate():
   if not n.startswith('src/'):continue
   target=E/'combined-source'/n;target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes((R/n).read_bytes())
   before=original/('preedit-'+n.replace('/','_'))
   combined.extend(difflib.unified_diff(before.read_text().splitlines(keepends=True),(R/n).read_text().splitlines(keepends=True),fromfile='before-B2/'+n,tofile='combined/'+n))
  (E/'combined-B2-C01.diff').write_text(''.join(combined))
 elif sys.argv[1]=='protect':
  a=json.loads((E/'baseline.json').read_text());b=protected();excluded=set(OWNED)|{'docs/vernacular/rounds/V5-02-X05-C4-D2-B2-C01.md'}
  changed=sorted(n for n in a.keys()|b.keys() if n not in excluded and a.get(n)!=b.get(n));write(E/'final-protection.json',{'changed':changed,'entries':len(a),'release_absent':not(R/'build/rewrite-release').exists()});assert not changed,changed
