#!/usr/bin/env python3
"""Frozen serialized focused production sealing verification; no full milestone gate."""
from pathlib import Path
import datetime,hashlib,json,os,shutil,stat,subprocess,sys
import xml.etree.ElementTree as ET
ROOT=Path(__file__).resolve().parents[3]
OUT=ROOT/'tests/native/v5-02-x05-a2-evidence'
SUP=ROOT/'tests/native/v5-02-x05-a2-support'
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
 if name.startswith(('src/engine/runtime/','src/stdlib/','tests/native/v5-02-x05-a2-support/')) or name in ['src/engine/CMakeLists.txt','src/cmd/planar/CMakeLists.txt','CLAUDE.md','docs/architecture.md','docs/testing.md','docs/vernacular/implementation.md','docs/vernacular/roadmap.md','docs/vernacular/external-profile-implementation.md']:
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
previous=json.loads((OUT/'baseline-inventory.json').read_text());prior_names=[t['name'] for t in previous['tests']]
assert len(prior_names)==552 and set(prior_names)<=set(names) and len(names)==555
write(attempt/'inventory-preservation.json',dict(previous=len(prior_names),current=len(names),missing=sorted(set(prior_names)-set(names)),added=sorted(set(names)-set(prior_names))))
pattern='^replacement.(X05 A[012]|command |runtime |interpretation |load |acceptance |cli|.*_cli|CLI11|help |invocation |workspace )'
reuse=OUT/'attempt01'
prior_freeze=json.loads((reuse/'source-freeze.json').read_text())
freeze_delta=sorted(k for k in prior_freeze.keys()|frozen.keys() if prior_freeze.get(k)!=frozen.get(k))
assert set(freeze_delta)<=set(['tests/native/v5-02-x05-a2-support/probe.py','tests/native/v5-02-x05-a2-support/verify.py']),freeze_delta
previous_commands=json.loads((reuse/'commands.json').read_text())
assert next(c for c in previous_commands if c['label']=='focused')['exit_code']==0
shutil.copy2(reuse/'focused.xml',attempt/'focused.xml')
shutil.copy2(reuse/'focused.log',attempt/'focused-reused.log')
shutil.copy2(reuse/'LastTest.log',attempt/'LastTest.log')
write(attempt/'focused-reuse.json',dict(source_attempt='attempt01',source_xml_sha256=sha(reuse/'focused.xml'),product_and_docs_byte_identical=True,only_changed_support=freeze_delta,original_command=next(c for c in previous_commands if c['label']=='focused')))
cases=ET.parse(attempt/'focused.xml').findall('.//testcase')
matched=[t['name'] for t in listing['tests'] if __import__('re').search(pattern,t['name'])]
assert cases and sorted(c.get('name') for c in cases)==sorted(matched)
assert all(c.get('status')=='run' and all(c.find(k) is None for k in ['failure','error','skipped']) for c in cases)
assert all(int(s.get(k,'0'))==0 for s in ET.parse(attempt/'focused.xml').iter('testsuite') for k in ['failures','errors','skipped','disabled'])
write(attempt/'test-summary.json',dict(executed=len(cases),new_guard_cases=sum('X05 A2' in c.get('name') for c in cases),historical_a0_cases=sum('X05 A0' in c.get('name') for c in cases),existing_command_cases=sum('replacement.command ' in c.get('name') for c in cases),failures=0,errors=0,skips=0,names=[c.get('name') for c in cases]))
assert sha(attempt/'LastTest.log')==sha(reuse/'LastTest.log')
run('production-probes',[sys.executable,SUP/'probe.py',attempt.name+'-production-probes'])
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
text=run('no-work',['cmake','--build',build,'--target','all','planar_tests']);assert 'ninja: no work to do.' in text
baseline=json.loads((OUT/'baseline.json').read_text());current=tree()
changed=sorted(k for k in baseline.keys()|current.keys() if baseline.get(k)!=current.get(k))
assert (ROOT/'src/engine/runtime/runtime.cppm').read_bytes()==(OUT/'original-source/src/engine/runtime/runtime.cppm').read_bytes()
allowed=['src/engine/CMakeLists.txt','src/cmd/planar/CMakeLists.txt','CLAUDE.md','docs/architecture.md','docs/testing.md','src/engine/runtime/runtime.cpp']+['docs/vernacular/'+n+'.md' for n in ['implementation','roadmap','external-profile-implementation']]
assert all(k in allowed or k.startswith(('src/engine/runtime/closure_guard/','tests/native/v5-02-x05-a2-support/')) for k in changed),changed
assert (ROOT/'AGENTS.md').is_symlink() and os.readlink(ROOT/'AGENTS.md')=='CLAUDE.md'
write(attempt/'baseline-diff.json',dict(changed=changed,historical_evidence_support_stdlib_vendor_oldtests_unchanged=True,root_only_exclusions=['.git','build'],evidence_excluded=str(OUT.relative_to(ROOT))))
import difflib
owned_diff=''
for name in changed:
 p=ROOT/name
 if not p.is_file():continue
 before=(OUT/'original-source'/name).read_text() if (OUT/'original-source'/name).exists() else ''
 owned_diff+=''.join(difflib.unified_diff(before.splitlines(True),p.read_text().splitlines(True),fromfile='before/'+name,tofile='candidate/'+name))
(attempt/'owned.diff').write_text(owned_diff)
stable();write(attempt/'source-stability.json',dict(unchanged=True,entries=len(frozen),freeze_sha256=sha(attempt/'source-freeze.json')))
write(attempt/'build-identities.json',{str(p.relative_to(ROOT)):sha(p) for p in sorted(build.rglob('*')) if p.is_file() and not p.is_symlink() and (os.access(p,os.X_OK) or p.suffix in ['.o','.pcm'] or p.name in ['CMakeCache.txt','compile_commands.json','build.ninja'])})
write(attempt/'pass-manifest.json',{str(p.relative_to(attempt)):sha(p) for p in sorted(attempt.rglob('*')) if p.is_file() and p.name!='pass-manifest.json'})
print('Completed frozen Debug feasibility gate:',len(cases),'tests;',len(names),'registered; no skips; no-work build',flush=True)
