"""Private exact-old-body differential and real production-freezer assembly probes.

Only runtime setup/test copies change. Response files expand recursively, all
object/PCM/dependency outputs are isolated, and shared compiled identities must
remain identical. No product command admission or native F validation is added.
"""
from pathlib import Path
import hashlib,json,re,shlex,subprocess,sys,time
ROOT=Path(__file__).resolve().parents[3]
BUILD=ROOT/'build/rewrite-debug'
EVIDENCE=ROOT/'tests/native/v5-02-x05-c1-evidence'
OUT=EVIDENCE/sys.argv[1]
OUT.mkdir()
COMMANDS=[]
def sha(data):return hashlib.sha256(data).hexdigest()
def write(name,obj):(OUT/name).write_text(json.dumps(obj,indent=2,sort_keys=True)+'\n')
def snapshot():return {str(p.relative_to(BUILD)):sha(p.read_bytes()) for p in BUILD.rglob('*') if p.is_file() and p.suffix in {'.pcm','.o','.a'}}
def run(label,args,expected=0):
 start=time.time();p=subprocess.run(list(map(str,args)),cwd=BUILD,capture_output=True,text=True)
 (OUT/(label+'.log')).write_text(p.stdout+p.stderr)
 COMMANDS.append(dict(label=label,argv=list(map(str,args)),cwd=str(BUILD),exit=p.returncode,seconds=time.time()-start))
 write('commands.json',COMMANDS);print(label,p.returncode,flush=True)
 assert p.returncode==expected,(label,(p.stdout+p.stderr)[-4000:])
 return p.stdout
ENTRIES=json.loads((BUILD/'compile_commands.json').read_text())
def compile(label,suffix,path):
 cmd=shlex.split(next(e['command'] for e in ENTRIES if e['file'].endswith(suffix)))
 def expand(args):
  result=[]
  for a in args:
   if a.startswith('@'):result.extend(expand(shlex.split((BUILD/a[1:]).read_text())))
   else:result.append(a)
  return result
 cmd=expand(cmd);cmd[cmd.index('-c')+1]=str(path);cmd[cmd.index('-o')+1]=str(OUT/(label+'.o'))
 cmd=[('-fmodule-output='+str(OUT/(label+'.pcm'))) if a.startswith('-fmodule-output=') else a for a in cmd]
 if '-MF' in cmd:cmd[cmd.index('-MF')+1]=str(OUT/(label+'.d'))
 assert not any(a.startswith('@') for a in cmd)
 run(label+'-compile',cmd+['-Wno-overlength-strings']);return OUT/(label+'.o')
link=shlex.split(subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=BUILD,text=True).splitlines()[-1].split(' && ')[1])
libs=link[link.index('src/engine/libplanar_runtime.a'):]
common=[link[0],'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++']
shared=snapshot();write('shared-compiled-before.json',shared)
runtime=(ROOT/'src/engine/runtime/runtime.cpp').read_text()
embedded=(BUILD/'src/engine/planar_runtime_source.cppm').read_text()
body=re.search(r'constexpr unsigned char source_bytes\[\] = \{([^}]*)\};',embedded).group(1)
source=bytes(int(x,16) for x in re.findall(r'0x([0-9a-f]{2})',body)).rstrip(b'\0').decode()
# Inject the actual pre-edit render file with only lexical function renames.
original=(EVIDENCE/'original-source/src/stdlib/planar/render.lua').read_text()
renamed=original
names=['render_material','normalize_render_request','prepare_profile_render','render','render_profile_material']
for name in names:renamed=re.sub(r'\b'+name+r'(?=\()',name+'_before',renamed)
restored=renamed
for name in names:restored=re.sub(r'\b'+name+r'_before(?=\()',name,restored)
assert restored==original
marker='-- Product-owned export table;'
assert source.count(marker)==1
modified=source.replace(marker,renamed+'\n'+marker)
modified=modified.replace('  render = render,','  render = render,\n  render_before = render_before,\n  render_profile_material_before = render_profile_material_before,')
def setup_copy(label,lua):
 needle='  auto source = embedded::source();';assert runtime.count(needle)==1
 text=runtime.replace(needle,'  constexpr std::string_view source=R"C1PROBE('+lua+')C1PROBE";')
 (OUT/(label+'.cpp')).write_text(text)
 return text
