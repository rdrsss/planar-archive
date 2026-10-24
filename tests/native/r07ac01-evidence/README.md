# R07-A-C01 corrective evidence

Implementation candidate; independent review is pending. Only runtime.cpp,
runtime.t.cpp, tests/stdlib/bridge.lua and stdlib/planar/README.md were changed,
plus this new evidence directory. Shared checkout and other agents' changes were
preserved. No Planar/queue/installer, installed binary or commit was invoked.

The four regression cases first ran against the unchanged R07-A runtime:
`red-build.log` succeeded and `red-regressions.log` records four failed cases,
11 failed assertions. The finalizer child was killed after its two-second bound;
conversion repair was refused; caught native errors masked instruction/memory/
ordinary errors and an older rethrown diagnostic; nontext native immaterial input
was accepted. The first wrapper test also had an invalid declaration-root fixture
(`planar.immaterial` is not a legal root). That fixture was corrected to a spec's
`note` field before green execution, adding both md/file valid round trips and
Lua-mutated invalid output checks. The original red log is retained unmodified.

Implementation removes setmetatable with no available alternate finalizer
creation route. Scoped traversal membership unwinds during C++ failures while
node/byte counters remain cumulative. Native callback errors use opaque userdata
with private uservalue metadata; Lua's allocator charges the token/metadata and
Lua GC reclaims them. No native pointer map or error-token vector persists. Token
creation and Lua error raising happen after native callback temporaries unwind.
Caught/rethrown older tokens retain their original diagnostic. Native input and
converted return wrappers both require an md/file payload.

The harness additionally catches 100,000 native failures before successful return
and proves cumulative conversion charging by repeatedly catching failures after
converting a 1 MiB field until native_bytes is exhausted. The original
`green-regressions.log` used 600,000 small failed conversions; the final equivalent
uses 600 larger failed conversions and is faster. The bounded finalizer probe
runs only in a forked test child; the production export table is unchanged.

Executed serial build commands from repository root:

```sh
cmake --build build/rewrite-debug --target all planar_tests
build/rewrite-debug/src/kernel/planar_runtime_tests '[r07ac01]'
ctest --test-dir build/rewrite-debug -R 'replacement.(runtime|loader|capture|canonical|hash)' --repeat until-fail:2 --output-on-failure --output-junit <repo>/tests/native/r07ac01-evidence/final-native.xml
python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug
cmake --build build/rewrite-debug --target all planar_tests
build/rewrite-debug/src/kernel/planar_runtime_tests '[runtime]'
```

`final-build.log`: success. `final-native.log`/XML: 35 matched native cases,
each passed twice in fresh processes, zero skips, 11.76 seconds total.
`final-runtime.log`: all 11 runtime cases and 336 assertions pass.
`final-lint.log`: pinned formatting and Doxygen pass, 21 active sources.
`final-no-change.log`: `ninja: no work to do.`

The four candidate source/document files are identified by candidate.sha256.
This is macOS debug evidence only; no Linux, release or sanitizer result is
claimed. This round does not implement schema/projection/CLI rendering policy
and does not record self approval.
