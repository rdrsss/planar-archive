# Current producer lineage regressions using the independently checked arithmetic workflow.
import argparse
import hashlib
import pathlib
import traceback

SOURCE = pathlib.Path(__file__).with_name('r15-r18-c03-vectors-cli.py').read_text()
HOOK = "        print('PASS', label, 'vectors=' + str(vector_assertions), 'accepted=' + str(len(units)), flush=True)"
SETUP = '''        if scenario == 'switch':
            (workspace / 'other.lua').write_text((workspace / 'job.lua').read_text().replace('id="job"', 'id="other"'))
            f = workspace / 'workspace.lua'
            f.write_text(f.read_text().replace('jobs=L{{id="job",file="job.lua"}}', 'jobs=L{{id="job",file="job.lua"},{id="other",file="other.lua"}}'))
'''
CHECK = '''        def history_counts():
            with sqlite3.connect(ledger) as db:
                return tuple(db.execute('SELECT COUNT(*) FROM ' + name).fetchone()[0] for name in ('completions', 'publications', 'evidence'))
        before_counts = history_counts()
        historical, historical_raw = run('history', '--event', '5')
        selected_job = 'job'
        producer = 's_worker'
        if scenario in ('activate', 'switch'):
            selected_job = 'other' if scenario == 'switch' else 'job'
            run('activate', '--job', selected_job, '--expect-revision', loaded['revision'], '--expect-job', 'job', '--request-id', 'again', '--actor', 'test')
        elif scenario in ('rename', 'structure', 'material'):
            f = workspace / 'job.lua'
            if scenario == 'rename':
                producer = 's_recreated'
                f.write_text(f.read_text().replace('s_worker', producer))
            elif scenario == 'structure':
                # Consumer appears before the structurally replaced producer.
                text = f.read_text().replace('{id="s_worker",kind="worker",work=L{"s"}},', '')
                text = text.replace('{id="reviewer",', '{id="s_worker",kind="worker",work=L{"s","z"}},{id="reviewer",')
                text = text.replace('{id="z_worker",kind="worker",work=L{"z"}},', '')
                f.write_text(text)
            else:
                f = workspace / 'spec.lua'
                f.write_text(f.read_text().replace('Verify s', 'Verify s with changed requirement'))
            loaded, _ = run('load', '--workspace', workspace / 'workspace.lua', '--registry', registry, '--expect-revision', loaded['revision'], '--request-id', 'changed', '--actor', 'test')
        else:
            raise AssertionError(scenario)
        assert history_counts() == before_counts, 'historical evidence was removed'
        assert run('history', '--event', '5')[0]['data'] == historical['data'], 'historical event changed'
        status, _ = run('status')
        preview, _ = run('preview', '--role', 'b_worker')
        state = next(r['state'] for r in status['roles'] if r['role'] == 'b_worker')
        prerequisite = next(p for p in preview['manifest']['prerequisites'] if p['id'] == 's_to_b')
        if scenario == 'material':
            assert prerequisite['status'] == 'available', prerequisite
            assert state != 'blocked', state
        else:
            assert state == 'blocked', (scenario, state, prerequisite)
            assert prerequisite['status'] == 'unavailable', prerequisite
            refused = subprocess.run([str(binary), 'claim', '--ledger', str(ledger), '--job', selected_job, '--role', 'b_worker', '--request-id', 'premature', '--actor', 'test'], capture_output=True, text=True, timeout=30)
            assert refused.returncode == 3, (refused.returncode, refused.stdout, refused.stderr)
            error = json.loads(refused.stderr)['error']
            assert error['code'] == 'claim_conflict', error
            assert error['details']['state'] == 'blocked', error
            claim, _ = run('claim', '--job', selected_job, '--role', producer, '--request-id', 'fresh_producer', '--actor', 'test')
            checked, _ = run('check', '--run', claim['run']['token'], '--root', current, '--base-root', base, '--scratch', scratch, '--request-id', 'fresh_check', '--actor', 'test')
            assert checked['outcome'] == 'completed', checked
            status, _ = run('status')
            state = next(r['state'] for r in status['roles'] if r['role'] == 'b_worker')
            assert state in ('ready', 'stale', 'done'), (scenario, state)
            preview, _ = run('preview', '--role', 'b_worker')
            prerequisite = next(p for p in preview['manifest']['prerequisites'] if p['id'] == 's_to_b')
            assert prerequisite['status'] == 'available', prerequisite
        print('PASS', scenario, 'history retained; current prerequisite correct', flush=True)
'''

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('binary', type=pathlib.Path)
    parser.add_argument('--scenario', choices=['activate', 'switch', 'rename', 'structure', 'material'])
    args = parser.parse_args()
    binary = args.binary.resolve(strict=True)
    print('binary_sha256=' + hashlib.sha256(binary.read_bytes()).hexdigest(), flush=True)
    assert SOURCE.count(HOOK) == 1
    assert SOURCE.count('        checks = []') == 1
    source = SOURCE.replace('        checks = []', SETUP + '        checks = []').replace(HOOK, CHECK)
    failures = 0
    scenarios = [args.scenario] if args.scenario else ['activate', 'switch', 'rename', 'structure', 'material']
    for scenario in scenarios:
        try:
            scope = dict(__name__='c06_fixture', scenario=scenario)
            exec(compile(source, 'c06_arithmetic_fixture', 'exec'), scope)
            scope['workflow'](binary, False)
        except Exception:
            failures += 1
            traceback.print_exc()
    print(f'SUMMARY scenarios={len(scenarios)} failures={failures} skips=0', flush=True)
    return int(failures != 0)

if __name__ == '__main__':
    raise SystemExit(main())
