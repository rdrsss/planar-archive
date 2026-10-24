"""Actual source-built recorded history in native-accepted scratch ledgers."""
from pathlib import Path
import hashlib
import json
import os
import sqlite3
import subprocess
import sys
import tempfile

binary, fixture = map(lambda p: str(Path(p).resolve()), sys.argv[1:3])
count = 0
with tempfile.TemporaryDirectory(prefix='planar-history-cli-') as temporary:
    root = Path(temporary).resolve()
    home = root / 'HOME'
    home.mkdir()
    env = dict(os.environ, HOME=str(home), PLANAR_DB=str(home / 'NEVER_LEGACY'))
    def snapshot(path):
        return {str(p.relative_to(path)): hashlib.sha256(p.read_bytes()).hexdigest() if p.is_file() else 'directory'
                for p in path.rglob('*')}
    def run(args, status=0, code=None):
        global count
        count += 1
        before = snapshot(root)
        result = subprocess.run([binary, *args], env=env, capture_output=True, timeout=40)
        assert snapshot(root) == before, args
        assert result.returncode == status, (args, result.returncode, result.stderr[:1000])
        assert b'PRIVATE_INTENT' not in result.stdout + result.stderr
        assert str(root).encode() not in result.stdout + result.stderr
        if status:
            assert not result.stdout
            value = json.loads(result.stderr)
            assert set(value) == {'version', 'kind', 'error'}
            assert value['version'] == 1 and value['kind'] == 'error'
            assert value['error']['code'] == code, value
            assert set(value['error']) in ({'code','message','field'}, {'code','message','field','details'})
            assert value['error']['field'] == ''
            return value
        assert not result.stderr and result.stdout.endswith(b'\n')
        assert len(result.stdout) <= 16777216
        value = json.loads(result.stdout)
        assert set(value) == {'version','kind','ledger','active_revision','last_event','data'}
        assert value['version'] == 1 and value['kind'] == 'history'
        assert 'null' not in result.stdout.decode()
        return value
    def setup(mode):
        path = root / mode
        path.mkdir()
        subprocess.run([fixture, str(path), mode], check=True, env=env, timeout=60)
        return path / 'ledger'
    empty = setup('empty')
    ledger = setup('history')
    base = ['history', '--ledger', str(ledger)]
    expected_interpretation = json.loads((Path(__file__).resolve().parents[1] / 'native/r13c2-fixtures/interpretation.json').read_text())
    connection = sqlite3.connect(ledger.as_uri() + '?mode=ro&immutable=1', uri=True)
    expected_results = {key: json.loads(saved) for key, saved in connection.execute('SELECT request_id,result FROM requests')}
    expected_payloads = {str(seq): json.loads(payload) for seq, payload in connection.execute('SELECT seq,payload FROM events')}
    connection.close()
    e = run(['history', '--ledger', str(empty)])
    assert e['active_revision'] == 'none' and e['last_event'] == '1'
    assert len(e['data']['events']) == 1 and set(e['data']) == {'selector','events'}
    state = run(['history','--ledger',str(empty),'--at-event','1'])['data']
    assert state['revision'] == 'none' and state['state'] == {'active_revision':'none'}
    page = run(base)
    assert page['active_revision'] == '3' and page['last_event'] == '5'
    assert [e['sequence'] for e in page['data']['events']] == ['1','2','3','4','5']
    assert page['data']['selector'] == 'page'
    for event in page['data']['events']:
        assert set(event) == {'sequence','type','version','revision','request_id','actor','occurred_at','payload'}
        assert set(event['payload']) == {'version','before','after','reason','references'}
        assert event['payload'] == expected_payloads[event['sequence']]
    first = run(base + ['--limit','2'])
    assert first['data']['next_after'] == '2'
    second = run(base + ['--after-event','2','--limit','2'])
    assert second['data']['next_after'] == '4'
    last = run(base + ['--after-event','4','--limit','2'])
    assert [e['sequence'] for e in last['data']['events']] == ['5'] and 'next_after' not in last['data']
    for after in ['5','6','9223372036854775807']:
        assert run(base + ['--after-event',after])['data'] == {'selector':'page','events':[]}
    assert len(run(base + ['--limit','1'])['data']['events']) == 1
    assert len(run(base + ['--limit','100'])['data']['events']) == 5
    revisions = []
    for number in ['1','2','3']:
        result = run(base + ['--revision',number])
        d = result['data']
        assert set(d) == {'selector','revision','interpretation','versions'}
        assert d['selector'] == 'revision' and d['revision']['id'] == number
        assert set(d['revision']) == {'id','parent','source','registry','stdlib','interpretation','versions','actor','accepted_at'}
        assert d['interpretation']['versions'] == d['versions']
        expected_interpretation['versions']['stdlib_digest'] = d['versions']['stdlib_digest']
        assert d['interpretation'] == expected_interpretation
        revisions.append(d['revision'])
    assert revisions[0]['source'] == revisions[2]['source'] != revisions[1]['source']
    assert [r['parent'] for r in revisions] == ['none','1','2']
    for key, revision, event, outcome in [('first','1','2','accepted'),('noop','1','3','noop'),('changed','2','4','accepted'),('reverted','3','5','accepted')]:
        d = run(base + ['--request-id',key])['data']
        assert set(d) == {'selector','result'} and d['selector'] == 'request'
        assert d['result']['revision'] == revision and d['result']['event'] == event and d['result']['outcome'] == outcome
        assert set(d['result']) == {'version','kind','ledger','request_id','outcome','revision','parent','event'}
        assert d['result'] == expected_results[key]
    for n, revision in enumerate(['none','1','1','2','3'],1):
        state = run(base + ['--at-event',str(n)])
        assert state['active_revision'] == '3' and state['last_event'] == '5'
        d = state['data']
        assert set(d) == {'selector','event','state','revision'} and d['selector'] == 'state'
        assert d['state'] == {'active_revision':revision}
        assert d['revision'] == 'none' if revision == 'none' else d['revision']['id'] == revision
    for flag, code in [('--revision','revision_not_found'),('--at-event','event_not_found'),('--request-id','request_not_found')]:
        run(base + [flag,'999'],3,code)
    for flag in ['--revision','--at-event','--after-event','--limit']:
        for bad in ['0','01','+1','-1','none','1.0','1x','9223372036854775808','', ' 1']:
            run(base + [flag,bad],2,'usage')
    for bad in ['101','1000']:
        run(base + ['--limit',bad],2,'usage')
    for bad in ['', 'bad key','a/b','é','a'*129]:
        run(base + ['--request-id',bad],2,'usage')
    for flag in ['--ledger','--revision','--request-id','--at-event','--after-event','--limit']:
        value = str(ledger) if flag == '--ledger' else '1'
        run(base + [flag],2,'usage')
        run(base + [flag + '=' + value],2,'usage')
        args = ['history'] if flag == '--ledger' else base
        run(args + [flag,value,flag,value],2,'usage')
    selectors = ['--revision','--request-id','--at-event']
    for i, selector in enumerate(selectors):
        for conflict in selectors[i+1:] + ['--after-event','--limit']:
            run(base + [selector,'1',conflict,'1'],2,'usage')
    for extra in [['x'],['--'],['--help'],['--unknown','1'],['init'],['render']]:
        run(base + extra,2,'usage')
    run(['history'],2,'usage')
    run(['history','--revision','1'],2,'usage')
    run(['history','--ledger',str(root/'missing')],3,'ledger_not_found')
    unknown = setup('unknown')
    u = ['history','--ledger',str(unknown)]
    run(u + ['--revision','1'],3,'unsupported_interpretation')
    run(u)
    run(u + ['--request-id','first'])
    run(u + ['--at-event','2'])
    oversized = setup('oversize')
    cap = run(['history','--ledger',str(oversized),'--revision','1'],3,'resource_limit')
    assert cap['error']['details'] == {'limit':'output_bytes','maximum':16777216}, cap
    run(['history','--ledger',str(oversized)])
    assert not list(home.iterdir())
print(f'{count} actual history CLI processes passed; scratch files and membership unchanged on every invocation')
