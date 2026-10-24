"""Finalize a bounded test-only candidate and release Debug after rehash."""
from pathlib import Path
import ast, difflib, hashlib, json, os, xml.etree.ElementTree as ET
root=Path(__file__).resolve().parents[3]
out=root/'tests/native/v5-02-x05-b3-b-d01-a-evidence'
support=root/'tests/native/v5-02-x05-b3-b-d01-a-support'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def dump(name,data):(out/name).write_text(json.dumps(data,indent=2)+'\n')
def meta(p):return dict(sha256=sha(p),bytes=p.stat().st_size,mode=p.stat().st_mode&0o777)
def current(keys):return {p:sha(root/p) for p in keys}
baseline=json.loads((out/'protected-baseline.json').read_text())
changed={p:dict(before=h,after=sha(root/p)) for p,h in baseline.items() if sha(root/p)!=h}
assert set(changed)<={'build/rewrite-debug/.ninja_log','build/rewrite-debug/Testing/Temporary/LastTest.log'}
compiled={p:h for p,h in baseline.items() if p.startswith('build/') and Path(p).suffix in {'.pcm','.o','.a'}}
now={str(p.relative_to(root)):sha(p) for p in (root/'build/rewrite-debug').rglob('*') if p.is_file() and p.suffix in {'.pcm','.o','.a'}}
assert now==compiled
checkpoint=json.loads((out/'checkpoint-baseline.json').read_text())
assert current(checkpoint)==checkpoint
assert set(checkpoint)=={str(p.relative_to(root)) for p in (root/'tests/native/v5-02-x05-b3-b-evidence').rglob('*') if p.is_file()}
for d in ['src','vendor','cmake','scripts','fixtures']:
 assert {p for p in baseline if p.startswith(d+'/')}=={str(p.relative_to(root)) for p in (root/d).rglob('*') if p.is_file()}
checks={}
for f,count in [('codec-baseline.xml',14),('final-prototype/prototype.xml',8)]:
 suite=ET.parse(out/f).getroot()[0];cases=suite.findall('testcase')
 assert len(cases)==count
 assert all(int(suite.attrib[k])==0 for k in ['failures','errors','skipped'])
 checks[f]=dict(cases=count,assertions=int(suite.attrib['tests']),failures=0,errors=0,skips=0)
inv=json.loads((out/'inventory.log').read_text())['tests']
prior=json.loads((root/'tests/native/v5-02-x05-b3-a-evidence/final-gate/inventory.log').read_text())['tests']
assert len(inv)==637 and len(prior)==627
assert {x['name'] for x in prior}<={x['name'] for x in inv}
assert 'ninja: no work to do.' in (out/'no-work.log').read_text()
assert os.readlink(root/'AGENTS.md')=='CLAUDE.md'
assert (out/'final-prototype/prototype.cpp').read_bytes()==(support/'prototype.cpp').read_bytes()
for p in support.glob('*.py'):ast.parse(p.read_text())
docs=[str(p.relative_to(out/'baseline-docs')) for p in (out/'baseline-docs').rglob('*') if p.is_file()]
paths=sorted(docs+[str(p.relative_to(root)) for p in support.iterdir() if p.is_file()]+[str((out/'REPORT.md').relative_to(root))])
dump('candidate-manifest.json',{p:meta(root/p) for p in paths})
diff=''
for f in paths:
 before=out/'baseline-docs'/f
 diff+=''.join(difflib.unified_diff(before.read_text().splitlines(keepends=True) if before.exists() else [],(root/f).read_text().splitlines(keepends=True),fromfile='a/'+f,tofile='b/'+f))
(out/'candidate.diff').write_text(diff)
libcpp=Path('/opt/homebrew/opt/llvm/include/c++/v1')
dump('pinned-library-identities.json',{str(p):meta(p) for p in [libcpp/'string',libcpp/'vector',libcpp/'__tree',libcpp/'__memory/shared_ptr.h',libcpp/'__memory/allocator.h',root/'vendor/glaze/a4a7/include/glaze/json/read.hpp',root/'vendor/glaze/a4a7/include/glaze/json/skip.hpp',root/'vendor/glaze/a4a7/include/glaze/core/read.hpp',root/'vendor/glaze/a4a7/include/glaze/core/context.hpp',root/'vendor/glaze/a4a7/include/glaze/util/parse.hpp']})
dump('checks.json',dict(checks=checks,inventory=637,preserved_approved_names=627,compiled_members=len(compiled),compiled_membership_and_hashes_unchanged=True,protected_entries=len(baseline),operational_log_changes=changed,checkpoint_members=len(checkpoint),checkpoint_unchanged=True,full_gate=False,production_api=False,independent_review='pending'))
exclude={'artifact-manifest.json','finalization.json'}
dump('artifact-manifest.json',{str(p.relative_to(out)):meta(p) for p in sorted(out.rglob('*')) if p.is_file() and str(p.relative_to(out)) not in exclude})
assert current({p:h for p,h in baseline.items() if p not in changed})=={p:h for p,h in baseline.items() if p not in changed}
assert current(checkpoint)==checkpoint
assert all(meta(root/p)==m for p,m in json.loads((out/'candidate-manifest.json').read_text()).items())
dump('finalization.json',dict(status='test-only feasibility candidate; independent review pending',debug_ownership_released=True,no_further_edits=True,protected_rehash_equal_except_recorded_logs=True,compiled_rehash_equal=True,checkpoint_rehash_equal=True,candidate_manifest_sha256=sha(out/'candidate-manifest.json'),candidate_diff_sha256=sha(out/'candidate.diff'),artifact_manifest_sha256=sha(out/'artifact-manifest.json'),report_sha256=sha(out/'REPORT.md')))
print((out/'finalization.json').read_text())
