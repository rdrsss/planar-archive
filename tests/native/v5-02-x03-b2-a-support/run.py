"""Serialized explicit command capture for this scratch-only round."""
import datetime
import json
from pathlib import Path
import shutil
import subprocess
import sys
root = Path(__file__).resolve().parents[3]
out = root / 'tests/native/v5-02-x03-b2-a-evidence'
name, *command = sys.argv[1:]
start = datetime.datetime.now(datetime.timezone.utc).isoformat()
result = subprocess.run(command, cwd=root, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
(out / (name + '.log')).write_bytes(result.stdout)
(out / (name + '-result.json')).write_text(json.dumps({'argv': command, 'cwd': str(root), 'started_at': start, 'ended_at': datetime.datetime.now(datetime.timezone.utc).isoformat(), 'exit_code': result.returncode}, indent=2) + '\n')
if command[0] == 'ctest' and '--show-only=json-v1' not in command:
    shutil.copy2(root / 'build/rewrite-debug/Testing/Temporary/LastTest.log', out / (name + '-LastTest.log'))
print(result.stdout.decode(errors='replace')[-5000:])
sys.exit(result.returncode)
