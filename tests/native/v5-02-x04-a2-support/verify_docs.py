"""Final build/lint binding for a strictly comment-only correction after the162-test pass."""
from pathlib import Path
import datetime,difflib,hashlib,json,re,shutil,subprocess,sys
from protect import ROOT,OUT,OWNED,sha
PASS=OUT/'passes'/sys.argv[1];PASS.mkdir(parents=True,exist_ok=False)
PREVIOUS=OUT/'passes/09-final'
def write(n,v):(PASS/n).write_text(json.dumps(v,indent=2,sort_keys=True)+'\n')
owned=OWNED+[str(p.relative_to(ROOT)) for p in sorted((ROOT/'tests/native/v5-02-x04-a2-support').glob('*.py'))]
freeze={n:sha(ROOT/n) for n in owned if (ROOT/n).exists()};write('source-freeze.json',freeze)
prior=json.loads((PREVIOUS/'source-freeze.json').read_text())
changes=[n for n in prior if freeze[n]!=prior[n]]
assert changes==['src/engine/capture/profile.cpp']
old=(PREVIOUS/'candidate-source'/changes[0]).read_text();new=(ROOT/changes[0]).read_text()
# Token comparison is verification tooling only; preserve string/character tokens
# exactly and discard comments and inter-token whitespace. The raw diff is retained.
pattern=r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|//[^\n]*|/\*.*?\*/|[A-Za-z_][A-Za-z_0-9]*|[0-9]+|[^\s]'
def tokens(s):return [t for t in re.findall(pattern,s,re.S) if not t.startswith('//') and not t.startswith('/*')]
assert tokens(old)==tokens(new)
(PASS/'comment-only.diff').write_text(''.join(difflib.unified_diff(old.splitlines(True),new.splitlines(True),fromfile='pass09/profile.cpp',tofile='final/profile.cpp')))
write('comment-only-proof.json',dict(changed_sources=changes,old_sha256=prior[changes[0]],new_sha256=freeze[changes[0]],tokens_identical=True,token_count=len(tokens(new)),prior_source_freeze_sha256=sha(PREVIOUS/'source-freeze.json'),prior_test_output_sha256=sha(PREVIOUS/'debug-focused.log'),prior_test_junit_sha256=sha(PREVIOUS/'debug-focused.xml'),prior_focused_results_sha256=sha(PREVIOUS/'focused-results.json')))
for n in freeze:
 target=PASS/'candidate-source'/n;target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(ROOT/n,target)
rows=[]
def run(label,args):
 start=datetime.datetime.now(datetime.timezone.utc)
 with (PASS/(label+'.log')).open('w') as stream:r=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=stream,stderr=subprocess.STDOUT)
 rows.append(dict(label=label,argv=list(map(str,args)),exit_code=r.returncode,start=start.isoformat(),seconds=(datetime.datetime.now(datetime.timezone.utc)-start).total_seconds()));write('commands.json',rows)
 assert {n:sha(ROOT/n) for n in freeze}==freeze,('source changed',label)
 if r.returncode:raise SystemExit((label,r.returncode))
 return (PASS/(label+'.log')).read_text()
BUILD=ROOT/'build/rewrite-debug'
run('debug-build',['cmake','--build',BUILD,'--target','all','planar_tests'])
listing=json.loads(run('debug-inventory',['ctest','--test-dir',BUILD,'--show-only=json-v1']))
prior_inventory=json.loads((PREVIOUS/'debug-inventory.log').read_text())
assert {t['name'] for t in listing['tests']}=={t['name'] for t in prior_inventory['tests']}
run('pinned-lint',[sys.executable,'scripts/rewrite-check.py','lint','--build-dir',BUILD])
assert 'ninja: no work to do.' in run('debug-no-work',['cmake','--build',BUILD,'--target','all','planar_tests'])
write('source-stability.json',dict(unchanged=True,files=freeze))
write('debug-build-identities.json',{str(p.relative_to(ROOT)):sha(p) for p in [BUILD/'bin/planar',BUILD/'src/engine/planar_capture_profile_tests',BUILD/'src/engine/planar_capture_native_tests',BUILD/'src/engine/libplanar_admission_syscalls.dylib',BUILD/'src/engine/CMakeFiles/planar_capture_profile.dir/capture/profile.cpp.o',BUILD/'src/engine/CMakeFiles/planar_capture_native.dir/capture/native.cpp.o']})
write('pass-manifest.json',{str(p.relative_to(PASS)):sha(p) for p in sorted(PASS.rglob('*')) if p.is_file() and p.name!='pass-manifest.json'})
print('Final source bound to comment-identical162-case pass; build/inventory/lint/no-work passed.')
