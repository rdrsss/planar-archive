#!/usr/bin/env python3
"""Exact reversible export fault variant; only module/namespace and syscall-boundary redirects."""
from pathlib import Path
import hashlib
import json
import sys
source, output = map(Path, sys.argv[1:])
output.mkdir(parents=True, exist_ok=True)
manifest = {}
for name in ['source_export.cppm', 'source_export.cpp']:
    original = (source / name).read_text()
    substitutions = [
        ('planar.engine.source_export;', 'planar.test.source_export;'),
        ('namespace planar::engine::source_export {', 'namespace planar::engine::source_export_fault {'),
    ]
    if name.endswith('.cpp'):
        substitutions += [('import planar.engine.ledger.format;', 'import planar.engine.ledger.format;\nimport planar.test.export_posix;')]
        for call in ['mkdirat', 'openat', 'write', 'fchmod', 'fsync', 'close', 'unlinkat']:
            substitutions += [('::' + call + '(', 'planar::test::export_posix::' + call + '(')]
        substitutions += [('int publish(int parent, const char* stage, const char* target) noexcept {',
                           'int publish(int parent, const char* stage, const char* target) {\n  if (auto result = planar::test::export_posix::before_publish(parent, stage, target)) return *result;')]
    generated = original
    counts = []
    for before, after in substitutions:
        count = generated.count(before)
        if not count:
            raise SystemExit(f'Missing substitution: {name}: {before}')
        counts.append(count)
        generated = generated.replace(before, after)
    restored = generated
    for before, after in reversed(substitutions):
        restored = restored.replace(after, before)
    if restored != original:
        raise SystemExit('Non-reversible transformation')
    dest = output / name
    if not dest.exists() or dest.read_text() != generated:
        dest.write_text(generated)
    manifest[name] = dict(source_sha256=hashlib.sha256(original.encode()).hexdigest(),
                          generated_sha256=hashlib.sha256(generated.encode()).hexdigest(), reverse_identical=True,
                          substitutions=[dict(before=b, after=a, count=n) for (b,a),n in zip(substitutions, counts)])
(output/'generation.json').write_text(json.dumps(manifest,indent=2)+'\n')
