# Real format3 profile acceptance, binding, replay and history regressions.
import argparse, json, os, pathlib, sqlite3, subprocess, tempfile, time


def fixture(root):
    workspace = root / 'workspace'
    workspace.mkdir()
    (workspace / 'workspace.lua').write_text('''local p=planar local L=p.list return p.workspace{version=1,id="profile_test",root={id="source",base="initial",excluded=L{}},spec="spec.lua",jobs=L{{id="job",file="job.lua"}},registry={id="tools",revision="v1"},toolchains=L{"shell"},checks=L{"test"}}''')
    (workspace / 'spec.lua').write_text('''local p=planar local L=p.list return p.spec{version=1,id="profile_test",goal=p.md"Original goal",work=L{{id="unit",summary=p.md"Work",paths=L{"src/"},checks=L{"test"}}},milestones=L{{id="m",work=L{"unit"},acceptance=L{{id="a",text=p.md"Works"}}}},constraints=L{},prescriptions=L{},contracts=L{},facets=L{},dependencies=L{},frozen=L{}}''')
    (workspace / 'job.lua').write_text('''local p=planar local L=p.list return p.job{version=1,id="job",shape="fanout_review",budget_bytes=131072,roles=L{{id="worker",kind="worker",work=L{"unit"}},{id="reviewer",kind="reviewer",targets=L{"unit"}}}}''')
    (root/'registry').mkdir();(root/'trusted').mkdir();(root/'data').mkdir()
    registry = root / 'registry/tools.json'
    registry.write_text(json.dumps(dict(version=1,id='tools',revision='v1',toolchains=[dict(id='shell',description='Fixture',command=['/bin/sh'],environment=[])],checks=[dict(id='test',toolchain='shell',argv=['-c','printf \'{"version":1,"matched":1,"passed":1,"failed":0}\' > "$PLANAR_CHECK_RESULT"'],coverage='Fixture',inputs=['own_output'],timeout_ms=10000,max_output_bytes=1024,selection='nonempty')])))
    profile = root / 'trusted/profile.json'
    profile.write_text(json.dumps(dict(version=1,modules=['extension.lua'])))
    (profile.parent / 'extension.lua').write_text('-- nonempty captured trusted profile module\nlocal marker = "v1"\n')
    return workspace, registry, profile, root / 'data/ledger.db'


def invoke(binary, profile, ledger, verb, *args, expected=0, env=None):
    command = [str(binary), 'with-profile', '--stdlib-profile', str(profile), verb, '--ledger', str(ledger), *map(str,args)]
    result = subprocess.run(command,capture_output=True,text=True,timeout=40,env=env)
    assert result.returncode == expected, (verb,result.returncode,result.stderr)
    if expected == 0:
        assert not result.stderr,result.stderr
        return json.loads(result.stdout),result.stdout
    return json.loads(result.stderr),result.stderr


