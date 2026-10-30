"""X08 public-binary acceptance: discovery, envelope rows, closure separation, old goldens, W vectors, projection,
history_id and removed-input replay.

argv: <case> <planar binary>. Every case runs real processes in fresh scratch trees below one temporary directory. The
operator database, the home directory and the legacy PLANAR_DB path are never touched; the forbidden legacy path is
asserted absent. Machine output is compared as canonical JSON bytes. Success writes stdout only and failure stderr only.
An unexpected result on the unmutated tree is a product defect candidate: assertions are never weakened here.
"""
from pathlib import Path
import json, os, shutil, sqlite3, subprocess, sys, tempfile

CASES = ('e01_no_discovery', 'e01_registration_policy', 'e02_envelope_rows_binary', 'e03_closure_separation_binary',
         'e07_old_goldens_unselected_profile', 'e07_old_w_vectors_binary', 'e08_projection_vector_binary',
         'e09_history_id_binary', 'e10_removed_profile_replay_binary')
if len(sys.argv) < 2 or sys.argv[1] not in CASES:
    raise SystemExit('unknown case ' + (sys.argv[1] if len(sys.argv) > 1 else ''))
if len(sys.argv) != 3:
    raise SystemExit('usage: x08_cli.py <case> <planar binary>')
CASE = sys.argv[1]
PLANAR = str(Path(sys.argv[2]).resolve())
REPO = Path(__file__).resolve().parents[2]
FIXTURE = REPO / 'fixtures/render/quota'
TREES = []


def canonical(value):
    return json.dumps(value, sort_keys=True, separators=(',', ':'), ensure_ascii=False).encode() + b'\n'


def error_bytes(code, message):
    return canonical({'error': {'code': code, 'field': '', 'message': message}, 'kind': 'error', 'version': 1})


def summary(command, text):
    return canonical({'command': command, 'data': {'text': text, 'type': 'summary'}, 'kind': 'extension', 'version': 1})


USAGE = error_bytes('usage', 'Expected command options or --help; see --help')
UNSAFE = error_bytes('unsafe_path', 'External profile overlaps a supplied input')
REFERENCE = error_bytes('reference', 'External registration identity conflict')
INTERNAL = error_bytes('internal', 'Invalid external profile protocol')
LEDGER_NOT_FOUND = error_bytes('ledger_not_found', 'Ledger was not found')
REQUEST_CONFLICT = error_bytes('request_conflict', 'Ledger request conflicts with its saved intent')
HELP_HEADER = b'Usage: planar with-profile --stdlib-profile FILE COMMAND [OPTIONS]\n\nCommands:\n'
INSPECT_SUMMARY = summary('inspect', 'inspect')
EVIL = "error('X08-STDLIB-D-RAN')\n"
PROFILE_P = 'with-profile --stdlib-profile ../trusted/p.json'


def tok(line):
    """Split a table cell on single spaces; the token "" is the empty string (x07-addendum section 2.1)."""
    if line == '':
        return []
    return ['' if piece == '""' else piece for piece in line.split(' ')]


def write(path, text):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(text if isinstance(text, bytes) else text.encode())


class Tree:
    """One scratch tree: root/{home,trusted,work,fx}. work is the invocation cwd and trusted is its sibling."""

    def __init__(self, root):
        self.root = Path(root).resolve()
        self.home = self.root / 'home'
        self.trusted = self.root / 'trusted'
        self.work = self.root / 'work'
        self.fx = self.root / 'fx'
        self.forbidden = self.root / 'forbidden-legacy.db'
        for directory in (self.home, self.trusted / 'modules', self.work):
            directory.mkdir(parents=True, exist_ok=True)
        TREES.append(self)

    def env(self, extra=None):
        env = {k: v for k, v in os.environ.items() if not k.startswith(('PLANAR_', 'X07_', 'X08_'))}
        env.update(HOME=str(self.home), PLANAR_DB=str(self.forbidden))
        env.update(extra or {})
        return env

    def run(self, args, cwd=None, env=None):
        return subprocess.run([PLANAR, *args], capture_output=True, env=env if env is not None else self.env(),
                              cwd=str(cwd or self.work), timeout=60)

    def copy_fixture(self):
        shutil.copytree(FIXTURE, self.fx)

    def workspace(self):
        return str(self.fx / 'workspace/workspace.lua')

    def registry(self):
        return str(self.fx / 'registry/quota-tools.json')

    def load_args(self, ledger, key, actor='driver', ws=None):
        return ['load', '--ledger', ledger, '--workspace', ws or self.workspace(), '--registry', self.registry(),
                '--expect-revision', 'none', '--request-id', key, '--actor', actor]


def rows(ledger):
    with sqlite3.connect(ledger) as db:
        return tuple(db.execute('SELECT COUNT(*) FROM ' + n).fetchone()[0] for n in ('revisions', 'requests', 'events'))


def assert_no_forbidden():
    for tree in TREES:
        assert not tree.forbidden.exists(), ('legacy database path was created', tree.forbidden.name)


def scratch_free(*outputs):
    for tree in TREES:
        for text in outputs:
            assert str(tree.root).encode() not in text, 'scratch path leaked into output'


# Lua source for the trusted modules. Each descriptor follows the in-process fixtures in bootstrap.t.cpp.
def opt(name, kind, required=False):
    return "{name='--%s',type='%s',required=%s,help='o'}" % (name, kind, 'true' if required else 'false')


def command(name, options, capabilities, body):
    return ("registration.add({name='%s',family='extension_read_v1',handler_id='custom.%s',options=engine.array{%s},"
            "help='h',response_kind='extension_v1',capabilities=engine.array{%s}},function(inv,ctx) %s end)\n"
            % (name, name, ','.join(sorted(options)), capabilities, body))


def inspect_module(name='inspect'):
    options = ("{name='--text',type='text',required=true,help='text'},"
               "{name='--flag',type='flag',required=false,help='flag'}") if name == 'inspect' else ''
    return ("local D={name='%s',family='extension_read_v1',handler_id='custom.%s',options=engine.array{%s},help='%s',"
            "response_kind='extension_v1',capabilities=engine.array{}}\n"
            "registration.add(D,function(inv) return {ok=true,value={version=1,kind='extension',command=inv.command,"
            "data={type='summary',text=D.help}}} end)\n" % (name, name, options, name))


def role_module(body):
    options = [opt('workspace', 'text', True), opt('registry', 'text', True), opt('root', 'text', True),
               opt('job', 'text', True), opt('role', 'text', True), opt('base-root', 'text'), opt('observations', 'text'),
               opt('budget-bytes', 'decimal'), opt('capture-outputs', 'flag'), opt('text', 'text')]
    return command('role', options, "'role_context_read_v1'", body)


