"""Record one direct serialized verification command and retain raw output."""
from pathlib import Path
import datetime,json,subprocess,sys,shutil
root=Path(__file__).resolve().parents[3];e=root/'tests/native/r14-d8-evidence'
name,*argv=sys.argv[1:];start=datetime.datetime.now(datetime.timezone.utc).isoformat()
p=subprocess.run(argv,cwd=root,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
(e/(name+'.log')).write_bytes(p.stdout)
if argv[0]=='ctest' and '--test-dir' in argv:
 last=root/argv[argv.index('--test-dir')+1]/'Testing/Temporary/LastTest.log'
 if last.exists():shutil.copyfile(last,e/(name+'-LastTest.log'))
rows=json.loads((e/'commands.json').read_text()) if (e/'commands.json').exists() else []
rows.append({'name':name,'argv':argv,'start':start,'end':datetime.datetime.now(datetime.timezone.utc).isoformat(),'exit':p.returncode})
(e/'commands.json').write_text(json.dumps(rows,indent=2)+'\n')
print(name, 'exit',p.returncode);print(p.stdout.decode(errors='replace')[-3000:]);sys.exit(p.returncode)
