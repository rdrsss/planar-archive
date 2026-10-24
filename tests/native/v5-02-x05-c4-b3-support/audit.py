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
helper=s.split('extension_invocation::prepare_history() {',1)[1].split('std::expected<value_ptr, native_history_error> extension_invocation::execute()',1)[0]
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
Path(sys.argv[1]).write_text(json.dumps(dict(exact_shape_lookup_boundary=True,all_original_b2a_invariants=True,native_helper_no_lua_or_recapture=True,original_private_selector_path=True,same_original_receipt_descriptor_gate=True,bind_before_header_before_sqlite=True,downward_format_boundary=True,owned_error_before_copy=True),indent=2)+'\n')
print('exact shape and native bound-history source audit passed')
