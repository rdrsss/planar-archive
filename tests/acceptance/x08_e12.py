"""X08 E12 variant acceptance: a generated binary whose fixed Lua exports are poisoned reads and exports stored ledgers identically.

argv: <case> <normal planar> <variant planar> <generated variant directory>. Every case runs real processes in its own scratch
tree below one temporary directory; HOME and PLANAR_DB point into that tree and the forbidden legacy path is asserted absent.
Normal outputs are canonical JSON bytes. The liveness cases prove the poison is reached (nonzero exit, empty stdout), so an
identical result cannot come from a variant that never runs the poisoned code. An unexpected result on the unmutated tree is a
product defect candidate: assertions are never weakened here.
"""
from pathlib import Path
import difflib, hashlib, json, os, re, shutil, sqlite3, stat, subprocess, sys, tempfile

CASES = ('e12_variant_reads_identically', 'e12_variant_exports_identically', 'e12_variant_poison_is_live',
         'e12_variant_poison_is_live_load', 'e12_variant_poison_is_live_profile_render', 'e12_variant_audit',
         'e12_sources_and_profile_removed')
if len(sys.argv) != 5 or sys.argv[1] not in CASES:
    raise SystemExit('usage: x08_e12.py <case> <normal planar> <variant planar> <generated variant directory>')
CASE = sys.argv[1]
NORMAL = str(Path(sys.argv[2]).resolve())
VARIANT = str(Path(sys.argv[3]).resolve())
GENERATED = Path(sys.argv[4]).resolve()
REPO = Path(__file__).resolve().parents[2]
FIXTURE = REPO / 'fixtures/render/quota'
STDLIB = REPO / 'src/stdlib/planar'
PRODUCT_CMD = REPO / 'src/cmd/planar'
PRODUCT_RUNTIME = REPO / 'src/engine/runtime'

# The independent plan (DECISIONS rows 2, 2c; INVARIANTS I12D): these lists are not read back from the generator.
EXPECTED_EMBED = ['planar/json.lua', 'planar/schema.lua', 'planar/semantic.lua', 'planar/selection.lua',
                  'planar/presentation.lua', 'planar/observations.lua', 'planar/render.lua', 'planar/acceptance.lua',
                  'planar/x08_poison.lua', 'planar/commands.lua', 'planar/bootstrap.lua']
EXPECTED_POISON = ['route_workspace', 'normalize_bundle', 'validate_bundle', 'prepare_shapes', 'check_shape', 'finish_shapes',
                   'interpret_bundle', 'select_role', 'select_profile_material', 'render', 'assemble_projection_material',
                   'prepare_profile_render', 'render_profile_material', 'role_profile_material', 'role_projection_base_material']
EXPECTED_CPP = ['bootstrap.cpp', 'bootstrap.cppm', 'invocation.cpp', 'invocation.cppm', 'main.cpp', 'runtime.cpp', 'runtime.cppm'] + [
    f'handlers/{family}/command.{ext}' for family in ('export', 'extension', 'history', 'init', 'load', 'render')
    for ext in ('cpp', 'cppm')]
JSON_COMMENT = '-- X08 variant: embedded stdlib bytes differ\n'
PARSE_ERROR = '  error("X08 poisoned stdlib export: parse_json",0)\n'
EXPECTED_WORKSPACE = ['combined.lua', 'spec.lua', 'split.lua', 'text/quota-api.md', 'text/shared.md', 'workspace.lua']
RENDER = ['render', '--workspace', 'workspace/workspace.lua', '--registry', 'registry/quota-tools.json', '--root', 'source=current',
          '--job', 'combined', '--role', 'builder', '--capture-outputs', '--base-root', 'source=base',
          '--observations', 'observations/empty.json']
SELECTOR = ['with-profile', '--stdlib-profile', '../trusted/p.json']
SHAPE = ("local D={id='custom',abi='shape_v1',revision='final',stages='optional',help='shape'}\n"
         "registration.shape(D,function(input) return {ok=true,value={obligations=engine.array{},"
         "guidance='custom workflow guidance'}} end)\n")


