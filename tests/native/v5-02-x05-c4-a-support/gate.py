"""Serialized focused Debug verification; no full/release suite or private compilation."""
from pathlib import Path
import json
import subprocess
import sys
import time
import xml.etree.ElementTree as ET
sys.dont_write_bytecode = True
from freeze import ROOT, EVIDENCE, verify_candidate, write
BUILD = ROOT / 'build/rewrite-debug'
OUT = EVIDENCE / 'final-gate'
OUT.mkdir()
commands = []
def run(label, args):
    verify_candidate()
    started = time.time()
    result = subprocess.run(list(map(str, args)), cwd=ROOT, capture_output=True, text=True)
    (OUT / (label + '.log')).write_text(result.stdout + result.stderr)
    commands.append(dict(label=label, argv=list(map(str, args)), cwd=str(ROOT), exit=result.returncode, seconds=time.time()-started))
    write(OUT / 'commands.json', commands)
    print(label, result.returncode, flush=True)
    assert result.returncode == 0, (label, (result.stdout + result.stderr)[-3000:])
    verify_candidate()
    return result.stdout
run('build', ['cmake', '--build', BUILD, '--target', 'all', 'planar_tests'])
listing = json.loads(run('inventory', ['ctest', '--test-dir', BUILD, '--show-only=json-v1']))
write(OUT / 'inventory.json', listing)
before = json.loads((EVIDENCE / 'inventory-before.json').read_text())
old = {test['name'] for test in before['tests']}
current = {test['name'] for test in listing['tests']}
assert len(old) == 743 and old <= current
new = sorted(current - old)
assert len(new) == 13 and sum(name.startswith('replacement.C4 A ') for name in new) == 13
selected = [test['name'] for test in listing['tests'] if Path(test['command'][0]).name in {'planar_runtime_tests', 'planar_command_tests', 'planar_capture_admission_tests', 'planar_capture_profile_tests', 'planar_capture_profile_codec_tests', 'planar_capture_host_path_tests', 'planar_capture_native_tests', 'planar_capture_tests'} or test['name'] == 'replacement.workspace_selector_cli']
assert len(selected) == len(set(selected)) == 422
selected_path = OUT / 'selected-tests.txt'
selected_path.write_text('\n'.join(selected) + '\n')
write(OUT / 'inventory-preservation.json', dict(current=len(current), preserved=len(old), added=new, focused=len(selected)))
for label, path, count in [('focused', selected_path, 422)]:
    run(label, ['ctest', '--test-dir', BUILD, '--tests-from-file', path, '--output-on-failure', '--output-junit', OUT / (label + '.xml')])
    (OUT / (label + '-LastTest.log')).write_bytes((BUILD / 'Testing/Temporary/LastTest.log').read_bytes())
    cases = ET.parse(OUT / (label + '.xml')).findall('.//testcase')
    assert len(cases) == count and all(case.get('status') == 'run' and not any(case.find(tag) is not None for tag in ['failure', 'error', 'skipped']) for case in cases)
for label, script in [('selection-vectors','tests/native/v5-02-x05-b3-c-support/vector.py'),('shape-definition-vectors','tests/native/v5-02-x05-b2-a-support/vector.py'),('canonical-vectors','tests/vectors/generate.py'),('record-v1-vectors','tests/vectors/records/generate.py'),('request-v2-vectors','tests/vectors/records/generate-request-v2.py'),('effective-provenance-vectors','tests/vectors/effective-provenance/generate.py')]:
    run(label, [sys.executable, script] + ([] if label == 'shape-definition-vectors' else ['--check']))
for label, script in [('historical','v5-02-b2-support/audit-variants.py'),('d6','r14-d6-support/audit.py'),('d6-c01','r14-d6-c01-support/audit.py'),('d7-b','r14-d7-b-support/audit.py'),('b1','v5-02-x05-b1-support/audit.py')]:
    run(label+'-audit', [sys.executable, ROOT/'tests/native'/script, BUILD, OUT/(label+'-audit-report.json')])
run('current-boundary-audit', [sys.executable, ROOT/'tests/native/v5-02-x05-c4-a-support/audit.py', OUT/'current-boundary-audit-report.json'])
run('pinned-lint', [sys.executable, 'scripts/rewrite-check.py', 'lint', '--build-dir', BUILD])
assert 'ninja: no work to do.' in run('no-work', ['cmake', '--build', BUILD, '--target', 'all', 'planar_tests'])
write(OUT / 'summary.json', dict(inventory=len(current), preserved=743, new=13, focused=422, vectors=6, audits=6, failures=0, errors=0, skips=0, disabled=0, candidate_entries=verify_candidate(), no_full_gate=True, no_release_run=True))
print('FROZEN FOCUSED C4-A GATE PASS', flush=True)
