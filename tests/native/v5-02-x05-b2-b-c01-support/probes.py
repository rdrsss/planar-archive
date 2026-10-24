"""Compile production-codec checks; preserve the known R1 red suite (exit 42)."""
from pathlib import Path
import json, shlex, subprocess, time, xml.etree.ElementTree as ET
root=Path(__file__).resolve().parents[3]
out=root/'tests/native/v5-02-x05-b2-b-c01-evidence'/__import__('sys').argv[1]
out.mkdir(exist_ok=False)
build=root/'build/rewrite-debug'
commands=[]
def run(label,args,expected_exit=0):
    start=time.time()
    with (out/(label+'.log')).open('w') as stream:
        result=subprocess.run([str(x) for x in args],cwd=build,stdout=stream,stderr=subprocess.STDOUT)
    commands.append(dict(label=label,argv=[str(x) for x in args],cwd=str(build),exit_code=result.returncode,seconds=round(time.time()-start,3)))
    (out/'probe-commands.json').write_text(json.dumps(commands,indent=2)+'\n')
    print(label,result.returncode,flush=True)
    assert result.returncode==expected_exit,label
compile_db=json.loads((build/'compile_commands.json').read_text())
original=shlex.split(next(r['command'] for r in compile_db if r['file'].endswith('json_codec/codec.t.cpp')))
for label, source, expected_cases in [('phase',root/'tests/native/v5-02-x05-b2-b-support/phase.cpp',2),('adversarial',root/'tests/native/v5-02-x05-b2-b-c01-evidence/adversarial.cpp',5),('controls',root/'tests/native/v5-02-x05-b2-b-c01-support/controls.cpp',2)]:
    cmd=original.copy()
    cmd[cmd.index('-o')+1]=str(out/(label+'.o'))
    cmd[-1]=str(source)
    run(label+'-compile',cmd)
    run(label+'-link',[cmd[0],'-stdlib=libc++','-nostdinc++','-isystem','/opt/homebrew/opt/llvm/include/c++/v1','-g','-arch','arm64','-Wl,-search_paths_first','-Wl,-headerpad_max_install_names','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++',out/(label+'.o'),'-o',out/label,'src/engine/libplanar_json_codec.a','vendor/catch2/src/libCatch2Maind.a','src/engine/lib@cmake_cxx_std.a','src/engine/libplanar_value.a','vendor/catch2/src/libCatch2d.a'])
    run(label,[out/label,'--reporter','junit','--out',out/(label+'.xml')],42 if label in {'adversarial','controls'} and __import__('sys').argv[1]=='red' else 0)
    suite=ET.parse(out/(label+'.xml')).getroot()
    cases=suite.findall('.//testcase')
    assert len(cases)==expected_cases
    totals=suite.find('testsuite').attrib
    assert int(totals['errors'])==0 and int(totals['skipped'])==0
    failures=int(totals['failures'])
    assert failures==((7 if label=='adversarial' else 2) if label in {'adversarial','controls'} and __import__('sys').argv[1]=='red' else 0)
    failed_cases=sum(c.find('failure') is not None for c in cases)
    assert failed_cases==((2 if label=='adversarial' else 1) if label in {'adversarial','controls'} and __import__('sys').argv[1]=='red' else 0)
    summary=dict(cases=len(cases),assertions=int(totals['tests']),failed_cases=failed_cases,failed_assertions=failures,errors=0,skips=0,expected_red=label in {'adversarial','controls'} and __import__('sys').argv[1]=='red')
    (out/(label+'-summary.json')).write_text(json.dumps(summary,indent=2)+'\n')
