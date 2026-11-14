# R20-CARRY-A shared helpers for the ctest-registered Python checks in this directory.
# Plain asserts and explicit timeouts; scratch roots live under the system temp directory and are removed on exit.
"""Shared helpers: scratch roots, mtime backdating, byte/mtime snapshots and a bounded subprocess runner."""
from pathlib import Path
import contextlib,os,shutil,subprocess,sys,tempfile

# A fixed old timestamp (2001-09-09) written with os.utime so no test ever sleeps to observe an mtime change.
OLD_NS=1_000_000_000_000_000_000
MARKER='r20 scratch input change'


def fail(message):
 """Raise the single failure type every check uses; the message is what ctest prints."""
 raise AssertionError(message)


def expect(condition,message):
 """Assert with an explicit message that survives `python -O`."""
 if not condition:fail(message)


@contextlib.contextmanager
def scratch(prefix):
 """A per-test temporary root, removed afterwards; never under the home directory or the repository."""
 d=tempfile.mkdtemp(prefix=prefix)
 try:yield Path(d)
 finally:shutil.rmtree(d,ignore_errors=True)


def run(cmd,timeout=240,cwd=None,env=None):
 """Run one command with an explicit timeout, capturing text output."""
 return subprocess.run([str(c) for c in cmd],capture_output=True,text=True,timeout=timeout,cwd=cwd,env=env)


def copy_inputs(repo,dest,relpaths):
 """Copy exactly the named repository files into the scratch root, keeping their relative layout."""
 for rel in relpaths:
  target=dest/rel
  target.parent.mkdir(parents=True,exist_ok=True)
  shutil.copyfile(repo/rel,target)


def append_marker(path,comment='//'):
 """Append a harmless comment line (the text has no keyword any generator replaces) to a scratch input."""
 with open(path,'a') as f:f.write(f'\n{comment} {MARKER}\n')


def listing(root):
 """Every regular file under root, as a sorted list of POSIX-relative names."""
 return sorted(p.relative_to(root).as_posix() for p in Path(root).rglob('*') if p.is_file())


def snapshot(root):
 """Map relative name -> (bytes, st_mtime_ns) for every file under root."""
 out={}
 for name in listing(root):
  p=Path(root)/name
  out[name]=(p.read_bytes(),p.stat().st_mtime_ns)
 return out


def backdate(root,names=None):
 """Set every (or the named) file's mtime to OLD_NS so any rewrite is observable as a newer mtime."""
 for name in (names if names is not None else listing(root)):
  os.utime(Path(root)/name,ns=(OLD_NS,OLD_NS))


def run_ok(cmd,label,timeout=240,cwd=None):
 """Run a command and require exit 0; the failure text carries the label and both streams."""
 r=run(cmd,timeout=timeout,cwd=cwd)
 expect(r.returncode==0,f'{label}: exit {r.returncode}\nstdout: {r.stdout[-1500:]}\nstderr: {r.stderr[-1500:]}')
 return r


def dispatch(cases,argv):
 """argv[1] is the case name; unknown names are an error; success prints '<case> passed'."""
 case=argv[1]
 if case not in cases:raise SystemExit('unknown case '+case)
 cases[case]()
 print(case,'passed')
