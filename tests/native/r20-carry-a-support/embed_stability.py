# R20-CARRY-A: cmake/embed_lua.cmake writes each module once (I19-I21).
"""argv: case, repository root, cmake path, ninja path (used only to select the generator).

No C++ toolchain: the scratch project is `project(NONE)` with a no-op planar_rewrite_module defined before the helper is
included. embed_unchanged_mtime observes mtime by backdating, never by sleeping. embed_content_change_rewrites compares the
helper with an in-test legacy-idiom reference (embed without REMAP, then read back, REPLACE, CONFIGURE) for five shapes.
embed_callsite_audit walks the CMake-reachable roots without git and pins the nine REMAP call sites.
"""
from pathlib import Path
import fnmatch,os,re,sys
sys.dont_write_bytecode=True
from common import OLD_NS,expect,scratch,run_ok,append_marker,dispatch
repo=Path(sys.argv[2]).resolve();cmake=sys.argv[3];ninja=sys.argv[4] if len(sys.argv)>4 else ''

PRELUDE='''cmake_minimum_required(VERSION 3.20)
project(embedscratch NONE)
function(planar_rewrite_module)
  set_property(GLOBAL APPEND PROPERTY R20_REGISTERED "${ARGV0}|${ARGV1}|${ARGV2}")
endfunction()
include("@REPO@/cmake/embed_lua.cmake")
set(S "${PROJECT_SOURCE_DIR}")
set(GEN "${PROJECT_SOURCE_DIR}/gen/deep/path")
# Legacy idiom reference: embed without REMAP, read the module back, REPLACE in the given order, CONFIGURE to another path.
# order RL = REMAP pairs then LITERAL pairs (the old call-site order); LR = the reverse (used only to prove order sensitivity).
function(legacy_ref tag order module remap_flat literal_flat)
  planar_embed_lua(ref_${tag}_${order} ${module} ${ARGN})
  file(READ "${CMAKE_CURRENT_BINARY_DIR}/ref_${tag}_${order}.cppm" text)
  foreach(step IN ITEMS ${order})
  endforeach()
  if(order STREQUAL "RL")
    set(steps remap literal)
  else()
    set(steps literal remap)
  endif()
  foreach(step IN LISTS steps)
    if(step STREQUAL "remap")
      list(LENGTH remap_flat count)
      set(i 0)
      while(i LESS count)
        list(GET remap_flat ${i} abs)
        math(EXPR j "${i} + 1")
        list(GET remap_flat ${j} logical)
        file(RELATIVE_PATH name "${PROJECT_SOURCE_DIR}/src/stdlib" "${abs}")
        if(name MATCHES "^\\\\.\\\\.")
          file(RELATIVE_PATH name "${PROJECT_SOURCE_DIR}" "${abs}")
        endif()
        string(REPLACE "${name}" "${logical}" text "${text}")
        math(EXPR i "${i} + 2")
      endwhile()
    else()
      list(LENGTH literal_flat count)
      set(i 0)
      while(i LESS count)
        list(GET literal_flat ${i} from)
        math(EXPR j "${i} + 1")
        list(GET literal_flat ${j} to)
        string(REPLACE "${from}" "${to}" text "${text}")
        math(EXPR i "${i} + 2")
      endwhile()
    endif()
  endforeach()
  file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/legacy_${tag}_${order}.cppm" CONTENT "${text}" @ONLY)
endfunction()
'''.replace('@REPO@',str(repo))
PRELUDE=PRELUDE.replace('  foreach(step IN ITEMS ${order})\n  endforeach()\n','')
JSON_LIT='"\\"planar/json.lua\\"" "\\"../json.lua\\""'
JSON_FLAT='"\\"planar/json.lua\\";\\"../json.lua\\""'
STD='${S}/src/stdlib/planar/'
SHAPES={
 # tag: (cmake text for the helper call under test, cmake text computing the references, Lua input mutated by the content change)
 'S1':dict(module='m.s1',parts=[STD+'json.lua','${GEN}/s1/commands.lua',STD+'bootstrap.lua'],
  call='REMAP "${GEN}/s1/commands.lua" planar/commands.lua',remap='"${GEN}/s1/commands.lua;planar/commands.lua"',literal='""',mutate='gen/deep/path/s1/commands.lua'),
 'S2':dict(module='m.s2',parts=[STD+'json.lua','${GEN}/s2/commands.lua',STD+'bootstrap.lua'],
  call='REMAP "${GEN}/s2/commands.lua" planar/commands.lua LITERAL '+JSON_LIT,remap='"${GEN}/s2/commands.lua;planar/commands.lua"',literal=JSON_FLAT,mutate='gen/deep/path/s2/commands.lua'),
 'S3':dict(module='m.s3',parts=[STD+'a.lua',STD+'b.lua'],
  call='REMAP "${S}/src/stdlib/planar/b.lua" planar/renamed.lua',remap='"${S}/src/stdlib/planar/b.lua;planar/renamed.lua"',literal='""',mutate='src/stdlib/planar/b.lua'),
 'S6':dict(module='m.s6',parts=[STD+'json.lua','${GEN}/s6/commands.lua','${GEN}/s6/other.lua'],
  call='REMAP "${GEN}/s6/commands.lua" planar/commands.lua "${GEN}/s6/other.lua" planar/other.lua',
  remap='"${GEN}/s6/commands.lua;planar/commands.lua;${GEN}/s6/other.lua;planar/other.lua"',literal='""',mutate='gen/deep/path/s6/other.lua'),
 'S7':dict(module='m.s7',parts=[STD+'json.lua',STD+'a.lua'],
  call='LITERAL '+JSON_LIT+' planar/a.lua ../a.lua',remap='""',
  literal='"\\"planar/json.lua\\";\\"../json.lua\\";planar/a.lua;../a.lua"',mutate='src/stdlib/planar/a.lua'),
 'S4':dict(module='m.s4',parts=[STD+'json.lua','${GEN}/s4/ord.lua'],
  call='REMAP "${GEN}/s4/ord.lua" planar/ord.lua LITERAL planar/ord.lua ../ord.lua',remap='"${GEN}/s4/ord.lua;planar/ord.lua"',literal='"planar/ord.lua;../ord.lua"',mutate='gen/deep/path/s4/ord.lua'),
}
LUA_FILES=['src/stdlib/planar/json.lua','src/stdlib/planar/bootstrap.lua','src/stdlib/planar/a.lua','src/stdlib/planar/b.lua',
 'gen/deep/path/s1/commands.lua','gen/deep/path/s2/commands.lua','gen/deep/path/s4/ord.lua','gen/deep/path/s6/commands.lua','gen/deep/path/s6/other.lua']

