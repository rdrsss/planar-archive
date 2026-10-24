"""Freeze final candidate and protect approved unowned source/golden bytes."""
from pathlib import Path
import hashlib,json
root=Path(__file__).resolve().parents[3];e=root/'tests/native/v5-02-x05-b4-b-c01-evidence';sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
paths=set()
for directory in ['src','scripts','cmake','tests/native/v5-02-x05-b4-b-c01-support','fixtures/render']:
 for p in (root/directory).rglob('*'):
  if p.is_file() and '__pycache__'not in p.parts:paths.add(p)
for name in ['CMakeLists.txt','CMakePresets.json','CLAUDE.md','docs/architecture.md','docs/testing.md','docs/vernacular/implementation.md','docs/vernacular/roadmap.md','docs/vernacular/external-profile-implementation.md']:paths.add(root/name)
(e/'frozen-source.json').write_text(json.dumps({str(p.relative_to(root)):sha(p) for p in sorted(paths)},indent=2)+'\n')
for name,digest in json.loads((e/'protected-start.json').read_text()).items():assert sha(root/name)==digest,name
old=json.loads((root/'tests/native/v5-02-x05-b4-b-evidence/frozen-source.json').read_text())
owned=set(json.loads((e/'baseline.json').read_text())['sha256'])
protected={name:digest for name,digest in old.items() if name not in owned}
for name,digest in protected.items():assert sha(root/name)==digest,name
(e/'protected-unowned.json').write_text(json.dumps(protected,indent=2)+'\n')
