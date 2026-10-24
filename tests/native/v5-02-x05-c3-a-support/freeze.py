"""Freeze a bounded candidate and verify preedit protected/release membership and bytes."""
from pathlib import Path
import difflib
import hashlib
import json
import os
import stat
import sys
sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[3]
SUPPORT = Path(__file__).parent
EVIDENCE = ROOT / 'tests/native/v5-02-x05-c3-a-evidence'
OWNED = list(json.loads((EVIDENCE / 'owned-before.json').read_text()))
def write(path, value):
    path.write_text(json.dumps(value, sort_keys=True, indent=2) + '\n')
def entry(path):
    result = {'mode': stat.S_IMODE(path.lstat().st_mode)}
    if path.is_symlink():
        result['link'] = os.readlink(path)
    elif path.is_file():
        result['sha256'] = hashlib.sha256(path.read_bytes()).hexdigest()
    else:
        result['directory'] = True
    return result
def snapshot(tops):
    result = {}
    for top in tops:
        path = ROOT / top
        for item in [path, *path.rglob('*')]:
            key = str(item.relative_to(ROOT))
            if key in OWNED or key.startswith('tests/native/v5-02-x05-c3-a-') or key.startswith('docs/vernacular/rounds/'):
                continue
            result[key] = entry(item)
    return result
def protected():
    current = snapshot(['src', 'vendor', 'cmake', 'scripts', 'fixtures', 'docs', 'tests'])
    before = json.loads((EVIDENCE / 'protected-before.json').read_text())
    changed = [key for key in set(current) | set(before) if current.get(key) != before.get(key)]
    assert not changed, changed
    release = snapshot(['build/rewrite-release'])
    assert release == json.loads((EVIDENCE / 'release-before.json').read_text()), 'release changed'
    assert (ROOT / 'AGENTS.md').is_symlink() and os.readlink(ROOT / 'AGENTS.md') == 'CLAUDE.md'
    return dict(protected_entries=len(current), release_entries=len(release), unchanged=True)
def candidate():
    paths = {ROOT / name for name in OWNED}
    for top in ['src', 'cmake', 'scripts', 'fixtures', 'tests/stdlib', 'tests/vectors', str(SUPPORT.relative_to(ROOT))]:
        paths.update(item for item in (ROOT / top).rglob('*') if item.is_file())
    paths.update(ROOT / name for name in ['CMakeLists.txt', 'CMakePresets.json', 'Makefile', '.clang-format'])
    return {str(path.relative_to(ROOT)): entry(path) for path in sorted(paths)}
def verify_candidate():
    current = candidate()
    assert current == json.loads((EVIDENCE / 'frozen-candidate.json').read_text()), 'candidate changed after freeze'
    return len(current)
if __name__ == '__main__':
    if sys.argv[1] == 'freeze':
        write(EVIDENCE / 'frozen-candidate.json', candidate())
        write(EVIDENCE / 'protected-before-gate.json', protected())
        diff = []
        for name in OWNED:
            old = (EVIDENCE / 'preedit' / name).read_text().splitlines(keepends=True)
            new = (ROOT / name).read_text().splitlines(keepends=True)
            diff.extend(difflib.unified_diff(old, new, fromfile='preedit/' + name, tofile='candidate/' + name))
            target = EVIDENCE / 'candidate-source' / name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes((ROOT / name).read_bytes())
        (EVIDENCE / 'candidate.diff').write_text(''.join(diff))
        print('frozen', len(candidate()), flush=True)
    else:
        print('verified', verify_candidate(), flush=True)
        write(EVIDENCE / 'protected-final.json', protected())
