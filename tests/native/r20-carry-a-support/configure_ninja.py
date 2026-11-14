# R20-CARRY-A: a scratch CMake+Ninja project that runs all four build-input generators at configure time (I12, I13).
"""argv: case, repository root, cmake path, ninja path. Both tools come from argv, never PATH.

Necessary, not sufficient: it shows that regenerating unchanged build inputs does not make Ninja do work and that a changed
input does. The real embed write path is covered by embed_stability.py and the real tree by the round's I17/I25 evidence.
"""
from pathlib import Path
import sys
sys.dont_write_bytecode=True
from common import expect,scratch,run,run_ok,copy_inputs,append_marker,snapshot,dispatch
repo=Path(sys.argv[2]).resolve();cmake=sys.argv[3];ninja=sys.argv[4]
T='tests/native/'
RT=['src/engine/runtime/runtime.cpp','src/engine/runtime/runtime.cppm','src/stdlib/planar/commands.lua']
INPUTS=RT+[T+'v5-02-b1-support/altered.lua',T+'r14-d6-support/load.lua',T+'r14-d6-support/generate.py']
GENS=[('b1',T+'v5-02-b1-support/generate.py'),('d6',T+'r14-d6-support/generate.py'),('d6c01',T+'r14-d6-c01-support/generate.py'),
 ('x05a0',T+'v5-02-x05-a0-support/generate.py')]
NO_WORK='ninja: no work to do.'

def project(tmp):
 root=tmp/'root';copy_inputs(repo,root,INPUTS)
 src=tmp/'proj';src.mkdir()
 lines=['cmake_minimum_required(VERSION 3.20)','project(r20scratch NONE)','set(all_generated)']
 for key,script in GENS:
  out=tmp/'gen'/key
  lines+=[f'execute_process(COMMAND "{sys.executable}" "{repo/script}" "{root}" "{out}" COMMAND_ERROR_IS_FATAL ANY OUTPUT_QUIET)',
   f'file(GLOB_RECURSE {key}_files "{out}/*")','list(APPEND all_generated ${'+key+'_files})']
 stamp=tmp/'build'/'stamp'
 lines+=[f'add_custom_command(OUTPUT "{stamp}" COMMAND "${{CMAKE_COMMAND}}" -E touch "{stamp}" DEPENDS ${{all_generated}})',
  f'add_custom_target(consume ALL DEPENDS "{stamp}")']
 (src/'CMakeLists.txt').write_text('\n'.join(lines)+'\n')
 return root,src,tmp/'build'

def configure(src,build):
 return run_ok([cmake,'-S',src,'-B',build,'-G','Ninja',f'-DCMAKE_MAKE_PROGRAM={ninja}'],'cmake configure')

def build(build_dir):
 r=run_ok([ninja],'ninja',cwd=build_dir)
 return r.stdout.strip()

def generated_count(build_dir):
 return len(list((build_dir.parent/'gen').rglob('*.*')))

def configure_zero_work():
 with scratch('r20-ninja-') as tmp:
  root,src,b=project(tmp)
  configure(src,b)
  first=build(b)
  expect(first!=NO_WORK and '[' in first,f'first ninja run did no work: {first!r}')
  expect(generated_count(b)>=90,f'scratch generated only {generated_count(b)} files, expected 63+18+3+7')
  configure(src,b)
  again=build(b)
  expect(again==NO_WORK,f'reconfigure over the unchanged tree made ninja do work: {again[-600:]!r}')

def configure_rebuilds_on_content_change():
 with scratch('r20-ninja-') as tmp:
  root,src,b=project(tmp)
  configure(src,b);build(b)
  d6=tmp/'gen'/'d6'
  d6_before={n:b_ for n,(b_,_) in snapshot(d6).items()}
  append_marker(root/'src/stdlib/planar/commands.lua','--')
  append_marker(root/'src/engine/runtime/runtime.cpp','//')
  configure(src,b)
  r=run([ninja],timeout=240,cwd=b)
  expect(r.returncode==0,f'ninja after a content change exited {r.returncode}: {r.stderr[-400:]}')
  expect(r.stdout.strip()!=NO_WORK,'a changed generator input did not make ninja do work')
  expect(build(b)==NO_WORK,'ninja did work a second time after the content change was consumed')
  # C13: the D6 generator alone must rewrite its outputs (write-if-different); a write-if-missing D6 keeps stale bytes.
  d6_after={n:b_ for n,(b_,_) in snapshot(d6).items()}
  expect(d6_after!=d6_before,'D6 generator output bytes unchanged after the content change (write-if-missing?)')

dispatch({'configure_zero_work':configure_zero_work,'configure_rebuilds_on_content_change':configure_rebuilds_on_content_change},sys.argv)
