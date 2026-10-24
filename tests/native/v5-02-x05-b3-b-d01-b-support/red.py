"""Isolate baseline ordinary-decode diagnostic regression without shared outputs."""
from pathlib import Path
import json,shlex,subprocess,hashlib,time
root=Path(__file__).resolve().parents[3];build=root/'build/rewrite-debug';out=root/'tests/native/v5-02-x05-b3-b-d01-b-evidence/red';out.mkdir()
commands=[]
def snapshot():return {str(p.relative_to(build)):hashlib.sha256(p.read_bytes()).hexdigest() for p in build.rglob('*') if p.is_file() and p.suffix in {'.o','.pcm','.a','.so','.dylib'}}
before=snapshot();(out/'shared-before.json').write_text(json.dumps(before,indent=2)+'\n')
def expand(args):
 result=[]
 for a in args:result.extend(expand(shlex.split((build/a[1:]).read_text()))) if a.startswith('@') else result.append(a)
 return result
def command(file,source,obj):
 entry=next(e for e in json.loads((build/'compile_commands.json').read_text()) if e['file'].endswith(file));a=expand(shlex.split(entry['command']));a[a.index('-c')+1]=str(source);a[a.index('-o')+1]=str(obj)
 if '-MF' in a:a[a.index('-MF')+1]=str(obj)+'.d'
 return ['-fmodule-output='+str(obj)+'.pcm' if x.startswith('-fmodule-output=') else x for x in a]
def run(label,args):
 t=time.time();p=subprocess.run(list(map(str,args)),cwd=build,capture_output=True,text=True);(out/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=list(map(str,args)),cwd=str(build),exit=p.returncode,seconds=time.time()-t));(out/'commands.json').write_text(json.dumps(commands,indent=2)+'\n');print(label,p.returncode,flush=True)
 if p.returncode:raise SystemExit(p.returncode)
source=root/'tests/native/v5-02-x05-b3-b-d01-b-evidence/baseline/src/engine/json_codec/codec.cpp'
(out/'baseline.cpp').write_bytes(source.read_bytes())
a=command('json_codec/codec.cpp',out/'baseline.cpp',out/'baseline.o');run('baseline-compile',a)
prefix=(root/'tests/native/v5-02-x05-b3-b-d01-b-support/probes.cpp').read_text().split('namespace {\nusing namespace planar::engine;')[0]
(out/'red.cpp').write_text(prefix+'''\nTEST_CASE("D01-B baseline ordinary diagnostic allocation escapes", "[red]") {
 using namespace planar::engine;
 for (auto input:{"null","[true]"}) {
   bool escaped=false;
   observation::start(1);
   try {auto r=codec_decode(input,{input==std::string_view("[true]")?5u:1048576u,128,100000});}
   catch (const std::bad_alloc&) {escaped=true;}
   observation::enabled=false;
   CHECK(escaped);
 }
}
''')
b=command('json_codec/codec.t.cpp',out/'red.cpp',out/'red.o');run('red-compile',b)
run('link',[a[0],'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++',out/'baseline.o',out/'red.o','-o',out/'red','vendor/catch2/src/libCatch2Maind.a','src/engine/lib@cmake_cxx_std.a','src/engine/libplanar_value.a','vendor/catch2/src/libCatch2d.a'])
run('red',[out/'red','--reporter','junit','--out',out/'red.xml']);assert before==snapshot();(out/'shared-stability.json').write_text(json.dumps(dict(entries=len(before),membership_and_hashes_unchanged=True),indent=2)+'\n')
