#!/usr/bin/env python3
"""Real process regressions for retained authoring-bundle execution dispatch."""
import argparse
import hashlib
import json
import os
import shlex
import shutil
import signal
from pathlib import Path
import subprocess
import tempfile
import time


class Trial:
    """An isolated driver-owned workspace, roots, HOME and explicit ledger."""

    def __init__(self, root, authoring, production):
        self.root = root
        self.authoring = authoring
        self.production = production
        self.ledger = root / 'state/ledger.db'
        self.calls = 0
        for name in ('workspace', 'state', 'current/src', 'base', 'scratch', 'home', 'raw'):
            (root / name).mkdir(parents=True)
        self.env = os.environ.copy()
        self.env['HOME'] = str(root / 'home')
        self.env['PLANAR_DB'] = str(root / 'unused-legacy.db')
        workspace = root / 'workspace'
        (workspace / 'workspace.lua').write_text('''local p=planar local L=p.list return p.workspace{version=1,id="authoring_execution",root={id="source",base="initial",excluded=L{}},spec="spec.lua",jobs=L{{id="job",file="job.lua"}},registry={id="tools",revision="v1"},toolchains=L{"shell"},checks=L{"test"}}\n''')
        (workspace / 'spec.lua').write_text('''local p=planar local L=p.list return p.spec{version=1,id="authoring_execution",goal=p.md"Exercise one registered authoring bundle",work=L{{id="unit",summary=p.md"Retained authoring execution",paths=L{"src/"},checks=L{"test"}}},milestones=L{{id="complete",work=L{"unit"},acceptance=L{{id="verified",text=p.md"Same-bundle execution and independent exact-target approval"}}}},constraints=L{},prescriptions=L{},contracts=L{},facets=L{},dependencies=L{},frozen=L{}}\n''')
        (workspace / 'job.lua').write_text('''local p=planar local L=p.list return p.job{version=1,id="job",shape="fanout_review",budget_bytes=131072,roles=L{{id="worker",kind="worker",work=L{"unit"}},{id="reviewer",kind="reviewer",targets=L{"unit"}}}}\n''')
        (root / 'current/src/output.txt').write_text('observed worker output\n')
        self.registry = root / 'registry.json'
        self.registry.write_text(json.dumps({
            'version': 1, 'id': 'tools', 'revision': 'v1',
            'toolchains': [{'id': 'shell', 'description': 'Isolated execution regression',
                            'command': ['/bin/sh'], 'environment': []}],
            'checks': [{'id': 'test', 'toolchain': 'shell',
                        'argv': ['-c', 'printf \'{"version":1,"matched":1,"passed":1,"failed":0}\' > "$PLANAR_CHECK_RESULT"'],
                        'coverage': 'Execution fixture', 'inputs': ['own_output'],
                        'timeout_ms': 10000, 'max_output_bytes': 1024,
                        'selection': 'nonempty'}]
        }, indent=2) + '\n')

    def run(self, verb, *args, binary=None, expected=0, code=None, profile=None):
        self.calls += 1
        stem = self.root / 'raw' / f'{self.calls:03d}-{verb}'
        argv = [str(binary or self.authoring)]
        if profile is not None:
            argv += ['with-profile', '--stdlib-profile', str(profile)]
        argv += [verb, '--ledger', str(self.ledger), *map(str, args)]
        stem.with_suffix('.argv.json').write_text(json.dumps(argv, indent=2) + '\n')
        result = subprocess.run(argv, cwd=self.root, env=self.env, capture_output=True, timeout=40)
        stem.with_suffix('.stdout').write_bytes(result.stdout)
        stem.with_suffix('.stderr').write_bytes(result.stderr)
        stem.with_suffix('.exit').write_text(str(result.returncode) + '\n')
        assert result.returncode == expected, (
            f'{verb}: expected exit {expected}, got {result.returncode}; '
            f'stdout={result.stdout!r}; stderr={result.stderr!r}')
        if expected == 0:
            assert not result.stderr, (verb, result.stderr)
            return json.loads(result.stdout), result.stdout
        diagnostic = json.loads(result.stderr)
        if code is not None:
            assert diagnostic['error']['code'] == code, (verb, diagnostic)
        return diagnostic, result.stderr

    def load(self, binary=None, profile=None):
        self.run('init', '--workspace-id', 'authoring_execution', binary=binary, profile=profile)
        loaded, _ = self.run('load', '--workspace', self.root / 'workspace/workspace.lua',
                             '--registry', self.registry, '--expect-revision', 'none',
                             '--request-id', 'load', '--actor', 'driver', binary=binary, profile=profile)
        self.activation = ['--job', 'job', '--expect-revision', loaded['revision'],
                           '--expect-job', 'NONE', '--request-id', 'activate', '--actor', 'driver']
        return loaded

    def unchanged_refusal(self, verb, *args, binary=None, code='interpretation_mismatch'):
        before = self.ledger.read_bytes()
        self.run(verb, *args, binary=binary, expected=3, code=code)
        assert self.ledger.read_bytes() == before, f'{verb}: refusal mutated ledger bytes'


