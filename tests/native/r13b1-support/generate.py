#!/usr/bin/env python3
"""Exact reversible test-only format transformation; optionally remove intern equality for mutation evidence."""
import hashlib
import json
from pathlib import Path
import sys

source, output = map(Path, sys.argv[1:3])
mutation = '--mutate-equality' in sys.argv[3:]
output.mkdir(parents=True, exist_ok=True)
manifest = {'verification_mutation': mutation, 'files': {}}
# Relocated declarations live in the real storage_types and history_codec modules; the cloned namespace re-exports them by name,
# and aliases the real types namespace used by the cloned storage declarations.
INJECTED = ('read_guard', 'read_gate_status', 'ledger_format', 'ledger_metadata', 'ledger_access', 'history_head',
            'revision_row', 'event_row', 'request_row', 'event_page', 'format_error_code', 'format_error',
            'record_reference', 'stored_record', 'event_references', 'connection')
USING = ''.join(f'\nusing ::planar::engine::ledger::{n};' for n in INJECTED) + '\nnamespace types = ::planar::engine::ledger::types;'
# Clone storage first, then format; the real storage/format modules are never imported by a variant file.
for name in ('storage.cppm', 'storage.cpp', 'format.cppm', 'format.cpp'):
    original = (source / name).read_text()
    generated = original
    substitutions = []
    if name.startswith('format'):
        substitutions.append(('planar.engine.ledger.format;', 'planar.test.record_format;'))
    substitutions += [
        ('planar.engine.ledger.storage;', 'planar.test.record_storage;'),
        ('namespace planar::engine::ledger {', 'namespace planar::engine::record_test {'),
    ]
    if name.endswith('.cppm'):
        substitutions += [('namespace planar::engine::record_test {', 'namespace planar::engine::record_test {' + USING)]
        if name == 'storage.cppm':
            # The cloned helper namespace must match the cpp definitions, which the general rename places in record_test::native.
            substitutions += [
                ('export namespace planar::engine::ledger::native {', 'export namespace planar::engine::record_test::native {'),
                ('} // namespace planar::engine::ledger::native', '} // namespace planar::engine::record_test::native'),
            ]
    if name.endswith('.cpp'):
        substitutions += [
            ('namespace planar::engine::record_test {', 'namespace planar::engine::record_test {\nusing namespace planar::engine::ledger;'),
        ]
    if name == 'storage.cpp':
        substitutions += [
            ('import planar.engine.ledger.sqlite;', 'import planar.engine.ledger.sqlite;\nimport planar.test.record_digest;'),
            ('digest_record(', 'planar::test::forced_digest('),
        ]
        if mutation:
            substitutions += [
                ('if (!bytes || *bytes != *encoded)\n        return refuse(format_error_code::hash_collision,',
                 'if (!bytes)\n        return refuse(format_error_code::hash_collision,'),
            ]
    counts = []
    for before, after in substitutions:
        count = generated.count(before)
        if count == 0:
            raise SystemExit(f'Missing declared substitution: {name}: {before}')
        counts.append(count)
        generated = generated.replace(before, after)
    if name.endswith('.cppm') and 'using ::planar::engine::ledger::event_references;' not in generated:
        raise SystemExit(f'Missing event_references injection: {name}')
    restored = generated
    for before, after in reversed(substitutions):
        restored = restored.replace(after, before)
    if restored != original:
        raise SystemExit('Non-reversible source transformation')
    dest = output / name
    if not dest.exists() or dest.read_text() != generated:
        dest.write_text(generated)
    manifest['files'][name] = {
        'source_sha256': hashlib.sha256(original.encode()).hexdigest(),
        'generated_sha256': hashlib.sha256(generated.encode()).hexdigest(),
        'substitutions': [{'before': b, 'after': a, 'count': n} for (b, a), n in zip(substitutions, counts)],
        'reverse_identical': True,
    }
text = json.dumps(manifest, indent=2) + '\n'
if not (output / 'generation.json').exists() or (output / 'generation.json').read_text() != text:
    (output / 'generation.json').write_text(text)
