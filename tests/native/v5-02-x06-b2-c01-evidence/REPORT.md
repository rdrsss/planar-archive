# V5-02-X06-B2-C01 gate report

## Candidate identity

HEAD bcfe1ad7ed94a85985f1189deba3b12cc851e7bd on dev/planar-v2-vernacular, plus a dirty working tree.
sha256 of `git diff` (tracked changes only): 5049380855a8c991b53622bfb95a966321b57988ef58ca7ec0c05a66429167ad.
Untracked candidate files (source_capture.cpp/.cppm/.t.cpp and the round documents) are not in that diff; sha256 of the sha256 list of the untracked files: 8af8bf440864a07c9869544bab66fa76ee76923eaf65ca8f53e4abfe30e5f6ab.
No source file was edited by the gate runner.

## Commands (serial, unpiped, macOS Debug, build/rewrite-debug)

1. `cmake --build build/rewrite-debug --target planar_tests`: exit 0 (129 steps ran, all linked).
2. `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure`: exit 0. "100% tests passed, 0 tests failed out of 940". No failures, no skipped, no disabled or not-run entries in the log.
3. `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`: exit 0. "Formatting and Doxygen checked 136 active source files". Doxygen hit the documented upstream 1.18.0 SIGBUS flake on attempt 1 and succeeded on attempt 2 of 3, with no diagnostics.
4. `cmake --build build/rewrite-debug` (first full build): exit 0, "ninja: no work to do." (planar_tests had already built everything).
5. `cmake --build build/rewrite-debug` (second): exit 0, "ninja: no work to do." Zero steps.

Raw logs and exit codes are in `raw/`.

## Inventory

`ctest -N` lists 940 tests (`inventory.txt`, sorted names; raw in `raw/ctest-N.log`). 940 ran and passed, so the matched count equals the registered count.

Comparison against the most recent committed inventory, `tests/native/v5-02-x06-b1-c01-evidence/inventory.txt` (928 names): 12 added (`inventory-added-vs-b1c01.txt`), 0 removed.
Comparison against the untracked B2 candidate inventory `tests/native/v5-02-x06-b2-evidence/inventory.txt` (937 names; not committed at this HEAD): 3 added, 0 removed:
- replacement.capture admission receipts report same admission and liveness
- replacement.source capture refuses a registry hard-linked to the declaration as path_denied
- replacement.source capture refuses receipts outside one original admission at construction

## Zero-work result

Both trailing builds reported "ninja: no work to do." Note the first of them followed the planar_tests build, so it did no work itself; no-work holds on two consecutive default builds after the successful build.

## Criteria to test map

Added by the C02 documentation correction (X06B2C01-R1). `SC` is
[`src/engine/acceptance/source_capture.t.cpp`](../../../src/engine/acceptance/source_capture.t.cpp);
`AD` is [`src/engine/capture/admission.t.cpp`](../../../src/engine/capture/admission.t.cpp).
Lines were checked with `grep -n` against the files whose hashes are in the
[C02 record](../../../docs/vernacular/rounds/V5-02-X06-B2-C02.md). Criteria 1 to 10 are those of the
[B2 round record](../../../docs/vernacular/rounds/V5-02-X06-B2.md); the C01 rows follow.

