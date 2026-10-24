import subprocess,json,sys,time,pathlib,shutil
out=pathlib.Path('tests/native/v5-02-x03-b2-b-evidence');name=sys.argv[1]; argv=sys.argv[2:]; start=time.time()
with (out/(name+'.log')).open('wb') as f:r=subprocess.run(argv,stdout=f,stderr=subprocess.STDOUT)
(out/(name+'.json')).write_text(json.dumps(dict(argv=argv,cwd=str(pathlib.Path.cwd()),start=start,end=time.time(),exit_code=r.returncode),indent=2)+'\n')
p=pathlib.Path('build/rewrite-debug/Testing/Temporary/LastTest.log')
if argv[0]=='ctest' and p.exists():shutil.copyfile(p,out/(name+'-LastTest.log'))
print(name,r.returncode);sys.exit(r.returncode)
