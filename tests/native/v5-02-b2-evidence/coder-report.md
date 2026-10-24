# V5-02-B2 implementation candidate

Candidate only; independent review is pending. Shared checkout on
`dev/planar-v2-vernacular`, HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`.
No commit, installation, publication, merge, installed binary, operator ledger,
legacy Planar tool or queue was used. Both gates are complete and both debug/release build directories are idle;
exclusive coder ownership is released at this handoff.

## Composition

Bootstrap parses once through CLI11 with retained family option storage. The
sole separate-token syntax guard, fixed root help and all parsed-count/presence
usage checks finish before registration. Complete registration uses B1's actual
embedded retained VM; bootstrap compares all descriptors against compiled CLI11
names, requiredness, scalar/flag arity, numeric/text type metadata, aliases and
repeat policy. Typed CLI11 decimal bindings retain their validated original
canonical tokens and distinguish absence.

The four family handlers compose `planar.cmd.invocation` below bootstrap and
handlers. Its finite invocation-local providers reuse capture_session,
initialize_new, read_history and source_export. Render routing and semantic
projection execute inside the retained callable; no fresh native semantic export
path remains in command handlers or adapters. Init/history/export facts and native
failures remain checked by B1. Native failures become typed values directly,
without encoding and decoding transport text. No Lua callback runs inside native
storage operations or write locks.

Export preallocates publication and stdout diagnostics before native service
execution. The adapter sets its publication bit immediately after native success,
before any service-facts allocation. The response boundary combines that bit with
B1's retained publication state and preserves uncertainty through native facts
construction, Lua failure, invalid projection, caught exhaustion, encoding and
stdout failures. Existing safe staging-token diagnostics and native error
precedence remain intact.

## Explicit ownership amendment and regressions

The coordinator approved the minimal `src/engine/runtime/runtime.cpp` amendment
before editing. B1's route validator compared the parsed root ID before the real
capture service, turning a caller mapping mismatch into an internal defect.
The unchanged real CLI suite demonstrated this in `focused-cli.log`/JUnit/LastTest.
The generic bridge still validates root.id as text and retains exact route/root,
jobs, path, family and handle checks. The concrete native capture adapter performs
mapping equality first, returning the historical `reference`/`root` refusal before
spec/jobs/registry/source reads. A dedicated test pins that precedence with all
routed inputs missing. No runtime interface or engine primitive changed.

The separate `export service facts allocation failure retains adapter publication`
regression injects bad_alloc between native success and facts construction.
`publication-red-*` disables only the generated test adapter's publication bit
and demonstrates the wrong internal diagnostic after publication. The exact
restored generated source passes in `publication-green-*`. Both source snapshots
are retained. Existing response encoding and post-invocation allocation assertions
remain in their original test, with only the shared-boundary generator adaptation.

Six real-process CLI bundle variants exercise changed same-VM semantic policy,
invalid registration, terminal caught registration exhaustion, native error
poisoning, compiled-signature drift, and postpublication Lua throw/projection/
caught-budget failures. The passing suite runs 99 actual CLI processes plus one
fixture helper process. `cli-assertion-accounting.json` records 410 evaluated
Python assert statements on its deterministic passing path (compound expressions
count once); this is separate from CTest suite and Catch2 assertion counts.

## Verification and raw evidence

Commands executed directly and serially per build directory:

- `cmake --preset debug` and focused `cmake --build ... --target all planar_tests`.
- Focused native command/export selection: 29 passed, zero skips.
- Final focused CLI/native selection: eight passed, zero skips, with all four
  existing real CLI suites and three new integration/regression cases.
- `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`: pinned
  formatting and Doxygen pass over 89 active C++ sources.
- `python3 tests/native/v5-02-b2-support/gate.py build/rewrite-debug tests/native/v5-02-b2-evidence/debug-final`.
- `cmake --preset release` after archiving the stale complete build tree.
- `python3 tests/native/v5-02-b2-support/gate.py build/rewrite-release tests/native/v5-02-b2-evidence/release-final`.
- Both-build `audit-variants.py`: exact regeneration/reversal comparisons of all
  seven variant families, including all historical fault variants and 21 B1
  runtime variants, six B2 CLI compositions, and the fixed-export harness.
  Debug/release generated inputs match byte for byte.
- `python3 tests/vectors/generate.py --check`: 12 hand byte, 20 upstream hash and
  41 canonical/domain vectors verified unchanged.

Both final gates pass 361/361 tests, zero skips, preserve all 358 baseline names,
check 89 active C++ sources and finish with zero-work rebuilds. Full gate results
are in `debug-final/summary.json` and
`release-final/summary.json`, with full JUnit and raw LastTest retained before
any later inventory query. Each gate checks all 358 baseline names, nonzero
execution/zero skips, formatting/Doxygen and a final zero-work build. Existing
actual CLI suites still report 247 render, 60 init, 115 history and 55 export/output
processes; their assertions and historical goldens were preserved.

Initial compile/generator integration failures remain in raw logs: an omitted
optional aggregate initializer, unsupported CLI11 get_pname spelling corrected
to get_positional, and exact generator anchors adjusted after formatting. Initial
focused testing also retained the real mapping-precedence regression and a test
expectation corrected for the deliberately altered requiredness bundle. None of
these failures is hidden by the final gate results.

`release-cache-action.json` records moving the full stale release tree to
`build/rewrite-release.pre-v5-02-b2-llvm-23.1.3_2`, preserving it instead of deleting
it. Both current compiler-ar entries point to pinned LLVM23.1.4. Executed host is
macOS26.5.2 ARM64, Clang23.1.4, CMake4.4.4 and Ninja1.13.2; platform details are
retained separately. Linux compilation/runtime and hardware power loss remain
unverified. Workspace naming, load/full R14, external profiles, execution flows
and installation remain outside this candidate.

## Identities and preservation

`baseline.json` and `baseline/` freeze the 29 initial owned inputs (including the
explicitly authorized runtime amendment). `candidate.json`, `candidate/` and
`owned.diff` freeze the changed candidate and exact baseline-relative diff.
`artifacts.json` freezes the evidence set. Final handoff reports their SHA256s.
`protection-audit.json` checks 5,816 protected inputs: 5,815 unchanged and only the
explicitly authorized runtime.cpp amendment differs. The user v5 bytes, contract
body, fixtures, goldens, historical evidence and AGENTS -> CLAUDE.md symlink remain
unchanged. Planning/reference prose marks B2 as a candidate awaiting review.
