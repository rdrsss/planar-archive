# X05-A0 test-only lexical sealing feasibility candidate

Status: implementation candidate for fresh independent review; not self-approved. The shared checkout was dev/planar-v2-vernacular at starting HEAD 3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a. No production integration, public profile API, CLI change, release build or Linux result is claimed. No commit/install/publish/merge or Planar operational tools were used.

Implemented only the new test support, narrow engine CMake wiring, this evidence, and candidate-only status paragraphs in the seven permitted reference files. [Baseline diff](attempt01/baseline-diff.json) verifies production runtime, shipped Lua, vendor and every prior test/fixture are byte-identical to the pre-edit baseline. AGENTS.md remains the CLAUDE.md symlink. Default `all` excludes the new audit/variant libraries and test executable; `planar_tests` builds them.

The audit reads actual pinned Lua 5.5 LClosure/UpVal/Proto metadata through existing private headers. Every retained actual Lua closure is independently inspected with every actual upvalue cell sealed. Descendant prototype flags inherit only through instack=false; instack=true captures invocation-local scratch. OP_SETUPVAL refuses only a sealed target. Reachable actual Lua helpers are additional roots, including helper functions in table keys/values/upvalues. Table/function identities preserve aliases and terminate cycles; distinct shared UpVal identities are counted. This is conservative rejection, including unreachable branches, not bytecode execution or a write guard. Existing proxy freezing remains responsible for table mutation; an explicit test demonstrates that lexical inspection alone permits a table-field assignment.

Graph edges/depth, prototype inspections, inspected words, Lua stack headroom and a fixed native PMR arena bound inspection. Default arena is 4 MiB with null fallback; graph/prototype limits and the arena refusal restore stack top. The experiment bounds each audit call independently: it does not establish whole-registration or whole-invocation shared accounting across roots/module evaluation/conversions. That remains an X05-A integration obligation.

The generated proof snapshots exact native function identities in a strong private Lua registry catalog immediately before module execution. It adds only the three digest and two JSON native callbacks at their explicit reviewed native installation sites after module execution. C roots refuse even if known. Reachable C functions require exact identity membership, so the same C function pointer with different closure/upvalues refuses. Native-owned contexts in those exact reviewed functions remain opaque; this is an explicit trusted installation policy, not arbitrary module-controlled whitelisting or semantic-honesty proof.

`string.gmatch` is removed by the real curated runtime. The adversarial standalone source-only test deliberately opens the pinned string library to create a retained stateful iterator and prove unknown C closure refusal; a separately invocation-created iterator is allowed and runs. This test does not add gmatch to the product. Known stable helpers and callback contexts come from the real pre-module curated VM in the compatibility variant. No safe-classification gap was observed within that exact curated installation; future native installs need explicit classification and review.

The identity snapshot may allocate Lua tables and runs only inside a protected C trampoline with trivial locals, including its recursive fixed native frames. The low-memory test forces LUA_ERRMEM in that protected phase. Inspection itself reserves no Lua capacity, calls no functions/metamethods and uses only nonallocating raw table/upvalue/stack reads after the caller reserves 2048 slots. No C++ destructor-bearing object crosses a Lua allocating longjmp. Native arena exceptions are caught outside Lua and returned as typed refusals. This depends on source-compiled valid Lua metadata and the pinned nonallocating API paths; it is not an arbitrary bytecode admission API.

Two generated runtime/interface variants apply count-asserted module/import/namespace replacements plus catalog-install/audit injections. Every root is audited before the unchanged freezer mutates any table. [Variant audit](attempt01/runtime-variant-audit.json) independently checks forward substitutions, exact reversal and source hashes. Variant 0 uses the exact ten-file shipped bundle, and its generated embedded source module is byte-identical to production apart from module name. Its native command_vm::create accepts the actual five handlers and reachable helper graph. Variant 1 changes only commands.lua by adding one retained scalar and one init-handler write; its exact reversible Lua change is recorded and native creation refuses. No production runtime source or public runtime API changed.

