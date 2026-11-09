# Independent real CLI vector workflows; only disposable temporary ledgers are read.
import argparse
import contextlib
import hashlib
import json
import pathlib
import sqlite3
import subprocess
import tempfile


def canonical_value(data):
    """Test-only decoder of the documented canonical_v1 record payload."""
    offset = 0
    def take(size):
        nonlocal offset
        value = data[offset:offset + size]
        assert len(value) == size, 'truncated canonical value'
        offset += size
        return value
    def integer():
        return int.from_bytes(take(8), 'big')
    def value():
        tag = take(1)[0]
        if tag < 2:
            return bool(tag)
        if tag == 2:
            raw = integer()
            return raw if raw < 2**63 else raw - 2**64
        if tag == 3:
            return take(integer()).decode('utf-8')
        if tag == 4:
            return [value() for _ in range(integer())]
        assert tag == 5, ('unexpected canonical tag', tag)
        result = {}
        for _ in range(integer()):
            key = value()
            assert key not in result
            result[key] = value()
        return result
    result = value()
    assert offset == len(data)
    return result


def vector_record(encoded):
    magic = b'PLANAR-RECORD\x01'
    assert encoded.startswith(magic) and encoded[len(magic)] == 3
    offset = len(magic) + 1
    def field():
        nonlocal offset
        size = int.from_bytes(encoded[offset:offset + 8], 'big')
        offset += 8
        result = encoded[offset:offset + size]
        assert len(result) == size
        offset += size
        return result
    assert field() == b'output_vector_v1'
    result = canonical_value(field())
    assert offset == len(encoded)
    return result


