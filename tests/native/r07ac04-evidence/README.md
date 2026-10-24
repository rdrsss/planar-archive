# R07-A-C04 native count-loop correction evidence

Candidate implementation; independent review pending. Ownership is limited to
src/kernel/runtime/runtime.cpp, runtime.t.cpp, tests/stdlib/bridge.lua,
stdlib/planar/README.md and this evidence directory. The checkout is shared;
others' edits were preserved. No Planar tools, installed binaries, legacy queue,
commits, installation or publication were used.

## Failing-before-fix evidence

before-runtime.cpp preserves the exact effective C03 runtime before this fix.
red-build.log builds the new probes against that unchanged runtime.
red-regressions.log records exit 42: one case / 11 assertions / seven failures.
All three subprocesses exhausted the 400 x 10 ms polling watchdog and were killed
with SIGKILL. Elapsed time was 17.042 seconds; scheduling can extend the polling
bound. No huge native operation ran in the test parent.

Each probe uses count 1152921504606846976. Direct string.rep and method rep use
empty bytes and an empty separator; table.move copies nil entries in an empty
table over indices 1 through count. These retain the native zero-output loop
conditions, rather than relying on eventual allocation failure. Capability
absence also failed before the fix. The same probes must return an internal
absent-function failure after the fix, within the subprocess watchdog.

## Correction and preserved coverage

The curated table library removes move; the string library removes rep alongside
the existing forbidden entries. Lua's string metatable indexes that library
(table installed by lstrlib.c1872), so method rep is removed as well. The
capability fixture explicitly checks table.move, string.rep and method rep.
No product consumer used either capability.

The fixture helper repeat_byte builds bytes by explicit Lua doubling and then
slices to the requested length. Its loop is logarithmic and VM-metered; the
strings remain real Lua allocations. The 600 MiB allocation requests still fail
with lua_bytes; cumulative native hash conversion and amplified native return
conversion still fail with native_bytes. The full runtime suite verifies those
specific diagnostics, including escaping errors and caught-conversion accounting.
The old pattern probes still construct exactly 100 or 50,000 'a' bytes before
calling the absent pattern functions. The small positive repeated-byte example
uses an explicit Lua loop and still checks ab-ab-ab. All seven positive string
checks and ordinary protected-call/native-error tests remain.

## Related native-loop source audit

Pinned sources below are under vendor/lua/0b10/src. This audit does not claim
native VM instruction metering or a wall-clock deadline.

- lstrlib.c139–160: rep can execute n-1 native iterations with zero result bytes;
  removed. ltablib.c126–154: move can iterate a numeric range of absent entries;
  removed.
- lstrlib.c96–132: reverse/lower/upper allocate and visit actual input bytes.
  byte (164–181) clips to string bounds and reserves result stack; char
  (185–194) uses actual arguments. sub/len do not introduce count-driven loops.
- lstrlib.c1613–1738 and 1771–1838: pack/packsize/unpack parse actual format bytes;
  packing padding allocates output, unpack validates input size and reserves
  result slots. Numeric format sizes do not create zero-output repetition.
- ltablib.c70–117 and 340–399: insert/remove/sort use computed table lengths;
  sort rejects lengths at INT_MAX and comparison callbacks execute Lua normally.
  ltable.c1230–1268 randomizes sparse length-search probes specifically to avoid
  small tables constructed from fixed probe keys yielding huge lengths. This
  audit does not equate sparse table length with its nonnil element count.
- ltablib.c169–188: concat requires an actual string/number at every visited
  index, otherwise it errors immediately. pack uses actual argument count;
  unpack (216–232) rejects oversized counts and reserves its result stack before
  iterating. Table creation allocates requested storage before returning.
- lutf8lib.c85–140, 154–170, 177–237: traversal is bounded by actual string bytes
  even for large offset counts; codepoint/char use result/argument stack bounds.
  Base numeric parsing scans actual bytes; exposed math functions do not have
  analogous scalar-count repetition loops.

## Executed verification

Commands ran directly and serially in build/rewrite-debug. Each command was also
wrapped with a finite external subprocess timeout (30 seconds focused probes,
120 seconds runtime, 180 seconds build/native/lint, 60 seconds final build).

```sh
cmake --build build/rewrite-debug --target planar_runtime_tests
build/rewrite-debug/src/kernel/planar_runtime_tests '[r07ac04]'
/opt/homebrew/opt/llvm/bin/clang-format -i src/kernel/runtime/runtime.cpp src/kernel/runtime/runtime.t.cpp
cmake --build build/rewrite-debug --target all planar_tests
build/rewrite-debug/src/kernel/planar_runtime_tests '[r07ac03],[r07ac04]'
build/rewrite-debug/src/kernel/planar_runtime_tests '[runtime]'
ctest --test-dir build/rewrite-debug -R 'replacement.(runtime|loader|capture|canonical|hash)' --repeat until-fail:2 --output-on-failure --output-junit <repo>/tests/native/r07ac04-evidence/native.xml
python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug
cmake --build build/rewrite-debug --target all planar_tests
```

Raw logs retain exit codes and elapsed seconds. Focused: three cases / 57
assertions in 0.195 seconds. Runtime: 16 cases / 418 assertions in 6.030 seconds.
Native: 40 selected cases, each passed twice in fresh processes, zero failures
or skips; native.xml contains 40 cases with no skipped entries. The raw CTest
transcript is native-ctest-lasttest.log. Pinned formatting and Doxygen checked
21 active sources. Final build: ninja: no work to do.

candidate.sha256 freezes the four owned source/reference files; its SHA256 is
0f0902d1631bc258bfb4e264f6859ebcc32adde2686105aeba1262c322cc0007.
Host evidence is macOS arm64 debug only. Linux, release, sanitizers and the full
CLI gate were not exercised. No self-approval is claimed.
