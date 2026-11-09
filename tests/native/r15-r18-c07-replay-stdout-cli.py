# Real run/recovery replay output failures in embedded and profile compositions.
import pathlib
import sys

source = pathlib.Path('tests/native/r15-r18-c03-vectors-cli.py').read_text()
hook = "        print('PASS', label, 'vectors=' + str(vector_assertions), 'accepted=' + str(len(units)), flush=True)"
extra = r'''        import os
        run('activate','--job','job','--expect-revision',loaded['revision'],'--expect-job','job','--request-id','stdout_activate','--actor','test')
        def mutation(verb,*args):
            request = ('--request-id','stdout_'+verb,'--actor','test')
            value,original = run(verb,*args,*request)
            with sqlite3.connect(ledger) as db: before = list(db.iterdump())
            read_fd,write_fd=os.pipe();os.close(read_fd)
            try:
                replay=subprocess.run([str(binary),*prefix,verb,'--ledger',str(ledger),*map(str,args),*request],stdout=write_fd,stderr=subprocess.PIPE,text=True,timeout=30)
            finally: os.close(write_fd)
            error=json.loads(replay.stderr)['error']
            expected=verb[0].upper()+verb[1:]+' may be complete; retry the same request ID or inspect request history'
            if not (replay.returncode==4 and error['code']=='io' and error['details']['operation']=='stdout' and error['message']==expected):
                failures.append((label,verb,replay.returncode,error))
            _,again=run(verb,*args,*request)
            assert again==original,(label,verb,'replay bytes changed')
            with sqlite3.connect(ledger) as db: assert list(db.iterdump())==before
            return value
        claim=mutation('claim','--job','job','--role','s_worker')
        token=claim['run']['token']
        notes=root/'notes.json'
        notes.write_text(json.dumps(dict(version=1,checks=[],findings=[],checkpoint=dict(progress='checked',next='continue',unresolved='',files=[]))))
        mutation('checkpoint','--run',token,'--observations',notes)
        mutation('refresh','--run',token,'--expect-fp',claim['dispatch']['work_fp'])
        takeover=mutation('takeover','--job','job','--role','s_worker','--run-id',claim['run']['id'])
        mutation('release','--run',takeover['run']['token'],'--reason','released')
        print('PASS replay controls',label,flush=True)
'''
assert source.count(hook) == 1
failures = []
scope = {'__name__': 'stdout_fixture', 'failures': failures}
exec(compile(source.replace(hook, extra), 'stdout_fixture', 'exec'), scope)
for profile in (False, True):
    scope['workflow'](pathlib.Path(sys.argv[1]).resolve(), False, profile)
assert not failures, failures
print('PASS 10 replay stdout scenarios; zero skips')
