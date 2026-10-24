"""Run the frozen current pre-B1 source executable only against isolated native format2 fixtures."""
from pathlib import Path
import hashlib
import json
import os
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[3]
evidence = root / 'tests/native/v5-02-x03-b1-evidence'
old = evidence / 'pre-b1-planar'
helper = root / 'build/rewrite-debug/src/engine/planar_format2_fixture'
# CMake runtime output defaults are explicit; no PATH executable lookup.
if not helper.exists():
    helper = root / 'build/rewrite-debug/bin/planar_format2_fixture'

def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def snapshot(directory):
    return {str(p.relative_to(directory)): ('directory' if p.is_dir() else digest(p))
            for p in sorted(directory.rglob('*'))}

assert digest(old) == (evidence / 'pre-b1-planar.sha256').read_text().split()[0]
runs = []
with tempfile.TemporaryDirectory(prefix='planar-b1-old-') as temp:
    scratch = Path(temp).resolve()
    home = scratch / 'home'
    home.mkdir()
    env = dict(os.environ, HOME=str(home), PLANAR_DB=str(scratch / 'unused-legacy.db'))
    ledger = scratch / 'ledger'
    subprocess.run([str(helper), str(ledger)], check=True, env=env)
    # Header2 is created through the actual candidate native initializer, never hand-edited.
    assert int.from_bytes(ledger.read_bytes()[60:64], 'big') == 2
    for sidecar in [None, '-journal', '-wal', '-shm']:
        if sidecar:
            Path(str(ledger) + sidecar).write_bytes(b'old-binary-sidecar-sentinel')
        commands = [
            [str(old), 'history', '--ledger', str(ledger)],
            [str(old), 'load', '--ledger', str(ledger), '--workspace', str(scratch / 'workspace'),
             '--registry', str(scratch / 'registry.json'), '--expect-revision', 'none',
             '--request-id', 'old-proof', '--actor', 'driver'],
        ]
        for argv in commands:
            before = snapshot(scratch)
            result = subprocess.run(argv, env=env, capture_output=True, text=True)
            after = snapshot(scratch)
            runs.append(dict(argv=argv, sidecar=sidecar, exit_code=result.returncode,
                             stdout=result.stdout, stderr=result.stderr,
                             before=before, after=after, unchanged=before == after))
            assert result.returncode == 3, runs[-1]
            assert 'unsupported_ledger_version' in result.stderr, runs[-1]
            assert before == after, runs[-1]
        if sidecar:
            Path(str(ledger) + sidecar).unlink()
(evidence / 'old-binary-results.json').write_text(json.dumps(dict(
    executable=str(old.relative_to(root)), executable_sha256=digest(old),
    fixture_executable=str(helper.relative_to(root)), fixture_sha256=digest(helper),
    baseline_build_log='baseline-build.log', runs=runs), indent=2) + '\n')
print(f'{len(runs)} actual pre-B1 executable refusals; exact scratch directory bytes and sidecars unchanged')
