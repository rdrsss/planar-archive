"""Compile private fault/read checks with copied dependencies and no implicit modules."""
from pathlib import Path
import hashlib,json,os,shlex,shutil,stat,subprocess,time,xml.etree.ElementTree as ET
import sys
sys.dont_write_bytecode = True
from freeze import ROOT as R,EVIDENCE as E,verify_candidate,write
B=R/'build/rewrite-debug';S=Path(__file__).parent;P=E/'private-dependencies';O=E/'private-probes';O.mkdir();commands=[]
def entry(p):
 return dict(mode=stat.S_IMODE(p.stat().st_mode),size=p.stat().st_size,sha256=hashlib.sha256(p.read_bytes()).hexdigest())
def compiled():
 return {str(p.resolve()):entry(p) for p in B.rglob('*') if p.is_file() and p.suffix in {'.o','.pcm','.a','.dylib'}}
def run(label,args,cwd=R,expected=0):
 verify_candidate();start=time.time();p=subprocess.run(list(map(str,args)),cwd=cwd,capture_output=True,text=True)
 (O/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=list(map(str,args)),cwd=str(cwd),exit=p.returncode,seconds=time.time()-start));write(O/'commands.json',commands)
 print(label,p.returncode,flush=True)
 assert p.returncode==expected,(label,(p.stdout+p.stderr)[-3000:]);verify_candidate();return p.stdout
before=compiled()
entries=json.loads((B/'compile_commands.json').read_text())
def expand(args):
 result=[]
 for a in args:
  if a.startswith('@'):result.extend(expand(shlex.split((B/a[1:]).read_text())))
  else:result.append(a)
 return result
args=expand(shlex.split(next(e['command'] for e in entries if e['file'].endswith('/runtime/external_registration.t.cpp'))))
implargs=expand(shlex.split(next(e['command'] for e in entries if e['file']==str(R/'src/engine/runtime/runtime.cpp'))))
for a in args+implargs:
 if a.startswith('-fmodule-file='):
  path=Path(a[len('-fmodule-file='):].split('=',1)[1]);path=(B/path).resolve()
  before[str(path)]=entry(path)
for p in Path('/opt/homebrew/opt/llvm/lib/c++').glob('*.dylib'):
 before[str(p.resolve())]=entry(p)
write(O/'compiled-before.json',before)
def destination(path):
 path=Path(path).resolve()
 return P/str(path).lstrip('/')
for name,identity in before.items():
 target=destination(name);target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(name,target);assert entry(target)==identity
write(O/'copied-before.json',{str(destination(name)):entry(destination(name)) for name in before})
def redirect(arguments,source,object_name):
 result=list(arguments)
 for i,a in enumerate(result):
  if a.startswith('-fmodule-file='):
   key,path=a[len('-fmodule-file='):].split('=',1);result[i]='-fmodule-file='+key+'='+str(destination(B/path))
  elif a.startswith('-fmodule-output='):result[i]='-fmodule-output='+str(O/(object_name+'.pcm'))
 for flag,suffix in [('-o','.o'),('-MF','.d'),('-MT','.o')]:
  if flag in result:result[result.index(flag)+1]=str(O/(object_name+suffix))
 result[result.index('-c')+1]=str(source)
 result+=['-fno-implicit-modules','-fno-implicit-module-maps']
 return result
source=O/'combined.cpp';source.write_text((R/'src/engine/runtime/external_registration.t.cpp').read_text()+'\n'+(S/'allocator-probes.inc').read_text())
run('compile',redirect(args,source,'probes'),B)
linkline=subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=B,text=True).splitlines()[-1]
link=shlex.split(linkline.split(' && ')[1]);libs=link[link.index('src/engine/libplanar_runtime.a'):]
libs=[str(destination(B/a)) if a.endswith(('.a','.o','.dylib')) else a for a in libs]
libs.append(str(destination(B/'src/engine/libplanar_admission_syscalls.dylib')))
common=[link[0],'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,'+str(destination(B/'src/engine/libplanar_admission_syscalls.dylib').parent)]
run('link',common+[O/'probes.o',*libs,'-o',O/'probes'],B)
run('probes',[O/'probes','[c4-a-private]','--reporter','junit','--out',O/'probes.xml'])
cases=ET.parse(O/'probes.xml').findall('.//testcase');assert len(cases)==4 and all(not any(c.find(t) is not None for t in ['failure','error','skipped']) for c in cases)
# Verification-only source copies: exact production path with explicit GC or
# controlled remaining inherited counters. No public seam or reset is shipped.
original=(R/'src/engine/runtime/runtime.cpp').read_text()
marker='      lua_pushcfunction(l, scan_extension_response);'
assert original.count(marker)==1
for label,injection,selector in [
 ('forced-gc','      lua_gc(l, LUA_GCCOLLECT);\n','C4 A retained registration aliases*'),
 ('node-boundary','      s.nodes = node_max;\n','[c4-a-node-variant]'),
 ('native-boundary','      s.native_bytes = memory_max - 512;\n','[c4-a-native-variant]')]:
 variant=O/(label+'-runtime.cpp');variant.write_text(original.replace(marker,injection+marker))
 run('compile-'+label,redirect(implargs,variant,label+'-runtime'),B)
 archive=O/('lib'+label+'-runtime.a')
 interface=destination(B/'src/engine/CMakeFiles/planar_runtime.dir/runtime/runtime.cppm.o')
 run('archive-'+label,['/opt/homebrew/opt/llvm/bin/llvm-ar','qc',archive,interface,O/(label+'-runtime.o')],B)
 copied_runtime=destination(B/'src/engine/libplanar_runtime.a')
 variantlibs=[str(archive) if value==str(copied_runtime) else value for value in libs]
 executable=O/(label+'-probes')
 run('link-'+label,common+[O/'probes.o',*variantlibs,'-o',executable],B)
 run(label,[executable,selector,'--reporter','junit','--out',O/(label+'.xml')])
 variantcases=ET.parse(O/(label+'.xml')).findall('.//testcase')
 assert len(variantcases)==1 and all(not any(case.find(tag) is not None for tag in ['failure','error','skipped']) for case in variantcases)
