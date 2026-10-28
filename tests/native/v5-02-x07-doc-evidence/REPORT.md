# V5-02-X07-DOC gate report

Gate runner output for the documentation-only round. No source file was edited by the runner. Raw logs are under
`tests/native/v5-02-x07-doc-evidence/raw/`. The checker named below is `tests/native/v5-02-x07-doc-evidence/check/doc_check.py`
(abbreviated `doc_check.py:LINE`); it is a gitignored evidence file and was never committed.

## Candidate identity

Identity tool output (raw/identity.log, pathspec `src tests/native`, which is empty for this docs-only round):

- HEAD 25704378472f3120baa6755c922894188fa94db5
- sha256 of `git diff --binary HEAD -- src tests/native` (tracked only): e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855 (the empty diff)
- combined sha256 including untracked bytes, same pathspec: e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855 (empty)

Docs-level identity (computed by the runner, not by the identity tool): sha256 of `git diff HEAD` (tracked docs only, two files)
9f88be5b2ebfda87bf4e77bf8b0298cb220c6c606fa3c4fdc274c6989653d10f; sha256 of untracked `docs/vernacular/x07-addendum.md`
5e87c2ad9a35248975e64f080086fd219a9fd9ca49131c23ff0237d18766d175; sha256 of untracked `docs/vernacular/rounds/V5-02-X07-DOC.md`
047fff5e187ec1beab73a589013a1ea95d338bebda64902e3d1fb5947f48e291.

## Commands and results

All run serially from the repository root, unpiped. Exit code files are in raw/.

| # | Command | Exit | Result |
|---|---|---|---|
| 1 | throwaway `check_md_links.py` (scratchpad, not committed) over addendum, architecture, implementation, round record, roadmap | 0 | 205 relative links and anchors checked, 0 bad (raw/links.log) |
| 2 | `git diff --stat HEAD` | 0 | `external-profile-implementation.md` (2 insertions), `profile-preparation-architecture.md` (4 insertions, 2 deletions); docs only (raw/diffstat.log) |
| 3 | `git status --short` | 0 | two modified owned docs; untracked `rounds/V5-02-X07-DOC.md` and `x07-addendum.md`; nothing else (raw/status.log) |
| 4 | `redact-local-paths.py redact` on the five explicit files | 0 | "redacted 0 files" (raw/redact.log) |
| 5 | `redact-local-paths.py check` on the same five files | 0 | no output, clean (raw/redact-check.log) |

Added by the runner, not in the brief: the coders' 35-check document checker run on the final tree
(`python3 -I doc_check.py .`, raw/doc_check.log): exit 0, 35 PASS, 0 FAIL, 0 skips. Note that the first redact and link attempts in
this session were invoked with a single quoted argument by mistake (the shell did not word-split); the redact reported "0 files" and
the link checker crashed. Both were rerun with five separate arguments and the logs overwritten. The redact result above is from
the corrected run (it still reports 0 files redacted, which is the expected clean result).

Matched tests, skips: no ctest tests are registered for this round (the round is documentation only, probe false, no product build,
per the round record's Agreed verification). The 35 checks are the matched test set and all ran (35 matched, 35 passed, 0 skipped,
0 failed). No red-time ctest name matches exist: RED.md records ctest "No tests were found" (exit 8) for all 35 names, which is a
property of using a script checker rather than Catch2.

No-change build check: NOT RUN. The round record says no product build, and no build was performed, so the rule that a no-change
build is only meaningful after a successful build leaves it not applicable. This is reported as a limitation, not as a pass.

## Registered test inventory

`ctest --test-dir build/rewrite-debug -N` (raw/ctest-N.log, exit 0): 1114 tests. Saved as `inventory.txt` (names only, 1114 lines).
Compared with the most recent committed-round inventory, `tests/native/v5-02-x06-d-c04-evidence/inventory.txt` (1114 lines): sorted
diff is empty, so zero new and zero removed names. This is expected because no source changed; the build directory is whatever was last
built (X06-D C04), not rebuilt by this round.

## (a) Acceptance criteria mapped to checks