def workflow(binary):
    with tempfile.TemporaryDirectory(prefix='planar-c04-profile-') as temp:
        root=pathlib.Path(temp).resolve(); w,registry,profile,ledger=fixture(root)
        run=lambda verb,*args,**kw:invoke(binary,profile,ledger,verb,*args,**kw)
        run('init','--workspace-id','profile_test')
        event,_=run('history','--event','1')
        assert event['data']['selector']=='event',event
        initial=['--workspace',w/'workspace.lua','--registry',registry,'--expect-revision','none','--request-id','first','--actor','test']
        loaded,first=run('load',*initial)
        assert loaded['revision']=='1',loaded
        no_op,_=run('load','--workspace',w/'workspace.lua','--registry',registry,'--expect-revision','1','--request-id','no-op','--actor','test')
        assert no_op['revision']=='1'
        run('export','--revision','1','--target',root/'exported')
        assert (root/'exported/spec.lua').read_bytes()==(w/'spec.lua').read_bytes()
        with sqlite3.connect(ledger) as db:
            raw=[row[0] for row in db.execute('SELECT encoded FROM records WHERE kind=1')]
            assert any((profile.parent/'extension.lua').read_bytes() in value for value in raw)
            assert any(profile.read_bytes() in value for value in raw)
        activated,_=run('activate','--job','job','--expect-revision','1','--expect-job','NONE','--request-id','activate','--actor','test')
        claimed,_=run('claim','--job','job','--role','worker','--request-id','claim','--actor','test')
        embedded=subprocess.run([str(binary),'preview','--ledger',str(ledger),'--role','worker'],capture_output=True,text=True,timeout=40)
        assert embedded.returncode==3 and json.loads(embedded.stderr)['error']['code']=='interpretation_mismatch'
        with sqlite3.connect(ledger) as db:
            seq=db.execute("SELECT seq FROM events WHERE type='run_claimed'").fetchone()[0]
        history,_=run('history','--event',seq)
        assert history['data']['selector']=='event' and history['data']['execution']['packet']['markdown']
        def safe(value):
            if isinstance(value,dict):
                assert not {'token','secret'} & value.keys()
                for child in value.values():safe(child)
            elif isinstance(value,list):
                for child in value:safe(child)
        safe(history)
        (w/'spec.lua').write_text((w/'spec.lua').read_text().replace('Original goal','Changed goal'))
        run('load','--workspace',w/'workspace.lua','--registry',registry,'--expect-revision','1','--request-id','material','--actor','test')
        status,_=run('status');assert next(row for row in status['roles'] if row['role']=='worker')['drifted']
        (profile.parent/'extension.lua').write_text('-- changed trusted profile\nlocal marker = "v2"\n')
        refusal,_=run('preview','--role','worker',expected=3)
        assert refusal['error']['code']=='interpretation_mismatch',refusal
        run('load','--workspace',w/'workspace.lua','--registry',registry,'--expect-revision','2','--request-id','profile-change','--actor','test')
        run('refresh','--run',claimed['run']['token'],'--expect-fp',claimed['dispatch']['work_fp'],'--request-id','refresh','--actor','test')
        profile.unlink();(profile.parent/'extension.lua').unlink();(w/'workspace.lua').unlink();registry.unlink()
        _,replayed=run('load',*initial);assert replayed==first
        changed=initial.copy();changed[-1]='different'
        refusal,_=run('load',*changed,expected=3);assert refusal['error']['code']=='request_conflict'
        print('PASS profile workflow, provenance, history, reconcile, no-op and early replay')


def paused_load(binary, root, profile, ledger, args):
    reached=root/'reached';gate=root/'gate';env=os.environ.copy()
    env.update(X07_REACHED_AFTER_PREPARE_LOAD=str(reached),X07_PAUSE_AFTER_PREPARE_LOAD=str(gate))
    p=subprocess.Popen([str(binary),'with-profile','--stdlib-profile',str(profile),'load','--ledger',str(ledger),*map(str,args)],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,env=env)
    deadline=time.monotonic()+40
    while not reached.exists():
        if p.poll() is not None:
            out,err=p.communicate();raise AssertionError(('prepare failed',p.returncode,out,err))
        if time.monotonic()>deadline:
            p.kill();p.communicate();raise AssertionError('preparation pause not reached')
        time.sleep(.01)
    return p,gate


def races(binary,variant):
    for mode in ['equal','conflict','receipt','cas']:
        with tempfile.TemporaryDirectory(prefix='planar-c04-race-') as temp:
            root=pathlib.Path(temp).resolve();w,registry,profile,ledger=fixture(root)
            invoke(binary,profile,ledger,'init','--workspace-id','profile_test')
            args=['--workspace',w/'workspace.lua','--registry',registry,'--expect-revision','none','--request-id','race','--actor','test']
            p,gate=paused_load(variant,root,profile,ledger,args)
            try:
                if mode in ['equal','conflict','cas']:
                    winner=args.copy()
                    if mode=='conflict':winner[-1]='other'
                    if mode=='cas':winner[-3]='winner'
                    _,raw=invoke(binary,profile,ledger,'load',*winner)
                if mode in ['equal','receipt']:
                    (w/'spec.lua').write_text((w/'spec.lua').read_text()+'\n-- changed after preparation\n')
                gate.touch();out,err=p.communicate(timeout=40)
                if mode=='equal':assert p.returncode==0 and out==raw and not err,(p.returncode,err)
                else:
                    assert p.returncode==3,(mode,p.returncode,err)
                    code=json.loads(err)['error']['code']
                    assert code=={'conflict':'request_conflict','receipt':'capture_changed','cas':'revision_conflict'}[mode],(mode,code)
                with sqlite3.connect(ledger) as db:
                    assert db.execute('SELECT count(*) FROM revisions').fetchone()[0]==(0 if mode=='receipt' else 1)
                print('PASS profile late',mode)
            finally:
                if p.poll() is None:p.kill();p.communicate()

if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('binary',type=pathlib.Path);parser.add_argument('--variant',type=pathlib.Path);args=parser.parse_args()
    workflow(args.binary.resolve())
    if args.variant:races(args.binary.resolve(),args.variant.resolve())
    print('SUMMARY scenarios='+str(5 if args.variant else 1)+' failures=0 skips=0')