# The only added shared field is one bool beside two existing bools. Rebuild
# the exact prior command-state declaration and compare command and full owners.
start=original.index('struct command_state {')
end=original.index('command_state& commands(',start)
prior=original[start:end].replace('struct command_state {','struct command_state_before_c4 {',1)
assert prior.count(', extension_execution = false')==1
prior=prior.replace(', extension_execution = false','')
layout=original[:end]+prior+original[end:]
point='  std::unique_ptr<implementation> owner;'
assert layout.count(point)==1
checks='''  struct owner_before_c4 {
    state runtime;
    command_state_before_c4 command;
    lua_State* lua;
  };
  static_assert(sizeof(command_state)==sizeof(command_state_before_c4));
  static_assert(alignof(command_state)==alignof(command_state_before_c4));
  static_assert(sizeof(implementation)==sizeof(owner_before_c4));
  static_assert(alignof(implementation)==alignof(owner_before_c4));
  std::println("layout {} command {} prior {} owner {} prior {} state {}",profile ? "profile" : "embedded",
               sizeof(command_state),sizeof(command_state_before_c4),sizeof(implementation),sizeof(owner_before_c4),sizeof(state));
'''
layout=layout.replace(point,checks+point)
path=O/'layout-runtime.cpp';path.write_text(layout)
run('compile-layout',redirect(implargs,path,'layout-runtime'),B)
archive=O/'liblayout-runtime.a'
run('archive-layout',['/opt/homebrew/opt/llvm/bin/llvm-ar','qc',archive,interface,O/'layout-runtime.o'],B)
layoutlibs=[str(archive) if value==str(copied_runtime) else value for value in libs]
run('link-layout',common+[O/'probes.o',*layoutlibs,'-o',O/'layout-probes'],B)
output=run('layout',[O/'layout-probes','[c4-a-layout-variant]','--reporter','junit','--out',O/'layout.xml'])
assert 'layout embedded' in output and 'layout profile' in output
write(O/'layout-proof.json',dict(shared_state_unchanged=True,added_shared_bool_in_existing_padding=True,
    command_and_complete_owner_size_alignment_equal=True,both_owner_positive_controls=True,raw_output=output))
# Reconstruct only the pre-fix fallback staging after the original exploratory
# assertion was corrected. This is a disclosed post-fix regression reconstruction.
pre=original
factory='''    charge(vm->_owner->runtime, sizeof(implementation) + 64);
    vm->_owner->command.registration_field = "catalog.commands";'''
assert pre.count(factory)==1
pre=pre.replace(factory,'    charge(vm->_owner->runtime, sizeof(implementation));')
staging='''      if (!_owner->executed && !c.shape_failure) {
        charge(s, 64);
        c.registration_field = "catalog.commands";
      }
'''
assert pre.count(staging)==1
pre=pre.replace(staging,'')
prepath=O/'preselection-runtime.cpp';prepath.write_text(pre)
run('compile-preselection',redirect(implargs,prepath,'preselection-runtime'),B)
archive=O/'libpreselection-runtime.a'
run('archive-preselection',['/opt/homebrew/opt/llvm/bin/llvm-ar','qc',archive,interface,O/'preselection-runtime.o'],B)
prelibs=[str(archive) if value==str(copied_runtime) else value for value in libs]
run('link-preselection',common+[O/'probes.o',*prelibs,'-o',O/'preselection-probes'],B)
run('red-preselection',[O/'preselection-probes','C4 A lifecycle refuses before admission*','--reporter','junit','--out',O/'preselection.xml'],expected=42)
red=ET.parse(O/'preselection.xml')
assert len(red.findall('.//testcase'))==1 and len(red.findall('.//failure'))==1
assert 'catalog.commands' in red.find('.//failure').get('message','')
run('green-preselection',[O/'probes','C4 A lifecycle refuses before admission*','--reporter','junit','--out',O/'preselection-green.xml'])
assert compiled()=={k:v for k,v in before.items() if Path(k).is_relative_to(B)}
assert all(entry(Path(k))==v and entry(destination(k))==v for k,v in before.items())
write(O/'compiled-final.json',dict(entries=len(before),shared_and_copied_unchanged=True,all_responses_expanded=True,implicit_modules_disabled=True,cases=4,final_product_probes_green=True))
# Disposable compiled copies are removed only after every comparison; manifests/sources/logs remain.
shutil.rmtree(P)
for p in O.iterdir():
 if p.suffix in {'.o','.a','.pcm'} or p.name in {'probes','forced-gc-probes','node-boundary-probes','native-boundary-probes','preselection-probes','layout-probes'}:p.unlink()
print('PRIVATE PROBES COMPLETE',flush=True)
