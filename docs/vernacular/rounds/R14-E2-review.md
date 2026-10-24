# R14-E2 independent review

Disposition: **approve**. No blocking findings. Approval covers only the exact
recorded-history candidate below; load membership remains an unresolved,
out-of-scope operator choice. No product fixes were made during review.

Reviewed shared checkout: `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`.
Candidate identity:
`cc6cfdaa19631a9a75de69edff3ebe2c02c777fa4f05903320fa33cd29586126`.
Candidate manifest: `tests/native/r14e2-evidence/candidate-files.json`, SHA256
`84fd80fcf004897952530bf416a7fd7006ce9bdcfd6752ae7f0363de69db42fe`.
Coder evidence manifest: `tests/native/r14e2-evidence/evidence-sha256.json`, SHA256
`020ebf78f73bf6d57a9a24bc9dd4eb24aef95d06fce645ec785a948c7930d051`.
All 17 candidate files and all 66 evidence members were independently rehashed.
All candidate copies equal current files, and the complete 15-file incremental
`candidate.patch` was independently regenerated and compared byte-for-byte.

The review read the active repository instructions, implementation plan, roadmap,
v4 design, ledger contract and R14-E2 brief. It examined the incremental diff,
baselines, source, test fixtures, raw full/focused logs, frozen evidence and
build/link wiring rather than treating the coder's summary as approval evidence.
Reviewer writes are limited to this report and
`tests/native/r14e2-review-evidence/`; shared product and earlier evidence were
preserved. The reviewer releases exclusive debug-build ownership on completion.

The substantive checks support approval:

- CLI11 owns required/duplicate/unknown and mutually exclusive bindings in
  `src/cmd/planar/handlers/history/command.cpp:17`. Numeric validation uses
  complete signed-64 `from_chars` parsing and canonical positive spelling.
  Bootstrap's existing syntax guard and parsed binding count retain separate
  tokens and reject terminators/extras. Independent probes cover both selector
  conflict orders, option permutations and valid option-like request keys.
- `src/engine/history/read.cpp:96` reads the head and every selector through the
  same supplied verified owner. `format.cpp:1094` retains its read transaction;
  typed reads never reopen it. Existing storage tests independently rerun here
  prove pinned DELETE-journal reads, later commits visible only to a fresh owner,
  and invalidation of all methods after an injected read failure.
- Public metadata includes every specified column. Revision and event IDs remain
  decimal strings, with `none` for absence; the unit test covers IDs through
  signed-64 maximum. Fifty additional real-binary probes include exact comparison
  of every revision metadata and event-summary column with scratch stored rows.
  Page cursor/end behavior, older states and current outer head are also covered.
- Request projection decodes only the saved public result. Neither private body
  fields nor argument paths enter output. State reads decode typed after-state
  and attach its matching revision metadata, without interpretation decoding.
  Revision reads use stored interpretation/version codecs. Imports and the
  recorded native query link exclude Lua/runtime/current interpretation policy;
  format's native prepared/capture dependency is accurately documented.
- Ordinary scratch fixtures use native initialize/complete_acceptance for initial,
  no-op, change and chronological revert, rather than SQL publication. The helper
  uses intentionally opaque prepared capture inputs, not a claim that its dummy
  Lua text was validated by current Lua. Unsupported interpretation is introduced
  only after native acceptance, by re-encoding and rehashing the two value records,
  restoring the exact trigger and passing the complete production opener.
  Unsupported revision interpretation refuses while page/request/state stay
  readable; rerun native storage tests also retain raw record access.
- The actual binary refuses the 9 MiB control-byte interpretation once JSON
  escaping exceeds the whole-envelope ceiling. Encoding reserves one byte for LF
  before returning buffered stdout. Only output-byte failure is remapped to
  maximum 16,777,216; depth/node error categories remain distinct. No partial
  stdout is emitted on the oversize refusal.
- Storage failures use the existing redacted typed adapter. Independent real
  processes cover empty/foreign/truncated files, future format, recognized corrupt
  event data, symlink and directory refusal. Scratch hashes and membership remain
  unchanged after every process, including errors; private intent markers and
  scratch host paths remain absent from public output.

Executed independent commands and results:

- `ctest --test-dir build/rewrite-debug -R 'replacement.(history|init_cli$|cli$)' -V --output-junit <absolute reviewer evidence>/focused.xml`:
  7/7 registered cases passed, zero skips; 115 history, 247 render and 60 init
  actual CLI processes. Raw output: `focused.log` and `focused.xml`.
- `python3 tests/native/r14e2-review-evidence/probe.py`: 50 additional actual CLI
  processes passed. Raw output and per-process outcomes: `probe.log` and
  `probe-results.json`. The first probe run expected corruption for a 200-byte
  truncation; the existing header opener classifies this incomplete format as
  `foreign_ledger`. That test expectation was corrected without a product edit;
  the original failure remains in `probe-initial.log`.
- `ctest --test-dir build/rewrite-debug -V -R 'replacement.ledger typed.*(snapshot|unsupported|failure)' --output-junit <absolute reviewer evidence>/storage.xml`:
  5/5 registered storage cases passed, zero skips; raw `storage.log`/`storage.xml`.
- `python3 tests/native/r14e2-review-evidence/audit.py`: exact candidate/patch,
  4,897 unchanged protected files, preserved AGENTS symlink, all 312 prior tests
  retained within 317 registered tests, and all three configured generated
  variants/reversals verified. New reviewer-owned files are explicitly excluded
  from this additive ownership check; coder evidence is not rewritten.
- `cmake --build build/rewrite-debug --target all planar_tests`: exit 0,
  `ninja: no work to do.` Raw `no-work.log`.

The coder's frozen full gate independently inspected here records 317/317 tests,
zero skips, formatting/Doxygen for 81 active sources, vectors and a no-work build.
The full run began before the Python oracle was strengthened; product bytes were
unchanged, and all seven focused cases were rerun with the final script by both
coder and reviewer. A second full run was unnecessary for this bounded review.

One informational wording issue remains outside any blocking finding: unchanged
`src/cmd/planar/response.cpp:127` says “render or init options” in generic usage
errors. Root `--help` correctly lists history, and invocation status/envelopes and
history behavior are correct. This does not prevent approval.

Limits: macOS Apple Silicon debug only. No release/Linux, exact 16 MiB successful
response boundary, allocation-failure injection, load CLI or export CLI claim.
No legacy tool/queue, installed binary, real operator database, installation,
commit, publication or merge was used. Approval does not resolve pending load
membership policy or the separately tracked integrated R14 gates.

Reviewer evidence is frozen by
`tests/native/r14e2-review-evidence/evidence-sha256.json` (all local evidence except
that manifest itself). The coordinator must recheck this candidate identity
before recording advancement; any product edit affecting it requires new review.
