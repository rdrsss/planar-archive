#!/usr/bin/env python3
"""Private load-handler VM; native changes are reversible identities and fixed admission only."""
from pathlib import Path
import sys
root,out=map(Path,sys.argv[1:]);out.mkdir(parents=True,exist_ok=True)
impl=(root/'src/engine/runtime/runtime.cpp').read_text();interface=(root/'src/engine/runtime/runtime.cppm').read_text()
commands=(root/'src/stdlib/planar/commands.lua').read_text()
handler=(root/'tests/native/r14-d6-support/load.lua').read_text()
registration='''  registration.add(descriptor("load","accept_load_v1",{
    option("ledger","text",true),option("workspace","text",true),option("registry","text",true),
    option("expect-revision","text",true),option("request-id","text",true),option("actor","text",true)
  }),load_command)
'''
start=commands.index('  local function load_command')
end=commands.index('  local function activate_command',start)
assert commands[start:end].count('local function ') == 1, 'load replacement must preserve every other handler'
commands=commands[:start]+handler+commands[end:]
for i in range(6):
 lua=commands
 if i==2: lua=commands.replace(registration,'')
 if i==3: lua=commands.replace('"load","accept_load_v1"','"load","export_source_v1"')
 if i==4: lua=commands.rsplit('end',1)[0]+registration+'end\n'
 if i==5: lua=commands.replace('option("actor","text",true)','option("actor","text",false)')
 if i==1:
  # Change actual retained interpretation policy and original bytes together.
  lua=lua.replace('local normalized=interpreted.value','local normalized=interpreted.value\n    normalized.bundle.spec.goal.bytes="D6 altered retained policy"')
 (out/f'commands_{i}.lua').write_text(lua)
 a=impl.replace('module planar.engine.runtime;',f'module planar.engine.load_test_{i};').replace('import planar.engine.runtime_source;',f'import planar.engine.load_source_{i};').replace('namespace planar::engine {',f'namespace planar::engine::load_test_{i} {{')
 b=interface.replace('module planar.engine.runtime;',f'module planar.engine.load_test_{i};').replace('namespace planar::engine {',f'namespace planar::engine::load_test_{i} {{')
 (out/f'runtime_{i}.cpp').write_text(a);(out/f'runtime_{i}.cppm').write_text(b)
 restored=a.replace(f'module planar.engine.load_test_{i};','module planar.engine.runtime;').replace(f'import planar.engine.load_source_{i};','import planar.engine.runtime_source;').replace(f'namespace planar::engine::load_test_{i} {{','namespace planar::engine {')
 assert restored==impl
 assert b.replace(f'module planar.engine.load_test_{i};','module planar.engine.runtime;').replace(f'namespace planar::engine::load_test_{i} {{','namespace planar::engine {')==interface
print('audited six private exact load-handler runtime/interface variants')
