"""Freeze candidate, protect complete pre-edit membership/modes/bytes/symlinks."""
from pathlib import Path
import hashlib,json,os,stat,sys
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parents[3];EVIDENCE=ROOT/'tests/native/v5-02-x05-c2-b2-evidence';SUPPORT=Path(__file__).parent
OWNED=list(json.loads((EVIDENCE/'owned-before.json').read_text()))
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def write(p,v):p.write_text(json.dumps(v,sort_keys=True,indent=2)+'\n')
def entry(p):
 st=p.lstat();v={'mode':stat.S_IMODE(st.st_mode)}
 if p.is_symlink():v['symlink']=os.readlink(p)
 elif p.is_file():v['sha256']=sha(p)
 else:v['directory']=True
 return v
def walk(top):
 p=ROOT/top;return {str(x.relative_to(ROOT)):entry(x)for x in [p,*p.rglob('*')]}
def protected():
 data={}
 for top in ['src','vendor','cmake','scripts','fixtures','docs','tests']:
  for key,val in walk(top).items():
   if key.startswith('tests/native/v5-02-x05-c2-b2-')or key in OWNED or key.startswith('docs/vernacular/rounds/'):continue
   data[key]=val
 for p in ROOT.iterdir():
  if (p.is_file()or p.is_symlink())and p.name not in OWNED:data[p.name]=entry(p)
 old=json.loads((EVIDENCE/'protected-before.json').read_text());changed=[k for k in set(data)|set(old)if data.get(k)!=old.get(k)]
 assert not changed,changed
 release=walk('build/rewrite-release');assert release==json.loads((EVIDENCE/'release-before.json').read_text()),'release membership/modes/bytes changed'
 assert (ROOT/'AGENTS.md').is_symlink()and os.readlink(ROOT/'AGENTS.md')=='CLAUDE.md'
 return dict(protected_entries=len(data),release_entries=len(release),all_bytes_membership_modes_symlinks_preserved=True)
def candidate():
 paths={ROOT/n for n in OWNED}
 for top in ['src','cmake','scripts','fixtures','tests/stdlib','tests/vectors',str(SUPPORT.relative_to(ROOT))]:paths.update(p for p in (ROOT/top).rglob('*')if p.is_file())
 paths.update(ROOT/n for n in ['CMakeLists.txt','CMakePresets.json','Makefile','.clang-format'])
 return {str(p.relative_to(ROOT)):entry(p)for p in sorted(paths)}
def verify_candidate():
 current=candidate();assert current==json.loads((EVIDENCE/'frozen-candidate.json').read_text()),'candidate changed after freeze';return len(current)
if __name__=='__main__':
 if sys.argv[1]=='freeze':
  write(EVIDENCE/'frozen-candidate.json',candidate());write(EVIDENCE/'protected-before-gate.json',protected());print('frozen',len(candidate()),flush=True)
 else:
  print('verified',verify_candidate(),flush=True);write(EVIDENCE/'protected-final.json',protected());print('protected membership/modes/symlinks/bytes pass',flush=True)
