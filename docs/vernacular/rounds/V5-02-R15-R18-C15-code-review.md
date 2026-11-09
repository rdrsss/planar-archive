# C15 independent candidate review

**Disposition: approve the exact bounded C15 candidate.** No blocking finding
was identified. This approves the three Python blank-line edits and the single
prose EOF normalization below, with the stated evidence qualification. Final
integrated staged/publication acceptance remains the final auditor's task.

The review read the implementation-plan authority, roadmap opening, v5 baseline,
C15 brief, map, approved plan review, mapping decisions/invariants, proposed
pairs, actual coder report and complete coder manifest. Both scripts and their
CTest registrations were read in full. An ordinary available model substitutes
for the historically prescribed unavailable model. This is a fresh independent
candidate review, with no legacy Planar workflow.

## Actual candidate and independently reproduced proof

| Path | Tested-original SHA256 | Reviewed candidate SHA256 | Exact change |
| --- | --- | --- | --- |
| `tests/native/r15-r18-c02-policy-cli.py` | `0ce05f0966cbfa009d7febc360d3336544e6f75a59c4cc4a6c2bafb42c980138` | `8b254fa4370af2c0e6269fc6d5e8188abae0bcf90c99f48c6ac258be6d6a113e` | Four spaces removed from blank lines 24 and 74 |
| `tests/native/r15-r18-c03-release-cli.py` | `c24ea9561ce40014e56f05f76e4d35392ef987f36632ab06b559ef46759b4076` | `0bbdfe951b051fdde625272620cfa4d397fb5124304b3af1ac089aa0b5dbd3a4` | Four spaces removed from blank line 25 |

The reviewer reread actual source bytes and independently reconstructed each
candidate from its preserved tested original. Every other byte, newline and
line number is identical. The changes are outside literals. No statement,
assertion or fixture changed. Both original copies in mapping and coder evidence
match the pinned original hashes.

Independent verification with Python 3.15.2 established identical AST dumps
including all attributes and identical semantic token kinds, strings, positions
and source-line values (excluding only `NL` and `ENCODING` tokens). C02 has
2,117 semantic tokens; C03 has 1,241. Whole-module marshaling and each recursive
nested code object's marshaling are byte-identical at each matching relative,
actual host-absolute and `/home/planar/src/` Linux-absolute filename label, with
`flags=0`, `dont_inherit=True`, `optimize=0`, marshal version 6. C02 has six code
objects including its module; C03 has three. These are local interpreter proofs
using the recorded filename labels, not Linux interpreter executions.

The active consumer search returned exactly the two direct Python registrations
in `src/cmd/planar/CMakeLists.txt`. Neither script uses `__file__`, source
inspection or source hashing, or reads itself. Their `read_text()` calls read
only generated temporary `workspace/spec.lua`; fixture writes, explicit native
command invocations and C03 SQLite reads are unchanged. No consumer was found
that treats the removed blank-line spaces as data.

The historical `V5-02-R15-R18-CODE-REVIEW.md` changes only by removal of one final
LF: original `7d9f949a6788e4108232a407a61a8f96eace157609bdd26f218cefbb1b84a430`
to candidate `498a44f503173a52d207fe97586529ba70e982f632c2c39557ac71cd6c17c25c`.
Appending exactly one LF reproduces the retained original backup and its
`ORIGINALS.json` hash. That path occurs in neither standard nor temporary
path-redaction file sets. The original backup and binding were preserved.

All 272 C14 frozen `src/` entries and the complete current file set match,
including all 179 engine files and all 190 product `.cpp`, `.cppm` and Lua
inputs excluding tests. Bootstrap remains
`b594cc78b53a51892b70c9e0557ddde6c65c21ee615551d26c73b2156ff467b4`;
runner remains
`70ea1208b8a70020523a94e1e94a1c6d52b2aee23cbe67263f6c96bdf804c65f`.
C15 changes no native, calibration or probe source/component scope; existing
C14 component-identity qualifications remain applicable and unchanged.

## Focused execution evidence

The reviewer checked every hash/size binding in the coder's 30-entry manifest,
then independently accounted from raw verbose logs, exit files and JUnit XML:

| Existing host configuration | Registered tests selected/passed | Failed/skipped | Harness scenarios passed |
| --- | --- | --- | --- |
| Debug | 2/2 | 0/0 | 5/5 |
| Release | 2/2 | 0/0 | 5/5 |

Each raw log contains exactly once the C02 three-scenario and C03 two-scenario
zero-failure/zero-skip summaries, and each named PASS observation. Each XML names
exactly the two expected registrations, with no failure, error, disabled or
skipped case. Both CTest exits are zero. The explicit source-built configuration
binary is recorded in each command; the existing workflows use temporary
ledgers. This is four registration executions and ten scenarios. No empty
selection was accepted. No new native build or workflow rerun was performed by
this reviewer.

Reviewer command:

```sh
python3 tests/native/r15-r18-c15-evidence/review/verify.py
```

Final exit zero. The independent audit script, result, stdout, stderr and
consumer search are retained under the reviewer-owned ignored evidence folder;
`review/MANIFEST.json` binds them and this report. An initial reviewer-script
run reached the final result write and exposed a local variable reuse error;
that audit-only error was corrected before the complete successful rerun. No
candidate bytes changed during this review.

## Exact acceptance boundary

C15 is a bounded source corrective round, not a records-only exception. The four
complete C14 platform gates retain their tested-original byte scope. C15 adds
the exact interpreted-code identity proof for only the two pinned pairs and
focused host executions. It establishes no fresh full-gate execution, raw
current/Linux source-context equality, Linux execution or calibration measure.
The exact one-LF prose inverse is separate from all redaction allowances.

The final publication checker must admit only the two exact reviewed source
pairs after checking both sides and proof; it must reject any other non-record
source difference. No blanket path exemption is approved. Final current records
and evidence indexes must retain this qualification. Restaging these reviewed
bytes, a clean cached whitespace check and final independent integrated approval
remain necessary before the authorized signed-off local commit.

Only this report and ignored reviewer evidence were written. No source, test,
build input, original backup or staging entry was changed. No build,
measurement, installed binary, operator database, old Planar tool, commit,
installation, publication, push or merge was invoked. R19 remains unselected.
All reviewer-owned unignored writes are complete.
