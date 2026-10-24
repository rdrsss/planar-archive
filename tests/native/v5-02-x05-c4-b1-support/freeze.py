"""Exact scoped identity and immutable non-owned protection; no product mutations."""
from pathlib import Path
import hashlib,json,stat,os
R=Path(__file__).resolve().parents[3];S=Path(__file__).parent;E=R/'tests/native/v5-02-x05-c4-b1-evidence'
OWNED=['src/engine/ledger/sqlite.cppm','src/engine/ledger/sqlite.cpp','src/engine/ledger/sqlite.t.cpp','docs/architecture.md','docs/testing.md','docs/vernacular/implementation.md','docs/vernacular/roadmap.md','docs/vernacular/external-profile-implementation.md']
def entry(p):
 d={'mode':stat.S_IMODE(p.lstat().st_mode)}
 if p.is_symlink():d['link']=os.readlink(p)
 elif p.is_file():d['sha256']=hashlib.sha256(p.read_bytes()).hexdigest()
 else:d['directory']=True
 return d
def write(p,d):p.write_text(json.dumps(d,sort_keys=True,indent=2)+'\n')
def candidate():return {str(p.relative_to(R)):entry(p) for p in [*(R/n for n in OWNED),E/'REPORT.md',*S.rglob('*')]}
def verify():assert candidate()==json.loads((E/'frozen-candidate.json').read_text())['entries']
def protection():
 before=json.loads((E/'baseline.json').read_text());current={}
 for top in ['src','vendor','cmake','scripts','fixtures','docs','tests','build/rewrite-release','build/install-release','CLAUDE.md','AGENTS.md','CMakeLists.txt','CMakePresets.json','Makefile']:
  p=R/top
  if not p.exists():current[top]={'absent':True};continue
  for q in [p,*p.rglob('*')] if p.is_dir() else [p]:
   name=str(q.relative_to(R))
   if name.startswith('tests/native/v5-02-x05-c4-b1-'):continue
   current[name]=entry(q)
 excluded=set(OWNED)|{'docs/vernacular/rounds/V5-02-X05-C4-B1.md'}
 changed=[name for name in before.keys()|current.keys() if name not in excluded and before.get(name)!=current.get(name)]
 write(E/'final-protection.json',{'baseline_entries':len(before),'current_entries':len(current),'protected_entries':len(before.keys()-excluded),'changed':sorted(changed),'release_absent':not(R/'build/rewrite-release').exists(),'agents_symlink':os.readlink(R/'AGENTS.md')})
 assert not changed,changed
if __name__=='__main__':write(E/'frozen-candidate.json',{'schema':1,'entries':candidate()})
