"""Actual candidate selector, ordering, confinement and physical traversal acceptance."""
from pathlib import Path
import json,os,shutil,subprocess,sys,tempfile
binary,invalid_registration,exhausted_registration=map(lambda p:Path(p).resolve(),sys.argv[1:])
repo=Path(__file__).resolve().parents[3];fixture=repo/'fixtures/render/quota';checks=0
with tempfile.TemporaryDirectory(prefix='planar-c-cli-') as temporary:
 root=Path(temporary).resolve();named=root/'names';named.mkdir();shutil.copytree(fixture/'workspace',named/'demo')
 source=fixture/'current';registry=fixture/'registry/quota-tools.json'
 env=dict(os.environ);env.pop('PLANAR_WORKSPACE_DIR',None)
 env['HOME']=str(root/'no-home');env['PLANAR_DB']=str(root/'no-legacy-db')
 def args(selector,**replace):
  fields={'workspace':selector,'registry':registry,'root':f'source={source}','job':'combined','role':'builder'};fields.update(replace)
  return ['render',*[token for key,value in fields.items() for token in ['--'+key,str(value)]]]
 def run(selector,status=0,code=None,directory=str(named),command=None,**replace):
  global checks
  current=env.copy()
  if directory is not None:current['PLANAR_WORKSPACE_DIR']=directory
  p=subprocess.run([str(command or binary),*args(selector,**replace)],cwd=root,env=current,capture_output=True)
  assert p.returncode==status,(selector,p.returncode,status,p.stdout,p.stderr)
  if status:
   assert not p.stdout
   error=json.loads(p.stderr)['error'];assert error['code']==code,(selector,error)
   assert str(root).encode() not in p.stderr
  else:assert not p.stderr
  checks+=1;return p
 explicit=run(named/'demo/workspace.lua').stdout
 assert run('demo').stdout==explicit
 assert run('demo',directory=str(root/'absent/../names')).stdout==explicit
 for name in ['a','9_A-z','x'*64]:
  shutil.copytree(named/'demo',named/name)
  assert run(name).stdout==explicit
 for invalid in ['.','..','bad.name','bad.LUA','a b','a\\b','a:b','_bad','-bad','x'*65]:
  run(invalid,2,'usage')
 run('workspace',2,'usage',directory=None)
 for directory in [None,'','relative','~/names','/bad\udcff']:
  run('demo',2,'usage',directory=directory)
 for directory in [None,'','relative','/bad\udcff']:
  assert run(named/'demo/workspace.lua',directory=directory).stdout==explicit
 # Suffix and slash classify as explicit, irrespective of malformed environment.
 assert run('names/demo/workspace.lua',directory='relative').stdout==explicit
 shutil.copytree(named/'demo',root/'local')
 assert run('./local/workspace.lua',directory=None).stdout==explicit
 shutil.copy(root/'local/workspace.lua',root/'local/a.LUA')
 run('local/a.LUA')
 run('missing.lua',4,'io',directory=None)
 run('missing',4,'io')
 run('demo',3,'reference',root='different=/not-present',registry='/not-present')
 # Leaf, final source root, and captured aliases use the unchanged native refusal.
 (named/'leaf').mkdir();(named/'leaf/workspace.lua').symlink_to(named/'demo/workspace.lua')
 run('leaf',3,'path_denied')
 (root/'source-alias').symlink_to(source,target_is_directory=True)
 run('demo',3,'path_denied',root=f'source={root}/source-alias')
 shutil.copytree(named/'demo',named/'captured');(named/'captured/spec.lua').unlink();(named/'captured/spec.lua').symlink_to(named/'demo/spec.lua')
 run('captured',3,'path_denied')
 # Render keeps intermediate-link/.. physical parent semantics.
 (root/'physical').mkdir();shutil.copytree(named/'demo',root/'physical/workspace')
 (root/'physical/workspace/child').mkdir();(root/'lexical').mkdir()
 (root/'lexical/hop').symlink_to(root/'physical/workspace/child',target_is_directory=True)
 assert run(root/'lexical/hop/../workspace.lua').stdout==explicit
 # Registration admission precedes selector/environment failure; parse still precedes registration.
 run('bad.name',4,'internal',command=invalid_registration)
 run('demo',3,'resource_limit',directory=None,command=exhausted_registration)
 # Absolute selected paths need no cwd lookup: launch from a cwd removed before exec.
 gone=root/'gone';gone.mkdir()
 def remove_cwd():os.rmdir(gone)
 p=subprocess.run([str(binary),*args(named/'demo/workspace.lua')],cwd=gone,env=env,capture_output=True,preexec_fn=remove_cwd)
 assert p.returncode==0 and p.stdout==explicit and not p.stderr,(p.returncode,p.stderr)
 checks+=1
 # Named absolute expansion is equally independent of cwd.
 gone.mkdir();name_env=dict(env,PLANAR_WORKSPACE_DIR=str(named))
 p=subprocess.run([str(binary),*args('demo')],cwd=gone,env=name_env,capture_output=True,preexec_fn=remove_cwd)
 assert p.returncode==0 and p.stdout==explicit and not p.stderr,(p.returncode,p.stderr)
 checks+=1
 # A relative selection needs one cwd snapshot and gets a redacted failure if it cannot be obtained.
 gone.mkdir()
 p=subprocess.run([str(binary),*args('names/demo/workspace.lua')],cwd=gone,env=env,capture_output=True,preexec_fn=remove_cwd)
 assert p.returncode==4 and not p.stdout
 error=json.loads(p.stderr)['error']
 assert error=={'code':'io','field':'','message':'Cannot obtain invocation cwd','details':{'operation':'capture'}},error
 checks+=1
 assert not (root/'no-home').exists() and not (root/'no-legacy-db').exists()
print(f'{checks} real CLI processes; exact name/file equivalence, validation ordering, physical render and alias refusals passed')
