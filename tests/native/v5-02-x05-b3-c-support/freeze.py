"""Freeze active production, support, and current prose before final serialized gate."""
from pathlib import Path
import hashlib,json
root=Path(__file__).resolve().parents[3];out=root/'tests/native/v5-02-x05-b3-c-evidence'
paths=set()
for directory in ['src','scripts','cmake','tests/native/v5-02-x05-b3-c-support']:
 for p in (root/directory).rglob('*'):
  if p.is_file() and '__pycache__'not in p.parts:paths.add(p)
for name in ['CMakeLists.txt','CMakePresets.json','CLAUDE.md','docs/architecture.md','docs/testing.md','docs/vernacular/implementation.md','docs/vernacular/roadmap.md','docs/vernacular/external-profile-implementation.md']:paths.add(root/name)
(out/'frozen-source.json').write_text(json.dumps({str(p.relative_to(root)):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(paths)},indent=2)+'\n')
