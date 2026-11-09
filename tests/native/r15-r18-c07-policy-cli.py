# Real arithmetic workflows for publication withdrawal and job-scoped role identity.
import argparse
import hashlib
import pathlib

SOURCE = pathlib.Path(__file__).with_name('r15-r18-c03-vectors-cli.py').read_text()
HOOK = "        print('PASS', label, 'vectors=' + str(vector_assertions), 'accepted=' + str(len(units)), flush=True)"


def workflow(binary, scenario):
    source = SOURCE
    if scenario == 'withdraw-republish':
        source = source.replace("            if unit == 'b':", "            if unit == 's':\n                assertion = 'test ! -f s/value || test \"$(cat s/value)\" -eq 1'\n            if unit == 'b':")
        extra = r'''
        with sqlite3.connect(ledger) as db:
            retained = db.execute('SELECT * FROM publications ORDER BY id').fetchall()
        spec = workspace/'spec.lua'
        spec.write_text(spec.read_text().replace('Verify s','Verify s again').replace('Verify b','Verify b again'))
        loaded,_=run('load','--workspace',workspace/'workspace.lua','--registry',registry,'--expect-revision',loaded['revision'],'--request-id','change','--actor','test')
        claim,_=run('claim','--job','job','--role','s_worker','--request-id','again_s','--actor','test')
        (current/'s/value').unlink()
        checked,_=run('check','--run',claim['run']['token'],'--root',current,'--base-root',base,'--scratch',scratch,'--request-id','missing_facet','--actor','test')
        assert checked['outcome']=='completed' and checked['completion']['publications']==[],checked
        status,_=run('status'); preview,_=run('preview','--role','b_worker')
        assert next(r['state'] for r in status['roles'] if r['role']=='b_worker')=='blocked',status
        assert next(p['status'] for p in preview['manifest']['prerequisites'] if p['id']=='s_to_b')=='unavailable',preview
        denied=subprocess.run([str(binary),'claim','--ledger',str(ledger),'--job','job','--role','b_worker','--request-id','premature','--actor','test'],capture_output=True,text=True,timeout=30)
        assert denied.returncode==3 and json.loads(denied.stderr)['error']['code']=='claim_conflict',(denied.returncode,denied.stderr)
        spec.write_text(spec.read_text().replace('Verify s again','Verify s restored'))
        loaded,_=run('load','--workspace',workspace/'workspace.lua','--registry',registry,'--expect-revision',loaded['revision'],'--request-id','restore_requirement','--actor','test')
        claim,_=run('claim','--job','job','--role','s_worker','--request-id','restore_s','--actor','test')
        (current/'s/value').write_text('1\n')
        checked,_=run('check','--run',claim['run']['token'],'--root',current,'--base-root',base,'--scratch',scratch,'--request-id','restore_facet','--actor','test')
        assert checked['outcome']=='completed' and len(checked['completion']['publications'])==1,checked
        preview,_=run('preview','--role','b_worker');status,_=run('status')
        assert next(p['status'] for p in preview['manifest']['prerequisites'] if p['id']=='s_to_b')=='available'
        assert next(r['state'] for r in status['roles'] if r['role']=='b_worker') in ('ready','stale')
        with sqlite3.connect(ledger) as db:
            assert db.execute('SELECT * FROM publications ORDER BY id LIMIT ?', (len(retained),)).fetchall()==retained
        print('PASS withdraw-republish history-preserved',flush=True)
'''
    else:
        other = 'aaa' if scenario == 'switch-earlier' else 'zzz'
        setup = '''        (workspace/'other.lua').write_text((workspace/'job.lua').read_text().replace('id="job"','id="%s"'))
        f=workspace/'workspace.lua'
        f.write_text(f.read_text().replace('jobs=L{{id="job",file="job.lua"}}','jobs=L{{id="job",file="job.lua"},{id="%s",file="other.lua"}}'))
''' % (other, other)
        assert source.count('        checks = []') == 1
        source = source.replace('        checks = []', setup + '        checks = []')
        extra = r'''
        with sqlite3.connect(ledger) as db:
            retained_completions=db.execute('SELECT * FROM completions ORDER BY id').fetchall()
            retained_publications=db.execute('SELECT * FROM publications ORDER BY id').fetchall()
        current_job='job'
        for cycle,job in enumerate(OTHER_JOBS):
            run('activate','--job',job,'--expect-revision',loaded['revision'],'--expect-job',current_job,'--request-id','activate_'+str(cycle),'--actor','test')
            current_job=job
            for role,owned in assignments:
                claim,_=run('claim','--job',job,'--role',role,'--request-id','claim_'+str(cycle)+role,'--actor','test')
                args=('--run',claim['run']['token'],'--root',current,'--base-root',base,'--scratch',scratch,'--request-id','check_'+str(cycle)+role,'--actor','test')
                checked,raw=run('check',*args)
                assert checked['outcome']=='completed',(job,role,checked)
                _,replay=run('check',*args);assert replay==raw
            status,_=run('status')
            assert next(r['state'] for r in status['roles'] if r['role']=='reviewer')=='ready',status
            with sqlite3.connect(ledger) as db:
                assert db.execute("SELECT COUNT(*) FROM reservation_state WHERE state='open'").fetchone()[0]==0
        with sqlite3.connect(ledger) as db:
            assert db.execute('SELECT * FROM completions ORDER BY id LIMIT ?', (len(retained_completions),)).fetchall()==retained_completions
            assert db.execute('SELECT * FROM publications ORDER BY id LIMIT ?', (len(retained_publications),)).fetchall()==retained_publications
        print('PASS SCENARIO repeated-shared-role-switches history-preserved',flush=True)
'''.replace('OTHER_JOBS', repr([other, 'job', other, 'job'])).replace('SCENARIO', scenario)
    assert source.count(HOOK) == 1
    namespace = {'__name__': 'c07_fixture'}
    exec(compile(source.replace(HOOK, extra), 'c07_arithmetic_fixture', 'exec'), namespace)
    namespace['workflow'](binary, False)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('binary', type=pathlib.Path)
    args = parser.parse_args()
    binary = args.binary.resolve(strict=True)
    print('binary_sha256=' + hashlib.sha256(binary.read_bytes()).hexdigest(), flush=True)
    failures = 0
    for scenario in ('withdraw-republish', 'switch-earlier', 'switch-later'):
        try:
            workflow(binary, scenario)
        except Exception as error:
            failures += 1
            print('FAIL', scenario, repr(error), flush=True)
    print('SUMMARY scenarios=3 failures=' + str(failures) + ' skips=0', flush=True)
    return int(failures != 0)


if __name__ == '__main__':
    raise SystemExit(main())
