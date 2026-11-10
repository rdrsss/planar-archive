"""X07 generated CLI variants: test-only embedded bundles; reverse-identical substitutions, no runtime injection API."""
from pathlib import Path
import hashlib,json,re,sys
root,out=map(Path,sys.argv[1:]);out.mkdir(parents=True,exist_ok=True)
commands=(root/'src/stdlib/planar/commands.lua').read_text()
def once(source,old,new):
 # Every replaced handler anchor must occur exactly once in the shipped bundle.
 assert source.count(old)==1,old
 return source.replace(old,new)
init_old='''    local initialized=context.initialize_ledger({workspace_id=invocation.options["workspace-id"]})
    if not initialized.ok then return initialized end
    local facts=initialized.value
    return result({version=1,kind="init",ledger=facts.ledger,workspace=facts.workspace,active_revision=facts.active_revision,event=facts.event})'''
init_new='''    return failed({code="schema",message="X07 substituted init callable",field=""})'''
history_old='''    return context.read_history({selector=selector,id=id,request_id=key,limit=count})'''
history_new='''    return failed({code="schema",message="X07 substituted history callable",field=""})'''
export_old='''    local exported=context.export_source({revision=invocation.options.revision})
    if not exported.ok then return exported end
    local facts=exported.value
    return result({version=1,kind="export",ledger=facts.ledger,revision=facts.revision,source=facts.source})'''
export_new='''    return failed({code="schema",message="X07 substituted export callable",field=""})'''
handler_changes=[(init_old,init_new),(history_old,history_new),(export_old,export_new)]
variants=[once(commands,old,new) for old,new in handler_changes]
# Variants 3 (env-driven pauses and a post-commit allocation fault) and 4 (tiny response ceiling) keep the shipped bundle.
variants+=[commands,commands]
# DC56 anchors: each marker is the exact line `// X07-ANCHOR:<name>` and must occur exactly once in its source file.
ANCHORS={'invocation.cpp':['after-early-miss','after-factory-open','after-prepare-load','after-commit','response-ceiling'],
 'handlers/extension/command.cpp':['before-result-seal','after-final-verify','response-ceiling']}
def pause(name):
 n=name.upper().replace('-','_')
 return ('{ if (const char* reached = std::getenv("X07_REACHED_%s")) { std::ofstream(reached) << "r"; } '
  'if (const char* gate = std::getenv("X07_PAUSE_%s")) { while (!std::filesystem::exists(gate)) '
  'std::this_thread::sleep_for(std::chrono::milliseconds(5)); } }')%(n,n)
def anchor_changes(variant,name,original):
 # Return exact (old,new) pairs so the reverse-identical audit still restores the shipped source.
 out=[]
 for anchor in ANCHORS.get(name,[]):
  marker='// X07-ANCHOR:'+anchor
  assert original.count(marker)==1,(name,anchor)
  if anchor=='response-ceiling':
   if variant!=4:continue
   found=re.findall(r'// X07-ANCHOR:response-ceiling\n\s*(?:\[\[maybe_unused\]\] )?constexpr std::size_t response_ceiling = \d+;',original)
   assert len(found)==1,(name,anchor)
   out.append((found[0],re.sub(r'= \d+;','= 64;',found[0])))
  elif variant==3:
   body=pause(anchor)
   if anchor=='after-commit':body+=' if (std::getenv("X07_FAULT_AFTER_COMMIT")) { throw std::bad_alloc(); }'
   out.append((marker,body))
 return out
