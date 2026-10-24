"""Execute one direct verification command, preserving raw output and provenance."""
from datetime import datetime, timezone
from pathlib import Path
import json
import subprocess
import sys

root = Path(__file__).resolve().parents[3]
name, *argv = sys.argv[1:]
evidence = root / 'tests/native/v5-02-x03-b1-evidence'
start = datetime.now(timezone.utc).isoformat()
with (evidence / (name + '.log')).open('w') as output:
    result = subprocess.run(argv, cwd=root, stdout=output, stderr=subprocess.STDOUT)
(evidence / (name + '-result.json')).write_text(json.dumps(dict(
    argv=argv, cwd=str(root), started_at=start,
    ended_at=datetime.now(timezone.utc).isoformat(), exit_code=result.returncode), indent=2) + '\n')
print(name, result.returncode)
sys.exit(result.returncode)
