"""Public load workflow, exact replay and isolated deterministic completion probes."""
from pathlib import Path
import concurrent.futures,json,os,shutil,sqlite3,subprocess,sys,tempfile,time
binary,probe=map(lambda p:str(Path(p).resolve()),sys.argv[1:])
repo=Path(__file__).resolve().parents[2]
count=0
with tempfile.TemporaryDirectory(prefix='planar-load-') as temp:
 root=Path(temp).resolve();env={**os.environ,'HOME':str(root),'PLANAR_DB':str(root/'forbidden-legacy.db')}
 env.pop('PLANAR_WORKSPACE_DIR',None)
 def fixture(name):
  p=root/name;shutil.copytree(repo/'fixtures/render/quota',p);return p
 def run(args,status=0,code=None,exe=binary,environ=None,closed=False):
  global count
  if closed:
   read,write=os.pipe();os.close(read)
   with os.fdopen(write,'wb') as out:
    p=subprocess.run([exe,*args],stdout=out,stderr=subprocess.PIPE,env=environ or env,cwd=root,timeout=40)
  else:p=subprocess.run([exe,*args],capture_output=True,env=environ or env,cwd=root,timeout=40)
  count+=1
  assert p.returncode==status,(args,p.returncode,p.stdout,p.stderr)
  data=p.stdout if status==0 else p.stderr
  if not closed: assert not (p.stderr if status==0 else p.stdout),(args,p.stdout,p.stderr)
  value=json.loads(data)
  assert data==json.dumps(value,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()+b'\n'
  assert str(root).encode() not in data and str(repo).encode() not in data
  if code:assert value['error']['code']==code,(args,value)
  return data,value
 def initialize(name):
  ledger=root/(name+'.db');run(['init','--ledger',str(ledger),'--workspace-id','quota']);return ledger
 def args(ledger,workspace,key,head='none',actor='driver',registry=None):
  return ['load','--ledger',str(ledger),'--workspace',str(workspace/'workspace/workspace.lua'),'--registry',str(registry or workspace/'registry/quota-tools.json'),'--expect-revision',head,'--request-id',key,'--actor',actor]
 def rows(ledger):
  with sqlite3.connect(ledger) as db:
   return tuple(db.execute('SELECT COUNT(*) FROM '+name).fetchone()[0] for name in ['revisions','requests','events'])
 def saved(ledger,key):
  with sqlite3.connect(ledger) as db:return db.execute('SELECT result FROM requests WHERE request_id=?',(key,)).fetchone()[0]+b'\n'
 def history(ledger,key):return run(['history','--ledger',str(ledger),'--request-id',key])[1]
 def wait(path,process):
  end=time.monotonic()+15
  while not path.exists():
   assert process.poll() is None,(process.returncode,process.communicate())
   assert time.monotonic()<end,path
   time.sleep(.005)
 def finish(process):
  out,err=process.communicate(timeout=30);assert process.returncode==0,(out,err);assert not err;return out
 # D8: public acceptance chronology, preserved historical interpretation/provenance and exact export.
 workspace=fixture('chronology');ledger=initialize('chronology');spec=workspace/'workspace/spec.lua'
 original=spec.read_bytes();os.chmod(spec,0o755)
 # Unselected source files never enter the declaration closure or its exported source.
 (workspace/'workspace/unrelated.txt').write_bytes(b'not declared')
 expected={str(p.relative_to(workspace/'workspace')):(p.read_bytes(),0o755 if p.stat().st_mode&0o111 else 0o644)
           for p in (workspace/'workspace').rglob('*') if p.is_file() and p.name!='unrelated.txt'}
 def rendered():
  return run(['render','--workspace',str(workspace/'workspace/workspace.lua'),'--registry',str(workspace/'registry/quota-tools.json'),
              '--root','source='+str(workspace/'current'),'--job','combined','--role','builder'])[1]
 def historical(flag,value,exe=binary):return run(['history','--ledger',str(ledger),flag,value],exe=exe)[1]
 responses=[];revisions=[];works=[];packets=[]
 for index,label in enumerate(['initial','identical','comment','prose','semantic','restore']):
  if label=='comment':spec.write_bytes(original+b'\n-- exact authored comment\n')
  if label=='prose':spec.write_bytes(original.replace(b'A tiny in-memory quota',b'A deliberately small in-memory quota'))
  if label=='semantic':spec.write_bytes(original.replace(b'Implement Quota.try_take and library tests.',b'Implement Quota.try_take with additional library tests.'))
  if label=='restore':spec.write_bytes(original)
  revision=['1','1','2','3','4','5'][index];parent=['none','none','1','2','3','4'][index]
  raw,result=run(args(ledger,workspace,'chronology-'+label,'none' if index==0 else ['1','1','2','3','4'][index-1]))
  assert result=={'version':1,'kind':'load','ledger':result['ledger'],'request_id':'chronology-'+label,
                 'outcome':'noop' if label=='identical' else 'accepted','revision':revision,'parent':parent,'event':str(index+2)}
  assert raw==saved(ledger,'chronology-'+label)
  responses.append(raw);revisions.append(historical('--revision',revision)['data'])
  packet=rendered();works.append(packet['manifest']['work']['digest']);packets.append(packet)
  event=historical('--at-event',str(index+2))['data']
  assert event['state']=={'active_revision':revision} and event['revision']['id']==revision
  assert history(ledger,'chronology-'+label)['data']['result']==result
 assert rows(ledger)==(5,6,7)
 assert len(set(works[:4]))==1 and works[4]!=works[0] and works[5]==works[0]
 assert packets[0]==packets[1] and packets[3]!=packets[0] and packets[5]==packets[0]
 refs=[d['revision'] for d in revisions]
 assert refs[0]['source']==refs[1]['source']==refs[5]['source']
 assert len({refs[n]['source'] for n in [0,2,3,4]})==4
 assert len({r['registry'] for r in refs})==1 and len({r['stdlib'] for r in refs})==1
 original_revision=historical('--revision','1')['data'];original_event=historical('--at-event','2')['data']
 original_request=history(ledger,'chronology-initial')['data']
 shutil.rmtree(workspace)
 for executable in [binary,probe]:
  # Probe embeds a changed retained load policy; recorded history/export must not interpret it.
  assert historical('--revision','1',executable)['data']==original_revision
  assert historical('--at-event','2',executable)['data']==original_event
  assert historical('--request-id','chronology-initial',executable)['data']==original_request
  assert run(args(ledger,workspace,'chronology-initial'),exe=executable)[0]==responses[0]
  target=root/('historical-export-'+Path(executable).name)
  exported=run(['export','--ledger',str(ledger),'--revision','1','--target',str(target)],exe=executable)[1]
  assert exported['source']==refs[0]['source']
  actual={str(p.relative_to(target)):(p.read_bytes(),p.stat().st_mode&0o777) for p in target.rglob('*') if p.is_file()}
  assert actual==expected,(actual.keys(),expected.keys())
  assert target.stat().st_mode&0o777==0o755
 assert rows(ledger)==(5,6,7)
 # All six bindings are text, required, single and separate; canonical expected token.
 workspace=fixture('grammar');ledger=initialize('grammar');valid=args(ledger,workspace,'grammar')
 for flag in valid[1::2]:
  i=valid.index(flag)
  for bad in [valid[:i]+valid[i+2:],valid+[flag,valid[i+1]],valid[:i]+[flag+'='+valid[i+1]]+valid[i+2:],valid[:i]+[flag,'']+valid[i+2:]]:run(bad,2,'usage')
 for token in ['0','01','+1','-1','1.0','1e0','9223372036854775808','none ',' 1']:
  bad=valid.copy();bad[bad.index('--expect-revision')+1]=token;run(bad,2,'usage')
 for extra in [['--'],['--help'],['x'],['--expected-revision','none'],['--root','source=x'],['--job','combined'],['--profile','x']]:run(valid+extra,2,'usage')
 for flag,value in [('--request-id','x'*129),('--request-id','bad/key'),('--actor','x'*257)]:
  bad=valid.copy();bad[bad.index(flag)+1]=value;run(bad,2,'usage')
 assert rows(ledger)==(0,0,1)
 first,_=run(valid);assert first==saved(ledger,'grammar');assert rows(ledger)==(1,1,2)
 # Ledger access preserves parent traversal even though workspace/registry targets are lexical.
 bad=valid.copy();bad[bad.index('--ledger')+1]=str(root/'missing/../grammar.db');run(bad,3,'path_denied')
 assert rows(ledger)==(1,1,2)
 # Exact noop and advancement; replay with missing inputs/advanced head preserves original response.
 noop=run(args(ledger,workspace,'noop','1'))[1];assert noop['outcome']=='noop';assert rows(ledger)==(1,2,3)
 spec=workspace/'workspace/spec.lua';original=spec.read_bytes();spec.write_bytes(original+b'\n-- changed original bytes\n')
 assert run(args(ledger,workspace,'advance','1'))[1]['revision']=='2'
 shutil.rmtree(workspace)
 assert run(valid)[0]==first;assert rows(ledger)==(2,3,4)
 for flag,value in [('--actor','other'),('--expect-revision','2'),('--workspace',str(root/'gone.lua')),('--registry',str(root/'gone.json'))]:
  bad=valid.copy();bad[bad.index(flag)+1]=value;run(bad,3,'request_conflict')
 assert history(ledger,'grammar')['kind']=='history'
 # Ledger location is excluded from intent; a closed exact ledger copy replays unchanged.
 copy=root/'ledger-copy.db';shutil.copyfile(ledger,copy);alias=valid.copy();alias[alias.index('--ledger')+1]=str(copy)
 assert run(alias)[0]==first and rows(copy)==(2,3,4)
 # A changed retained handler that would throw after accept is bypassed on exact early replay.
 workspace=fixture('changed-policy');ledger=initialize('changed-policy');a=args(ledger,workspace,'posterror');first=run(a)[0]
 assert run(a,exe=probe)[0]==first
 shutil.rmtree(workspace);assert run(a,exe=probe)[0]==first;assert rows(ledger)==(1,1,2)
 # Expanded names and explicit lexical aliases have identical bodies; changed env conflicts.
 workspace=fixture('named');ledger=initialize('named');directory=root/'names';directory.mkdir();shutil.copytree(workspace/'workspace',directory/'quota')
 named=args(ledger,workspace,'named');named[named.index('--workspace')+1]='quota'
 namedenv={**env,'PLANAR_WORKSPACE_DIR':str(directory)}
 first=run(named,environ=namedenv)[0]
 explicit=named.copy();explicit[explicit.index('--workspace')+1]=str(directory/'quota/workspace.lua')
 assert run(explicit,environ={**env,'PLANAR_WORKSPACE_DIR':'invalid-relative'})[0]==first
 alias=explicit.copy();alias[alias.index('--workspace')+1]=str(directory/'quota/x/../workspace.lua');assert run(alias)[0]==first
 run(named,3,'request_conflict',environ={**env,'PLANAR_WORKSPACE_DIR':str(root/'other-names')})
 # Every job and referenced file is required; failed keys remain reusable with no history changes.
 for label,mutate in [
  ('unselected',lambda w:(w/'workspace/split.lua').write_text('return planar.job {version=1,id="broken"}\n')),
  ('reference',lambda w:(w/'workspace/spec.lua').write_text('return planar.spec {version=1,id="quota",goal=planar.file("missing.md")}\n')),
  ('constructor',lambda w:(w/'workspace/split.lua').write_text('return planar.spec {version=1,id="wrong"}\n'))]:
  workspace=fixture(label);ledger=initialize(label);mutate(workspace)
  p=subprocess.run([binary,*args(ledger,workspace,label)],env=env,capture_output=True,timeout=30)
  assert p.returncode==(4 if label=='reference' else 3),(label,p.stdout,p.stderr);assert rows(ledger)==(0,0,1)
  assert str(root).encode() not in p.stderr
 # Concurrent equal requests create one request; conflicting bodies and competing CAS have one winner.
 for mode in ['equal','conflict','cas']:
  workspace=fixture(mode);ledger=initialize(mode)
  a=args(ledger,workspace,'race');b=args(ledger,workspace,'race' if mode!='cas' else 'other',actor='other' if mode=='conflict' else 'driver')
  def contender(intent):
   deadline=time.monotonic()+15
   for attempt in range(20):
    assert time.monotonic()<deadline,('concurrency deadline',mode)
    outcome=subprocess.run([binary,*intent],capture_output=True,env=env,timeout=max(.1,deadline-time.monotonic()))
    if outcome.returncode==0:return outcome
    error=json.loads(outcome.stderr)['error']
    retry=outcome.returncode==4 and error in [{'code':'ledger_busy','message':'Ledger is busy','field':''},{'code':'io','message':'Explicit ledger recovery is required','field':'','details':{'operation':'ledger'}}]
    if not retry:return outcome
    print(f'concurrency {mode} attempt {attempt+1}: {outcome.stderr.decode().strip()}',flush=True)
    time.sleep(.01)
   raise AssertionError(('bounded concurrency retry exhausted',mode,outcome.stderr))
  with concurrent.futures.ThreadPoolExecutor(2) as pool:
   outcomes=list(pool.map(contender,[a,b]))
  if mode=='equal':assert all(p.returncode==0 for p in outcomes) and outcomes[0].stdout==outcomes[1].stdout
  else:
   assert sorted(p.returncode for p in outcomes)==[0,3],[(p.returncode,p.stderr) for p in outcomes]
   failure=next(p for p in outcomes if p.returncode);assert json.loads(failure.stderr)['error']['code']==('request_conflict' if mode=='conflict' else 'revision_conflict')
  assert rows(ledger)==(1,1,2)
 # Deterministic miss, intervening commit and advanced head: locked replay wins before stale CAS.
 workspace=fixture('late');ledger=initialize('late');a=args(ledger,workspace,'late')
 p=subprocess.Popen([probe,*a],stdout=subprocess.PIPE,stderr=subprocess.PIPE,env=env,cwd=root)
 wait(Path(str(ledger)+'.miss'),p);first=run(a)[0]
 spec=workspace/'workspace/spec.lua';spec.write_bytes(spec.read_bytes()+b'\n-- advanced\n')
 run(args(ledger,workspace,'advance','1'));Path(str(ledger)+'.resume').touch()
 assert finish(p)==first;assert rows(ledger)==(2,2,3)
 # The prepared capture is changed before final locked native verification: complete rollback.
 workspace=fixture('capture-race');ledger=initialize('capture-race');a=args(ledger,workspace,'capture-race')
 p=subprocess.Popen([probe,*a],stdout=subprocess.PIPE,stderr=subprocess.PIPE,env=env,cwd=root)
 wait(Path(str(ledger)+'.prepared'),p);spec=workspace/'workspace/spec.lua';spec.write_bytes(spec.read_bytes()+b'\n-- race\n');Path(str(ledger)+'.resume').touch()
 out,err=p.communicate(timeout=30);assert p.returncode==3 and not out and json.loads(err)['error']['code']=='capture_changed',(p.returncode,out,err)
 assert rows(ledger)==(0,0,1);run(a);assert rows(ledger)==(1,1,2)
 # Real Lua resource exhaustion and malformed/Lua errors after native commit retain uncertainty.
 message='Load may be complete; retry the same request ID or inspect request history'
 for mode in ['postalloc','postencode','posttransport','posterror','postmemory','postinstructions','postchanged','postfalse','postmalformed']:
  workspace=fixture(mode);ledger=initialize(mode);a=args(ledger,workspace,mode)
  _,value=run(a,4,'io',exe=probe);assert value['error']=={'code':'io','field':'','message':message,'details':{'operation':'load'}}
  assert rows(ledger)==(1,1,2);assert run(a)[0]==saved(ledger,mode);history(ledger,mode)
 workspace=fixture('earlytransport');ledger=initialize('earlytransport');a=args(ledger,workspace,'earlytransport');first=run(a)[0]
 shutil.rmtree(workspace);run(a,4,'io',exe=probe);assert run(a)[0]==first;assert rows(ledger)==(1,1,2)
 # Closed stdout after commit and after early replay use the preallocated stdout diagnostic.
 workspace=fixture('stdout');ledger=initialize('stdout');a=args(ledger,workspace,'stdout')
 for _ in range(2):
  _,value=run(a,4,'io',closed=True);assert value['error']=={'code':'io','field':'','message':message,'details':{'operation':'stdout'}}
  assert rows(ledger)==(1,1,2)
 assert run(a)[0]==saved(ledger,'stdout')
 assert not (root/'forbidden-legacy.db').exists()
print(f'load CLI: {count} checked invocations; grammar, saved-byte replay, all-job refusal, concurrent CAS, deterministic late replay/capture race, completion and stdout failures')
