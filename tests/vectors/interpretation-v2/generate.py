#!/usr/bin/env python3
"""Independent declarative interpretation-v2, shape H and portable tree vectors.
Uses the existing independent effective-tree grammar, never product code or Lua.
"""
from pathlib import Path
import importlib.util,ctypes,struct,subprocess,tempfile,json,sys,copy
sys.dont_write_bytecode=True
HERE=Path(__file__).resolve().parent;ROOT=HERE.parents[2]
spec=importlib.util.spec_from_file_location('oracle',HERE.parent/'effective-provenance/generate.py');o=importlib.util.module_from_spec(spec);spec.loader.exec_module(o)
def outputs():
 with tempfile.TemporaryDirectory(prefix='v2-oracle-') as d:
  lib=Path(d)/'xxhash.so';subprocess.run(['cc','-shared','-fPIC','-O2',str(ROOT/'vendor/xxhash/0.8.3/xxhash.c'),'-o',str(lib)],check=True)
  xx=ctypes.CDLL(str(lib));xx.XXH3_128bits.argtypes=[ctypes.c_void_p,ctypes.c_size_t];xx.XXH3_128bits.restype=o.Hash
  def raw(b):
   h=xx.XXH3_128bits(b,len(b));return struct.pack('>QQ',h.high,h.low)
  o.known_bytes(raw)
  cat=copy.deepcopy(o.catalog)
  def shape(id,tier,module,ordinal):return dict(descriptor=dict(id=id,abi='shape_v1',revision='vone',stages='forbidden',help='historical'),owner=dict(tier=tier,module=module,ordinal=ordinal))
  cat['shapes']=[shape('custom','external','external/ext/z.lua',1),shape('fanout_review','shipped','shipped/planar/z.lua',1),shape('pipeline','shipped','shipped/planar/z.lua',1)]
  manifest=copy.deepcopy(o.manifest);manifest['catalog']=cat
  files=copy.deepcopy(o.files);files['__planar__/effective.bin']=(420,o.canonical(manifest))
  def tree_digest(files):
   tree={}
   for p,(mode,b) in files.items():
    n=tree;segments=p.split('/')
    for s in segments[:-1]:n=n.setdefault(s,{})
    n[segments[-1]]=(mode,b)
   def descend(n):
    if isinstance(n,tuple):return 1,raw(o.file_record(*n))
    return 2,raw(o.directory_record([(s.encode(),*descend(n[s])) for s in sorted(n)]))
   return descend(tree)[1].hex()
  h='xxh3_128:'+raw(o.canonical(['planar','canonical_v1','shape_definition_v1',dict(descriptor=cat['shapes'][0]['descriptor'],owner={k:cat['shapes'][0]['owner'][k] for k in ['tier','module']},module_bytes=files['external/ext/z.lua'][1].hex())])).hex()
  specdata=dict(id='spec',version=1,goal={},frozen=[],work=[dict(id='alpha'),dict(id='beta')],**{k:[] for k in ['constraints','contracts','dependencies','facets','milestones','prescriptions','questions']})
  workspace=dict(id='workspace',version=1,spec='spec.lua',root={},registry={},jobs=[],checks=[],toolchains=[])
  jobs=[];models=[]
  for jid,shapeid in [('a_custom','custom'),('b_shipped','fanout_review'),('c_pipeline','pipeline')]:
   jobs.append(dict(file=jid+'.lua',value=dict(id=jid,version=1,shape=shapeid,roles=[dict(id='worker'),dict(id='other'),dict(id='reviewer')])))
   model=dict(id=jid,file=jid+'.lua',assignments=[dict(work='alpha',role='worker'),dict(work='beta',role='other')],reviewer='reviewer',effective_checks=[],pipeline_predecessors=[],graph=dict(vertices=[],edges=[]))
   if shapeid=='custom':model['selected_shape']=dict(id='custom',revision='vone',definition=h,obligations=[dict(id='shape:custom:edge',source='shape',producer='alpha',consumer='beta',producer_role='worker',consumer_role='other',phase='start',quality='done')],guidance='Historical guidance.\n')
   models.append(model)
  B=dict(workspace=workspace,spec=specdata,jobs=jobs,registry=dict(id='registry',revision='vone',version=1,checks=[],toolchains=[]),workspace_file='workspace.lua',spec_file='spec.lua',registry_file='input.json')
  S=dict(dependencies=[],jobs=models)
  V=dict(canonical='canonical_v1',declarations='declarations_shapes_v1',hash='xxh3_128_v1',interpretation='interpretation_v2',normalization='normalization_shapes_v1',normalizer='exact_bytes_v1',projection='projection_v2',selection='selection_shapes_v1',semantics='dependencies_shapes_v1',registration='external_registration_v1',provenance='effective_stdlib_v1',stdlib_digest=tree_digest(files))
  E=dict(registration='external_registration_v1',catalog=cat,selected_shapes=[dict(job='a_custom',id='custom',revision='vone',definition=h)])
  I=dict(version=2,bundle=B,semantic=S,versions=V,extensions=E)
  result={'interpretation.bin':o.canonical(I),'versions.bin':o.canonical(V),'interpretation.json':json.dumps(I,indent=2,sort_keys=True)+'\n','definition.txt':h+'\n','digest.txt':V['stdlib_digest']+'\n','files.tsv':'# path\tmode\tpayload hex\n'+''.join(f'{p}\t{mode}\t{b.hex()}\n' for p,(mode,b) in sorted(files.items()))}
  # A profile with zero external modules still requires v2 and effective provenance.
  empty=copy.deepcopy(files)
  for p in list(empty):
   if p.startswith('external/'):del empty[p]
  empty['profile/input.json']=(420,b'{"version":1,"modules":[]}')
  em=copy.deepcopy(manifest);em['modules']=[];em['catalog']['shapes']=em['catalog']['shapes'][1:]
  empty['__planar__/effective.bin']=(420,o.canonical(em));ei=copy.deepcopy(I);ei['bundle']['jobs']=ei['bundle']['jobs'][1:];ei['semantic']['jobs']=ei['semantic']['jobs'][1:];ei['extensions']['catalog']=em['catalog'];ei['extensions']['selected_shapes']=[];ei['versions']['stdlib_digest']=tree_digest(empty)
  result.update({'empty-interpretation.bin':o.canonical(ei),'empty-versions.bin':o.canonical(ei['versions']),'empty-files.tsv':'# path\tmode\tpayload hex\n'+''.join(f'{p}\t{mode}\t{b.hex()}\n' for p,(mode,b) in sorted(empty.items()))})
  return result
if __name__=='__main__':
 for n,b in outputs().items():
  if isinstance(b,str):b=b.encode()
  if '--check' in sys.argv:assert (HERE/n).read_bytes()==b,n
  else:(HERE/n).write_bytes(b)
 print('9 independent interpretation-v2 artifacts verified')
