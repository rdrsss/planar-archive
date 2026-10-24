"""Verify exact reversible probe graph and original embedded file boundaries/bytes."""
from pathlib import Path
import hashlib,json,re,subprocess,sys,tempfile
root=Path(__file__).resolve().parents[3];build=Path(sys.argv[1]).resolve();report=Path(sys.argv[2]);generated=build/'src/cmd/planar/r14-d7-b-generated';sha=lambda x:hashlib.sha256(x).hexdigest()
with tempfile.TemporaryDirectory(prefix='d7b-audit-') as temporary:
 expected=Path(temporary)
 subprocess.run([sys.executable,str(root/'tests/native/r14-d7-b-support/generate.py'),str(root),str(expected)],check=True)
 files={}
 for p in expected.rglob('*'):
  if p.is_file():
   rel=p.relative_to(expected);assert (generated/rel).read_bytes()==p.read_bytes(),rel;files[str(rel)]=sha(p.read_bytes())
 assert set(files)=={str(p.relative_to(generated)) for p in generated.rglob('*') if p.is_file()}
 module=(build/'src/cmd/planar/planar_load_cli_probe_source.cppm').read_text()
 arrays={name:bytes(int(x,16) for x in re.findall(r'0x([0-9a-f]{2})',body)) for name,body in re.findall(r'constexpr unsigned char (\w+)\[\] = \{([^}]*)\};',module)}
 originals=re.findall(r'\{"([^"]+)", \{reinterpret_cast<const char\*>\((original_\d+)\)',module)
 assert len(originals)==10
 assembled=b''
 for logical,name in originals:
  source=expected/'commands.lua' if logical=='planar/commands.lua' else root/'src/stdlib'/logical
  assert arrays[name]==source.read_bytes(),logical;assembled+=source.read_bytes()+b'\n'
 assert arrays['source_bytes']==assembled
 assert 'std::string_view separator() { return "\\n"; }' in module
 report.write_text(json.dumps({'path_base':'build directory','files':files,'originals':[p for p,n in originals],'assembly_sha256':sha(assembled),'module_sha256':sha(module.encode())},indent=2)+'\n')
print('Audited full exact load CLI probe graph, every original, separator and assembly')
