"""Exact shape lookup invariants plus bounded new native helper source boundaries."""
from pathlib import Path
import json,sys
R=Path(__file__).resolve().parents[3]
s=(R/'src/engine/runtime/runtime.cpp').read_text()
a='command_vm::resolve_shape_definition(std::string_view id) {';b='std::expected<projection_definition, registration_error> command_vm::resolve_projection_definition('
assert s.count(a)==1 and s.count(b)==1
body=s.split(a,1)[1].split(b,1)[0]
for forbidden in ['lua_', 's.instructions =', 's.native_bytes =', 's.nodes =', 'services', 'function_reference', 'filesystem']:assert forbidden not in body,forbidden
for required in ['hash_value(domain, input)','command_text("external")','source.bytes','source.path']:assert required in body,required
assert '"ordinal"' not in body
assert 'if (_owner && _owner->command.profile)\n    return std::unexpected(external_protocol());' in s
helper=s.split('extension_invocation::prepare_history() {',1)[1].split('std::expected<value_ptr, extension_execution_error> extension_invocation::execute()',1)[0]
for forbidden in ['lua_', 'profile_capture::open', 'command_services', 'current_path', 'getenv', '.invoke(']:assert forbidden not in helper,forbidden
for required in ['_owner->prepared->history_query','_owner->paths[i]','ledger::open_existing_gated','history::read_history','file->gate_status()','_owner->capture.verify()','_owner->authenticated_history']:assert required in helper,required
assert '_owner->history_failure.emplace(std::in_place_type<registration_error>, std::move(*_owner->first_failure))' in helper
adapter=s.split('class original_history_guard final',1)[1].split('extension_invocation::prepare_history()',1)[0]
for required in ['_capture.verify_file("extension.ledger", descriptor)','_capture.verify_file("extension.ledger", _descriptor)','F_DUPFD_CLOEXEC','_capture.verify()']:assert required in adapter,required
for forbidden in ['lua_', 'profile_capture::open', '::open(', '::openat(']:assert forbidden not in adapter,forbidden
f=(R/'src/engine/ledger/format.cpp').read_text();opener=f.split('format_file::open_impl(const std::filesystem::path& path',1)[1].split('std::expected<stored_record, format_error> format_file::read_record',1)[0]
assert opener.index('guard->bind(held.fd)')<opener.index('header(held.fd)')<opener.index('connection::open_gated')
assert 'db->gate_status() != read_gate_status::ready || verify_gate(guard) != read_gate_status::ready' in opener
assert 'import planar.engine.capture' not in f and 'import planar.engine.runtime' not in f
execute=s.split('extension_invocation::execute() {',1)[1].split('extension_invocation::verify()',1)[0]
import re
execute=re.sub(r'\s+', ' ', execute)
for required in ['capabilities.size() == 1','capabilities[0]->string == "history_read_v1"','_owner->history_attempted','owner.command.extension_history = {}','original.prepare_history()','codec_encode_reserved','verified = verify()']:
 assert required in execute,required
assert execute.index('original.prepare_history()')<execute.index('lua_pushcfunction(l, execute_shape)')
service=s.split('int extension_read_history(lua_State* l) {',1)[1].split('int extension_context_index',1)[0]
assert service.index('bridge.read(*bridge.owner)')<service.index('push(l, bridge.result)')
for forbidden in ['std::string ', 'value_ptr ', 'command_services', 'lua_pcall']:
 assert forbidden not in service,forbidden
