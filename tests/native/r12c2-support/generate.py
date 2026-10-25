#!/usr/bin/env python3
"""Mechanical fault variant: module/namespace identity, one import, three syscall targets only."""
import hashlib
import json
from pathlib import Path
import sys

source, output = map(Path, sys.argv[1:])
output.mkdir(parents=True, exist_ok=True)
manifest = {}
for name in ('format.cppm', 'format.cpp'):
    original = (source / name).read_text()
    generated = original
    substitutions = [
        ('planar.engine.ledger.format;', 'planar.test.ledger_format;'),
        ('namespace planar::engine::ledger {', 'namespace planar::engine::ledger_fault {'),
    ]
    if name.endswith('.cppm'):
        # Real storage stays in use; the types relocated to storage_types/storage are injected into the cloned namespace.
        substitutions += [('namespace planar::engine::ledger_fault {', 'namespace planar::engine::ledger_fault {\nusing ::planar::engine::ledger::read_guard;\nusing ::planar::engine::ledger::read_gate_status;\nusing ::planar::engine::ledger::storage_read_guard;\nusing ::planar::engine::ledger::read_snapshot;\nusing ::planar::engine::ledger::write_transaction;\nusing ::planar::engine::ledger::ledger_format;\nusing ::planar::engine::ledger::ledger_metadata;\nusing ::planar::engine::ledger::ledger_access;\nusing ::planar::engine::ledger::history_head;\nusing ::planar::engine::ledger::revision_row;\nusing ::planar::engine::ledger::event_row;\nusing ::planar::engine::ledger::request_row;\nusing ::planar::engine::ledger::event_page;\nusing ::planar::engine::ledger::format_error_code;\nusing ::planar::engine::ledger::format_error;\nusing ::planar::engine::ledger::record_reference;\nusing ::planar::engine::ledger::stored_record;')]
    if name.endswith('.cpp'):
        substitutions += [
            ('import planar.engine.ledger.sqlite;', 'import planar.engine.ledger.sqlite;\nimport planar.test.ledger_posix;'),
            ('namespace planar::engine::ledger_fault {', 'namespace planar::engine::ledger_fault {\nusing namespace planar::engine::ledger;'),
            ('::fsync(', 'planar::test::ledger_posix::fsync('),
            ('::linkat(', 'planar::test::ledger_posix::linkat('),
            ('::unlinkat(', 'planar::test::ledger_posix::unlinkat('),
        ]
    counts = []
    for before, after in substitutions:
        count = generated.count(before)
        if count == 0:
            raise SystemExit(f'Missing declared substitution: {name}: {before}')
        counts.append(count)
        generated = generated.replace(before, after)
    # Reversibility establishes unchanged bytes outside the declared substitutions.
    restored = generated
    for before, after in reversed(substitutions):
        restored = restored.replace(after, before)
    if restored != original:
        raise SystemExit('Non-reversible source transformation')
    dest = output / name
    if not dest.exists() or dest.read_text() != generated:
        dest.write_text(generated)
    manifest[name] = {
        'source_sha256': hashlib.sha256(original.encode()).hexdigest(),
        'generated_sha256': hashlib.sha256(generated.encode()).hexdigest(),
        'substitutions': [{'before': b, 'after': a, 'count': n} for (b, a), n in zip(substitutions, counts)],
        'reverse_identical': True,
    }
(output / 'generation.json').write_text(json.dumps(manifest, indent=2) + '\n')
