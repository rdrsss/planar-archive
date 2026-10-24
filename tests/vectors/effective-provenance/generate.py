#!/usr/bin/env python3
"""Independent declarative effective fixture: Python values + documented binary grammars.
No product code or existing fixture is read. Upstream pinned xxHash supplies hashes.
"""
from pathlib import Path
import argparse,ctypes,struct,subprocess,tempfile,json
HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[2]
def u64(n):return struct.pack('>Q',n)
def blob(b):return u64(len(b))+b
def canonical(v):
 if isinstance(v,bool):return bytes([int(v)])
 if isinstance(v,int):return b'\x02'+struct.pack('>q',v)
 if isinstance(v,str):return b'\x03'+blob(v.encode())
 if isinstance(v,bytes):return b'\x03'+blob(v)
 if isinstance(v,list):return b'\x04'+u64(len(v))+b''.join(map(canonical,v))
 return b'\x05'+u64(len(v))+b''.join(canonical(k)+canonical(v[k]) for k in sorted(v,key=lambda k:k.encode()))
def opt(name,kind='text',required=False):return dict(name=name,type=kind,required=required,help='option')
def command(name,options,cap):return dict(descriptor=dict(name=name,family=name,handler_id='shipped.'+name+'.v1',options=sorted(options,key=lambda o:o['name']),help='help',response_kind=name+'_v1',capabilities=[cap]),owner=dict(tier='shipped',module='shipped/planar/commands.lua',ordinal=2))
role=[opt('--'+x,required=True) for x in ['workspace','registry','root','job','role']]+[opt('--base-root'),opt('--observations'),opt('--budget-bytes','decimal'),opt('--capture-outputs','flag')]
history=[opt('--ledger',required=True),opt('--request-id')]+[opt('--'+x,'decimal') for x in ['revision','at-event','after-event','limit']]
catalog=dict(version=2,commands=[command('export',[opt('--ledger',required=True),opt('--target',required=True),opt('--revision','decimal',True)],'export_source_v1'),command('history',history,'read_history_v1'),command('init',[opt('--ledger',required=True),opt('--workspace-id',required=True)],'initialize_ledger_v1'),command('load',[opt('--'+x,required=True) for x in ['ledger','workspace','registry','expect-revision','request-id','actor']],'accept_load_v1'),command('render',role,'render_capture_v1')],shapes=[],projections=[])
# Deliberately nonlexical original/module orders; historical assembly defines shipped ordinal 2.
originals=[('planar/z.lua',b'-- z\n'),('planar/commands.lua',b'-- commands\n')]
separator='\n-- next --\n'
old=dict(version=1,scheme='ordered_concat_v1',files=[p for p,b in originals],separator=separator)
modules=[dict(path='external/ext/z.lua',mode=493),dict(path='external/ext/a.lua',mode=420)]
manifest=dict(version=1,scheme='effective_stdlib_v1',registration='external_registration_v1',embedded='__planar__/embedded.bin',assembly='__planar__/embedded.lua',profile='profile/input.json',modules=modules,catalog=catalog)
files={'shipped/'+p:(420,b) for p,b in originals}
files.update({'__planar__/embedded.lua':(420,b''.join(b+separator.encode() for p,b in originals)),'__planar__/embedded.bin':(420,canonical(old)),'profile/input.json':(493,b'{ "version": 1, "modules": ["ext/z.lua", "ext/a.lua"] }\n'),'external/ext/z.lua':(493,b'-- external z\x00\xff\n'),'external/ext/a.lua':(420,b'-- external a\n'),'__planar__/effective.bin':(420,canonical(manifest))})
MAGIC=b'PLANAR-RECORD\x01'
def file_record(mode,b):return MAGIC+b'\x01'+struct.pack('>I',mode)+blob(b)
def directory_record(children):return MAGIC+b'\x02'+struct.pack('>II',493,len(children))+b''.join(blob(n)+bytes([k])+d for n,k,d in children)
def known_bytes(digest):
 # Read-only pre-existing hand-byte fixtures are independent of this new encoder.
 values={'false':False,'true':True,'zero':0,'negative':-1,'minimum':-2147483648,'maximum':2147483647,'empty_string':'','raw':bytes.fromhex('00ff8041'),'empty_array':[],'empty_object':{},'array':[True,False],'object':{'é':False,'z':True}}
 rows={line.split('\t')[0]:line.split('\t')[1] for line in (ROOT/'tests/vectors/canonical-v1.tsv').read_text().splitlines()}
 for n,v in values.items():assert canonical(v).hex()==rows[n],n+' known canonical bytes'
 records={line.split('\t')[0]:line.split('\t')[1:] for line in (ROOT/'tests/vectors/records/record-v1.tsv').read_text().splitlines() if not line.startswith('#')}
 empty=file_record(420,b'');binary=file_record(493,b'A\x00\xff\r\n')
 preimages={'file-empty':empty,'file-binary-executable':binary,'directory-empty':directory_record([]),'directory-two':directory_record([(b'A.txt',1,digest(empty)),(b'bin',1,digest(binary))])}
 for n,b in preimages.items():
  assert b.hex()==records[n][1],n+' known portable record bytes'
  assert digest(b).hex()==records[n][2],n+' known portable record digest'
 print('12 established canonical known-byte and 4 record portable-byte/digest vectors pass (read-only)')
class Hash(ctypes.Structure):_fields_=[('low',ctypes.c_uint64),('high',ctypes.c_uint64)]
def outputs():
 with tempfile.TemporaryDirectory(prefix='effective-oracle-') as d:
  lib=Path(d)/'xxhash.so';subprocess.run(['cc','-shared','-fPIC','-O2',str(ROOT/'vendor/xxhash/0.8.3/xxhash.c'),'-o',str(lib)],check=True)
  xx=ctypes.CDLL(str(lib));xx.XXH3_128bits.argtypes=[ctypes.c_void_p,ctypes.c_size_t];xx.XXH3_128bits.restype=Hash
  def digest(b):
   h=xx.XXH3_128bits(b,len(b));return struct.pack('>QQ',h.high,h.low)
  known_bytes(digest)
  tree={}
  for p,(mode,b) in files.items():
   node=tree;segments=p.split('/')
   for s in segments[:-1]:node=node.setdefault(s,{})
   node[segments[-1]]=(mode,b)
  def descend(node):
   if isinstance(node,tuple):
    mode,b=node;return 1,digest(file_record(mode,b))
   children=[]
   for name in sorted(node):
    kind,h=descend(node[name]);children.append((name.encode(),kind,h))
   return 2,digest(directory_record(children))
  return {'files.tsv':'# path\tmode\tpayload hex\n'+''.join(f'{p}\t{mode}\t{b.hex()}\n' for p,(mode,b) in sorted(files.items())), 'catalog.bin':canonical(catalog),'manifest.bin':canonical(manifest),'digest.txt':descend(tree)[1].hex()+'\n','description.json':json.dumps(manifest,indent=2,sort_keys=True)+'\n'}
def main():
 parser=argparse.ArgumentParser();parser.add_argument('--check',action='store_true');a=parser.parse_args()
 for n,b in outputs().items():
  if isinstance(b,str):b=b.encode()
  if a.check:assert (HERE/n).read_bytes()==b,n+' drift'
  else:(HERE/n).write_bytes(b)
 print('5 independent effective-provenance fixture artifacts verified')
if __name__=='__main__':main()
