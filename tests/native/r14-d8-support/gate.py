"""Serialized D8 verification; archive test output before inventory and audit exact variants."""
from pathlib import Path
import json,subprocess,sys,xml.etree.ElementTree as ET
root=Path(__file__).resolve().parents[3];e=root/'tests/native/r14-d8-evidence';mode=sys.argv[1];build='build/rewrite-'+mode
runner=root/'tests/native/r14-d8-support/run.py'
def run(name,*args):subprocess.run([sys.executable,str(runner),mode+'-'+name,*map(str,args)],cwd=root,check=True)
run('build','cmake','--build',build,'--target','all','planar_tests')
report=e/(mode+'-full.xml')
run('full','ctest','--test-dir',build,'--output-on-failure','--output-junit',report)
cases=ET.parse(report).findall('.//testcase');assert len(cases)>=402
assert not any(c.find('skipped') is not None or c.find('failure') is not None or c.get('status') in ['notrun','disabled'] for c in cases)
# Inventory runs only after run.py archived full JUnit and LastTest.
p=subprocess.run(['ctest','--test-dir',build,'--show-only=json-v1'],cwd=root,capture_output=True,check=True)
(e/(mode+'-inventory.json')).write_bytes(p.stdout)
inventory=json.loads(p.stdout);assert len(inventory['tests'])==len(cases)
run('lint','python3','scripts/rewrite-check.py','lint','--build-dir',build)
run('support-format','/opt/homebrew/opt/llvm/bin/clang-format','--dry-run','--Werror','tests/native/r14-d8-support/acceptance_fault.t.cpp')
for name,script in [('historical','v5-02-b2-support/audit-variants.py'),('d6','r14-d6-support/audit.py'),('c01','r14-d6-c01-support/audit.py'),('d7b','r14-d7-b-support/audit.py')]:
 run(name+'-audit','python3','tests/native/'+script,build,e/(mode+'-'+name+'-audit.json'))
run('vectors','python3','tests/vectors/generate.py','--check')
run('record-vectors','python3','tests/vectors/records/generate.py','--check')
run('no-work','cmake','--build',build,'--target','all','planar_tests')
assert 'ninja: no work to do.' in (e/(mode+'-no-work.log')).read_text()
print(mode,len(cases),'tests, zero skips; all final gates complete')
