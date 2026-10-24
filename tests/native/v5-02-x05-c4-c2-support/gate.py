"""Frozen final correction gate; prior439 run remains separately pinned."""
from pathlib import Path
import json,subprocess,time,xml.etree.ElementTree as ET,hashlib
from freeze import R,S,E,write,candidate
commands=[]
def verify():assert candidate()==json.loads((E/'frozen-candidate.json').read_text())
def run(label,args):
 verify();start=time.monotonic();p=subprocess.run(list(map(str,args)),cwd=R,capture_output=True,text=True)
 (E/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=list(map(str,args)),cwd=str(R),exit=p.returncode,seconds=time.monotonic()-start));write(E/'gate-results.json',commands);print(label,p.returncode,flush=True);assert p.returncode==0,(p.stdout+p.stderr)[-6000:];verify();return p.stdout
build=['cmake','--build','build/rewrite-debug','--target','all','planar_tests','-j','2']
run('final-build',build)
listing=run('inventory-selected',['ctest','--test-dir','build/rewrite-debug','--show-only=json-v1']);(E/'inventory-final.json').write_text(listing)
a=json.loads((E/'baseline-inventory.json').read_text())['tests'];b=json.loads(listing)['tests'];renames=json.loads((E/'inventory-renames.json').read_text())
assert len(a)==791 and len(b)==800;assert {renames.get(x['name'],x['name'])for x in a}<={x['name']for x in b}
filters='[c4c2],[projection-definition],[projection-admission],[extension-execution],[c4b3],[c4b3c01],[projection-frame]'
binary=R/'build/rewrite-debug/src/engine/planar_runtime_tests'
listing=run('scoped-runtime-list',[binary,filters,'--list-tests','--reporter','xml']);(E/'scoped-runtime-list.xml').write_text(listing)
listed=ET.fromstring(listing).findall('.//TestCase');assert len(listed)>=30,len(listed)
run('scoped-runtime',[binary,filters,'--reporter','junit','--out',E/'scoped-runtime.xml'])
cases=ET.parse(E/'scoped-runtime.xml').findall('.//testcase');assert len(cases)==len(listed) and all(not any(c.find(t)is not None for t in ['failure','error','skipped'])for c in cases)
for label,script,args in [('selection','tests/native/v5-02-x05-b3-c-support/vector.py',['--check']),('shape-definition','tests/native/v5-02-x05-b2-a-support/vector.py',[]),('canonical','tests/vectors/generate.py',['--check']),('record-v1','tests/vectors/records/generate.py',['--check']),('request-v2','tests/vectors/records/generate-request-v2.py',['--check']),('effective-provenance','tests/vectors/effective-provenance/generate.py',['--check'])]:run(label+'-vectors',['python3','-B',script,*args])
for label,script in [('historical','v5-02-b2-support/audit-variants.py'),('d6','r14-d6-support/audit.py'),('d6-c01','r14-d6-c01-support/audit.py'),('d7-b','r14-d7-b-support/audit.py'),('b1','v5-02-x05-b1-support/audit.py')]:run(label+'-audit',['python3','-B',R/'tests/native'/script,R/'build/rewrite-debug',E/(label+'-audit.json')])
run('current-native-audit',['python3','-B',S/'audit.py',E/'current-native-audit.json'])
run('final-lint',['python3','-B','scripts/rewrite-check.py','lint','--build-dir','build/rewrite-debug'])
run('private-final',['python3','-B',S/'private.py'])
for name,n in [('baseline-layout',1),('green-layout',1),('green-private',9)]:
 rows=ET.parse(E/'private'/(name+'.xml')).findall('.//testcase');assert len(rows)==n and all(not any(c.find(t)is not None for t in ['failure','error','skipped'])for c in rows)
run('private-cleanup',['python3','-B',S/'cleanup.py'])
run('adapter-supplement',['python3','-B',E/'adapter-supplement.py'])
assert 'no work to do' in run('no-work',build)
old=(E/'attempt2-frozen-gate/frozen-candidate.json').read_bytes()
write(E/'verification-summary.json',dict(final_scoped_runtime=len(cases),final_private=9,layout_cases=2,adapter_cases=17,prior_focused_cases=439,prior_focused_candidate_sha256=hashlib.sha256(old).hexdigest(),current_inventory=800,baseline_inventory=791,preserved_all_with_explicit_rename=True,renames=renames,failures=0,errors=0,skips=0,full_gate=False,release_gate=False))
run('protection',['python3','-B',S/'freeze.py','protect']);verify()
