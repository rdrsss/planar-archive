#!/usr/bin/env python3
"""Test-only exact runtime/interface variants; source injection is never a product API."""
from pathlib import Path
import sys
root,out=map(Path,sys.argv[1:]);out.mkdir(parents=True,exist_ok=True)
impl=(root/'src/engine/runtime/runtime.cpp').read_text()
interface=(root/'src/engine/runtime/runtime.cppm').read_text()
commands=(root/'src/stdlib/planar/commands.lua').read_text()
start=commands.index('    local id=invocation.options["workspace-id"]')
end=commands.index('\n  end\n  local function history_command',start)
altered=commands[:start]+(root/'tests/native/v5-02-b1-support/altered.lua').read_text().rstrip()+commands[end:]
# C01 preserves rawget as a backing-aware read; keep the historical fixture immutable.
altered=altered.replace("rawset or rawget or getmetatable", "rawset or getmetatable")
# Change a lexical binding after registration: retained refs must still execute the original.
altered=altered.replace('\nend\n','\n  init_command=function() error("replaced lexical binding") end\nend\n')
altered=altered.replace('    local opened=context.open_workspace', '''    local mode=invocation.options.job
    if mode=="forged" then return context.open_workspace({workspace={}}) end
    if mode=="wrongslot" then return context.open_workspace({workspace=invocation.options.registry}) end
    if mode=="extra" then return context.open_workspace({workspace=invocation.options.workspace,extra=true}) end
    if mode=="order" then return context.capture_render({route={}}) end
    if mode=="reused" then local first=context.open_workspace({workspace=invocation.options.workspace});pcall(context.open_workspace,{workspace=invocation.options.workspace});return first end
    local opened=context.open_workspace''')
altered=altered.replace('    local exported=context.export_source({revision=invocation.options.revision})', '''    if invocation.options.revision=="13" then return context.export_source({revision="14"}) end
    local exported=context.export_source({revision=invocation.options.revision})
    if exported.ok and invocation.options.revision=="11" then return {ok=true,value={version=1,kind="export",forged=true}} end
    if exported.ok and invocation.options.revision=="12" then pcall(function() while true do end end) end''')