| Item | Test case (SECTION) | File:line | Limits |
| --- | --- | --- | --- |
| B2-1 factory needs three original receipts | `source capture factories require three original receipts and refuse moved and poisoned authority` (SECTIONs: moved declaration receipt; poisoned declaration receipt; moved registry receipt); arity by `static_assert` | SC:105-108 (arity), SC:110 (case), SC:113, SC:126, SC:137 | Arity is a compile-time check of `open_bound`; behavior refusals are the three sections. |
| B2-2 root and declaration admitted before registration, no content read | `source capture admission precedes registration and reads no declaration content` | SC:240 | Proven by `planar_admission_read_attempts() == 0` after admission and after `p.verify()`, then `> 0` after capture. Ordering against real registration belongs to the runtime row; no registration runs in this row. |
| B2-3 module/declaration hard link in disjoint dirs refused | `source capture refuses a hard-linked trusted declaration before content or factory` | SC:254 | Existing X04 admission behavior; no external chunk machinery exists in this row. Admission does not refuse aliases between two untrusted supplied files (see the registry/declaration row). |
| B2-4 replacement between registration and first read refused | `source capture refuses a declaration replaced between registration and first read` | SC:269 | Replacement is simulated after admission and before the bound open. |
| B2-5 `same_selection` plus actual-leaf verification, no receipt replacement | `source capture refuses retained authority for another selection or admission` (SECTIONs: declaration receipt retained for a different admitted file; declaration receipt from a second independent admission; successful bound open keeps the original authority through repeated plain reads) | SC:285, SC:287, SC:305, SC:328 | The same-selection refusal message is "Retained file authority cannot be replaced". |
| B2-6 discovery, seal, move, precommit, poison retained | `source capture retained flow survives moves and detects post-seal mutation` (four mutations, see N2 row) | SC:342 | Debug macOS only; precommit here is the capture `verify()` after seal and move. |
| B2-7 positive registry colocation | `source capture registry colocation binds standalone input and refuses trusted alias` (SECTION registry beside the declaration is bound as input.json and excluded from workspace files) | SC:388, SC:389 | Distinct-file colocation only. |
| B2-7 (negative half) registry hard-linked to a trusted module | same case, SECTION registry hard-linked to a trusted module is refused at admission | SC:409 | Refused at admission with `unsafe_path`. |
| B2-8 missing declaration keeps load-family diagnostic | `source capture vanished declaration reports admitted change beside embedded missing` (SECTIONs: missing selected declaration keeps the existing load-family diagnostic through the known-declaration adapter; known declaration adapter passes other codes through unchanged) | SC:423, SC:433, SC:454 | The adapter `known_declaration_error` is pinned by test but nothing calls it yet (runtime-row obligation). See X-CAP record. |
| B2-9 charges and terminal failure unchanged | `source capture terminal failure after capture refuses later operations`, plus the unchanged load tests | SC:482 | No native, ledger or charging code changed; the admission additions are const predicates. The unchanged load tests are covered by the 940/940 gate above, not individually mapped here. |
| B2-10 red-first evidence | see Red-first evidence below | n/a | n/a |
| C01 R1 diagnostic | same case as B2-8: raw admission diagnostic code `io`, message "Cannot open confined input", field `profile`; adapter result `io` / "Cannot inspect confined input", empty limit, maximum 0, equal to `open_embedded` | SC:433 (SECTION), SC:454 (passthrough SECTION) | Raw diagnostic is pinned in the test; wiring is not delivered. |
| C01 R2 same admission, moved-from, poisoned, foreign receipts at construction | `source capture refuses receipts outside one original admission at construction` (SECTIONs: registry receipt from a second independent admission; moved-from root receipt; poisoned registry receipt; poisoned root receipt); the B2-1 sections for moved and poisoned declaration or registry receipts; the B2-5 section for a declaration from a second admission; predicates in `capture admission receipts report same admission and liveness` | SC:154, SC:156, SC:178, SC:193, SC:206; SC:113-152; SC:305-327; AD:86-161 | `same_admission` is identity of the shared state, not liveness (AD:148-149). No role check on supplied receipts (advisory N2). |
| C01 N2 post-seal mutation | `source capture retained flow survives moves and detects post-seal mutation`, mutation 3 (append to `spec.lua`, a file discovered through the workspace root), also mutations 0 to 2 (declaration bytes, registry bytes, declaration mode) | SC:342 (case); `spec_captured` check SC:353-356; mutation 3 at SC:376-377; `capture_changed` checks SC:380-384 | Asserts `capture_changed` on `verify()` and a poisoned admission. |
| C01 registry hard-linked to declaration | `source capture refuses a registry hard-linked to the declaration as path_denied` | SC:221 | Admission accepts this pair; capture refuses it with `path_denied` / "Hard-link input alias" (SC:234-235), nothing sealed (`sealed_ready()` false), admission poisoned. Admission does not refuse aliases between two untrusted supplied files. |

The map agrees with the "Criteria assessment" table of the
[C01 review](../../../docs/vernacular/rounds/V5-02-X06-B2-C01-review.md). Limits that apply to every row: Debug
macOS only; no RelWithDebInfo, Linux or full release gate; ordering against real
registration and any caller of `known_declaration_error` belong to the runtime row.

## Red-first evidence

Two regressions had a red phase. Test sources are the current files unless stated.
Snapshots are copies of the exact files used at red; the shared fact for both is
HEAD `bcfe1ad7ed94a85985f1189deba3b12cc851e7bd`.

### Regression B: `source_capture` (R1 adapter and R2 construction predicate)

