#!/usr/bin/env python3
"""Audited reversible source variant: only returned verified metadata is supplied.

This proves comparison and rollback plumbing under controlled metadata results;
it does not reproduce an actual external format change or convert SQLite data.
"""
import hashlib
import json
from pathlib import Path
import sys

source, output = map(Path, sys.argv[1:3])
output.mkdir(parents=True, exist_ok=True)
manifest = {'purpose': __doc__, 'files': {}}
# Relocated declarations live in the real storage_types and history_codec modules; the cloned namespace re-exports them by name,
# and aliases the real types namespace used by the cloned storage declarations.
INJECTED = ('read_guard', 'read_gate_status', 'ledger_format', 'ledger_metadata', 'ledger_access', 'history_head',
            'revision_row', 'event_row', 'request_row', 'event_page', 'format_error_code', 'format_error',
            'record_reference', 'stored_record', 'event_references', 'connection')
USING = ''.join(f'\nusing ::planar::engine::ledger::{n};' for n in INJECTED) + '\nnamespace types = ::planar::engine::ledger::types;'
fixture = '::planar::test::metadata_fixture::'
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
    substitutions = []
    if name.startswith('format'):
        substitutions.append(('planar.engine.ledger.format;', 'planar.test.metadata_format;', 1))
    if name in ('acceptance.cppm', 'acceptance.cpp', 'acceptance_seam.cpp'):
        substitutions.append(('planar.engine.ledger.acceptance;', 'planar.test.metadata_acceptance;', 1))
    if name == 'acceptance_seam.cppm':
        # The shared transaction body is a cloned interface partition, so it runs over the cloned storage.
        substitutions.append(('planar.engine.ledger.acceptance:seam;', 'planar.test.metadata_acceptance:seam;', 1))
    if name == 'acceptance.cppm':
        # Variants never see the profile partition or the runtime; the comment keeps the substitution reversible.
        substitutions.append(('export import :profile;\n', '// profile partition omitted from the generated variant\n', 1))
        substitutions.append(('using ::planar::engine::ledger::acceptance_result;',
                              'using ::planar::engine::metadata_test::acceptance_result;', 1))
    if name != 'acceptance_seam.cppm':
        substitutions.append(('planar.engine.ledger.storage;', 'planar.test.metadata_storage;', 1))
    substitutions += [
        ('namespace planar::engine::ledger {', 'namespace planar::engine::metadata_test {', 1),
    ]
    if name.endswith('.cppm'):
        substitutions += [('namespace planar::engine::metadata_test {', 'namespace planar::engine::metadata_test {' + (SEAM_USING if name == 'acceptance_seam.cppm' else USING), 1)]
        if name == 'storage.cppm':
            # The cloned helper namespace must match the cpp definitions, which the general rename places in metadata_test::native.
            substitutions += [
                ('export namespace planar::engine::ledger::native {', 'export namespace planar::engine::metadata_test::native {', 1),
                ('} // namespace planar::engine::ledger::native', '} // namespace planar::engine::metadata_test::native', 1),
            ]
    else:
        substitutions += [('namespace planar::engine::metadata_test {',
                           'namespace planar::engine::metadata_test {\nusing namespace planar::engine::ledger;', 1)]
    if name == 'storage.cpp':
        substitutions += [
            ('import std;', 'import std;\nimport planar.test.metadata_fixture;', 1),
            # begin_write: metadata verified under the new write snapshot.
            ('  auto checked    = verify(state->db);',
             '  auto checked    = verify(state->db);\n'
             f'  checked = {fixture}supply(std::move(checked), {fixture}boundary::begin);', 1),
            # verify_before_commit: one site serves both record-writer and acceptance commits.
            ('    auto checked = verify(state.db);',
             '    auto checked = verify(state.db);\n'
             f'    checked = {fixture}supply(std::move(checked), site == commit_site::acceptance ? '
             f'{fixture}boundary::acceptance_commit : {fixture}boundary::writer_commit);', 1),
        ]
    generated = original
    records = []
    for before, after, expected in substitutions:
        count = generated.count(before)
        if count != expected:
            raise SystemExit(f'{name}: expected {expected} substitutions, found {count}: {before}')
        generated = generated.replace(before, after)
        records.append({'before': before, 'after': after, 'count': count})
    if name.endswith('.cppm') and name != 'acceptance_seam.cppm' and 'using ::planar::engine::ledger::event_references;' not in generated:
        raise SystemExit(f'Missing event_references injection: {name}')
    restored = generated
    for before, after, _ in reversed(substitutions):
        if restored.count(after) != 1:
            raise SystemExit(f'Non-unique reverse substitution: {name}: {after}')
        restored = restored.replace(after, before)
    if restored != original:
        raise SystemExit(f'Non-reversible source transformation: {name}')
    dest = output / name
    if not dest.exists() or dest.read_text() != generated:
        dest.write_text(generated)
    manifest['files'][name] = {
        'source_sha256': hashlib.sha256(original.encode()).hexdigest(),
        'generated_sha256': hashlib.sha256(generated.encode()).hexdigest(),
        'substitutions': records,
        'reverse_identical': True,
    }
text = json.dumps(manifest, indent=2) + '\n'
dest = output / 'generation.json'
if not dest.exists() or dest.read_text() != text:
    dest.write_text(text)
