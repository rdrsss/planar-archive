# C02 native policy regressions. Fresh temporary ledgers only; tokens never logged.
import json, pathlib, subprocess, tempfile, sys
CLI=pathlib.Path(sys.argv[1]).resolve() if len(sys.argv)>1 else pathlib.Path('build/rewrite-debug/bin/planar').resolve()

def readiness(active=True):
    repo=pathlib.Path.cwd();binary=CLI
    scratch=tempfile.TemporaryDirectory(prefix='planar-c02-policy-');root=pathlib.Path(scratch.name).resolve();ledger=root/'ledger.db'
    for n in ['workspace','current','base','scratch']:(root/n).mkdir()
    w=root/'workspace'
    (w/'workspace.lua').write_text('''local p=planar local L=p.list return p.workspace{version=1,id="draft",root={id="source",base="initial",excluded=L{}},spec="spec.lua",jobs=L{{id="job",file="job.lua"}},registry={id="tools",revision="v1"},toolchains=L{"shell"},checks=L{"test"}}''')
    (w/'spec.lua').write_text('''local p=planar local L=p.list return p.spec{version=1,id="draft",goal=p.md"Draft check smoke",work=L{{id="unit",summary=p.md"Work",paths=L{"src/"},checks=L{"test"}}},milestones=L{{id="m",work=L{"unit"},acceptance=L{{id="a",text=p.md"Works"}}}},constraints=L{},prescriptions=L{},contracts=L{},facets=L{},dependencies=L{},frozen=L{}}''')
    (w/'job.lua').write_text('''local p=planar local L=p.list return p.job{version=1,id="job",shape="fanout_review",budget_bytes=131072,roles=L{{id="reviewer",kind="reviewer",targets=L{"unit"}},{id="worker",kind="worker",work=L{"unit"}}}}''')
    registry=root/'registry.json';registry.write_text(json.dumps({'version':1,'id':'tools','revision':'v1','toolchains':[{'id':'shell','description':'Local smoke fixture','command':['/bin/sh'],'environment':[]}],'checks':[{'id':'test','toolchain':'shell','argv':['-c','printf \'{"version":1,"matched":1,"passed":1,"failed":0}\' > "$PLANAR_CHECK_RESULT"'],'coverage':'smoke','inputs':['own_output'],'timeout_ms':10000,'max_output_bytes':1024,'selection':'nonempty'}]}))
    (root/'current/src').mkdir();(root/'current/src/a').write_text('work\n')
    def run(verb,*args):
     p=subprocess.run([str(binary),verb,'--ledger',str(ledger),*map(str,args)],capture_output=True,text=True)
     if p.returncode:raise RuntimeError(verb+' failed '+str(p.returncode)+' '+p.stderr)
     assert not p.stderr,p.stderr
     return json.loads(p.stdout),p.stdout
    run('init','--workspace-id','draft')
    l,_=run('load','--workspace',w/'workspace.lua','--registry',registry,'--expect-revision','none','--request-id','load','--actor','draft')
    run('activate','--job','job','--expect-revision',l['revision'],'--expect-job','NONE','--request-id','activate','--actor','draft')
    claim,_=run('claim','--job','job','--role','worker','--request-id','claim','--actor','draft')

    q=subprocess.run([str(binary),'check','--ledger',str(ledger),'--run',claim['run']['token'],'--root',str(root/'current'),'--base-root',str(root/'base'),'--scratch',str(root/'scratch'),'--request-id','passing','--actor','review'],capture_output=True,text=True)
    assert q.returncode==0,q.stderr
    checked=json.loads(q.stdout)
    original=(w/'spec.lua').read_text()
    (w/'spec.lua').write_text(original.replace('Draft check smoke','Changed goal'))
    run('load','--workspace',w/'workspace.lua','--registry',registry,'--expect-revision','1','--request-id','changed','--actor','review')
    if active:
        worker,_=run('claim','--job','job','--role','worker','--request-id','rework','--actor','review')
    (w/'spec.lua').write_text(original)
    run('load','--workspace',w/'workspace.lua','--registry',registry,'--expect-revision','2','--request-id','restored','--actor','review')
    status,_=run('status')
    print('READINESS',json.dumps([{k:r[k] for k in ['role','state','drifted']} for r in status['roles']]))
    states={r['role']:r for r in status['roles']}
    if not active:
        assert states['worker']['state']=='done'
        assert states['reviewer']['state']=='ready', 'load must derive review readiness from restored worker state'
        run('claim','--job','job','--role','reviewer','--request-id','restored-review','--actor','review')
        return
    assert states['worker']['state']=='running' and states['worker']['drifted']
    assert states['reviewer']['state']=='blocked', 'reviewer must remain blocked while restored worker run is active'
    p=subprocess.run([str(binary),'claim','--ledger',str(ledger),'--job','job','--role','reviewer','--request-id','early-review','--actor','review'],capture_output=True,text=True)
    assert p.returncode!=0 and 'claim_conflict' in p.stderr
    run('refresh','--run',worker['run']['token'],'--expect-fp',worker['dispatch']['work_fp'],'--request-id','refresh-restored','--actor','review')
    run('check','--run',worker['run']['token'],'--root',root/'current','--base-root',root/'base','--scratch',root/'scratch','--request-id','restored-check','--actor','review')
    status,_=run('status')
    assert {r['role']:r['state'] for r in status['roles']}=={'worker':'done','reviewer':'ready'}

def inactive_restoration():
    readiness(False)