- Snapshot: [`red-snapshot-B/`](red-snapshot-B/): `source_capture.cppm` `db5feedbad0c...`, `source_capture.cpp` `4f4c4290f258...`, `source_capture.t.cpp` `83aedf8f3600...` (full hashes in [`raw/B-red-snapshot-hashes.txt`](raw/B-red-snapshot-hashes.txt) and [`raw/B-red-hashes-header.txt`](raw/B-red-hashes-header.txt)). This is the B2 baseline implementation plus a three-line stub `known_declaration_error` returning `to_source_error(admission)` and no R2 construction predicate ([`raw/red-line-verify.txt`](raw/red-line-verify.txt) shows both).
- Commands, copied from [`raw/B-red.log`](raw/B-red.log) (unpiped):
  - `cmake --build build/rewrite-debug --target planar_acceptance_source_capture_tests` (exit 0; [`raw/B-red-build.log`](raw/B-red-build.log))
  - `ctest --test-dir build/rewrite-debug --no-tests=error -R "source capture" --output-on-failure` (exit 8; [`raw/B-red-ctest-raw.log`](raw/B-red-ctest-raw.log))
- Real reds (five assertions, line numbers in the hashed `red-snapshot-B/source_capture.t.cpp`, confirmed by `grep -n` in [`raw/red-line-verify.txt`](raw/red-line-verify.txt); the current test file has the same assertions on the same lines):

| Red line (snapshot = current) | Assertion | Case |
| --- | --- | --- |
| 146 | `REQUIRE_FALSE(source)` for a moved registry receipt | factories require three original receipts (SC:110) |
| 171 | `REQUIRE_FALSE(source)` for a registry receipt from a second admission | refuses receipts outside one original admission (SC:154) |
| 323 | message `"Captured external profile changed"` for a declaration receipt from a second admission | refuses retained authority (SC:285) |
| 445 | adapted message `"Cannot inspect confined input"` | vanished declaration (SC:423), R1 |
| 450 | adapted message equals the embedded message | vanished declaration (SC:423), R1 |

  The failing-line list is also in [`raw/B-red-lines.txt`](raw/B-red-lines.txt).
- Red versus final test file: `diff red-snapshot-B/source_capture.t.cpp src/engine/acceptance/source_capture.t.cpp` differs only at lines 225-226 (clang-format alignment of `auto p` and `auto r` in the registry hard-link case). Final hashes: `source_capture.cppm` `58865a5a328b...`, `source_capture.cpp` `db0e6dfdfd7e...`, `source_capture.t.cpp` `3924ecc0c0ff...`.
- Green: [`raw/B-green.log`](raw/B-green.log) records the same two commands, exit 0, 11/11 "source capture" tests; its header notes an earlier pre-format green run whose raw log was overwritten and is not retained. Format checks: [`raw/B-format-check-before.log`](raw/B-format-check-before.log), [`raw/B-format-check-after.log`](raw/B-format-check-after.log).
- Guards that already passed at red (not real reds, kept as regression coverage): the moved-from root, poisoned registry and poisoned root sections (already refused at `add_root` by the shared failed-or-moved-from check, which poisons the admission through the registry receipt); the registry hard-linked to the declaration case (SC:221; [`raw/B-hardlink-probe.log`](raw/B-hardlink-probe.log) shows `capture code=3 detail=path_denied msg=Hard-link input alias`, `sealed=0`, `p.verify=0`); and N2 post-seal mutation including `spec.lua` (SC:342). Baseline hashes before any C01 change: [`raw/B-baseline-hashes.txt`](raw/B-baseline-hashes.txt).

### Regression A: admission receipt predicates

