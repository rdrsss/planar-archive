# R07-A-C04 independent review

Disposition: **request-changes**. C04 fixes table.move/string.rep, including string method access. Prior finalizer, conversion membership, native diagnostic identity, immaterial-wrapper, protected-call quota and pattern removals remain intact on source review and regression verification. Full effective R07-A bridge is not approved because the remaining native table helpers violate the required data bound.

## High: sparse table lengths still drive unmetered native shifts/sort

src/kernel/runtime/runtime.cpp:371 opens the complete table library and removes only move. It leaves insert, remove and sort. The state is created with fixed hash seed zero at line 439.

vendor/lua/0b10/src/ltable.c:1232 explicitly warns that a fixed hash-search algorithm permits a small table with huge length; lines1240-1251 take randomness directly from the state seed. Seed zero makes the probes deterministic powers of two for a zero-array table. `table.create(0,128)` followed by true entries at powers of two from 1 through 2^60 has only 61 entries but length 2^60. A bounded pinned-library probe verified that exact length and entry count twice. No huge shift was executed.

vendor/lua/0b10/src/ltablib.c:70-117 uses that computed length as insert/remove's native loop endpoint. Nil-to-nil assignments do not require proportional allocation (ltable.c914-925), so the 512 MiB allocator and 100 million Lua instruction limits do not bound these loops. The earlier hash_search mitigation argument is invalid for this runtime's seed, and randomness would not establish a pure-data bound anyway.

Sort also requires correction. ltablib.c390-399 rejects n>=INT_MAX, but a sparse 31-entry table can have length 2^30, below that rejection. sort_comp at271-287 permits any function, including exposed native `type`. `table.sort(t,type)` repeatedly returns truthy strings and partition's first scan at302-307 visits the native range until reporting invalid order. No Lua instructions are executed by that comparator. Do not assume all comparison callbacks execute metered Lua.

A separate bounded probe used 17 entries and length 65536. insert(t,1,false), remove(t,1), and sort(t,type) each executed with a count-one VM hook installed immediately before the native call. All recorded zero VM instructions; peak Lua allocation was 11600 bytes for shifts, 11716 for sort. Sort returned invalid-order error after its native scan. These completed twice under finite 5 second subprocess timeouts. Only lengths and small native operations were executed; no arbitrary huge-operation timing claim is needed for the source finding.

Recommended correction: remove unused table.insert, table.remove and table.sort with absence tests and regression evidence, or replace them with explicitly bounded/metered helpers. Future ordered product operations can use metered Lua. Update the README/evidence audit to state the actual restriction; do not use randomized length search or INT_MAX as proof of proportional storage bounds. concat requires a real string/number at every index; unpack reserves result stack before iteration, and create allocates requested capacity, so they do not have this sparse nil-range failure mode.

## Independent verification

All commands ran serially in the shared build/rewrite-debug directory under finite external timeouts. `cmake --build ... --target all planar_tests` passed with no work before and after. Runtime [runtime] passed 16 cases/418 assertions. CTest replacement.(runtime|loader|capture|canonical|hash) passed 40 cases, zero failures/skips (native.xml checked directly). Pinned lint passed formatting/Doxygen over 21 active sources. Raw commands, exit codes, durations and logs are retained beside this report.

Before/after composite manifests covered 12 distinct effective files with no mismatch. C04 manifest SHA256: 0f0902d1631bc258bfb4e264f6859ebcc32adde2686105aeba1262c322cc0007. Repository implementation files were read-only; only scratch probes/evidence and routine build-generated files were written. No Planar tools, old host queue, installed binaries, commits, install or publish actions.

Limits: macOS arm64 debug only. No Linux/release/sanitizer or full CLI gate. Standalone bounded probes link the exact pinned built Lua libraries and mirror the runtime's seed/library entry points; they are source confirmation, not injected production exports. No schema/CLI approval.
