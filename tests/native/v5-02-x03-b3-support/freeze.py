"""Freeze the B3 review candidate after gates and post-verification prose."""
from pathlib import Path
import difflib,hashlib,json,os,shutil
ROOT=Path(__file__).resolve().parents[3];OUT=ROOT/'tests/native/v5-02-x03-b3-evidence';SUP=ROOT/'tests/native/v5-02-x03-b3-support'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def write(n,v):(OUT/n).write_text(json.dumps(v,indent=2,sort_keys=True)+'\n')
base=json.loads((OUT/'baseline-complete-filesystem.json').read_text());files={};links={}
for directory,dirs,names in os.walk(ROOT,followlinks=False):
 p=Path(directory)
 if p==ROOT:dirs[:]=[d for d in dirs if d not in ('.git','build')]
 dirs[:]=[d for d in dirs if p/d not in (OUT,SUP)]
 for name in dirs[:]+names:
  f=p/name;n=str(f.relative_to(ROOT))
  if f.is_symlink():links[n]=os.readlink(f)
  elif f.is_file():files[n]=sha(f)
owned=json.loads((OUT/'owned-status-paths.json').read_text());new='docs/vernacular/x03-format-evidence.md'
assert not(base['files'].keys()-files.keys())
changed={n for n in base['files'] if base['files'][n]!=files[n]};added=files.keys()-base['files'].keys()
assert changed==set(owned),(changed,set(owned))
assert added=={new},added
assert links==base['symlinks'] and links['AGENTS.md']=='CLAUDE.md'
for n in owned:assert sha(OUT/'baseline-owned'/n)==base['files'][n]
for preset in ('debug','release'):
 assert json.loads((OUT/(preset+'-source-stability.json')).read_text())['unchanged']
 for n,h in json.loads((OUT/(preset+'-build-identities.json')).read_text()).items():assert sha(ROOT/n)==h,n
write('final-complete-filesystem.json',dict(path_base=str(ROOT),exclusions=base['exclusions'],files=files,symlinks=links))
paths=owned+[new]+[str(p.relative_to(ROOT)) for p in sorted(SUP.glob('*.py'))]
candidate={n:sha(ROOT/n) for n in paths}
for n in paths:
 p=OUT/'candidate-source'/n;p.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/n,p)
write('candidate-manifest.json',dict(path_base=str(ROOT),files=candidate))
diff=[]
for n in sorted(paths):
 prior=OUT/'baseline-owned'/n
 before=prior.read_text().splitlines(True) if prior.exists() else []
 diff.extend(difflib.unified_diff(before,(ROOT/n).read_text().splitlines(True),fromfile='baseline/'+n if prior.exists() else '/dev/null',tofile='candidate/'+n))
(OUT/'owned.diff').write_text(''.join(diff))
write('ownership-audit.json',dict(baseline_regular_files=len(base['files']),final_regular_files=len(files),changed_existing=sorted(changed),added=sorted(added),symlinks=links,executable_source_unchanged=True,status_prose_finalized_after_both_gates=True,artifact_path_base=str(OUT),candidate_path_base=str(ROOT)))
write('identity.json',dict(repository=str(ROOT),head=(OUT/'baseline-head.log').read_text().strip(),candidate_manifest_sha256=sha(OUT/'candidate-manifest.json'),owned_diff_sha256=sha(OUT/'owned.diff'),baseline_complete_filesystem_sha256=sha(OUT/'baseline-complete-filesystem.json'),final_complete_filesystem_sha256=sha(OUT/'final-complete-filesystem.json'),source_binaries_unchanged_after_verification=True))
artifacts={str(p.relative_to(OUT)):sha(p) for p in sorted(OUT.rglob('*')) if p.is_file() and p.name!='artifact-manifest.json'}
write('artifact-manifest.json',dict(path_base=str(OUT),exclusions=['artifact-manifest.json'],files=artifacts))
print(json.dumps(dict(candidate_members=len(candidate),artifacts=len(artifacts),candidate_manifest_sha256=sha(OUT/'candidate-manifest.json'),artifact_manifest_sha256=sha(OUT/'artifact-manifest.json'),identity_sha256=sha(OUT/'identity.json'),owned_diff_sha256=sha(OUT/'owned.diff')),indent=2))
