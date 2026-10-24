# R07-A-C02 independent review

Disposition: request-changes. The quota-propagation correction and four prior fixes pass, but the effective full trusted runtime still permits native pattern matching to bypass its execution budget.

## Finding P1 — native Lua patterns bypass execution quota

`src/kernel/runtime/runtime.cpp:373-378` opens the string library and removes only dump/format. It leaves string.find (pattern mode), string.match, string.gmatch and string.gsub callable, also through string method lookup. These functions execute native recursive/backtracking loops without VM instruction hook calls. `vendor/lua/0b10/src/lstrlib.c:508-520` max_expand repeatedly calls match; `str_find_aux` at 776-819, `gmatch_aux` at 841-854, and `str_gsub` at 945-990 all enter this matcher. The matcher depth cap protects C stack depth, not total work. `runtime.cpp:439` installs a Lua VM count hook; it cannot stop this native work, and host checks after return do not help while it runs.

Minimal embedded trusted export: `function() return string.find(string.rep('a',100),'a*a*a*a*a*b') ~= nil end`. It exceeded four seconds twice. The 50-byte variant completed successfully in ~0.55-0.88 seconds, already comparable to consuming the entire 100-million VM-instruction budget, despite executing only a handful of Lua instructions. The 50,000-byte variants using find/match/gmatch/gsub each exceeded four seconds twice. The same find with explicit plain=true completed successfully in ~0.011 seconds twice. No quota diagnostic occurred in the timeout cases; subprocesses were killed by the reviewer timeout.

This is a resource-bound defect in buggy trusted code, not a claim that a caller can inject executable code. The fixed embedded source was substituted only in a scratch-linked reviewer harness; production runtime source/archive stayed unchanged.

Requested narrow correction: remove match/gmatch/gsub and remove find or expose only a curated plain-search form. Ensure original functions cannot still be reached through string methods. Schema code can use explicit byte loops and plain search. Add bounded subprocess regressions for these native paths and retained failing-before-fix evidence. Broader native matcher fuel would also solve the issue but is unnecessary for the current product surface.

## Positive review

C02 curated pcall checks the latched quota before returning to Lua and has only trivial locals across lua_pcall/luaL_error. Removing xpcall closes the hook-disabled handler path; host checks both successful and failed returns. Independent caught/repeated/64-level nested calls all terminate resource_limit/instructions. Close-handler loops through the mutable string metatable, including error/quota unwinding and nested caught quota, also terminate correctly.

Original four fixes remain effective: no table finalizer capability, RAII active traversal cleanup, opaque Lua-owned diagnostic identity, and structural md/file-only immaterial payloads. Independent nested conversion repair and old-token rethrow probes pass. Ordinary protected-call result/error identity and nil/multiple return arity pass. Native allocation, malformed return-count and capability probes behave as documented. Caught allocation failure may recover within the still-enforced memory ceiling; no bypass of allocated-memory accounting was found.

No additional longjmp lifetime defect was found: setup/push and callback raising frames keep trivial locals; conversion-owned C++ objects use nonallocating Lua reads and an internal protected anchor; hash callback errors allocate/raise only after native temporaries unwind.

## Verification

- cmake --build build/rewrite-debug --target all planar_tests: no work before and after review.
- ctest --test-dir build/rewrite-debug -R 'replacement.(runtime|loader|capture|canonical|hash)' --repeat until-fail:2 --output-on-failure: 37 selected cases passed twice; zero failures/skips.
- build/rewrite-debug/src/kernel/planar_runtime_tests '[runtime]': 13 cases, 361 assertions passed.
- python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug: formatting and Doxygen passed for 21 active sources.
- Independent alternate embedded source against unchanged libplanar_runtime.a: 16 scenarios twice (results.json), then 6 pattern/plain variants twice (pattern-results.json). This includes expected failures/timeouts described above.
- Reviewed retained red-regressions.log and earlier review sources/results. The ordinary nil-error fixture correction matches pinned Lua 5.5 behavior; it does not weaken quota loops.

The original C02 four-file manifest SHA256 is 3524ade75167e515a4f9e365c01db03d1fd461ebe24f33c5089815c075f90cd9, verified before/after. All remaining effective full-bridge file hashes match the merged R07-A/C01/C02 manifests; effective-candidate.sha256 has SHA256 e473b80078adda0e5fb27b6238672e1a25d2a3dc4342a491318483832a7bb6cf. No implementation or repository source edits were made.

Scratch source/probe/commands/results are adjacent. invalid-probe-results.json preserves an initial scratch-source Lua syntax error (<close>= token adjacency), corrected before valid runs; it is not product evidence. native-ctest-lasttest.log preserves raw CTest output. macOS arm64 debug only; release/Linux/sanitizers not exercised. No CLI renderer, schema/selection behavior, persistence, installation or approval is inferred.
