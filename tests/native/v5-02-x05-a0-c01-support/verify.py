#!/usr/bin/env python3
"""Frozen, serialized Debug-only feasibility verification (not the final X05 gate)."""
from pathlib import Path
import datetime,hashlib,json,os,shutil,stat,subprocess,sys
import xml.etree.ElementTree as ET
ROOT=Path(__file__).resolve().parents[3]
OUT=ROOT/'tests/native/v5-02-x05-a0-c01-evidence'
SUP=ROOT/'tests/native/v5-02-x05-a0-support'
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
attempt=OUT/sys.argv[1];attempt.mkdir(exist_ok=False)
frozen=tree();write(attempt/'source-freeze.json',frozen)
for name,row in frozen.items():
 if name.startswith(('src/engine/runtime/','src/stdlib/','tests/native/v5-02-x05-a0-support/')) or name=='src/engine/CMakeLists.txt':
  target=attempt/'candidate-source'/name;target.parent.mkdir(parents=True,exist_ok=True)
  if 'symlink' in row:target.symlink_to(row['symlink'])
  else:shutil.copy2(ROOT/name,target)
commands=[]
def stable():
 now=tree();assert now==frozen,sorted(k for k in now.keys()|frozen.keys() if now.get(k)!=frozen.get(k))
def run(label,args):
 stable();start=datetime.datetime.now(datetime.timezone.utc)
 with (attempt/(label+'.log')).open('w') as out:
  result=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=out,stderr=subprocess.STDOUT)
 end=datetime.datetime.now(datetime.timezone.utc)
 commands.append(dict(label=label,argv=list(map(str,args)),start_utc=start.isoformat(),end_utc=end.isoformat(),seconds=(end-start).total_seconds(),exit_code=result.returncode))
 write(attempt/'commands.json',commands);print(label,result.returncode,flush=True);stable()
 if result.returncode:raise SystemExit((label,result.returncode))
 return (attempt/(label+'.log')).read_text()
build=ROOT/'build/rewrite-debug'
run('configure',['cmake','--preset','debug'])
run('build',['cmake','--build',build,'--target','all','planar_tests'])
listing=json.loads(run('inventory',['ctest','--test-dir',build,'--show-only=json-v1']))
prior=json.loads((ROOT/'tests/native/v5-02-x04-b4-evidence/attempt01/debug/inventory.log').read_text())
old=[t['name'] for t in prior['tests']];names=[t['name'] for t in listing['tests']]
assert len(old)==520 and set(old)<=set(names) and len(names)==len(set(names))
a0prior=json.loads((ROOT/'tests/native/v5-02-x05-a0-evidence/attempt01/inventory.log').read_text())
a0names=[t['name'] for t in a0prior['tests']]
assert len(a0names)==532 and set(a0names)<=set(names) and len(names)==535
write(attempt/'inventory-preservation.json',dict(previous=len(old),previous_a0=len(a0names),current=len(names),missing=sorted(set(old)-set(names)),added=sorted(set(names)-set(old))))
run('focused',['ctest','--test-dir',build,'-R','^replacement.(X05 A0|command )','--output-on-failure','--output-junit',attempt/'focused.xml'])
cases=ET.parse(attempt/'focused.xml').findall('.//testcase')
assert cases and all(c.get('status')=='run' and all(c.find(k) is None for k in ['failure','error','skipped']) for c in cases)
assert all(int(s.get(k,'0'))==0 for s in ET.parse(attempt/'focused.xml').iter('testsuite') for k in ['failures','errors','skipped','disabled'])
write(attempt/'test-summary.json',dict(executed=len(cases),new_audit_cases=sum('X05 A0' in c.get('name') for c in cases),existing_command_cases=sum('replacement.command ' in c.get('name') for c in cases),failures=0,errors=0,skips=0,names=[c.get('name') for c in cases]))
shutil.copy2(build/'Testing/Temporary/LastTest.log',attempt/'LastTest.log')
run('current-frame-probes',[sys.executable,ROOT/'tests/native/v5-02-x05-a0-c01-support/probe.py','green'])
run('graph-invariant-probe',[sys.executable,ROOT/'tests/native/v5-02-x05-a0-c01-support/fault-probe.py','green-exact'])
run('reservation-failure-probe',[sys.executable,ROOT/'tests/native/v5-02-x05-a0-c01-support/fault-probe.py','reservation-failure-final'])
run('loadkx-details',[build/'src/engine/planar_x05_closure_audit_tests','X05 A0 source compiled LOADKX*','-s'])
for label,script in [('canonical','tests/vectors/generate.py'),('records','tests/vectors/records/generate.py'),('request-v2','tests/vectors/records/generate-request-v2.py'),('effective-provenance','tests/vectors/effective-provenance/generate.py')]:
 run(label+'-vectors',[sys.executable,script,'--check'])
for label,script in [('historical','v5-02-b2-support/audit-variants.py'),('d6','r14-d6-support/audit.py'),('d6-c01','r14-d6-c01-support/audit.py'),('d7-b','r14-d7-b-support/audit.py')]:
 run(label+'-audit',[sys.executable,ROOT/'tests/native'/script,build,attempt/(label+'-audit-report.json')])
