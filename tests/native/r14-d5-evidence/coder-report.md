# R14-D5 bounded native load capture/preparation candidate

Implementation candidate, not self-approved. Owned paths are in `owned.json`.
Shared checkout and all prior rewrite work were preserved. No storage/schema/
codec/runtime/stdlib/CLI algorithm changed. No Planar tools/skills/queue,
installed binary, operator database, commit, install, publication or merge.

The new `planar.engine.acceptance.load` module is move-only and terminal on
failure, consumption or reuse. Its explicit normalized absolute paths never use
cwd/environment/name lookup. `workspace()` returns original `{file,workspace}`
once, `capture(route)` checks fixed route/captured workspace agreement before
routed reads, captures all declarations/references and exact explicit registry,
and seals with `finish_for_acceptance`. Declaration semantic errors (including
wrong constructors/headers) remain for Lua interpretation with logical paths.
`prepare(B,M,exact_stdlib_files)` consumes the owner and uses only pure codecs,
header/identity/file/reference correspondence and full native snapshot source
membership. It cannot accept caller source subsets. Prepared data retains the
original native receipts and provenance. There is no tree enumeration,
source/base/observation capture, product semantic dispatcher, ambient embedded
source lookup, Lua during preparation or new public command/service.

Seven new cases cover complete multi-job closure/exact six source files, all
embedded provenance bytes, actual Lua all-job invalid/unselected/duplicate IDs,
wrong constructors/headers and logical diagnostics, malformed/mismatched route,
B/M/header/file/reference bytes, sealing and repeat receipts, selected bytes,
modes, roots, ancestors, links, registry, unrelated siblings, explicit/missing/
aliased/oversized inputs, terminal state, moved/self-moved owners, live ownership
assignment and provenance bounds/corruption. Existing lower-level tests exercise
remaining cumulative capture, source/provenance tree and interpretation bounds.
No production declaration/Lua schema/default/normalization/graph algorithm is
copied into the new orchestration layer.

Completed focused CTest passed **123/123** (117 unit cases and all six current
actual CLI suites), zero failures/skips. All 373 prior names remain; inventory
is **380**, adding seven. That completed run predates the final private Doxygen
annotations: `comment-only-proof.json` verifies identical lexical tokens after
removing comments/whitespace against a transparently reconstructed pre-comment
snapshot; `comment-only.diff` is retained. The completed original JUnit/LastTest
are byte-identical in both `attempt-01-*` and `focused-final.*` artifacts. The
coordinator explicitly directed avoiding a redundant 123-case rerun for comments.

Final final-bytes rebuild, pinned formatting/Doxygen (**92 active sources**),
unchanged vectors (**12/20/41**), exact seven generated variant families and fixed
runtime harness, zero-work build and inventory all pass. Exact argv/exits are in
`final-commands.json`; other command history, the superseded overlap and aborted
redundant rerun are disclosed in `commands.md`. Initial compile failures,
Doxygen failure and all raw passing/interrupted logs are retained.

Tests use actual embedded Lua and provenance. They do not establish retained
same-VM handler/provenance binding; D6 must add that API/order/execution proof.
D7 still owns public admission, D8 full acceptance, and external profiles follow
full R14. This is bounded debug/macOS ARM64 evidence, not a full milestone,
release/Linux, hardware power-loss or public load result. No atomic completion
was added here; existing acceptance transaction cases run in the focused suite.

All product/reference/evidence prose writes finish before `freeze.py` computes
repository-relative candidate snapshots, exact baseline diff and evidence-relative
artifacts. The exact candidate/diff/artifact identities are in
`handoff-identities.json`. Debug and release are released and idle after the final
checks; no further build/test command is running or planned by this coder.