- Snapshot: [`red-snapshot-A/`](red-snapshot-A/): `admission.cppm` `b289112661a1...`, `admission.cpp` `72520ca64dcc...` (HEAD with the six predicates stubbed), `admission.t.cpp` `64819c705851...` ([`raw/A-red-hashes.txt`](raw/A-red-hashes.txt), [`raw/red-line-verify.txt`](raw/red-line-verify.txt)).
- Result at red ([`raw/A-red.log`](raw/A-red.log)): test #52 `replacement.capture admission receipts report same admission and liveness` Failed, 0.02 s; 60 assertions, 43 passed, 17 failed. Failing lines in the hashed `red-snapshot-A/admission.t.cpp`: 115, 116, 117, 118, 119, 120, 123, 124, 125, 126, 132, 133, 145, 146, 147, 156, 157.
- Real reds: 16 of the 17. **Line 147 was a wrong expectation, not a real red.** `CHECK_FALSE(admission_root->same_admission(*admission_file))` assumed poisoning changes identity; identity is the shared admission state, and poisoning changes liveness only. It was inverted to a `CHECK` at the current `admission.t.cpp:149` (line 148 is the explanatory comment).
- Attempt 1 was a fixture bug and is not evidence of the fix: [`raw/A-red-attempt1-fixture-bug.log`](raw/A-red-attempt1-fixture-bug.log) (test sources `fd244831b8cb...`, [`raw/A-red-attempt1-hashes.txt`](raw/A-red-attempt1-hashes.txt)) failed `REQUIRE(admission_file)` at line 142 and stopped early; build log [`raw/A-red-attempt1-build.log`](raw/A-red-attempt1-build.log).
- Green ([`raw/A-green.log`](raw/A-green.log)): same test #52 Passed, 1/1, with `admission.t.cpp` `75ffa7bed2be...` and `admission.cpp` `7886d7407df2...` ([`raw/A-green-hashes.txt`](raw/A-green-hashes.txt)); regression log [`raw/A-green-regression.log`](raw/A-green-regression.log); format checks [`raw/A-format-check.log`](raw/A-format-check.log), [`raw/A-format-check-after.log`](raw/A-format-check-after.log); pre-format copy [`raw/A-pre-format.t.cpp`](raw/A-pre-format.t.cpp); pre-run listing [`raw/A-count-prelist.log`](raw/A-count-prelist.log).
- **Missing command lines.** Unlike `raw/B-red.log`, `raw/A-red.log` and `raw/A-green.log` begin with `Test project ...` and carry no command line. What is known: each is a single-test ctest run of `replacement.capture admission receipts report same admission and liveness` (test #52, build directory `build/rewrite-debug`, `Start 52` line, `Filters:` line `capture admission receipts report same admission and liveness`); red failed in 0.02 s and green passed 1/1. The output format is consistent with a `ctest -R` run, but that is unverified. The exact command was not recorded and is not reconstructed here. The build logs [`raw/A-red-build.log`](raw/A-red-build.log) and [`raw/A-green-build.log`](raw/A-green-build.log) are ninja output only (steps 0 to 4 of 5 rebuilding `planar_capture_admission_tests`) and also contain no command line.

## X-CAP record

Contract source: [X-CAP](../../../docs/vernacular/external-profile-contract.md#x-cap--strict-capture-and-trust),
line 172 "Missing supplied paths retain existing family capture errors" and line
203 "a changed receipt refuses `capture_changed` before result authorization/commit".
The first governs a path that is missing when supplied; the second governs an
admitted receipt that changes afterwards.

- (a) Declaration removed after admission, before the bound open: refused with code `capture`, detail code `capture_changed`, message "Captured external profile changed", zero content reads (`planar_admission_read_attempts() == 0`), admission poisoned (`p.verify()` false). Test: `source capture vanished declaration reports admitted change beside embedded missing`, SECTION `declaration removed after admission is an admitted-change refusal with zero content reads`, [SC:465-479](../../../src/engine/acceptance/source_capture.t.cpp); the comment at SC:466 points to this record.
- (b) Declaration missing when supplied: admission refuses with raw code `io`, message "Cannot open confined input", field `profile` (SC:433-452). `known_declaration_error` ([`source_capture.cpp:103-107`](../../../src/engine/acceptance/source_capture.cpp)) adapts exactly that pair to the load-family diagnostic code `io`, message "Cannot inspect confined input", empty limit, maximum 0, equal to the `open_embedded` diagnostic; every other code passes through unchanged (SC:454-464: `path_denied` stays `capture`, `resource_limit` stays `resource_limit`).
- The adapter is not yet wired: nothing in the product calls `known_declaration_error`; the runtime row owns that call and the ordering against real registration.
- Advisories carried forward (wording from the [C01 review](../../../docs/vernacular/rounds/V5-02-X06-B2-C01-review.md), "Advisory notes"):
  - **X06B2C01-N1**, the adapter keys on a message, not on absence: on macOS the metadata open uses `O_EVTONLY`, which fails with EACCES on a mode-000 file, so an unreadable declaration would also be rewritten to "Cannot inspect confined input" while the embedded path reports "Cannot open confined input". The runtime row should distinguish absence at admission or document the macOS difference.
  - **X06B2C01-N2**, no role check on supplied receipts: the factory does not check the declaration or registry role of the receipts. Not a defect given existing mechanisms; the runtime row may add an explicit role predicate.
  - **X06B2C01-N3**: the construction refusal reports `capture_changed` / "Captured external profile changed" for moved-from, poisoned and foreign receipts alike; a later reviewer may prefer `invalid_state` for a moved-from handle.

## Limitations

Debug only; no RelWithDebInfo, Linux or full release gate. Candidate is a dirty tree, so the identity is a diff hash plus untracked-file hash rather than a commit. The Doxygen retry is a known flake, not a clean first pass.