setup_copy('differential-runtime',modified)
write('old-body-transformation.json',dict(original_sha256=sha(original.encode()),renamed_sha256=sha(renamed.encode()),reverse_identical=True,function_renames=names,current_assembly_sha256=sha(source.encode()),private_assembly_sha256=sha(modified.encode()),scope='run_trusted setup only; command owner still uses current embedded bytes'))
comparator='''  if (entry == "render" || entry == "render_profile_material") {
    auto before = run_trusted(std::string(entry) + "_before", input);
    REQUIRE(before);
    REQUIRE(result);
    CHECK(*codec_encode(*before) == *codec_encode(*result));
    std::println("C1 old-body exact response comparison {}", entry);
  }
'''
objects=[]
for name in ['schema','external_registration']:
 text=(ROOT/f'src/engine/runtime/{name}.t.cpp').read_text()
 needle='  auto result = run_trusted(entry, input);';assert text.count(needle)==1
 text=text.replace(needle,needle+'\n'+comparator)
 (OUT/(name+'.cpp')).write_text(text)
 objects.append(compile(name,f'/runtime/{name}.t.cpp',OUT/(name+'.cpp')))
obj=compile('differential-runtime','/runtime/runtime.cpp',OUT/'differential-runtime.cpp')
run('differential-link',common+objects+[obj]+libs+['-o',OUT/'differential'])
log=run('differential',[OUT/'differential','[render],[profile-render]','--reporter','junit','--out',OUT/'differential.xml'])
# Catch JUnit captures stdout into the XML system-out section.
xml=(OUT/'differential.xml').read_text()
comparisons=log.count('C1 old-body exact response comparison')
assert comparisons>60,comparisons
write('differential-summary.json',dict(exact_response_comparisons=comparisons,archived_original_sha256=sha(original.encode()),includes='all existing render/profile-render fixtures and goldens, workers/reviewers/recovery, capture/observations, optional budgets and errors'))
# Exercise the actual existing native freezer on run_trusted input immediately
# before Lua invocation. Fresh backings/sealed/memo are private setup only.
freeze='''  int input = lua_gettop(l);
  lua_newtable(l); int memo=lua_gettop(l);
  lua_newtable(l); int backings=lua_gettop(l);
  lua_pushvalue(l,backings);lua_rawseti(l,LUA_REGISTRYINDEX,backing_index);
  lua_newtable(l); int sealed=lua_gettop(l);
  freeze_graph(l,input,memo,backings,sealed);
  lua_settop(l,input);
'''
frozen_lua=source.replace('  assemble_projection_material = assemble_projection_material,', '''  assemble_projection_material = function(input)
    local ok=pcall(function() input.base={} end)
    if ok then error("input root was not frozen") end
    if input.base[1] then
      ok=pcall(function() input.base[1].text="changed" end)
      if ok then error("base record was not frozen") end
    end
    if input.custom[1] then
      ok=pcall(function() input.custom[1].text="changed" end)
      if ok then error("custom record was not frozen") end
    end
    local first=assemble_projection_material(input)
    local second=assemble_projection_material(input)
    if presentation_json(first)~=presentation_json(second) then error("repeat changed frozen input") end
    return first
  end,''')
text=setup_copy('frozen-runtime',frozen_lua)
needle='int setup(lua_State* l) {';assert text.count(needle)==1
text=text.replace(needle,'void freeze_graph(lua_State*,int,int,int,int);\n'+needle)
needle='  push(l, context(l).input.get());\n  lua_call(l, 1, LUA_MULTRET);';assert text.count(needle)==1
text=text.replace(needle,'  push(l, context(l).input.get());\n'+freeze+'  lua_call(l, 1, LUA_MULTRET);')
(OUT/'frozen-runtime.cpp').write_text(text)
obj=compile('frozen-runtime','/runtime/runtime.cpp',OUT/'frozen-runtime.cpp')
# Same external tests, selected only pure C1 algorithm cases. The differential
# wrapper is inert for this export.
run('frozen-link',common+[objects[1],obj]+libs+['-o',OUT/'frozen'])
run('frozen',[OUT/'frozen','[projection-assembly]','--reporter','junit','--out',OUT/'frozen.xml'])
assert snapshot()==shared
write('shared-compiled-stability.json',dict(entries=len(shared),membership_and_hashes_identical=True))
print('PRIVATE DIFFERENTIAL AND ACTUAL FREEZER PASS',comparisons,flush=True)
