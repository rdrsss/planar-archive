"""Frozen-source focused Debug gate; ordinary builds serialized."""
from pathlib import Path
import hashlib,json,subprocess,sys,time,xml.etree.ElementTree as ET
root=Path(__file__).resolve().parents[3];build=root/'build/rewrite-debug';e=root/'tests/native/v5-02-x05-b3-b-d01-b-evidence';out=e/'final-gate';out.mkdir()
commands=[]
def run(label,args):
 t=time.time();p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,text=True);(out/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=list(map(str,args)),cwd=str(root),exit=p.returncode,seconds=time.time()-t));(out/'commands.json').write_text(json.dumps(commands,indent=2)+'\n');print(label,p.returncode,flush=True)
 if p.returncode:raise SystemExit(p.returncode)
 return p.stdout
frozen=json.loads((e/'frozen-source.json').read_text())
def verify():
 for name,digest in frozen.items():assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
verify()
run('build',['cmake','--build','--preset','debug','--target','all','planar_tests'])
listing=run('inventory',['ctest','--preset','debug','--show-only=json-v1']);(out/'inventory.json').write_text(listing);tests={t['name'] for t in json.loads(listing)['tests']};old={t['name'] for t in json.loads((e/'inventory-before.json').read_text())['tests']};assert old<=tests;assert len(tests)==641
for label,file,expected in [('focused','selected-tests.txt',375),('history-codec','history-codec-selected.txt',11)]:
 p=root/'tests/native/v5-02-x05-b3-b-d01-b-support'/file;selected=p.read_text().splitlines();assert len(selected)==expected and set(selected)<=tests
 run(label,['ctest','--test-dir',build,'--tests-from-file',p,'--output-on-failure','--output-junit',out/(label+'.xml')]);cases=ET.parse(out/(label+'.xml')).findall('.//testcase');assert len(cases)==expected and not any(x.find('skipped') is not None or x.find('failure') is not None or x.find('error') is not None for x in cases)
run('codec',[build/'src/engine/planar_json_codec_tests','--reporter','junit','--out',out/'codec.xml'])
# Same established vector and six source audits as preceding focused gates.
for label,path in [('shape-definition','tests/native/v5-02-x05-b2-a-support/vector.py'),('canonical','tests/vectors/generate.py'),('record-v1','tests/vectors/records/generate.py'),('request-v2','tests/vectors/records/generate-request-v2.py'),('effective-provenance','tests/vectors/effective-provenance/generate.py')]:
 run(label,[sys.executable,path]+([] if label=='shape-definition' else ['--check']))
for label,path in [('historical','tests/native/v5-02-b2-support/audit-variants.py'),('d6','tests/native/r14-d6-support/audit.py'),('d6-c01','tests/native/r14-d6-c01-support/audit.py'),('d7-b','tests/native/r14-d7-b-support/audit.py'),('b1','tests/native/v5-02-x05-b1-support/audit.py'),('b2-a','tests/native/v5-02-x05-b2-a-support/audit.py')]:run(label+'-audit',[sys.executable,path,build,out/(label+'-audit.json')])
run('allocation-probes',[sys.executable,'tests/native/v5-02-x05-b3-b-d01-b-support/run.py','final-probes'])
run('pinned-lint',[sys.executable,'scripts/rewrite-check.py','lint','--build-dir',build])
run('support-format',['/opt/homebrew/opt/llvm/bin/clang-format','--dry-run','--Werror','tests/native/v5-02-x05-b3-b-d01-b-support/probes.cpp'])
assert 'ninja: no work to do.' in run('no-work',['cmake','--build','--preset','debug','--target','all','planar_tests'])
verify();(out/'frozen-stability.json').write_text(json.dumps(dict(entries=len(frozen),unchanged=True),indent=2)+'\n')
