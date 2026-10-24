"""Whole-operation production observer; fully private compiled inputs and outputs."""
from pathlib import Path
import hashlib,json,shlex,shutil,subprocess,sys,time
sys.dont_write_bytecode=True
R=Path(__file__).resolve().parents[3];E=R/'tests/native/v5-02-x05-c2-b1-evidence';S=Path(__file__).parent;B=R/'build/rewrite-debug'
P=E/sys.argv[1];P.mkdir();commands=[]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def write(p,v):p.write_text(json.dumps(v,sort_keys=True,indent=2)+'\n')
def run(label,args):
 start=time.time();p=subprocess.run(list(map(str,args)),cwd=B,capture_output=True,text=True);(P/(label+'.log')).write_text(p.stdout+p.stderr)
 commands.append(dict(label=label,argv=list(map(str,args)),cwd=str(B),exit=p.returncode,seconds=time.time()-start));write(P/'commands.json',commands);print(label,p.returncode,flush=True)
 assert p.returncode==0,(label,(p.stdout+p.stderr)[-4000:]);return p.stdout
original={str(p.relative_to(R)):sha(p)for p in B.rglob('*')if p.is_file()and p.suffix in {'.o','.pcm','.a'}}
write(P/'shared-compiled-before.json',original);copied={}
for relative,digest in original.items():
 src=R/relative;dst=P/'private'/src.relative_to(B);dst.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(src,dst);assert sha(dst)==digest;copied[str(dst.relative_to(E))]=digest
write(P/'copied-compiled.json',copied)
entries=json.loads((B/'compile_commands.json').read_text());raw=shlex.split(next(e['command']for e in entries if e['file'].endswith('/json_codec/codec.cpp')))
def expand(args):
 result=[]
 for a in args:
  if a.startswith('@'):result.extend(expand(shlex.split((B/a[1:]).read_text())))
  else:result.append(a)
 return result
base=expand(raw)
for i,a in enumerate(base):
 if a.startswith('-fmodule-file='):
  key,path=a[len('-fmodule-file='):].split('=',1);source=(B/path).resolve();base[i]='-fmodule-file='+key+'='+str(P/'private'/source.relative_to(B))
 if a.startswith('-fmodule-output='):raise AssertionError('unexpected module output must be isolated')
base+=['-fno-implicit-modules','-fno-implicit-module-maps']
def compile(label,source,extra=[]):
 cmd=base.copy();cmd[cmd.index('-c')+1]=str(source);cmd[cmd.index('-o')+1]=str(P/(label+'.o'))
 for flag in ['-MF','-MT']:
  if flag in cmd:cmd[cmd.index(flag)+1]=str(P/(label+('.d'if flag=='-MF'else'.o')))
 run(label+'-compile',cmd+extra);return P/(label+'.o')
common=[base[0],'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++']
shutil.copyfile(S/'production.cpp',P/'production.cpp');obj=compile('production',P/'production.cpp')
value=P/'private/src/engine/libplanar_value.a';codec=P/'private/src/engine/libplanar_json_codec.a'
run('production-link',common+[obj,codec,value,'-o',P/'production']);run('production',[P/'production'])
# Explicit controlled adapter only: admitted data unchanged; physical buffer reduced.
source=(R/'src/engine/json_codec/codec.cpp').read_text();needle='state.buffer.storage.resize(planar_codec_detail::projection_scratch_bytes);';assert source.count(needle)==1
changed=source.replace(needle,'state.buffer.storage.resize(512);')
(P/'scratch-defect-codec.cpp').write_text(changed);write(P/'transformations.json',dict(source='src/engine/json_codec/codec.cpp',source_sha256=sha(R/'src/engine/json_codec/codec.cpp'),replacement_before=needle,replacement_after='state.buffer.storage.resize(512);',controlled_adapter=True))
defect=compile('scratch-defect-codec',P/'scratch-defect-codec.cpp');defect_probe=compile('scratch-defect',P/'production.cpp',['-DSCRATCH_DEFECT'])
run('scratch-defect-link',common+[defect_probe,defect,value,'-o',P/'scratch-defect']);run('scratch-defect',[P/'scratch-defect'])
now={str(p.relative_to(R)):sha(p)for p in B.rglob('*')if p.is_file()and p.suffix in {'.o','.pcm','.a'}};assert now==original
private={str(p.relative_to(E)):sha(p)for p in (P/'private').rglob('*')if p.is_file()};assert private==copied
write(P/'compiled-stability.json',dict(shared_entries=len(now),private_entries=len(private),membership_and_bytes_unchanged=True,private_implicit_modules_disabled=True))
