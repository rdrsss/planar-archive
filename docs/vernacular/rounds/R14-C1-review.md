# R14-C1 independent review

Disposition: **approve**. No blocking findings or requested changes.

Reviewed the exact bounded typed-read candidate on
`dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, in the shared checkout. Existing
rewrite changes were preserved. The reviewer owned only this report and
`tests/native/r14c1-review-evidence/`; product/source/evidence from the coder
remained read-only. No Planar tools, operator state, legacy binary, commits,
installation or publishing were used. No subagents were dispatched by this
reviewer.

The reviewed candidate identity is
`32decfef27b4453d44f705aaac1cfc108ffc6abd3af7d6763353a5084619f07a`.
The coder candidate manifest SHA256 is
`c7dbf4344e8e34604998e307f81d21913853c8e1bc393035b72c77bdb8dad806`;
the coder raw-evidence manifest SHA256 is
`af7c2ee05d943c8e13cdde27ed45506ee69d5575d5e2ef05f8f1da4610a34462`.
Independently recomputed the embedded identity, checked all six live/frozen owned
files, all 44 coder evidence entries and all 12,363 protected hashes. Preserved
the exact B2 test-file prefix, engine CMake, schema/init payload and AGENTS
symlink. Reviewed the candidate diff and raw logs, including the documented
preliminary overlap and final serialized replacements.

The implementation satisfies the R14-C1 handoff:

- DTOs own every returned string and retain exact stored event/result bytes and
  request record_v1 bytes. The raw body is read only after bounded record/type
  validation, rather than re-encoding a request
  (`src/engine/ledger/format.cpp:667`, `:695`, `:700`; `format.cppm:24`).
  Existing full history verification establishes cross-links in the pinned
  snapshot. State-at-event composes pure event after-state and revision reads.
- Each bounded column decoder checks octet length before copying and distinguishes
  wrapper column/type failures from actual SQLite storage errors
  (`format.cpp:531`, `:541`). The bounds match request128, actor256, time24,
  event/result1MiB and encoded-record64MiB. Shared codec enums/reference types
  are explicitly qualified in exported DTOs; both generated namespace variants
  regenerate exactly and reverse identically.
- Valid absent revision/event/request keys return successful optional misses.
  Invalid arguments and actual read failures poison all reads through the same
  existing owner, preserving cached metadata (`format.cpp:1157`, `:1166`,
  `:1176`, `:1214`). Moves transfer validity; moved-from reads refuse safely.
  Metadata explicitly excludes moved-from use (`format.cppm:76`).
- Pages enforce cursor0..INT64_MAX and explicit limit1..100, use ascending
  exclusive SQL selection, and derive continuation only from this verified
  snapshot's last sequence (`format.cpp:1238`). No unbounded history container,
  raw SQL/SQLite product escape hatch, mutation API, Lua execution or policy
  reinterpretation is introduced. The DELETE-journal policy remains unchanged.

Independent verification was serialized in the exclusively held debug build
folder. Commands and raw results are preserved in
[review evidence](../../../tests/native/r14c1-review-evidence/README.md):

- `cmake --build --preset debug --target all planar_tests`: exit0, no work.
- Focused `replacement.(ledger|canonical|hash|record|tree|provenance|interpretation)`:
  **151/151** pass, zero skips, including all prior143. This reruns exact saved
  row/body/result SQL-byte comparisons, 115-event bounded pagination, optional
  misses and malformed arguments, unsupported interpretation raw reads, and
  DELETE commit-busy/pinned-page/release/commit/fresh-open behavior.
- Three independent reviewer probe cases: **731 assertions**, all pass. These
  add unsigned maximum IDs/cursors, SIZE_MAX limits, non-ASCII and whitespace
  keys, legal token punctuation, poison through both move operations, owned
  result lifetimes and cursor/limit edge combinations. A reviewer-only VFS
  forces real SQLITE_IOERR_READ independently for all **six** read methods:
  head, revision, event, request, page and existing record. Each returns typed
  I/O failure; every subsequent read refuses; cached metadata remains; fresh
  open succeeds. Six fault hits are visible in `probes.txt`.
- `replacement.cli`: **247 real processes**, zero skips, pass.
- Both vector checks pass: 12 hand byte vectors, 20 upstream hashes,
  41 canonical/domain vectors, and 10 independent record vectors.
- Formatting/Doxygen: **64 active C++ files**, pass. Exact generated/reversible
  namespace audits, candidate/evidence hashes and 12,363 protected hashes pass.
- Final all-plus-tests build: exit0, exactly `ninja: no work to do.`

Two initial reviewer-probe harness issues are preserved and explained in the
review evidence: a helper incorrectly called metadata on a moved-from owner,
and a cached page prevented the first attempted VFS fault. The final probes
correct the harness only, retain both preliminary outputs and run with zero
failures/skips. These are not candidate findings. Reviewer-only compiled
artifacts were removed after recording their hashes; the build script preserves
exact compile/link commands and can recreate them.

Reviewer raw-evidence manifest SHA256:
`fbe80e75a947012816f06bf9b3c10a9cd0da7e4efab17a134f15bf8fbb86c956`.
It is `tests/native/r14c1-review-evidence/evidence.json`; this report is hashed
separately at handoff to avoid a circular manifest.

Approval is limited to native C1 bounded typed reads. Verification ran on macOS
arm64 debug. Release/Linux and the integrated R14 acceptance writer, public
history/load CLI and export gates remain outstanding. The candidate remains
unchanged at review completion. Debug build ownership is released to the
coordinator; the reviewer has no remaining build/test process.
