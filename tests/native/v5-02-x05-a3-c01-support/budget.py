"""Private remaining-budget probe of real cumulative final reservations, no host exhaustion."""
from pathlib import Path
import hashlib,json,shlex,subprocess,sys,os,time
R=Path.cwd();B=R/'build/rewrite-debug';O=R/'tests/native/v5-02-x05-a3-c01-evidence'/sys.argv[1];O.mkdir(exist_ok=False);records=[]
def run(label,args,remaining=None,high=False):
 env=os.environ.copy()
 if remaining is not None:env['C01_REMAINING']=str(remaining)
 if high:env['C01_HIGH']='1'
 t=time.monotonic();p=subprocess.run(list(map(str,args)),cwd=B,env=env,text=True,capture_output=True);(O/(label+'.log')).write_text(p.stdout+p.stderr);records.append(dict(label=label,argv=list(map(str,args)),remaining=remaining,exit_code=p.returncode,seconds=time.monotonic()-t));(O/'commands.json').write_text(json.dumps(records,indent=2)+'\n');assert p.returncode==0,(label,p.stdout+p.stderr);return p.stdout
entries=json.loads((B/'compile_commands.json').read_text())
def compile(label,suffix,text):
 p=O/(label+'.cpp');p.write_text(text);a=shlex.split(next(e['command'] for e in entries if e['file'].endswith(suffix)));a[a.index('-c')+1]=str(p);a[a.index('-o')+1]=str(O/(label+'.o'));run('compile-'+label,a);return O/(label+'.o')
base=(R/'src/engine/runtime/runtime.cpp').read_text();subs=[('  auto reserve = [&](std::size_t bytes, std::size_t passes) {','''  if(auto remaining=std::getenv("C01_REMAINING"))s.native_bytes=memory_max-std::stoull(remaining);
  auto c01_start=s.native_bytes;
  auto reserve = [&](std::size_t bytes, std::size_t passes) {'''),('  auto embedded = history::prepare_stdlib(c.originals, c.separator, c.assembly);','''  std::println("final_reservation_charge={}",s.native_bytes-c01_start);
  auto embedded = history::prepare_stdlib(c.originals, c.separator, c.assembly);'''),('    if (lua)\n      lua_close(lua);','''    if (lua) {lua_close(lua);if(runtime.memory!=0)std::abort();}''')];text=base
for a,b in subs:assert text.count(a)==1;text=text.replace(a,b)
a='  for (auto b : effective->digest)'
b='  auto actual_metadata=std::ranges::find(embedded->files,"__planar__/assembly.canonical",&history::tree_file::path)->payload.size();\n  auto actual_effective=std::ranges::find(effective->files,"__planar__/effective.bin",&history::tree_file::path)->payload.size();\n  if(actual_metadata!=metadata)throw external_protocol();\n  std::println("metadata_bytes={} effective_manifest_bytes={} catalog_bytes={}",actual_metadata,actual_effective,c.effective_catalog_bytes.size());\n  for (auto b : effective->digest)'
assert text.count(a)==1;text=text.replace(a,b);subs.append((a,b))
rev=text
for a,b in reversed(subs):rev=rev.replace(b,a)
assert rev==base;(O/'delta.json').write_text(json.dumps(dict(original_sha256=hashlib.sha256(base.encode()).hexdigest(),reverse_identical=True,substitutions=[dict(before=a,after=b) for a,b in subs]),indent=2)+'\n')
main='''import std;
import planar.engine.runtime;
using namespace planar::engine;
int main(){auto p=std::make_shared<profile_closure>();p->profile={"profile","profile.json","0644",R"({"version":1,"modules":[]})"};auto vm=command_vm::create(p);if(!vm){std::println("{} / {} / {}",vm.error().detail.code,vm.error().detail.message,vm.error().field);return vm.error().detail.code=="resource_limit"&&vm.error().detail.message=="Native conversion allocation exceeded"&&vm.error().field=="catalog"?0:1;}std::println("success catalog_bytes={}",vm->effective_catalog().size());return 0;}
'''
fixture=(R/'src/engine/runtime/external_registration.t.cpp').read_text().split('TEST_CASE("X05 A3 C01 high catalog bytes remain valid"')[1]
fixture=fixture[fixture.index('  std::string source'):fixture.index('  auto vm =')]
main=main.replace('auto vm=command_vm::create(p);','if(std::getenv("C01_HIGH")){'+fixture+'p->profile.bytes=R"({"version":1,"modules":["m1.lua"]})";p->modules.push_back({"modules","m1.lua","0644",source});}auto vm=command_vm::create(p);')
po=compile('main' ,'/runtime/commands.t.cpp',main);ro=compile('runtime','/runtime/runtime.cpp',text)
a=shlex.split(subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=B,text=True).splitlines()[-1].split(' && ')[1]);libs=[x for x in a[a.index('src/engine/libplanar_runtime.a'):] if 'catch2' not in x and x!='src/engine/libplanar_runtime.a'];binary=O/'probe';run('link',[a[0],'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++',po,ro,*libs,'-o',binary]);high=run('high-catalog',[binary],high=True);assert 'success' in high;highcharge=int(high.split('final_reservation_charge=')[1].splitlines()[0]);assert 'success' in run('high-exact-control',[binary],highcharge,True);assert 'resource_limit' in run('high-one-byte-short',[binary],highcharge-1,True);measure=run('measure',[binary]);charge=int(measure.split('final_reservation_charge=')[1].splitlines()[0]);control=run('exact-control',[binary],charge);failure=run('one-byte-short',[binary],charge-1);assert 'success' in control and 'resource_limit' in failure and 'final_reservation_charge' not in failure;(O/'summary.json').write_text(json.dumps(dict(actual_charge=charge,control_remaining=charge,refusal_remaining=charge-1,live_lua_zero_after_destruction=True,metric='controlled remaining charged native budget, not host exhaustion'),indent=2)+'\n');print(charge,'exact control and one-byte-short refusal pass')