generated=build/'src/engine/v5-02-x05-a0-generated';generation=json.loads((generated/'generation.json').read_text())
for name,row in generation.items():
 if name=='lua':continue
 original=ROOT/'src/engine/runtime'/('runtime.cppm' if name.endswith('.cppm') else 'runtime.cpp')
 assert sha(original)==row['source_sha256'] and sha(generated/name)==row['generated_sha256'] and row['reverse_identical']
 text=original.read_text()
 for sub in row['substitutions']:
  assert text.count(sub['before'])==sub['count'];text=text.replace(sub['before'],sub['after'])
 assert text==(generated/name).read_text()
 for sub in reversed(row['substitutions']):
  assert text.count(sub['after'])==sub['count'];text=text.replace(sub['after'],sub['before'])
 assert text==original.read_text()
assert (generated/'commands_0.lua').read_bytes()==(ROOT/'src/stdlib/planar/commands.lua').read_bytes()
lua=generation['lua'];altered=(generated/'commands_1.lua').read_text();original=(ROOT/'src/stdlib/planar/commands.lua').read_text()
assert lua['count']==altered.count(lua['after'])==1 and altered.replace(lua['after'],lua['before'])==original
assert sha(ROOT/'src/stdlib/planar/commands.lua')==lua['source_sha256'] and sha(generated/'commands_1.lua')==lua['altered_sha256']
# The variant0 source module must be the exact production embedded module apart from its module name.
a=(build/'src/engine/planar_runtime_source.cppm').read_text()
b=(build/'src/engine/planar_x05_source_0.cppm').read_text()
assert a.replace('module planar.engine.runtime_source;','module planar.engine.x05_a0_0_source;')==b
write(attempt/'runtime-variant-audit.json',generation)
run('pinned-product-lint',[sys.executable,'scripts/rewrite-check.py','lint','--build-dir',build])
cache=(build/'CMakeCache.txt').read_text().splitlines();compiler=next(s.split('=',1)[1] for s in cache if s.startswith('CMAKE_CXX_COMPILER:'))
cpp=sorted(SUP.glob('*.cpp'))+sorted(SUP.glob('*.cppm'))
run('pinned-support-format',[Path(compiler).parent/'clang-format','--dry-run','--Werror',*cpp])
config=attempt/'Doxyfile-support'
config.write_text('\n'.join(['PROJECT_NAME = "X05 A0 verification only"','QUIET = YES','GENERATE_HTML = NO','GENERATE_LATEX = NO','GENERATE_XML = YES','WARNINGS = YES','WARN_IF_UNDOCUMENTED = YES','WARN_IF_DOC_ERROR = YES','WARN_NO_PARAMDOC = YES','WARN_AS_ERROR = FAIL_ON_WARNINGS','EXTRACT_ALL = NO','FILE_PATTERNS = *.cpp *.cppm','EXTENSION_MAPPING = cppm=C++','MACRO_EXPANSION = YES','EXPAND_ONLY_PREDEF = YES','PREDEFINED = export=','INPUT = '+' '.join('"'+str(p)+'"' for p in cpp),'OUTPUT_DIRECTORY = "'+str(attempt/'doxygen')+'"'])+'\n')
run('pinned-support-doxygen',['bash','scripts/cpp-lint-doxygen.sh',config])
text=run('no-work',['cmake','--build',build,'--target','all','planar_tests']);assert 'ninja: no work to do.' in text
baseline=json.loads((OUT/'baseline.json').read_text());current=tree()
changed=sorted(k for k in baseline.keys()|current.keys() if baseline.get(k)!=current.get(k))
allowed=['src/engine/CMakeLists.txt','CLAUDE.md','docs/architecture.md']+['docs/vernacular/'+n+'.md' for n in ['implementation','roadmap','v5-alignment','acceptance-matrix','external-profile-implementation']]
assert all(k in allowed or k.startswith(('tests/native/v5-02-x05-a0-support/','tests/native/v5-02-x05-a0-c01-support/')) for k in changed),changed
write(attempt/'baseline-diff.json',dict(changed=changed,production_runtime_stdlib_vendor_oldtests_unchanged=True,root_only_exclusions=['.git','build'],evidence_excluded=str(OUT.relative_to(ROOT))))
stable();write(attempt/'source-stability.json',dict(unchanged=True,entries=len(frozen),freeze_sha256=sha(attempt/'source-freeze.json')))
write(attempt/'build-identities.json',{str(p.relative_to(ROOT)):sha(p) for p in sorted(build.rglob('*')) if p.is_file() and not p.is_symlink() and (os.access(p,os.X_OK) or p.suffix in ['.o','.pcm'] or p.name in ['CMakeCache.txt','compile_commands.json','build.ninja'])})
write(attempt/'pass-manifest.json',{str(p.relative_to(attempt)):sha(p) for p in sorted(attempt.rglob('*')) if p.is_file() and p.name!='pass-manifest.json'})
print('Completed frozen Debug feasibility gate:',len(cases),'tests;',len(names),'registered; no skips; no-work build',flush=True)
