"""Real-process retained dispatch/registration/publication checks in temporary explicit roots."""
from pathlib import Path
import json,subprocess,sys,tempfile
product,fixture,*variants=map(Path,sys.argv[1:]);checks=0

def run(binary,args,status,code=None):
 global checks
 p=subprocess.run([str(binary),*map(str,args)],capture_output=True)
 assert p.returncode==status,(binary,args,p.returncode,p.stdout,p.stderr)
 if status:
  assert not p.stdout,(args,p.stdout)
  e=json.loads(p.stderr)['error']
  if code:assert e['code']==code,(args,e)
  assert b'B2 postpublication' not in p.stderr
 else:assert not p.stderr,(args,p.stderr)
 checks+=1
 return p

help=run(product,['--help'],0).stdout
with tempfile.TemporaryDirectory(prefix='planar-b2-cli-') as temporary:
 root=Path(temporary).resolve()
 ledger=root/'ledger';subprocess.run([str(fixture),str(root),'history'],check=True,capture_output=True)
 before=ledger.read_bytes()
 for index,binary in enumerate(variants):
  assert run(binary,['--help'],0).stdout==help
  for args in [[],['load'],['init'],['init','--help'],['--help','init'],['init','--ledger=x'],
               ['init','--ledger','x','--workspace-id','sample','--'],
               ['history','--ledger','missing','--revision','01'],
               ['history','--ledger','missing','--revision','9223372036854775808'],
               ['history','--ledger','missing','--limit','101'],
               ['history','--ledger','missing','--revision','1','--after-event','1'],
               ['export','--ledger','missing','--revision','1','--target','out','--revision','1'],
               ['render','--workspace','missing','--registry','missing','--root','s=missing','--job','x','--role','y','--capture-outputs'],
               ['render','--workspace','missing','--registry','missing','--root','s=missing','--job','x','--role','y','--capture-outputs','--base-root','t=missing']]:
   if index==5 and args==['init']:continue  # Deliberately altered compiled requiredness reaches catalog comparison.
   run(binary,args,2,'usage')
  if index in [1,2,5]:
   code='resource_limit' if index==2 else 'internal';status=3 if index==2 else 4
   p=run(binary,['init','--ledger',root/f'invalid-{index}','--workspace-id','sample'],status,code)
   assert not (root/f'invalid-{index}').exists()
   assert str(root).encode() not in p.stderr and b'lua' not in p.stderr and b'family' not in p.stderr
 # Identical parser tokens, shipped callable changes the observable policy and never publishes.
 target=root/'semantic'
 p=run(variants[0],['init','--ledger',target,'--workspace-id','projection'],3,'schema')
 assert json.loads(p.stderr)['error']['message']=='B2 retained callable executed'
 assert not target.exists()
 run(product,['init','--ledger',target,'--workspace-id','projection'],0)
 assert target.is_file()
 # A selected execution callable must come from the same admitted VM, even for an exempt read.
 before_status=target.read_bytes()
 p=run(variants[0],['status','--ledger',target],3,'schema')
 assert json.loads(p.stderr)['error']['message']=='B2 retained execution callable executed'
 assert target.read_bytes()==before_status
 ordinary=run(product,['status','--ledger',target],0)
 assert json.loads(ordinary.stdout)['kind']=='status'
 assert target.read_bytes()==before_status
 # A forged response cannot replace a native failure.
 run(variants[0],['export','--ledger',root/'missing','--revision','1','--target',root/'not-published'],3,'ledger_not_found')
 assert not (root/'not-published').exists()
 # Lua throw, invalid projection, and caught cumulative exhaustion after publication.
 for index in [0,3,4]:
  target=root/f'published-{index}'
  p=run(variants[index],['export','--ledger',ledger,'--revision','1','--target',target],4,'io')
  e=json.loads(p.stderr)['error']
  assert e['message']=='Export may be complete; inspect the explicit target before retrying'
  assert e['details']=={'operation':'export'}
  assert (target/'a.lua').read_bytes()==b'original'
  assert sorted(p.name for p in target.iterdir())==['a.lua']
 assert ledger.read_bytes()==before
 assert not list(root.glob('.planar-export-*'))
print(f'{checks} real CLI processes; retained callable, typed parsing, precedence, native poisoning and postpublication fallbacks passed')
