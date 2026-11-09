# Draft R17/R18 CLI integration exercise. Run from the repository root after building planar.
import json,pathlib,subprocess,tempfile,sys
repo=pathlib.Path.cwd();binary=pathlib.Path(sys.argv[1]).resolve() if len(sys.argv)>1 else repo/'build/rewrite-debug/bin/planar'
scratch=tempfile.TemporaryDirectory(prefix='planar-r17-smoke-');root=pathlib.Path(scratch.name).resolve();ledger=root/'ledger.db'
for n in ['workspace','current','base','scratch']:(root/n).mkdir()
w=root/'workspace'
(w/'workspace.lua').write_text('''local p=planar local L=p.list return p.workspace{version=1,id="draft",root={id="source",base="initial",excluded=L{}},spec="spec.lua",jobs=L{{id="job",file="job.lua"}},registry={id="tools",revision="v1"},toolchains=L{"shell"},checks=L{"test"}}''')
(w/'spec.lua').write_text('''local p=planar local L=p.list return p.spec{version=1,id="draft",goal=p.md"Draft check smoke",work=L{{id="unit",summary=p.md"Work",paths=L{"src/"},checks=L{"test"}}},milestones=L{{id="m",work=L{"unit"},acceptance=L{{id="a",text=p.md"Works"}}}},constraints=L{},prescriptions=L{},contracts=L{},facets=L{},dependencies=L{},frozen=L{}}''')
(w/'job.lua').write_text('''local p=planar local L=p.list return p.job{version=1,id="job",shape="fanout_review",budget_bytes=131072,roles=L{{id="worker",kind="worker",work=L{"unit"}},{id="reviewer",kind="reviewer",targets=L{"unit"}}}}''')
registry=root/'registry.json';registry.write_text(json.dumps({'version':1,'id':'tools','revision':'v1','toolchains':[{'id':'shell','description':'Local smoke fixture','command':['/bin/sh'],'environment':[]}],'checks':[{'id':'test','toolchain':'shell','argv':['-c','printf \'{"version":1,"matched":1,"passed":1,"failed":0}\' > "$PLANAR_CHECK_RESULT"'],'coverage':'smoke','inputs':['own_output'],'timeout_ms':10000,'max_output_bytes':1024,'selection':'nonempty'}]}))
(root/'current/src').mkdir();(root/'current/src/a').write_text('work\n')
def run(verb,*args):
 p=subprocess.run([str(binary),verb,'--ledger',str(ledger),*map(str,args)],capture_output=True,text=True)
 if p.returncode:raise RuntimeError(verb+' failed '+str(p.returncode)+' '+p.stderr)
 assert not p.stderr,p.stderr
 print(verb,'PASS',flush=True);return json.loads(p.stdout),p.stdout
run('init','--workspace-id','draft')
l,_=run('load','--workspace',w/'workspace.lua','--registry',registry,'--expect-revision','none','--request-id','load','--actor','draft')
run('activate','--job','job','--expect-revision',l['revision'],'--expect-job','NONE','--request-id','activate','--actor','draft')
claim,_=run('claim','--job','job','--role','worker','--request-id','claim','--actor','draft')
check,raw=run('check','--run',claim['run']['token'],'--root',root/'current','--base-root',root/'base','--scratch',root/'scratch','--request-id','check','--actor','draft');assert check['outcome']=='completed',check
_,replay=run('check','--run',claim['run']['token'],'--root',root/'current','--base-root',root/'base','--scratch',root/'scratch','--request-id','check','--actor','draft');assert raw==replay
status,_=run('status');assert any(r['role']=='worker' and r['state']=='done' for r in status['roles'])
reviewer,_=run('claim','--job','job','--role','reviewer','--request-id','review','--actor','draft')
verdict=root/'verdict.json';verdict.write_text(json.dumps({'version':1,'targets':[{'work':'unit','output':check['observation']['units'][0]['output'],'work_fp':check['completion']['work_fp']}],'policy':reviewer['dispatch']['work_fp'],'decision':'approve','findings':[],'repair':[],'summary':'Draft smoke approval'}))
v,vr=run('verdict','--run',reviewer['run']['token'],'--verdict',verdict,'--request-id','verdict','--actor','draft')
_,vrr=run('verdict','--run',reviewer['run']['token'],'--verdict',verdict,'--request-id','verdict','--actor','draft');assert vr==vrr
status,_=run('status');assert status['review']['acceptance'][0]['accepted'],status
print('R17/R18 check, replay, reviewer approval, acceptance PASS',flush=True)
