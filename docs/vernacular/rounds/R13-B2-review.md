# R13-B2 independent review

Disposition: **approve**. No blocking or requested-change finding remains.
This approves complete immutable trees and the narrow writer-abort addition;
R13-C provenance, integrated R13 debug/release and R14 acceptance/export remain
outside this approval.

## Scope and exact candidate

Fresh reviewer `/root/r13b2_reviewer` reviewed shared
`dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Read the active repository
instructions, implementation/roadmap, v4 and ledger contract, R13/B/B2 briefs,
corrected B1 approval, candidate diff and raw implementation evidence. Product
files were read-only. Writes were confined to this report and
`tests/native/r13b2-review-evidence/`; no edits or evidence were reverted. No
Planar tools/skills/queue, operator data, installed binary, subagents, commits,
installation, publication or merge were used.

Candidate manifest `tests/native/r13b2-evidence/candidate.json` SHA256:
`2922ca995c29d62059014442dca7dd0c9a771160ccbdc6cf2a8bed67b60ef928`.
Nine-file candidate identity:
`7328603aef4edf6a49cdd62a92274b6d046b55f7ae2820a1e42c517d4cc40ec0`.
Candidate keys are repository-relative, with exact snapshots under the evidence
directory's `candidate/`. Identity hashes the compact sorted-key UTF-8 files map
with comma/colon separators and no LF. All live files and snapshots match; the
candidate diff was independently reconstructed exactly from preserved baselines.

Coder evidence manifest SHA256:
`6cc2a2ae3e6da470247cf80327fc4d7fb5f15fcaa05c75a57174f2d72fa8d259`.
All 42 folder-relative entries match. Original B1/B1-review/C01/C01-review
evidence entries (50/16/48/18) and all 3746 repository-relative protected inputs
also match. A final independent audit rechecked candidate and evidence after
verification. The initial coder audit KeyError remains preserved; its correction
changes only the local audit's manifest-schema handling.

## Technical assessment

`tree.cppm` defines the concrete sealed one-root view API, complete owned output,
fixed profiles, root/depth counting and caller-owned transactions. Root is a
directory at depth zero and counts once; file segments count toward depth 64.
Source permits 64 MiB, 4096 files and 8192 directories. Provenance independently
permits 16 MiB, 4096 files and the induced maximum 258049 directories. Both
retain the 16 MiB single-file and 1024-byte path ceilings. Only file ancestors
and root are represented, and reconstruction refuses other empty directories.

At `src/engine/history/tree.cpp:116`, construction checks input count, bytes,
depth, length, portable grammar and normalized modes before building the bounded
local-name trie. Case-folded sibling keys detect aliases; exact leaf/prefix
checks reject duplicates and file/directory conflicts in either order. Reverse
insertion order is a valid bottom-up traversal; directory children are sorted
by their original ASCII names. Only the typed writer performs record insertion
and complete bucket comparison. Portable digests remain independent of local IDs.

At `tree.cpp:58`, reconstruction uses exact root kind/digest and validated record
references, rejects value/request descendants, and counts every expanded child
occurrence before allocating traversal entries. Payload totals likewise count
each expanded file occurrence. Directory limits bound traversal independently
of unique record IDs. One decoded record is retained per iteration, avoiding a
maximum-size record per recursion level; pending paths and owned output have
explicit aggregate count/path/byte bounds. Complete sorted output owns bytes
beyond reader lifetime. Older roots reconstruct from recorded bytes without
timestamps, filesystem capture, patch replay or current Lua policy.

The new `record_writer::abort() noexcept` at `format.cpp:740` only releases its
existing transaction state. It is idempotent and makes no new rollback-error
reporting claim. Both writer tree APIs establish an abort guard before allocating
or validating. Every expected refusal and unwinding exception abandons the
entire writer, including earlier pending records; only success disarms the
guard. Neither API commits. Typed storage errors retain the original ledger
error. Reader tree-local refusal does not invent a storage failure or replace
the snapshot; actual record-read failure retains B1's sticky reader semantics.

The ledger diff is limited to abort declaration/definition and its test. Schema,
SQLite primitive, record codec, reader implementation and existing collision
helpers remain protected. Build dependencies descend from trees to format/value,
then format to pure records/SQLite; no ledger-to-tree edge, raw SQL surface,
filesystem/Lua capability or product hash override was introduced. Reference
documentation agrees with the API and observed registry.

The permanent tests cover exact/excessive bytes, files, directories, depth and
paths, profile differences, shared expansion to 258049 directory occurrences,
binary/CRLF/empty/executable files, sorting, old roots, reuse and owned lifetime.
The maximum-ID case seeds ID max-1, inserts a novel file at max, then fails its
parent allocation with original `unsupported_state`; fresh verified reads prove
the inserted prefix absent and commit unavailable. Existing forced file,
directory, value and request collisions plus both ambiguous-bucket orders and
exact local-child-ID comparison remain active. The pure codec/storage layers
continue to reject malformed stored topology/cycles before tree traversal.

## Independent verification

Direct commands, logs, JUnit, generation snapshots and the independent audit
are retained in [review evidence](../../../tests/native/r13b2-review-evidence/README.md).
Checks ran serially with exclusive debug ownership.

| Check | Result |
| --- | --- |
| `cmake --build --preset debug --target all planar_tests` | Exit 0; no work before tests and in final repeated build. |
| Focused CTest `replacement.(ledger\|canonical\|hash\|record\|tree)` | 93/93 passed, zero failures/skips; 13 tree cases, retained collision/I/O-fault cases; 35.24 seconds. |
| Independent exception/profile harness linked to candidate modules | One case, four scenarios, 46 assertions passed; actual one-shot allocation exceptions in build/read abort prior pending inserts; invalid profiles also abort. |
| Existing vector generator `--check` | 12 hand-byte, 20 upstream-hash, 41 canonical/domain vectors pass. |
| Record vector generator `--check` | Ten independent vectors pass. |
| Active formatting/Doxygen | 44 source files pass. |
| Supplementary helper formatting/Doxygen | Two files pass, zero warnings. |
| Independent protection/generation audit | Exact candidate/diff/evidence/protection matches; fresh reversible C2/B1 generation matches configured bytes/manifests; both B1 digest sites redirected with no equality mutation. |

The first independent harness invocation failed before reaching a probe because
its temporary `/tmp` path crossed macOS's symlinked prefix; the safe opener
correctly refused initialization. Initial source/log and exit 42 remain in the
evidence. Canonicalizing that owned scratch root, as the product tests do,
resolved the verification setup error without changing product files.

This is macOS Apple Silicon debug verification. The allocation probe covers the
first allocation inside each guarded API, not exhaustive allocator failures.
Integrated R13 debug/release and Linux execution remain future gates; no full
210-case run is claimed here. No product choice remains open in this slice.
The reviewed candidate is unchanged and debug is released on delivery.

Independent evidence manifest has 23 folder-relative entries; SHA256:
`c8653d82555b650dffa67da565703b323ac85294e80c7a3fb39af009625e45a6`.
This report is outside that manifest and receives a separate delivery hash.
