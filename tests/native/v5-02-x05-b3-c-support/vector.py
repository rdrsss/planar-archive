"""Independently authored X-SHAPE W fixture and canonical/domain hash oracle.
No production selection output is read. The normalized material below is transcribed
from the fixture declaration and X-SHAPE/old W contracts.
"""
from pathlib import Path
import ctypes, importlib.util, json, subprocess, tempfile, sys
root=Path(__file__).resolve().parents[3];here=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('canonical',root/'tests/vectors/generate.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
registry=json.loads((root/'fixtures/render/quota/registry/quota-tools.json').read_text())
check=registry['checks'][0].copy();check['inputs']=sorted(check['inputs'])
tool=registry['toolchains'][0].copy();tool['environment']=sorted(tool['environment'],key=lambda x:x['name'])
with tempfile.TemporaryDirectory() as tmp:
 lib=Path(tmp)/'xx.so';subprocess.run(['cc','-shared','-fPIC','-O2',str(root/'vendor/xxhash/0.8.3/xxhash.c'),'-o',str(lib)],check=True)
 xx=ctypes.CDLL(str(lib));xx.XXH3_128bits.argtypes=[ctypes.c_void_p,ctypes.c_size_t];xx.XXH3_128bits.restype=m.Hash
 def digest(domain,value):
  b=m.encode(['planar','canonical_v1',domain,value]);h=xx.XXH3_128bits(b,len(b));return f'xxh3_128:{h.high:016x}{h.low:016x}'
 descriptor=dict(id='custom',abi='shape_v1',revision='final',stages='optional',help='shape')
 definition=digest('shape_definition_v1',dict(descriptor=descriptor,owner=dict(tier='external',module='external/m1.lua'),module_bytes=(here/'shape.lua').read_bytes().hex()))
 text=lambda s:dict(kind='text',bytes=s)
 obligations=[dict(id='shape:custom:'+id,producer='library',consumer='command',phase=phase,quality='done',source='shape',producer_role='library_worker',consumer_role='cli_worker')for id,phase in [('a','start'),('z','complete')]]
 w=dict(versions=['declarations_v1','selection_v1','dependencies_v1','exact_bytes_v1','canonical_v1','xxh3_128_v1','shape_selection_v1'],workspace='quota',spec='quota',job='split',role='library_worker',root='source',base=dict(status='not_captured',root='source',label='initial',authority='declaration_assertion_unverified'),goal=text('Goal'),units=[dict(id='library',summary=text('Library'),paths=['src/core/'],checks=['contract'])],milestones=[dict(id='done',work=['library'],acceptance=[dict(id='pass',text=text('Pass'))])],constraints=[],prescriptions=[],contracts=[],dependencies=[],facets=[],pipeline_predecessors=[],questions=[],ownership=dict(work=[dict(work='library',paths=['src/core/'])],frozen=[],excluded=['generated/']),effective_checks=[dict(work='library',checks=['contract'])],checks=[check],toolchains=[tool],shape=dict(kind='custom',max_rounds=2,mapping=[dict(work='library',role='library_worker')],reviewer='reviewer',definition=definition,revision='final',obligations=obligations),start_facets=[])
 result=dict(value=w,digest=digest('work_external_shape_v1',w))
 outputs={'expected-library-worker.json':json.dumps(result,indent=2)+'\n','expected-library-worker.preimage.hex':m.encode(['planar','canonical_v1','work_external_shape_v1',w]).hex()+'\n'}
 for name,data in outputs.items():
  if '--check'in sys.argv:assert (here/name).read_text()==data,name
  else:(here/name).write_text(data)
 print('Independent authored W, full canonical preimage, selected source definition and external domain digest verified')