HISTORY_OPTIONS = [opt('after-event', 'history_id'), opt('at-event', 'history_id'), opt('ledger', 'text', True),
                   opt('limit', 'decimal'), opt('request-id', 'text'), opt('revision', 'history_id')]


def reporter_module():
    body = ("local o,p=inv.options,inv.present local names={'revision','at-event','after-event'} local t={} "
            "for i=1,3 do local n=names[i] t[#t+1]=n..'='..type(o[n])..':'..o[n]..':'..(p[n] and 'true' or 'false') end "
            "t[#t+1]='limit='..math.type(o.limit)..':'..o.limit..':'..(p.limit and 'true' or 'false') "
            "local s='' for i=1,#t do s=s..(i>1 and ';' or '')..t[i] end "
            "return {ok=false,error={code='schema',message=s,field=''}}")
    return command('hist', HISTORY_OPTIONS, "'history_read_v1'", body)


def decimal_module():
    body = ("return {ok=true,value={version=1,kind='extension',command=inv.command,data={type='summary',"
            "text=math.type(inv.options.count)..':'..inv.options.count}}}")
    return command('dec', [opt('count', 'decimal')], '', body)


def reporter_message(ids, limit):
    out = ''
    for name in ('revision', 'at-event', 'after-event'):
        present = name in ids
        out += '%s=string:%s:%s;' % (name, ids[name] if present else '0', 'true' if present else 'false')
    out += 'limit=integer:%s:%s' % (limit or '0', 'true' if limit else 'false')
    return out


def schema_bytes(message):
    return error_bytes('schema', message)


def lua_repeat(name, unit, count):
    n = name
    return (f"local {n},n_{n},u_{n}='',{count},'{unit}' while n_{n}>0 do if n_{n}%2==1 then {n}={n}..u_{n} end "
            f"n_{n}=n_{n}//2 if n_{n}>0 then u_{n}=u_{n}..u_{n} end end ")


def projection_module(sections, prelude=''):
    body = ("ctx.read_history({}) local p=ctx.project({id='p'}) return {ok=true,value={version=1,kind='extension',"
            "command=inv.command,data=p}}")
    return (command('hproj', HISTORY_OPTIONS, "'history_read_v1','projection_apply_v1'", body)
            + "registration.projection({id='p',abi='projection_v1',revision='final',input='history_snapshot_v1',help='p'},"
              "function(input) %s return {version=1,sections=engine.array{%s}} end)\n" % (prelude, sections))


def profile_json(modules):
    return canonical({'version': 1, 'modules': modules}).decode()


def valid_trusted(t):
    write(t.trusted / 'p.json', profile_json(['modules/p.lua']))
    write(t.trusted / 'modules/p.lua', inspect_module('inspect'))
    write(t.trusted / 'q.json', profile_json(['modules/q.lua']))
    write(t.trusted / 'modules/q.lua', inspect_module('other'))


def quota_ledger(t, ledger, key='k1', actor='driver'):
    made = t.run(['init', '--ledger', ledger, '--workspace-id', 'quota'])
    assert made.returncode == 0 and not made.stderr, made
    loaded = t.run(t.load_args(ledger, key, actor))
    assert loaded.returncode == 0 and not loaded.stderr, loaded
    return loaded


# ---------------------------------------------------------------------------------------------------------------------
# E01: no discovery from the environment or from stdlib.d without with-profile; root flag is usage.

def case_e01_no_discovery():
    with tempfile.TemporaryDirectory(prefix='planar-x08-') as temp:
        t = Tree(temp)
        valid_trusted(t)
        write(t.work / 'p.json', profile_json(['modules/p.lua']))
        write(t.work / 'modules/p.lua', inspect_module('inspect'))
        write(t.work / 'stdlib.d/evil.lua', EVIL)
        write(t.home / 'stdlib.d/evil.lua', EVIL)
        env = t.env({'PLANAR_STDLIB_PROFILE': str(t.work / 'p.json'), 'PLANAR_PROFILE': str(t.work / 'p.json'),
                     'PLANAR_STDLIB_D': str(t.work / 'stdlib.d')})
        bare = t.run(['inspect', '--text', 'value'], env=env)
        assert (bare.returncode, bare.stdout, bare.stderr) == (2, b'', USAGE), bare
        root_flag = t.run(['--stdlib-profile', '../trusted/p.json', 'inspect', '--text', 'value'], env=env)
        assert (root_flag.returncode, root_flag.stdout, root_flag.stderr) == (2, b'', USAGE), root_flag
        selected = t.run(['with-profile', '--stdlib-profile', '../trusted/p.json', 'inspect', '--text', 'value'], env=env)
        assert (selected.returncode, selected.stdout, selected.stderr) == (0, INSPECT_SUMMARY, b''), selected
        assert b'X08' not in selected.stdout + selected.stderr
        scratch_free(bare.stderr, root_flag.stderr, selected.stdout)
    assert_no_forbidden()
    print(CASE, 'passed')


# ---------------------------------------------------------------------------------------------------------------------
# E01 and X-POL: registration policy refusals through the binary, and explicit stdlib.d selection only.

OVERRIDE_MODULE = ("local D={name='history',family='extension_read_v1',handler_id='custom.history',options=engine.array{},"
                   "help='history',response_kind='extension_v1',capabilities=engine.array{}}\n"
                   "registration.add(D,function(inv) return {ok=true,value={version=1,kind='extension',command=inv.command,"
                   "data={type='summary',text='x'}}} end)\n")
DUPLICATE_MODULE = ("local D={name='inspect',family='extension_read_v1',handler_id='custom.inspect',options=engine.array{},"
                    "help='inspect',response_kind='extension_v1',capabilities=engine.array{}}\n"
                    "registration.add(D,function(inv) return {ok=true,value={version=1,kind='extension',command=inv.command,"
                    "data={type='summary',text='x'}}} end)\n"
                    "registration.add(D,function(inv) return {ok=true,value={version=1,kind='extension',command=inv.command,"
                    "data={type='summary',text='x'}}} end)\n")
ALIAS_MODULE = ("local D={name='inspect',family='extension_read_v1',handler_id='custom.inspect',options=engine.array{},"
                "help='inspect',response_kind='extension_v1',capabilities=engine.array{},alias='zz'}\n"
                "registration.add(D,function(inv) return {ok=true,value={version=1,kind='extension',command=inv.command,"
                "data={type='summary',text='x'}}} end)\n")
