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
for name in ('format.cppm', 'format.cpp'):
    original = (source / name).read_text()
    substitutions = [
        ('planar.engine.ledger.format;', 'planar.test.metadata_format;', 1),
        ('namespace planar::engine::ledger {', 'namespace planar::engine::metadata_test {', 1),
    ]
    if name.endswith('.cppm'):
        substitutions += [('namespace planar::engine::metadata_test {', 'namespace planar::engine::metadata_test {\nusing ::planar::engine::ledger::read_guard;\nusing ::planar::engine::ledger::read_gate_status;', 1)]
    if name == 'format.cpp':
        substitutions += [
            ('import std;', 'import std;\nimport planar.test.metadata_fixture;', 1),
            ('namespace planar::engine::metadata_test {',
             'namespace planar::engine::metadata_test {\nusing namespace planar::engine::ledger;', 1),
            ('  auto checked    = verify(state->db);',
             '  auto checked    = verify(state->db);\n'
             '  checked = ::planar::test::metadata_fixture::supply(std::move(checked), '
             '::planar::test::metadata_fixture::boundary::begin);', 1),
            ('  auto checked = verify(state->db);',
             '  auto checked = verify(state->db);\n'
             '  checked = ::planar::test::metadata_fixture::supply(std::move(checked), '
             '::planar::test::metadata_fixture::boundary::writer_commit);', 1),
            ('  auto checked = verify(db);',
             '  auto checked = verify(db);\n'
             '  checked = ::planar::test::metadata_fixture::supply(std::move(checked), '
             '::planar::test::metadata_fixture::boundary::acceptance_commit);', 1),
        ]
    generated = original
    records = []
    for before, after, expected in substitutions:
        count = generated.count(before)
        if count != expected:
            raise SystemExit(f'{name}: expected {expected} substitutions, found {count}: {before}')
        generated = generated.replace(before, after)
        records.append({'before': before, 'after': after, 'count': count})
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
