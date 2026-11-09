"""Independent C01 policy regressions through the source-built CLI, no ledger edits."""
import argparse
import json
import pathlib
import subprocess
import tempfile


class Fixture:
    def __init__(self, binary):
        self.binary = binary
        self.tmp = tempfile.TemporaryDirectory(prefix='planar-c01-policy-')
        self.root = pathlib.Path(self.tmp.name).resolve()
        self.seq = 0
        self.revision = 'none'
        for name in ('workspace', 'current', 'base', 'scratch'):
            (self.root / name).mkdir()
        self.w = self.root / 'workspace'
        (self.w / 'workspace.lua').write_text('''local p=planar local L=p.list return p.workspace{version=1,id="probe",root={id="source",base="initial",excluded=L{}},spec="spec.lua",jobs=L{{id="job",file="job.lua"}},registry={id="tools",revision="v1"},toolchains=L{"shell"},checks=L{"test"}}''')
        self.spec()
        self.job()
        self.registry = self.root / 'registry.json'
        self.registry.write_text(json.dumps({'version': 1, 'id': 'tools', 'revision': 'v1', 'toolchains': [{'id': 'shell', 'description': 'C01 verification', 'command': ['/bin/sh'], 'environment': []}], 'checks': [{'id': 'test', 'toolchain': 'shell', 'argv': ['-c', 'printf \'{"version":1,"matched":1,"passed":1,"failed":0}\' > "$PLANAR_CHECK_RESULT"'], 'coverage': 'probe', 'inputs': ['own_output'], 'timeout_ms': 10000, 'max_output_bytes': 1024, 'selection': 'nonempty'}]}))
        (self.root / 'current/src').mkdir()
        (self.root / 'current/src/a').write_text('one\n')
        self.run('init', '--workspace-id', 'probe')
        self.load()
        self.mutate('activate', '--job', 'job', '--expect-revision', self.revision, '--expect-job', 'NONE')

    def close(self):
        self.tmp.cleanup()

    def spec(self, summary='Work', rationale='Original rationale'):
        (self.w / 'spec.lua').write_text('''local p=planar local L=p.list return p.spec{version=1,id="probe",goal=p.md"Check policy",work=L{{id="unit",summary=p.md%s,paths=L{"src/"},checks=L{"test"}}},milestones=L{{id="m",work=L{"unit"},acceptance=L{{id="a",text=p.md"Works"}}}},constraints=L{},prescriptions=L{},contracts=L{},facets=L{},dependencies=L{},frozen=L{},rationale=p.immaterial(p.md%s)}''' % (json.dumps(summary), json.dumps(rationale)))

    def job(self, reviewer='reviewer'):
        (self.w / 'job.lua').write_text('''local p=planar local L=p.list return p.job{version=1,id="job",shape="fanout_review",budget_bytes=131072,roles=L{{id="worker",kind="worker",work=L{"unit"}},{id=%s,kind="reviewer",targets=L{"unit"}}}}''' % json.dumps(reviewer))

    def run(self, verb, *args):
        p = subprocess.run([str(self.binary), verb, '--ledger', str(self.root / 'ledger.db'), *map(str, args)], capture_output=True, text=True, timeout=30)
        if p.returncode or p.stderr:
            raise AssertionError(f'{verb}: exit {p.returncode}: {p.stderr}')
        return json.loads(p.stdout)

    def mutate(self, verb, *args):
        self.seq += 1
        return self.run(verb, *args, '--request-id', f'probe-{self.seq}', '--actor', 'verification')

    def load(self):
        result = self.mutate('load', '--workspace', self.w / 'workspace.lua', '--registry', self.registry, '--expect-revision', self.revision)
        self.revision = result['revision']
        return result

    def claim(self, role):
        return self.mutate('claim', '--job', 'job', '--role', role)

    def check(self, claim):
        return self.mutate('check', '--run', claim['run']['token'], '--root', self.root / 'current', '--base-root', self.root / 'base', '--scratch', self.root / 'scratch')

    def verdict(self, claim, check, decision='approve'):
        path = self.root / 'verdict.json'
        path.write_text(json.dumps({'version': 1, 'targets': [{'work': 'unit', 'output': check['observation']['units'][0]['output'], 'work_fp': check['completion']['work_fp']}], 'policy': claim['dispatch']['work_fp'], 'decision': decision, 'findings': [], 'repair': [], 'summary': 'C01 verification'}))
        return self.mutate('verdict', '--run', claim['run']['token'], '--verdict', path)

    def state(self, role):
        return next(r['state'] for r in self.run('status')['roles'] if r['role'] == role)

    def completed(self):
        result = self.check(self.claim('worker'))
        assert result['outcome'] == 'completed', result
        return result


