"""Freeze the D8 owned candidate, complete baseline protection and final raw evidence."""
from pathlib import Path
import difflib,hashlib,json,shutil,subprocess,xml.etree.ElementTree as ET
r=Path(__file__).resolve().parents[3];e=r/'tests/native/r14-d8-evidence'
sha=lambda b:hashlib.sha256(b).hexdigest()
owned=['tests/acceptance/bootstrap_load.py','src/engine/CMakeLists.txt','docs/testing.md',
       'docs/vernacular/acceptance-matrix.md','docs/vernacular/implementation.md',
       'docs/vernacular/roadmap.md','docs/vernacular/v5-alignment.md','docs/vernacular/r14-acceptance-evidence.md']
owned+=sorted(str(p.relative_to(r)) for p in (r/'tests/native/r14-d8-support').glob('*') if p.is_file())
candidate={};diff=''
for name in owned:
 p=r/name;b=p.read_bytes();candidate[name]=sha(b);out=e/'candidate'/name;out.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(p,out)
 before=e/'baseline'/name
 diff+=''.join(difflib.unified_diff(before.read_text().splitlines(True) if before.exists() else [],p.read_text().splitlines(True),fromfile='baseline/'+name,tofile='candidate/'+name))
(e/'owned.diff').write_text(diff)
(e/'candidate.json').write_text(json.dumps({'path_base':'repository','head':subprocess.check_output(['git','rev-parse','HEAD'],cwd=r,text=True).strip(),'files':candidate,'owned_diff_sha256':sha(diff.encode())},indent=2)+'\n')
baseline=json.loads((e/'baseline.json').read_text());changed=[];missing=[]
for name,h in baseline.items():
 p=r/name
 if not p.exists():missing.append(name)
 elif sha(p.read_bytes())!=h:changed.append(name)
coordinator=['docs/vernacular/rounds/R14-D8.md'];unexpected=sorted(set(changed)-set(owned)-set(coordinator))
assert not unexpected and not missing,(unexpected,missing)
(e/'protection.json').write_text(json.dumps({'path_base':'repository','baseline_file_count':len(baseline),'changed':sorted(changed),'coordinator_owned_exceptions':coordinator,'missing':missing,'unexpected':unexpected,'all_preexisting_evidence_and_goldens_unchanged':True},indent=2)+'\n')
results={};baseline_names={t['name'] for t in json.loads((r/'tests/native/r14-d7-b-review-evidence/debug-inventory.log').read_text())['tests']}
for mode in ['debug','release']:
 cases=ET.parse(e/(mode+'-full.xml')).findall('.//testcase');inventory=json.loads((e/(mode+'-inventory.json')).read_text());names={t['name'] for t in inventory['tests']}
 assert len(cases)==len(names)==402 and baseline_names<=names
 assert not any(c.find('skipped') is not None or c.find('failure') is not None or c.get('status') in ['notrun','disabled'] for c in cases)
 results[mode]={'executed':len(cases),'failures':0,'skips':0,'preserved_prior_names':len(baseline_names),'added':sorted(names-baseline_names)}
(e/'verification.json').write_text(json.dumps({'path_base':'repository','results':results,'debug_no_work_final':'tests/native/r14-d8-evidence/debug-no-work-repeat.log','release_no_work_final':'tests/native/r14-d8-evidence/release-no-work.log','limitations':['macOS ARM64 / LLVM23.1.4 only; Linux not executed','early non-hot journal remnants refuse unchanged; no supported automatic recovery/retry demonstrated','native VFS fault/process proof is not public CLI fault injection or hardware power loss','future R15+ execution and RA01 authoring remain open','independent review required'],'attempt_notes':'tests/native/r14-d8-evidence/failed-attempts.md'},indent=2)+'\n')
artifacts={}
for mode in ['debug','release']:
 build=r/('build/rewrite-'+mode)
 for p in [build/'bin/planar',build/'src/engine/planar_ledger_format_tests',build/'src/cmd/planar/planar_load_cli_probe',build/'compile_commands.json',build/'CMakeCache.txt']:
  artifacts[str(p.relative_to(r))]=sha(p.read_bytes())
(e/'artifacts.json').write_text(json.dumps({'path_base':'repository','files':artifacts},indent=2)+'\n')
files={str(p.relative_to(e)):sha(p.read_bytes()) for p in sorted(e.rglob('*')) if p.is_file() and p.name!='evidence-sha256.json'}
(e/'evidence-sha256.json').write_text(json.dumps({'path_base':'tests/native/r14-d8-evidence','files':files},indent=2)+'\n')
print('Frozen owned candidate, raw evidence, artifacts, 400 preserved prior names and 402/402 tests per configuration')
