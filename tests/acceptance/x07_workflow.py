"""X07 workflow acceptance: a custom shape loads and renders under a profile; a custom verb coexists with shipped history and export.

argv: variant-3 binary (unused here), ctest case, production planar, variant-4 binary (unused here).
Scratch trees only; canonical JSON is asserted byte for byte where the contract fixes it.
"""
from pathlib import Path
import json,os,shutil,sqlite3,subprocess,sys,tempfile
case,planar=sys.argv[2],str(Path(sys.argv[3]).resolve())
repo=Path(__file__).resolve().parents[2]
def canonical(value):return json.dumps(value,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()+b'\n'
def opt(name,kind,required=False):return "{name='--%s',type='%s',required=%s,help='o'}"%(name,kind,'true' if required else 'false')
def command(name,options,capabilities,body):
 return "registration.add({name='%s',family='extension_read_v1',handler_id='custom.%s',options=engine.array{%s},help='h',response_kind='extension_v1',capabilities=engine.array{%s}},function(inv,ctx) %s end)\n"%(name,name,','.join(sorted(options)),capabilities,body)
HISTORY=[opt('after-event','history_id'),opt('at-event','history_id'),opt('ledger','text',True),opt('limit','decimal'),opt('request-id','text'),opt('revision','history_id')]
SHAPE="local D={id='custom',abi='shape_v1',revision='final',stages='optional',help='shape'}\nregistration.shape(D,function(input) return {ok=true,value={obligations=engine.array{},guidance='custom workflow guidance'}} end)\n"
VERB=(command('histp',HISTORY,"'history_read_v1','projection_apply_v1'","ctx.read_history({}) local p=ctx.project({id='p'}) return {ok=true,value={version=1,kind='extension',command=inv.command,data=p}}")
 +"registration.projection({id='p',abi='projection_v1',revision='final',input='history_snapshot_v1',help='p'},function(input) return {version=1,sections=engine.array{{id='workflow_note',text='## workflow_note\\n\\nrecorded by a custom verb\\n',mandatory=false}}} end)\n")
with tempfile.TemporaryDirectory(prefix='planar-x07-workflow-') as temp:
 root=Path(temp).resolve();work=root/'work';trusted=root/'trusted';fx=root/'fx'
 for d in (work,trusted/'modules'):d.mkdir(parents=True)
 shutil.copytree(repo/'fixtures/render/quota',fx)
 env={**os.environ,'HOME':str(root),'PLANAR_DB':str(root/'forbidden-legacy.db')}
 env.pop('PLANAR_WORKSPACE_DIR',None)
 def profile(source):
  (trusted/'p.json').write_text('{"version":1,"modules":["modules/p.lua"]}');(trusted/'modules/p.lua').write_text(source)
 selector=['with-profile','--stdlib-profile','../trusted/p.json']
 def run(args,status=0):
  got=subprocess.run([planar,*args],capture_output=True,env=env,cwd=work,timeout=60)
  assert got.returncode==status,(args,got.returncode,got.stdout[:400],got.stderr[:400])
  data=got.stdout if status==0 else got.stderr
  assert not (got.stderr if status==0 else got.stdout),got
  value=json.loads(data);assert data==canonical(value),data[:200]
  assert str(root).encode() not in data and str(repo).encode() not in data
  return got,value
 def init(name):
  ledger=work/(name+'.db');run(['init','--ledger',str(ledger),'--workspace-id','quota']);return ledger
 def customize():
  for name in ('split.lua','combined.lua'):
   path=fx/'workspace'/name;text=path.read_text();assert 'fanout_review' in text,name;path.write_text(text.replace('fanout_review','custom'))
 ws=str(fx/'workspace/workspace.lua');reg=str(fx/'registry/quota-tools.json')
 def load(ledger,key):return ['load','--ledger',str(ledger),'--workspace',ws,'--registry',reg,'--expect-revision','none','--request-id',key,'--actor','driver']
 if case=='x07.workflow_shape_load':
  customize();profile(SHAPE);ledger=init('shape')
  # Control: the custom shape ID is unknown to the embedded dialect, so the embedded load refuses it at job.shape.
  run(load(ledger,'embedded'),3)
  got,value=run([*selector,*load(ledger,'custom')])
  assert value['kind']=='load',value
  with sqlite3.connect(ledger) as db:
   assert db.execute('SELECT COUNT(*) FROM revisions').fetchone()==(1,)
   body=db.execute("SELECT encoded FROM records JOIN requests ON requests.body_id=records.id WHERE request_id='custom'").fetchone()[0]
  # Profile provenance: request_v2 with the profile selector tag and the normalized absolute selector.
  assert body.startswith(b'PLANAR-REQUEST\x02') and str(trusted/'p.json').encode() in body,body[:80]
  again,_=run([*selector,*load(ledger,'custom')]);assert again.stdout==got.stdout
 elif case=='x07.workflow_render_external':
  customize();profile(SHAPE)
  render=['render','--workspace',ws,'--registry',reg,'--root','source='+str(fx/'current'),'--job','combined','--role','builder']
  run(render,3)
  got,value=run([*selector,*render])
  text=json.dumps(value)
  # The mandatory shape_policy section carries the custom shape's guidance into the packet.
  assert 'shape_policy' in text and 'custom workflow guidance' in text,text[:300]
 elif case=='x07.workflow_verb_history_export':
  profile(VERB);ledger=init('verb');run(load(ledger,'k'))
  got,value=run([*selector,'histp','--ledger',str(ledger)])
  assert value['kind']=='extension' and value['command']=='histp' and value['data']['type']=='projection',value
  assert value['data']['id']=='p' and 'workflow_note' in [s['id'] for s in value['data']['sections']],value
  assert 'Recorded history; ledger' in value['data']['text'],value['data']['text'][:200]
  plain_history,_=run(['history','--ledger',str(ledger),'--revision','1'])
  profiled_history,_=run([*selector,'history','--ledger',str(ledger),'--revision','1'])
  assert profiled_history.stdout==plain_history.stdout
  target=work/'out';exported,_=run([*selector,'export','--ledger',str(ledger),'--revision','1','--target','out'])
  files={str(p.relative_to(target)):p.read_bytes() for p in target.rglob('*') if p.is_file()};assert files
  # After the profile is deleted, ordinary history and export keep working and agree byte for byte.
  shutil.rmtree(trusted)
  after,_=run(['history','--ledger',str(ledger),'--revision','1']);assert after.stdout==plain_history.stdout
  second,_=run(['export','--ledger',str(ledger),'--revision','1','--target','out2'])
  assert {str(p.relative_to(work/'out2')):p.read_bytes() for p in (work/'out2').rglob('*') if p.is_file()}==files
  assert second.stdout.replace(b'out2',b'out')==exported.stdout.replace(b'out2',b'out')
 else:raise SystemExit('unknown case '+case)
 assert not (root/'forbidden-legacy.db').exists()
print(case,'passed')