def smoke(trial):
    trial.load()
    trial.run('activate', *trial.activation)
    print('PASS authoring init/load/activate uses the registered bundle', flush=True)


def binding_control(trial):
    trial.load()
    trial.unchanged_refusal('activate', *trial.activation, binary=trial.production)
    print('PASS genuinely different production bundle refuses activation without mutation', flush=True)


def lifecycle(trial):
    counter = trial.root / 'check-executions'
    registry = json.loads(trial.registry.read_text())
    registry['checks'][0]['argv'][1] = (
        'printf x >> ' + shlex.quote(str(counter)) + '; ' + registry['checks'][0]['argv'][1])
    trial.registry.write_text(json.dumps(registry) + '\n')
    smoke(trial)
    trial.run('orchestrator')
    trial.run('preview', '--role', 'worker')
    trial.run('status', '--role', 'worker')
    trial.unchanged_refusal('preview', '--role', 'worker', binary=trial.production)
    trial.unchanged_refusal('status', '--role', 'worker', binary=trial.production)
    # Already-saved mutation requests replay before current-bundle binding.
    before = trial.ledger.read_bytes()
    _, first = trial.run('activate', *trial.activation)
    _, replay = trial.run('activate', *trial.activation, binary=trial.production)
    assert first == replay and trial.ledger.read_bytes() == before
    changed = trial.activation.copy()
    changed[-1] = 'different_actor'
    trial.unchanged_refusal('activate', *changed, binary=trial.production, code='request_conflict')
    worker, _ = trial.run('claim', '--job', 'job', '--role', 'worker',
                           '--request-id', 'worker', '--actor', 'worker_session')
    check_args = ['--run', worker['run']['token'], '--root', trial.root / 'current',
                  '--base-root', trial.root / 'base', '--scratch', trial.root / 'scratch',
                  '--request-id', 'check', '--actor', 'worker_session']
    trial.unchanged_refusal('check', *check_args, binary=trial.production)
    checked, first = trial.run('check', *check_args)
    assert checked['outcome'] == 'completed', checked
    assert counter.read_bytes() == b'x', 'check command must execute exactly once'
    assert checked['observation']['units'][0]['work'] == 'unit', checked
    before = trial.ledger.read_bytes()
    _, replay = trial.run('check', *check_args, binary=trial.production)
    assert first == replay and trial.ledger.read_bytes() == before
    assert counter.read_bytes() == b'x', 'saved check replay re-executed the subprocess'
    changed_check = check_args.copy()
    changed_check[-1] = 'different_actor'
    trial.unchanged_refusal('check', *changed_check, binary=trial.production, code='request_conflict')
    status, _ = trial.run('status')
    assert next(r for r in status['roles'] if r['role'] == 'worker')['state'] == 'done', status
    reviewer, _ = trial.run('claim', '--job', 'job', '--role', 'reviewer',
                             '--request-id', 'reviewer', '--actor', 'review_session')
    verdict = trial.root / 'verdict.json'
    verdict.write_text(json.dumps({
        'version': 1,
        'targets': [{'work': 'unit', 'output': checked['observation']['units'][0]['output'],
                     'work_fp': checked['completion']['work_fp']}],
        'policy': reviewer['dispatch']['work_fp'], 'decision': 'approve', 'findings': [],
        'repair': [], 'summary': 'Independent exact-target fixture approval'
    }) + '\n')
    args = ['--run', reviewer['run']['token'], '--verdict', verdict,
            '--request-id', 'verdict', '--actor', 'review_session']
    trial.unchanged_refusal('verdict', *args, binary=trial.production)
    _, first = trial.run('verdict', *args)
    before = trial.ledger.read_bytes()
    _, replay = trial.run('verdict', *args, binary=trial.production)
    assert first == replay and trial.ledger.read_bytes() == before
    changed_verdict = args.copy()
    changed_verdict[-1] = 'different_actor'
    trial.unchanged_refusal('verdict', *changed_verdict, binary=trial.production, code='request_conflict')
    status, _ = trial.run('status')
    assert status['review']['acceptance'][0]['accepted'], status
    # Existing exempt reads are still available across bundle identities.
    trial.run('status', binary=trial.production)
    trial.run('history', binary=trial.production)
    trial.run('orchestrator')
    print('PASS authoring lifecycle, exact review, EX-6 negatives and cross-bundle early replay', flush=True)


