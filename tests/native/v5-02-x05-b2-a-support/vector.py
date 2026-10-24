"""Independent canonical_v1 encoder plus upstream seed-zero XXH3 known answer."""
from pathlib import Path
import ctypes, importlib.util, json, subprocess, tempfile
root = Path(__file__).resolve().parents[3]
spec = importlib.util.spec_from_file_location('canonical_oracle', root/'tests/vectors/generate.py')
oracle = importlib.util.module_from_spec(spec)
spec.loader.exec_module(oracle)
source = "local D={id='custom',abi='shape_v1',revision='initial',stages='optional',help='shape'}; registration.shape(D,function() error('shape F must not execute') end); D.revision='final'"
value = {'descriptor':{'id':'custom','abi':'shape_v1','revision':'final','stages':'optional','help':'shape'}, 'owner':{'tier':'external','module':'external/m1.lua'}, 'module_bytes':source.encode().hex()}
encoded = oracle.encode(['planar','canonical_v1','shape_definition_v1',value])
with tempfile.TemporaryDirectory(prefix='planar-b2-a-oracle-') as tmp:
    lib=Path(tmp)/'xxhash.so'
    subprocess.run(['cc','-shared','-fPIC','-O2',str(root/'vendor/xxhash/0.8.3/xxhash.c'),'-o',str(lib)],check=True)
    xx=ctypes.CDLL(str(lib)); xx.XXH3_128bits.argtypes=[ctypes.c_void_p,ctypes.c_size_t]; xx.XXH3_128bits.restype=oracle.Hash
    h=xx.XXH3_128bits(encoded,len(encoded))
    result={'source':source,'canonical_hex':encoded.hex(),'canonical_bytes':len(encoded),'digest':f'xxh3_128:{h.high:016x}{h.low:016x}'}
    print(json.dumps(result,indent=2))