families=['render', 'init', 'history', 'export', 'load', 'extension', 'activate', 'status', 'preview', 'claim', 'takeover', 'refresh', 'checkpoint', 'release', 'recover', 'check', 'check-recover', 'verdict']
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
 write(d/'commands.lua',bundle,commands,[handler_changes[i]] if i<len(handler_changes) else [])
 for name in ['runtime.cpp','runtime.cppm']:
  original=(root/'src/engine/runtime'/name).read_text()
  changes=[('planar.engine.runtime;',f'planar.engine.x07_bundle_{i};'),('planar.engine.runtime_source;',f'planar.engine.x07_bundle_source_{i};'),('namespace planar::engine {',f'namespace planar::engine::x07_bundle_{i} {{')]
  data=original
  for a,b in changes:data=data.replace(a,b)
  write(d/name,data,original,changes)
 paths=['bootstrap.cpp','bootstrap.cppm','invocation.cpp','invocation.cppm']+[f'handlers/{f}/command.{ext}' for f in families for ext in ['cpp','cppm']]
 for name in paths:
  original=(root/'src/cmd/planar'/name).read_text()
  changes=[('import planar.engine.runtime; // X07-REAL-RUNTIME','import planar.engine.x07_real_runtime; // X07-REAL-RUNTIME'),('planar.cmd.bootstrap',f'planar.cmd.x07_{i}.bootstrap'),('planar.cmd.invocation',f'planar.cmd.x07_{i}.invocation'),('planar.cmd.planar.handlers.',f'planar.cmd.x07_{i}.handlers.'),('planar.cmd.execution.',f'planar.cmd.x07_{i}.execution.'),('::planar::cmd::handlers::',f'::planar::cmd::x07_{i}::handlers::'),('namespace planar::cmd {',f'namespace planar::cmd::x07_{i} {{'),('namespace planar::cmd::handlers {',f'namespace planar::cmd::x07_{i}::handlers {{'),('planar.engine.runtime;',f'planar.engine.x07_bundle_{i};'),('engine::command_',f'engine::x07_bundle_{i}::command_'),('engine::profile_invocation',f'engine::x07_bundle_{i}::profile_invocation'),('engine::registration_error',f'engine::x07_bundle_{i}::registration_error'),('engine::extension_',f'engine::x07_bundle_{i}::extension_'),('engine::prepared_profile',f'engine::x07_bundle_{i}::prepared_profile'),('import planar.engine.x07_real_runtime; // X07-REAL-RUNTIME','import planar.engine.runtime; // X07-REAL-RUNTIME')]
  if name=='invocation.cpp':
   # Bind finite service vocabulary to this exact runtime type owner.
   changes.append(('using namespace engine;',f'using namespace engine;\nusing engine::x07_bundle_{i}::command_services;\nusing engine::x07_bundle_{i}::command_service_result;\nusing engine::x07_bundle_{i}::command_render_options;\nusing engine::x07_bundle_{i}::command_init_options;\nusing engine::x07_bundle_{i}::command_history_options;\nusing engine::x07_bundle_{i}::command_history_selector;\nusing engine::x07_bundle_{i}::command_export_options;\nusing engine::x07_bundle_{i}::command_load_options;\nusing engine::x07_bundle_{i}::command_activate_options;\nusing engine::x07_bundle_{i}::command_status_options;\nusing engine::x07_bundle_{i}::command_preview_options;\nusing engine::x07_bundle_{i}::command_run_options;\nusing engine::x07_bundle_{i}::command_recovery_options;\nusing engine::x07_bundle_{i}::command_recover_options;\nusing engine::x07_bundle_{i}::command_check_options;'))
  changes=anchor_changes(i,name,original)+changes
  data=original
  for a,b in changes:data=data.replace(a,b)
  dest=d/name;dest.parent.mkdir(parents=True,exist_ok=True);write(dest,data,original,changes)
 original=(root/'src/cmd/planar/main.cpp').read_text()
 changes=[('planar.cmd.bootstrap',f'planar.cmd.x07_{i}.bootstrap'),('planar::cmd::invoke',f'planar::cmd::x07_{i}::invoke')]
 data=original
 for a,b in changes:data=data.replace(a,b)
 write(d/'main.cpp',data,original,changes)
(out/'generation.json').write_text(json.dumps(manifest,indent=2)+'\n')
print(f'audited {len(variants)} X07 substitution variants and {len(manifest)} generated inputs')
