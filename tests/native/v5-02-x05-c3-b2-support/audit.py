"""Check exact current shape lookup and private original-owner orchestration boundaries."""
from pathlib import Path
import json,sys
root=Path(__file__).resolve().parents[3]
s=(root/'src/engine/runtime/runtime.cpp').read_text()
start='command_vm::resolve_shape_definition(std::string_view id) {'
end='std::expected<projection_definition, registration_error> command_vm::resolve_projection_definition('
assert s.count(start)==1 and s.count(end)==1
body=s.split(start,1)[1].split(end,1)[0]
# Preserve every original B2-A invariant, on the actual method rather than later APIs.
for forbidden in ['lua_', 's.instructions =', 's.native_bytes =', 's.nodes =', 'services', 'function_reference', 'filesystem']:
 assert forbidden not in body,forbidden
assert 'hash_value(domain, input)' in body
assert 'command_text("external")' in body
assert 'source.bytes' in body and 'source.path' in body
assert '"ordinal"' not in body
assert 'if (_owner && _owner->command.profile)\n    return std::unexpected(external_protocol());' in s
owner=s.split('struct extension_invocation::implementation {',1)[1]
for forbidden in ['lua_', 'getenv', 'current_path', 'resolve_explicit', 'lexically_normal', 'profile_capture::open', '.invoke(', 'services']:
 assert forbidden not in owner,forbidden
for required in ['profile_capture', 'command_vm', 'capture.snapshot()', 'command_vm::create(*snapshot)',
                 'prepare_extension_invocation(name, options)', 'validate_workspace_directory',
                 'validate_host_path_inputs', 'prepare_host_path(argument, explicit_cwd)',
                 'path->spelling', 'admission_file_role::declaration', 'admission_file_role::registry',
                 'admission_file_role::observation', 'admission_file_role::ledger',
                 'std::move(_owner->vm._owner->command.shape_failure)']:
 assert required in owner,required
assert owner.count('_owner->capture.verify()')==2
assert owner.index('_owner->paths.push_back')<owner.index('_owner->capture.admit_root')
assert owner.index('_owner->capture.admit_file')<owner.index('_owner->capture.verify()')
api=(root/'src/engine/runtime/runtime.cppm').read_text().split('class extension_invocation {',1)[1].split('private:',1)[0]
for forbidden in ['command_vm&', 'profile_capture&', 'extension_prepared_invocation', 'invoke(', 'command_services']:
 assert forbidden not in api,forbidden
helpers=(root/'src/engine/capture/host_path.cpp').read_text()
preflight=helpers.split('std::expected<void, diagnostic> validate_host_path_inputs(',1)[1].split('std::expected<prepared_host_path, diagnostic> prepare_host_path(',1)[0]
assert 'std::filesystem' not in preflight and 'lexically_normal' not in preflight
prepared=helpers.split('std::expected<prepared_host_path, diagnostic> prepare_host_path(',1)[1].split('std::expected<std::string, diagnostic> snapshot_workspace_directory(',1)[0]
assert prepared.index('cwd.lexically_normal()')<prepared.index('intent_path_max - explicit_cwd->size()')
result=dict(exact_shape_lookup_boundary=True,all_original_b2a_invariants=True,
            original_capture_vm_pair=True,no_ambient_reacquisition=True,no_handler_provider_lua_dispatch=True,
            normalized_all_before_metadata=True,original_receipts_verified=True,
            private_prepared_state=True,published_first_failure_preserved=True,
            pure_preflight_and_old_normalization_join_order=True)
Path(sys.argv[1]).write_text(json.dumps(result,sort_keys=True,indent=2)+'\n')
print('current shape/owner/preflight source audit passed')