def immaterial_and_lineage(f):
    check = f.completed()
    reviewer = f.claim('reviewer')
    f.verdict(reviewer, check)
    assert f.run('status')['review']['acceptance'][0]['accepted']
    f.spec(rationale='Changed optional rationale')
    f.load()
    assert f.state('reviewer') == 'done', 'F3 optional prose forced an extra review'
    assert f.run('status')['review']['acceptance'][0]['accepted'], 'F3 optional prose lost approval'
    f.job('replacement')
    f.load()
    f.job()
    f.load()
    assert f.state('worker') == 'done', 'worker evidence must survive reviewer recreation'
    assert not f.run('status')['review']['acceptance'][0]['accepted'], 'F1 approval crossed reviewer recreation'
    assert f.state('reviewer') == 'ready'
    f.verdict(f.claim('reviewer'), check)
    assert f.run('status')['review']['acceptance'][0]['accepted']


def target_change_and_blocker(f, failed=False):
    check = f.completed()
    reviewer = f.claim('reviewer')
    # Keep a reviewer run across material change/restoration, and start a worker
    # while stale. Restoring declarations leaves its existing run active.
    f.spec(summary='Changed work')
    f.load()
    worker = f.claim('worker')
    f.spec()
    f.load()
    result = f.verdict(reviewer, check, 'reject')
    assert result['verdict']['applicability'] == 'current', result
    assert f.run('status')['review']['blockers'], 'unowned rejection must open blocker'
    (f.root / 'current/src/a').write_text('two\n')
    # Worker acknowledged the changed W; explicitly refresh it before checking.
    status = f.run('status', '--role', 'worker')
    f.mutate('refresh', '--run', worker['run']['token'], '--expect-fp', status['role_detail']['acknowledged']['work'])
    if failed:
        (f.root / 'current/outside-owned-paths').write_text('boundary failure\n')
    changed = f.check(worker)
    if failed:
        assert changed['outcome'] != 'completed', changed
        assert f.state('worker') == 'running'
        assert f.state('reviewer') == 'blocked'
        assert not f.run('status')['review']['blockers'], 'F4 evidence-only target change retained obsolete blocker'
        return
    assert changed['outcome'] == 'completed', changed
    assert f.state('reviewer') == 'stale', 'F3 changed target retained done reviewer'
    assert not f.run('status')['review']['blockers'], 'F4 changed target retained obsolete blocker'
    f.verdict(f.claim('reviewer'), changed)
    assert f.run('status')['review']['acceptance'][0]['accepted']


def evidence_only_blocker(f):
    target_change_and_blocker(f, failed=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', type=pathlib.Path, required=True)
    args = parser.parse_args()
    failures = 0
    for scenario in (immaterial_and_lineage, target_change_and_blocker, evidence_only_blocker):
        fixture = None
        try:
            fixture = Fixture(args.binary.resolve())
            scenario(fixture)
            print(f'PASS {scenario.__name__}', flush=True)
        except Exception as error:
            failures += 1
            print(f'FAIL {scenario.__name__}: {error}', flush=True)
        finally:
            if fixture:
                fixture.close()
    print(f'3 scenarios, {failures} failures, 0 skips', flush=True)
    raise SystemExit(bool(failures))


if __name__ == '__main__':
    main()
