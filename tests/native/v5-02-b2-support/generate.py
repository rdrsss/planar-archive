"""Exact-source real CLI compositions with test-only embedded bundles; no runtime injection API."""
from pathlib import Path
import hashlib,json,sys
root,out=map(Path,sys.argv[1:]);out.mkdir(parents=True,exist_ok=True)
commands=(root/'src/stdlib/planar/commands.lua').read_text()
variants=[
 commands.replace('    local id=invocation.options["workspace-id"]','    if invocation.options["workspace-id"]=="projection" then return failed({code="schema",message="B2 retained callable executed",field=""}) end\n    local id=invocation.options["workspace-id"]').replace('    if not exported.ok then return exported end','    if not exported.ok then return result({version=1,kind="export"}) end\n    error("B2 postpublication Lua failure")'),
 commands.replace('name=name,family=name','name=name,family="history"'),
 commands.replace('if registration then','if registration then\n  pcall(function() while true do end end)'),
 commands.replace('    if not exported.ok then return exported end','    if not exported.ok then return exported end\n    do return result({version=1,kind="export",forged=true}) end'),
 commands.replace('    if not exported.ok then return exported end','    if not exported.ok then return exported end\n    pcall(function() while true do end end)'),
 commands,
]
manifest={}
def write(path,data,original=None,substitutions=()):
 if not path.exists() or path.read_text()!=data:path.write_text(data)
 row={'sha256':hashlib.sha256(data.encode()).hexdigest()}
 if original is not None:
  restored=data
  for before,after in reversed(substitutions):restored=restored.replace(after,before)
  assert restored==original,path
  row.update(source_sha256=hashlib.sha256(original.encode()).hexdigest(),reverse_identical=True,substitutions=substitutions)
 manifest[str(path.relative_to(out))]=row
for i,bundle in enumerate(variants):
 d=out/str(i);d.mkdir(exist_ok=True)
 write(d/'commands.lua',bundle)
 for name in ['runtime.cpp','runtime.cppm']:
  original=(root/'src/engine/runtime'/name).read_text()
  changes=[('planar.engine.runtime;',f'planar.engine.cli_bundle_{i};'),('planar.engine.runtime_source;',f'planar.engine.cli_bundle_source_{i};'),('namespace planar::engine {',f'namespace planar::engine::cli_bundle_{i} {{')]
  data=original
  for a,b in changes:data=data.replace(a,b)
  write(d/name,data,original,changes)
 paths=['bootstrap.cpp','bootstrap.cppm','invocation.cpp','invocation.cppm']+[f'handlers/{f}/command.{ext}' for f in ['render','init','history','export','load','extension'] for ext in ['cpp','cppm']]
 for name in paths:
  original=(root/'src/cmd/planar'/name).read_text()
  changes=[('import planar.engine.runtime; // X07-REAL-RUNTIME','import planar.engine.x07_real_runtime; // X07-REAL-RUNTIME'),('planar.cmd.bootstrap',f'planar.cmd.cli_{i}.bootstrap'),('planar.cmd.invocation',f'planar.cmd.cli_{i}.invocation'),('planar.cmd.planar.handlers.',f'planar.cmd.cli_{i}.handlers.'),('namespace planar::cmd {',f'namespace planar::cmd::cli_{i} {{'),('namespace planar::cmd::handlers {',f'namespace planar::cmd::cli_{i}::handlers {{'),('planar.engine.runtime;',f'planar.engine.cli_bundle_{i};'),('engine::command_',f'engine::cli_bundle_{i}::command_'),('engine::profile_invocation',f'engine::cli_bundle_{i}::profile_invocation'),('engine::registration_error',f'engine::cli_bundle_{i}::registration_error'),('engine::extension_',f'engine::cli_bundle_{i}::extension_'),('engine::prepared_profile',f'engine::cli_bundle_{i}::prepared_profile'),('import planar.engine.x07_real_runtime; // X07-REAL-RUNTIME','import planar.engine.runtime; // X07-REAL-RUNTIME')]
  if name=='invocation.cpp':
   # Bind finite service vocabulary to this exact runtime type owner.
   changes.append(('using namespace engine;',f'using namespace engine;\nusing engine::cli_bundle_{i}::command_services;\nusing engine::cli_bundle_{i}::command_service_result;\nusing engine::cli_bundle_{i}::command_render_options;\nusing engine::cli_bundle_{i}::command_init_options;\nusing engine::cli_bundle_{i}::command_history_options;\nusing engine::cli_bundle_{i}::command_history_selector;\nusing engine::cli_bundle_{i}::command_export_options;\nusing engine::cli_bundle_{i}::command_load_options;'))
  if i==5 and name=='handlers/init/command.cpp':
   changes.append(('->required()', '->required(false)'))
  data=original
  for a,b in changes:data=data.replace(a,b)
  dest=d/name;dest.parent.mkdir(parents=True,exist_ok=True);write(dest,data,original,changes)
 # real process entry point; identical I/O implementation.
 original=(root/'src/cmd/planar/main.cpp').read_text()
 changes=[('planar.cmd.bootstrap',f'planar.cmd.cli_{i}.bootstrap'),('planar::cmd::invoke',f'planar::cmd::cli_{i}::invoke')]
 data=original
 for a,b in changes:data=data.replace(a,b)
 write(d/'main.cpp',data,original,changes)
(out/'generation.json').write_text(json.dumps(manifest,indent=2)+'\n')
print(f'audited {len(variants)} real CLI compositions and {len(manifest)} generated inputs')
