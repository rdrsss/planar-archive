"""X07 profile load acceptance: saved replay, locked relookup, CAS races, receipt races and post-commit failures.

argv: variant-3 binary (pause and fault anchors), ctest case, production planar, variant-4 binary (tiny response ceiling).
Every case runs real processes in its own scratch tree and never touches the operator database or home directory.
"""
from pathlib import Path
import json,os,shutil,sqlite3,subprocess,sys,tempfile,time
variant,case,planar,tiny=sys.argv[1],sys.argv[2],*sys.argv[3:5]
variant,planar,tiny=(str(Path(p).resolve()) for p in (variant,planar,tiny))
repo=Path(__file__).resolve().parents[2]
MESSAGE='Load may be complete; retry the same request ID or inspect request history'
def canonical(value):return json.dumps(value,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()+b'\n'
class World:
 """One scratch tree: root/{anc/trusted, work, fx}; fx is a private copy of the quota fixture."""
 def __init__(self,root):
  self.root=Path(root).resolve();self.work=self.root/'work';self.trusted=self.root/'anc/trusted';self.fx=self.root/'fx'
  self.work.mkdir();self.trusted.mkdir(parents=True);(self.trusted/'modules').mkdir()
  shutil.copytree(repo/'fixtures/render/quota',self.fx)
  (self.trusted/'p.json').write_text('{"version":1,"modules":["modules/p.lua"]}');(self.trusted/'modules/p.lua').write_text('-- profile v1\n')
  self.env={**os.environ,'HOME':str(self.root),'PLANAR_DB':str(self.root/'forbidden-legacy.db')}
  for name in [k for k in self.env if k.startswith(('X07_','PLANAR_WORKSPACE_DIR'))]:del self.env[name]
 selector='../anc/trusted/p.json'
 def ws(self):return str(self.fx/'workspace/workspace.lua')
 def reg(self):return str(self.fx/'registry/quota-tools.json')
 def load(self,ledger,key,actor='driver',head='none',profile=True,selector=None,workspace=None,registry=None):
  args=['load','--ledger',str(ledger),'--workspace',workspace or self.ws(),'--registry',registry or self.reg(),'--expect-revision',head,'--request-id',key,'--actor',actor]
  return ['with-profile','--stdlib-profile',selector or self.selector,*args] if profile else args
 def run(self,exe,args,env=None,closed=False):
  if closed:
   read,write=os.pipe();os.close(read)
   with os.fdopen(write,'wb') as out:return subprocess.run([exe,*args],stdout=out,stderr=subprocess.PIPE,env=env or self.env,cwd=self.work,timeout=60)
  return subprocess.run([exe,*args],capture_output=True,env=env or self.env,cwd=self.work,timeout=60)
 def init(self,name):
  ledger=self.work/(name+'.db');made=self.run(planar,['init','--ledger',str(ledger),'--workspace-id','quota']);assert made.returncode==0,made;return ledger
 def popen(self,exe,args,env=None):
  return subprocess.Popen([exe,*args],stdout=subprocess.PIPE,stderr=subprocess.PIPE,env=env or self.env,cwd=self.work)
 def paused(self,anchor,args):
  """Start the variant paused at an anchor; returns (process, release) once the anchor is reached."""
  flag,gate=self.work/(anchor+'.reached'),self.work/(anchor+'.gate')
  process=self.popen(variant,args,{**self.env,f'X07_REACHED_{anchor}':str(flag),f'X07_PAUSE_{anchor}':str(gate)})
  end=time.monotonic()+30
  while not flag.exists():
   assert process.poll() is None,(anchor,process.returncode,process.communicate())
   assert time.monotonic()<end,anchor
   time.sleep(.005)
  return process,gate
def finish(process,gate=None):
 if gate:gate.touch()
 out,err=process.communicate(timeout=60);return process.returncode,out,err
def rows(ledger):
 with sqlite3.connect(ledger) as db:return tuple(db.execute('SELECT COUNT(*) FROM '+n).fetchone()[0] for n in ['revisions','requests','events'])
def saved(ledger,key):
 with sqlite3.connect(ledger) as db:return db.execute('SELECT result FROM requests WHERE request_id=?',(key,)).fetchone()[0]+b'\n'
def code(err):return json.loads(err)['error']['code']
def replace_file(path):
 """Replace a file by an equal-bytes new inode, as an atomic editor save would."""
 data=path.read_bytes();path.rename(str(path)+'.old');path.write_bytes(data)
def replace_dir(path):
 """Replace a directory by a byte-equal copy with a new identity."""
 old=Path(str(path)+'.old');path.rename(old);shutil.copytree(old,path)
def advance(world,ledger,key,head):
 spec=world.fx/'workspace/spec.lua';spec.write_bytes(spec.read_bytes()+b'\n-- advanced\n')
 done=world.run(planar,world.load(ledger,key,head=head,profile=False));assert done.returncode==0,done
def run_case():
 with tempfile.TemporaryDirectory(prefix='planar-x07-replay-') as temp:
  w=World(temp)
  if case=='x07.replay_after_inputs_deleted':
   ledger=w.init('l');first=w.run(planar,w.load(ledger,'k'));assert first.returncode==0 and not first.stderr,first
   advance(w,ledger,'k2','1');assert rows(ledger)==(2,2,3)
   for path in (w.root/'anc',w.fx):shutil.rmtree(path)
   again=w.run(planar,w.load(ledger,'k'))
   # The saved bytes return with the profile, modules, workspace and registry deleted and the head advanced.
   assert again.returncode==0 and again.stdout==first.stdout==saved(ledger,'k') and not again.stderr,again
   assert rows(ledger)==(2,2,3)
  elif case=='x07.replay_conflict_matrix':
   ledger=w.init('l');first=w.run(planar,w.load(ledger,'k'));assert first.returncode==0,first
   shutil.copytree(w.root/'anc/trusted',w.root/'anc/other');shutil.copytree(w.fx,w.root/'fx2')
   changed={'selector':w.load(ledger,'k',selector='../anc/other/p.json'),'actor':w.load(ledger,'k',actor='someone'),
    'head':w.load(ledger,'k',head='1'),'workspace':w.load(ledger,'k',workspace=str(w.root/'fx2/workspace/workspace.lua')),
    'registry':w.load(ledger,'k',registry=str(w.root/'fx2/registry/quota-tools.json'))}
   for name,args in changed.items():
    got=w.run(planar,args);assert got.returncode==3 and not got.stdout and code(got.stderr)=='request_conflict',(name,got)
   assert rows(ledger)==(1,1,2)
  elif case=='x07.replay_same_selector_changed_bytes_new_provenance':
   ledger=w.init('l');assert w.run(planar,w.load(ledger,'k1')).returncode==0
   (w.trusted/'modules/p.lua').write_text('-- profile v2\n')
   second=w.run(planar,w.load(ledger,'k2',head='1'));assert second.returncode==0,second
   def revision(n):
    got=w.run(planar,['history','--ledger',str(ledger),'--revision',str(n)]);assert got.returncode==0,got
    return json.loads(got.stdout)['data']['revision']
   first,after=revision(1),revision(2)
   # Same selector text, changed profile bytes, fresh ID: new effective provenance (the stdlib reference changes).
   assert first['stdlib']!=after['stdlib'] and first['source']==after['source'],(first,after)
   assert rows(ledger)==(2,2,3)
  elif case in ('x07.relookup_equal_replay_before_stale_cas','x07.relookup_unequal_conflicts'):
   equal=case.endswith('before_stale_cas')
   ledger=w.init('l');args=w.load(ledger,'k',actor='driver')
   process,gate=w.paused('AFTER_EARLY_MISS',args)
   rival=w.run(planar,w.load(ledger,'k',actor='driver' if equal else 'rival'));assert rival.returncode==0,rival
   status,out,err=finish(process,gate)
   if equal:
    # Equal body: the locked exact relookup replays before the stale head comparison could conflict.
    assert status==0 and out==rival.stdout and not err,(status,out,err)
   else:
    assert status==3 and not out and code(err)=='request_conflict',(status,out,err)
   assert rows(ledger)==(1,1,2)
  elif case=='x07.two_process_cas_one_winner':
   for attempt in range(8):
    ledger=w.init(f'cas{attempt}')
    a=w.popen(planar,w.load(ledger,f'a{attempt}'));b=w.popen(planar,w.load(ledger,f'b{attempt}'))
    results=[finish(a),finish(b)];statuses=sorted(r[0] for r in results)
    assert statuses==[0,3],(attempt,results)
    loser=next(r for r in results if r[0]);assert not loser[1] and code(loser[2])=='revision_conflict',loser
    assert rows(ledger)==(1,1,2),(attempt,rows(ledger))
  elif case in ('x07.receipt_races_refuse_before_authorization','x07.final_race_rolls_back_before_commit'):
   early=case.startswith('x07.receipt')
   anchor='AFTER_FACTORY_OPEN' if early else 'AFTER_PREPARE_LOAD'
   races={'profile module file':lambda v:(v.trusted/'modules/p.lua').write_text('-- raced\n'),
    'profile root':lambda v:replace_dir(v.trusted),'profile ancestor':lambda v:replace_dir(v.root/'anc')}
   if early:
    races.update({'registry file':lambda v:replace_file(v.fx/'registry/quota-tools.json'),
     'workspace file':lambda v:replace_file(v.fx/'workspace/workspace.lua')})
   for name,race in races.items():
    with tempfile.TemporaryDirectory(prefix='planar-x07-race-') as inner:
     v=World(inner);ledger=v.init('l');process,gate=v.paused(anchor,v.load(ledger,'k'))
     race(v)
     status,out,err=finish(process,gate)
     assert status==3 and not out and code(err)=='capture_changed',(name,status,out,err)
     assert rows(ledger)==(0,0,1),(name,rows(ledger))
  elif case=='x07.post_commit_allocation_failure_keeps_receipt':
   ledger=w.init('l');env={**w.env,'X07_FAULT_AFTER_COMMIT':'1'}
   got=w.run(variant,w.load(ledger,'k'),env)
   assert got.returncode==4 and not got.stdout,got
   assert json.loads(got.stderr)['error']['message']==MESSAGE and code(got.stderr)=='io',got
   assert rows(ledger)==(1,1,2);replay=w.run(planar,w.load(ledger,'k'))
   assert replay.returncode==0 and replay.stdout==saved(ledger,'k'),replay
  elif case=='x07.post_commit_encoding_failure_keeps_receipt':
   ledger=w.init('l');got=w.run(tiny,w.load(ledger,'k'))
   assert got.returncode==4 and not got.stdout and json.loads(got.stderr)['error']['message']==MESSAGE,got
   assert rows(ledger)==(1,1,2);replay=w.run(planar,w.load(ledger,'k'))
   assert replay.returncode==0 and replay.stdout==saved(ledger,'k'),replay
  elif case=='x07.post_commit_stdout_failure_keeps_receipt':
   ledger=w.init('l')
   for _ in range(2):
    got=w.run(planar,w.load(ledger,'k'),closed=True)
    assert got.returncode==4,got
    assert json.loads(got.stderr)['error']=={'code':'io','field':'','message':MESSAGE,'details':{'operation':'stdout'}},got
    assert rows(ledger)==(1,1,2)
   assert w.run(planar,w.load(ledger,'k')).stdout==saved(ledger,'k')
  else:raise SystemExit('unknown case '+case)
  assert not (w.root/'forbidden-legacy.db').exists()
run_case()
print(case,'passed')
