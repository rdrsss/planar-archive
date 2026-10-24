# R12-C1-C01 defensive recovery correction candidate

Shared checkout and pre-existing rewrite edits preserved. This candidate awaits
fresh independent review. Debug directory is released; no build/test process
remains. No commits, installed binaries, operator data or Planar operational
tools were used.

`candidate-manifest.json` identifies the complete corrected C1 nine-file scope.
`candidate/` contains exact current copies. Existing `baseline/` and
`correction-baseline.json` remain unchanged and identify the frozen C1 candidate
before this correction. `candidate.diff` is the correction delta, including the
prior interrupted coder's regression. `full-c1-candidate.diff` compares the final
nine files with the original C1 baseline retained under `r12c1-evidence/baseline/`.
The manifest SHA256 is
`2a90623073df705d33b4c4db0cd37cd933f31e8bae48e9456e0cec5bba81202f`.

The replacement coder added only a fixed eight-byte EOF read in `hot_header`:
matching existing SQLite journal magic returns a typed `corrupt` refusal for an
unsupported super-journal footer before scratch creation or writable recovery.
The guard does not decode or open the encoded pathname. A short/failed tail read
returns a typed storage error. Malformed or coincidental matching footers are
intentionally refused. SQL schema, primitives, ordinary opener and vendors were
not changed by this correction.

The existing regression from the interrupted coder was preserved. Coordinator
authorized pinned formatter changes only; `test-formatting.diff` records those
whitespace changes. Removing whitespace from the before/after text produced
identical strings. No reproduction tooling or new regression variants were added.

Commands were run serially in `build/rewrite-debug`:

- Existing pre-fix regression through `ctest --test-dir build/rewrite-debug -R
  'replacement.ledger format recovery refuses super-journal authority unchanged'
  --output-on-failure`: exit 8, one case failed, 18 of 153 assertions failed.
  `red-focused.log` preserves output; prior `red-build.log` is unchanged.
- `cmake --build build/rewrite-debug --target all planar_tests`: exit 0 in
  `build-corrected.log`, repeated after formatting in `build-formatted.log`.
- `ctest --test-dir build/rewrite-debug -R 'replacement.ledger'
  --output-on-failure`: exit 0, 36/36 cases passed, zero skips in
  `focused-corrected.log`; after formatting, `focused-final.log` again records
  36/36, zero skips. Ordinary and redundant recovery and backup cases passed.
- `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`:
  `lint-corrected.log` records initial formatting failure in the guard and prior
  test edit. After pinned formatting, `lint-final.log` records exit 0 and
  formatting/Doxygen checks for all 35 active C++ source files.
- Final `cmake --build build/rewrite-debug --target all planar_tests`: exit 0,
  `no-work-final.log` records `ninja: no work to do.`

`protected-prior-evidence-audit.json` checks all 54 historical evidence files from
`prior-evidence-baseline.json`, all nine frozen correction-baseline snapshots,
and unchanged files outside the four permitted correction candidate paths.
The full candidate and evidence manifests use SHA256. This host verification is
macOS debug only. Broad fault injection and integrated debug/release gates remain
R12-C2; this record does not self-approve the candidate or complete R12.
