"""X07 extension service acceptance: late receipts survive to the result seal and the 16 MiB ceiling covers extension replies.

argv: variant-3 binary (pause anchors), ctest case, production planar, variant-4 binary (tiny response ceiling).
Scratch trees only; no operator database, home directory or host path appears in any asserted output.
"""
from pathlib import Path
import json,os,shutil,subprocess,sys,tempfile,time
variant,case,planar,tiny=sys.argv[1],sys.argv[2],*sys.argv[3:5]
variant,planar,tiny=(str(Path(p).resolve()) for p in (variant,planar,tiny))
repo=Path(__file__).resolve().parents[2]
def opt(name,kind,required=False):return "{name='--%s',type='%s',required=%s,help='o'}"%(name,kind,'true' if required else 'false')
def command(name,options,capabilities,body):
 return "registration.add({name='%s',family='extension_read_v1',handler_id='custom.%s',options=engine.array{%s},help='h',response_kind='extension_v1',capabilities=engine.array{%s}},function(inv,ctx) %s end)\n"%(name,name,','.join(sorted(options)),capabilities,body)
ROLE=[opt('workspace','text',True),opt('registry','text',True),opt('root','text',True),opt('job','text',True),opt('role','text',True),
 opt('base-root','text'),opt('observations','text'),opt('budget-bytes','decimal'),opt('capture-outputs','flag')]
MODULE=(command('roleread',ROLE,"'role_context_read_v1'","local r=ctx.read_role({}) return {ok=true,value={version=1,kind='extension',command=inv.command,data={type='role',value=r}}}")
 +command('note',[opt('text','text',True)],'',"return {ok=true,value={version=1,kind='extension',command=inv.command,data={type='summary',text='n'}}}"))
with tempfile.TemporaryDirectory(prefix='planar-x07-services-') as temp:
 root=Path(temp).resolve();work=root/'work';trusted=root/'trusted';fx=root/'fx'
 for d in (work,trusted/'modules'):d.mkdir(parents=True)
 shutil.copytree(repo/'fixtures/render/quota',fx)
 (trusted/'p.json').write_text('{"version":1,"modules":["modules/p.lua"]}');(trusted/'modules/p.lua').write_text(MODULE)
 env={**os.environ,'HOME':str(root),'PLANAR_DB':str(root/'forbidden-legacy.db')}
 for name in [k for k in env if k.startswith(('X07_','PLANAR_WORKSPACE_DIR'))]:del env[name]
 selector=['with-profile','--stdlib-profile','../trusted/p.json']
 role=['roleread','--workspace',str(fx/'workspace/workspace.lua'),'--registry',str(fx/'registry/quota-tools.json'),'--root','source='+str(fx/'current'),'--job','combined','--role','builder']
 def run(exe,args,environ=None):return subprocess.run([exe,*args],capture_output=True,env=environ or env,cwd=work,timeout=60)
 if case=='x07.late_receipt_survives_to_seal':
  # Control: with no pause the extension role read succeeds and prints one canonical JSON line.
  control=run(variant,[*selector,*role]);assert control.returncode==0 and not control.stderr,control
  assert json.loads(control.stdout)['data']['type']=='role'
  flag,gate=work/'reached',work/'gate'
  process=subprocess.Popen([variant,*selector,*role],stdout=subprocess.PIPE,stderr=subprocess.PIPE,cwd=work,
   env={**env,'X07_REACHED_BEFORE_RESULT_SEAL':str(flag),'X07_PAUSE_BEFORE_RESULT_SEAL':str(gate)})
  end=time.monotonic()+30
  while not flag.exists():
   assert process.poll() is None,(process.returncode,process.communicate());assert time.monotonic()<end;time.sleep(.005)
  # The late workspace slot is replaced between handler return and the seal: final verification must refuse it.
  path=fx/'workspace/workspace.lua';data=path.read_bytes();path.rename(str(path)+'.old');path.write_bytes(data)
  gate.touch();out,err=process.communicate(timeout=60)
  assert process.returncode==3 and not out and json.loads(err)['error']['code']=='capture_changed',(process.returncode,out,err)
 elif case=='x07.response_ceiling_extension':
  ok=run(planar,[*selector,'note','--text','v']);assert ok.returncode==0,ok
  got=run(tiny,[*selector,'note','--text','v'])
  assert got.returncode==3 and not got.stdout,got
  error=json.loads(got.stderr)['error']
  assert error['code']=='resource_limit' and error['message']=='JSON codec refused input',error
  assert error['details']['limit']=='output_bytes' and error['details']['maximum']==16777216,error
  assert str(root).encode() not in got.stderr
 else:raise SystemExit('unknown case '+case)
 assert not (root/'forbidden-legacy.db').exists()
print(case,'passed')
