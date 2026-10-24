"""One serialized frozen focused Debug gate. No full milestone or release suite."""
from pathlib import Path
import hashlib,json,subprocess,sys,time,xml.etree.ElementTree as ET
sys.dont_write_bytecode=True
from freeze import ROOT,EVIDENCE,verify_candidate,write
BUILD=ROOT/'build/rewrite-debug';OUT=EVIDENCE/'final-gate';OUT.mkdir();commands=[]
def run(label,args):
 verify_candidate();start=time.time();p=subprocess.run(list(map(str,args)),cwd=ROOT,capture_output=True,text=True)
 (OUT/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=list(map(str,args)),cwd=str(ROOT),exit=p.returncode,seconds=time.time()-start));write(OUT/'commands.json',commands)
 print(label,p.returncode,flush=True);assert p.returncode==0,(label,(p.stdout+p.stderr)[-3000:]);verify_candidate();return p.stdout
run('build',['cmake','--build','--preset','debug','--target','all','planar_tests'])
listing=run('inventory',['ctest','--preset','debug','--show-only=json-v1']);write(OUT/'inventory.json',json.loads(listing));tests={t['name']for t in json.loads(listing)['tests']}
old_json=EVIDENCE/'preedit/inventory.json';old={t['name']for t in json.loads(old_json.read_text())['tests']};assert len(old)==682 and old<=tests
new=sorted(tests-old);assert len(new)==9 and all('X05 C2 A' in n for n in new)
selected=(ROOT/'tests/native/v5-02-x05-c1-evidence/final-gate/selected-tests.txt').read_text().splitlines()+new
assert len(selected)==len(set(selected))==425
(OUT/'selected-tests.txt').write_text('\n'.join(selected)+'\n');write(OUT/'inventory-preservation.json',dict(current=len(tests),preserved=len(old),added=new,focused=len(selected)))
for label,path,count in [('focused',OUT/'selected-tests.txt',425),('history-codec',ROOT/'tests/native/v5-02-x05-b3-c-support/history-codec-selected.txt',11)]:
 run(label,['ctest','--test-dir',BUILD,'--tests-from-file',path,'--output-on-failure','--output-junit',OUT/(label+'.xml')]);(OUT/(label+'-LastTest.log')).write_bytes((BUILD/'Testing/Temporary/LastTest.log').read_bytes());cases=ET.parse(OUT/(label+'.xml')).findall('.//testcase');assert len(cases)==count and all(c.get('status')=='run' and not any(c.find(tag)is not None for tag in ['failure','error','skipped']) for c in cases)
run('projection-identity',[BUILD/'src/engine/planar_runtime_tests','[projection-definition]','--reporter','junit','--out',OUT/'projection-identity.xml'])
run('production-render',[BUILD/'src/engine/planar_runtime_tests','[render],[profile-render]','--reporter','junit','--out',OUT/'production-render.xml'])
for label,path in [('selection','tests/native/v5-02-x05-b3-c-support/vector.py'),('shape-definition','tests/native/v5-02-x05-b2-a-support/vector.py'),('canonical','tests/vectors/generate.py'),('record-v1','tests/vectors/records/generate.py'),('request-v2','tests/vectors/records/generate-request-v2.py'),('effective-provenance','tests/vectors/effective-provenance/generate.py')]:run(label,[sys.executable,path]+([]if label=='shape-definition'else['--check']))
for label,path in [('historical','tests/native/v5-02-b2-support/audit-variants.py'),('d6','tests/native/r14-d6-support/audit.py'),('d6-c01','tests/native/r14-d6-c01-support/audit.py'),('d7-b','tests/native/r14-d7-b-support/audit.py'),('b1','tests/native/v5-02-x05-b1-support/audit.py'),('b2-a','tests/native/v5-02-x05-b2-a-support/audit.py')]:run(label+'-audit',[sys.executable,path,BUILD,OUT/(label+'-audit.json')])
run('projection-vector',[sys.executable,'tests/native/v5-02-x05-c2-a-support/vector.py'])
run('private-probes',[sys.executable,'tests/native/v5-02-x05-c2-a-support/probes.py','final-probes'])
run('support-format',['/opt/homebrew/opt/llvm/bin/clang-format','--dry-run','--Werror','tests/native/v5-02-x05-c2-a-support/allocation.cpp'])
run('pinned-lint',[sys.executable,'scripts/rewrite-check.py','lint','--build-dir',BUILD])
assert 'ninja: no work to do.' in run('no-work',['cmake','--build','--preset','debug','--target','all','planar_tests'])
verify_candidate();write(OUT/'summary.json',dict(inventory=len(tests),all_prior_names_preserved=682,focused=425,supplemental=11,failures=0,errors=0,skips=0,disabled=0,common_vectors=6,source_runtime_audits=6,candidate_entries=verify_candidate(),no_full_gate=True,no_release_run=True))
print('FROZEN FOCUSED C2-A GATE PASS',flush=True)
