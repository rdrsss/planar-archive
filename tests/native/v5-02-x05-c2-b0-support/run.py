"""Private standalone compile; shared compiled/module/archive dependencies stay read-only."""
from pathlib import Path
import hashlib,json,shlex,shutil,subprocess,sys,time
ROOT=Path(__file__).resolve().parents[3];E=ROOT/'tests/native/v5-02-x05-c2-b0-evidence';B=ROOT/'build/rewrite-debug';S=Path(__file__).parent
sys.dont_write_bytecode=True
from protect import check_support
if (E/'frozen-support.json').exists():check_support()
P=E/sys.argv[1];P.mkdir();commands=[]
shutil.copyfile(S/'counting.cpp',P/'counting.cpp')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(label,args):
 start=time.time();p=subprocess.run([str(x)for x in args],cwd=ROOT,capture_output=True,text=True);(P/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=[str(x)for x in args],exit=p.returncode,seconds=time.time()-start));(P/'commands.json').write_text(json.dumps(commands,indent=2)+'\n');print(label,p.returncode,flush=True)
 if p.returncode:print((p.stdout+p.stderr)[-6000:]);sys.exit(1)
 return p.stdout
# Copy full compiled membership, not only directly linked dependencies.
original=json.loads((E/'compiled-before.json').read_text());copied={}
for relative,digest in original.items():
 src=ROOT/relative;assert sha(src)==digest,relative;dst=P/'private'/src.relative_to(B);dst.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(src,dst);copied[str(dst.relative_to(E))]=sha(dst);assert sha(dst)==digest
(P/'copied-compiled.json').write_text(json.dumps(copied,sort_keys=True,indent=2)+'\n')
entries=json.loads((B/'compile_commands.json').read_text());cmd=shlex.split(next(e['command']for e in entries if e['file'].endswith('/json_codec/codec.cpp')))
def expand(args):
 result=[]
 for a in args:
  result.extend(expand(shlex.split((B/a[1:]).read_text())))if a.startswith('@')else result.append(a)
 return result
cmd=expand(cmd)
# Every module and all linked objects are private copies. Expand and absolutize.
for i,a in enumerate(cmd):
 if a.startswith('-fmodule-file='):
  key,path=a[len('-fmodule-file='):].split('=',1);cmd[i]='-fmodule-file='+key+'='+str(P/'private'/path)
cmd[cmd.index('-c')+1]=str(S/'counting.cpp');cmd[cmd.index('-o')+1]=str(P/'counting.o')
if '-MF'in cmd:cmd[cmd.index('-MF')+1]=str(P/'counting.d')
run('compile',cmd)
common=[cmd[0],'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++']
libs=[P/'private/src/engine/libplanar_json_codec.a',P/'private/src/engine/libplanar_value.a']
run('link',common+[P/'counting.o',*libs,'-o',P/'counting'])
run('counting',[P/'counting'])
# A throwing streaming sink must terminate at the pinned noexcept flush helper.
args=[str(P/'counting'),'--throw-flush'];p=subprocess.run(args,cwd=P,capture_output=True,text=True);(P/'throw-flush.log').write_text(p.stdout+p.stderr);assert p.returncode==-6,p.returncode
commands.append(dict(label='throw-flush',argv=args,exit=p.returncode,expected_signal='SIGABRT'));(P/'commands.json').write_text(json.dumps(commands,indent=2)+'\n')
if (E/'frozen-support.json').exists():check_support()
now={str(p.relative_to(ROOT)):sha(p)for p in B.rglob('*')if p.is_file()and p.suffix in {'.o','.pcm','.a'}};assert now==original
(P/'compiled-stability.json').write_text(json.dumps(dict(entries=len(now),unchanged=True),indent=2)+'\n')
