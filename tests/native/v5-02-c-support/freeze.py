"""Round-local exact input snapshots; all manifest paths are repository-relative."""
from pathlib import Path
import hashlib,json,subprocess,sys
root=Path(__file__).resolve().parents[3];out=root/'tests/native/v5-02-c-evidence'
files=['tests/acceptance/bootstrap_paths.py','src/engine/capture/host_path.cppm','src/engine/capture/host_path.cpp','src/engine/capture/host_path.t.cpp','src/engine/CMakeLists.txt','src/engine/acceptance/prepared.t.cpp','src/cmd/planar/invocation.cppm','src/cmd/planar/invocation.cpp','src/cmd/planar/bootstrap.t.cpp','src/cmd/planar/CMakeLists.txt','docs/architecture.md','docs/testing.md','docs/cli-reference.md','CLAUDE.md','docs/vernacular/implementation.md','docs/vernacular/roadmap.md','docs/vernacular/v5-alignment.md','docs/vernacular/acceptance-matrix.md','docs/vernacular/command-contract.md']
mode=sys.argv[1]
if mode=='baseline':
 protected={str(p.relative_to(root)):hashlib.sha256(p.read_bytes()).hexdigest() for base in ['fixtures','tests/native','tests/vectors'] for p in (root/base).rglob('*') if p.is_file() and not any(x in p.parts for x in ['v5-02-c-support','v5-02-c-evidence','__pycache__'])}
 for p in [root/'docs/vernacular/planar-design-spec-v5.md',root/'src/engine/runtime/runtime.cpp',root/'src/engine/runtime/runtime.cppm']:
  protected[str(p.relative_to(root))]=hashlib.sha256(p.read_bytes()).hexdigest()
 (out/'protected.json').write_text(json.dumps(protected,indent=2)+'\n')
 (out/'baseline-status.txt').write_text(subprocess.check_output(['git','status','--short'],cwd=root,text=True))
 (out/'baseline-inventory.json').write_text(subprocess.check_output(['ctest','--test-dir','build/rewrite-debug','--show-only=json-v1'],cwd=root,text=True))
manifest={}
if mode=='candidate':files += sorted(str(p.relative_to(root)) for p in (root/'tests/native/v5-02-c-support').glob('*.py'))
for name in files:
 p=root/name;dest=out/mode/name;dest.parent.mkdir(parents=True,exist_ok=True);dest.write_bytes(p.read_bytes());manifest[name]=hashlib.sha256(p.read_bytes()).hexdigest()
(out/f'{mode}.json').write_text(json.dumps({'path_base':'repository root','files':manifest},indent=2)+'\n')
if mode=='candidate':
 rows=[]
 for name in files:
  before=out/'baseline'/name
  p=subprocess.run(['git','diff','--no-index','--',str(before) if before.exists() else '/dev/null',str(root/name)],capture_output=True,text=True)
  assert p.returncode in [0,1];rows.append(p.stdout)
 (out/'owned.diff').write_text(''.join(rows))
 protected=json.loads((out/'protected.json').read_text());changed=[name for name,digest in protected.items() if hashlib.sha256((root/name).read_bytes()).hexdigest()!=digest]
 assert not changed,changed
 (out/'protection-audit.json').write_text(json.dumps({'path_base':'repository root','checked':len(protected),'unchanged':len(protected),'changed':changed},indent=2)+'\n')
print(mode,len(manifest))
