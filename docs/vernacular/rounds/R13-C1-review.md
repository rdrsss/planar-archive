# R13-C1 independent review

Disposition: **approve**. No blocking or requested-change finding remains.
Approval covers exact embedded provenance preparation, native historical
verification and the shared pure tree digest. C2 interpretation, the complete
R13 debug/release milestone and R14 acceptance/export remain pending.

## Scope and exact candidate

Fresh reviewer `/root/r13c1_reviewer` reviewed shared
`dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Read active instructions,
implementation/roadmap, v4, ledger contract, R13/C/C1 and B2 approval, candidate
diff, tests and raw evidence. The large pre-existing rewrite was preserved.
Product files were read-only; writes are confined to this report and
`tests/native/r13c1-review-evidence/`. No Planar operations/skills/queue,
operator state, installed binary, subagents, commits, installation, publication
or merge were used.

Candidate manifest `tests/native/r13c1-evidence/candidate.json` SHA256:
`96177933ccbdfb9ad82c6b9e76c90af4032d3aba6cfd6ce8b879610b3378ffd1`.
Ten-file candidate identity:
`8d992bfe0ac3796ae72b113b22e3fe5d11fa648f214561a2a1d341656ecb336f`.
Candidate keys are repository-relative with exact snapshots under the evidence
directory's `candidate/`. Identity hashes the compact sorted-key UTF-8 files
map with comma/colon separators and no LF. All live files and snapshots match;
the exact diff independently reconstructs from preserved baselines.

Coder evidence manifest SHA256:
`4954fcff06c7094edea2f41bfa0472b24aee2589f79bccb6fadb70882432aa4f`.
All 47 folder-relative entries match. Of 11,582 original repository-relative
protected inputs, 11,580 match; the only differences are unrelated Finder
`.DS_Store` and `docs/.DS_Store`. Exact before/current hashes are retained in
review evidence. The coordinator confirmed preservation of these OS metadata
changes; no files were reverted. All protected product/vendor/prior evidence
paths agree. Final audit rechecked candidate/evidence/protection after tests.

## Technical assessment

At `tree.cpp:116`, one templated construction path retains B2's profile, byte,
file, path, depth, mode, alias and topology validation, bounded trie, child order
and bottom-up record construction. `digest_tree` supplies original
`digest_record` hashing with positive placeholder child IDs; the portable record
preimage omits those IDs. It opens no database and adds no second Merkle recipe.
The real writer still uses original collision-safe interning and real local
references. The abort guard encloses the shared constructor before allocation
and disarms only on success. Storage errors retain their original vocabulary;
read/abort behavior and codec/schema/storage implementations are unchanged.

At `cmake/embed_lua.cmake:7`, each configured original retains exact byte
literals and a portable `src/stdlib`-relative name in selection order. The
existing chunk remains the same ordered bytes plus one LF after every original,
including the last. Runtime views access generated arrays only. Independent
generated-array inspection and native tests agree with all eight authored files;
CLI acceptance preserves existing rendering and identity behavior.

At `provenance.cpp:68`, registry preparation preserves exactly `input.json`
captured bytes and normalized 0644/0755 mode, without parsing or recapture.
Stdlib originals are separate from explicit synthetic assembly and canonical
manifest files. The fixed canonical_v1 manifest has exactly version 1, scheme
`ordered_concat_v1`, ordered original paths and exact separator bytes. These
retain boundaries/order even when concatenated chunks are equal. Only stdlib
modes normalize to 0644. Original paths cannot occupy case aliases of the
reserved namespace; shared tree validation owns the remaining path grammar.

At `provenance.cpp:72`, count/path/separator/payload sums bound metadata scratch
before encoding, canonical encoding uses the remaining payload budget, and the
combined total is checked before owning payloads. Original plus chunk plus
manifest count against the same 16 MiB/4096-file profile. Assembly agreement
uses views, avoiding a second concatenated scratch chunk. The complete count
boundary is 4094 originals plus two synthetic files, not 4096 originals.

At `provenance.cpp:115`, historical verification first bounds the complete
file inventory and validates tree topology/modes, then uses bounded canonical
decoding with fixed envelope/version/type checks. Exact membership, duplicate
paths, namespace aliases, missing/extra originals and chunk agreement are
revalidated through preparation. It executes neither Lua nor filesystem reads.
Strict canonical decoding and exact fixed fields make re-encoding reproduce the
recorded manifest, preserving the complete supplied tree identity.

Permanent tests include committed scratch-ledger intern/reopen/reconstruct/
verify and exact bytes/modes, pure versus stored digests across displaced local
IDs, original/order/separator/boundary changes, malformed manifests and synthetic-
inclusive limits. Both exact 16 MiB total and 4096 total files succeed; excess
refuses. Existing B2 rollback/collision/graph/profile cases remain active.
Dependencies descend from provenance to generated source, canonical and tree;
ledger has no upward dependency. No Lua policy, schema, CLI or portable render
identity is changed. Architecture/testing references describe the actual scope.

## Independent verification

Exact argv, raw logs, JUnit, audits and probes live in
[review evidence](../../../tests/native/r13c1-review-evidence/README.md).
Checks ran directly and serially with exclusive debug ownership.

| Check | Result |
| --- | --- |
| Product/test build and final no-change build | Exit 0; both report `ninja: no work to do.` |
| Focused `replacement.(ledger\|canonical\|hash\|record\|tree\|provenance)` | 99/99 passed, zero skips/failures; 13 tree and six provenance cases; 35.93 seconds. |
| Existing CLI acceptance | 1/1 passed, zero skips; existing suite executes 247 binary invocations. |
| Independent manifest/order probe | Two cases, 52 assertions; identical-chunk order identity, binary separator, historical manifest/refusal matrix and maximum inventory pass. |
| B2 allocation/abort probe against current modules | One case, four scenarios, 46 assertions; actual one-shot allocation exceptions and invalid profiles abort earlier pending records. |
| Both vector checks | 12 hand-byte, 20 upstream-hash, 41 canonical/domain and ten independent record vectors pass. |
| Active plus helper lint | 47 active files and two verification helpers pass pinned formatting/Doxygen; no warnings. |
| Independent candidate/protection/generation audit | Exact candidate/diff, all coder evidence, all protected product/evidence, fresh C2/B1 variants and generated original/chunk bytes pass. |

The independent probe initially failed to compile because its own edits targeted
const shared children; cloned values corrected the harness without product edits.
Its original source/log remain preserved. A subsequent abort-probe link aborted
when the compiler's temporary `libLTO.dylib` symlink already existed; the exact
command passed on retry. Active Doxygen initially died from diagnostic-free
SIGBUS; its documented wrapper passed on attempt two. All original failures and
command results remain in evidence. The initial protection audit's two Finder
metadata differences and the coder's initial JUnit-path/overlap records are
preserved and explicitly explained; no product test failure was hidden.

Executed platform is macOS Apple Silicon debug. The registry contains 216 cases;
only the 99 focused cases and one CLI case were run here. The allocation probe
covers the first guarded allocation, not exhaustive allocator failures. No full
debug/release, Linux, C2 interpretation or R14 acceptance claim is made.
No unresolved product choice remains in C1. The candidate is current and debug
is released on delivery.

Independent evidence manifest has 34 folder-relative entries; SHA256:
`2604550a891c026caf77d3bb2694455f881ac2a936f778eea599acd64b84c0c5`.
