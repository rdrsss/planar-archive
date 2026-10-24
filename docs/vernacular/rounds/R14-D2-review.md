# R14-D2 independent review

Verdict: **approve**. No blocking findings in the exact candidate below.

This fresh reviewer examined the active guide, implementation plan, roadmap,
v4 design, ledger contract, R14-D/D2 briefs, the bounded candidate diff and raw
coder evidence. The checkout is shared. Reviewer writes are limited to this
report and new `tests/native/r14d2-review-evidence/`; product and reference files
were read-only. Others' work was preserved, including the newly supplied v5
specification, which is outside this review. No legacy tools, installed binary,
operator state, commit, installation, publication or merge was used.

## Exact candidate and evidence

Starting HEAD: `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a` on
`dev/planar-v2-vernacular`, with the pre-existing rewrite preserved.
The reviewed ten-file manifest is
`tests/native/r14d2-evidence/candidate.json`, SHA256
`3da96db6f13b830b48d05bb024aa810e403f3cd5a0caa8f79b7358a2e74d7936`.
The current files and frozen copies independently matched all ten hashes before
verification and current files matched again afterward. The coder evidence
manifest `evidence-sha256.json`, SHA256
`5c67ee687ee1415b917c26b021a8211d47f820e1eacca6153782a4dc53c2a2da`,
matched all 72 folder-relative entries. Independent protection checks matched
11,614 existing files and confirmed `AGENTS.md -> CLAUDE.md`.

Reviewer raw commands, timestamped exit records, JUnit, LastTest logs, audit and
current-candidate copy are under `tests/native/r14d2-review-evidence/`.
`evidence-sha256.json` there freezes those artifacts and `results.json` records
this report's identity. The coordinator must recheck the candidate before advancing.

## Contract review

- `native.cpp:106` separates the private ancestor map from public captured
  directories. `read_file` at line159 records logical-root/path identities from
  descriptor-relative no-follow traversal and compares existing identities during
  discovery and verification. No directory timestamps, modes or membership enter
  selected-ancestor equality.
- `retain` at line225 charges unique receipts before merging a successful file
  read. Failed content/path reads do not retain ancestor claims. Shared ancestors
  are deduplicated; repeated retained reads reuse their bytes. The separate
  existing 100,000-entry/64 MiB budget and 1024-byte portable path limit bound
  retained and per-read staging. There is no descriptor retained per ancestor.
  Allocation exceptions keep the existing unrecoverable boundary.
- Initial sealing and acceptance verification reread retained files through the
  original roots and compare original ancestor/file/root identities and exact
  bytes/normalized modes. Verification adds no receipts or alias-map entries and
  publishes no replacement snapshot/topology. Root and standalone registry parent
  checks retain their prior behavior. Required path failures map to
  `capture_changed` at sealing/reverification; discovery keeps typed refusals.
- Whole-tree enumeration still verifies independent directory identities,
  membership and topology. Selected reads leave public directory topology empty.
  Unrelated siblings, including inaccessible files, links, FIFO, oversized files,
  edits/additions/removal/rename, do not enter selected closure checks.
- The six additions exercise the actual missing guarantee: an original selected
  file inode hardlinked into a replacement parent. They also cover discovery and
  both sealing paths, nested chains, shared/repeated reads, failed-read staging,
  removal/link replacement, aliases, moves, immutable values and normalized modes.
  Existing native/facade cases retain root, registry and complete-tree coverage.
- Interface comments and architecture/testing additions explain the private
  contract. The five narrowly authorized status updates select closure-only load
  membership while retaining pending load service/CLI/full-R14 scope. Authority
  rules and historical G evidence are preserved. No API, serialized source,
  schema, policy, CLI or dependency edge was added.

## Executed verification and reused evidence

Reviewer commands ran strictly serially; each process completed before the next
build or CTest command began. Timestamped command records establish that ordering.

| Reviewer command | Result |
| --- | --- |
| `cmake --build build/rewrite-debug --target all planar_tests` | Exit 0; no work |
| `ctest --test-dir build/rewrite-debug -R 'replacement.capture' --output-on-failure --output-junit <review capture.xml>` | 31/31 passed, zero skips; 39.88s |
| `ctest --test-dir build/rewrite-debug -V -R '^replacement\.(cli\|init_cli\|history_cli\|export_cli\|acceptance \|atomic )' --output-junit <review dependent.xml>` | 28/28 passed, zero skips; 21.57s |
| Final `cmake --build build/rewrite-debug --target all planar_tests` | Exit 0; no work |

The dependent run includes four actual CLI suites, twelve prepared/request cases
and twelve atomic cases. Its raw output confirms 247 render, 60 init, 115 history
and 55 export/output processes, 60 render host-path checks and six native goldens.
All test invocations used source-built executables and scratch inputs.

The preserved pre-fix native implementation/interface exactly match the frozen
baseline. Its hardlink-parent regression log has one selected test and fails at
`REQUIRE_FALSE(verified)` because old verification succeeded. The green reviewer
capture run includes that same regression and the five further additions.

The coder full debug gate's raw LastTest log independently contains 337 tests and
337 passes; its gate log records zero skips, formatting/Doxygen over 86 active
sources and a no-work rebuild. Existing 331 G names are retained with six additions.
The independently hash-verified coder package also retains both vector checks,
five exact/reversible generated-variant comparisons, helper syntax lint and
protected-input audit. These successful checks were reused; no redundant full
gate or product modification was needed after reviewer focused passes.

The coder started its full gate approximately five seconds before the initial
focused capture command completed. That violated the intended command sequencing
and remains explicitly recorded, not retroactively described as serial. There
was no concurrent compilation or source edit; the full gate completed its own
337-case sequence, its later dependent rerun was serial, and this reviewer has
now independently rerun all capture/dependent cases serially. The observed overlap
therefore does not leave an unresolved candidate-correctness concern.

## Limits and handoff

This is macOS ARM64 debug verification with fresh LLVM 23.1.4 and CMake 4.4.4.
The stale LLVM 23.1.3_2 debug cache was preserved by the coder, with toolchain source
policy unchanged. Release was neither run nor modified by this reviewer; G's
release 331 result is prior composition evidence, not a D2 release claim.
Release/load composition and Linux execution remain later gates. This is a
cooperative capture protocol; no hostile racing, hardware power-loss or full
R14 load-service/CLI guarantee is inferred.

Debug and release build-directory ownership is released. No further work is
running. This D2 approval permits the coordinator to close this bounded checkpoint
after checking the exact candidate; it does not assess the user-owned v5 spec.