def canonical(value):
    return json.dumps(value, sort_keys=True, separators=(',', ':'), ensure_ascii=False).encode() + b'\n'


def digest(data):
    return hashlib.sha256(data).hexdigest()


def normal_output(got):
    """A successful normal run: exit 0, empty stderr, stdout exactly canonical JSON."""
    assert got.returncode == 0 and got.stderr == b'', (got.returncode, got.stderr[:300])
    assert got.stdout == canonical(json.loads(got.stdout)), got.stdout[:200]
    return got.stdout


def assert_dead(got, where):
    """A poisoned run: nonzero exit with empty stdout."""
    assert got.returncode != 0 and got.stdout == b'', (where, got.returncode, got.stdout[:200], got.stderr[:200])


def tree(path):
    """Regular files under path as {relative path: (sha256, raw mode)}; symlinks and other types are refused."""
    entries = {}
    for item in sorted(path.rglob('*')):
        if item.is_dir():
            continue
        assert item.is_file() and not item.is_symlink(), item
        entries[item.relative_to(path).as_posix()] = (digest(item.read_bytes()), stat.S_IMODE(item.stat().st_mode))
    return entries


def normalised(entries):
    """The same entries with the executable bit mapped to 0755 and everything else to 0644."""
    return {name: (value, 0o755 if mode & 0o111 else 0o644) for name, (value, mode) in entries.items()}


def generation():
    return json.loads((GENERATED / 'generation.json').read_text())


def changed(before, after):
    matcher = difflib.SequenceMatcher(None, before, after, autojunk=False)
    return [op for op in matcher.get_opcodes() if op[0] != 'equal']


def rows(ledger):
    with sqlite3.connect(ledger) as db:
        return tuple(db.execute('SELECT COUNT(*) FROM ' + name).fetchone()[0] for name in ('revisions', 'requests', 'events'))


class World:
    """One scratch tree: work (cwd), trusted (the profile), fx (plain fixture copy) and fxc (custom-shape fixture copy)."""

    def __init__(self, root):
        self.root = root
        self.work = root / 'work'
        self.trusted = root / 'trusted'
        self.fx = root / 'fx'
        self.fxc = root / 'fxc'
        self.work.mkdir()
        shutil.copytree(FIXTURE, self.fx)
        shutil.copytree(FIXTURE, self.fxc)
        for name in ('split.lua', 'combined.lua'):
            path = self.fxc / 'workspace' / name
            text = path.read_text()
            assert 'fanout_review' in text, name
            path.write_text(text.replace('fanout_review', 'custom'))
        (self.trusted / 'modules').mkdir(parents=True)
        (self.trusted / 'p.json').write_text('{"version":1,"modules":["modules/p.lua"]}')
        (self.trusted / 'modules/p.lua').write_text(SHAPE)
        self.env = {key: value for key, value in os.environ.items() if not key.startswith(('PLANAR_', 'X07_', 'X08_'))}
        self.env.update(HOME=str(root), PLANAR_DB=str(root / 'forbidden-legacy.db'))

    def run(self, exe, args, cwd=None):
        got = subprocess.run([exe, *args], capture_output=True, env=self.env, cwd=cwd or self.work, timeout=120)
        assert str(self.root).encode() not in got.stdout + got.stderr, (args, got.stderr[:200])
        return got

    def init(self, name):
        """A fresh ledger made by the normal binary; both binaries then read it or load into their own copy."""
        ledger = self.work / (name + '.db')
        normal_output(self.run(NORMAL, ['init', '--ledger', str(ledger), '--workspace-id', 'quota']))
        return ledger


def load_args(ledger, workspace, registry, key, selector=False):
    args = ['load', '--ledger', str(ledger), '--workspace', str(workspace), '--registry', str(registry),
            '--expect-revision', 'none', '--request-id', key, '--actor', 'driver']
    return SELECTOR + args if selector else args


