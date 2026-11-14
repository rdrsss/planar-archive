# R20-CARRY-A: TIMEOUT headroom guard for the two named slow replacement tests (I1-I3).
"""argv: case, build directory, ctest path.

The tests are identified by exact registered name, never number. The guard reads the registered TIMEOUT property with
`ctest --show-only=json-v1`, runs no test and edits no CMake file. A missing name is a distinct failure from a low limit.
The per-test minima equal the D32 limits exactly (1680 for the v2 allocation test, 840 for the above4096 test), so a limit
one below (1679, 839) fails as well as a revert to 120. Lowering either limit requires a deliberate edit of this guard.
"""
from pathlib import Path
import json,sys
sys.dont_write_bytecode=True
from common import expect,run,dispatch
V2='replacement.v2 every native allocation failure remains a redacted typed resource refusal'
ABOVE='replacement.recorded effective history selects wider fixed tree profile for valid closure above4096 files'
MINIMUM={V2:1680,ABOVE:840}
TOTAL_FLOOR=1872
build=str(Path(sys.argv[2]).resolve());ctest=sys.argv[3]

def registered():
 r=run([ctest,'--test-dir',build,'--show-only=json-v1'],timeout=120)
 expect(r.returncode==0,f'ctest --show-only=json-v1 exit {r.returncode}: {r.stderr[:400]}')
 return json.loads(r.stdout)['tests']

def limit_of(name):
 tests=registered()
 matches=[t for t in tests if t['name']==name]
 expect(len(matches)>0,f'NOT REGISTERED: {name!r} is absent from {len(tests)} registered tests')
 expect(len(matches)==1,f'{name!r} registered {len(matches)} times, expected exactly once')
 limits=[p['value'] for p in matches[0].get('properties',[]) if p['name']=='TIMEOUT']
 expect(len(limits)==1,f'{name!r}: expected one TIMEOUT property, found {limits}')
 return float(limits[0])

def headroom(name):
 limit=limit_of(name)
 shown=int(limit) if limit==int(limit) else limit
 minimum=MINIMUM[name]
 expect(limit>=minimum,f'{name}: TIMEOUT {shown} < {minimum}')

def timeout_headroom_v2_allocation():headroom(V2)
def timeout_headroom_above4096():headroom(ABOVE)

def timeout_names_registered():
 tests=registered()
 for name in (V2,ABOVE):
  count=sum(1 for t in tests if t['name']==name)
  expect(count==1,f'{name!r} registered {count} times, expected exactly once')
 expect(len(tests)>=TOTAL_FLOOR,f'registered total {len(tests)} < {TOTAL_FLOOR}')

dispatch({'timeout_headroom_v2_allocation':timeout_headroom_v2_allocation,'timeout_headroom_above4096':timeout_headroom_above4096,
 'timeout_names_registered':timeout_names_registered},sys.argv)