FOO_MODULE = ("local D={name='foo',family='extension_read_v1',handler_id='custom.foo',options=engine.array{},help='foo',"
              "response_kind='extension_v1',capabilities=engine.array{}}\n"
              "registration.add(D,function(inv) return {ok=true,value={version=1,kind='extension',command=inv.command,"
              "data={type='summary',text=D.help}}} end)\n")


def case_e01_registration_policy():
    with tempfile.TemporaryDirectory(prefix='planar-x08-') as temp:
        t = Tree(temp)
        t.copy_fixture()
        valid_trusted(t)
        for name, module in (('over', OVERRIDE_MODULE), ('dup', DUPLICATE_MODULE), ('alias', ALIAS_MODULE)):
            write(t.trusted / (name + '.json'), profile_json(['modules/%s.lua' % name]))
            write(t.trusted / ('modules/%s.lua' % name), module)
        write(t.trusted / 'sd.json', profile_json(['stdlib.d/foo.lua']))
        write(t.trusted / 'stdlib.d/foo.lua', FOO_MODULE)
        write(t.work / 'stdlib.d/evil.lua', EVIL)
        write(t.home / 'stdlib.d/evil.lua', EVIL)
        assert t.run(['init', '--ledger', 'l.db', '--workspace-id', 'quota']).returncode == 0
        before = rows(t.work / 'l.db')

        override = t.run(['with-profile', '--stdlib-profile', '../trusted/over.json', 'history', '--ledger', 'l.db'])
        assert (override.returncode, override.stdout, override.stderr) == (3, b'', REFERENCE), override
        assert rows(t.work / 'l.db') == before

        duplicate = t.run(['with-profile', '--stdlib-profile', '../trusted/dup.json', 'inspect', '--text', 'v'])
        assert (duplicate.returncode, duplicate.stdout, duplicate.stderr) == (3, b'', REFERENCE), duplicate
        assert rows(t.work / 'l.db') == before

        alias = t.run(['with-profile', '--stdlib-profile', '../trusted/alias.json', 'inspect', '--text', 'v'])
        assert (alias.returncode, alias.stdout, alias.stderr) == (4, b'', INTERNAL), alias
        assert rows(t.work / 'l.db') == before

        # A job naming a custom shape without a selected profile refuses at job.shape, as the x07 workflow control does.
        for name in ('split.lua', 'combined.lua'):
            path = t.fx / 'workspace' / name
            text = path.read_text()
            assert 'fanout_review' in text, name
            path.write_text(text.replace('fanout_review', 'custom'))
        shaped = t.run(t.load_args('l.db', 'shape', ws=t.workspace()))
        assert shaped.returncode == 3 and shaped.stdout == b'', shaped
        refusal = json.loads(shaped.stderr)['error']
        assert refusal['code'] == 'schema' and refusal['field'] == 'job.shape', refusal
        assert rows(t.work / 'l.db') == before

        # An explicitly selected profile registers stdlib.d/foo.lua; the unselected stdlib.d trees are never read.
        env = t.env({'PLANAR_STDLIB_D': str(t.work / 'stdlib.d'), 'PLANAR_STDLIB_PROFILE': str(t.trusted / 'sd.json')})
        selected = t.run(['with-profile', '--stdlib-profile', '../trusted/sd.json', 'foo'], env=env)
        assert (selected.returncode, selected.stdout, selected.stderr) == (0, summary('foo', 'foo'), b''), selected
        unselected = t.run(['foo'], env=env)
        assert (unselected.returncode, unselected.stdout, unselected.stderr) == (2, b'', USAGE), unselected
        other = t.run(['with-profile', '--stdlib-profile', '../trusted/p.json', 'foo'], env=env)
        assert (other.returncode, other.stdout, other.stderr) == (2, b'', USAGE), other
        assert b'X08-STDLIB-D-RAN' not in selected.stdout + selected.stderr + other.stderr
        assert rows(t.work / 'l.db') == before
    assert_no_forbidden()
    print(CASE, 'passed')


# ---------------------------------------------------------------------------------------------------------------------
# E02: the 44 production rows and the 12 opaque rows, each run with and without the trusted directory.

