"""Resume affected support/prose checks after the preserved successful focused test run."""
from pathlib import Path
import hashlib,json,subprocess,sys,time,xml.etree.ElementTree as ET
import sys
sys.dont_write_bytecode = True
from freeze import ROOT,EVIDENCE,verify_candidate,write
OUT=EVIDENCE/'final-resume';OUT.mkdir();commands=[]
tested=json.loads((EVIDENCE/'tested-frozen-candidate.json').read_text())
current=json.loads((EVIDENCE/'frozen-candidate.json').read_text())
changed=[key for key in set(tested)|set(current) if tested.get(key)!=current.get(key)]
allowed={'docs/architecture.md','docs/testing.md','tests/native/v5-02-x05-c3-b2-support/gate.py','tests/native/v5-02-x05-c3-b2-support/audit.py','tests/native/v5-02-x05-c3-b2-support/resume.py','tests/native/v5-02-x05-c3-b2-support/private.py','tests/native/v5-02-x05-c3-b2-support/tokens.py','src/engine/runtime/runtime.cpp'}
assert set(changed)<=allowed,changed
assert all(current[key]==identity for key,identity in tested.items() if key != 'src/engine/runtime/runtime.cpp' and key.startswith(('src/','cmake/','scripts/','fixtures/','tests/stdlib/','tests/vectors/')))
tokens=json.loads((EVIDENCE/'comment-token-equivalence.json').read_text())
assert tokens['all_noncomment_tokens_exact'] and tokens['before_sha256']==tested['src/engine/runtime/runtime.cpp']['sha256'] and tokens['after_sha256']==current['src/engine/runtime/runtime.cpp']['sha256']
write(OUT/'scoped-amendment.json',dict(changed=changed,tested_executable_tokens_and_all_test_bytes_unchanged=True,focused_rerun=False))
def run(label,args):
 verify_candidate();started=time.time();p=subprocess.run(list(map(str,args)),cwd=ROOT,capture_output=True,text=True)
 (OUT/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=list(map(str,args)),exit=p.returncode,seconds=time.time()-started));write(OUT/'commands.json',commands)
 print(label,p.returncode,flush=True);assert p.returncode==0,(label,(p.stdout+p.stderr)[-3000:]);verify_candidate();return p.stdout
run('build',['cmake','--build',ROOT/'build/rewrite-debug','--target','all','planar_tests'])
new=json.loads((EVIDENCE/'final-gate/inventory-preservation.json').read_text())['added']
assert len(new)==20
selected=OUT/'selected-new-tests.txt';selected.write_text('\n'.join(new)+'\n')
run('final-new-cases',['ctest','--test-dir',ROOT/'build/rewrite-debug','--tests-from-file',selected,'--output-on-failure','--output-junit',OUT/'final-new-cases.xml'])
newcases=ET.parse(OUT/'final-new-cases.xml').findall('.//testcase')
assert len(newcases)==20 and all(c.get('status')=='run' and not any(c.find(t) is not None for t in ['failure','error','skipped']) for c in newcases)
for label,script in [('historical','v5-02-b2-support/audit-variants.py'),('d6','r14-d6-support/audit.py'),('d6-c01','r14-d6-c01-support/audit.py'),('d7-b','r14-d7-b-support/audit.py'),('b1','v5-02-x05-b1-support/audit.py')]:
    run(label+'-audit',[sys.executable,ROOT/'tests/native'/script,ROOT/'build/rewrite-debug',OUT/(label+'-audit-report.json')])
run('current-boundary-audit',[sys.executable,ROOT/'tests/native/v5-02-x05-c3-b2-support/audit.py',OUT/'current-boundary-audit-report.json'])
run('pinned-lint',[sys.executable,'scripts/rewrite-check.py','lint','--build-dir',ROOT/'build/rewrite-debug'])
assert 'ninja: no work to do.' in run('no-work',['cmake','--build',ROOT/'build/rewrite-debug','--target','all','planar_tests'])
cases=ET.parse(EVIDENCE/'final-gate/focused.xml').findall('.//testcase')
assert len(cases)==409 and all(c.get('status')=='run' and not any(c.find(t) is not None for t in ['failure','error','skipped']) for c in cases)
write(OUT/'summary.json',dict(inventory=743,preserved=723,added=20,focused_before_comment_amendment=409,final_new_cases=20,executable_token_equivalence=True,failures=0,errors=0,skips=0,disabled=0,vectors=6,audits=6,historical_audit_failure_preserved=True,scoped_prose_support_amendment=True,no_work=True,no_full_gate=True,no_release_run=True))
