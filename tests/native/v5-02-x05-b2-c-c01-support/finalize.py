"""Verify freeze and produce complete scoped candidate and evidence identities."""
from pathlib import Path
import json,hashlib,os,difflib,xml.etree.ElementTree as ET
root=Path(__file__).resolve().parents[3];out=root/'tests/native/v5-02-x05-b2-c-c01-evidence'
def dump(path,obj):path.write_text(json.dumps(obj,indent=2)+'\n')
def meta(p):return dict(sha256=hashlib.sha256(p.read_bytes()).hexdigest(),bytes=p.stat().st_size,mode=p.stat().st_mode&0o777)
def identity(p):return {'symlink':os.readlink(p)} if p.is_symlink() else {'sha256':meta(p)['sha256'],'mode':meta(p)['mode']}
freeze=json.loads((out/'final-gate/source-freeze.json').read_text());assert all(identity(root/p)==v for p,v in freeze.items());assert os.readlink(root/'AGENTS.md')=='CLAUDE.md'
paths=[str(p.relative_to(out/'baseline')) for p in (out/'baseline').rglob('*') if p.is_file()]
dump(out/'baseline-manifest.json',{p:meta(out/'baseline'/p) for p in sorted(paths)})
paths += [str(p.relative_to(root)) for p in (root/'tests/native/v5-02-x05-b2-c-c01-support').glob('*') if p.is_file()]
paths += ['tests/native/v5-02-x05-b2-c-c01-evidence/REPORT.md']
dump(out/'candidate-manifest.json',{p:meta(root/p) for p in sorted(paths)})
diff=''
for p in sorted(paths):
 prior=out/'baseline'/p;a=prior.read_text().splitlines(keepends=True) if prior.exists() else [];b=(root/p).read_text().splitlines(keepends=True)
 diff+=''.join(difflib.unified_diff(a,b,fromfile='a/'+p,tofile='b/'+p))
(out/'candidate.diff').write_text(diff);(out/'candidate.diff.sha256').write_text(meta(out/'candidate.diff')['sha256']+'  candidate.diff\n')
checks={}
for path,count in [('final-gate/focused.xml',353),('final-gate/history-codec.xml',11),('final-probes/allocation.xml',2)]:
 cases=ET.parse(out/path).findall('.//testcase');assert len(cases)==count
 assert all(not any(c.find(t) is not None for t in ['failure','error','skipped']) for c in cases)
 checks[path]={'cases':count,'failures':0,'errors':0,'skips':0}
dump(out/'finalization.json',{'source_unchanged':True,'frozen_entries':len(freeze),'checks':checks,'candidate_manifest_sha256':meta(out/'candidate-manifest.json')['sha256'],'candidate_diff_sha256':meta(out/'candidate.diff')['sha256']})
excluded={'artifact-manifest.json','finalization-progress.log'}
dump(out/'artifact-manifest.json',{str(p.relative_to(out)):meta(p) for p in sorted(out.rglob('*')) if p.is_file() and str(p.relative_to(out)) not in excluded})
assert all(identity(root/p)==v for p,v in freeze.items())
print('FINALIZED',len(paths),'candidate members;',len(freeze),'unchanged source entries')
print('candidate manifest',meta(out/'candidate-manifest.json')['sha256']);print('candidate diff',meta(out/'candidate.diff')['sha256']);print('artifact manifest',meta(out/'artifact-manifest.json')['sha256']);print('report',meta(out/'REPORT.md')['sha256'])
