#!/usr/bin/env python3
"""Execute the real host gate and retain its inventory, JUnit and process logs."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import runpy
import subprocess
import sys
import time
import xml.etree.ElementTree as ET


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', required=True, type=Path)
    parser.add_argument('--evidence', required=True, type=Path)
    parser.add_argument('--preset', required=True, choices=('debug', 'release'))
    args = parser.parse_args()
    repo = args.repo.resolve()
    build = repo / 'build' / ('rewrite-' + args.preset)
    attempt = args.evidence.resolve() / 'checks' / args.preset / str(time.time_ns())
    attempt.mkdir(parents=True, exist_ok=False)
    # Reuse the corrected idle-output classifier, including its existing tests.
    previous = repo / 'tests/native/r19-cmake-support'
    sys.path.insert(0, str(previous))
    is_no_work_output = runpy.run_path(str(previous / 'check.py'))['is_no_work_output']
    manifest = {'preset': args.preset, 'commands': [], 'matched': 0,
                'passed': 0, 'failed': 0, 'errors': [], 'started_ns': time.time_ns()}

    def snapshot():
        paths = sorted(p for folder in ('src', 'cmake', 'scripts', 'tests/acceptance')
                       for p in (repo / folder).rglob('*') if p.is_file())
        paths += [repo / name for name in ('CMakeLists.txt', 'CMakePresets.json', 'Makefile')]
        paths += sorted(p for p in (repo / 'tests/native').rglob('*')
                        if p.is_file() and '-evidence' not in str(p.relative_to(repo)))
        return {str(p.relative_to(repo)): hashlib.sha256(p.read_bytes()).hexdigest()
                for p in paths if '__pycache__' not in p.parts}

    def save():
        (attempt / 'MANIFEST.json').write_text(json.dumps(manifest, indent=2) + '\n')

    def run(name, argv):
        argv = list(map(str, argv))
        start = time.time_ns()
        with (attempt / (name + '.stdout')).open('wb') as out, (attempt / (name + '.stderr')).open('wb') as err:
            result = subprocess.run(argv, cwd=repo, stdout=out, stderr=err)
        manifest['commands'].append({'name': name, 'argv': argv, 'exit': result.returncode,
                                     'duration_ns': time.time_ns() - start})
        save()
        if result.returncode:
            raise RuntimeError(f'{name} exited {result.returncode}; raw logs retained')
        return (attempt / (name + '.stdout')).read_bytes()

    before = snapshot()
    (attempt / 'source-before.json').write_text(json.dumps(before, indent=2) + '\n')
    try:
        if os.environ.get('PLANAR_CHECK_RESULT'):
            view = Path.cwd()
            expected = json.loads((args.evidence / 'driver/projection.json').read_text())['candidate']
            observed = {str(p.relative_to(view)): hashlib.sha256(p.read_bytes()).hexdigest()
                        for p in sorted(view.rglob('*')) if p.is_file()}
            actual = {p: hashlib.sha256((repo / p).read_bytes()).hexdigest() for p in expected}
            if observed != expected or actual != expected:
                raise RuntimeError('Checked output projection differs from the actual repository candidate')
            (attempt / 'checked-projection.json').write_text(json.dumps(observed, indent=2) + '\n')
        run('idle-classifier-tests', [sys.executable, previous / 'check_test.py'])
        run('configure', ['cmake', '--preset', args.preset])
        run('build', ['cmake', '--build', build, '--parallel', '4', '--target', 'all', 'planar_tests'])
        inventory = json.loads(run('inventory', ['ctest', '--test-dir', build, '--show-only=json-v1']))
        names = [t['name'] for t in inventory['tests']]
        if not names or len(names) != len(set(names)):
            raise RuntimeError('Empty or duplicate test inventory')
        failure = None
        try:
            run('ctest', ['ctest', '--test-dir', build, '--output-on-failure', '--output-junit', attempt / 'ctest.xml'])
        except RuntimeError as error:
            failure = error
        cases = ET.parse(attempt / 'ctest.xml').findall('.//testcase')
        manifest['matched'] = len(cases)
        manifest['passed'] = sum(c.find('failure') is None and c.find('error') is None and
                                 c.find('skipped') is None and c.get('status') not in {'notrun', 'disabled'}
                                 for c in cases)
        manifest['failed'] = len(cases) - manifest['passed']
        if sorted(c.get('name') for c in cases) != sorted(names) or manifest['passed'] != len(names):
            raise RuntimeError('Actual JUnit does not match passing nonempty inventory with zero skips')
        if failure:
            raise failure
        run('lint', [sys.executable, repo / 'scripts/rewrite-check.py', 'lint', '--build-dir', build])
        repeat = run('repeat-build', ['cmake', '--build', build, '--parallel', '4', '--target', 'all', 'planar_tests'])
        if not is_no_work_output(repeat):
            raise RuntimeError('No-change build performed work or produced unexplained output')
    except Exception as error:
        manifest['errors'].append(str(error))
    after = snapshot()
    (attempt / 'source-after.json').write_text(json.dumps(after, indent=2) + '\n')
    if before != after:
        manifest['errors'].append('Candidate source/support changed during the check')
    manifest['exit'] = int(bool(manifest['errors']))
    manifest['finished_ns'] = time.time_ns()
    save()
    result = {key: manifest[key] for key in ('matched', 'passed', 'failed')}
    result['version'] = 1
    if os.environ.get('PLANAR_CHECK_RESULT'):
        with Path(os.environ['PLANAR_CHECK_RESULT']).open('x') as stream:
            json.dump(result, stream)
    print(json.dumps({**result, 'exit': manifest['exit'], 'errors': manifest['errors']}), flush=True)
    return manifest['exit']


if __name__ == '__main__':
    sys.exit(main())
