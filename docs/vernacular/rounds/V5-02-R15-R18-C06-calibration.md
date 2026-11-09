# C06 reduced test-budget calibration

Status: exploratory measurement; no final acceptance claim.
Coordinator granted a C06 source freeze and Debug lane; a subsequent policy review
may require another Lua correction, so final pins remain pending.
The prior49MiB /26M instruction /34,500 node candidate and absolute pins remain
unverified until a coherent rebuilt source freeze. Production ceilings are unchanged.

## Measurement integrity

The C05 aggregate script was exploratory: it printed low-margin cells but did not
return failure, rounded before comparison, and did not include loader outcomes.
The prior report already labels its results non-green; its exit status cannot be
used as acceptance evidence. C06 measurement-only tools live under
`tests/native/r15-r18-c06-evidence/calibration/`. The new runner retains one sorted
inventory, exact per-test names, XML result selection and failure/skip status. The
new aggregator checks both runtime and loader inventories/results, compares
`4 * peak <= 3 * ceiling` exactly, includes all five columns and the native-boundary
instruction check, requires previously measured budget fixture coverage, and fails
on any missing measurement or failed test. Per-column exclusion reasons follow
X06-D-C04 D9; only the recorded I1/I2 paired control has a manual exclusion.

Five synthetic tooling controls passed: an exact25% free cell is accepted, a24%
free cell is refused, a loader failure is refused, missing runtime measurement is refused, and a D29 citation mismatch is refused. These are integrity controls for the evidence scripts, not product
tests or measured margins. Raw results are in `tooling-selftest.json`.

## Planned execution

After explicit source freeze and lane grant, rebuild the ordinary runtime test
target, then produce a private instrumented scratch archive and test executable
from coherent matching objects/PCMs. First run coupled-family and exact-boundary
controls; tune only reduced test constants if measurements require it. Remeasure
all runtime cases independently in Debug and RelWithDebInfo, plus loader tests,
and retain full five-column tables. Native MiB choice must be the smallest
qualifying value below64MiB and below the deliberately refusing large-Lua fixture.
Keep window8 request-charge pin and the existing field/tree/projection gaps.

Linux absolute pin requires an actual focused run in a copied-context Linux
candidate. No historical platform delta will be assumed. Coordinator owns Docker
preflight and final Debug/Release/Linux gates; an independent prober owns budget
mutations after final measurement.

Source freeze and Debug lane2 granted by coordinator. Initial coherent Debug runtime+loader target rebuild is underway; source-before.json records exact source identities. No measurement yet.

## First C06 measurement

Coherent Debug runtime+loader build passed. Four focused controls ran:2pass/2fail,
3732 assertions. The quarter-free guard correctly rejects26M instructions at49MiB:
ordinary native31,116,830; boundary extrapolation19,513,574 needs at least26,018,099.
The old macOS off pin also correctly fails: measured342,983 versus326,112. Route
and first-error controls passed. These are retained failures, not acceptance.

Private raised scaling probes confirm the native-pad slope23 and instruction
slope22. Coupled model predicts minimum51MiB /29M instructions /37,500 nodes.
49MiB needs27M and35,500 nodes, whose node-boundary native38,894,790 exceeds
38,535,168 allowance.50MiB needs28M and36,500 nodes, whose39,444,790 exceeds
39,321,600. At51MiB, measured node boundary37,498 uses39,994,790 versus40,108,032
allowance. Sink/first family native peaks are39,224,018/39,224,178, nodes27,873.
Raised big-Lua live peak55,316,706 stays above51MiB53,477,376. These do not replace
final normal-limit margin measurements. `coupled-model.json` records predictions.

The route far-probe must remain below its independent1MiB text quota. At29M the
old1.5x job count may exceed that quota; a smaller still-refusing upper bracket
will be measured before any fixture edit, retaining exact instruction diagnostics.
No test ceilings have yet been changed by this agent.

## Completed exploratory batch and product finding

Instrumented Debug ran487 cases:484pass/3fail after correcting four XML metadata
records. Loader ran19/19. The real failures are the quarter-free guard, old off
pin, and unchanged big-Lua refusal. The XML runner initially attempted integer
conversion of Catch `skipped="false"`; four successful raw XML records were
misclassified. Original results and a reparse manifest are retained; no product
output or exit code changed. The corrected parser accepts boolean/numeric false.

The big-Lua failure is a product terminal-error defect, not a fixture resize.
`loader.cpp::allocate` records composed refusal, Lua GC retry succeeds, and
`load_declaration` checks `s.composed` only when final status is LUA_ERRMEM.
The unchanged ordinary Debug test also fails. Instrumented t301.runlog records
a real live-Lua refusal (`why=4`) while t301.xml reports prepared=true. Sent to
C07 search; this agent does not own or change loader product code.

Exact margin checker exits1 with11 reported problems:3failed tests, four native
boundary instruction cells (some also independently flagged as ordinary cells),
and stale D29 source-comment values. All five columns and506 total test outcomes
are represented. N30 projection node peak is actually25,424; ordinary node-boundary
native38,344,790 at34,498 nodes. The model's N30 floor is therefore measured.

Build lane2 released after scratch compilation; no process remains after batch
completion. All test constants still unchanged by this agent. Final calibration
waits for C07 product corrections/source freeze.

Handoff: fresh continuation must invoke `build_measure.py debug normal PASSLABEL`
(and release/raised equivalents); each alphanumeric label creates a new scratch
root and refuses overwrite, preserving earlier evidence. Build ordinary coherent
`planar_runtime_tests planar_lua_loader_tests` first. `run_all.py` requires a new
output directory and retains XML, inventory and result mapping. `aggregate.py`
requires runtime+loader subdirectories plus `ordinary/padded-0-0.probe`, and
compares source-comment ordinary values exactly (D29). Final source comments must
be updated from ceiling-independent pass1 values before the final measurement.
All four Python files parse successfully; tooling hashes retained.
