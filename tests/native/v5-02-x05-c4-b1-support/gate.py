"""Serialized focused Debug verification of a frozen candidate; no full/release gate."""
from pathlib import Path
import json,subprocess,time,xml.etree.ElementTree as ET
from freeze import R,S,E,verify,write,protection,entry
commands=[]
def run(label,args):
 verify();start=time.monotonic();p=subprocess.run(list(map(str,args)),cwd=R,capture_output=True,text=True)
 (E/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=list(map(str,args)),cwd=str(R),exit=p.returncode,seconds=time.monotonic()-start));write(E/'gate-results.json',commands);print(label,p.returncode,flush=True);assert p.returncode==0,(p.stdout+p.stderr)[-4000:];verify();return p.stdout
build=['cmake','--build','build/rewrite-debug','--target','planar_ledger_sqlite_tests','planar_ledger_format_tests','planar_history_read_tests','-j','2']
run('final-build',build)
listing=run('inventory-selected',['ctest','--test-dir','build/rewrite-debug','--show-only=json-v1'])
inventory=json.loads(listing)['tests'];counts={}
targets={'planar_ledger_sqlite_tests':33,'planar_ledger_format_tests':80,'planar_history_read_tests':3}
selected=[]
for target,expected in targets.items():
 rows=[t['name'] for t in inventory if Path(t['command'][0]).name==target]
 assert len(rows)==expected,(target,len(rows));counts[target]=len(rows);selected+=rows
(E/'selected-tests.txt').write_text('\n'.join(selected)+'\n')
run('focused-ctest',['ctest','--test-dir','build/rewrite-debug','--tests-from-file',E/'selected-tests.txt','--output-on-failure','--output-junit',E/'focused-ctest.xml'])
cases=ET.parse(E/'focused-ctest.xml').findall('.//testcase');assert len(cases)==116 and all(not any(c.find(t) is not None for t in ['failure','error','skipped']) for c in cases)
run('private-final',['python3','-B',S/'private.py'])
for name,n in [('green-backup',1),('green-direct',2),('green-receipt',1),('green-new-alloc',1),('green-open-alloc',1)]:
 cases=ET.parse(E/'private'/(name+'.xml')).findall('.//testcase');assert len(cases)==n and all(not any(c.find(t) is not None for t in ['failure','error','skipped']) for c in cases)
run('final-lint',['python3','-B','scripts/rewrite-check.py','lint','--build-dir','build/rewrite-debug'])
assert 'no work to do' in run('no-work',build)
listing=run('inventory-final',['ctest','--test-dir','build/rewrite-debug','--show-only=json-v1']);(E/'inventory-final.json').write_text(listing)
a=json.loads((E/'inventory-before.json').read_text())['tests'];b=json.loads(listing)['tests'];assert len(a)==756 and len(b)==767;assert {x['name'] for x in a}<={x['name'] for x in b}
write(E/'verification-summary.json',dict(focused_cases=counts,total=116,private_green_cases=6,original_inventory=756,current_inventory=767,preserved_all=True,skips=0))
protection();verify()
files={str(p.relative_to(R)):entry(p) for p in E.rglob('*') if p.name not in {'artifact-manifest.json','finalization.json'}}
write(E/'artifact-manifest.json',dict(schema=1,entries=files))
write(E/'finalization.json',dict(schema=1,manifest=entry(E/'artifact-manifest.json'),candidate=entry(E/'frozen-candidate.json'),report=entry(E/'REPORT.md'),artifact_entries=len(files),candidate_entries=len(json.loads((E/'frozen-candidate.json').read_text())['entries'])))