def custom_ledger(w):
    """The profile-written format2 ledger: the custom-shape load of the customised fixture (x07.workflow_shape_load)."""
    ledger = w.init('custom')
    normal_output(w.run(NORMAL, load_args(ledger, w.fxc / 'workspace/workspace.lua', w.fxc / 'registry/quota-tools.json',
                                          'custom', selector=True)))
    assert rows(ledger)[0] == 1, rows(ledger)
    with sqlite3.connect(ledger) as db:
        body = db.execute("SELECT encoded FROM records JOIN requests ON requests.body_id=records.id "
                          "WHERE request_id='custom'").fetchone()[0]
    assert body.startswith(b'PLANAR-REQUEST\x02'), body[:40]
    return ledger


def case_reads(w):
    ledger = custom_ledger(w)
    shutil.rmtree(w.trusted)
    base = ['history', '--ledger', str(ledger)]
    events = json.loads(normal_output(w.run(NORMAL, base)))['data']['events']
    assert len(events) >= 2, events
    for args in (base, base + ['--revision', '1'], base + ['--request-id', 'custom'],
                 base + ['--at-event', events[-1]['sequence']],
                 base + ['--after-event', events[0]['sequence'], '--limit', '1']):
        expected = normal_output(w.run(NORMAL, args))
        got = w.run(VARIANT, args)
        assert got.returncode == 0 and got.stderr == b'' and got.stdout == expected, (args, got.returncode, got.stderr[:300])
    assert not (w.root / 'forbidden-legacy.db').exists()


def case_exports(w):
    ledger = custom_ledger(w)
    # Captured before any process runs: the customised workspace tree the ledger was loaded from (the registry is not exported).
    captured = {}
    for item in sorted((w.fxc / 'workspace').rglob('*')):
        if item.is_file():
            mode = stat.S_IMODE(item.stat().st_mode)
            captured[item.relative_to(w.fxc / 'workspace').as_posix()] = (digest(item.read_bytes()), 0o755 if mode & 0o111 else 0o644)
    assert sorted(captured) == EXPECTED_WORKSPACE, sorted(captured)
    results = {}
    for label, exe in (('normal', NORMAL), ('variant', VARIANT)):
        cwd = w.root / ('export-' + label)
        cwd.mkdir()
        stdout = normal_output(w.run(exe, ['export', '--ledger', str(ledger), '--revision', '1', '--target', 'out'], cwd=cwd))
        results[label] = (stdout, tree(cwd / 'out'), cwd / 'out')
    assert results['normal'][0] == results['variant'][0], (results['normal'][0], results['variant'][0])
    assert results['normal'][1] == results['variant'][1], (sorted(results['normal'][1]), sorted(results['variant'][1]))
    exported = results['variant'][1]
    assert all(mode in (0o644, 0o755) for _, mode in exported.values()), exported
    assert normalised(exported) == captured, (sorted(normalised(exported)), sorted(captured))
    out = results['variant'][2]
    expected_dirs = {Path(name).parent.as_posix() for name in exported} - {'.'}
    actual_dirs = {item.relative_to(out).as_posix() for item in out.rglob('*') if item.is_dir()}
    assert actual_dirs == expected_dirs, (actual_dirs, expected_dirs)
    assert not (w.root / 'forbidden-legacy.db').exists()


def case_poison_is_live(w):
    shipped = (STDLIB / 'json.lua').read_bytes()
    assert (GENERATED / 'json.lua').read_bytes() != shipped
    gen = generation()
    assert gen['json_lua']['shipped_sha256'] != gen['json_lua']['generated_sha256']
    normal_output(w.run(NORMAL, RENDER, cwd=w.fx))
    assert_dead(w.run(VARIANT, RENDER, cwd=w.fx), 'render')
    assert not (w.root / 'forbidden-legacy.db').exists()


