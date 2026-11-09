# C06 exact diagnostics through source-built CLI and disposable ledgers.
import importlib.util
import json
import pathlib
import sqlite3
import shlex
import subprocess
import sys
import time

spec = importlib.util.spec_from_file_location('fixture', 'tests/native/r15-r18-c01-policy-cli.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
binary = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else 'build/rewrite-release/bin/planar').resolve()
results = []
failures = []
expected = {
'verdict_grammar': ('schema', 'verdict', 'JSON codec refused input'),
'verdict_shape': ('schema', 'verdict.version', 'Required field'),
'verdict_normalize': ('schema', 'verdict.decision', 'Invalid enum value'),
'verdict_nested': ('schema', 'verdict.targets.0.output', 'Malformed output digest'),
'scratch': ('path_denied', 'scratch', 'Scratch directory overlaps a checked root'),
'registry': ('reference', 'registry', 'Registry environment uses a reserved name'),
'in_progress': ('check_in_progress', '', 'A check is already in progress for this role'),
}
for case in ('verdict_grammar', 'verdict_shape', 'verdict_normalize', 'verdict_nested', 'scratch', 'registry', 'in_progress', 'scratch_create'):
    fixture = module.Fixture(binary)
    child = None
    try:
        if case in ('registry', 'in_progress'):
            registry = json.loads(fixture.registry.read_text())
            if case == 'registry':
                registry['toolchains'][0]['environment'] = [{'name': 'PLANAR_BAD', 'value': 'bad'}]
            else:
                registry['checks'][0]['argv'][1] = 'while [ ! -f ' + shlex.quote(str(fixture.root/'release-check')) + ' ]; do /bin/sleep 0.02; done; ' + registry['checks'][0]['argv'][1]
                registry['checks'][0]['timeout_ms'] = 30000
            fixture.registry.write_text(json.dumps(registry))
            fixture.load()
        if case.startswith('verdict'):
            completed = fixture.completed()
            claim = fixture.claim('reviewer')
            verdict = fixture.root / 'invalid.json'
            verdict.write_text('{' if case == 'verdict_grammar' else '{}')
            if case in ('verdict_normalize', 'verdict_nested'):
                verdict.write_text(json.dumps({'version': 1, 'targets': [{'work': 'unit', 'output': completed['observation']['units'][0]['output'], 'work_fp': completed['completion']['work_fp']}], 'policy': claim['dispatch']['work_fp'], 'decision': 'approve', 'findings': [], 'repair': ['worker'], 'summary': 'invalid approval'}))
            if case == 'verdict_nested':
                document = json.loads(verdict.read_text())
                document['targets'][0]['output'] = 'bad'
                verdict.write_text(json.dumps(document))
            args = ['verdict', '--run', claim['run']['token'], '--verdict', str(verdict)]
        else:
            claim = fixture.claim('worker')
            args = ['check', '--run', claim['run']['token'], '--root', str(fixture.root/'current'), '--base-root', str(fixture.root/'base'), '--scratch', str(fixture.root/('current' if case == 'scratch' else 'scratch'))]
        if case == 'scratch_create':
            (fixture.root/'scratch/planar-check.1.1').write_text('collision')
        command = [str(binary), *args, '--ledger', str(fixture.root/'ledger.db'), '--actor', 'review']
        if case == 'in_progress':
            child = subprocess.Popen([*command, '--request-id', 'slow'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            for _ in range(100):
                with sqlite3.connect(fixture.root/'ledger.db') as db:
                    opened = db.execute("SELECT count(*) FROM reservation_state WHERE state='open'").fetchone()[0]
                if opened:
                    break
                time.sleep(.02)
            assert opened
        with sqlite3.connect(fixture.root/'ledger.db') as db:
            before = list(db.iterdump())
        result = subprocess.run([*command, '--request-id', 'probe'], capture_output=True, text=True, timeout=30)
        error = json.loads(result.stderr)['error']
        with sqlite3.connect(fixture.root/'ledger.db') as db:
            after = list(db.iterdump())
        wanted = dict(zip(('code', 'field', 'message'), expected[case])) if case != 'scratch_create' else {'code': 'io', 'field': 'scratch', 'message': 'Cannot create check scratch directory', 'details': {'operation': 'capture'}}
        good = result.returncode == (4 if case == 'scratch_create' else 3) and error == wanted and (before == after if case != 'scratch_create' else before != after)
        if case == 'scratch_create':
            with sqlite3.connect(fixture.root/'ledger.db') as db:
                good = good and db.execute("SELECT count(*) FROM reservation_state WHERE state='open'").fetchone()[0] == 1
            (fixture.root/'scratch/planar-check.1.1').unlink()
            fixture.mutate('check-recover', '--run-id', claim['run']['id'], '--reservation', '1', '--confirm-stopped', 'true')
        results.append({'case': case, 'pass': good, 'actual': error, 'expected': wanted})
        if not good:
            failures.append(case)
        if child:
            for request, actor, code in [('slow', 'review', 'check_in_progress'), ('slow', 'different', 'request_conflict')]:
                replay = subprocess.run([str(binary), *args, '--ledger', str(fixture.root/'ledger.db'), '--actor', actor, '--request-id', request], capture_output=True, text=True, timeout=30)
                assert replay.returncode == 3 and json.loads(replay.stderr)['error']['code'] == code, replay.stderr
            (fixture.root/'release-check').write_text('continue')
            child.communicate(timeout=10)
            assert child.returncode == 0
    finally:
        if child and child.poll() is None:
            child.terminate()
            child.communicate()
        fixture.close()

# Real accepted declarations distinguish verified active revision from registry v1,
# and force two producer mismatches through the public command adapter.
for case in ('delivery_missing', 'delivery_changed', 'delivery_inline', 'vector'):
    fixture = module.Fixture(binary)
    try:
        registry = json.loads(fixture.registry.read_text())
        registry['checks'][0]['inputs'] = ['own_output', 'expected_contracts'] if case != 'vector' else ['own_output']
        if case == 'vector':
            integration = dict(registry['checks'][0], id='integration', inputs=['own_output', 'output_vector'])
            registry['checks'].append(integration)
            workspace_file = fixture.w/'workspace.lua'
            workspace_file.write_text(workspace_file.read_text().replace('checks=L{"test"}', 'checks=L{"test","integration"}'))
        fixture.registry.write_text(json.dumps(registry))
        for unit in ('a', 'z'):
            for tree in ('current', 'base'):
                (fixture.root/tree/unit).mkdir()
                (fixture.root/tree/unit/'value').write_text('old')
        (fixture.w/'contract.txt').write_text('accepted')
        expected_text = 'p.md"accepted"' if case == 'delivery_inline' else 'p.file"contract.txt"'
        contracts = '{}' if case == 'vector' else '{{id="api",producer="a",consumers=L{"unit"},expected=' + expected_text + ',conformance="test",review_note=p.md"Reviewed fixture"}}'
        facets = '{{id="af",producer="a",paths=L{"a/value"}},{id="zf",producer="z",paths=L{"z/value"}}}' if case == 'vector' else '{}'
        deps = '{{id="ad",consumer="unit",source="facet",ref="af",phase="complete",quality="done"},{id="zd",consumer="unit",source="facet",ref="zf",phase="complete",quality="done"}}' if case == 'vector' else '{{id="api_start",consumer="unit",source="contract",ref="api",phase="start",quality="available"}}'
        (fixture.w/'spec.lua').write_text('local p=planar local L=p.list return p.spec{version=1,id="probe",goal=p.md"Diagnostic fixture",work=L{{id="unit",summary=p.md"Consumer",paths=L{"src/"},checks=L{"test"}},{id="a",summary=p.md"Producer a",paths=L{"a/"},checks=L{"test"}},{id="z",summary=p.md"Producer z",paths=L{"z/"},checks=L{"test"}}},milestones=L{{id="m",work=L{"unit","a","z"},acceptance=L{{id="a",text=p.md"Works"}}}},constraints=L{},prescriptions=L{},contracts=L' + contracts + ',facets=L' + facets + ',dependencies=L' + deps + ',frozen=L{}}')
        (fixture.w/'job.lua').write_text('local p=planar local L=p.list return p.job{version=1,id="job",shape="fanout_review",budget_bytes=131072,roles=L{{id="worker",kind="worker",work=L{"unit"}},{id="a_worker",kind="worker",work=L{"a"}},{id="z_worker",kind="worker",work=L{"z"}},{id="reviewer",kind="reviewer",targets=L{"unit","a","z"}}}}')
        if case == 'vector':
            spec_file = fixture.w/'spec.lua'
            spec_file.write_text(spec_file.read_text().replace('summary=p.md"Consumer",paths=L{"src/"},checks=L{"test"}', 'summary=p.md"Consumer",paths=L{"src/"},checks=L{"integration"}'))
        fixture.load()
        if case == 'delivery_changed':
            (fixture.root/'current/contract.txt').write_text('wrong')
        if case == 'vector':
            (fixture.root/'base/src').mkdir()
            (fixture.root/'base/src/a').write_bytes((fixture.root/'current/src/a').read_bytes())
            for unit in ('a', 'z'):
                done = fixture.check(fixture.claim(unit + '_worker'))
                assert done['outcome'] == 'completed', done
            for unit in ('a', 'z'):
                (fixture.root/'current'/unit/'value').write_text('changed')
        claim = fixture.claim('worker')
        with sqlite3.connect(fixture.root/'ledger.db') as db:
            before = list(db.iterdump())
        result = subprocess.run([str(binary), 'check', '--ledger', str(fixture.root/'ledger.db'), '--run', claim['run']['token'], '--root', str(fixture.root/'current'), '--base-root', str(fixture.root/'base'), '--scratch', str(fixture.root/'scratch'), '--request-id', 'diagnostic', '--actor', 'verification'], capture_output=True, text=True, timeout=30)
        with sqlite3.connect(fixture.root/'ledger.db') as db:
            after = list(db.iterdump())
        wanted = {'code': 'observation_target_mismatch', 'message': 'Wrong output vector', 'field': '', 'details': {'work': 'a'}} if case == 'vector' else {'code': 'revision_conflict', 'message': 'Ledger revision does not match the expected revision', 'field': '', 'details': {'expected': fixture.revision, 'actual': 'working_bytes'}}
        actual = json.loads(result.stderr)['error']
        good = result.returncode == 3 and before == after and actual == wanted
        results.append({'case': case, 'pass': good, 'actual': actual, 'expected': wanted})
        if not good:
            failures.append(case)
    finally:
        fixture.close()
print(json.dumps(results, indent=2))
print(f'{len(results)} scenarios, {len(failures)} failures, 0 skips')
raise SystemExit(bool(failures))
