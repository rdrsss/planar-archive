"""Check the narrow native lookup boundary; runtime tests supply behavior evidence."""
from pathlib import Path
import json,sys
root=Path(__file__).resolve().parents[3]
s=(root/'src/engine/runtime/runtime.cpp').read_text()
body=s.split('command_vm::resolve_shape_definition(std::string_view id) {',1)[1].split('std::expected<command_vm, registration_error> command_vm::create_impl',1)[0]
for forbidden in ['lua_', 's.instructions =', 's.native_bytes =', 's.nodes =', 'services', 'function_reference', 'filesystem']:
    assert forbidden not in body,forbidden
assert 'hash_value(domain, input)' in body
assert 'command_text("external")' in body
assert 'source.bytes' in body and 'source.path' in body
assert '"ordinal"' not in body
assert 'if (_owner && _owner->command.profile)\n    return std::unexpected(external_protocol());' in s
result={'native_only_lookup':True,'no_invocation_budget_reset':True,'no_callable_or_provider_access':True,'ordinal_absent_from_identity':True,'generic_profile_invocation_closed':True}
Path(sys.argv[-1]).write_text(json.dumps(result,indent=2)+'\n')
