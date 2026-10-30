"""X08 CTest timeout headroom guard for the named prepare_load replacement test (Q7 A, decision 10).

argv: ctest case, production planar, build directory. The test is identified by its exact CTest name, never by its
number: the number changes whenever the suite is regenerated. The guard reads the registered TIMEOUT property with
`ctest --show-only=json-v1`, runs no test and edits no CMake file. It asserts the TIMEOUT is at least 120 seconds and
the registered suite still holds at least 1227 tests.
"""
from pathlib import Path
import json,subprocess,sys
case,planar,builddir=sys.argv[1],str(Path(sys.argv[2]).resolve()),str(Path(sys.argv[3]).resolve())
NAME='replacement.X06 B3 C a file replaced at any allocation inside prepare_load is never published as verified'
def timeout_headroom_531():
 assert Path(planar).is_file(),planar
 listing=subprocess.run(['ctest','--test-dir',builddir,'--show-only=json-v1'],capture_output=True,timeout=60)
 assert listing.returncode==0,(listing.returncode,listing.stderr[:400])
 tests=json.loads(listing.stdout)['tests']
 assert len(tests)>=1227,len(tests)
 matches=[t for t in tests if t['name']==NAME]
 assert len(matches)==1,len(matches)
 limits=[p['value'] for p in matches[0].get('properties',[]) if p['name']=='TIMEOUT']
 assert len(limits)==1 and float(limits[0])>=120,limits
CASES={'timeout_headroom_531':timeout_headroom_531}
if case not in CASES:raise SystemExit('unknown case '+case)
CASES[case]()
print(case,'passed')
