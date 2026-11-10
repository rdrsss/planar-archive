"""X08 generated E12 variant: a test-only CLI whose fixed Lua exports are poisoned (Q3 A, D8-2; DECISIONS rows 2, 2b, 2c, 29).

argv: repository root, output directory. Writes <output>/0 with the C++ variant, the Lua inputs and generation.json. CMake
reads generation.json and embeds the chunk in the order it lists, so the binary and the audit cannot disagree.

C++ copies differ from product files only by the module and namespace substitutions recorded per file, each reverse-identical
(the X07 audit). The embedded Lua is the shipped chunk with three changes: json.lua gets one prepended comment line;
x08_poison.lua is inserted between acceptance.lua and commands.lua and reassigns the chunk-level locals in POISONED, which the
commands.lua handlers and the bootstrap export table capture; bootstrap.lua has the parse_json definition replaced. The shipped
sources are read only; no product hook is added.
"""
from pathlib import Path
import hashlib, json, re, sys

root, output = map(Path, sys.argv[1:3])
VARIANT = 0
STDLIB = root / 'src/stdlib/planar'
PRODUCT = root / 'src/cmd/planar'
RUNTIME = root / 'src/engine/runtime'
variant = output / str(VARIANT)
# Shipped chunk order: one concatenated chunk at run time (src/cmd/planar/CMakeLists.txt embeds these in this order).
CHUNK = ['json.lua', 'schema.lua', 'semantic.lua', 'selection.lua', 'presentation.lua', 'observations.lua', 'render.lua',
         'acceptance.lua', 'commands.lua', 'bootstrap.lua']
POISONED = ['route_workspace', 'normalize_bundle', 'validate_bundle', 'prepare_shapes', 'check_shape', 'finish_shapes',
            'interpret_bundle', 'select_role', 'select_profile_material', 'render', 'assemble_projection_material',
            'prepare_profile_render', 'render_profile_material', 'role_profile_material', 'role_projection_base_material']
JSON_COMMENT = '-- X08 variant: embedded stdlib bytes differ\n'
PARSE_BODY = 'local function parse_json(input)\n  error("X08 poisoned stdlib export: parse_json",0)\nend\n'
FAMILIES = ['render', 'init', 'history', 'export', 'load', 'extension', 'activate', 'status', 'preview', 'claim', 'takeover', 'refresh', 'checkpoint', 'release', 'recover', 'check', 'check-recover', 'verdict']
RUNTIME_FILES = ['runtime.cpp', 'runtime.cppm']
PRODUCT_FILES = ['bootstrap.cpp', 'bootstrap.cppm', 'invocation.cpp', 'invocation.cppm'] + [
    f'handlers/{family}/command.{ext}' for family in FAMILIES for ext in ('cpp', 'cppm')]
# Each anchor marker must occur exactly once; variant 0 keeps the shipped anchor comments and changes no anchor.
ANCHORS = {'invocation.cpp': ['after-early-miss', 'after-factory-open', 'after-prepare-load', 'after-commit', 'response-ceiling'],
           'handlers/extension/command.cpp': ['before-result-seal', 'after-final-verify', 'response-ceiling']}


def sha(text):
    return hashlib.sha256(text.encode()).hexdigest()


def shipped(name):
    return (STDLIB / name).read_text()


def put(path, text):
    # Rewrite only on change so an unchanged generator run leaves timestamps and downstream steps alone.
    if not path.exists() or path.read_text() != text:
        path.write_text(text)


# Lua: every poisoned name must be a single chunk-level local declared before acceptance.lua ends (the poison insertion point),
# so the assignment reassigns that local instead of creating a global.
chunk_text = '\n'.join(shipped(name) for name in CHUNK)
insertion = len('\n'.join(shipped(name) for name in CHUNK[:CHUNK.index('acceptance.lua') + 1]))
for name in POISONED:
    matches = list(re.finditer(rf'^local (?:function )?{re.escape(name)}\b', chunk_text, re.M))
    assert len(matches) == 1, (name, len(matches))
    assert matches[0].start() < insertion, (name, 'declared after the poison insertion point')

json_text = JSON_COMMENT + shipped('json.lua')
boot = shipped('bootstrap.lua')
definitions = re.findall(r'^local function parse_json\(input\)\n.*?^end\n', boot, re.M | re.S)
assert len(definitions) == 1, len(definitions)
boot_text = boot.replace(definitions[0], PARSE_BODY, 1)
assert boot_text.count(PARSE_BODY) == 1
poison_text = ''.join(f'{name} = function() error("X08 poisoned stdlib export: {name}",0) end\n' for name in POISONED)

# C++: the X07 substitutions with variant index 0. Every substitution is applied forward and must restore the shipped text.
RUNTIME_CHANGES = [('planar.engine.runtime;', 'planar.engine.x08_bundle_0;'),
                   ('planar.engine.runtime_source;', 'planar.engine.x08_bundle_source_0;'),
                   ('namespace planar::engine {', 'namespace planar::engine::x08_bundle_0 {')]
