"""Freeze owned candidate and protect source, historical evidence and release bytes."""
from pathlib import Path
import hashlib,json,os,stat,sys
ROOT=Path(__file__).resolve().parents[3]
EVIDENCE=ROOT/'tests/native/v5-02-x05-c1-evidence'
SUPPORT=ROOT/'tests/native/v5-02-x05-c1-support'
OWNED=['src/stdlib/planar/render.lua','src/stdlib/planar/bootstrap.lua','src/engine/runtime/external_registration.t.cpp','CLAUDE.md','docs/architecture.md','docs/testing.md','docs/vernacular/implementation.md','docs/vernacular/roadmap.md','docs/vernacular/external-profile-implementation.md']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def write(p,v):p.write_text(json.dumps(v,indent=2,sort_keys=True)+'\n')
def filesystem():
 rows={}
 for current,dirs,files in os.walk(ROOT,followlinks=False):
  base=Path(current)
  dirs[:]=[n for n in dirs if not(base==ROOT and n in ['.git','build']) and base/n!=EVIDENCE]
  for n in files+dirs:
   p=base/n;s=p.lstat();key=str(p.relative_to(ROOT))
   if stat.S_ISLNK(s.st_mode):rows[key]={'symlink':os.readlink(p),'mode':stat.S_IMODE(s.st_mode)}
   elif stat.S_ISREG(s.st_mode):rows[key]={'sha256':sha(p),'mode':stat.S_IMODE(s.st_mode)}
 return rows

def candidate():
 paths={ROOT/n for n in OWNED}
 for directory in ['src','cmake','scripts','fixtures','tests/stdlib','tests/vectors','tests/native/v5-02-x05-c1-support']:
  paths.update(p for p in (ROOT/directory).rglob('*') if p.is_file())
 paths.update(ROOT/n for n in ['CMakeLists.txt','CMakePresets.json','Makefile','.clang-format'])
 return {str(p.relative_to(ROOT)):sha(p) for p in sorted(paths)}
def verify_candidate():
 expected=json.loads((EVIDENCE/'frozen-candidate.json').read_text());actual=candidate()
 assert actual==expected,'candidate changed'
 return len(actual)
def protected():
 old=json.loads((EVIDENCE/'baseline-filesystem.json').read_text());now=filesystem()
 allowed=set(OWNED)|{'docs/vernacular/rounds/V5-02-X05-C1.md','docs/vernacular/rounds/V5-02-X05-C.md'}
 changed=[n for n in old if old[n]!=now.get(n)];assert set(changed)<=allowed,changed
 new=set(now)-set(old);assert all(n.startswith('tests/native/v5-02-x05-c1-support/') or n in allowed for n in new),new
 release=ROOT/'build/rewrite-release'
 compiled={str(p.relative_to(release)):sha(p) for p in release.rglob('*') if p.is_file() and p.suffix in {'.o','.pcm','.a'}}
 assert compiled==json.loads((EVIDENCE/'rewrite-release-start.json').read_text()),'protected release compiled bytes changed'
 assert (ROOT/'AGENTS.md').is_symlink() and os.readlink(ROOT/'AGENTS.md')=='CLAUDE.md'
 return dict(baseline_entries=len(old),current_entries=len(now),changed=changed,added=sorted(new),protected_release_compiled=len(compiled),historical_files_membership_bytes_modes_symlinks_preserved=True)
if __name__=='__main__':
 if sys.argv[1]=='freeze':
  write(EVIDENCE/'frozen-candidate.json',candidate());write(EVIDENCE/'frozen-filesystem.json',filesystem());write(EVIDENCE/'protected-before-gate.json',protected())
  print('Frozen candidate',len(candidate()))
 else:
  print('Verified candidate',verify_candidate());write(EVIDENCE/'protected-final.json',protected())
