"""Frozen focused Debug gate; normal and generated runtimes, existing contract vectors and private probes."""
from pathlib import Path
import json,subprocess,time,xml.etree.ElementTree as ET
from freeze import R,S,E,entry,write,candidate
commands=[]
def verify():assert candidate()==json.loads((E/'frozen-candidate.json').read_text())
def run(label,args):
 verify();start=time.monotonic();p=subprocess.run(list(map(str,args)),cwd=R,capture_output=True,text=True)
 (E/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=list(map(str,args)),cwd=str(R),exit=p.returncode,seconds=time.monotonic()-start));write(E/'gate-results.json',commands);print(label,p.returncode,flush=True);assert p.returncode==0,(p.stdout+p.stderr)[-6000:];verify();return p.stdout
build=['cmake','--build','build/rewrite-debug','--target','planar','planar_runtime_tests','planar_ledger_format_tests','planar_ledger_sqlite_tests','planar_history_read_tests','planar_history_fixture','planar_load_cli_probe',*[f'planar_cli_variant_{i}' for i in range(6)],'-j','2']
run('final-build',build)
listing=run('inventory-selected',['ctest','--test-dir','build/rewrite-debug','--show-only=json-v1']);inventory=json.loads(listing)['tests'];counts={};selected=[]
for target,expected in {'planar_runtime_tests':287,'planar_history_read_tests':3}.items():
 rows=[t['name'] for t in inventory if Path(t['command'][0]).name==target];assert len(rows)==expected,(target,len(rows));counts[target]=len(rows);selected+=rows
format_new=[t['name'] for t in inventory if Path(t['command'][0]).name=='planar_ledger_format_tests' and 'C4 B2' in t['name']];assert len(format_new)==2;selected+=format_new
cli=[t['name'] for t in inventory if Path(t['command'][0]).name.startswith('python')];assert len(cli)==7;selected+=cli
(E/'selected-tests.txt').write_text('\n'.join(selected)+'\n')
run('focused-ctest',['ctest','--test-dir','build/rewrite-debug','--tests-from-file',E/'selected-tests.txt','--output-on-failure','--output-junit',E/'focused-ctest.xml'])
cases=ET.parse(E/'focused-ctest.xml').findall('.//testcase');assert len(cases)==299 and all(not any(c.find(t) is not None for t in ['failure','error','skipped']) for c in cases)
for label,script,args in [('selection','tests/native/v5-02-x05-b3-c-support/vector.py',['--check']),('shape-definition','tests/native/v5-02-x05-b2-a-support/vector.py',[]),('canonical','tests/vectors/generate.py',['--check']),('record-v1','tests/vectors/records/generate.py',['--check']),('request-v2','tests/vectors/records/generate-request-v2.py',['--check']),('effective-provenance','tests/vectors/effective-provenance/generate.py',['--check'])]:run(label+'-vectors',['python3','-B',script,*args])
for label,script in [('historical','v5-02-b2-support/audit-variants.py'),('d6','r14-d6-support/audit.py'),('d6-c01','r14-d6-c01-support/audit.py'),('d7-b','r14-d7-b-support/audit.py'),('b1','v5-02-x05-b1-support/audit.py')]:run(label+'-audit',['python3','-B',R/'tests/native'/script,R/'build/rewrite-debug',E/(label+'-audit.json')])
run('current-native-audit',['python3','-B',S/'audit.py',E/'current-native-audit.json'])
run('final-lint',['python3','-B','scripts/rewrite-check.py','lint','--build-dir','build/rewrite-debug'])
run('private-final',['python3','-B',S/'private.py'])
for name,n in [('green-header',1),('green-native',6)]:
 cases=ET.parse(E/'private'/(name+'.xml')).findall('.//testcase');assert len(cases)==n and all(not any(c.find(t) is not None for t in ['failure','error','skipped']) for c in cases)
assert 'no work to do' in run('no-work',build)
listing=run('inventory-final',['ctest','--test-dir','build/rewrite-debug','--show-only=json-v1']);(E/'inventory-final.json').write_text(listing)
a=json.loads((E/'baseline-inventory.json').read_text())['tests'];b=json.loads(listing)['tests'];assert len(a)==767 and len(b)==773;assert {x['name'] for x in a}<={x['name'] for x in b}
write(E/'verification-summary.json',dict(focused_cases=counts,cli_cases=7,total=299,private_green_cases=7,header_red_failures=1,original_inventory=767,current_inventory=773,preserved_all=True,skips=0))
run('protection',['python3','-B',S/'freeze.py','protect']);verify()