def quoted(parts):return ' '.join(f'"{p}"' for p in parts)

def shape_text(tag,s):
 out=f'planar_embed_lua(t_{tag} {s["module"]} {quoted(s["parts"])} {s["call"]})\n'
 out+=f'legacy_ref({tag} RL {s["module"]} {s["remap"]} {s["literal"]} {quoted(s["parts"])})\n'
 out+=f'legacy_ref({tag} LR {s["module"]} {s["remap"]} {s["literal"]} {quoted(s["parts"])})\n'
 return out

S5_PARTS=[STD+'json.lua',STD+'a.lua']
def s5_text():
 # S5: a bare REMAP (zero pairs) and a call without REMAP equal a plain call; a bare REMAP before LITERAL still parses LITERAL.
 out=f'planar_embed_lua(t_S5a m.s5 {quoted(S5_PARTS)} REMAP)\n'
 out+=f'planar_embed_lua(t_S5b m.s5 {quoted(S5_PARTS)})\n'
 out+=f'planar_embed_lua(t_S5c m.s5c {quoted(S5_PARTS)} REMAP LITERAL {JSON_LIT})\n'
 out+=f'legacy_ref(S5 RL m.s5 "" "" {quoted(S5_PARTS)})\n'
 out+=f'legacy_ref(S5c RL m.s5c "" {JSON_FLAT} {quoted(S5_PARTS)})\n'
 return out

