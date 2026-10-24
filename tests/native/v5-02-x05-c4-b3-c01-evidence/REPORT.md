# X05-C4-B3-C01 focused Debug corrective candidate

Combined B3+C01 awaits fresh independent review of C4B3-R1. This record does
not approve the candidate. Shared dev/planar-v2-vernacular, starting HEAD
3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a, intentional dirty rewrite.
Coordinator owns the round. Only runtime.cpp, external_registration.t.cpp,
listed current reference prose and new C01 support/evidence are edited. Original
B3 candidate/review evidence is preserved. No commits, install, publish, merge,
release rebuild, installed binary, Planar tools or operator data were used.

The actual live invocation context carries an exact native identity in the
existing synchronous bridge. It remains strongly rooted by the protected input
stack and registry backing until execute cleanup clears the complete bridge,
before releasing those roots. Both ordinary index and supported proxy-aware
rawget apply the same missing-known-service rule to this identity only. Missing
read_history/read_role/project returns nil with sticky protocol refusal, including
lookup without a call and caught nil-call tails. Ordinary scratch and frozen
retained tables keep their existing behavior, including service-named data keys.

Installation explicitly retrieves the context's registry backing and uses that
one table for actual methods and the index closure. Thus history contexts expose
exactly read_history through indexing, rawget, next and pairs; pure contexts
expose none. Enumeration does not poison. Captured aliases use the same native
closures. Numeric lookup/ipairs/table concatenation retain ordinary empty-array
behavior. Other frozen-table iteration and library APIs are unchanged. No denied
callables or additional capability is introduced.

Pinned compiled comparison is command_state1008→1016, VM1416→1424, bridge32→40;
invocation488 stays unchanged. The existing initial sizeof(bridge) charge rises
by exactly8 bytes before allocation/use; no reset, new quota or full X06 accounting
is claimed. The bridge's existing RAII reset clears identity on success, protocol,
native and Lua memory exits. Protected callbacks keep only trivial live locals;
original native error precedence, once-only service, lazy no-read semantics,
original receipt verification and raw history correspondence remain B3 behavior.

The production regression was added and run before changing runtime.cpp:
regression-red.xml fails one actual retained pure handler because execute accepts
its summary after caught rawget of a missing method. The exact runtime/test
snapshot is retained in regression-red-sources. Private baseline-reviewer.xml
runs the original reviewer source against copied pre-C01 B3 production and fails
the same two cases/five assertions (history missing role/project; pure missing
history/role/project), with zero SQLite reads. The final copied candidate passes
all five original reviewer cases, including deep correspondence, repeated calls
and final receipt mutation.

Four colocated production cases add raw/ordinary missing lookup and caught-call
matrices, semantic/summary pure tails, history semantic tails, actual rawget of
available history in both formats, exact one/zero-method next/pairs, captured
aliases and ordinary same-name scratch/frozen table controls. The final focused
Debug gate is429 cases:304 runtime,82 format,33 SQLite,3 history-read,7 CLI.
Inventory790 preserves all786 B3 names. Zero failures/errors/skips/disabled are
required and recorded in the raw JUnit and verification-summary.json.

Private green coverage totals13 cases: one original lazy history success,
seven B3/private boundary cases (including new actual context-install Lua
allocation refusal), and five original reviewer cases. Allocation refusal occurs
after exact identity installation, before handler entry: sticky resource refusal,
zero SQLite reads and cleared owner/context identity/input/push memo are checked.
Existing service allocation/longjmp probes also explicitly check identity cleanup.
Copied dependency inputs cover every built object/PCM/archive/dylib plus referenced
standard-library PCMs and LLVM libc++ dylibs, recursively expand response files,
and disable implicit modules/module maps. Membership/mode/byte hashes are checked
before and after each variant; cleanup.py verifies exact owned removal membership
before deleting only disposable copies/products. Sources, raw commands, logs,
XML, layout and removal inventories remain. macOS system libraries remain host
prerequisites. The earlier private-attempt1 snapshot and cleanup proof remain.

Final frozen-candidate.json covers product/tests/support/prose and this report.
Each final gate command checks that identity before and after. gate-results.json
retains exact argv/cwd/status/time. Six common vectors, historical/current source
and generated variant audits, pinned formatting/Doxygen over118 active sources,
no-work and final protection pass only if gate.py completes. Protection covers
all baseline enumerated source/vendor/build tooling/fixtures/reference/test and
installed-release membership/modes/bytes outside owned paths and the coordinator
round. AGENTS.md remains the CLAUDE.md symlink and rewrite-release remains absent.
Artifact manifest and finalization records bind the completed evidence afterward.

Attempt1's test-only compile typo is preserved in attempt1-sources and its build
log. The subsequent test invocation used the old executable after that failed
build; green-attempt1.xml repeats the red regression and is not green evidence.
Green-attempt2 passes four cases before expanded ordinary-index/table controls;
final gate supersedes that preliminary test scope. All failed attempts remain.

This is focused macOS Debug evidence. Latest complete approved gate remains676
per configuration. Public init creates format1. Role/projection services and
combined capabilities, X06 profile interpretation/history acceptance/shared
accounting, X07 cutover, mandatory X08 external-profile acceptance, execution/RA01
and Linux remain open. No full or release gate is claimed.