def recovery():
    repo=pathlib.Path.cwd();binary=CLI
    scratch=tempfile.TemporaryDirectory(prefix='planar-c02-policy-');root=pathlib.Path(scratch.name).resolve();ledger=root/'ledger.db'
    for n in ['workspace','current','base','scratch']:(root/n).mkdir()
    w=root/'workspace'
    (w/'workspace.lua').write_text('''local p=planar local L=p.list return p.workspace{version=1,id="draft",root={id="source",base="initial",excluded=L{}},spec="spec.lua",jobs=L{{id="job",file="job.lua"}},registry={id="tools",revision="v1"},toolchains=L{"shell"},checks=L{"test"}}''')
    (w/'spec.lua').write_text('''local p=planar local L=p.list return p.spec{version=1,id="draft",goal=p.md"Draft check smoke",work=L{{id="unit",summary=p.md"Work",paths=L{"src/"},checks=L{"test"}}},milestones=L{{id="m",work=L{"unit"},acceptance=L{{id="a",text=p.md"Works"}}}},constraints=L{},prescriptions=L{},contracts=L{},facets=L{},dependencies=L{},frozen=L{}}''')
    (w/'job.lua').write_text('''local p=planar local L=p.list return p.job{version=1,id="job",shape="fanout_review",budget_bytes=131072,roles=L{{id="reviewer",kind="reviewer",targets=L{"unit"}},{id="worker",kind="worker",work=L{"unit"}}}}''')
    registry=root/'registry.json';registry.write_text(json.dumps({'version':1,'id':'tools','revision':'v1','toolchains':[{'id':'shell','description':'Local smoke fixture','command':['/bin/sh'],'environment':[]}],'checks':[{'id':'test','toolchain':'shell','argv':['-c','printf \'{"version":1,"matched":1,"passed":1,"failed":0}\' > "$PLANAR_CHECK_RESULT"'],'coverage':'smoke','inputs':['own_output'],'timeout_ms':10000,'max_output_bytes':1024,'selection':'nonempty'}]}))
    (root/'current/src').mkdir();(root/'current/src/a').write_text('work\n')
    def run(verb,*args):
     p=subprocess.run([str(binary),verb,'--ledger',str(ledger),*map(str,args)],capture_output=True,text=True)
     if p.returncode:raise RuntimeError(verb+' failed '+str(p.returncode)+' '+p.stderr)
     assert not p.stderr,p.stderr
     return json.loads(p.stdout),p.stdout
    run('init','--workspace-id','draft')
    l,_=run('load','--workspace',w/'workspace.lua','--registry',registry,'--expect-revision','none','--request-id','load','--actor','draft')
    run('activate','--job','job','--expect-revision',l['revision'],'--expect-job','NONE','--request-id','activate','--actor','draft')
    claim,_=run('claim','--job','job','--role','worker','--request-id','claim','--actor','draft')

    q=subprocess.run([str(binary),'check','--ledger',str(ledger),'--run',claim['run']['token'],'--root',str(root/'current'),'--base-root',str(root/'base'),'--scratch',str(root/'scratch'),'--request-id','passing','--actor','review'],capture_output=True,text=True)
    assert q.returncode==0,q.stderr
    checked=json.loads(q.stdout)
    reviewer,_=run('claim','--job','job','--role','reviewer','--request-id','review','--actor','review')
    verdict=root/'verdict.json'
    verdict.write_text(json.dumps({'version':1,'targets':[{'work':'unit','output':checked['observation']['units'][0]['output'],'work_fp':checked['completion']['work_fp']}],'policy':reviewer['dispatch']['work_fp'],'decision':'reject','findings':[],'repair':['worker'],'summary':'Repair required'}))
    run('verdict','--run',reviewer['run']['token'],'--verdict',verdict,'--request-id','reject','--actor','review')
    preview,_=run('preview','--role','worker')
    recover,_=run('recover','--role','worker')
    assert preview['work_fp']==recover['current']['work_fp'], 'recovery must use current repair work identity'
    assert preview['work_fp']!=claim['dispatch']['work_fp'], 'repair must change work identity'
    assert preview['manifest']['inputs']['repair']=='1', 'preview repair provenance'
    repair_claim,_=run('claim','--job','job','--role','worker','--request-id','repair-claim','--actor','review')
    assert repair_claim['manifest']['inputs']['repair']=='1', 'claim repair provenance'
    assert repair_claim['dispatch']['work_fp']==preview['work_fp']
    refresh,_=run('refresh','--run',repair_claim['run']['token'],'--expect-fp',repair_claim['dispatch']['work_fp'],'--request-id','refresh-repair','--actor','review')
    assert refresh['manifest']['inputs']['repair']=='1', 'refresh repair provenance'
    assert refresh['dispatch']['work_fp']==preview['work_fp']
    recovered,_=run('recover','--role','worker')
    assert recovered['current']['work_fp']==preview['work_fp']
    assert recovered['acknowledged']['dispatch']['work_fp']==preview['work_fp']
    assert recovered['acknowledged']['manifest']['inputs']['repair']=='1'
    takeover,_=run('takeover','--job','job','--role','worker','--run-id',repair_claim['run']['id'],'--request-id','takeover-repair','--actor','review')
    assert takeover['manifest']['inputs']['repair']=='1', 'takeover repair provenance'
    assert takeover['dispatch']['work_fp']==preview['work_fp']

failures=0
for case in (readiness,inactive_restoration,recovery):
    try:
        case()
        print('PASS',case.__name__)
    except Exception as error:
        failures+=1
        print('FAIL',case.__name__,str(error))
print(f'SUMMARY tests=3 failures={failures} skips=0')
sys.exit(bool(failures))