PROD_ROWS = [
    ('E01', PROFILE_P + ' inspect --text value --flag', 0, 'HND'),
    ('E02', PROFILE_P + ' history --ledger missing', 3, 'KSA'),
    ('E03', 'with-profile inspect --text value', 2, 'PFX'),
    ('E04', 'with-profile --stdlib-profile', 2, 'PFX'),
    ('E05', 'with-profile --stdlib-profile "" inspect --text value', 2, 'PFX'),
    ('E06', 'with-profile --stdlib-profile inspect --text value', 2, 'PRB'),
    ('E07', 'with-profile --stdlib-profile ../trusted/p.json --stdlib-profile ../trusted/q.json inspect --text v', 2, 'PFX'),
    ('E08', 'with-profile --stdlib-profile --bogus inspect --text v', 2, 'PFX'),
    ('E09', 'with-profile --stdlib-profile --help inspect --text v', 2, 'PFX'),
    ('E10', 'with-profile --stdlib-profile --help', 2, 'PFX'),
    ('E11', PROFILE_P + ' --bogus inspect --text v', 2, 'PRB'),
    ('E12', 'with-profile --bogus --stdlib-profile ../trusted/p.json inspect --text v', 2, 'PFX'),
    ('E13', PROFILE_P + ' absent --text v', 2, 'FIN'),
    ('E14', PROFILE_P + ' inspect --text --stdlib-profile', 2, 'FIN'),
    ('E15', PROFILE_P + ' inspect --stdlib-profile ../trusted/q.json --text v', 2, 'FIN'),
    ('E16', PROFILE_P + ' inspect --text --bogus', 2, 'FIN'),
    ('E17', PROFILE_P + ' inspect --text --flag', 2, 'FIN'),
    ('E18', PROFILE_P, 2, 'PFX'),
    ('E19', PROFILE_P + ' -- inspect --text v', 2, 'PRB'),
    ('E20', PROFILE_P + ' inspect --text v --', 2, 'FIN'),
    ('E21', 'with-profile --stdlib-profile -- inspect --text v', 2, 'PFX'),
    ('E22', PROFILE_P + ' --help', 0, 'HLP'),
    ('E23', 'with-profile --help --stdlib-profile ../trusted/p.json', 0, 'HLP'),
    ('E24', PROFILE_P + ' --help inspect --text v', 2, 'PFX'),
    ('E25', PROFILE_P + ' inspect --help', 2, 'FIN'),
    ('E26', PROFILE_P + ' --help --help', 2, 'PFX'),
    ('E27', PROFILE_P + ' history', 2, 'PRB'),
    ('E28', PROFILE_P + ' history --ledger a --ledger b', 2, 'PRB'),
    ('E29', PROFILE_P + ' history --ledger a --bogus b', 2, 'PRB'),
    ('E30', PROFILE_P + ' history --ledger a --', 2, 'PRB'),
    ('E31', 'with-profile --stdlib-profile=../trusted/p.json inspect --text v', 2, 'SYN'),
    ('E32', PROFILE_P + ' inspect --text=v', 2, 'SYN'),
    ('E33', '-- ' + PROFILE_P + ' inspect --text v', 2, 'PFX'),
    ('E34', PROFILE_P + ' init --ledger a --workspace-id w', 0, 'HND'),
    ('E35', PROFILE_P + ' export --ledger a --revision 1 --target out', 3, 'KSA'),
    ('E36', PROFILE_P + ' load --ledger a --workspace w --registry r --expect-revision none --request-id k --actor a', 2, 'NRM'),
    ('E37', PROFILE_P + ' render --workspace w --registry r --root s=src --job j --role r', 2, 'NRM'),
    ('E38', PROFILE_P + ' init --ledger a', 2, 'PRB'),
    ('E39', PROFILE_P + ' export --ledger a --revision bad --target out', 2, 'PRB'),
    ('E40', PROFILE_P + ' load --ledger a', 2, 'PRB'),
    ('E41', PROFILE_P + ' render --workspace w', 2, 'PRB'),
    ('E42', PROFILE_P + ' --help --', 2, 'PFX'),
    ('E43', PROFILE_P + ' inspect --text ""', 2, 'FIN'),
    ('E44', PROFILE_P + ' history --ledger a inspect --text v', 2, 'PRB'),
]
OPAQUE_ROWS = [
    ('O01', 'inspect --text v --flag', 0, 'HND'),
    ('O02', 'inspect history --text --stdlib-profile', 2, 'FIN'),
    ('O03', 'absent --unknown -- history', 2, 'FIN'),
    ('O04', '--bogus inspect --text v', 2, 'PRB'),
    ('O05', '--help', 0, 'HLP'),
    ('O06', '-- inspect --text v', 2, 'PRB'),
    ('O07', 'history --ledger a', 3, 'KSA'),
    ('O08', 'history', 2, 'PRB'),
    ('O09', 'history --ledger a --bogus b', 2, 'PRB'),
    ('O10', 'history --ledger a --', 2, 'PRB'),
    ('O11', 'history --ledger a inspect --text v', 2, 'PRB'),
    ('O12', '', 2, 'PFX'),
]
NOT_ACCESSED = {'SYN', 'PFX', 'PRB', 'NRM', 'ERL'}
ACCESSED = {'KSA', 'PRF', 'FIN', 'LAT', 'HND', 'HLP'}


def envelope_tree(root):
    t = Tree(root)
    valid_trusted(t)
    return t


def mask_ledger(text):
    value = json.loads(text)
    value['ledger'] = 'x' * 32
    return canonical(value)


def case_e02_envelope_rows_binary():
    assert len(PROD_ROWS) == 44 and len(OPAQUE_ROWS) == 12, (len(PROD_ROWS), len(OPAQUE_ROWS))
    with tempfile.TemporaryDirectory(prefix='planar-x08-') as temp:
        outer = Path(temp)
        help_outputs = {}
        table = [(rid, line, code, cls) for rid, line, code, cls in PROD_ROWS]
        table += [(rid, (PROFILE_P + ' ' + tail).strip(), code, cls) for rid, tail, code, cls in OPAQUE_ROWS]
        for rid, line, code, cls in table:
            argv = tok(line)
            a_tree = envelope_tree(outer / (rid + '-a'))
            a = a_tree.run(argv)
            assert a.returncode == code, (rid, line, a.returncode, a.stderr[:300])
            if code == 2:
                assert (a.stdout, a.stderr) == (b'', USAGE), (rid, a.stdout[:200], a.stderr[:300])
            elif cls == 'KSA':
                assert (a.stdout, a.stderr) == (b'', LEDGER_NOT_FOUND), (rid, a.stdout[:200], a.stderr[:300])
            elif cls == 'HLP':
                assert a.stdout.startswith(HELP_HEADER) and a.stderr == b'', (rid, a.stderr[:300])
                help_outputs[rid] = a.stdout
            elif rid == 'E01' or rid == 'O01':
                assert (a.stdout, a.stderr) == (INSPECT_SUMMARY, b''), (rid, a.stdout[:200], a.stderr[:300])
            elif rid == 'E34':
                assert a.stderr == b'', a.stderr[:300]
                assert (a_tree.work / 'a').exists(), 'init did not create the target'
                control = a_tree.run(['init', '--ledger', 'plain', '--workspace-id', 'w'])
                assert control.returncode == 0 and mask_ledger(a.stdout.decode()) == mask_ledger(control.stdout.decode()), \
                    (a.stdout[:200], control.stdout[:200])
            assert str(a_tree.root).encode() not in a.stdout + a.stderr, rid

            b_tree = envelope_tree(outer / (rid + '-b'))
            shutil.rmtree(b_tree.trusted)
            b = b_tree.run(argv)
            if cls in NOT_ACCESSED:
                assert (b.returncode, b.stdout, b.stderr) == (a.returncode, a.stdout, a.stderr), (rid, line, b.stderr[:300])
            else:
                assert cls in ACCESSED, (rid, cls)
                assert (b.returncode, b.stdout, b.stderr) != (a.returncode, a.stdout, a.stderr), (rid, line)
            if rid == 'E34':
                assert not (b_tree.work / 'a').exists(), 'a row without the profile created the target'
        assert help_outputs['E22'] == help_outputs['E23'] == help_outputs['O05']
        # X08 C01 survivor I02: a duplicate --stdlib-profile is a usage error whether the values differ, are identical, or the
        # request is help. The multi_option_policy Throw (envelope.cpp) and the occurrence count guard in classify_prefix both
        # refuse these, so the Throw to TakeLast mutant is behaviourally equivalent; this block pins the observable policy.
        dup = envelope_tree(outer / 'dup')
        for tail in ('inspect --text v', '--help', 'init --ledger plain --workspace-id w'):
            for pair in ('../trusted/p.json ../trusted/p.json', '../trusted/p.json ../trusted/q.json',
                         '../trusted/q.json ../trusted/p.json'):
                first, second = pair.split()
                got = dup.run(tok('with-profile --stdlib-profile %s --stdlib-profile %s %s' % (first, second, tail)))
                assert (got.returncode, got.stdout, got.stderr) == (2, b'', USAGE), (pair, tail, got.stderr[:300])
        assert not (dup.work / 'plain').exists()
    assert_no_forbidden()
    print(CASE, 'passed')


