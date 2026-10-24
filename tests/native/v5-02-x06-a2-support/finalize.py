from pathlib import Path
import json,hashlib,xml.etree.ElementTree as ET,re
R=Path(__file__).resolve().parents[3];O=R/'tests/native/v5-02-x06-a2-evidence';a=json.loads((O/'baseline.json').read_text());b=json.loads((O/'final.json').read_text());candidate=json.loads((O/'candidate.json').read_text());allowed=set(candidate['files'])
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def mutable(p):return p in allowed or p.startswith('build/rewrite-debug/') or p=='build/rewrite-debug'
errors=[];changed=[];metadata_dirs=[]
for p in sorted(set(a)|set(b)):
 if mutable(p):
  if a.get(p)!=b.get(p):changed.append(p)
  continue
 if p not in a or p not in b:errors.append(p);continue
 x,y=a[p],b[p]
 if x['type']=='directory' and y['type']=='directory':
  if any(x[k]!=y[k] for k in ['mode','device','inode','type']):errors.append(p)
  elif x!=y:metadata_dirs.append(p)
 elif x!=y:errors.append(p)
assert not errors,errors
for p,h in candidate['files'].items():assert sha(R/p)==h,p
for p in (O/'preedit').rglob('*'):
 if p.is_file():assert sha(p)==a[p.relative_to(O/'preedit').as_posix()]['sha256'],p
assert candidate['diff_sha256']==sha(O/'candidate.diff')
t=ET.parse(O/'focused-tests.xml');cases=t.findall('.//testcase');assert len(cases)==106;assert not t.findall('.//failure') and not t.findall('.//error') and not t.findall('.//skipped')
assert all(c.get('status') not in ('notrun','disabled') for c in cases)
added=set(json.loads((O/'inventory-preservation.json').read_text())['added']);new=[c for c in cases if c.get('name') in added];assert len(new)==18
assertions=sum(int(re.search(r'All tests passed \((\d+) assertions?',c.findtext('system-out') or '').group(1)) for c in new)
assert 'ninja: no work to do.' in (O/'no-work.log').read_text()
result=dict(baseline_members=len(a),final_members=len(b),unexpected_changes=errors,allowed_changed_paths=changed,directory_metadata_changes=metadata_dirs,directory_membership_verified=True,release_exact_all_attributes=True,prior_evidence_exact_all_attributes=True,root_dotfiles_and_symlinks_protected=True,focused_cases=106,new_cases=18,new_assertions=assertions,baseline_sha256=sha(O/'baseline.json'),final_sha256=sha(O/'final.json'),candidate_sha256=sha(O/'candidate.json'),candidate_diff_sha256=sha(O/'candidate.diff'))
(O/'protection-result.json').write_text(json.dumps(result,indent=2,sort_keys=True)+'\n')
artifacts={p.relative_to(O).as_posix():dict(sha256=sha(p),size=p.stat().st_size) for p in sorted(O.rglob('*')) if p.is_file() and p.name!='artifact-manifest.json'}
support={p.relative_to(R).as_posix():dict(sha256=sha(p),size=p.stat().st_size) for p in sorted((R/'tests/native/v5-02-x06-a2-support').rglob('*')) if p.is_file()}
(O/'artifact-manifest.json').write_text(json.dumps(dict(candidate=candidate,evidence=artifacts,support=support),indent=2,sort_keys=True)+'\n')
print(json.dumps(dict(protected=True,baseline_members=len(a),final_members=len(b),evidence_files=len(artifacts),new_assertions=assertions,candidate_sha256=sha(O/'candidate.json'),diff_sha256=sha(O/'candidate.diff'),report_sha256=sha(O/'REPORT.md'),manifest_sha256=sha(O/'artifact-manifest.json')),indent=2))
