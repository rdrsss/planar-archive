# R14-C3-B1 independent review

Disposition: **approve**. No blocking findings or unresolved product questions in
this bounded preparation slice. This approval does not cover B2 atomic acceptance,
semantic declaration closure, a load service, or CLI behavior.

## Exact candidate and authority

Reviewed six-file candidate
`f3ddd6eeaae180475b74a1d16e975c1b671da87fb74c98d3a96b47c49dca6d34`.
The SHA256 of the coder's evidence-manifest.json is
`cba2498ae8bf7b555ba866929e8b9fe2022ad92a8164abfe9b44322d4dd7f9a9`.
Independently recomputed both identities, all six current/frozen candidate file
hashes, all 50 evidence-manifest entries, and all 4696 protected file hashes.
They match. Starting Git revision remains
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a` on
`dev/planar-v2-vernacular`; substantial pre-existing rewrite changes are preserved.

Read the active repository guide, implementation/roadmap, v4 design, ledger
contract, C3/C3-B/B1 briefs, six-file candidate/diff and raw coder evidence.
Shared checkout; reviewer writes are confined to this record and
`tests/native/r14c3b1-review-evidence/`. No product edits, Planar operational tools,
legacy binary invocation, operator state, commits, installation or publication.

## Review assessment

- `prepared.cpp:36` bounds request views and framed aggregate before ownership,
  validates request/actor/positive signed-64 head and already-normalized absolute
  paths, and uses the existing request record encoder. No cwd or filesystem query
  enters intent; request ID is retained separately from exact body equality.
- `prepared.cpp:89` consumes the opaque native capability and resolves unique,
  nonempty paths exclusively in its sealed workspace snapshot. Shared source tree
  validation enforces complete bounds, modes, topology and aliases before source
  payload copies. Registry accepts exactly its one captured input.json entry.
  Other roots cannot become declaration source through this API.
- `prepared.cpp:125` reuses pure provenance verification and freshly decodes exact
  interpretation/versions, rejects unsupported versions, compares recomputed
  stdlib identity and reads workspace ID only after fixed structure validation.
  No caller digest or mutable parsed value pointer is an input authority. Current
  schema/semantic closure remains the expressly deferred service responsibility.
- The owner has no public construction bypass, default construction, copy, mutable
  prepared fields, SQL handle or arbitrary receipt callback. Independently owned
  const shared values remain safe after input destruction and owner transfer.
  `prepared.cpp:156` delegates final verification to the precise retained native
  capture. Preparation itself performs no receipt check or filesystem access,
  preserving the required later replay-before-receipt sequence.
- Reviewed CMake and the actual prepared-test compile/link. Real native captures
  and prepared data are constructed. The link includes native capture and pure
  tree/provenance/interpretation/record codecs, with no ledger format/SQLite,
  capture facade, Lua/loader/runtime or embedded-source dependency. Only the six
  owned candidate files changed in the scoped baseline; architecture/testing
  references accurately describe this boundary.

## Raw evidence and independent verification

The coder's raw XML contains 151 history-focused cases, 25 capture cases and 12
acceptance cases, all nonzero with zero skips/failures. History/capture case names
match C3-A exactly. CLI raw evidence records 247 real processes. Inspected both
vector checks, pinned formatting/Doxygen for 70 active files, four helper-file
format checks, exact variant generation and no-work logs. The only product change
after the first green behavioral run is the recorded three Doxygen comments;
final repeated evidence is retained. Initial fixture-assumption failures are
accurately identified rather than claimed as product regressions.

Executed `python3 tests/native/r14c3b1-review-evidence/verify.py`, which:

1. Independently validates the hashes/raw counts above and reproduces both
   generated fault/collision variants byte-for-byte against configured outputs.
2. Compiles reviewer-only adversarial cases using the candidate's real compile
   flags/BMIs and actual native-only link inputs; exact argv and logs are retained.
   Four cases pass with **57 assertions**: preparation after all captured paths
   disappear; unselected captured changes still fail the retained verifier;
   caller buffer overwrites cannot alter prepared provenance/interpretation;
   request copies and moves preserve exact owned argument/body bytes.
3. Independently runs `ctest --test-dir build/rewrite-debug -R
   replacement.acceptance --output-on-failure --output-junit <review evidence>`:
   **12/12 passed**, zero skips/failures.
4. Runs `cmake --build build/rewrite-debug --target all planar_tests`:
   **ninja: no work to do.**

Protection-count reduction was investigated, not inferred from the smaller number.
The C3-A manifest had 12326 entries; B1 removes 7709 and adds 79. Removed entries
comprise 6387 Doxygen output files, 1284 clangd cache files, five pytest cache
files, and 33 other paths. All 33 other paths (including root instructions/build
configuration and the three omitted reference documents) still match their
pre-C3-A hashes. Thus the narrower declared B1 baseline does not hide changes to
those paths. Full comparison is retained in protection-comparison.json and
omitted-comparison.json. AGENTS.md remains the CLAUDE.md symlink.

No unrelated full-suite repeat was needed after these focused checks and matching
raw integrated evidence. This review covers macOS debug only; Linux and the final
integrated R14 release gate remain outstanding. Cooperative capture cannot prevent
writes after final receipt verification. No callable acceptance writer exists in
B1, and that absence is consistent with the approved split.

Debug build directory is released; no reviewer build/test process remains.
Coordinator must verify the candidate identity is still current before advancing.