# ---------------------------------------------------------------------------------------------------------------------
# E03: closure separation. Overlap, ledger alias and late root refusals precede registration and the handler.

def closure_tree(root):
    t = Tree(root)
    t.copy_fixture()
    valid_trusted(t)
    write(t.trusted / 'chunk.json', profile_json(['modules/chunk.lua']))
    write(t.trusted / 'modules/chunk.lua', "error('X08-CHUNK-RAN')\n")
    write(t.trusted / 'workspace.lua', '-- placeholder\n')
    shutil.copytree(t.fx / 'current', t.trusted / 'current')
    (t.work / 'src').mkdir()
    return t


def case_e03_closure_separation_binary():
    with tempfile.TemporaryDirectory(prefix='planar-x08-') as temp:
        t = closure_tree(Path(temp) / 'a')
        render = ['render', '--workspace', None, '--registry', t.registry(), '--root', None, '--job', 'combined',
                  '--role', 'builder']
        # (a) a render declaration or root inside, equal to, or containing the trusted tree is unsafe_path before
        # registration; the chunk that would raise X08-CHUNK-RAN never runs.
        overlaps = [(str(t.trusted / 'workspace.lua'), str(t.fx / 'current')),
                    (t.workspace(), str(t.trusted / 'current')),
                    (t.workspace(), str(t.trusted)),
                    (t.workspace(), str(t.root))]
        for workspace, root in overlaps:
            args = list(render)
            args[2] = workspace
            args[6] = 'source=' + root
            got = t.run(['with-profile', '--stdlib-profile', '../trusted/chunk.json', *args])
            assert (got.returncode, got.stdout, got.stderr) == (3, b'', UNSAFE), (workspace, root, got)
            assert b'X08-CHUNK-RAN' not in got.stdout + got.stderr
        # (b) a ledger that is a hard link of the profile or of a module refuses for history and export.
        assert t.run(['init', '--ledger', 'seed.db', '--workspace-id', 'quota']).returncode == 0
        os.link(t.trusted / 'modules/p.lua', t.work / 'alias-module.db')
        os.link(t.trusted / 'p.json', t.work / 'alias-profile.db')
        for args in (['history', '--ledger', 'alias-module.db'], ['history', '--ledger', 'alias-profile.db'],
                     ['export', '--ledger', 'alias-module.db', '--revision', '1', '--target', 'out']):
            got = t.run(['with-profile', '--stdlib-profile', '../trusted/p.json', *args])
            assert (got.returncode, got.stdout, got.stderr) == (3, b'', UNSAFE), (args, got)
        assert not (t.work / 'out').exists()
        # (c) an extension verb whose late workspace slot overlaps the trusted tree is unsafe_path after the final
        # parse and before the handler; the handler body would report X08-HANDLER-RAN.
        write(t.trusted / 'role.json', profile_json(['modules/role.lua']))
        write(t.trusted / 'modules/role.lua',
              role_module("return {ok=false,error={code='schema',message='X08-HANDLER-RAN',field=''}}"))
        role = ['role', '--workspace', None, '--registry', t.registry(), '--root', 'source=' + str(t.work / 'src'),
                '--job', 'combined', '--role', 'builder']
        live = list(role)
        live[2] = t.workspace()
        control = t.run(['with-profile', '--stdlib-profile', '../trusted/role.json', *live])
        assert (control.returncode, control.stdout) == (3, b'') and b'X08-HANDLER-RAN' in control.stderr, control
        overlap = list(role)
        overlap[2] = str(t.trusted / 'workspace.lua')
        got = t.run(['with-profile', '--stdlib-profile', '../trusted/role.json', *overlap])
        assert (got.returncode, got.stdout, got.stderr) == (3, b'', UNSAFE), got
        assert b'X08-HANDLER-RAN' not in got.stdout + got.stderr
        # (d) disjoint trees succeed: the same render under a valid profile equals the ordinary render, and history equals.
        plain_args = ['render', '--workspace', t.workspace(), '--registry', t.registry(),
                      '--root', 'source=' + str(t.fx / 'current'), '--job', 'combined', '--role', 'builder',
                      '--capture-outputs', '--base-root', 'source=' + str(t.fx / 'base'),
                      '--observations', str(t.fx / 'observations/empty.json')]
        plain = t.run(plain_args)
        assert plain.returncode == 0 and not plain.stderr, plain
        profiled = t.run(['with-profile', '--stdlib-profile', '../trusted/p.json', *plain_args])
        assert (profiled.returncode, profiled.stdout, profiled.stderr) == (0, plain.stdout, b''), profiled.stderr[:300]
        quota_ledger(t, 'loaded.db')
        history = ['history', '--ledger', 'loaded.db', '--revision', '1']
        ordinary = t.run(history)
        assert ordinary.returncode == 0, ordinary
        under = t.run(['with-profile', '--stdlib-profile', '../trusted/p.json', *history])
        assert (under.returncode, under.stdout, under.stderr) == (0, ordinary.stdout, b''), under
        scratch_free(control.stderr, got.stderr)
    assert_no_forbidden()
    print(CASE, 'passed')


# ---------------------------------------------------------------------------------------------------------------------
# E07: the six old quota packets with an unselected profile in cwd and with an empty selected profile.

JOBS = [('combined-builder', 'combined', 'builder', 'observations/empty.json'),
        ('combined-reviewer', 'combined', 'reviewer', 'reviewer-resolved.json'),
        ('split-library_worker', 'split', 'library_worker', 'observations/empty.json'),
        ('split-cli_worker', 'split', 'cli_worker', 'observations/empty.json'),
        ('split-reviewer', 'split', 'reviewer', 'reviewer-resolved.json'),
        ('split-cli_worker-recovery', 'split', 'cli_worker', 'observations/recovery.json')]


def render_args(job, role, observations):
    return ['render', '--workspace', 'workspace/workspace.lua', '--registry', 'registry/quota-tools.json',
            '--root', 'source=current', '--job', job, '--role', role, '--capture-outputs', '--base-root', 'source=base',
            '--observations', observations]


def golden_tree(root):
    t = Tree(root)
    t.copy_fixture()
    shutil.copy(FIXTURE / 'native/reviewer-resolved.json', t.fx / 'reviewer-resolved.json')
    write(t.fx / 'p.json', profile_json([]))
    write(t.fx / 'stdlib.d/evil.lua', EVIL)
    write(t.trusted / 'e.json', profile_json([]))
    return t


