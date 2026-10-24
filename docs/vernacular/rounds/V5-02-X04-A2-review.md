# V5-02-X04-A2 independent review

Disposition: **approve** for strict bounded native profile/ordered-module capture,
retained original receipts and staged metadata admission. No blocking findings.
This does not close integrated X04-A or the mandatory external-profile gate.

Reviewed in the shared `dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, against CLAUDE.md, the parent X04-A
split/brief, A2 brief and coordinator dispositions, approved A1 review,
implementation/roadmap/v5 direction, and X-CAP/X-LIMIT/X-CLI plus the bounded
implementation path. There is no separate parent X04-A approval report: that
integrated gate remains open. Reviewer writes are limited to this report and
`tests/native/v5-02-x04-a2-review-evidence/`; product and author evidence are
unchanged. No Planar tools, installed binary, operator data, commits, installation,
publication or merge were used. No subagents were dispatched.

## Exact candidate and evidence integrity

| Identity | SHA256 |
| --- | --- |
| Author candidate manifest | `9e1f7b32c0e1c09e71c44726e72568641fb5362b29bfacd36795f04f722c307a` |
| Author owned diff | `e22e9d36cf0f4a9c73ca333c4908e66f0cf905443c9184f943059f011360c63a` |
| Author artifact manifest | `2a3322aa18ac2123909989119f7213b36efcd21697565463eefd61b1868210e5` |
| Author final pass manifest | `fb27104739753e435db0501d2a8b45015699fbc974fbb79c6dfa27a5dab2aefc` |
| Author final source freeze | `88059c697f5982a3958fd21665d3b285fdf5e05703fa52cd6496467f3b264915` |

Independent integrity checks match all **17 candidate paths**, **183 author
artifacts**, each entry in pass09/pass10 archive and pass11 manifest, their frozen
source copies, and final live source/build identities. All **15,780** protected
final filesystem entries match. Recomputed baseline-to-final changes exactly
match the ownership audit: assigned files plus the explicitly coordinator-owned
A2 round record. Root-only `.git`/`build` exclusions retain nested historical build
paths; AGENTS.md remains the CLAUDE.md symlink. Native test source is unchanged.
The review manifest freezes this report and all reviewer evidence except itself.

## Boundary assessment

- `profile.cpp:182` parses through the profile-specific Glaze adapter, then uses
  strict library `validate_json` to reject the pair-range reader's permissive
  separators. Decoded keys preserve duplicate/escaped-key detection; unknown,
  missing, wrong-type and malformed values refuse. Valid raw integer views avoid
  an INT32/INT64/UINT64 classification cap; fractional/exponent spellings remain
  schema errors. The generic canonical/int32 codec and vendor sources are unchanged.
- `profile.cpp:94` captures a library-validated raw string view, checks its body
  against 6144 bytes, and delegates decoding to Glaze using fixed 6152-byte scratch.
  The actual pinned `json/read.hpp:1040–1341` unpadded reader checks its available
  tail, uses checked `resize(n)`/`resize(n+8)` before writes, and checked short-string
  `assign`. Its padded branch also resizes before chunk writes. With raw body at
  most 6144, eight extra bytes fit. Decoding cannot expand beyond raw length;
  1024 decoded bytes are checked before the retained string allocation. Raw
  escapes, surrogate pairs, exact bounds and invalid strings exercise these paths.
  Context `max_string_length` is not relied upon for JSON enforcement.
- `profile.cpp:119` bounds value nodes/depth. The pinned array/pair-range readers
  call `try_emplace_back` before element construction/growth; the adapter checks
  64 array elements and the 132 cumulative-value ceiling there. A resource
  refusal unwinds local parsing without exposing a partial closure. Full strict
  validation follows this bounded structural pass, so it is not the first walk
  over arbitrarily nested untrusted structure.
- `profile.cpp:228` applies portable byte/segment policy, exact `.lua`, reserved
  top segments, duplicate paths and case-folded path-prefix aliases. Full-list
  policy finishes before filesystem module selection. `open` at line285 captures
  the profile first, metadata-selects **all** listed modules, then reads modules
  in declared order. Every module is non-NUL UTF-8 text and initial bytecode ESC
  refuses. Unicode/spaced host profile filenames remain separate from portable
  module paths. Original profile whitespace/raw bytes, module order and normalized
  modes are retained in the immutable shared closure.
- `native.cpp:195` binds the opened descriptor to original A1 admission metadata
  before new size checks, allocation or reads. The bound descriptor enforces
  profile65536, module1MiB and remaining8MiB limits; cached bound calls enforce new
  caller ceilings too. The `bounded` receipt flag at line111 is retained/upgraded
  by `read_bounded`; verification at line357 limits rereads to original charged
  bytes before allocation/read. Existing unbounded callers keep their behavior.
- The profile owner keeps both A1 admission and native content capabilities alive.
  Its staged root/file/target checks and descriptor-only `verify_file` preserve
  original receipts and poison failed owners. Repeated `verify` surrounds native
  content verification with metadata checks; snapshots never change. Move/source
  failure and self-move semantics are explicit. Native fields/messages stay closed
  and logical; no new host selector, module contents or traceback enters errors.
- Known and late supplied slots can use these seams after closure capture without
  reading their contents. No Lua, CLI, provenance acceptance, ledger or handler
  wiring is inferred. Metadata and content budgets are local owners; later
  whole-invocation sharing of64MiB/10000 remains explicitly open.

## Executed independent verification

Exact argv, UTC starts, exits, durations, raw output and JUnit are under
`tests/native/v5-02-x04-a2-review-evidence/`.

1. Serialized debug `cmake --build build/rewrite-debug --target all planar_tests`
   passed and reported no work.
2. CTest inventory contains **488** tests, retains all **451** A1 names and adds
   exactly **37** profile cases. This is not a full488-test execution claim.
3. `ctest --test-dir build/rewrite-debug -R
   'replacement.*(profile|capture|path|acceptance)' --output-on-failure
   --output-junit <absolute reviewer path>` passed **162/162**, with zero failures,
   errors or skips, including all new profile cases and existing capture,
   admission, path and acceptance coverage.
4. Pinned formatting/Doxygen passed for **104** active sources. The final
   serialized `all planar_tests` build again reported no work.
5. Candidate hashes remained unchanged after every product command; final current
   candidate, author evidence and build identity checks precede evidence release.

Three initial reviewer-runner assertions were corrected before any product
command: archived pass manifest naming, valid JUnit system-out children and
symlink lstat modes. Those initial runner files and explanation remain retained.
No product failure was hidden or repaired by this reviewer.

## Author evidence and retained failures

Both raw pass09 and pass10 XML/logs establish162/162 with zero failures/errors/
skips. Their frozen sources match their archives. Final pass11 changes only
`profile.cpp` comments/formatting: independent token comparison matches pass09,
and the retained raw diff confirms the additions are documentation. Pass11's
build, equal inventory,104-source lint/Doxygen and no-work evidence is therefore
bound to those passing tests; the independent run additionally tests final bytes.

Pass09's canonical/record/request-v2 vectors, four inherited runtime/CLI audits
and five generated storage/export forward/reverse audits remain source-bound
supporting evidence. They were inspected and hash-verified, without repeating
unrelated suites. The author's pass09/10 Doxygen failures, initial compile and
fixture failures remain disclosed. Attempt01 lacks a pre-command source freeze
and is excluded as acceptance evidence.

The real bounded-reread regression is preserved: attempt06's exact-one-test run
observes **2** reads after growth where zero is required; attempt07 passes after
the narrow fix. Exact reconstructed native/test sources match attempt06 freeze
hashes. Incomplete exploratory reconstruction is retained and not treated as
exact proof. Independent final coverage passes sealing growth, acceptance growth,
cache-upgrade and tighter caller ceiling cases.

The linked observer is the unchanged A1 test-only interposer. The tests include
positive-controlled profile, module and supplied-input reads, followed by zero
watched attempts for full-list/module-metadata refusals, quota refusal, metadata
admission and mismatched descriptor binding. This is read/pread/readv/mmap API
observation plus source inspection, not universal kernel tracing.

## Limits carried forward

Only macOS ARM64 debug was independently executed. No release, Linux, full488-test
run, external Lua registration, public selector CLI, provenance/interpretation-v2,
whole-invocation budget sharing, load acceptance or final commit-seal wiring is
approved by this slice. Cooperative retained receipts cannot prevent a write
after a check or detect every change-and-revert. Full integrated X04-A review and
the mandatory external-profile gate remain open.
