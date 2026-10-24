# R12-B-C01 independent review

Disposition: **approve** for the corrected full R12-B candidate. Both previous
findings are resolved; no new finding was identified. This approves the bounded
initialized-state format slice and does not complete R12.

## Identity and review boundary

Fresh reviewer `/root/r12bc01_reviewer`, ordinary gpt-6.1-sol with high reasoning,
reviewed the shared `dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Writes were limited to this report
and `tests/native/r12bc01-review-evidence/`; other agents' changes were preserved.
No product edits, legacy operational tools, installed binaries, operator data,
commits, installation, publication or merge were used. Builds were serialized
with exclusive access to the existing debug directory. All probe databases used
explicit temporary roots and were removed afterward.

The corrected seven-file candidate manifest has SHA256
`a068dd223db5786d2bbcb96ce48a52c801e977862810a84101f4ff3f13025071`.
The 39-entry correction evidence manifest has SHA256
`d8038ade832f2aecfe1070c4467c8f0ccc6505b2b7636e36276a82107f56807f`.
Every entry matched independently before and after verification. Both seven-file
before/current snapshots match their manifests; all three protected primitive
files and all 52 historical evidence entries match their preserved hashes.
See `identity-before.json`, `identity-after.json` and
`protected-and-snapshot-audit.json` in the new reviewer evidence directory.

Review basis: active repository instructions, implementation plan/roadmap,
relevant v4 persistence/acceptance requirements, normative ledger contract,
R12/R12-B/C01 briefs, original independent review, full seven-file corrected
candidate, complete correction diff and raw RED/GREEN evidence. The prior full-B
review evidence was reused after hash verification, with independent source
inspection of the complete format implementation/interface/tests, build wiring,
schema fixture, affected references and primitive text-copy path.

## Findings resolved

**R12-B-F01 (P1): resolved.** `format.cpp:570` checks destination sidecars before
random identity/staging work. `format.cpp:615` repeats the same retained-parent
check immediately before atomic no-replace `linkat`. The helper at line 285 uses
`fstatat(..., AT_SYMLINK_NOFOLLOW)`, so existing links, including dangling links,
and nonregular entries refuse without being followed. The late refusal passes
through existing owned staging cleanup; no published target is removed on
uncertainty. This fits the existing cooperative-directory contract.

The original nine-case RED evidence is preserved: three suffixes times regular
file/symlink/directory incorrectly published targets, with 27 failed assertions.
The new native case and independent current executable both refuse all nine,
retain every original sibling entry and publish no target. The broader probe
also verifies successful initialization leaves only a 0600 target, and immediate
read-only/read-write reopen preserves the directory snapshot. The late recheck
is verified by source inspection; no deterministic mid-initialization filesystem
interposition was added or claimed.

**R12-B-F02 (P2): resolved.** `format.cpp:319` uses `octet_length` for all four
persisted schema TEXT columns; all four integer guards execute before the wrapper
copies name/type/table/SQL at line 334. `format.cpp:374` uses byte lengths for all
three metadata TEXT columns; identity/time exact lengths and workspace 1–64-byte
bounds execute before their copies at line 386. Pinned SQLite's byte-length
implementation counts embedded NUL bytes; its function registration/direct-column
optimization were inspected. The primitive owns the copy using
`sqlite3_column_bytes`, so this ordering fixes the actual allocation path.
Normative identity character CHECK and BLOB constraints remain unchanged.

Every remaining format text copy was audited: existing-file encoding, journal
mode and integrity-check results are SQLite-generated PRAGMA output; creation's
journal result is likewise generated. Initialization event values are compared
inside SQL without copying stored event text. No other persisted metadata/schema
text is copied ahead of its guard. No parser, product allocation hook, primitive
or vendor change was introduced.

The reviewer recompiled the standalone allocation probe against the current
module map/static libraries into a separate directory. Reviewer-created scratch
fixtures were opened using both the frozen before-fix executable and independently
linked current executable. All **12 expectations passed**: each of the three
NUL-suffixed metadata fields reproduces the original >2 MiB C++ request; all six
current NUL/multibyte fixtures refuse as `corrupt`, preserve file hashes and have
a largest C++ allocation request of **768 bytes**. Multibyte refusals already
passed before correction; they are not presented as RED regressions. This tests
the native owned-copy bound, not SQLite C allocations, RSS or total memory.

## Executed verification

Direct commands, with raw output preserved in the reviewer evidence directory:

```sh
cmake --build build/rewrite-debug --target all planar_tests
ctest --test-dir build/rewrite-debug -R replacement.ledger --output-on-failure --output-junit <repo>/tests/native/r12bc01-review-evidence/focused.xml
python3 tests/native/r12bc01-review-evidence/adversarial.py
python3 tests/native/r12bc01-review-evidence/allocation-check.py
```

The build reported `ninja: no work to do.` Focused CTest selected **28**, passed
**28**, failed **0**, skipped **0** (13 primitive and 15 format cases).
`focused.xml`, `focused.log` and `focused-counts.json` record the results.

The independent full-B adversarial script was copied unchanged to the reviewer's
directory and run against a freshly compiled/linked current probe. It passed
**24/24** checks, including the nine corrected sidecar cases, successful
init/reopen/snapshot locking, random identities, unsafe/missing target refusals
and initialized-state cross-row corruption refusals. Exact commands, sources,
objects, binaries, results and snapshots are retained separately from all coder
and previous reviewer artifacts. The historical RSS observations were repeated
by that unchanged script but are not used as an allocation threshold or gate.

The coder's exact frozen formatting/Doxygen log was inspected: **35 active
sources passed**, and its final no-change build did no work. The independent
build also did no work. A repeat broad lint/full-registry run was unnecessary
for this correction after those logs and identities were verified. A reviewer
artifact-only source-excerpt script initially exceeded a file's line count; it
was corrected and rerun successfully, without affecting product or test results.

`source-audit.txt` preserves the critical byte-guard, primitive-copy, SQLite
implementation and publication/cleanup excerpts. `manifest.json` freezes this
report and all reviewer artifacts except itself.

## Limits and advancement

Executed platform: macOS Apple Silicon debug. Linux execution, release/full
integration, recognized hot-journal recovery, backup/restore, storage/process
fault injection and the broader concurrency matrix remain R12-C. Cooperative
directory ownership remains required. Full record/tree and accepted-history
semantics remain R13/R14; explicit unsupported populated state is correct here.
No broader v4 execution or power-loss guarantee follows from this approval.

The coordinator may advance to R12-C after checking the approved candidate is
still current. Further edits affecting this approved scope require renewed
review. No change to the contract or operator product decision is needed.
