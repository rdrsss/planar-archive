"""Serialized focused Debug gate on frozen pure rendering candidate; no B milestone."""
from pathlib import Path
import hashlib,json,subprocess,sys,time,xml.etree.ElementTree as ET
root=Path(__file__).resolve().parents[3];build=root/'build/rewrite-debug';e=root/'tests/native/v5-02-x05-b4-b-c01-evidence';out=e/'final-gate';out.mkdir();commands=[]
def run(label,args):
 t=time.time();p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,text=True);(out/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=list(map(str,args)),cwd=str(root),exit=p.returncode,seconds=time.time()-t));(out/'commands.json').write_text(json.dumps(commands,indent=2)+'\n');print(label,p.returncode,flush=True)
 if p.returncode:raise SystemExit(p.returncode)
 return p.stdout
def verify():
 for file in ['frozen-source.json','protected-start.json','protected-unowned.json']:
  for name,digest in json.loads((e/file).read_text()).items():assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
verify();run('build',['cmake','--build','--preset','debug','--target','all','planar_tests'])
listing=run('inventory',['ctest','--preset','debug','--show-only=json-v1']);(out/'inventory.json').write_text(listing);tests={t['name'] for t in json.loads(listing)['tests']};old={t['name'] for t in json.loads((root/'tests/native/v5-02-x05-b4-b-evidence/final-gate/inventory.json').read_text())['tests']};assert len(old)==674 and old<=tests
new=sorted(tests-old);assert len(new)==2;selected=(root/'tests/native/v5-02-x05-b4-b-evidence/final-gate/selected-tests.txt').read_text().splitlines()+new;(out/'selected-tests.txt').write_text('\n'.join(selected)+'\n')
for label,path,count in [('focused',out/'selected-tests.txt',410),('history-codec',root/'tests/native/v5-02-x05-b3-c-support/history-codec-selected.txt',11)]:
 run(label,['ctest','--test-dir',build,'--tests-from-file',path,'--output-on-failure','--output-junit',out/(label+'.xml')]);(out/(label+'-LastTest.log')).write_bytes((build/'Testing/Temporary/LastTest.log').read_bytes());cases=ET.parse(out/(label+'.xml')).findall('.//testcase');assert len(cases)==count and not any(x.find('skipped')is not None or x.find('failure')is not None or x.find('error')is not None for x in cases)
run('production-render',[build/'src/engine/planar_runtime_tests','[profile-render]','--reporter','junit','--out',out/'production-render.xml'])
for label,path in [('selection','tests/native/v5-02-x05-b3-c-support/vector.py'),('shape-definition','tests/native/v5-02-x05-b2-a-support/vector.py'),('canonical','tests/vectors/generate.py'),('record-v1','tests/vectors/records/generate.py'),('request-v2','tests/vectors/records/generate-request-v2.py'),('effective-provenance','tests/vectors/effective-provenance/generate.py')]:run(label,[sys.executable,path]+([]if label=='shape-definition'else['--check']))
for label,path in [('historical','tests/native/v5-02-b2-support/audit-variants.py'),('d6','tests/native/r14-d6-support/audit.py'),('d6-c01','tests/native/r14-d6-c01-support/audit.py'),('d7-b','tests/native/r14-d7-b-support/audit.py'),('b1','tests/native/v5-02-x05-b1-support/audit.py'),('b2-a','tests/native/v5-02-x05-b2-a-support/audit.py')]:run(label+'-audit',[sys.executable,path,build,out/(label+'-audit.json')])
run('private-probes',[sys.executable,'tests/native/v5-02-x05-b4-b-c01-support/probes.py','final-probes'])
run('allocation-probes',[sys.executable,'tests/native/v5-02-x05-b4-b-c01-support/allocation.py','final-allocation'])
run('support-format',['/opt/homebrew/opt/llvm/bin/clang-format','--dry-run','--Werror','tests/native/v5-02-x05-b4-b-c01-support/semantic.cpp','tests/native/v5-02-x05-b4-b-c01-support/allocation.cpp'])
run('pinned-lint',[sys.executable,'scripts/rewrite-check.py','lint','--build-dir',build]);assert 'ninja: no work to do.'in run('no-work',['cmake','--build','--preset','debug','--target','all','planar_tests']);verify();(out/'frozen-stability.json').write_text(json.dumps(dict(unchanged=True,inventory=len(tests),focused=410,supplemental=11,private_compile=True),indent=2)+'\n')
