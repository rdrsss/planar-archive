"""Frozen A3 candidate verification, serialized Debug scope gate only."""
from pathlib import Path
import datetime,hashlib,json,os,shutil,stat,subprocess,sys,re,difflib
import xml.etree.ElementTree as ET
ROOT=Path(__file__).resolve().parents[3];OUT=ROOT/'tests/native/v5-02-x05-a3-evidence';SUP=ROOT/'tests/native/v5-02-x05-a3-support';BUILD=ROOT/'build/rewrite-debug'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def write(p,v):p.write_text(json.dumps(v,indent=2,sort_keys=True)+'\n')
def tree():
 rows={}
 for cur,ds,fs in os.walk(ROOT,followlinks=False):
  base=Path(cur);ds[:]=[n for n in ds if not(base==ROOT and n in ['.git','build']) and base/n!=OUT]
  for n in ds+fs:
   p=base/n;s=p.lstat();key=str(p.relative_to(ROOT))
   if stat.S_ISLNK(s.st_mode):rows[key]={'symlink':os.readlink(p),'mode':stat.S_IMODE(s.st_mode)}
   elif stat.S_ISREG(s.st_mode):rows[key]={'sha256':sha(p),'mode':stat.S_IMODE(s.st_mode)}
 return rows
attempt=OUT/sys.argv[1];attempt.mkdir(exist_ok=False);frozen=tree();write(attempt/'source-freeze.json',frozen)
base=json.loads((OUT/'baseline.json').read_text());changed=sorted(k for k in base.keys()|frozen.keys() if base.get(k)!=frozen.get(k))
allowed=['src/engine/runtime/runtime.cpp','src/engine/runtime/runtime.cppm','src/engine/runtime/external_registration.t.cpp','src/engine/CMakeLists.txt','src/cmd/planar/CMakeLists.txt','CLAUDE.md','docs/architecture.md','docs/testing.md','docs/vernacular/implementation.md','docs/vernacular/roadmap.md','docs/vernacular/external-profile-implementation.md']
assert all(k in allowed or k.startswith('tests/native/v5-02-x05-a3-support/') for k in changed),changed
write(attempt/'scope-audit.json',dict(changed=changed,allowed=allowed,shipped_lua_unchanged=all(base[k]==frozen[k] for k in base if k.startswith('src/stdlib/')),historical_unchanged=True,symlink=os.readlink(ROOT/'AGENTS.md')))
diff=[]
for name in changed:
 target=attempt/'candidate-source'/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(ROOT/name,target)
 original=OUT/'original-source'/name;old=original.read_text() if original.exists() else ''
 diff.extend(difflib.unified_diff(old.splitlines(True),(ROOT/name).read_text().splitlines(True),fromfile='a/'+name,tofile='b/'+name))
(attempt/'owned.diff').write_text(''.join(diff));commands=[]
def run(label,args):
 start=datetime.datetime.now(datetime.timezone.utc)
 with (attempt/(label+'.log')).open('w') as out:r=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=out,stderr=subprocess.STDOUT)
 end=datetime.datetime.now(datetime.timezone.utc);commands.append(dict(label=label,argv=list(map(str,args)),start=start.isoformat(),end=end.isoformat(),seconds=(end-start).total_seconds(),exit_code=r.returncode));write(attempt/'commands.json',commands);print(label,r.returncode,flush=True)
 if r.returncode:raise SystemExit((label,r.returncode))
 return (attempt/(label+'.log')).read_text()
