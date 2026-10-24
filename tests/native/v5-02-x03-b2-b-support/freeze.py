import pathlib,os,json,hashlib,shutil
root=pathlib.Path.cwd();out=root/'tests/native/v5-02-x03-b2-b-evidence'
files=[];links=[]
for d,dirs,names in os.walk(root,followlinks=False):
 rel=pathlib.Path(d).relative_to(root)
 dirs[:]=[n for n in dirs if not (rel==pathlib.Path('.') and n in ('.git','build')) and rel/n!=pathlib.Path('tests/native/v5-02-x03-b2-b-evidence')]
 for n in dirs[:]:
  p=pathlib.Path(d)/n
  if p.is_symlink():links.append({'path':str(p.relative_to(root)),'target':os.readlink(p)});dirs.remove(n)
 for n in names:
  p=pathlib.Path(d)/n
  if p.is_symlink():links.append({'path':str(p.relative_to(root)),'target':os.readlink(p)})
  elif p.is_file():files.append({'path':str(p.relative_to(root)),'sha256':hashlib.sha256(p.read_bytes()).hexdigest()})
base=json.loads((out/'baseline.json').read_text());previous={x['path']:x['sha256'] for x in base['files']};current={x['path']:x['sha256'] for x in files}
changed=[p for p,h in current.items() if previous.get(p)!=h];removed=sorted(set(previous)-set(current))
manifest={'path_base':str(root),'files':files,'symlinks':links,'changed_or_added':changed,'removed':removed,'symlinks_unchanged':links==base['symlinks']}
(out/'final-source-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
for name in changed:
 p=out/'candidate-source'/name;p.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(root/name,p)
(out/'candidate-manifest.json').write_text(json.dumps({'path_base':str(root),'files':[x for x in files if x['path'] in changed]},indent=2)+'\n')
print(json.dumps({'changed_or_added':changed,'removed':removed,'symlinks_unchanged':links==base['symlinks']},indent=2))