def make_project(tmp,body):
 src=tmp/'proj';src.mkdir()
 for rel in LUA_FILES:
  p=src/rel;p.parent.mkdir(parents=True,exist_ok=True)
  p.write_text(f'-- scratch {rel}\nlocal M={{}}\nreturn M\n')
 # The S2/S5 json part must literally embed the quoted logical name "planar/json.lua" in its view row, which the helper does.
 (src/'CMakeLists.txt').write_text(PRELUDE+body)
 return src,tmp/'build'

def configure(src,build):
 args=[cmake,'-S',src,'-B',build]
 if ninja:args+=['-G','Ninja',f'-DCMAKE_MAKE_PROGRAM={ninja}']
 return run_ok(args,'cmake configure of the embed scratch project',timeout=120)

def read(build,name):return (build/name).read_bytes()
def mtime(build,name):return (build/name).stat().st_mtime_ns

def embed_unchanged_mtime():
 with scratch('r20-embed-') as tmp:
  src,build=make_project(tmp,shape_text('S1',SHAPES['S1']))
  configure(src,build)
  first=read(build,'t_S1.cppm');text=first.decode()
  expect(len(first)>0,'embed output is empty')
  expect('"planar/commands.lua"' in text,'embed output lacks the remapped logical name planar/commands.lua')
  expect('gen/deep/path/s1/commands.lua' not in text,'embed output still contains the scratch generated logical path')
  os.utime(build/'t_S1.cppm',ns=(OLD_NS,OLD_NS))
  configure(src,build)
  expect(mtime(build,'t_S1.cppm')==OLD_NS,'reconfigure over the unchanged tree moved the module mtime (the helper wrote it more than once)')
  expect(read(build,'t_S1.cppm')==first,'reconfigure over the unchanged tree changed the module bytes')

