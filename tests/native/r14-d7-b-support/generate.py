"""Audited exact-source load CLI probe; no instrumentation enters production."""
from pathlib import Path
import hashlib,json,sys
root,out=map(Path,sys.argv[1:]);out.mkdir(parents=True,exist_ok=True)
manifest={}
def write(name,original,changes):
 data=original
 for before,after in changes:
  assert data.count(before)==1,(name,before,data.count(before))
  data=data.replace(before,after)
 restored=data
 for before,after in reversed(changes):restored=restored.replace(after,before)
 assert restored==original,name
 path=out/name;path.parent.mkdir(parents=True,exist_ok=True)
 if not path.exists() or path.read_text()!=data:path.write_text(data)
 manifest[name]={'source_sha256':hashlib.sha256(original.encode()).hexdigest(),'sha256':hashlib.sha256(data.encode()).hexdigest(),'reverse_identical':True,'substitutions':changes}
commands=(root/'src/stdlib/planar/commands.lua').read_text()
write('commands.lua',commands,[('    return context.accept_load({bundle=normalized.bundle,semantic=normalized.semantic})','''    local accepted=context.accept_load({bundle=normalized.bundle,semantic=normalized.semantic})
    local mode=invocation.options["request-id"]
    if mode=="posterror" then error("after completion") end
    if mode=="postmemory" then pcall(function() local s="x";while true do s=s..s end end) end
    if mode=="postinstructions" then pcall(function() while true do end end) end
    if mode=="postchanged" then accepted.value.event="99" end
    if mode=="postfalse" then return failed({code="schema",message="after completion",field=""}) end
    if mode=="postmalformed" then return nil end
    return accepted''')])
for name in ['runtime.cpp','runtime.cppm']:
 original=(root/'src/engine/runtime'/name).read_text()
 changes=[('planar.engine.runtime;','planar.engine.load_cli_probe;'),('namespace planar::engine {','namespace planar::engine::load_cli_probe {')]
 if name=='runtime.cpp':changes.append(('planar.engine.runtime_source;','planar.engine.load_cli_probe_source;'))
 if name=='runtime.cpp':changes.append(('      auto encoded = codec_encode(copied);', '      auto encoded = codec_encode(copied, {std::get<command_load_options>(c.options).request_id == \"postencode\" ? 1u : 16777216u, 256, 4000000});'))
 write(name,original,changes)
paths=['bootstrap.cpp','bootstrap.cppm','invocation.cpp','invocation.cppm']+[f'handlers/{f}/command.{ext}' for f in ['render','init','history','export','load','extension'] for ext in ['cpp','cppm']]
for name in paths:
 original=(root/'src/cmd/planar'/name).read_text()
 # Module names may occur many times; audit each exact transformation as one replacement block.
 changes=[]
 replacements=[('import planar.engine.runtime; // X07-REAL-RUNTIME','import planar.engine.x07_real_runtime; // X07-REAL-RUNTIME'),('planar.cmd.bootstrap','planar.cmd.load_cli_probe.bootstrap'),('planar.cmd.invocation','planar.cmd.load_cli_probe.invocation'),('planar.cmd.planar.handlers.','planar.cmd.load_cli_probe.handlers.'),('namespace planar::cmd {','namespace planar::cmd::load_cli_probe {'),('namespace planar::cmd::handlers {','namespace planar::cmd::load_cli_probe::handlers {'),('planar.engine.runtime;','planar.engine.load_cli_probe;'),('engine::command_','engine::load_cli_probe::command_'),('engine::profile_invocation','engine::load_cli_probe::profile_invocation'),('engine::registration_error','engine::load_cli_probe::registration_error'),('engine::extension_','engine::load_cli_probe::extension_'),('engine::prepared_profile','engine::load_cli_probe::prepared_profile'),('import planar.engine.x07_real_runtime; // X07-REAL-RUNTIME','import planar.engine.runtime; // X07-REAL-RUNTIME')]
 data=original
 for a,b in replacements:data=data.replace(a,b)
 restored=data
 for a,b in reversed(replacements):restored=restored.replace(b,a)
 assert restored==original,name
 changes=[(original,data)]
 if name=='invocation.cpp':
  changes.append(('using namespace engine;', '''using namespace engine;
using engine::load_cli_probe::command_services;
using engine::load_cli_probe::command_service_result;
using engine::load_cli_probe::command_render_options;
using engine::load_cli_probe::command_init_options;
using engine::load_cli_probe::command_history_options;
using engine::load_cli_probe::command_history_selector;
using engine::load_cli_probe::command_export_options;
using engine::load_cli_probe::command_load_options;
void probe_barrier(const std::string& ledger, std::string_view stage) {
  const auto marker=ledger+"."+std::string(stage);
  {std::ofstream file(marker);file<<"ready";}
  const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(20);
  while (!std::filesystem::exists(ledger+".resume")) {
    if (std::chrono::steady_clock::now()>deadline) throw std::runtime_error("probe timeout");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
}'''))
  changes.append(('    auto accepted = ledger::complete_acceptance', '''    if (options.request_id == "capture-race") probe_barrier(options.ledger,"prepared");
    auto accepted = ledger::complete_acceptance'''))
  changes.append(('    auto decoded = codec_decode(completion.saved_result);', '''    if (options.request_id == "postalloc") throw std::bad_alloc(); // synthetic exception, not allocator exhaustion
    auto decoded = codec_decode(completion.saved_result);'''))
  changes.append(('          early_completion.phase        = engine::ledger::completion_phase::replayed;', '''          early_completion.phase        = engine::ledger::completion_phase::replayed;
          if (load.request_id == "earlytransport") throw std::bad_alloc();'''))
  changes.append(('    auto result = vm->invoke(std::move(options), *services);', '''    const auto probe_id = loading ? std::get<engine::load_cli_probe::command_load_options>(options).request_id : "";
    if (probe_id == "late") probe_barrier(std::get<engine::load_cli_probe::command_load_options>(options).ledger,"miss");
    auto result = vm->invoke(std::move(options), *services);
    if (probe_id == "posttransport" && published()) throw std::bad_alloc();'''))
 write(name,original,changes)
original=(root/'src/cmd/planar/main.cpp').read_text();data=original.replace('planar.cmd.bootstrap','planar.cmd.load_cli_probe.bootstrap').replace('planar::cmd::invoke','planar::cmd::load_cli_probe::invoke');write('main.cpp',original,[(original,data)])
(out/'generation.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('Audited exact reversible load CLI probe sources')
