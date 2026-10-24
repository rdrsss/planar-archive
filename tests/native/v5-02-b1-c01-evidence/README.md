# V5-02-B1-C01 coder evidence

Candidate is a correction awaiting independent review, not coder approval.
Shared checkout at HEAD 3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a.
No commit, installation, network, old Planar tool, installed binary or operator
DB was used. Debug was exclusively held for serialized direct verification.

Six owned files are frozen in `candidate/`, with baseline snapshots in
`baseline/`, hashes in `candidate-files.json`, and exact changes in `owned.diff`.
No runtime interface, shipped commands.lua, golden or prior evidence changed.
`baseline-sha256.json` covers the initial tree excluding .git/build/new C01
artifacts. `audit.py` verifies all protected files and the AGENTS.md symlink.

The original runtime is frozen in `red/runtime.cpp`, alongside regression source,
generator, CMake, build/configure logs, JUnit and LastTest output. `red/sha256.json`
freezes that evidence. All three initial deterministic cases failed: a captured
field could be changed; captured rawget was unavailable for ordinary reads; and
257 levels were accepted. No stress program was used. Final test strengthening
adds an isolated helper-only captured table to avoid relying on a directly
captured alias for transitive reachability, plus a fourth returned-table case.

Implementation uses a fixed 256-element trivial native frame array, private Lua
memo/backing maps, and iterative traversal of table keys/values and trusted Lua
closure upvalues. Global/registration tables and all retained callable refs are
roots. Opaque C closure upvalues are excluded. Each edge (including repeated
aliases and scalar leaves) consumes existing cumulative node/native-byte budget;
512-byte per-visit charging also covers each 12-byte frame. Every iteration
checks twelve slots of live stack headroom and capacity before its bounded
pushes. The 256-frame bound follows the existing depth ceiling, without a lower
undocumented cap. Lua maps/backings are under the 64 MiB registration allocator.
Typed depth refusal at 257 is deterministic; no recursive native helper is added.

Private next/rawget/pairs wrappers are installed before bundle evaluation, so
captured aliases remain useful after proxy sealing. Original identities, #,
ipairs, shared/cyclic aliases and function/table keys are retained. Native result
and codec conversion read private backings with original object tags/identity;
new scratch tables remain mutable. Test-only generator variant 0 removes exactly
`rawget or` from the historical fixture's absence assertion because C01 requires
read access. The historical altered.lua is unchanged; rawset/getmetatable/
registration absence and all write-denial assertions remain. Positive tests read
frozen/ordinary rawget contents and encode a frozen tagged array nested in an
ordinary object. No externally reachable source-injection API is added.

Commands were executed directly and serially:

- `cmake --preset debug`, then
  `cmake --build build/rewrite-debug --target all planar_tests`.
- `ctest --test-dir build/rewrite-debug -R 'replacement.(command|runtime|loader|provenance|interpretation|acceptance prepared|cli$|init_cli$|history_cli$|export_cli$)' --output-on-failure --output-junit <absolute evidence>/verified.xml`:
  87 passes, zero failures/skips, including the prior 83 and four C01 cases.
  JUnit/LastTest were archived before querying inventory.
- `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`:
  formatting and Doxygen pass over 87 sources.
- `python3 tests/vectors/generate.py --check`: 12 byte, 20 hash, 41 domain vectors.
- Repeating `cmake --build build/rewrite-debug --target all planar_tests`:
  `ninja: no work to do.`
- `ctest --test-dir build/rewrite-debug --show-only=json-v1`, then
  `python3 tests/native/v5-02-b1-c01-evidence/audit.py`:
  all 354 previous names preserved, 358 total; all 21 runtime/interface/bundle
  variants and original fixed-export variant audited; 13,488 protected files
  unchanged.

The initial focused run's one failure was the rawget-absence fixture mismatch;
`focused-initial.*` retains it. `pre-strengthen.*` retains the first successful87
run before helper-only coverage strengthening. `verified.*` is final.
This is a focused debug macOS result. No full358 run, release/Linux run, CLI
composition, workspace naming, full load or external-profile support is claimed.
B2 owns the integrated composition/full gate.