def unselected_env(t):
    return t.env({'PLANAR_STDLIB_PROFILE': str(t.fx / 'p.json'), 'PLANAR_PROFILE': str(t.fx / 'p.json'),
                  'PLANAR_STDLIB_D': str(t.fx / 'stdlib.d')})


def prose(value):
    W = value['manifest']['work']['value']
    out = [W['goal']['bytes'], *[u['summary']['bytes'] for u in W['units']]]
    out += [r['text']['bytes'] for k in ['constraints', 'prescriptions'] for r in W[k]]
    out += [c['text']['bytes'] for m in W['milestones'] for c in m['acceptance']]
    out += [c['declaration'][k]['bytes'] for c in W['contracts'] for k in ['expected', 'review_note']]
    return out


def case_e07_old_goldens_unselected_profile():
    with tempfile.TemporaryDirectory(prefix='planar-x08-') as temp:
        t = golden_tree(Path(temp) / 'g')
        env = unselected_env(t)
        native = FIXTURE / 'native'
        for name, job, role, observations in JOBS:
            args = render_args(job, role, observations)
            plain = t.run(args, cwd=t.fx, env=env)
            assert plain.returncode == 0 and plain.stderr == b'', (name, plain.stderr[:300])
            assert plain.stdout == (native / (name + '.json')).read_bytes(), name
            value = json.loads(plain.stdout)
            markdown = value['packet']['markdown']
            assert markdown.encode() == (native / (name + '.md')).read_bytes(), name
            original = (FIXTURE / 'expected' / (name + '.md')).read_text()
            for text in prose(value):
                assert text in markdown and text in original, (name, text[:80])
            empty = t.run(['with-profile', '--stdlib-profile', '../trusted/e.json', *args], cwd=t.fx, env=env)
            assert (empty.returncode, empty.stdout, empty.stderr) == (0, plain.stdout, b''), (name, empty.stderr[:300])
        scratch_free(plain.stdout)
    assert_no_forbidden()
    print(CASE, 'passed')


# ---------------------------------------------------------------------------------------------------------------------
# E07 W: manifest.work equality across render routes, and a history comparison across embedded and profile loads.

def diffs(x, y, path=''):
    out = []
    if isinstance(x, dict) and isinstance(y, dict):
        for key in sorted(set(x) | set(y)):
            here = path + '/' + key
            if key not in x or key not in y:
                out.append(here)
            else:
                out += diffs(x[key], y[key], here)
    elif isinstance(x, list) and isinstance(y, list):
        if len(x) != len(y):
            out.append(path + '#length')
        for index, (u, v) in enumerate(zip(x, y)):
            out += diffs(u, v, '%s[%d]' % (path, index))
    elif x != y:
        out.append(path)
    return out


# History fields that a profile acceptance records differently (external-profile-contract.md:614-630: V and E are
# interpretation_v2/versions_v2 for every profile acceptance, stdlib_digest is the effective tree, and accepted_at is
# wall clock). Record-ID fields point at the differing interpretation, stdlib and versions records. ledger is the
# per-init identity. Everything else must match, including the whole bundle and semantic model.
JUSTIFIED = [
    ('/ledger', 'per-init ledger identity'),
    ('/data/revision/accepted_at', 'wall-clock acceptance time'),
    ('/data/interpretation/version', 'profile acceptance records interpretation_v2 (external-profile-contract.md:614-630)'),
    ('/data/interpretation/versions', 'versions_v2 for profile acceptance (external-profile-contract.md:614-630)'),
    ('/data/interpretation/extensions', 'profile E catalog and selected shapes (external-profile-contract.md:614-630)'),
    ('/data/versions', 'revision-level copy of versions_v2 (external-profile-contract.md:614-630)'),
    ('/data/revision/interpretation', 'record ID of the differing interpretation record (ledger-contract.md:91-100)'),
    ('/data/revision/stdlib', 'record ID of the effective stdlib tree (effective_stdlib_v1)'),
    ('/data/revision/versions', 'record ID of the differing versions record (ledger-contract.md:91-100)'),
]


def key_paths(value, path=''):
    out = []
    if isinstance(value, dict):
        for key in sorted(value):
            out.append(path + '/' + key)
            out += key_paths(value[key], path + '/' + key)
    elif isinstance(value, list):
        for item in value:
            out += key_paths(item, path)
    return out


def justified(path):
    return any(path == prefix or path.startswith(prefix + '/') or path.startswith(prefix + '#') for prefix, _ in JUSTIFIED)


def case_e07_old_w_vectors_binary():
    with tempfile.TemporaryDirectory(prefix='planar-x08-') as temp:
        t = golden_tree(Path(temp) / 'w')
        env = unselected_env(t)
        native = FIXTURE / 'native'
        for name, job, role, observations in JOBS:
            args = render_args(job, role, observations)
            plain = t.run(args, cwd=t.fx, env=env)
            empty = t.run(['with-profile', '--stdlib-profile', '../trusted/e.json', *args], cwd=t.fx, env=env)
            golden = json.loads((native / (name + '.json')).read_bytes())
            assert plain.returncode == 0 and empty.returncode == 0, (name, plain.stderr[:200], empty.stderr[:200])
            work = [json.loads(plain.stdout)['manifest']['work'], json.loads(empty.stdout)['manifest']['work'],
                    golden['manifest']['work']]
            for field in ('value', 'digest'):
                assert work[0][field] == work[1][field] == work[2][field], (name, field)
            assert work[0]['digest'].startswith('xxh3_128:'), name
        # Two fresh ledgers: embedded load and empty-profile load of the same request.
        t.run(['init', '--ledger', 'a1.db', '--workspace-id', 'quota'], env=env)
        t.run(['init', '--ledger', 'a2.db', '--workspace-id', 'quota'], env=env)
        embedded = t.run(t.load_args('a1.db', 'k1'), env=env)
        profiled = t.run(['with-profile', '--stdlib-profile', '../trusted/e.json', *t.load_args('a2.db', 'k1')], env=env)
        assert embedded.returncode == 0 and not embedded.stderr, embedded
        assert profiled.returncode == 0 and not profiled.stderr, profiled
        a, b = json.loads(embedded.stdout), json.loads(profiled.stdout)
        assert {k: a[k] for k in ('outcome', 'revision', 'parent', 'event')} == \
               {k: b[k] for k in ('outcome', 'revision', 'parent', 'event')}, (a, b)
        first = t.run(['history', '--ledger', 'a1.db', '--revision', '1'], env=env)
        second = t.run(['history', '--ledger', 'a2.db', '--revision', '1'], env=env)
        assert first.returncode == 0 and second.returncode == 0, (first.stderr[:200], second.stderr[:200])
        hist_a, hist_b = json.loads(first.stdout), json.loads(second.stdout)
        changed = diffs(hist_a, hist_b)
        unexplained = [path for path in changed if not justified(path)]
        assert not unexplained, unexplained
        assert not any(path.startswith('/data/interpretation/bundle') or path.startswith('/data/interpretation/semantic')
                       for path in changed), changed
        # Equal request, different ledger identity: the revision and selector are identical.
        assert hist_a['data']['selector'] == hist_b['data']['selector'] == 'revision'
        print('history fields that differ (all justified):', ', '.join(changed))
        # History carries no W-derived digest: the only digest-named fields are the stdlib tree digests.
        digest_fields = sorted(path for path in key_paths(hist_a) if path.rsplit('/', 1)[-1].endswith('digest'))
        assert digest_fields == ['/data/interpretation/versions/stdlib_digest', '/data/versions/stdlib_digest'], digest_fields
        print('history W-derived digest: none recorded; manifest.work compared per job above')
        generator = (REPO / 'tests/vectors/interpretation-v2/generate.py').read_text()
        if 'quota' in generator:
            raise SystemExit('interpretation-v2 covers quota: compare digest.txt')
        print('interpretation-v2: not applicable')
        scratch_free(first.stdout, second.stdout)
    assert_no_forbidden()
    print(CASE, 'passed')


