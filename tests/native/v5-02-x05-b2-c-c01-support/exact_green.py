from pathlib import Path
import subprocess,json,shlex,hashlib,time,sys
root=Path(__file__).resolve().parents[3];out=root/'tests/native/v5-02-x05-b2-c-c01-evidence'/ (sys.argv[1] if len(sys.argv)>1 else 'exact-green');out.mkdir();build=root/'build/rewrite-debug';commands=[]
def run(label,args):
 start=time.time();p=subprocess.run([str(x) for x in args],cwd=build,capture_output=True,text=True)
 (out/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=[str(x) for x in args],exit=p.returncode,seconds=time.time()-start));(out/'probe-commands.json').write_text(json.dumps(commands,indent=2)+'\n');print(label,p.returncode,flush=True);return p.returncode
entries=json.loads((build/'compile_commands.json').read_text())
def compile(label,suffix,path):
 cmd=shlex.split(next(e['command'] for e in entries if e['file'].endswith(suffix)));cmd[cmd.index('-c')+1]=str(path);cmd[cmd.index('-o')+1]=str(out/(label+'.o'));assert run(label+'-compile',cmd)==0;return out/(label+'.o')
base=(root/'src/engine/runtime/runtime.cpp').read_text()
helper='''
void review_references(lua_State* l,int stage) {
 for(auto& entry:commands(l).entries) {
  lua_rawgeti(l,LUA_REGISTRYINDEX,entry.reference);
  review_reference(stage,entry.reference,lua_type(l,-1),lua_topointer(l,-1));
  lua_pop(l,1);
 }
}
'''
subs=[('#include <cstdlib>','#include <cstdlib>\nextern "C" void review_reference(int,int,int,const void*);'),('int execute_shape(lua_State* l) {',helper+'\nint execute_shape(lua_State* l) {\n  review_references(l,0);'),('        release_shape_temporaries(owner.lua);','        release_shape_temporaries(owner.lua);\n        review_references(owner.lua,1);')]
variant=base
for a,b in subs:assert variant.count(a)==1;variant=variant.replace(a,b)
reverse=variant
for a,b in reversed(subs):reverse=reverse.replace(b,a)
assert reverse==base
(out/'registry-runtime.cpp').write_text(variant)
(out/'registry-transform.json').write_text(json.dumps(dict(original_sha256=hashlib.sha256(base.encode()).hexdigest(),variant_sha256=hashlib.sha256(variant.encode()).hexdigest(),reverse_identical=True,substitutions=[dict(before=a,after=b) for a,b in subs]),indent=2)+'\n')
runtime=compile('registry-runtime','/runtime/runtime.cpp',out/'registry-runtime.cpp');main=compile('registry-main','/runtime/external_registration.t.cpp',root/'tests/native/v5-02-x05-b2-c-review-evidence/registry.cpp')
link=shlex.split(subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=build,text=True).splitlines()[-1].split(' && ')[1]);libs=link[link.index('src/engine/libplanar_runtime.a'):];libs=[a for a in libs if 'catch2' not in a and a!='src/engine/libplanar_runtime.a']
assert run('registry-link',[link[0],'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++',main,runtime,*libs,'-o',out/'registry'])==0
assert run('registry',[out/'registry'])==0
