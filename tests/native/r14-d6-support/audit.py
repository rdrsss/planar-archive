"""Compare private exact native variants and decoded embedded originals/assembly."""
from pathlib import Path
import hashlib,json,re,subprocess,sys,tempfile
root=Path(__file__).resolve().parents[3];build=Path(sys.argv[1]).resolve();report=Path(sys.argv[2])
sha=lambda b:hashlib.sha256(b).hexdigest()
rows={}
with tempfile.TemporaryDirectory(prefix='planar-d6-audit-') as d:
 out=Path(d)
 subprocess.run([sys.executable,str(root/'tests/native/r14-d6-support/generate.py'),str(root),str(out)],check=True)
 for i in range(6):
  files={}
  for name in [f'runtime_{i}.cpp',f'runtime_{i}.cppm',f'commands_{i}.lua']:
   expected=(out/name).read_bytes();actual=(build/'src/engine/r14-d6-generated'/name).read_bytes()
   assert actual==expected,name
   files[name]=sha(actual)
  embedded=(build/f'src/engine/planar_load_source_{i}.cppm').read_text()
  arrays={name:bytes(int(x,16) for x in re.findall(r'0x([0-9a-f]{2})',body)) for name,body in re.findall(r'constexpr unsigned char (\w+)\[\] = \{([^}]*)\};',embedded)}
  originals=re.findall(r'\{"([^"]+)", \{reinterpret_cast<const char\*>\((original_\d+)\)',embedded)
  assert len(originals)==10
  assembled=b''
  for logical,name in originals:
   expected=(out/f'commands_{i}.lua').read_bytes() if logical=='planar/commands.lua' else (root/'src/stdlib'/logical).read_bytes()
   assert arrays[name]==expected,(i,logical)
   assembled+=expected+b'\n'
  assert arrays['source_bytes']==assembled,(i,'assembly')
  assert 'std::string_view separator() { return "\\n"; }' in embedded
  rows[str(i)]={'files':files,'logical_originals':[p for p,n in originals],'assembly_sha256':sha(assembled),'embedded_module_sha256':sha(embedded.encode())}
report.write_text(json.dumps({'path_base':'build directory','variants':rows,'production_admission':5,'private_admission':5},indent=2)+'\n')
print('Audited six exact D6 native/interface variants and every logical original/assembly byte')
