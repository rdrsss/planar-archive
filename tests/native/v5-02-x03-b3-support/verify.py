"""B3 evidence capture and serialized helper-equivalent complete gates."""
from pathlib import Path
import datetime, hashlib, json, os, shutil, subprocess, sys, time
import xml.etree.ElementTree as ET
ROOT=Path(__file__).resolve().parents[3]
OUT=ROOT/'tests/native/v5-02-x03-b3-evidence'
SUP=ROOT/'tests/native/v5-02-x03-b3-support'
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def write(n,v): (OUT/n).write_text(json.dumps(v,indent=2,sort_keys=True)+'\n')
def snapshot():
 files={};links={}
 for base,dirs,names in os.walk(ROOT,followlinks=False):
  p=Path(base)
  if p==ROOT: dirs[:]=[d for d in dirs if d not in ('.git','build')]
  dirs[:]=[d for d in dirs if p/d not in (OUT,SUP)]
  for name in dirs[:]+names:
   f=p/name;rel=str(f.relative_to(ROOT))
   if f.is_symlink(): links[rel]=os.readlink(f)
   elif f.is_file(): files[rel]=sha(f)
 return dict(path_base=str(ROOT),exclusions=['root .git/','root build/','tests/native/v5-02-x03-b3-evidence/','tests/native/v5-02-x03-b3-support/'],files=files,symlinks=links)
def run(label,argv):
 start=datetime.datetime.now(datetime.timezone.utc).isoformat();t=time.time()
 with (OUT/(label+'.log')).open('wb') as log:
  result=subprocess.run(list(map(str,argv)),cwd=ROOT,stdout=log,stderr=subprocess.STDOUT)
 write(label+'.json',dict(argv=list(map(str,argv)),cwd=str(ROOT),start_utc=start,end_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),seconds=time.time()-t,exit_code=result.returncode))
 print(label,result.returncode,flush=True)
 if result.returncode: raise RuntimeError(label+' failed; stop for review')
 return (OUT/(label+'.log')).read_text(errors='replace')
def baseline():
 write('baseline-complete-filesystem.json',snapshot())
 for label,args in [('baseline-head',['git','rev-parse','HEAD']),('baseline-branch',['git','branch','--show-current']),('baseline-status',['git','status','--short']),('baseline-diff',['git','diff','--binary'])]:run(label,args)
 write('baseline-build-identities.json',{str(p.relative_to(ROOT)):sha(p) for b in ('rewrite-debug','rewrite-release') for p in (ROOT/'build'/b).rglob('*') if p.is_file() and not p.is_symlink() and (p.suffix=='.t' or p.name in ('planar','CMakeCache.txt','compile_commands.json'))})
 print('baseline regular files',len(json.loads((OUT/'baseline-complete-filesystem.json').read_text())['files']),flush=True)
def gate(preset):
 build=ROOT/'build'/('rewrite-'+preset)
 run(preset+'-configure',['cmake','--preset',preset])
 run(preset+'-build',['cmake','--build',build,'--target','all','planar_tests'])
 listing=run(preset+'-inventory',['ctest','--test-dir',build,'--show-only=json-v1'])
 inventory=json.loads(listing)['tests'];names=[t['name'] for t in inventory]
 assert len(names)==430 and len(set(names))==430
 # Compare the exact independently approved inventory, not only its size.
 prior=json.loads((ROOT/'tests/native/v5-02-x03-b2-b-c01-review-evidence/inventory.log').read_text())
 assert names==[t['name'] for t in prior['tests']]
 run(preset+'-full',['ctest','--test-dir',build,'--output-on-failure','--output-junit',OUT/(preset+'-full.xml')])
 shutil.copyfile(build/'Testing/Temporary/LastTest.log',OUT/(preset+'-LastTest.log'))
 shutil.copyfile(build/'Testing/Temporary/CTestCostData.txt',OUT/(preset+'-CTestCostData.txt'))
 cases=ET.parse(OUT/(preset+'-full.xml')).findall('.//testcase')
 assert len(cases)==len(names)>0
 assert sorted(c.get('name') for c in cases)==sorted(names)
 assert all(c.get('status')=='run' and not any(c.find(tag) is not None for tag in ('skipped','failure','error')) for c in cases)
 for suite in ET.parse(OUT/(preset+'-full.xml')).getroot().iter('testsuite'):
  assert all(int(suite.get(key,'0'))==0 for key in ('failures','errors','skipped','disabled'))
 write(preset+'-test-summary.json',dict(inventory_count=len(names),executed=len(cases),failures=0,errors=0,skips=0,all_status_run=True,approved_430_names_exact=True,names=names))
 run(preset+'-lint',[sys.executable,'scripts/rewrite-check.py','lint','--build-dir',build])
 output=run(preset+'-no-work',['cmake','--build',build,'--target','all','planar_tests'])
 assert 'ninja: no work to do.' in output
 write(preset+'-build-identities.json',{str(p.relative_to(ROOT)):sha(p) for p in build.rglob('*') if p.is_file() and not p.is_symlink() and (os.access(p,os.X_OK) or p.name in ('CMakeCache.txt','compile_commands.json','build.ninja'))})
 current=snapshot();base=json.loads((OUT/'baseline-complete-filesystem.json').read_text())
 assert current==base, 'Non-build source changed during gate'
 write(preset+'-source-stability.json',dict(unchanged=True,baseline_manifest_sha256=sha(OUT/'baseline-complete-filesystem.json'),regular_files=len(base['files']),symlinks=base['symlinks']))
 print(preset,'430 passed; source stable; no work',flush=True)
if __name__=='__main__':
 if sys.argv[1]=='baseline':baseline()
 else:gate(sys.argv[1])