COMMAND_CHANGES = [
    ('import planar.engine.runtime; // X07-REAL-RUNTIME', 'import planar.engine.x08_real_runtime; // X07-REAL-RUNTIME'),
    ('planar.cmd.bootstrap', 'planar.cmd.x08_0.bootstrap'),
    ('planar.cmd.invocation', 'planar.cmd.x08_0.invocation'),
    ('planar.cmd.planar.handlers.', 'planar.cmd.x08_0.handlers.'),
    ('planar.cmd.execution.', 'planar.cmd.x08_0.execution.'),
    ('::planar::cmd::handlers::', '::planar::cmd::x08_0::handlers::'),
    ('namespace planar::cmd {', 'namespace planar::cmd::x08_0 {'),
    ('namespace planar::cmd::handlers {', 'namespace planar::cmd::x08_0::handlers {'),
    ('planar.engine.runtime;', 'planar.engine.x08_bundle_0;'),
    ('engine::command_', 'engine::x08_bundle_0::command_'),
    ('engine::profile_invocation', 'engine::x08_bundle_0::profile_invocation'),
    ('engine::registration_error', 'engine::x08_bundle_0::registration_error'),
    ('engine::extension_', 'engine::x08_bundle_0::extension_'),
    ('engine::prepared_profile', 'engine::x08_bundle_0::prepared_profile'),
    ('import planar.engine.x08_real_runtime; // X07-REAL-RUNTIME', 'import planar.engine.runtime; // X07-REAL-RUNTIME'),
]
INVOCATION_EXTRA = [('using namespace engine;', 'using namespace engine;\n' + '\n'.join(
    f'using engine::x08_bundle_0::{name};' for name in ['command_services', 'command_service_result', 'command_render_options',
                                                       'command_init_options', 'command_history_options',
                                                       'command_history_selector', 'command_export_options',
                                                       'command_load_options', 'command_activate_options', 'command_status_options',
                                                       'command_preview_options', 'command_run_options',
                                                       'command_recovery_options', 'command_recover_options',
                                                       'command_check_options']))]
MAIN_CHANGES = [('planar.cmd.bootstrap', 'planar.cmd.x08_0.bootstrap'), ('planar::cmd::invoke', 'planar::cmd::x08_0::invoke')]

manifest_cpp = {}


def write_cpp(rel, source, changes):
    original = source.read_text()
    for anchor in ANCHORS.get(rel, []):
        assert original.count(f'// X07-ANCHOR:{anchor}') == 1, (rel, anchor)
    data = original
    for before, after in changes:
        data = data.replace(before, after)
    restored = data
    for before, after in reversed(changes):
        restored = restored.replace(after, before)
    assert restored == original, rel
    dest = variant / rel
    dest.parent.mkdir(parents=True, exist_ok=True)
    put(dest, data)
    manifest_cpp[rel] = {'sha256': sha(data), 'source_sha256': sha(original), 'reverse_identical': True,
                         'substitutions': [[before, after] for before, after in changes]}


variant.mkdir(parents=True, exist_ok=True)
for rel in RUNTIME_FILES:
    write_cpp(rel, RUNTIME / rel, RUNTIME_CHANGES)
for rel in PRODUCT_FILES:
    extra = INVOCATION_EXTRA if rel == 'invocation.cpp' else []
    write_cpp(rel, PRODUCT / rel, COMMAND_CHANGES + extra)
write_cpp('main.cpp', PRODUCT / 'main.cpp', MAIN_CHANGES)

# Embedded chunk in order; the poison file sits between acceptance.lua and commands.lua.
put(variant / 'json.lua', json_text)
put(variant / 'bootstrap.lua', boot_text)
put(variant / 'x08_poison.lua', poison_text)
embedded = []
for name in CHUNK:
    if name == 'json.lua':
        path = variant / 'json.lua'
    elif name == 'bootstrap.lua':
        path = variant / 'bootstrap.lua'
    else:
        path = STDLIB / name
    embedded.append({'logical': f'planar/{name}', 'path': str(path), 'generated': path.parent == variant})
    if name == 'acceptance.lua':
        embedded.append({'logical': 'planar/x08_poison.lua', 'path': str(variant / 'x08_poison.lua'), 'generated': True})

manifest = {
    'variant': VARIANT,
    'json_lua': {'shipped_sha256': sha(shipped('json.lua')), 'generated_sha256': sha(json_text),
                 'prepended_line': JSON_COMMENT},
    'bootstrap_lua': {'shipped_sha256': sha(boot), 'generated_sha256': sha(boot_text),
                      'replaced_definition_sha256': sha(definitions[0]), 'replacement': PARSE_BODY},
    'poison': {'path': str(variant / 'x08_poison.lua'), 'sha256': sha(poison_text), 'names': POISONED},
    'embedded': embedded,
    'cpp': manifest_cpp,
}
put(variant / 'generation.json', json.dumps(manifest, indent=2, sort_keys=True) + '\n')
print(f'generated X08 variant {VARIANT}: {len(embedded)} embedded chunks, {len(manifest_cpp)} C++ inputs, '
      f'{len(POISONED)} poisoned exports')
