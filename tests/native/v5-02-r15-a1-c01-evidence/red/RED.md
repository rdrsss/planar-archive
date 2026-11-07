# RED evidence V5-02-R15-A1-C01 (rewritten in C02)

## Irrelevant first attempt

The earlier text of this file described a `cmake --build build/rewrite-debug --target planar_tests` build and 69 `ctest -R ... --no-tests=error` runs that each reported "No tests were found", with every check marked not-registered and logs under `red/raw/*.log`. That was an irrelevant first attempt: no CHK-* check is a ctest test (this is a documentation round with no TEST_CASE), the attempt touched the build directory, and it is not red evidence. The logs are kept, not deleted, and are not cited as evidence. (`SHA256SUMS` of the earlier data files: 5153b654e79aa8e94d69af4b13dd3e7d0fdb9ba2fa132eb4b3a19a3561b8f068)

## The real red

The red observation is the repaired checker run in default mode on the pre-C01 candidate (snapshot in `raw/pre-c01-snapshot`, candidate diff in `raw/pre-c01-candidate.diff`, comparison verdicts in `raw/pre-c01-snapshot-cmp.txt`).

Command, from `red/red-run.txt`:

```text
python3 -I <scratchpad>/ex_checks.py --repo <repo root>
```

The command uses `ex_checks.py --repo` with the repository root and a scratchpad copy of the checker (placeholders `<scratchpad>` and `<repo root>`; `red/ex_checks.py` is the evidence copy). The pre-fix identity lines, pasted verbatim from `red/red-run.txt`:

```text
# identity.py output for IDSPEC_DOCS, verbatim. Command: python3 <identity.py> docs/vernacular/execution-contract.md docs/vernacular/v5-alignment.md docs/vernacular/ledger-contract.md docs/vernacular/external-profile-contract.md docs/vernacular/acceptance-matrix.md docs/vernacular/implementation.md CLAUDE.md
| identity | value |
|---|---|
| HEAD | `d536922b59bf9118b91e4188a26d770f9745d600` |
| sha256(git diff --binary HEAD -- docs/vernacular/execution-contract.md docs/vernacular/v5-alignment.md docs/vernacular/ledger-contract.md docs/vernacular/external-profile-contract.md docs/vernacular/acceptance-matrix.md docs/vernacular/implementation.md CLAUDE.md), tracked only | `eb2deed97a9fa03401a5625c75d0ccbd4d869c314457b5b634fa474253f27302` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec docs/vernacular/execution-contract.md docs/vernacular/v5-alignment.md docs/vernacular/ledger-contract.md docs/vernacular/external-profile-contract.md docs/vernacular/acceptance-matrix.md docs/vernacular/implementation.md CLAUDE.md, evidence excluded | `b7e34425d33cc06c10492bf3c5a6162e2d8681477af64fb0510d3cbb0b591bb7` |
| file | state | sha256 |
|---|---|---|
| `CLAUDE.md` | modified | `7cdf4eadca490e6d3f6c6c697cd373c2c8a683f41612de02f98dce8b69b72163` |
| `docs/vernacular/acceptance-matrix.md` | modified | `6370cb09d99eaa33bf068f787e6ed040c3a81d160d6d2cbba47b3944e7178be5` |
| `docs/vernacular/external-profile-contract.md` | modified | `e190e2d1d88ad0a40cab3bd4bb31bbe826774a58f729224d083abcac877577da` |
| `docs/vernacular/implementation.md` | modified | `486a80a78d247e45badd73b2b96184bb952317d1229dbdba94d93cdd734e35fb` |
| `docs/vernacular/ledger-contract.md` | modified | `3e595653feeaa63f824f1f0bbf929a4923f10854dbcdd3e35725b8ff6223ceaf` |
| `docs/vernacular/v5-alignment.md` | modified | `23c6b7f86b0cbad42c26d455e810c7e7788e3c096a752c7f31a484f8041166fc` |
| `docs/vernacular/execution-contract.md` | untracked | `fd4c9134001eb960b9ca4f4357a29d3ff3d29589608d42d87848c6db77145ad1` |
# identity IDSPEC_DOCS exit code: 0
# identity.py output for IDSPEC_SRC, verbatim. Command: python3 <identity.py> src tests
| identity | value |
|---|---|
| HEAD | `d536922b59bf9118b91e4188a26d770f9745d600` |
| sha256(git diff --binary HEAD -- src tests), tracked only | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests, evidence excluded | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| file | state | sha256 |
|---|---|---|
# identity IDSPEC_SRC exit code: 0
```

Result: 70 PASS and 29 FAIL, exit code 1. `red/red-audit-run.txt` is the audit-mode run on the same snapshot and adds the six audit FAIL lines.

The 29 failing checks (every line is an assertion message, not a missing-file or traceback line):

- CHK-I32-decisions-table
- CHK-I33-confirm-stopped
- CHK-I48-enum-checks
- CHK-I54-run-token
- CHK-I57-payload-version
- CHK-I59-record-additions
- CHK-I67-column-conventions
- CHK-C01-R1-token-status-lookup
- CHK-C01-R1-status-qualifier
- CHK-C01-R1-a09-lookup
- CHK-C01-R2-event-reason-ordinal1
- CHK-C01-R3-op1-rule-unchanged
- CHK-C01-R4-role-state-run-drift
- CHK-C01-R4-transitions-drifted
- CHK-C01-R4-run-id-replay-text
- CHK-C01-R4-states-text
- CHK-C01-R5-c6-text
- CHK-C01-R5-evidence-reason-check
- CHK-C01-decisions-rows
- CHK-C01-R7-header-scope
- CHK-C01-R8-m-note
- CHK-C01-R9-d56-basis
- CHK-C01-R9-d55-corruption
- CHK-C01-R10-alignment-pointers
- CHK-C01-R11-cutover-target
- CHK-C01-R12-ex8-label
- CHK-C01-relabel-contract
- CHK-C01-relabel-record
- CHK-C01-relabel-no-live

## Raw logs and the C02 red

- `raw/red-checks/` holds 72 files: 71 logs and SUMMARY.txt. The 71 logs are one log per C01 invariant row (C1 to C61, C59b and G1 to G8; C59b has two logs). 34 of the logs record a FAIL of their own check: 29 in default mode (the FAIL set of `red/red-run.txt`) and 5 audit-only (`CHK-C01-green-evidence`, `CHK-C01-prober-evidence`, `CHK-C01-record-closure`, `CHK-C01-record-d81` and `CHK-C01-sql-evidence-recorded`, which FAIL only in `red/red-audit-run.txt`); the other 37 record a PASS. Corrected in C04 (the C03 review, R6); earlier text said 72 logs and 29 FAIL.
- `raw/orig-checker-run.txt` is the original checker run: 15 FAIL lines against the 13 of the 2026-11-01 run; I45 and I59 ran on a tree that had grown (record section C01-R8 explains it).
- `raw/pre-c01-snapshot`, `raw/pre-c01-candidate.diff` and `raw/pre-c01-snapshot-cmp.txt` are the pre-C01 snapshot, its diff and its comparison.
- C02 red on the pre-fix tree (the C01 candidate before any C02 edit): `red/red-c02-run.txt` and `red/red-c02-audit-run.txt`. The expected FAIL sets are 12 in default mode and 19 in audit mode; the CHK wave wrote both files.

## Limit

The red run is behavioural, but 62 of the 67 first-run failures of the original checker (record section (j)) prove absence only: they report a missing document, section, table or object. The 29 failures above are assertion failures on the pre-C01 candidate. No hash of a record or REPORT is written here.