def workflow(binary, grouped, with_profile=False):
    label = ('grouped' if grouped else 'split') + ('-profile' if with_profile else '')
    with tempfile.TemporaryDirectory(prefix='planar-c03-vectors-') as temp:
        root = pathlib.Path(temp).resolve()
        workspace, current, base, scratch = [root / name for name in ('workspace', 'current', 'base', 'scratch')]
        for directory in (workspace, current, base, scratch):
            directory.mkdir()
        ledger = root / 'ledger.db'
        prefix = []
        if with_profile:
            (root / 'trusted').mkdir()
            profile = root / 'trusted/profile.json'
            profile.write_text('{"version":1,"modules":[]}')
            prefix = ['with-profile', '--stdlib-profile', str(profile)]
        units = ('a', 'b', 'c', 's', 'z')
        values = dict(a=2, b=4, c=9, s=1, z=3)
        for unit in units:
            for tree in (current, base):
                (tree / unit).mkdir()
                (tree / unit / 'value').write_text(str(values[unit]) + '\n')
        quoted_units = ','.join(json.dumps(unit) for unit in units)
        quoted_checks = ','.join(json.dumps('check_' + unit) for unit in units)
        (workspace / 'workspace.lua').write_text('''local p=planar local L=p.list return p.workspace{version=1,id="vectors",root={id="source",base="initial",excluded=L{}},spec="spec.lua",jobs=L{{id="job",file="job.lua"}},registry={id="tools",revision="v1"},toolchains=L{"shell"},checks=L{''' + quoted_checks + '}}')
        work = ','.join('{id="%s",summary=p.md"Verify %s",paths=L{"%s/"},checks=L{"check_%s"}}' % (u, u, u, u) for u in units)
        (workspace / 'spec.lua').write_text('''local p=planar local L=p.list return p.spec{version=1,id="vectors",goal=p.md"Verify grouped and split dependent arithmetic outputs",work=L{''' + work + '''},milestones=L{{id="m",work=L{''' + quoted_units + '''},acceptance=L{{id="correct",text=p.md"All arithmetic outputs and integration checks are correct"}}}},constraints=L{},prescriptions=L{},contracts=L{},facets=L{{id="a_files",producer="a",paths=L{"a/value"}},{id="z_files",producer="z",paths=L{"z/value"}},{id="s_files",producer="s",paths=L{"s/value"}}},dependencies=L{{id="a_to_b",consumer="b",source="facet",ref="a_files",phase="complete",quality="done"},{id="z_to_c",consumer="c",source="facet",ref="z_files",phase="complete",quality="done"},{id="s_to_b",consumer="b",source="facet",ref="s_files",phase="start",quality="available"}},frozen=L{}}''')
        assignments = [('s_worker', ['s']), ('z_worker', ['z'])]
        assignments += [('group_worker', ['a', 'b', 'c'])] if grouped else [(u + '_worker', [u]) for u in ('a', 'b', 'c')]
        roles = ','.join('{id="%s",kind="worker",work=L{%s}}' % (name, ','.join(json.dumps(u) for u in owned)) for name, owned in assignments)
        (workspace / 'job.lua').write_text('local p=planar local L=p.list return p.job{version=1,id="job",shape="fanout_review",budget_bytes=131072,roles=L{' + roles + ',{id="reviewer",kind="reviewer",targets=L{' + quoted_units + '}}}}')
        checks = []
        for unit in units:
            assertion = 'test "$(cat %s/value)" -eq %s' % (unit, values[unit])
            if unit == 'b':
                assertion = 'test "$(cat b/value)" -eq "$(( $(cat a/value) * 2 ))"'
            if unit == 'c':
                assertion = 'test "$(cat c/value)" -eq "$(( $(cat z/value) * 3 ))"'
            command = 'set -eu; ' + assertion + '; printf \'{"version":1,"matched":1,"passed":1,"failed":0}\' > "$PLANAR_CHECK_RESULT"'
            checks.append(dict(id='check_' + unit, toolchain='shell', argv=['-c', command], coverage='Check arithmetic output for ' + unit, inputs=['own_output', 'output_vector'] if unit in ('b', 'c') else ['own_output'], timeout_ms=10000, max_output_bytes=1024, selection='nonempty'))
        registry = root / 'registry.json'
        registry.write_text(json.dumps(dict(version=1, id='tools', revision='v1', toolchains=[dict(id='shell', description='POSIX arithmetic checks', command=['/bin/sh'], environment=[])], checks=checks)))
        def run(verb, *args):
            result = subprocess.run([str(binary), *prefix, verb, '--ledger', str(ledger), *map(str, args)], capture_output=True, text=True, timeout=30)
            assert result.returncode == 0, (label, verb, result.returncode, result.stderr)
            assert result.stderr == '', (label, verb, result.stderr)
            return json.loads(result.stdout), result.stdout
        run('init', '--workspace-id', 'vectors')
        loaded, _ = run('load', '--workspace', workspace / 'workspace.lua', '--registry', registry, '--expect-revision', 'none', '--request-id', 'load', '--actor', 'test')
        run('activate', '--job', 'job', '--expect-revision', loaded['revision'], '--expect-job', 'NONE', '--request-id', 'activate', '--actor', 'test')
        targets = {}
        vector_assertions = 0
        for role, owned in assignments:
            claim, _ = run('claim', '--job', 'job', '--role', role, '--request-id', 'claim_' + role, '--actor', 'test')
            args = ('--run', claim['run']['token'], '--root', current, '--base-root', base, '--scratch', scratch, '--request-id', 'check_' + role, '--actor', 'test')
            checked, raw = run('check', *args)
            assert checked['outcome'] == 'completed', (label, role, checked['outcome'], checked['withheld'])
            _, replay = run('check', *args)
            assert replay == raw, (label, role, 'check replay differs')
            observed = {item['work']: item['output'] for item in checked['observation']['units']}
            wanted_observed = set(owned)
            if 'b' in owned:
                wanted_observed.add('a')
            if 'c' in owned:
                wanted_observed.add('z')
            assert set(observed) == wanted_observed, (label, role, 'observation membership', sorted(observed), sorted(wanted_observed))
            with contextlib.closing(sqlite3.connect(ledger.as_uri() + '?mode=ro', uri=True)) as database:
                rows = database.execute('SELECT e.work,e."check",r.encoded FROM evidence e JOIN records r ON r.id=e.vector_id WHERE reservation_id=? ORDER BY e.work,e."check"', (int(checked['reservation']),)).fetchall()
                publications = database.execute('SELECT work,facet,payload_id FROM publications WHERE completion_id=? ORDER BY work,facet', (int(checked['completion']['id']),)).fetchall()
                expected_facets = {unit for unit in owned if unit in ('a', 's', 'z')}
                assert {unit for unit, _, _ in publications} == expected_facets
                def retained_files(record_id, relative=''):
                    kind, encoded = database.execute('SELECT kind,encoded FROM records WHERE id=?', (record_id,)).fetchone()
                    if kind == 1:
                        header = b'PLANAR-RECORD\x01\x01'
                        assert encoded.startswith(header)
                        mode_offset = len(header)
                        assert int.from_bytes(encoded[mode_offset:mode_offset + 4], 'big') == 420
                        length_offset = mode_offset + 4
                        size = int.from_bytes(encoded[length_offset:length_offset + 8], 'big')
                        payload = encoded[length_offset + 8:]
                        assert len(payload) == size
                        return {relative: payload}
                    assert kind == 2
                    children = database.execute('SELECT name,child_id FROM tree_edges WHERE parent_id=? ORDER BY ordinal', (record_id,)).fetchall()
                    assert children, ('empty retained facet tree', relative)
                    files = {}
                    for name, child_id in children:
                        files.update(retained_files(child_id, relative + '/' + name if relative else name))
                    return files
                for unit, facet, payload_id in publications:
                    assert facet == unit + '_files'
                    assert retained_files(payload_id) == {unit + '/value': (str(values[unit]) + '\n').encode()}

            assert len(rows) == 2 * len(owned), (label, role, 'missing evidence')
            for unit, check_id, encoded in rows:
                vector = vector_record(encoded)
                expected = {unit}
                if check_id == 'check_b':
                    expected.add('a')
                if check_id == 'check_c':
                    expected.add('z')
                wanted = [{'work': item, 'output': observed[item]} for item in sorted(expected)]
                assert vector == {'version': 1, 'targets': wanted}, (label, unit, check_id, vector, wanted)
                vector_assertions += 1
            for unit in owned:
                targets[unit] = dict(work=unit, output=observed[unit], work_fp=checked['completion']['work_fp'])
        status, _ = run('status')
        assert all(row['state'] == ('ready' if row['role'] == 'reviewer' else 'done') for row in status['roles'])
        reviewer, _ = run('claim', '--job', 'job', '--role', 'reviewer', '--request-id', 'review', '--actor', 'test')
        verdict = root / 'verdict.json'
        verdict.write_text(json.dumps(dict(version=1, targets=[targets[u] for u in units], policy=reviewer['dispatch']['work_fp'], decision='approve', findings=[], repair=[], summary='Independent arithmetic integration verification')))
        _, first = run('verdict', '--run', reviewer['run']['token'], '--verdict', verdict, '--request-id', 'approve', '--actor', 'test')
        _, replay = run('verdict', '--run', reviewer['run']['token'], '--verdict', verdict, '--request-id', 'approve', '--actor', 'test')
        assert first == replay
        status, _ = run('status')
        acceptance = status['review']['acceptance']
        assert {row['work'] for row in acceptance if row['accepted']} == set(units), (label, acceptance)
        assert all(row['state'] == 'done' for row in status['roles'])
        print('PASS', label, 'vectors=' + str(vector_assertions), 'accepted=' + str(len(units)), flush=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('binary', type=pathlib.Path)
    args = parser.parse_args()
    binary = args.binary.resolve(strict=True)
    print('binary_sha256=' + hashlib.sha256(binary.read_bytes()).hexdigest(), flush=True)
    failures = 0
    for grouped, with_profile in ((True, False), (False, False), (True, True)):
        try:
            workflow(binary, grouped, with_profile)
        except Exception as error:
            failures += 1
            print('FAIL', ('grouped' if grouped else 'split') + ('-profile' if with_profile else ''), repr(error), flush=True)
    print('SUMMARY scenarios=3 failures=' + str(failures) + ' skips=0', flush=True)
    return int(failures != 0)


if __name__ == '__main__':
    raise SystemExit(main())
