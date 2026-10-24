from pathlib import Path
import hashlib,json,os,stat,shutil
root=Path.cwd();out=root/'tests/native/v5-02-x05-a1-evidence'
rows={}
for cur,ds,fs in os.walk(root,followlinks=False):
 base=Path(cur);ds[:]=[n for n in ds if not(base==root and n in ['.git','build']) and base/n!=out]
 for n in ds+fs:
  p=base/n;s=p.lstat();k=str(p.relative_to(root))
  if stat.S_ISLNK(s.st_mode):rows[k]={'symlink':os.readlink(p),'mode':stat.S_IMODE(s.st_mode)}
  elif stat.S_ISREG(s.st_mode):rows[k]={'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'mode':stat.S_IMODE(s.st_mode)}
(out/'baseline.json').write_text(json.dumps(rows,indent=2,sort_keys=True)+'\n')
for name in ['src/engine/runtime/runtime.cpp','src/engine/runtime/runtime.cppm','src/engine/CMakeLists.txt','CLAUDE.md','docs/architecture.md','docs/testing.md','docs/vernacular/implementation.md','docs/vernacular/roadmap.md','docs/vernacular/external-profile-implementation.md']:
 p=root/name;dst=out/'original-source'/name;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,dst)
print('Frozen original:',len(rows),'entries')