run('configure',['cmake','--preset','debug']);run('build',['cmake','--build',BUILD,'--target','all','planar_tests'])
listing=json.loads(run('inventory',['ctest','--test-dir',BUILD,'--show-only=json-v1']));names=[t['name'] for t in listing['tests']];old=[t['name'] for t in json.loads((OUT/'baseline-inventory.json').read_text())['tests']]
assert len(old)==555 and len(names)==570 and len(names)==len(set(names)) and set(old)<=set(names)
write(attempt/'inventory-preservation.json',dict(previous=len(old),current=len(names),missing=sorted(set(old)-set(names)),added=sorted(set(names)-set(old))))
pattern='^replacement.(X05 A[0123]|command |runtime |interpretation |load |acceptance |cli|.*_cli|CLI11|help |invocation |workspace |profile |effective )'
run('focused',['ctest','--test-dir',BUILD,'-R',pattern,'--output-on-failure','--output-junit',attempt/'focused.xml']);shutil.copy2(BUILD/'Testing/Temporary/LastTest.log',attempt/'LastTest.log')
cases=ET.parse(attempt/'focused.xml').findall('.//testcase');matched=[n for n in names if re.search(pattern,n)]
assert cases and sorted(c.get('name') for c in cases)==sorted(matched)
assert all(c.get('status')=='run' and all(c.find(k) is None for k in ['failure','error','skipped']) for c in cases)
assert all(int(s.get(k,'0'))==0 for s in ET.parse(attempt/'focused.xml').iter('testsuite') for k in ['failures','errors','skipped','disabled'])
write(attempt/'test-summary.json',dict(executed=len(cases),new_a3=sum('X05 A3' in c.get('name') for c in cases),failures=0,errors=0,skips=0,disabled=0,names=[c.get('name') for c in cases]))
run('production-probes',[sys.executable,SUP/'probe.py',attempt.name+'-production-probes'])
for label,script in [('canonical','tests/vectors/generate.py'),('records','tests/vectors/records/generate.py'),('request-v2','tests/vectors/records/generate-request-v2.py'),('effective-provenance','tests/vectors/effective-provenance/generate.py')]:run(label+'-vectors',[sys.executable,script,'--check'])
for label,script in [('historical','v5-02-b2-support/audit-variants.py'),('d6','r14-d6-support/audit.py'),('d6-c01','r14-d6-c01-support/audit.py'),('d7-b','r14-d7-b-support/audit.py')]:run(label+'-audit',[sys.executable,ROOT/'tests/native'/script,BUILD,attempt/(label+'-report.json')])
generated=BUILD/'src/engine/v5-02-x05-a0-generated';generation=json.loads((generated/'generation.json').read_text())
for name,row in generation.items():
 if name=='lua':continue
 original=ROOT/'src/engine/runtime'/('runtime.cppm' if name.endswith('.cppm') else 'runtime.cpp');assert sha(original)==row['source_sha256'] and sha(generated/name)==row['generated_sha256'] and row['reverse_identical'];text=original.read_text()
 for sub in row['substitutions']:assert text.count(sub['before'])==sub['count'];text=text.replace(sub['before'],sub['after'])
 assert text==(generated/name).read_text()
 for sub in reversed(row['substitutions']):assert text.count(sub['after'])==sub['count'];text=text.replace(sub['after'],sub['before'])
 assert text==original.read_text()
assert (generated/'commands_0.lua').read_bytes()==(ROOT/'src/stdlib/planar/commands.lua').read_bytes();write(attempt/'a0-runtime-audit.json',generation)
run('pinned-product-lint',[sys.executable,'scripts/rewrite-check.py','lint','--build-dir',BUILD]);assert 'ninja: no work to do.' in run('no-work',['cmake','--build',BUILD,'--target','all','planar_tests'])
now=tree();write(attempt/'source-after.json',now);assert now==frozen,sorted(k for k in now.keys()|frozen.keys() if now.get(k)!=frozen.get(k))
artifacts={str(p.relative_to(BUILD)):sha(p) for p in BUILD.rglob('*') if p.is_file() and (p.suffix in ['.o','.pcm','.a','.dylib'] or p.name in ['build.ninja','CMakeCache.txt','compile_commands.json'] or (os.access(p,os.X_OK) and p.suffix==''))};write(attempt/'build-identities.json',artifacts)
print('Frozen source stable; focused scope gate passed',len(cases),flush=True)