def case_poison_is_live_load(w):
    plain = w.fx / 'workspace/workspace.lua', w.fx / 'registry/quota-tools.json'
    ledger = w.init('embedded-normal')
    normal_output(w.run(NORMAL, load_args(ledger, plain[0], plain[1], 'embedded')))
    assert rows(ledger)[0] == 1, rows(ledger)
    ledger = w.init('embedded-variant')
    assert_dead(w.run(VARIANT, load_args(ledger, plain[0], plain[1], 'embedded')), 'embedded load')
    assert rows(ledger)[0] == 0, rows(ledger)
    custom = w.fxc / 'workspace/workspace.lua', w.fxc / 'registry/quota-tools.json'
    ledger = w.init('profile-normal')
    normal_output(w.run(NORMAL, load_args(ledger, custom[0], custom[1], 'custom', selector=True)))
    assert rows(ledger)[0] == 1, rows(ledger)
    # Repair note (X08 repair round): with-profile load is NOT poisonable through a generated variant. By the shipped design
    # (src/cmd/planar/invocation.cpp execute_profile_load, "Profile load always runs on the one real runtime that owns the
    # ledger's prepared_profile type, even in generated variants") it never reaches the variant's embedded Lua. The variant must
    # therefore accept it exactly as the normal binary does; the poison liveness for the profile path is proved by the
    # with-profile render case. DECISIONS row 2c(b) is amended accordingly.
    ledger = w.init('profile-variant')
    variant = normal_output(w.run(VARIANT, load_args(ledger, custom[0], custom[1], 'custom', selector=True)))
    assert json.loads(variant)['outcome'] == 'accepted' and rows(ledger)[0] == 1, (variant, rows(ledger))
    assert not (w.root / 'forbidden-legacy.db').exists()


def case_poison_is_live_profile_render(w):
    workspace, registry = w.fxc / 'workspace/workspace.lua', w.fxc / 'registry/quota-tools.json'
    args = SELECTOR + ['render', '--workspace', str(workspace), '--registry', str(registry),
                       '--root', 'source=' + str(w.fxc / 'current'), '--job', 'combined', '--role', 'builder']
    got = normal_output(w.run(NORMAL, args))
    assert b'custom workflow guidance' in got, got[:300]
    assert_dead(w.run(VARIANT, args), 'with-profile render')
    assert not (w.root / 'forbidden-legacy.db').exists()


def case_audit(_):
    gen = generation()
    # 1. Generated C++ equals the product C++ after the recorded reverse substitutions, file by file.
    assert sorted(gen['cpp']) == sorted(EXPECTED_CPP), sorted(gen['cpp'])
    for rel in EXPECTED_CPP:
        product = (PRODUCT_RUNTIME if rel.startswith('runtime.') else PRODUCT_CMD) / rel
        restored = (GENERATED / rel).read_text()
        for before, after in reversed(gen['cpp'][rel]['substitutions']):
            restored = restored.replace(after, before)
        expected = product.read_text()
        assert 'x08_' not in restored, rel
        assert restored == expected, (rel, list(difflib.unified_diff(expected.splitlines(), restored.splitlines(), lineterm=''))[:20])
    # 2. The embedded list is the shipped order with json.lua replaced, the poison inserted and bootstrap.lua replaced.
    assert [entry['logical'] for entry in gen['embedded']] == EXPECTED_EMBED, [e['logical'] for e in gen['embedded']]
    shipped_chunk = '\n'.join((STDLIB / name.removeprefix('planar/')).read_text()
                              for name in EXPECTED_EMBED if 'x08_poison' not in name)
    for name in EXPECTED_POISON:
        assert len(re.findall(rf'^local (?:function )?{name}\b', shipped_chunk, re.M)) == 1, name
    for entry in gen['embedded']:
        name = entry['logical'].removeprefix('planar/')
        path = Path(entry['path'])
        if name in ('json.lua', 'bootstrap.lua', 'x08_poison.lua'):
            assert path == GENERATED / name and entry['generated'] is True, entry
        else:
            assert path == STDLIB / name and entry['generated'] is False, entry
            assert path.read_bytes() == (STDLIB / name).read_bytes(), name
    # 3. json.lua: exactly one prepended comment line.
    shipped_json = (STDLIB / 'json.lua').read_text()
    generated_json = (GENERATED / 'json.lua').read_text()
    assert generated_json == JSON_COMMENT + shipped_json, 'json.lua is not the shipped file plus one comment line'
    assert changed(shipped_json.splitlines(True), generated_json.splitlines(True)) == [('insert', 0, 0, 0, 1)]
    assert gen['json_lua']['shipped_sha256'] == digest(shipped_json.encode())
    assert gen['json_lua']['generated_sha256'] == digest(generated_json.encode())
    # 4. bootstrap.lua: only the parse_json definition body differs.
    shipped_lines = (STDLIB / 'bootstrap.lua').read_text().splitlines(True)
    start = shipped_lines.index('local function parse_json(input)\n')
    end = next(i for i in range(start + 1, len(shipped_lines)) if shipped_lines[i] == 'end\n')
    generated_lines = (GENERATED / 'bootstrap.lua').read_text().splitlines(True)
    assert generated_lines == shipped_lines[:start + 1] + [PARSE_ERROR] + shipped_lines[end:], 'bootstrap.lua body changed'
    for tag, i1, i2, _, _ in changed(shipped_lines, generated_lines):
        assert start < i1 and i2 <= end, (tag, i1, i2)
    assert gen['bootstrap_lua']['shipped_sha256'] == digest(''.join(shipped_lines).encode())
    assert gen['bootstrap_lua']['generated_sha256'] == digest(''.join(generated_lines).encode())
    # 5. The poison chunk contains only the planned assignments, in the planned order.
    poison = (GENERATED / 'x08_poison.lua').read_text()
    assert poison.splitlines() == [f'{name} = function() error("X08 poisoned stdlib export: {name}",0) end'
                                   for name in EXPECTED_POISON], poison[:200]
    assert gen['poison']['names'] == EXPECTED_POISON, gen['poison']['names']
    assert gen['poison']['sha256'] == digest(poison.encode())


