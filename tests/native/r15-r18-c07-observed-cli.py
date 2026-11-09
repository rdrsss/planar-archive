# Current own observations cannot be inherited from consumed units in completion history.
import argparse
import pathlib

parser=argparse.ArgumentParser()
parser.add_argument('binary',type=pathlib.Path)
parser.add_argument('--snapshot-probe',required=True,type=pathlib.Path)
args=parser.parse_args()
source=pathlib.Path('tests/native/r15-r18-c03-vectors-cli.py').read_text()
hook="        print('PASS', label, 'vectors=' + str(vector_assertions), 'accepted=' + str(len(units)), flush=True)"
extra=r'''        def snapshot_lines():
            return subprocess.check_output([str(PROBE),str(ledger)],text=True).splitlines()
        before=snapshot_lines()
        assert any(line.startswith('completion b_worker a ') for line in before)
        with sqlite3.connect(ledger) as db:
            retained=list(db.execute('SELECT id,run_id,seq,dispatch_id,work_fp,observation_id FROM completions ORDER BY id'))
        f=workspace/'job.lua'
        f.write_text(f.read_text().replace('a_worker','a_recreated'))
        loaded,_=run('load','--workspace',workspace/'workspace.lua','--registry',registry,'--expect-revision',loaded['revision'],'--request-id','replace_a','--actor','test')
        after=snapshot_lines()
        assert any(line.startswith('completion b_worker a ') for line in after),after
        assert not any(line.startswith('observed a ') for line in after),after
        assert not any(line.startswith('completion a_worker ') for line in after),after
        with sqlite3.connect(ledger) as db:
            assert list(db.execute('SELECT id,run_id,seq,dispatch_id,work_fp,observation_id FROM completions ORDER BY id'))==retained
        claim,_=run('claim','--job','job','--role','a_recreated','--request-id','fresh_a','--actor','test')
        checked,_=run('check','--run',claim['run']['token'],'--root',current,'--base-root',base,'--scratch',scratch,'--request-id','fresh_a_check','--actor','test')
        assert checked['outcome']=='completed',checked
        own=next(item['output'] for item in checked['observation']['units'] if item['work']=='a')
        assert any(line.startswith('observed a '+own+' ') for line in snapshot_lines())
        # A newer own failed check observation wins over the earlier passing completion.
        spec=workspace/'spec.lua';spec.write_text(spec.read_text().replace('Verify a','Reverify a'))
        loaded,_=run('load','--workspace',workspace/'workspace.lua','--registry',registry,'--expect-revision',loaded['revision'],'--request-id','stale_a','--actor','test')
        claim,_=run('claim','--job','job','--role','a_recreated','--request-id','failed_a','--actor','test')
        (current/'a/value').write_text('99\n')
        failed,_=run('check','--run',claim['run']['token'],'--root',current,'--base-root',base,'--scratch',scratch,'--request-id','failed_a_check','--actor','test')
        assert failed['outcome']!='completed',failed
        newer=next(item['output'] for item in failed['observation']['units'] if item['work']=='a')
        assert newer!=own
        assert any(line.startswith('observed a '+newer+' ') for line in snapshot_lines())
        print('PASS observed owner replacement/new own/failed own/history',label,flush=True)
'''
assert source.count(hook)==1
scope={'__name__':'observed_fixture','PROBE':args.snapshot_probe.resolve()}
exec(compile(source.replace(hook,extra),'observed_fixture','exec'),scope)
scope['workflow'](args.binary.resolve(),False)
print('PASS 4 observed ownership controls; zero skips')