def recovery(trial):
    # Stop a real check only after its subprocess reaches an explicit rendezvous.
    started = trial.root / 'started'
    child_pid = trial.root / 'child-pid'
    registry = json.loads(trial.registry.read_text())
    registry['checks'][0]['argv'] = ['-c',
        'echo $$ > ' + shlex.quote(str(child_pid)) + '; : > ' + shlex.quote(str(started)) +
        '; while :; do /bin/sleep 0.05; done']
    registry['checks'][0]['timeout_ms'] = 30000
    trial.registry.write_text(json.dumps(registry) + '\n')
    smoke(trial)
    worker, _ = trial.run('claim', '--job', 'job', '--role', 'worker',
                          '--request-id', 'worker', '--actor', 'worker_session')
    token = worker['run']['token']
    notes = trial.root / 'notes.json'
    notes.write_text(json.dumps({'version': 1, 'checks': [], 'findings': [],
        'checkpoint': {'progress': 'Work observed', 'next': 'Continue checks',
                       'unresolved': '', 'files': []}}) + '\n')
    checkpoint, _ = trial.run('checkpoint', '--run', token, '--observations', notes,
                              '--request-id', 'checkpoint', '--actor', 'worker_session')
    assert checkpoint['checkpoint']['id'] != 'none', checkpoint
    refreshed, _ = trial.run('refresh', '--run', token, '--expect-fp', worker['dispatch']['work_fp'],
                             '--request-id', 'refresh', '--actor', 'worker_session')
    assert refreshed['dispatch']['work_fp'] == worker['dispatch']['work_fp'], refreshed
    recovered, _ = trial.run('recover', '--role', 'worker')
    assert recovered['checkpoint']['value']['progress'] == 'Work observed', recovered
    taken, _ = trial.run('takeover', '--job', 'job', '--role', 'worker',
                         '--run-id', worker['run']['id'], '--request-id', 'takeover',
                         '--actor', 'replacement_session')
    assert taken['run']['generation'] != worker['run']['generation'], taken
    trial.unchanged_refusal('release', '--run', token, '--reason', 'released',
                            '--request-id', 'fenced-release', '--actor', 'worker_session',
                            code='fenced')
    trial.run('release', '--run', taken['run']['token'], '--reason', 'interrupted',
              '--request-id', 'release', '--actor', 'replacement_session')
    status, _ = trial.run('status')
    assert next(r for r in status['roles'] if r['role'] == 'worker')['state'] == 'ready', status
    worker, _ = trial.run('claim', '--job', 'job', '--role', 'worker',
                          '--request-id', 'resumed-worker', '--actor', 'resumed_session')
    argv = [str(trial.authoring), 'check', '--ledger', str(trial.ledger),
            '--run', worker['run']['token'], '--root', str(trial.root / 'current'),
            '--base-root', str(trial.root / 'base'), '--scratch', str(trial.root / 'scratch'),
            '--request-id', 'stopped-check', '--actor', 'resumed_session']
    trial.calls += 1
    stem = trial.root / 'raw' / f'{trial.calls:03d}-stopped-check'
    stem.with_suffix('.argv.json').write_text(json.dumps(argv, indent=2) + '\n')
    process = subprocess.Popen(argv, cwd=trial.root, env=trial.env,
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE, start_new_session=True)
    try:
        deadline = time.monotonic() + 15
        while not started.exists() and process.poll() is None and time.monotonic() < deadline:
            time.sleep(.02)
        assert started.exists() and child_pid.exists(), 'real check subprocess did not reach rendezvous'
        active, _ = trial.run('status', '--request-id', 'stopped-check')
        assert active['request']['outcome'] == 'in_progress', active
        pid = int(child_pid.read_text())
        os.killpg(process.pid, signal.SIGKILL)
        out, err = process.communicate(timeout=10)
        stem.with_suffix('.stdout').write_bytes(out)
        stem.with_suffix('.stderr').write_bytes(err)
        stem.with_suffix('.exit').write_text(str(process.returncode) + '\n')
        try:
            os.killpg(pid, signal.SIGKILL)
        except ProcessLookupError:
            pass
        deadline = time.monotonic() + 10
        while time.monotonic() < deadline:
            try:
                os.kill(pid, 0)
            except ProcessLookupError:
                break
            time.sleep(.02)
        else:
            raise AssertionError('stopped check child is still alive')
        child_pid.unlink()
        status, _ = trial.run('status')
        role = next(r for r in status['roles'] if r['role'] == 'worker')
        reservation = role['run']['reservation']
        result, _ = trial.run('check-recover', '--reservation', reservation,
                              '--run-id', worker['run']['id'], '--confirm-stopped', 'true',
                              '--request-id', 'check-recover', '--actor', 'driver')
        assert result['kind'] == 'check' and result['outcome'] == 'interrupted', result
        state, _ = trial.run('status', '--request-id', 'stopped-check')
        assert state['request']['outcome'] == 'interrupted', state
        assert state['request']['result'] == result, state
        trial.run('release', '--run', worker['run']['token'], '--reason', 'interrupted',
                  '--request-id', 'final-release', '--actor', 'resumed_session')
    finally:
        if process.poll() is None:
            os.killpg(process.pid, signal.SIGKILL)
            process.communicate(timeout=10)
        if child_pid.exists():
            try:
                os.killpg(int(child_pid.read_text()), signal.SIGKILL)
            except ProcessLookupError:
                pass
    print('PASS authoring checkpoint/refresh/recover/takeover/fencing/release/stopped check-recover', flush=True)


