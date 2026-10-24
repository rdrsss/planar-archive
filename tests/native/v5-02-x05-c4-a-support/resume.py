"""Final isolated summary-bound correction checks after the preserved422 gate."""
from pathlib import Path
import json,subprocess,sys,time,xml.etree.ElementTree as ET
from freeze import ROOT as R,EVIDENCE as E,verify_candidate,write
B=R/'build/rewrite-debug';O=E/'final-resume';O.mkdir();commands=[]
def run(label,args):
 verify_candidate();start=time.time();p=subprocess.run(list(map(str,args)),cwd=R,capture_output=True,text=True)
 (O/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=list(map(str,args)),cwd=str(R),exit=p.returncode,seconds=time.time()-start));write(O/'commands.json',commands)
 print(label,p.returncode,flush=True);assert p.returncode==0,(label,(p.stdout+p.stderr)[-3000:]);verify_candidate();return p.stdout
run('build',['cmake','--build',B,'--target','all','planar_tests'])
listing=json.loads(run('inventory',['ctest','--test-dir',B,'--show-only=json-v1']));write(O/'inventory.json',listing)
old={t['name'] for t in json.loads((E/'inventory-before.json').read_text())['tests']};now={t['name'] for t in listing['tests']}
assert len(old)==743 and len(now)==756 and old<=now
selected=[t['name'] for t in listing['tests'] if t['name'].startswith(('replacement.C4 A ','replacement.X05 C2 ','replacement.C3 B2 '))]
new=[t for t in selected if t.startswith('replacement.C4 A ')];assert len(new)==13 and len(selected)>30
path=O/'selected-tests.txt';path.write_text('\n'.join(selected)+'\n')
run('focused',['ctest','--test-dir',B,'--tests-from-file',path,'--output-on-failure','--output-junit',O/'focused.xml'])
cases=ET.parse(O/'focused.xml').findall('.//testcase');assert len(cases)==len(selected) and all(c.get('status')=='run' and not any(c.find(t) is not None for t in ['failure','error','skipped']) for c in cases)
for label,script in [('selection-vectors','tests/native/v5-02-x05-b3-c-support/vector.py'),('shape-definition-vectors','tests/native/v5-02-x05-b2-a-support/vector.py'),('canonical-vectors','tests/vectors/generate.py'),('record-v1-vectors','tests/vectors/records/generate.py'),('request-v2-vectors','tests/vectors/records/generate-request-v2.py'),('effective-provenance-vectors','tests/vectors/effective-provenance/generate.py')]:
 run(label,[sys.executable,'-B',script]+([] if label=='shape-definition-vectors' else ['--check']))
for label,script in [('historical','v5-02-b2-support/audit-variants.py'),('d6','r14-d6-support/audit.py'),('d6-c01','r14-d6-c01-support/audit.py'),('d7-b','r14-d7-b-support/audit.py'),('b1','v5-02-x05-b1-support/audit.py')]:
 run(label+'-audit',[sys.executable,'-B',R/'tests/native'/script,B,O/(label+'-audit-report.json')])
run('current-boundary-audit',[sys.executable,'-B',R/'tests/native/v5-02-x05-c4-a-support/audit.py',O/'current-boundary-audit-report.json'])
run('pinned-lint',[sys.executable,'-B','scripts/rewrite-check.py','lint','--build-dir',B])
assert 'ninja: no work to do.' in run('no-work',['cmake','--build',B,'--target','all','planar_tests'])
write(O/'summary.json',dict(inventory=756,preserved=743,new=13,focused=len(selected),failures=0,errors=0,skips=0,disabled=0,vectors=6,audits=6,prior_focused=422,source_delta='summary raw scan resource classification and corresponding assertion only',no_full_gate=True,no_release_run=True))
