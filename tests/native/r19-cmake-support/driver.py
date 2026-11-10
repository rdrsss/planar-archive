#!/usr/bin/env python3
"""Drive the real engine CMake task with explicit candidate paths and fresh state."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

REPO = Path(__file__).resolve().parents[3]
EVIDENCE = REPO / 'tests/native/r19-cmake-evidence'
DRIVER = EVIDENCE / 'driver'
WORKSPACE = Path(__file__).parent / 'workspace'


def save(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2) + '\n')


def invoke(context, verb, *arguments, required=True):
    folder = Path(context['folder'])
    number = len(list(folder.glob('*.command.json'))) + 1
    stem = folder / f'{number:03d}-{verb}'
    command = [context['binary'], verb, '--ledger', context['ledger'], *map(str, arguments)]
    save(stem.with_suffix('.command.json'), {'argv': command})
    env = os.environ.copy()
    env['HOME'] = str(DRIVER / 'home')
    result = subprocess.run(command, cwd=REPO, env=env, capture_output=True)
    stem.with_suffix('.stdout').write_bytes(result.stdout)
    stem.with_suffix('.stderr').write_bytes(result.stderr)
    stem.with_suffix('.exit').write_text(str(result.returncode) + '\n')
    print(f'{number:03d} {verb}: exit {result.returncode}', flush=True)
    if required and (result.returncode or result.stderr):
        raise RuntimeError(f'{verb}: {result.stderr.decode(errors="replace")}')
    try:
        value = json.loads(result.stdout)
    except (ValueError, UnicodeError):
        value = result.stdout.decode(errors='replace')
    return result, value


def preflight():
    DRIVER.mkdir(parents=True, exist_ok=True)
    (DRIVER / 'home').mkdir(exist_ok=True)
    (DRIVER / 'scratch').mkdir(exist_ok=True)
    registry = {
        'version': 1, 'id': 'engine_cmake_tools', 'revision': 'v1',
        'toolchains': [{'id': 'python', 'description': 'Actual repository CMake gates',
                        'command': [sys.executable, str(Path(__file__).parent / 'check.py')],
                        'environment': [{'name': 'PATH', 'value': os.environ['PATH']},
                                        {'name': 'HOME', 'value': str(DRIVER / 'home')}]}],
        'checks': [{'id': 'cmake_' + preset, 'toolchain': 'python',
                    'argv': ['--repo', str(REPO), '--evidence', str(EVIDENCE), '--preset', preset],
                    'coverage': 'Actual target graph, full registered tests, lint and no-work rebuild',
                    'inputs': ['own_output'], 'timeout_ms': 3600000,
                    'max_output_bytes': 65536, 'selection': 'nonempty'}
                   for preset in ['debug', 'release']]
    }
    save(DRIVER / 'registry.json', registry)
    unknown = {'status': 'unknown'}
    save(DRIVER / 'provenance.json', {
        'version': 1, 'driver': {'name': 'Codex external driver', 'version': unknown},
        'trial': {'id': 'r19_engine_cmake'}, 'view': {'id': 'actual_engine_working_tree'},
        'model': {'requested': unknown, 'actual': unknown},
        'session': {'requested': unknown, 'actual': unknown},
        'settings': [{'name': 'starting_revision', 'value': subprocess.check_output(
            ['git', 'rev-parse', 'HEAD'], cwd=REPO).decode().strip()}]
    })
    failures = []
    for label, candidate in [
        ('authoring', REPO / 'build/rewrite-debug/src/cmd/planar/planar_authoring_stack'),
        ('production', REPO / 'build/rewrite-debug/bin/planar'),
    ]:
        folder = DRIVER / label
        folder.mkdir(exist_ok=False)
        binary = folder / 'candidate'
        shutil.copy2(candidate, binary)
        context = {'folder': str(folder), 'binary': str(binary),
                   'binary_sha256': hashlib.sha256(binary.read_bytes()).hexdigest(),
                   'ledger': str(folder / 'ledger.db'), 'bundle_trial': label}
        try:
            invoke(context, 'init', '--workspace-id', 'engine_cmake')
            _, loaded = invoke(context, 'load', '--workspace', WORKSPACE / 'workspace.lua',
                               '--registry', DRIVER / 'registry.json', '--expect-revision', 'none',
                               '--request-id', 'cmake.load.1', '--actor', 'codex_driver')
            invoke(context, 'activate', '--job', 'combined', '--expect-revision', loaded['revision'],
                   '--expect-job', 'NONE', '--request-id', 'cmake.activate.1', '--actor', 'codex_driver')
            invoke(context, 'status')
            invoke(context, 'preview', '--role', 'builder')
            _, claim = invoke(context, 'claim', '--job', 'combined', '--role', 'builder',
                              '--request-id', 'cmake.claim.1', '--actor', 'cmake_builder',
                              '--provenance', DRIVER / 'provenance.json')
            context['claim'] = claim
            invoke(context, 'recover', '--role', 'builder', '--root', 'source=' + str(REPO / 'src/engine'),
                   '--base-root', 'source=' + str(DRIVER / 'base'))
            if label == 'authoring':
                invoke(context, 'orchestrator')
            context['preflight_failures'] = failures
            save(DRIVER / 'context.json', context)
            print('Pre-implementation compatibility trial passed: ' + label, flush=True)
            return
        except (RuntimeError, KeyError) as error:
            failures.append({'trial': label, 'error': str(error)})
            save(folder / 'FAILED.json', failures[-1])
    raise RuntimeError('Both fresh-state trials failed: ' + repr(failures))


def check(request_id):
    context = json.loads((DRIVER / 'context.json').read_text())
    _, result = invoke(context, 'check', '--run', context['claim']['run']['token'],
                       '--root', REPO / 'src/engine', '--base-root', DRIVER / 'base',
                       '--scratch', DRIVER / 'scratch', '--request-id', request_id,
                       '--actor', 'cmake_builder', '--provenance', DRIVER / 'provenance.json')
    save(DRIVER / (request_id + '.result.json'), result)
    if result.get('outcome') != 'completed':
        raise RuntimeError('Check did not complete the role; inspect retained actual evidence')
    invoke(context, 'status')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('phase', choices=['preflight', 'check'])
    parser.add_argument('--request-id', default='cmake.check.1')
    args = parser.parse_args()
    if args.phase == 'preflight':
        preflight()
    else:
        check(args.request_id)