Pinned opcode handling is backed by vendor/lua/0b10/src/lvm.c:1256-1260 (LOADKX), :1407-1419 (NEWTABLE always skips one payload, including k=0), and :1901-1913 (SETLIST skips one iff k). The source emitter is lcode.c:461-467, :1877-1881 and :1892-1902. No hand-built instructions, binary-chunk parser or vendor modifications were used. Pinned SIZE_Bx is 17 (lopcodes.h:42-48), so the first LOADKX needs constant index 131072, i.e. at least 131073 distinct constants in one prototype. The fixed-width base62 string source fixture reaches that threshold under a bounded Lua allocator and executes its expected result.

Observed isolated LOADKX fixture: 786,487 source bytes (limit 1,048,576), 27,054,318 peak live Lua bytes (limit 67,108,864), 2 actual LOADKX instructions and 136,304 inspected words. The extra numeric lookup constant also uses LOADKX. A 1200-element constructor exercises NEWTABLE plus SETLIST(k), and an empty constructor exercises NEWTABLE with k=0. No opcode case is represented by a skip. This isolated memory result does not prove shared multi-module registration budget behavior.

Frozen Debug verification completed 44/44 focused cases: 12 new audit cases and 32 existing command cases, zero failures/errors/skips/disabled cases. Inventory is 532 distinct registered names, preserving all 520 prior names. No full 532-case suite was run; the final integrated X05 gate remains future work. [Raw JUnit](attempt01/focused.xml), [test summary](attempt01/test-summary.json), [inventory](attempt01/inventory-preservation.json), [commands and timings](attempt01/commands.json).

Canonical, record-v1, request-v2 and effective-provenance vector checks passed. Historical B2, D6, D6-C01 and D7-B source audits passed. Pinned formatting/Doxygen passed on all 113 active product sources and separately on all four new support C++ files. The final all+planar_tests build reported `ninja: no work to do.`. New support Doxygen is explicitly configured in the verification driver because generic product lint excludes tests/native.

Initial evidence is preserved: configure-initial/build-initial failed the generator's exact replacement assertion because a JSON installation pattern was ambiguous; the pattern was narrowed to its unique post-module native installation block. initial-audit.xml/log records 11/12 passed and one fixture assertion failure: 600 elements correctly did not emit SETLIST(k), whose pinned vC is 10 bits. The source fixture was increased to 1200, and second-audit.xml/log plus frozen final evidence pass. doxygen-preflight.log records missing exported field docs; those docs were added and the strict gate passes. These are retained development attempts, not unexplained skips or detector weakening.

Baseline contains 18,584 regular/symlink entries; final frozen source contains 18,590. Only root .git/build and this new evidence directory are excluded, with nested names retained. Frozen source was hashed before and after every recorded command; source-stability.json reports exact equality. Candidate snapshot SHA256: `fb30a7afb6727652498b8e4cefb41d21e983dc6a75d161427f7122b777e23cf8`. baseline.json, originals/, candidate-source/, source-freeze.json, build-identities.json and the artifact manifest retain the candidate and raw evidence.

This establishes bounded feasibility only. Live external source execution/registration, native source owners, descriptors/shape/projection admission, resistance to later-module alias mutation, cumulative shared budgets and final integration review are still required. The mandatory external-profile gate, public cutover, storage/history profile dispatch, execution/RA01 and Linux acceptance remain open; public init remains format1.

Executed command timings (seconds; full argv and UTC start/end in commands.json):

| Check | Seconds | Exit |
| --- | ---: | ---: |
| configure | 7.665 | 0 |
| build | 29.983 | 0 |
| inventory | 0.141 | 0 |
| focused | 37.758 | 0 |
| loadkx-details | 0.120 | 0 |
| canonical-vectors | 0.669 | 0 |
| records-vectors | 0.642 | 0 |
| request-v2-vectors | 0.652 | 0 |
| effective-provenance-vectors | 0.773 | 0 |
| historical-audit | 0.400 | 0 |
| d6-audit | 0.236 | 0 |
| d6-c01-audit | 0.147 | 0 |
| d7-b-audit | 0.196 | 0 |
| pinned-product-lint | 1.319 | 0 |
| pinned-support-format | 0.044 | 0 |
| pinned-support-doxygen | 0.038 | 0 |
| no-work | 0.250 | 0 |

Total measured command time: 81.034 seconds. Full-tree hashing between commands is additional elapsed verification time.
