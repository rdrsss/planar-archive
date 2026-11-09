# Draft R17/R18 CLI integration exercise. Run from the repository root after building planar.
import json,pathlib,subprocess,tempfile,sys,time,os,signal
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

failures=[]
checks=[]
def verify(name, condition, detail):
 checks.append(name)
 print(name, 'PASS' if condition else 'FAIL', json.dumps(detail), flush=True)
 if not condition: failures.append(name)
def check_args(request):
 return [str(binary),'check','--ledger',str(ledger),'--run',claim['run']['token'],'--root',str(root/'current'),'--base-root',str(root/'base'),'--scratch',str(root/'scratch'),'--request-id',request,'--actor','draft']
# A command rendezvous makes active-reservation observation deterministic.
command='echo $$ > "'+str(root/'child-pid')+'"; touch "'+str(root/'started')+'"; while [ ! -f "'+str(root/'continue')+'" ]; do sleep 0.05; done; exit 1'
data=json.loads(registry.read_text());data['checks'][0]['argv']=['-c',command]
# Reloading the registry before a fresh claim avoids changing a running role's W.
run('release','--reason','released','--run',claim['run']['token'],'--request-id','release','--actor','draft')
registry.write_text(json.dumps(data))
run('load','--workspace',w/'workspace.lua','--registry',registry,'--expect-revision',l['revision'],'--request-id','load-fail','--actor','draft')
claim,_=run('claim','--job','job','--role','worker','--request-id','claim-fail','--actor','draft')
p=subprocess.Popen(check_args('failed-check'),stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,start_new_session=True)
try:
 deadline=time.monotonic()+8
 while not (root/'started').exists() and p.poll() is None and time.monotonic()<deadline: time.sleep(.02)
 assert (root/'started').exists(), 'check did not reach rendezvous'
 def lookup(label):
  q=subprocess.run([str(binary),'status','--ledger',str(ledger),'--request-id','failed-check'],capture_output=True,text=True)
  obj=json.loads(q.stdout) if q.returncode==0 else {'error':q.stderr}
  req=obj.get('request',{})
  verify(label,q.returncode==0 and set(req)=={'request_id','operation','outcome','reservation','run'} and req['operation']=='check' and req['outcome']=='in_progress' and set(req['run'])=={'id','generation'} and req['run']['id']==claim['run']['id'] and req['run']['generation']==claim['run']['generation'],obj)
 lookup('RT2 active reservation')
 (root/'continue').touch()
 out,err=p.communicate(timeout=15);assert p.returncode==0,(out,err)
 failed=json.loads(out)
 completed,_=run('status','--request-id','failed-check')
 verify('RT2 completed request result',set(completed['request'])=={'request_id','operation','outcome','result'} and completed['request']['operation']=='check' and completed['request']['outcome']=='recorded',completed['request'])
 recovered,_=run('recover','--role','worker')
 verify('RT1 evidence preserved',len(recovered['evidence'])==len(failed['evidence']) and any(e['outcome']=='fail' for e in recovered['evidence']),recovered['evidence'])
 # The next invocation is stopped after reservation, leaving only durable rows.
 (root/'continue').unlink();(root/'started').unlink()
 p=subprocess.Popen(check_args('abandoned-check'),stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,start_new_session=True)
 deadline=time.monotonic()+8
 while not (root/'started').exists() and p.poll() is None and time.monotonic()<deadline: time.sleep(.02)
 assert (root/'started').exists()
 os.killpg(p.pid,signal.SIGKILL);p.communicate(timeout=5)
 child_pid=int((root/'child-pid').read_text())
 os.killpg(child_pid,signal.SIGKILL)
 deadline=time.monotonic()+5
 while time.monotonic()<deadline:
  try: os.kill(child_pid,0)
  except ProcessLookupError: break
  time.sleep(.02)
 else: raise AssertionError('check child did not stop')
 (root/'child-pid').unlink()
 q=subprocess.run([str(binary),'status','--ledger',str(ledger),'--request-id','abandoned-check'],capture_output=True,text=True)
 obj=json.loads(q.stdout) if q.returncode==0 else {'error':q.stderr}
 req=obj.get('request',{})
 verify('RT2 abandoned reservation',q.returncode==0 and set(req)=={'request_id','operation','outcome','reservation','run'} and req['outcome']=='in_progress' and set(req['run'])=={'id','generation'},obj)
 status,_=run('status');reservation=next(r['run']['reservation'] for r in status['roles'] if r['role']=='worker')
 run('check-recover','--reservation',reservation,'--run-id',claim['run']['id'],'--confirm-stopped','true','--request-id','recover-check','--actor','draft')
 # A fresh passing registry lets the worker finish and the reviewer reject it.
 run('release','--reason','released','--run',claim['run']['token'],'--request-id','release-fail','--actor','draft')
 data['checks'][0]['argv']=['-c','printf \'{"version":1,"matched":1,"passed":1,"failed":0}\' > "$PLANAR_CHECK_RESULT"'];registry.write_text(json.dumps(data))
 status,_=run('status')
 run('load','--workspace',w/'workspace.lua','--registry',registry,'--expect-revision','2','--request-id','load-pass','--actor','draft')
 claim,_=run('claim','--job','job','--role','worker','--request-id','claim-pass','--actor','draft')
 q=subprocess.run(check_args('passing-check'),capture_output=True,text=True);assert q.returncode==0,q.stderr
 check=json.loads(q.stdout);assert check['outcome']=='completed',check
 reviewer,_=run('claim','--job','job','--role','reviewer','--request-id','review','--actor','draft')
 finding={'id':'repair-one','severity':'must','work':'unit','path':'src/a','line':1,'text':'Repair this output'}
 verdict=root/'verdict.json';verdict.write_text(json.dumps({'version':1,'targets':[{'work':'unit','output':check['observation']['units'][0]['output'],'work_fp':check['completion']['work_fp']}],'policy':reviewer['dispatch']['work_fp'],'decision':'reject','findings':[finding],'repair':['worker'],'summary':'Repair required'}))
 run('verdict','--run',reviewer['run']['token'],'--verdict',verdict,'--request-id','reject','--actor','draft')
 recovered,_=run('recover','--role','worker')
 verify('RT1 findings preserved',recovered['findings']==[{**finding,'verdict':'1'}],recovered['findings'])
finally:
 if p.poll() is None: os.killpg(p.pid,signal.SIGKILL);p.communicate()
 if (root/'child-pid').exists():
  try: os.killpg(int((root/'child-pid').read_text()),signal.SIGKILL)
  except ProcessLookupError: pass
print('SUMMARY',len(checks)-len(failures),'passed',len(failures),'failed')
sys.exit(bool(failures))
