#!/usr/bin/env python3
"""Audited reversible namespace/digest variant plus an exact private allocator export for verification."""
import hashlib
import json
from pathlib import Path
import sys

source, output = map(Path, sys.argv[1:3])
mutation = "--mutate-final-receipt" in sys.argv[3:]
output.mkdir(parents=True, exist_ok=True)
manifest = {}
# Relocated declarations live in the real storage_types and history_codec modules; the cloned namespace re-exports them by name,
# and aliases the real types namespace used by the cloned storage declarations.
INJECTED = ('read_guard', 'read_gate_status', 'ledger_format', 'ledger_metadata', 'ledger_access', 'history_head',
            'revision_row', 'event_row', 'request_row', 'event_page', 'execution_event_detail', 'format_error_code', 'format_error',
            'record_reference', 'stored_record', 'event_references', 'connection')
USING = ''.join(f'\nusing ::planar::engine::ledger::{n};' for n in INJECTED) + '\nnamespace types = ::planar::engine::ledger::types;'
# The seam interface imports storage_types only, so it cannot name the storage-only declarations.
SEAM_ONLY_STORAGE = ('read_guard', 'read_gate_status', 'connection')
SEAM_USING = ''.join(f'\nusing ::planar::engine::ledger::{n};' for n in INJECTED if n not in SEAM_ONLY_STORAGE) + '\nnamespace types = ::planar::engine::ledger::types;'
# Clone storage, format and acceptance; no variant file imports the real storage/format/acceptance modules.
for name in ('storage.cppm', 'storage.cpp', 'format.cppm', 'format.cpp', 'acceptance.cppm', 'acceptance.cpp',
             'acceptance_seam.cppm', 'acceptance_seam.cpp'):
    original = (source / name).read_text()
    if name in ('acceptance_seam.cppm', 'acceptance_seam.cpp'):
        # The seam entities live in a nested non-exported seam_detail namespace inside the single renamed ledger namespace.
        if original.count('namespace seam_detail {') != 1 or 'planar::engine::ledger::seam_detail' in original:
            raise SystemExit(f'Unexpected seam_detail shape: {name}')
        if name.endswith('.cppm') and 'export namespace' in original:
            raise SystemExit(f'Seam interface must not use an export namespace: {name}')
    changes = []
    if name.startswith('format'):
        changes.append(('planar.engine.ledger.format;', 'planar.test.atomic_format;'))
    if name in ('acceptance.cppm', 'acceptance.cpp', 'acceptance_seam.cpp'):
        changes.append(('planar.engine.ledger.acceptance;', 'planar.test.atomic_acceptance;'))
    if name == 'acceptance_seam.cppm':
        # The shared transaction body is a cloned interface partition, so it runs over the cloned storage.
        changes.append(('planar.engine.ledger.acceptance:seam;', 'planar.test.atomic_acceptance:seam;'))
    if name == 'acceptance.cppm':
        # Variants never see the profile partition or the runtime; the comment keeps the substitution reversible.
        changes.append(('export import :profile;\n', '// profile partition omitted from the generated variant\n'))
        changes.append(('using ::planar::engine::ledger::acceptance_result;', 'using ::planar::engine::atomic_test::acceptance_result;'))
    if name != 'acceptance_seam.cppm':
        changes.append(('planar.engine.ledger.storage;', 'planar.test.atomic_storage;'))
    changes += [
        ('namespace planar::engine::ledger {', 'namespace planar::engine::atomic_test {'),
    ]
    if name.endswith('.cppm'):
        changes += [('namespace planar::engine::atomic_test {', 'namespace planar::engine::atomic_test {' + (SEAM_USING if name == 'acceptance_seam.cppm' else USING))]
        if name == 'storage.cppm':
            # The cloned helper namespace must match the cpp definitions, which the general rename places in atomic_test::native.
            changes += [
                ('export namespace planar::engine::ledger::native {', 'export namespace planar::engine::atomic_test::native {'),
                ('} // namespace planar::engine::ledger::native', '} // namespace planar::engine::atomic_test::native'),
            ]
    else:
        changes += [('namespace planar::engine::atomic_test {', 'namespace planar::engine::atomic_test {\nusing namespace planar::engine::ledger;')]
    if name == 'storage.cpp':
        changes += [
            ('import planar.engine.ledger.sqlite;', 'import planar.engine.ledger.sqlite;\nimport planar.test.atomic_digest;'),
            ('digest_record(', 'planar::test::atomic_digest::digest('),
            ('digest_request_v2(', 'planar::test::atomic_digest::digest_v2('),
            ('\n#undef LEDGER_TRY', '\nstd::expected<std::int64_t, format_error> allocator_boundary(std::int64_t maximum) {\n  return next_identifier(maximum);\n}\n#undef LEDGER_TRY'),
        ]
    if name == 'storage.cppm':
        changes += [('struct storage_state;\nclass write_transaction;', '''/// @brief Invoke the unchanged private allocator boundary in the isolated test variant.
/// @param maximum Exact currently allocated signed-64 maximum.
/// @return Next ID or the production exhaustion refusal.
std::expected<std::int64_t, format_error> allocator_boundary(std::int64_t maximum);
struct storage_state;
class write_transaction;''')]
    if mutation and name == 'acceptance_seam.cpp':
        changes.append(("if (!candidate.verify())", "if (false)"))
    generated = original
    counts = []
    for before, after in changes:
        count = generated.count(before)
        if before in ('digest_record(', 'digest_request_v2(') and count != 2:
            raise SystemExit(f'Expected exactly one reader and one writer digest site: {name}: {before}: {count}')
        if before.startswith('export import :profile;') and count != 1:
            raise SystemExit(f'Expected exactly one profile re-export: {name}: {count}')
        if not count:
            raise SystemExit(f'Missing exact substitution: {name}: {before}')
        counts.append(count)
        generated = generated.replace(before, after)
    if name.endswith('.cppm') and name != 'acceptance_seam.cppm' and 'using ::planar::engine::ledger::event_references;' not in generated:
        raise SystemExit(f'Missing event_references injection: {name}')
    restored = generated
    for before, after in reversed(changes):
        restored = restored.replace(after, before)
    if restored != original:
        raise SystemExit('Source transformation is not byte reversible')
    target = output / name
    if not target.exists() or target.read_text() != generated:
        target.write_text(generated)
    manifest[name] = {
        'source_sha256': hashlib.sha256(original.encode()).hexdigest(),
        'generated_sha256': hashlib.sha256(generated.encode()).hexdigest(),
        'reverse_identical': True,
        'substitutions': [{'before': b, 'after': a, 'count': n} for (b, a), n in zip(changes, counts)],
    }
report = json.dumps(manifest, indent=2) + '\n'
if not (output / 'generation.json').exists() or (output / 'generation.json').read_text() != report:
    (output / 'generation.json').write_text(report)
