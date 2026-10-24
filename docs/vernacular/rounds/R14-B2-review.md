# R14-B2 independent review

Verdict: **approve**. No blocking findings or unresolved product questions in the
assigned complete-history verification slice. This is a review of the exact
uncommitted candidate below, not approval of the later acceptance writer/CLI.

## Identity, authority and scope

Reviewed on `dev/planar-v2-vernacular` in the shared current checkout. Read the
active CLAUDE instructions, implementation plan, roadmap, R14-B/B2 round briefs,
ledger contract, applicable v4 persistence/history design and the candidate
report. No product changes, Planar tools, installed binary, operator database,
commits, installation, publication or merge. Reviewer writes are confined to this
record and `tests/native/r14b2-review-evidence/`.

Candidate identity:
`704c8c5f514d6159b6351ac169207f868e009bc36d8de39ab15db8568501de87`.
The seven files match both current source and frozen copies in the candidate
manifest. Manifest SHA256:
`fe1e3ed28b588eb7ab65d8555166b6f5c57debcb50bbb828e6f799eb2df41666`.
Coder evidence manifest SHA256:
`48b8641a676ba9f01153a252912eb67a8049d09c6ed81150fe6706a2eb9d2cb7`.
All 61 raw members and 12,283 protected files were independently hashed and match.
The explicit public-module ownership expansion changes comments only; schema and
initial-payload definitions remain unchanged. Final candidate and coder evidence
hashes were checked again after verification. See reviewer `identity.json` and
`audit.txt`; reviewer `manifest.json` freezes this record and its raw evidence.

## Review findings and reasoning

The shared private verifier is used by RO/RW open and transition, write
acquisition, precommit, staged backup/restore and scratch/original quiescent
recovery. `format.cpp:622` streams ordered events with fixed head/parent/five-ref
state; initialization, gap-free sequence, before/after continuity, first revision
1, subsequent increasing IDs with immediate prior-head parent, no-op reason and
unchanged exact references, actor/time, saved result, body operation/actor/expected
head and current projection are checked. Unique request event references plus
per-event reverse lookup and final row counts reject orphan requests/revisions
and repeated acceptance (`format.cpp:724`, `format.cpp:763`). Syntax validation
places no causal ordering on timestamps; positive revision gaps remain accepted.

`format.cpp:531` independently reconstructs and releases bounded source, registry
and stdlib trees, validates registry layout and exact stdlib manifest/digest, then
checks fixed canonical interpretation/separate-version equality and historical
workspace/provenance metadata. The unsupported-interpretation fallback is reached
only after `decode_interpretation` performs its fixed shape and canonical checks
(`interpretation_codec.cpp:140`); it extracts already validated metadata with the
same explicit canonical bounds. It neither reruns Lua nor rederives semantic
capture closure. Those are acceptance preparation/writer obligations.

Exact no-op classification compares complete record encodings when IDs differ
(`format.cpp:602`). The preceding record pass checks every digest-bucket member;
unequal directory child-ID encodings with equal portable digest refuse as
ambiguous, and duplicate equivalent IDs cannot manufacture a changed acceptance.
Source comments may change exact history while normalized interpretation remains
unchanged. The positive fixture changes an existing captured workspace declaration
comment, with initial/change/no-op/revert relationships and backward clocks.

Untrusted variable columns are length-checked before copying; event type and
request outcome have fixed schema CHECK vocabularies verified by integrity_check
before reading them. Storage prepare/bind/step failures retain their owning
SQLite error category through the verifier and pure tree reader. The native
retained state is bounded per record/tree/value, independent of total history;
there is no history-sized cache. Production and both generated format variants
link pure history codecs/tree_core, without an owner-wrapper/Lua dependency cycle.

The early unknown-event-row-version check (`format.cpp:818`) occurs only after
exact header/schema recognition and always refuses. This is appropriate version
negotiation: otherwise v1's CHECK(version=1) masks an unknown row codec as generic
corruption. It provides no read/write success path and skips no integrity check
for a successfully opened database. Event/result unknown payload versions remain
unsupported_state; outer header format remains unsupported_version. Unsupported
interpretation policy versions alone may retain raw-record access after the fixed
integrity checks; unknown record/payload codecs are still refused.

## Independent execution and raw evidence

Executed directly and serially in the exclusively held debug directory:

- `cmake --build --preset debug --target all planar_tests`: passed, no work.
  Raw `build.txt`.
- `ctest --test-dir build/rewrite-debug -R 'replacement.(ledger|canonical|hash|record|tree|provenance|interpretation)' --output-on-failure --output-junit <repo>/tests/native/r14b2-review-evidence/focused.xml`:
  **143 passed, zero failures/skips**, including the ten history cases and all
  prior retained cases. Raw `focused.txt`/`focused.xml`.
- `python3 tests/native/r14b2-review-evidence/build_probes.py`: compiled an owned
  translation unit including only the candidate's scratch fixture and independent
  probes; linked unchanged candidate archives, without changing CMake or product
  objects. Exact compile/link commands in `probe-build.txt`.
- `tests/native/r14b2-review-evidence/probes '[review]' --reporter console`:
  **five cases, 1,344 assertions passed**, zero skips. Raw `probes-final.txt`.
  Independent probes cover reciprocal request/event mismatch, init metadata,
  event/result reference/identity/reason corruption, and unsupported interpretation
  with mutually consistent revision/event references but wrong separate versions,
  stdlib or registry content. Real exclusive SQLite locking returns busy and
  preserves bytes. A test-only default VFS forwards real operations and returns
  SQLITE_IOERR_READ during accepted-revision lookup; open returns io and preserves
  bytes. The same targeted read failure during precommit returns io, rolls back
  the tentative record, and leaves exact original database bytes reopenable.
- Independently regenerated both fault/digest variants into temporary directories,
  compared configured format source/module/generation manifests byte-for-byte,
  confirmed every reverse transform is identical, and confirmed the digest
  variant has no verification mutation. `audit.txt`.
- Final `cmake --build --preset debug --target all planar_tests`: exactly
  `ninja: no work to do.` Raw `no-work.txt`.

The probe implementation and reconstruction script remain in reviewer evidence.
Disposable probe executable/object were hashed in `probe-products.json` then
removed; no generated reviewer product enters the default graph. The earlier
four-case `probes.txt` log is retained; `probes-final.txt` is the complete final
five-case run.

Also inspected and hash-verified coder raw evidence for CLI247, vector checks,
64-file pinned formatting/Doxygen, exact product/format dependency audits, real
hot-journal history recovery, and the pre-B2 positive-fixture failure after 71
passing assertions. These checks were not redundantly rerun beyond the independent
focused suite, generated-variant audit and no-work builds above.

## Limits and disposition

This is macOS debug verification. Release/Linux and the eventual integrated R14
gate remain outstanding as documented. The owned actual VFS probes target open
and precommit revision lookup, not every SQL site or filesystem publication
phase. Existing focused storage/full/collision/POSIX fault cases remain passing;
there is no claim of exhaustive fault placement. No new acceptance writer,
history CLI or filesystem export is enabled in this slice.

The reviewed candidate remains exact. The debug build directory is released to
the coordinator. Reviewer work is complete; this agent should be retired from
subsequent dispatch without implying a termination API was invoked.