# ---------------------------------------------------------------------------------------------------------------------
# E08: projection omission vectors at the real 1 MiB budget.

BUDGET = 1048576
NOTICE = '## omissions\n\nOptional sections omitted: b, a.\n'


def projection_tree(root):
    t = Tree(root)
    t.copy_fixture()
    write(t.trusted / 'p.json', profile_json(['modules/p.lua']))
    assert t.run(['init', '--ledger', 'led.db', '--workspace-id', 'quota']).returncode == 0
    loaded = t.run(t.load_args('led.db', 'k1'))
    assert loaded.returncode == 0 and not loaded.stderr, loaded
    return t


def hproj(t):
    return t.run(['with-profile', '--stdlib-profile', '../trusted/p.json', 'hproj', '--ledger', 'led.db'])


def case_e08_projection_vector_binary():
    with tempfile.TemporaryDirectory(prefix='planar-x08-') as temp:
        t = projection_tree(Path(temp) / 'p')
        write(t.trusted / 'modules/p.lua', projection_module(''))
        base = hproj(t)
        assert base.returncode == 0 and not base.stderr, base
        first = json.loads(base.stdout)['data']['sections']
        assert first[0]['id'] == 'recorded_history', first
        M = first[0]['bytes']

        # V1: optional a nearly fills the budget and b is small; omitting b alone still exceeds, then a goes.
        a_bytes, b_bytes = BUDGET - M - 20, 37
        write(t.trusted / 'modules/p.lua', projection_module(
            "{id='a',text=a..'\\n',mandatory=false},{id='b',text=b..'\\n',mandatory=false}",
            lua_repeat('a', 'A', a_bytes - 7) + lua_repeat('b', 'B', b_bytes - 7)))
        v1 = hproj(t)
        assert v1.returncode == 0 and not v1.stderr, v1
        data = json.loads(v1.stdout)['data']
        assert data['omitted'] == ['b', 'a'], data['omitted']
        assert len(data['text'].encode()) == M + 1 + len(NOTICE.encode()), len(data['text'].encode())
        assert data['text'].endswith('\n' + NOTICE), data['text'][-120:]
        assert [(s['id'], s['included'], s['bytes']) for s in data['sections']] == \
               [('recorded_history', True, M), ('a', False, a_bytes), ('b', False, 37), ('omissions', True, 48)], \
               data['sections']

        # V2: an exact fit with a mandatory pad of p = budget - M - 49 bytes and optionals a and b of 37 bytes each.
        p = BUDGET - M - 49
        write(t.trusted / 'modules/p.lua', projection_module(
            "{id='pad',text=pad..'\\n',mandatory=true},{id='a',text=a..'\\n',mandatory=false},"
            "{id='b',text=b..'\\n',mandatory=false}",
            lua_repeat('pad', 'P', p - 9) + lua_repeat('a', 'A', 30) + lua_repeat('b', 'B', 30)))
        v2 = hproj(t)
        assert v2.returncode == 0 and not v2.stderr, v2
        data = json.loads(v2.stdout)['data']
        assert data['omitted'] == ['b', 'a'], data['omitted']
        assert len(data['text'].encode()) == BUDGET, len(data['text'].encode())
        # An included section after the first carries its leading joiner in its bytes (the omissions 48 is 47 plus one,
        # and the in-process C4 D4 vector reports a first-position pad as 11 with no joiner), so pad reports p + 1 and
        # the joined text M + (p + 1) + 48 is exactly the budget. Omitted sections report their own content length.
        assert [(s['id'], s['included'], s['bytes']) for s in data['sections']] == \
               [('recorded_history', True, M), ('pad', True, p + 1), ('a', False, 37), ('b', False, 37),
                ('omissions', True, 48)], data['sections']

        # V3: one more byte of mandatory pad is a closed budget_exceeded refusal with empty stdout.
        write(t.trusted / 'modules/p.lua', projection_module(
            "{id='pad',text=pad..'\\n',mandatory=true},{id='a',text=a..'\\n',mandatory=false},"
            "{id='b',text=b..'\\n',mandatory=false}",
            lua_repeat('pad', 'P', p - 8) + lua_repeat('a', 'A', 30) + lua_repeat('b', 'B', 30)))
        v3 = hproj(t)
        assert v3.returncode == 3 and v3.stdout == b'', (v3.returncode, v3.stdout[:200])
        refusal = json.loads(v3.stderr)['error']
        assert refusal['code'] == 'budget_exceeded' and refusal['field'] == 'render.budget_bytes', refusal
        assert refusal['message'] == 'Mandatory Markdown exceeds budget', refusal
        assert refusal['details']['budget_bytes'] == BUDGET and refusal['details']['mandatory_bytes'] == BUDGET + 1, refusal
        scratch_free(v3.stderr)
    assert_no_forbidden()
    print(CASE, 'passed')


# ---------------------------------------------------------------------------------------------------------------------
# E09: history_id and decimal vectors through the binary; values reach Lua as exact strings and absent as "0".

HISTORY_TOKENS_OK = ['1', '2147483647', '2147483648', '9223372036854775807']
HISTORY_TOKENS_USAGE = ['0', '00', '01', '+1', '-1', '1.0', '1e0', '', ' ', ' 1', '1 ', '9223372036854775808']