| Criterion (round record) | Checks (doc_check.py line) |
|---|---|
| 1. five parts, every decision marked, no unmarked choice | addendum_has_five_parts :324; decisions_rows_marked :680; no_unresolved_new_choice :699; uncertain_readings_cited :712 |
| 2. 44 envelope and 12 opaque rows, with exit, class, accessed, clause; verb-as-value change called out | envelope_rows_exactly_44 :360; opaque_rows_exactly_12 :386; outcome_rows_internally_consistent :412; exit_matches_class :435; rows_equal_reference_outcomes :454; class_changes_called_out :494; clause_cells_cite_allowed_steps :509; spike_labels_translated :962; fixture_layout_sibling_trusted :1124; ksa_class_derived :1148 |
| 3. diagnostic table cites only the closed table | diagnostic_rows_use_closed_messages :537; diagnostic_classes_complete :565; usage_message_is_public_usage :1167 |
| 4. factory section: admission table, single creation site, X-CLI step 8 and substitution per family, replacement sentence | admission_table_covers_families :589; step8_and_substitution_covered :612; creation_sites_and_scan_stated :637; architecture_sentence_replaced :654; substitution_per_family :1009; family_registry_closed :1043; ownership_boundaries_stated :1059; factory_conversions_stated :1086; init_admission_alias_only :1101; wave_attachments_complete :794 |
| 5. CL-1 to CL-6 addressed, none repaired or reclassified | carried_limits_complete :742; limits_not_reclassified :762; brief_inputs_complete :785; non_goals_stated :989 |
| 6. docs only, links resolve, contracts unamended | diff_scope_docs_only :929; owned_doc_edits_scoped :876; links_resolve :831; also gate commands 1 to 3 above |
| 7. no machine-local path | no_machine_local_paths :945; also gate commands 4 and 5 |

No corrective round: no earlier findings are closed by this round.

## (b) Red-first

Red snapshot identity (red/RED.md, both runs): HEAD 25704378472f3120baa6755c922894188fa94db5, candidate.diff sha256
e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855 (empty), SHA256SUMS sha256
5153b654e79aa8e94d69af4b13dd3e7d0fdb9ba2fa132eb4b3a19a3561b8f068; snapshots red/snapshot and red/snapshot-2.

Two kinds of red evidence exist, and they differ:

1. red/RED.md (raw/, raw-2/): ran each of the 35 names through ctest; all "No tests were found", exit 8. This is
   not-registered, not an assertion failure, so it is not a behavioural red. Per-test commands are in each raw log; I did not
   re-verify them.
2. check/red.log, written by the checker author: Run A, command `python3 -I tests/native/v5-02-x07-doc-evidence/check/doc_check.py <repo root>`
   on the tree with `docs/vernacular/x07-addendum.md` absent, exit 1 (check/red.exit prints 1). Every check FAILed with
   "missing docs/vernacular/x07-addendum.md" except diff_scope_docs_only, which PASSed (guard that already passed). Failing line for
   each is the FAIL line in check/red.log. Run B (headings-only stub addendum) is described at line 41 of check/red.log. The
   snapshot hash for this checker red run is not recorded in check/red.log; use the RED.md identity above, which covers the same
   HEAD with no src or tests/native change. This is a gap.

Real red: all checks except two. Guards that passed already: diff_scope_docs_only (expectRed false per INVARIANTS I24) and
no_machine_local_paths (I25, expectRed false; in Run A it failed only because the file was absent). Mutation probing is not done
(probe false, per the round record); each mutant listed in plan/INVARIANTS.md was not applied by me.

## (c) Deliberate behaviour differences

None. No product behaviour changed. The one deliberate document-level difference from the spike is the X07 outcome table's
verb-as-value row (E06), which changes class from the spike's final label to a probe refusal before profile access under X-CLI step 3;
the addendum calls it out and class_changes_called_out checks it.

## Limitations

- No product build, no ctest run of new tests, no no-change build check (docs-only round).
- The 35 checks are a throwaway script in a gitignored folder; reviewers must trust the raw log and the final tree.
- The inventory is of an earlier-built tree and was not rebuilt.
- Red evidence: RED.md is a not-registered result; the behavioural red is only in check/red.log without its own snapshot hash.
- Redact reported 0 files changed, so no redaction was needed.
- Linux not run.

## Verdict

All brief commands met their criteria (exit 0, 205 links OK, docs-only diff, redaction clean) and the added 35-check run passed.
The no-change build check was not applicable.
