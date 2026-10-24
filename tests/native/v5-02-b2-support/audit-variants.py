"""Regenerate verification-only native variants into scratch and compare every byte."""
from pathlib import Path
import hashlib,json,subprocess,sys,tempfile
root=Path(__file__).resolve().parents[3];build=Path(sys.argv[1]).resolve();report=Path(sys.argv[2])
rows=[('r12c2','src/engine/ledger','src/engine/r12c2-generated'),('r13b1','src/engine/ledger','src/engine/r13b1-generated'),('r14c3b2','src/engine/ledger','src/engine/r14c3b2-generated'),('r14f1','src/engine/export','src/engine/r14f1-generated'),('r14f2','src/cmd/planar','src/cmd/planar/r14f2-generated'),('v5-02-b1','.', 'src/engine/v5-02-b1-generated'),('v5-02-b2','.', 'src/cmd/planar/v5-02-b2-generated')]
audit={}
with tempfile.TemporaryDirectory(prefix='planar-b2-audit-') as temporary:
 for label,source,generated in rows:
  target=Path(temporary)/label
  script=root/'tests/native'/f'{label}-support/generate.py'
  p=subprocess.run([sys.executable,str(script),str(root/source),str(target)],check=True,capture_output=True,text=True)
  files={}
  for file in target.rglob('*'):
   if not file.is_file():continue
   relative=file.relative_to(target);actual=build/generated/relative
   assert actual.is_file() and actual.read_bytes()==file.read_bytes(),(label,relative)
   files[str(relative)]=hashlib.sha256(file.read_bytes()).hexdigest()
  actual_files={str(p.relative_to(build/generated)) for p in (build/generated).rglob('*') if p.is_file()}
  assert set(files)==actual_files,(label,'unexpected generated files',actual_files-set(files))
  audit[label]={'count':len(files),'files':files,'generator_stdout':p.stdout}
 for name,identity in [('runtime_test.cpp','implementation'),('runtime_test.cppm','interface')]:
  source='runtime.cpp' if identity=='implementation' else 'runtime.cppm'
  original=(root/'src/engine/runtime'/source).read_text()
  actual=(build/'src/engine'/name).read_text().replace('module planar.engine.runtime_test;','module planar.engine.runtime;').replace('import planar.engine.runtime_test_source;','import planar.engine.runtime_source;').replace('namespace planar::engine::runtime_test {','namespace planar::engine {')
  assert actual==original,name
 audit['fixed-export-harness']={'exact_runtime_implementation_and_interface':True}
report.write_text(json.dumps(audit,indent=2)+'\n')
print('Audited all seven generated variant families and the fixed-export exact-source harness')
