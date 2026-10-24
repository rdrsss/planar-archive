# V5-02-X03-B1 independent review

Disposition: **approve**, limited to native schema recognition, explicit format2
new-target initialization and lifecycle publication/backup/restore. No blocking
product finding. This does not approve B2 storage/acceptance or the external-profile
gate. Reviewed on shared `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, preserving the intentional rewrite.
Reviewer owned only this report and `tests/native/v5-02-x03-b1-review-evidence/`;
no source fixes, installed binaries, operator data, Planar tools, commits,
installation, publication or merge. Debug verification ownership is released.

## Exact reviewed candidate

All 16 manifest members match both working files and candidate-owned snapshots;
reconstructed owned diff matches byte for byte. Final author artifacts verify all
78 members. Reviewer initial copies preserve the observed handoff state.

| Identity | SHA256 |
| --- | --- |
| Candidate manifest | `470b63c257154091e71a121f4a223676c7985a894c68cec0465f5c1b9617b165` |
| Owned diff | `47b06d068a1e2a397ffbab461ffa947308ea76e2d43eae27ed662fdd2a131760` |
| Final author artifact manifest | `2b96c283867328337f66a4bdfbb41b5e5284ec45e0998e636cc3bca63f1fce53` |
| Frozen pre-B1 executable | `be1c4cee1b358208831faa919707ce5fd01de3ea31bc63db333a027f8844464a` |

## Source and boundary findings

- `src/engine/ledger/format.cpp:285` selects the exact v1 schema and changes only
  the two contracted CHECK expressions for v2. Header recognition at line297,
  selected-schema verification at line352 and `verify` at line983 enforce supported
  user_version, exact schema and matching ledger_meta.format. No ALTER, conversion
  or schema repair was added. The independent process probe also compares complete
  stored SQL to v1 with only the two normative substitutions.
- `format.cppm:14` adds an explicit enum discriminator to native metadata.
  `types.cppm` is byte-unchanged. No existing error enum values, record codecs,
  request/prepared representation, command signatures or JSON response fields
  changed. The native metadata struct grows; no cross-version native binary ABI
  compatibility is claimed. Runtime init response validation remains exact at
  `src/engine/runtime/runtime.cpp:1444`; the independent public-init probe confirms
  format1 and the existing six JSON keys.
- `format.cpp:1120` opens and verifies read-only first. At line1154 the format2
  guard precedes rollback and `connection::open(...read_write)` at line1163.
  `begin_records` at line1299 funnels through that opener. Both public acceptance
  overloads at lines1464/1470 funnel through the same implementation, which calls
  begin_records at line1484 before request relookup, CAS or interning. Existing
  record_writer owners can only arise through this guarded constructor path.
  No format1 request can enter a format2 file through these APIs in B1.
- Codec2 request presence refuses unsupported_state at `format.cpp:1029`, after
  exact schema selection and before record decoding. Codec1 files/requests retain
  native reads. Unknown/malformed cases remain closed. These are partial reader
  capabilities, not complete format2 accepted-history support.
- `recover_quiescent` at line1985 opens original descriptors read-only and refuses
  header2 at line2005 before the first mkdirat/scratch/writable operation.
  Missing/invalid recovery inputs may fail earlier; none bypass this restriction.
- `initialize_selected` at line1818 keeps public initialize_new on v1 and exposes
  only explicit native initialize_new_v2. `publish_new` at line1741 verifies staged
  metadata including format before atomic no-overwrite link publication.
  `backup_new` at line1841 verifies the source read-only, performs online backup
  into owned new staging, then verifies preserved identity/workspace/time/format;
  restore_new delegates to that path. This intentional new-target lifecycle write
  does not grant mutable access to an existing format2 ledger. Existing path,
  sidecar, sync, cleanup and uncertain-publication behavior is reused and tested.

## Independent verification

Direct commands, UTC times, status, raw logs, JUnit and LastTest are retained in
review evidence. Build-directory commands were serialized; independent read-only
lint/vector checks were batched after the focused run completed.

| Check | Result |
| --- | --- |
| `cmake --build build/rewrite-debug --target all planar_tests` | Pass, no work |
| Focused CTest regex `ledger\|history\|record\|request\|prepared\|acceptance\|publication` | 167 passed, 0 failed, 0 skipped; 46.71s |
| Current CTest inventory versus frozen baseline | 416 names; all 409 retained; exactly 7 added |
| `scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | Pinned format and Doxygen pass, 98 active files |
| Pinned formatting of both new support C++ files | Pass |
| Independent record/request-v2 vector oracles | 10 + 2 verified |
| Second build of all + planar_tests | No work |
| Reviewer process probe | 8 old-binary refusals; candidate history and load refusal unchanged; public init v1 and exact SQL delta verified |

The process probe invokes the frozen pre-B1 executable, never PATH/installed
Planar, against actual candidate-initialized format2 fixtures. History and load
both return exit3 unsupported_ledger_version for clean, journal, WAL and SHM cases;
complete recursive scratch directory hashes remain identical. HOME and PLANAR_DB
are scratch-only. Candidate history succeeds read-only, and candidate load with
real quota declarations/registry refuses mutation with exit3 and unchanged bytes.
The expected CLI mapping for native unsupported_state is the pre-existing
unsupported_ledger_version (`src/cmd/planar/response.cpp:110`). Reviewer attempt1
incorrectly expected unsupported_recorded_state; its script/log/result are retained,
and the corrected final probe passes without a product change.

## Evidence provenance and limits

The initial brief reported author artifact hash `5156bdd9bf004bbb9f169d881e0511849213337b6f983d726abcb70ea7847835`.
Coordinator and reviewer observed final `2b96...` instead. The author disclosed
an evidence-only edit after premature handoff: adding
old-binary-setup-attempt-1.txt from an earlier tool transcript, updating README,
and rerunning freeze.py (which also regenerates candidate-status and manifests).
The candidate manifest and owned diff remain exactly the reported identities.
The earlier artifact manifest is not available here for an independent historical
member-by-member comparison; this chronology is attributed to author/coordinator
messages. Approval binds the verified final 2b96... artifact, not the superseded
hash. The copied traceback is a reconstructed transcript excerpt, not a
contemporaneously preserved raw command log or an executable refusal success.
The observed final artifact remained stable throughout independent verification.
Future handoffs should freeze before release and explicitly announce any revision.

The 14,859-file baseline manifest was reconstructed from current inventory using
12 exact frozen owned pre-edit files; it is not a contemporaneous full-tree freeze.
Those 12 snapshots match baseline-owned.sha256 and baseline-manifest, and the
reviewer recomputed all baseline current hashes: only 11 declared owned files
changed, none are missing. This supports scoped preservation against the supplied
baseline, not retrospective proof that unrelated files never changed before its
reconstruction. Old-binary provenance is the supplied pre-edit copy/hash, baseline
no-work log and recorded dynamic dependency list; the reviewer reran the exact
frozen artifact but did not reproduce a pristine historical full-source build.
The author disclosed initial test compilation/setup failures; raw compile failure
and reconstructed setup excerpt are preserved. No success is inferred from them.

The focused suite includes format1 reliability/crash/full cases; these do not
prove format2 acceptance. B2 still owes typed v1/v2 stored request dispatch,
request equality/interner/collision redaction, CAS/replay/atomicity/crash/disk-full
and recovery integration. Public admission remains X07; no release/full milestone,
Linux or complete format2 accepted-history acceptance is claimed by this review.
