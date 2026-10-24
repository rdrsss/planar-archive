"""Freeze owned candidate and protect source, historical evidence and release bytes."""
from pathlib import Path
import hashlib,json,os,stat,sys
ROOT=Path(__file__).resolve().parents[3]
EVIDENCE=ROOT/'tests/native/v5-02-x05-c2-a-evidence'
SUPPORT=ROOT/'tests/native/v5-02-x05-c2-a-support'
OWNED=['src/engine/runtime/runtime.cppm','src/engine/runtime/runtime.cpp','src/engine/runtime/external_registration.t.cpp','CLAUDE.md','docs/architecture.md','docs/testing.md','docs/vernacular/implementation.md','docs/vernacular/roadmap.md','docs/vernacular/external-profile-implementation.md']
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
 for directory in ['src','cmake','scripts','fixtures','tests/stdlib','tests/vectors','tests/native/v5-02-x05-c2-a-support']:
  paths.update(p for p in (ROOT/directory).rglob('*') if p.is_file())
 paths.update(ROOT/n for n in ['CMakeLists.txt','CMakePresets.json','Makefile','.clang-format'])
 return {str(p.relative_to(ROOT)):sha(p) for p in sorted(paths)}
def verify_candidate():
 expected=json.loads((EVIDENCE/'frozen-candidate.json').read_text());actual=candidate()
 assert actual==expected,'candidate changed'
 return len(actual)
def protected():
 release=ROOT/'build/rewrite-release'
 expected=json.loads((EVIDENCE/'preedit/release-manifest.json').read_text())
 actual={str(p):sha(p)for p in sorted(release.rglob('*'))if p.is_file()}
 assert actual==expected,'protected release files changed'
 assert (ROOT/'AGENTS.md').is_symlink() and os.readlink(ROOT/'AGENTS.md')=='CLAUDE.md'
 before=(EVIDENCE/'preedit/src/engine/runtime/runtime.cpp').read_text();after=(ROOT/'src/engine/runtime/runtime.cpp').read_text()
 shape='std::expected<shape_definition, registration_error> command_vm::resolve_shape_definition'
 oldend='std::expected<command_vm, registration_error> command_vm::create_impl'
 newend='std::expected<projection_definition, registration_error> command_vm::resolve_projection_definition'
 assert before[before.index(shape):before.index(oldend)]==after[after.index(shape):after.index(newend)]
 checkpoint=ROOT/'tests/native/v5-02-x05-c1-review-evidence/source-before.json'
 old=json.loads(checkpoint.read_text());now=filesystem()
 allowed=set(OWNED)|{'docs/vernacular/rounds/V5-02-X05-C1.md','docs/vernacular/rounds/V5-02-X05-C2.md','docs/vernacular/rounds/V5-02-X05-C2-A.md'}
 changed=[n for n in old if old[n]!=now.get(n)]
 assert set(changed)<=allowed,changed
 review=json.loads((ROOT/'tests/native/v5-02-x05-c1-review-evidence/artifact-manifest.json').read_text())
 for name,digest in review.items():assert sha(ROOT/name)==digest,name
 additions=set(now)-set(old)
 review_extra={'tests/native/v5-02-x05-c1-review-evidence/artifact-manifest.json','tests/native/v5-02-x05-c1-review-evidence/finalization.json'}
 assert all(n in allowed or n in review or n in review_extra or n.startswith('tests/native/v5-02-x05-c2-a-support/') for n in additions),sorted(additions-set(review)-review_extra-allowed)
 return dict(protected_release_files=len(actual),all_release_bytes_preserved=True,approved_shape_resolver_identical=True,agents_symlink_preserved=True,
   historical_checkpoint=str(checkpoint.relative_to(ROOT)),historical_checkpoint_sha256=sha(checkpoint),prior_checkpoint_entries=len(old),changed_from_checkpoint=changed,
   added_from_checkpoint=sorted(additions),prior_review_artifacts_verified=len(review),historical_membership_bytes_modes_symlinks_preserved=True,
   checkpoint_is_prior_approved_not_contemporaneous_preedit=True)
if __name__=='__main__':
 if sys.argv[1]=='freeze':
  write(EVIDENCE/'frozen-candidate.json',candidate());write(EVIDENCE/'frozen-filesystem.json',filesystem());write(EVIDENCE/'protected-before-gate.json',protected())
  print('Frozen candidate',len(candidate()))
 else:
  print('Verified candidate',verify_candidate());write(EVIDENCE/'protected-final.json',protected())
