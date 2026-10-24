"""Actual export processes using unchanged native acceptance fixture and a read-only oracle."""
from pathlib import Path
import hashlib
import json
import os
import socket
import sqlite3
import stat
import subprocess
import sys
import tempfile

binary, fixture = [str(Path(p).resolve()) for p in sys.argv[1:3]]
count = 0
with tempfile.TemporaryDirectory(prefix='planar-export-cli-') as temporary:
    root = Path(temporary).resolve()
    home = root/'HOME'
    home.mkdir()
    env = dict(os.environ, HOME=str(home), PLANAR_DB=str(home/'NEVER_LEGACY'))
    def snapshot(path):
        return {str(p.relative_to(path)): ('link', os.readlink(p)) if p.is_symlink() else
                ('file', hashlib.sha256(p.read_bytes()).hexdigest(), stat.S_IMODE(p.stat().st_mode)) if p.is_file() else
                ('directory' if p.is_dir() else 'special', stat.S_IMODE(p.stat().st_mode)) for p in path.rglob('*')}
    def setup(mode):
        p = root/mode
        p.mkdir()
        subprocess.run([fixture,str(p),mode],env=env,check=True,timeout=60)
        return p/'ledger'
    ledger = setup('history')
    unknown = setup('unknown')
    empty = setup('empty')
    db = sqlite3.connect(ledger.as_uri()+'?mode=ro&immutable=1',uri=True)
    sources = {str(r):str(s) for r,s in db.execute('SELECT id, source_id FROM revisions')}
    identity = db.execute('SELECT identity FROM ledger_meta').fetchone()[0]
    db.close()
    def run(args, status=0, code=None, target=None, closed=False):
        global count
        count += 1
        before = snapshot(root)
        if closed:
            reader, writer = os.pipe()
            os.close(reader)
            try:
                p = subprocess.run([binary,*args],env=env,cwd=root,stdout=writer,stderr=subprocess.PIPE,timeout=40)
            finally:
                os.close(writer)
            output = b''
        else:
            p = subprocess.run([binary,*args],env=env,cwd=root,capture_output=True,timeout=40)
            output = p.stdout
        assert p.returncode == status,(args,p.returncode,p.stderr)
        assert str(root).encode() not in output+p.stderr and b'PRIVATE' not in output+p.stderr
        after = snapshot(root)
        if target is not None:
            prefix = str(target.relative_to(root))
            after = {k:v for k,v in after.items() if k != prefix and not k.startswith(prefix+'/')}
        assert before == after,(args,before.keys() ^ after.keys())
        if status:
            assert not output
            value = json.loads(p.stderr)
            assert value['kind'] == 'error' and value['version'] == 1
            assert value['error']['code'] == code,value
            assert value['error']['field'] == ''
            assert p.stderr.endswith(b'\n')
            return value
        assert not p.stderr and output.endswith(b'\n')
        value = json.loads(output)
        assert set(value) == {'version','kind','ledger','revision','source'}
        assert value['version'] == 1 and value['kind'] == 'export'
        assert len(value['ledger']) == 32 and all(c in '0123456789abcdef' for c in value['ledger'])
        assert value['revision'] in ['1','2','3'] and isinstance(value['source'],str)
        assert output == json.dumps(value,sort_keys=True,separators=(',',':')).encode()+b'\n'
        return value
    def args(target, revision='1', source=ledger):
        return ['export','--ledger',str(source),'--revision',revision,'--target',str(target)]
    def exact(target, content):
        assert sorted(str(p.relative_to(target)) for p in target.rglob('*')) == ['a.lua']
        assert (target/'a.lua').read_bytes() == content
        assert stat.S_IMODE(target.stat().st_mode) == 0o755
        assert stat.S_IMODE((target/'a.lua').stat().st_mode) == 0o644
    for revision,content in [('1',b'original'),('2',b'changed'),('3',b'original')]:
        target = root/f'export-{revision}'
        old = os.umask(0o077)
        try:
            value = run(args(target,revision),target=target)
        finally:
            os.umask(old)
        assert value['source'] == sources[revision]
        assert value['ledger'] == identity
        exact(target,content)
    target = root/'unknown-export'
    run(args(target,source=unknown),target=target)
    exact(target,b'original')
    target = root/'relative'
    run(args(Path('relative')),target=target)
    exact(target,b'original')
    for name in ['directory','file','symlink','dangling','fifo','socket']:
        target = root/name
        if name == 'directory': target.mkdir()
        elif name == 'file': target.write_bytes(b'preserve')
        elif name == 'symlink': target.symlink_to(root/'export-1',target_is_directory=True)
        elif name == 'dangling': target.symlink_to(root/'nonexistent')
        elif name == 'fifo': os.mkfifo(target)
        else:
            endpoint = socket.socket(socket.AF_UNIX)
            endpoint.bind(str(target))
            endpoint.close()
        # Snapshot intentionally excludes FIFO bytes.
        run(args(target),3,'target_exists')
    (root/'link-parent').symlink_to(root/'export-1',target_is_directory=True)
    for target in [root/'missing'/'out',root/'missing'/'..'/'out',root/'link-parent'/'out']:
        run(args(target),3,'path_denied')
    run(args(root/'new',source=root/'missing-ledger'),3,'ledger_not_found')
    run(args(root/'new',source=empty),3,'revision_not_found')
    run(args(root/'new','999'),3,'revision_not_found')
    run(args(root/'new','9223372036854775807'),3,'revision_not_found')
    base = args(root/'new')
    for flag in ['--ledger','--revision','--target']:
        i = base.index(flag)
        run(base[:i]+base[i+2:],2,'usage')
        run(base[:i]+[flag],2,'usage')
        run(base+[flag,base[i+1]],2,'usage')
        run(base[:i]+[flag+'='+base[i+1]]+base[i+2:],2,'usage')
        run(base[:i+1]+['']+base[i+2:],2,'usage')
    for bad in ['0','01','+1','-1','none','1.0','1x','9223372036854775808',' 1']:
        run(args(root/'new',bad),2,'usage')
    for extra in [['x'],['--'],['--help'],['--unknown','1'],['init'],['history'],['render'],['export']]:
        run(base+extra,2,'usage')
    run(['export'],2,'usage')
    target = root/'pipe-published'
    error = run(args(target),4,'io',target=target,closed=True)
    assert error['error'] == {'code':'io','field':'','message':'Export may be complete; inspect the explicit target before retrying','details':{'operation':'stdout'}}
    exact(target,b'original')
    error = run(args(root/'new','999'),3,'revision_not_found',closed=True)
    assert 'details' not in error['error'] and 'may be complete' not in error['error']['message']
    for command,options in [('history',['--ledger',str(ledger)]),('init',['--ledger',str(root/'pipe-init'),'--workspace-id','sample'])]:
        target = root/'pipe-init' if command == 'init' else None
        error = run([command,*options],4,'io',target=target,closed=True)
        assert error['error'] == {'code':'io','field':'','message':'Cannot write stdout','details':{'operation':'stdout'}}
    help_result = subprocess.run([binary,'--help'],capture_output=True,env=env,timeout=10)
    assert help_result.returncode == 0 and not help_result.stderr
    assert b'planar export --ledger PATH --revision ID --target NEW_DIRECTORY\n' in help_result.stdout
    assert not list(home.iterdir())
print(f'{count} actual export/output CLI processes passed; no-reader pipe established before launch; ledger bytes and sibling membership preserved')
