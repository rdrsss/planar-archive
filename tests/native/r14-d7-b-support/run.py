"""Record one direct serialized verification command and retain raw output."""
from pathlib import Path
import datetime,json,subprocess,sys
root=Path(__file__).resolve().parents[3];e=root/'tests/native/r14-d7-b-evidence'
name,*argv=sys.argv[1:];start=datetime.datetime.now(datetime.timezone.utc).isoformat()
p=subprocess.run(argv,cwd=root,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
(e/(name+'.log')).write_bytes(p.stdout)
rows=json.loads((e/'commands.json').read_text()) if (e/'commands.json').exists() else []
rows.append({'name':name,'argv':argv,'start':start,'end':datetime.datetime.now(datetime.timezone.utc).isoformat(),'exit':p.returncode})
(e/'commands.json').write_text(json.dumps(rows,indent=2)+'\n')
print(name, 'exit',p.returncode);print(p.stdout.decode(errors='replace')[-3000:]);sys.exit(p.returncode)
