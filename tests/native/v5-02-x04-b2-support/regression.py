"""Serialized source-frozen focused B2 verification; unique attempt directories."""
from pathlib import Path
import datetime,hashlib,json,shutil,subprocess,sys,xml.etree.ElementTree as ET
from protect import ROOT,OUT,filesystem,sha
attempt=OUT/sys.argv[1];attempt.mkdir(exist_ok=False)
frozen=filesystem()
def write(p,v):p.write_text(json.dumps(v,indent=2,sort_keys=True)+'\n')
write(attempt/'source-freeze.json',frozen)
for name,row in frozen.items():
 if name.startswith(('src/','cmake/','scripts/','tests/native/v5-02-x04-b2-support/')) or name in ['CMakeLists.txt','CMakePresets.json','Makefile']:
  target=attempt/'candidate-source'/name;target.parent.mkdir(parents=True,exist_ok=True)
  if 'symlink' in row:target.symlink_to(row['symlink'])
  else:shutil.copy2(ROOT/name,target)
rows=[]
def run(label,args):
 assert filesystem()==frozen,'source changed before '+label
 start=datetime.datetime.now(datetime.timezone.utc)
 with (attempt/(label+'.log')).open('w') as f:r=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=f,stderr=subprocess.STDOUT)
 rows.append(dict(label=label,argv=list(map(str,args)),start=start.isoformat(),end=datetime.datetime.now(datetime.timezone.utc).isoformat(),exit_code=r.returncode));write(attempt/'commands.json',rows)
 print(label,r.returncode,flush=True)
 assert filesystem()==frozen,'source changed after '+label
 if r.returncode:raise SystemExit(label+' failed')
 return (attempt/(label+'.log')).read_text()
build=ROOT/'build/rewrite-debug'
run('build',['cmake','--build',build,'--target','all','planar_tests'])
run('classification-regression',['ctest','--test-dir',build,'-R','effective catalog closed schemas','--output-on-failure','--output-junit',attempt/'regression.xml'])
write(attempt/'stability.json',dict(unchanged=True,freeze_sha256=sha(attempt/'source-freeze.json')))
write(attempt/'manifest.json',{str(p.relative_to(attempt)):sha(p) for p in attempt.rglob('*') if p.is_file() and p.name!='manifest.json'})