variants=[altered,
 commands.replace('name=name,family=name','name=name,family="history"'),
 commands.replace('registration.add(descriptor("export"','registration.add(descriptor("history"'),
 commands.replace('  registration.add(descriptor("init"','  if false then registration.add(descriptor("init"').replace('  }),init_command)','  }),init_command) end'),
 commands.replace('  }),init_command)','  }),"not a callable")'),
 commands.replace('type=kind,required=required','type=kind,required="true"'),
 commands.replace('}),init_command)','}),render)'),
 commands.replace('  }),init_command)','  }),init_command)\n  registration.add(descriptor("init","initialize_ledger_v1",{option("ledger","text",true),option("workspace-id","text",true)}),init_command)'),
 commands.replace('  local function option(name,kind,required)','  local function option(name,kind,required)').replace('help="Explicit "..name.." value"','help="Explicit "..name.." value",extra=true'),
 commands.replace('if registration then','if registration then\n  pcall(function() while true do end end)'),
 commands.replace('if registration then','if registration then\n  pcall(function() local bytes="x";while true do bytes=bytes..bytes end end)'),
 commands.replace('if registration then','if registration then\n  engine.hash_value("forbidden",{})'),
 commands.replace('options=engine.array(options)','options=engine.array({[2]=options[1]})'),
 commands.replace('type=kind,required=required','type=0.5,required=required'),
 commands.replace('options=engine.array(options)','options=engine.array({options[1],options[1]})'),
 commands.replace('help="Planar "..name','help="bad\\0"'),
 commands.replace('return {name=name,family=name','local cycle={};cycle.self=cycle;return {cycle=cycle,name=name,family=name'),
]
variants.append(altered.replace('help="Explicit "..name.." value"','help="'+'x'*4096+'"').replace('help="Planar "..name','help="'+'x'*4096+'"'))
# C01 bounded registration-owned closure graphs and read compatibility.
stateful=commands.replace('if registration then', """if registration then
  local key={}; local function fkey() end
  local captured={value='kept',nested={value='nested'},'one','two'}
  captured.self=captured; captured[key]='table-key'; captured[fkey]='function-key'
  local tagged=engine.array({'kept'})
  local failure={code='schema',message='retained immutable refusal',field=''}
  local hidden={value='helper-only'}
  local function helper_only() return hidden end
  local function transitive_only() return helper_only() end
  local alias=captured
  local saved_next,saved_rawget,saved_pairs,saved_ipairs=next,rawget,pairs,ipairs
  local function helper() return alias end
  local function transitive() return helper() end
""")
probe="""    local id=invocation.options["workspace-id"]
    local t=transitive()
    if id=='return' then return {ok=false,error=failure}\n    elseif id=='helper' then transitive_only().value='changed'
    elseif id=='direct' then t.value='changed'
    elseif id=='new' then t.new='changed'
    elseif id=='nested' then t.nested.value='changed'
    elseif id=='key' then key.changed=true
    elseif id=='reads' then
      if not (t==captured and t==t.self and #t==2 and t[1]=='one' and t[key]=='table-key' and t[fkey]=='function-key') then error('identity or length') end
      if saved_rawget(t,'value')~='kept' then error('rawget') end
      local n=0;for k,v in saved_pairs(t) do n=n+1 end;if n~=7 then error('pairs') end
      n=0;for k,v in saved_ipairs(t) do n=n+1 end;if n~=2 then error('ipairs') end
      n=0;local k=nil;repeat local v;k,v=saved_next(t,k);if k~=nil then n=n+1 end until k==nil;if n~=7 then error('next') end
      local encoded,err=engine.json_encode({list=tagged});if err or encoded~='{\"list\":[\"kept\"]}' then error('tagged codec') end
      local scratch={'scratch'};scratch.extra=true;scratch[1]='changed'
      if #scratch~=1 or saved_rawget(scratch,1)~='changed' or saved_next(scratch)==nil then error('scratch') end
    end
"""
stateful=stateful[:stateful.index('    local id=invocation.options["workspace-id"]')]+probe+stateful[stateful.index('    local valid=#id'):]
variants.append(stateful)
for edges in [254,255]:
 variants.append(commands.replace('if registration then',f'if registration then\n  __c01_graph={{}};local t=__c01_graph;for i=1,{edges} do t.child={{}};t=t.child end'))
for i,source in enumerate(variants):
 (out/f'commands_{i}.lua').write_text(source)
 (out/f'runtime_{i}.cpp').write_text(impl.replace('module planar.engine.runtime;','module planar.engine.command_test_'+str(i)+';').replace('import planar.engine.runtime_source;','import planar.engine.command_source_'+str(i)+';').replace('namespace planar::engine {','namespace planar::engine::command_test_'+str(i)+' {'))
 (out/f'runtime_{i}.cppm').write_text(interface.replace('module planar.engine.runtime;','module planar.engine.command_test_'+str(i)+';').replace('namespace planar::engine {','namespace planar::engine::command_test_'+str(i)+' {'))
# Audit exact reversibility of generated native files: only module/source/namespace identities differ.
for i in range(len(variants)):
 restored=(out/f'runtime_{i}.cpp').read_text().replace('module planar.engine.command_test_'+str(i)+';','module planar.engine.runtime;').replace('import planar.engine.command_source_'+str(i)+';','import planar.engine.runtime_source;').replace('namespace planar::engine::command_test_'+str(i)+' {','namespace planar::engine {')
 assert restored==impl
 restored=(out/f'runtime_{i}.cppm').read_text().replace('module planar.engine.command_test_'+str(i)+';','module planar.engine.runtime;').replace('namespace planar::engine::command_test_'+str(i)+' {','namespace planar::engine {')
 assert restored==interface
print(f'audited {len(variants)} exact runtime/interface variants')