def embed_content_change_rewrites():
 for tag,s in SHAPES.items():
  with scratch('r20-embed-') as tmp:
   src,build=make_project(tmp,shape_text(tag,s));configure(src,build)
   t=f't_{tag}.cppm';ref=f'legacy_{tag}_RL.cppm';rev=f'legacy_{tag}_LR.cppm'
   expect(read(build,t)==read(build,ref),f'{tag}: first configure differs from the legacy-idiom reference')
   if tag=='S4':
    # The shape is provably order-sensitive: the two reference orders embed different bytes.
    expect(read(build,ref)!=read(build,rev),'S4: REMAP-then-LITERAL and LITERAL-then-REMAP references are equal; shape is not order-sensitive')
    expect(b'"../ord.lua"' in read(build,ref) and b'"planar/ord.lua"' in read(build,rev),'S4: reference orders do not embed the expected names')
   if tag=='S6':
    expect(b'planar/commands.lua' in read(build,t) and b'planar/other.lua' in read(build,t) and b'gen/deep/path/s6' not in read(build,t),'S6: both REMAP pairs must be applied')
   if tag=='S7':
    expect(b'"../json.lua"' in read(build,t) and b'"../a.lua"' in read(build,t),'S7: both LITERAL pairs must be applied')
   before=read(build,t)
   os.utime(build/t,ns=(OLD_NS,OLD_NS))
   append_marker(src/s['mutate'],'--')
   configure(src,build)
   after=read(build,t)
   expect(after!=before,f'{tag}: module bytes did not change after the Lua input changed')
   expect(after==read(build,ref),f'{tag}: module differs from the legacy-idiom reference after an input change (REMAP then LITERAL order)')
   expect(mtime(build,t)>OLD_NS,f'{tag}: module not rewritten with a newer mtime after an input change')
 with scratch('r20-embed-') as tmp:
  writer=('get_property(r20_reg GLOBAL PROPERTY R20_REGISTERED)\nget_property(r20_deps DIRECTORY PROPERTY CMAKE_CONFIGURE_DEPENDS)\n'
   'file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/registered.txt" "${r20_reg}")\nfile(WRITE "${CMAKE_CURRENT_BINARY_DIR}/deps.txt" "${r20_deps}")\n')
  src,build=make_project(tmp,shape_text('S2',SHAPES['S2'])+writer);configure(src,build)
  reg=(build/'registered.txt').read_text().split(';');deps=(build/'deps.txt').read_text().split(';')
  expect(any(r.startswith('t_S2|INTERFACE|') and r.endswith('t_S2.cppm') for r in reg),f'embed helper did not register its module through planar_rewrite_module: {reg}')
  for part in ('src/stdlib/planar/json.lua','gen/deep/path/s2/commands.lua','src/stdlib/planar/bootstrap.lua'):
   expect(any(d.endswith(part) for d in deps),f'embedded part {part} is not in CMAKE_CONFIGURE_DEPENDS: {deps}')
 with scratch('r20-embed-') as tmp:
  src,build=make_project(tmp,s5_text());configure(src,build)
  a,b,c=read(build,'t_S5a.cppm'),read(build,'t_S5b.cppm'),read(build,'t_S5c.cppm')
  plain=read(build,'legacy_S5_RL.cppm')
  expect(a==b,'S5: a bare REMAP and a call without REMAP produce different bytes')
  expect(a==plain and b==plain,'S5: bare REMAP output differs from a plain call')
  expect(c==read(build,'legacy_S5c_RL.cppm'),'S5: bare REMAP followed by LITERAL differs from the LITERAL-only reference')
  expect(b'"../json.lua"' in c,'S5: LITERAL after a bare REMAP was not applied')
  # Stability is asserted only for the calls that never reach a REMAP/LITERAL pair: the LITERAL-bearing S5c is covered by
  # embed_unchanged_mtime's single-write rule (I19) and its bytes above.
  for n in ('t_S5a.cppm','t_S5b.cppm'):os.utime(build/n,ns=(OLD_NS,OLD_NS))
  configure(src,build)
  for n,data in (('t_S5a.cppm',a),('t_S5b.cppm',b)):
   expect(mtime(build,n)==OLD_NS and read(build,n)==data,f'S5: {n} not stable across reconfigure')

# ---- static call-site audit (D30', D17a) ------------------------------------------------------------------------------
PRUNE=['build*','.git','vendor','.claude','.codex','.worktrees','worktrees','bin','dist']
def reachable_files():
 out=[repo/'CMakeLists.txt']
 roots=[repo/'cmake',repo/'src']+sorted(p for p in (repo/'tests'/'native').iterdir() if p.is_dir() and fnmatch.fnmatch(p.name,'*-support'))
 for root in roots:
  for d,dirs,files in os.walk(root):
   dirs[:]=sorted(x for x in dirs if not any(fnmatch.fnmatch(x,pat) for pat in PRUNE))
   for f in sorted(files):
    if f=='CMakeLists.txt' or f.endswith('.cmake'):out.append(Path(d)/f)
 return out

def strip_comments(text):
 out=[]
 for line in text.split('\n'):
  quote=False;cut=len(line)
  for i,ch in enumerate(line):
   if ch=='"' and (i==0 or line[i-1]!='\\'):quote=not quote
   if ch=='#' and not quote:cut=i;break
  out.append(line[:cut])
 return '\n'.join(out)

