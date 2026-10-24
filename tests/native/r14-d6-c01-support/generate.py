"""Exact private D6 source with dedicated identities; no product hooks."""
from pathlib import Path
import subprocess,sys,tempfile
root,out=map(Path,sys.argv[1:]);out.mkdir(parents=True,exist_ok=True)
with tempfile.TemporaryDirectory(prefix="planar-c01-generate-") as d:
 subprocess.run([sys.executable,str(root/'tests/native/r14-d6-support/generate.py'),str(root),d],check=True)
 for name in ['runtime_0.cpp','runtime_0.cppm','commands_0.lua']:
  original=(Path(d)/name).read_text()
  data=original.replace('planar.engine.load_test_0','planar.engine.load_c01_test').replace('planar.engine.load_source_0','planar.engine.load_c01_source').replace('namespace planar::engine::load_test_0 {','namespace planar::engine::load_c01_test {')
  restored=data.replace('planar.engine.load_c01_test','planar.engine.load_test_0').replace('planar.engine.load_c01_source','planar.engine.load_source_0').replace('namespace planar::engine::load_c01_test {','namespace planar::engine::load_test_0 {')
  assert restored==original,name
  (out/name).write_text(data)
print('Generated exact C01 private runtime identities')