def case_sources_removed(w):
    ledger = custom_ledger(w)
    before = {}
    before['history'] = normal_output(w.run(NORMAL, ['history', '--ledger', str(ledger)]))
    before['revision'] = normal_output(w.run(NORMAL, ['history', '--ledger', str(ledger), '--revision', '1']))
    cwd = w.root / 'before'
    cwd.mkdir()
    before['export'] = normal_output(w.run(NORMAL, ['export', '--ledger', str(ledger), '--revision', '1', '--target', 'out'],
                                           cwd=cwd))
    before_tree = tree(cwd / 'out')
    assert sorted(before_tree) == EXPECTED_WORKSPACE, sorted(before_tree)
    shutil.rmtree(w.trusted)
    shutil.rmtree(w.fx)
    shutil.rmtree(w.fxc)
    after = {}
    after['history'] = normal_output(w.run(NORMAL, ['history', '--ledger', str(ledger)]))
    after['revision'] = normal_output(w.run(NORMAL, ['history', '--ledger', str(ledger), '--revision', '1']))
    cwd = w.root / 'after'
    cwd.mkdir()
    after['export'] = normal_output(w.run(NORMAL, ['export', '--ledger', str(ledger), '--revision', '1', '--target', 'out'],
                                          cwd=cwd))
    assert after == before, sorted(after)
    assert tree(cwd / 'out') == before_tree
    assert not (w.root / 'forbidden-legacy.db').exists()


FUNCTIONS = {'e12_variant_reads_identically': case_reads, 'e12_variant_exports_identically': case_exports,
             'e12_variant_poison_is_live': case_poison_is_live, 'e12_variant_poison_is_live_load': case_poison_is_live_load,
             'e12_variant_poison_is_live_profile_render': case_poison_is_live_profile_render, 'e12_variant_audit': case_audit,
             'e12_sources_and_profile_removed': case_sources_removed}
with tempfile.TemporaryDirectory(prefix='planar-x08-e12-') as temp:
    root = Path(temp).resolve()
    FUNCTIONS[CASE](None if CASE == 'e12_variant_audit' else World(root))
    assert not (root / 'forbidden-legacy.db').exists()
print(CASE, 'passed')