def statements(text):
 """(line, tokens) for every planar_embed_lua( call; tokens keep quoted strings whole."""
 found=[]
 for m in re.finditer(r'(?<![\w(])planar_embed_lua\(',text):
  i=m.end();depth=1;quote=False
  while depth and i<len(text):
   ch=text[i]
   if ch=='"' and text[i-1]!='\\':quote=not quote
   elif not quote and ch=='(':depth+=1
   elif not quote and ch==')':depth-=1
   i+=1
  body=text[m.end():i-1]
  tokens=re.findall(r'"(?:\\.|[^"\\])*"|[^\s"]+',body)
  found.append((text.count('\n',0,m.start())+1,tokens))
 return found

# (relative file, target prefix) of the nine REMAP sites; spelling is the token pair following REMAP when D17a fixes it.
SITES=[('src/engine/runtime/CMakeLists.txt','planar_command_source_',None),('src/engine/runtime/CMakeLists.txt','planar_load_source_',None),
 ('src/engine/runtime/CMakeLists.txt','planar_load_c01_source','LITERAL'),('src/engine/runtime/CMakeLists.txt','planar_x05_source_','${x05_remap}'),
 ('src/cmd/planar/CMakeLists.txt','planar_cli_bundle_source_',None),('src/cmd/planar/CMakeLists.txt','planar_load_cli_probe_source',None),
 ('src/cmd/planar/CMakeLists.txt','planar_x07_bundle_source_',None),('src/cmd/planar/CMakeLists.txt','planar_authoring_source_','${authoring_remap}'),
 ('tests/native/v5-02-x08-support/x08.cmake','planar_x08_bundle_source_',None)]
OFFENDERS=[re.compile(r'file\(\s*CONFIGURE\s+OUTPUT\s+"\$\{(?:\w*source_module|\w*x08_module)\}"'),
 re.compile(r'file\(\s*READ\s+"\$\{(?:\w*source_module|\w*x08_module|output)\}"')]

def embed_callsite_audit():
 problems=[];remap_sites=[];literal_sites=[]
 for path in reachable_files():
  rel=path.relative_to(repo).as_posix();text=strip_comments(path.read_text())
  for pattern in OFFENDERS:
   for m in pattern.finditer(text):
    problems.append(f'{rel}:{text.count(chr(10),0,m.start())+1}: legacy read-back/rewrite of an embed output: {m.group(0)}')
  for line,tokens in statements(text):
   target=tokens[0] if tokens else ''
   if 'REMAP' in tokens:remap_sites.append((rel,target,line,tokens))
   if 'LITERAL' in tokens:literal_sites.append((rel,target,line))
 for rel,prefix,spelling in SITES:
  hit=[s for s in remap_sites if s[0]==rel and s[1].startswith(prefix)]
  if len(hit)!=1:
   problems.append(f'{rel}: planar_embed_lua({prefix}...) must carry the standalone token REMAP exactly once, found {len(hit)}')
  elif spelling and spelling!='LITERAL':
   toks=hit[0][3];i=toks.index('REMAP')
   if toks[i+1:i+2]!=[spelling]:problems.append(f'{rel}:{hit[0][2]}: {prefix}... must spell `REMAP {spelling}` (D17a), found {toks[i:i+2]}')
 expected={(r,p) for r,p,_ in SITES}
 for rel,target,line,_ in remap_sites:
  if not any(rel==r and target.startswith(p) for r,p in expected):problems.append(f'{rel}:{line}: unexpected REMAP call {target}')
 if len(remap_sites)!=9:problems.append(f'{len(remap_sites)} planar_embed_lua( statements contain REMAP, expected exactly 9')
 if [(r,t.split("_source")[0]) for r,t,_ in literal_sites]!=[('src/engine/runtime/CMakeLists.txt','planar_load_c01')]:
  problems.append(f'LITERAL must appear only in the D6-C01 call, found {[(r,t,l) for r,t,l in literal_sites]}')
 expect(not problems,'embed call-site audit failed:\n'+'\n'.join(problems))

dispatch({'embed_unchanged_mtime':embed_unchanged_mtime,'embed_content_change_rewrites':embed_content_change_rewrites,
 'embed_callsite_audit':embed_callsite_audit},sys.argv)
