"""Frozen focused Debug gate, with serialized commands and isolated copied-dependency probes."""
from pathlib import Path
import json,subprocess,time,xml.etree.ElementTree as ET
from freeze import R,S,E,write,candidate
commands=[]
def verify():assert candidate()==json.loads((E/'frozen-candidate.json').read_text())
def run(label,args):
 verify();start=time.monotonic();p=subprocess.run(list(map(str,args)),cwd=R,capture_output=True,text=True)
 (E/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=list(map(str,args)),cwd=str(R),exit=p.returncode,seconds=time.monotonic()-start));write(E/'gate-results.json',commands);print(label,p.returncode,flush=True);assert p.returncode==0,(p.stdout+p.stderr)[-6000:];verify();return p.stdout
build=['cmake','--build','--preset','debug','--target','all','planar_tests']

run('final-build',build)
listing=run('inventory-selected',['ctest','--test-dir','build/rewrite-debug','--show-only=json-v1']);(E/'inventory-final.json').write_text(listing)
a=json.loads((E/'inventory-before.json').read_text())['tests'];b=json.loads(listing)['tests']
assert len(a)==838 and len(b)==851
old={t['name'] for t in a};current={t['name'] for t in b};assert old<=current
selected=[t['name'] for t in b if Path(t['command'][0]).name in {'planar_capture_admission_tests','planar_capture_native_tests','planar_capture_profile_tests','planar_capture_profile_codec_tests','planar_capture_host_path_tests','planar_capture_tests'} or t['name'].startswith('replacement.C3 B2 ')]
assert len(selected)==184
(E/'selected-tests.txt').write_text('\n'.join(selected)+'\n')
write(E/'inventory-preservation.json',dict(baseline=838,current=851,preserved=838,added=sorted(current-old),focused=len(selected)))
run('focused',['ctest','--test-dir','build/rewrite-debug','--tests-from-file',E/'selected-tests.txt','--output-on-failure','--output-junit',E/'focused.xml'])
(E/'focused-LastTest.log').write_bytes((R/'build/rewrite-debug/Testing/Temporary/LastTest.log').read_bytes())
cases=ET.parse(E/'focused.xml').findall('.//testcase');assert len(cases)==184 and all(c.get('status')=='run' and not any(c.find(t)is not None for t in ['failure','error','skipped']) for c in cases)
for label,script,args in [('selection','tests/native/v5-02-x05-b3-c-support/vector.py',['--check']),('shape-definition','tests/native/v5-02-x05-b2-a-support/vector.py',[]),('canonical','tests/vectors/generate.py',['--check']),('record-v1','tests/vectors/records/generate.py',['--check']),('request-v2','tests/vectors/records/generate-request-v2.py',['--check']),('effective-provenance','tests/vectors/effective-provenance/generate.py',['--check'])]:run(label+'-vectors',['python3','-B',script,*args])
for label,script in [('historical','v5-02-b2-support/audit-variants.py'),('d6','r14-d6-support/audit.py'),('d6-c01','r14-d6-c01-support/audit.py'),('d7-b','r14-d7-b-support/audit.py'),('b1','v5-02-x05-b1-support/audit.py')]:run(label+'-audit',['python3','-B',R/'tests/native'/script,R/'build/rewrite-debug',E/(label+'-audit.json')])
run('approved-runtime-audit',['python3','-B',R/'tests/native/v5-02-x05-c4-c2-support/audit.py',E/'approved-runtime-audit.json'])
run('current-native-audit',['python3','-B',S/'audit.py',E/'current-native-audit.json'])
run('final-lint',['python3','-B','scripts/rewrite-check.py','lint','--build-dir','build/rewrite-debug'])
run('private-final',['python3','-B',S/'private.py'])
rows=ET.parse(E/'private/probe.xml').findall('.//testcase');assert len(rows)==6 and all(not any(c.find(t)is not None for t in ['failure','error','skipped']) for c in rows)
run('private-cleanup',['python3','-B',S/'cleanup.py'])
assert 'no work to do' in run('no-work',build)
write(E/'verification-summary.json',dict(focused=184,private=6,current_inventory=851,baseline_inventory=838,preserved_all=True,added=13,vectors=6,audits=7,failures=0,errors=0,skips=0,disabled=0,full_gate=False,release_gate=False))
run('protection',['python3','-B',S/'freeze.py','protect']);verify()

write(E/'bookkeeping-identity.json',dict(owned_product_test_prose_unchanged=candidate()==json.loads((E/'frozen-candidate.json').read_text())))