def case_e09_history_id_binary():
    with tempfile.TemporaryDirectory(prefix='planar-x08-') as temp:
        t = Tree(Path(temp) / 'h')
        write(t.trusted / 'p.json', profile_json(['modules/p.lua']))
        write(t.trusted / 'modules/p.lua', reporter_module() + decimal_module())
        assert t.run(['init', '--ledger', 'led.db', '--workspace-id', 'quota']).returncode == 0

        def hist(extra):
            return t.run(['with-profile', '--stdlib-profile', '../trusted/p.json', 'hist', '--ledger', 'led.db', *extra])

        def dec(extra):
            return t.run(['with-profile', '--stdlib-profile', '../trusted/p.json', 'dec', *extra])

        control = hist([])
        assert (control.returncode, control.stdout) == (3, b'') and \
               control.stderr == schema_bytes(reporter_message({}, None)), control
        for name in ('revision', 'at-event', 'after-event'):
            for token in HISTORY_TOKENS_OK:
                got = hist(['--' + name, token])
                assert (got.returncode, got.stdout, got.stderr) == \
                       (3, b'', schema_bytes(reporter_message({name: token}, None))), (name, token, got)
            for token in HISTORY_TOKENS_USAGE:
                got = hist(['--' + name, token])
                assert (got.returncode, got.stdout, got.stderr) == (2, b'', USAGE), (name, token, got)
        magnitudes = ('1', '9223372036854775807')
        for magnitude in magnitudes:
            conflicts = [['--revision', magnitude, '--at-event', magnitude],
                         ['--revision', magnitude, '--request-id', 'k'],
                         ['--at-event', magnitude, '--request-id', 'k'],
                         ['--revision', magnitude, '--after-event', magnitude],
                         ['--at-event', magnitude, '--after-event', magnitude],
                         ['--revision', magnitude, '--limit', '5'],
                         ['--at-event', magnitude, '--limit', '5'],
                         ['--request-id', 'k', '--after-event', magnitude],
                         ['--request-id', 'k', '--limit', '5']]
            for extra in conflicts:
                got = hist(extra)
                assert (got.returncode, got.stdout, got.stderr) == (2, b'', USAGE), (extra, got)
        paging = hist(['--after-event', '9223372036854775807', '--limit', '5'])
        assert (paging.returncode, paging.stdout, paging.stderr) == \
               (3, b'', schema_bytes(reporter_message({'after-event': '9223372036854775807'}, '5'))), paging
        paging = hist(['--limit', '100', '--after-event', '2147483648'])
        assert paging.stderr == schema_bytes(reporter_message({'after-event': '2147483648'}, '100')), paging
        # Decimal boundary: 2147483647 is the largest accepted general decimal; 2147483648 is usage.
        for token, text in (('2147483647', 'integer:2147483647'), ('1', 'integer:1')):
            got = dec(['--count', token])
            assert (got.returncode, got.stdout, got.stderr) == (0, summary('dec', text), b''), (token, got)
        for token in ('2147483648', '9223372036854775807', '0', '-1', '1.0'):
            got = dec(['--count', token])
            assert (got.returncode, got.stdout, got.stderr) == (2, b'', USAGE), (token, got)
        for token in ('1', '100'):
            assert hist(['--limit', token]).stderr == schema_bytes(reporter_message({}, token))
        for token in ('0', '101', '2147483648', '9223372036854775807'):
            got = hist(['--limit', token])
            assert (got.returncode, got.stdout, got.stderr) == (2, b'', USAGE), (token, got)
        scratch_free(control.stderr)
    assert_no_forbidden()
    print(CASE, 'passed')


# ---------------------------------------------------------------------------------------------------------------------
# E10: removed profile, workspace and registry; the identical with-profile request replays without new rows.

def case_e10_removed_profile_replay_binary():
    with tempfile.TemporaryDirectory(prefix='planar-x08-') as temp:
        t = Tree(Path(temp) / 'r')
        t.copy_fixture()
        write(t.trusted / 'e.json', profile_json([]))
        ledger = t.work / 'l.db'
        assert t.run(['init', '--ledger', 'l.db', '--workspace-id', 'quota']).returncode == 0
        profiled = ['with-profile', '--stdlib-profile', '../trusted/e.json']
        first = t.run([*profiled, *t.load_args('l.db', 'k', 'driver')])
        assert first.returncode == 0 and not first.stderr, first
        assert rows(ledger) == (1, 1, 2), rows(ledger)
        saved = sqlite3.connect(ledger).execute('SELECT result FROM requests WHERE request_id=?', ('k',)).fetchone()[0]
        assert saved + b'\n' == first.stdout, 'saved result differs from the first response'

        shutil.rmtree(t.trusted)
        shutil.rmtree(t.fx / 'workspace')
        shutil.rmtree(t.fx / 'registry')
        again = t.run([*profiled, *t.load_args('l.db', 'k', 'driver')])
        assert (again.returncode, again.stdout, again.stderr) == (0, first.stdout, b''), again
        assert rows(ledger) == (1, 1, 2), rows(ledger)

        rival = t.run([*profiled, *t.load_args('l.db', 'k', 'rival')])
        assert (rival.returncode, rival.stdout, rival.stderr) == (3, b'', REQUEST_CONFLICT), rival
        assert rows(ledger) == (1, 1, 2), rows(ledger)

        # The embedded request_v2 carries selector_tag 00 and the profile request carries 01, so the same ID with a
        # different body conflicts (external-profile-contract.md:552-560 and the format2 replay table row at :642).
        embedded = t.run(t.load_args('l.db', 'k', 'driver'))
        assert (embedded.returncode, embedded.stdout, embedded.stderr) == (3, b'', REQUEST_CONFLICT), embedded
        assert rows(ledger) == (1, 1, 2), rows(ledger)
        scratch_free(first.stdout, again.stdout)
    assert_no_forbidden()
    print(CASE, 'passed')


DISPATCH = {
    'e01_no_discovery': case_e01_no_discovery,
    'e01_registration_policy': case_e01_registration_policy,
    'e02_envelope_rows_binary': case_e02_envelope_rows_binary,
    'e03_closure_separation_binary': case_e03_closure_separation_binary,
    'e07_old_goldens_unselected_profile': case_e07_old_goldens_unselected_profile,
    'e07_old_w_vectors_binary': case_e07_old_w_vectors_binary,
    'e08_projection_vector_binary': case_e08_projection_vector_binary,
    'e09_history_id_binary': case_e09_history_id_binary,
    'e10_removed_profile_replay_binary': case_e10_removed_profile_replay_binary,
}
DISPATCH[CASE]()
