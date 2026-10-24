"""Frozen D3 focused Debug verification; commands execute serially."""
from pathlib import Path
import json,subprocess,time,xml.etree.ElementTree as ET
from freeze import R,S,E,write,candidate
commands=[]
def verify():assert candidate()==json.loads((E/'frozen-candidate.json').read_text())
def run(label,args):
 verify();start=time.monotonic();p=subprocess.run(list(map(str,args)),cwd=R,capture_output=True,text=True)
 (E/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=list(map(str,args)),cwd=str(R),exit=p.returncode,seconds=time.monotonic()-start));write(E/'gate-results.json',commands);print(label,p.returncode,flush=True);assert p.returncode==0,(p.stdout+p.stderr)[-6000:];verify();return p.stdout
build=['cmake','--build','build/rewrite-debug','--target','all','planar_tests']
run('final-build',build)
listing=run('inventory-selected',['ctest','--test-dir','build/rewrite-debug','--show-only=json-v1']);(E/'inventory-final.json').write_text(listing)
a=json.loads((R/'tests/native/v5-02-x05-c4-d2-b2-c01-evidence/inventory-final.json').read_text())['tests'];b=json.loads(listing)['tests']
renames={'replacement.C4 C2 general decimal maximum remains Lua integer and history projection is lazy while role stays closed':'replacement.C4 general decimal maximum remains Lua integer and history projection is lazy'}
old={t['name'] for t in a};current={t['name']for t in b};mapped={renames.get(n,n)for n in old};assert len(a)==854 and len(b)==865 and mapped<=current
selected=[t['name']for t in b if Path(t['command'][0]).name in {'planar_runtime_tests','planar_capture_admission_tests','planar_capture_native_tests','planar_capture_profile_tests','planar_capture_profile_codec_tests','planar_capture_host_path_tests','planar_capture_tests','planar_command_tests','planar_lua_loader_tests'} or t['name']=='replacement.cli']
assert len(selected)>500
(E/'selected-tests.txt').write_text('\n'.join(selected)+'\n');write(E/'inventory-preservation.json',dict(baseline=854,current=865,unchanged_names=853,renames=renames,added=sorted(current-mapped),focused=len(selected)))
affected=[name for name in selected if name.startswith('replacement.C4 D3 ') or name=='replacement.C4 A capabilities refuse before instruction entry and closed context exposes no service']
assert len(affected)==12
(E/'affected-tests.txt').write_text('\n'.join(affected)+'\n')
run('corrected-focused',['ctest','--test-dir','build/rewrite-debug','--tests-from-file',E/'affected-tests.txt','--output-on-failure','--output-junit',E/'corrected-focused.xml'])
(E/'corrected-focused-LastTest.log').write_bytes((R/'build/rewrite-debug/Testing/Temporary/LastTest.log').read_bytes())
cases=ET.parse(E/'corrected-focused.xml').findall('.//testcase');assert len(cases)==12 and all(c.get('status')=='run' and not any(c.find(t)is not None for t in ['failure','error','skipped']) for c in cases)
previous=json.loads((E/'attempt1-frozen-gate/frozen-candidate.json').read_text())
now=candidate();product=[n for n in now if n.startswith('src/') and not n.endswith('.t.cpp')]
changed=[n for n in product if previous[n]!=now[n]]
assert changed==['src/engine/runtime/runtime.cpp']
old_source=(E/'attempt1-frozen-gate/candidate-source/src/engine/runtime/runtime.cpp').read_text()
new_source=(R/'src/engine/runtime/runtime.cpp').read_text()
import re
assert re.sub(r' +///< Whether the one original-owner role preparation has started\.', '', new_source)==old_source
write(E/'product-stability-after-fixture-correction.json',dict(unchanged_product=product,only_test_fixture_and_private_member_comment_changed=True,initial_focused=525,initial_passed=524,corrected_focused=12))
for label,script,args in [('selection','tests/native/v5-02-x05-b3-c-support/vector.py',['--check']),('shape-definition','tests/native/v5-02-x05-b2-a-support/vector.py',[]),('canonical','tests/vectors/generate.py',['--check']),('record-v1','tests/vectors/records/generate.py',['--check']),('request-v2','tests/vectors/records/generate-request-v2.py',['--check']),('effective-provenance','tests/vectors/effective-provenance/generate.py',['--check'])]:run(label+'-vectors',['python3','-B',script,*args])
for label,script in [('historical','v5-02-b2-support/audit-variants.py'),('d6','r14-d6-support/audit.py'),('d6-c01','r14-d6-c01-support/audit.py'),('d7-b','r14-d7-b-support/audit.py'),('b1','v5-02-x05-b1-support/audit.py')]:run(label+'-audit',['python3','-B',R/'tests/native'/script,R/'build/rewrite-debug',E/(label+'-audit.json')])
run('retained-native-audit',['python3','-B',R/'tests/native/v5-02-x05-c4-d2-b2-c01-support/audit.py',E/'retained-native-audit.json'])
run('current-runtime-audit',['python3','-B',S/'audit.py',E/'current-runtime-audit.json'])
run('final-lint',['python3','-B','scripts/rewrite-check.py','lint','--build-dir','build/rewrite-debug'])
run('private-final',['python3','-B',S/'private.py'])
rows=ET.parse(E/'private/private.xml').findall('.//testcase');assert len(rows)==5 and all(not any(c.find(t)is not None for t in ['failure','error','skipped']) for c in rows)
run('private-cleanup',['python3','-B',S/'cleanup.py'])
assert 'no work to do' in run('no-work',build)
write(E/'verification-summary.json',dict(initial_focused=525,initial_passed=524,corrected_focused=len(cases),private=5,current_inventory=865,baseline_inventory=854,preserved_all_with_explicit_rename=True,added=11,vectors=6,audits=7,failures=0,errors=0,skips=0,disabled=0,full_gate=False,release_gate=False))
run('protection',['python3','-B',S/'freeze.py','protect']);verify()