def profile_parity(trial):
    trusted = trial.root / 'trusted'
    trusted.mkdir()
    profile = trusted / 'profile.json'
    profile.write_text(json.dumps({'version': 1, 'modules': ['extension.lua']}) + '\n')
    (trusted / 'extension.lua').write_text('-- distinct captured trusted profile bytes\nlocal marker = "v1"\n')
    trial.load(profile=profile)
    # Cloned driver-owned state preserves ledger identity while each executable
    # executes activation freshly, rather than comparing a replay with execution.
    original = trial.ledger
    clone = original.with_name('production-copy.db')
    shutil.copy2(original, clone)
    _, authoring = trial.run('activate', *trial.activation, profile=profile)
    trial.ledger = clone
    try:
        _, production = trial.run('activate', *trial.activation, binary=trial.production, profile=profile)
    finally:
        trial.ledger = original
    assert authoring == production, 'actual profile activation differs between compiled compositions'
    worker, _ = trial.run('claim', '--job', 'job', '--role', 'worker',
                          '--request-id', 'profile-worker', '--actor', 'worker_session', profile=profile)
    for verb, args in [('status', []), ('status', ['--role', 'worker']),
                       ('preview', ['--role', 'worker']), ('recover', ['--role', 'worker'])]:
        _, authoring = trial.run(verb, *args, profile=profile)
        _, production = trial.run(verb, *args, binary=trial.production, profile=profile)
        assert authoring == production, f'actual {verb} profile response differs by composition'
    trial.unchanged_refusal('preview', '--role', 'worker')
    result, _ = trial.run('check', '--run', worker['run']['token'], '--root', trial.root / 'current',
                          '--base-root', trial.root / 'base', '--scratch', trial.root / 'scratch',
                          '--request-id', 'profile-check', '--actor', 'worker_session', profile=profile)
    assert result['outcome'] == 'completed', result
    # Profile bytes remain material; authoring macro cannot bypass this boundary.
    (trusted / 'extension.lua').write_text('-- deliberately changed profile bytes\nlocal marker = "v2"\n')
    before = trial.ledger.read_bytes()
    trial.run('preview', '--role', 'worker', profile=profile, expected=3, code='interpretation_mismatch')
    assert trial.ledger.read_bytes() == before
    # Embedded-only authoring is still refused under with-profile, with existing precedence.
    for binary in (trial.authoring, trial.production):
        trial.run('orchestrator', profile=profile, binary=binary, expected=2)
    print('PASS actual execution-profile parity, closed authoring admission and byte binding', flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--authoring', required=True, type=Path)
    parser.add_argument('--production', required=True, type=Path)
    parser.add_argument('--case', choices=('smoke', 'binding-control', 'lifecycle', 'recovery', 'profile-parity', 'all'), default='all')
    parser.add_argument('--evidence', type=Path, help='New directory retaining exact argv and process outputs')
    args = parser.parse_args()
    authoring = args.authoring.resolve(strict=True)
    production = args.production.resolve(strict=True)
    cases = {'smoke': smoke, 'binding-control': binding_control, 'lifecycle': lifecycle,
             'recovery': recovery, 'profile-parity': profile_parity}
    selected = list(cases) if args.case == 'all' else [args.case]
    def execute(root):
        root.mkdir(parents=True, exist_ok=False)
        identity = {'authoring_sha256': hashlib.sha256(authoring.read_bytes()).hexdigest(),
                    'production_sha256': hashlib.sha256(production.read_bytes()).hexdigest()}
        (root / 'BINARIES.json').write_text(json.dumps(identity, indent=2) + '\n')
        for name in selected:
            cases[name](Trial(root / name, authoring, production))
        print(f'SUMMARY scenarios={len(selected)} failures=0 skips=0', flush=True)
    if args.evidence:
        execute(args.evidence.resolve())
    else:
        with tempfile.TemporaryDirectory(prefix='planar-authoring-execution-') as temporary:
            execute(Path(temporary).resolve() / 'trial')


if __name__ == '__main__':
    main()
