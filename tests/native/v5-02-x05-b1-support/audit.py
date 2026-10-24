"""Audit exact native copies, embedded helper originals and unchanged product export list."""
from pathlib import Path
import hashlib, json, re, sys
root = Path(__file__).resolve().parents[3]
build = Path(sys.argv[1]).resolve()
report = Path(sys.argv[2])
sha = lambda data: hashlib.sha256(data).hexdigest()
rows = {}
for suffix in ['cpp', 'cppm']:
    production = (root / f'src/engine/runtime/runtime.{suffix}').read_text()
    expected = production.replace('module planar.engine.runtime;', 'module planar.engine.shapes_test;').replace('import planar.engine.runtime_source;', 'import planar.engine.shapes_test_source;').replace('namespace planar::engine {', 'namespace planar::engine::shapes_test {')
    actual = (build / f'src/engine/shapes_test.{suffix}').read_text()
    assert actual == expected
    restored = actual.replace('module planar.engine.shapes_test;', 'module planar.engine.runtime;').replace('import planar.engine.shapes_test_source;', 'import planar.engine.runtime_source;').replace('namespace planar::engine::shapes_test {', 'namespace planar::engine {')
    assert restored == production
    rows[suffix] = {'production': sha(production.encode()), 'test': sha(actual.encode())}
embedded = (build / 'src/engine/planar_shapes_test_source.cppm').read_text()
arrays = {name: bytes(int(x, 16) for x in re.findall(r'0x([0-9a-f]{2})', body)) for name, body in re.findall(r'constexpr unsigned char (\w+)\[\] = \{([^}]*)\};', embedded)}
originals = re.findall(r'\{"([^"]+)", \{reinterpret_cast<const char\*>\((original_\d+)\)', embedded)
assert [p for p, _ in originals] == ['planar/json.lua', 'planar/schema.lua', 'planar/semantic.lua', 'tests/stdlib/x05-shapes.lua']
assembled = b''
for path, name in originals:
    expected = (root / ('src/stdlib/' + path if path.startswith('planar/') else path)).read_bytes()
    assert arrays[name] == expected
    assembled += expected + b'\n'
assert arrays['source_bytes'] == assembled
before = (root / 'tests/native/v5-02-x05-b1-evidence/original-source/engine-CMakeLists.txt').read_text()
after = (root / 'src/engine/CMakeLists.txt').read_text()
block = lambda text: text[text.index('planar_embed_lua(planar_runtime_source '):text.index('planar_rewrite_module(planar_closure_guard')]
assert block(before) == block(after)
report.write_text(json.dumps({'native': rows, 'originals': [p for p, _ in originals], 'assembly_sha256': sha(assembled), 'product_embed_list_unchanged': True}, indent=2) + '\n')
print('Exact native forward/reverse copies and four original/assembly sources pass; production embed list unchanged')
