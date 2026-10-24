#!/usr/bin/env python3
"""Archive one direct, serialized verification command and its exact exit status."""
from pathlib import Path
import datetime,json,subprocess,sys
root=Path(__file__).resolve().parents[3]
evidence=root/'tests/native/r14-d7-a-evidence'
name,*command=sys.argv[1:]
start=datetime.datetime.now(datetime.timezone.utc).isoformat()
result=subprocess.run(command,cwd=root,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
(evidence/(name+'.log')).write_bytes(result.stdout)
records=evidence/'commands.json'
rows=json.loads(records.read_text()) if records.exists() else []
rows.append(dict(name=name,argv=command,start=start,end=datetime.datetime.now(datetime.timezone.utc).isoformat(),exit=result.returncode))
records.write_text(json.dumps(rows,indent=2)+'\n')
print(result.stdout.decode(errors='replace')[-12000:])
sys.exit(result.returncode)