assert 'extension_history_matches(l, -1, bridge.result)' in s
assert 'root_metadata_bytes + sizeof(extension_history_bridge)' in s
shared=s.split('void refuse_missing_extension_service(lua_State* l) {',1)[1].split('int frozen_rawget',1)[0]
for required in ['context_identity','lua_topointer(l, 1)','lua_isnil(l, -1)','"read_history"','"read_role"','"project"']:assert required in shared,required
assert s.count('refuse_missing_extension_service(l);')==2
context_setup=s.split('int execute_shape(lua_State* l)',1)[1].split('void release_shape_temporaries',1)[0]
assert 'lua_getfield(l, -1, "__index")' not in context_setup
assert 'context_identity = lua_topointer(l, -1)' in context_setup
assert 'lua_rawgeti(l, LUA_REGISTRYINDEX, backing_index);' in context_setup
Path(sys.argv[1]).write_text(json.dumps(dict(exact_shape_lookup_boundary=True,all_original_b2a_invariants=True,native_helper_no_lua_or_recapture=True,original_private_selector_path=True,same_original_receipt_descriptor_gate=True,bind_before_header_before_sqlite=True,downward_format_boundary=True,owned_error_before_copy=True),indent=2)+'\n')
print('exact shape and native bound-history source audit passed')

frame=s.split('struct projection_call_frame {',1)[1].split('void begin_invocation',1)[0]
for forbidden in ['begin_invocation(', 'release_shape_temporaries(', 'instructions = 0', 'nodes = 0', 'native_bytes = 0', 'extension_history =']:assert forbidden not in frame,forbidden
for required in ['sizeof(projection_call_frame) + sizeof(projection_scan)', 'lua_settop(lua, top)', 'runtime.input               = std::move(input)', 'command.extension_execution = extension', 'lua_pushvalue(l, base + 1)', 'convert(l, base + 1, 1)', 'codec_projection_count(', 's.lua_memory_failed = true']:assert required in frame,required
ordinary=s.split('command_vm::invoke_projection(',1)[1].split('command_vm::validate_profile(',1)[0]
assert 'call_retained_projection(l, selected->function_reference, input)' in ordinary
assert 'scan_projection' not in ordinary and 'codec_projection_count' not in ordinary
assert 'project_probe' not in s
print('nested protected projection shared-helper source audit passed')

resolver=s.split('command_vm::resolve_projection_definition_impl(std::string_view id',1)[1].split('std::expected<extension_prepared_invocation',1)[0]
for required in ['(_owner->command.invoked && !active)', 'bridge.owner != active', '!bridge.context_identity', '!bridge.projection_attempted', 'bridge.projection_result', '!_owner->command.extension_execution']:
 assert required in resolver,required
for required in ['"assemble_projection_material"','sizeof(std::array<int, 8>)','sizeof(extension_history_bridge)','c.shape_phases[6]', 'command_integer(1048576)', 'No current-policy reinterpretation.', 'owner.assembly_failure.emplace', 'owner.authenticated_history', 'owner.authenticated_projection']:
 assert required in s,required
assembly=s.split('std::expected<value_ptr, diagnostic> call_retained_assembly(',1)[1].split('// Compare raw/proxy',1)[0]
for forbidden in ['begin_invocation(', 'instructions =', 'nodes =', 'native_bytes =', 'release_shape_temporaries']:
 assert forbidden not in assembly,forbidden
for required in ['projection_call_frame restore', 'scan_assembly_response', 'convert(l, base + 1, 1)', 'codec_encode_reserved']:
 assert required in assembly,required
print('active-owner service, trusted assembly and retained DATA source audit passed')

selected=s.split('// Stage owning fallbacks before entering the selected projection phase.',1)[1].split('// Reserve graph/map/text construction',1)[0]
assert selected.index('charge(s,')<selected.index('registration_error projection_failure')<selected.index('try {')
for catch in selected.split('} catch')[1:]:
 for forbidden in ['std::format(', 'external_resource(', 'external_protocol(']:assert forbidden not in catch,forbidden
assert '&projection_failure' in selected and 'std::move(projection_failure.field)' in selected
resolver=s.split('command_vm::resolve_projection_definition_impl(std::string_view id',1)[1].split('std::expected<extension_prepared_invocation',1)[0]
assert 'std::move(active_failure->field)' in resolver and 'shape_failure.emplace(std::move(*active_failure))' in resolver
print('staged selected-projection error ownership and allocation-safe catches passed')
