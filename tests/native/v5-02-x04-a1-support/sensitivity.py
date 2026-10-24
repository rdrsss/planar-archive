"""Reversible omissions prove syscall tests detect reads and an absent pre-read binding.
These are controlled negative controls, not claims about an older production bug.
Run only under exclusive source/build ownership. Restore exact source in finally.
"""
from pathlib import Path
import hashlib,json,subprocess
ROOT=Path(__file__).resolve().parents[3]
OUT=ROOT/'tests/native/v5-02-x04-a1-evidence'
BUILD=ROOT/'build/rewrite-debug'
rows=[]
variants=[
 ('missing-binding',ROOT/'src/engine/capture/native.cpp','    if (admission) {','    if (false && admission) {',
  'replacement.capture admission selected inputs perform zero observed data reads before binding'),
 ('metadata-read-attempt',ROOT/'src/engine/capture/admission.cpp','      auto metadata = status(fd.fd);',
  '      auto metadata = status(fd.fd);\n      if (!is_directory) { char byte; (void)::read(fd.fd, &byte, 1); }',
  'replacement.capture admission all selected file roles have zero observed metadata phase data reads'),
]
def run(label,args):
 with (OUT/(label+'.log')).open('w') as f:
  return subprocess.run(args,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT).returncode
for label,path,before,after,test in variants:
 original=path.read_bytes();text=original.decode();assert text.count(before)==1
 variant=text.replace(before,after).encode()
 try:
  path.write_bytes(variant)
  code=run(label+'-build',['cmake','--build',str(BUILD),'--target','planar_capture_admission_tests'])
  assert code==0,(label,'build',code)
  code=run(label+'-test',['ctest','--test-dir',str(BUILD),'-R','^'+test+'$','--output-on-failure'])
  assert code==8,(label,'expected nonzero regression failure',code)
  rows.append(dict(label=label,path=str(path.relative_to(ROOT)),before=before,after=after,
     original_sha256=hashlib.sha256(original).hexdigest(),variant_sha256=hashlib.sha256(variant).hexdigest(),
     test=test,expected_failure_exit=code))
 finally:
  path.write_bytes(original)
 assert path.read_bytes()==original
code=run('negative-controls-restored-build',['cmake','--build',str(BUILD),'--target','planar_capture_admission_tests'])
assert code==0
(OUT/'sensitivity.json').write_text(json.dumps(dict(purpose=__doc__,exact_restoration=True,variants=rows),indent=2)+'\n')
print('Both controlled omissions failed their exact nonzero test; exact sources restored.')
