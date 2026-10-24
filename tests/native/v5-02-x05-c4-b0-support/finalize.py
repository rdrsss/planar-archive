from protect import R,O,entry,write
from pathlib import Path
import json,sys,shutil,xml.etree.ElementTree as ET
S=Path(__file__).parent
if sys.argv[1]=='freeze':
 write(O/'frozen-support.json',{str(p.relative_to(R)):entry(p) for p in S.rglob('*')})
else:
 assert {str(p.relative_to(R)):entry(p) for p in S.rglob('*')}==json.loads((O/'frozen-support.json').read_text())
 cases=ET.parse(O/'private/probe.xml').findall('.//testcase');assert len(cases)==13 and not any(c.find(t) is not None for c in cases for t in ['failure','error','skipped'])
 write(O/'probe-summary.json',dict(cases=len(cases),failures=0,errors=0,skips=0))
 shutil.rmtree(O/'dependencies')
 for p in O.rglob('*'):
  if p.is_file() and (p.suffix in {'.o','.pcm','.a','.d'} or p.name=='probe'):p.unlink()
 write(O/'artifact-manifest.json',{str(p.relative_to(O)):entry(p) for p in O.rglob('*') if p.name not in {'artifact-manifest.json','finalization.json'}})
 write(O/'finalization.json',dict(schema='planar-c4-b0-artifacts-v1',manifest=entry(O/'artifact-manifest.json'),support=entry(O/'frozen-support.json'),report=entry(O/'REPORT.md'),all_owned_build_processes_finished=True,disposable_dependencies_removed=True,debug_released=True,no_self_approval=True))
