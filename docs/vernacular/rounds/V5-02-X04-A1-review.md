# V5-02-X04-A1 independent review

Disposition: **approve** for the bounded native metadata-admission and bound-read
prerequisite. No blocking findings. This does not complete X04-A or the mandatory
external-profile gate.

Reviewed in the shared `dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, against the round brief,
implementation/roadmap/v5 direction, and exact X-CLI/X-CAP/X-LIMIT contract.
Reviewer ownership was limited to this report and
`tests/native/v5-02-x04-a1-review-evidence/`; no product/status source was edited.
No Planar tools, installed binary, operator data, commits, installation,
publication or merge were used. No subagents were dispatched.

## Exact candidate and protection

| Identity | SHA256 |
| --- | --- |
| Author candidate manifest | `0ac575d8d962a97733807a439d43cac866af5be1fb1c9d736a8fbf9e85d48094` |
| Author owned diff | `97362f82d1e2205cdd157ee7b29c909e923a07ff66bfff0d731e396f9d7c3b85` |
| Author artifact manifest | `30e81d989a21d124f030ed83954c6de663d89526a3566f16b0df22c5583a9b1d` |
| Author final archived pass manifest | `232c3fb8c04494f0af488643621d6170c817070f6da21e4627ab841ba96e4806` |

The independent integrity check matched all 11 candidate files to both the live
checkout and frozen candidate-source copies, all 115 author artifact entries,
and all 34 entries in the authoritative `passes/03-final-target-alias` archive.
Its candidate/source-freeze links match. Current product binary, admission test
executable, observer library and admission object match the author's frozen
build identities. Every record in the author's complete final filesystem
manifest still matched before independent verification, including the AGENTS.md
symlink. The baseline/final manifests and owned diff confine implementation
changes to the authorized files; native.t.cpp remains byte-identical.

## Boundary review

- `admission.cpp` retains explicit directory/file descriptors and original
  metadata, parent/name and ancestor chains. `contains` and `relationships`
  (lines 209–234) use device/inode ancestry, enforce both root-containment
  directions and reciprocal selected-file aliases, and preserve registry
  colocation. Observation/declaration separation and export target overlap are
  explicit roles. Supplied roots may overlap one another.
- Traversal performs no-follow metadata checks at each component, checks the
  opened identity against the named entry, and never enumerates siblings.
  Existing and absent targets preserve their original directory/name receipts.
  Regular target aliases are checked before the ordinary target-type refusal.
  Retained verification compares original descriptors and parent/name links;
  additions do not replace earlier receipts. Failed/moved states refuse, and
  self-move preserves ownership. No verification callback or Lua is present.
- File identity includes mode, size, mtime and ctime; directory identity includes
  mode without timestamps. This catches selected changes while allowing unrelated
  sibling edits. Host profile filenames allow spaces/Unicode; portable module
  grammar is correctly left to A2.
- `native.cpp:195` binds the already opened descriptor to the original admitted
  file before size-based allocation or content reading. The new overload always
  uses this path, including cached-file reuse, and compares fresh content/identity
  to any retained entry. The original overload and its original cached-byte
  behavior remain unchanged; the shared helper's default null admission adds no
  filesystem work to existing callers.
- Counts are checked before new slots/files: 65 trusted and 10000 total selected
  files, with trusted files included. The internal 100000 receipt/selection cap
  derives from the existing native enumeration/ancestor entry ceiling. Checked
  cumulative 64 MiB admission charging precedes owned key/component/receipt
  copies; the host-component regression specifically exercises this ordering.
  These are accounting ceilings, not RSS or descriptor-capacity guarantees.
- New overlap, changed-receipt, resource and protocol messages use the exact
  closed X-LIMIT text. Existing missing/no-follow/type failures retain native
  messages. No host paths or selected contents enter new diagnostics.

## Independent verification

Raw commands, timestamps, exits and output are under
`tests/native/v5-02-x04-a1-review-evidence/`.

1. Serialized debug `cmake --build build/rewrite-debug --target all planar_tests`
   passed and reported no work.
2. CTest JSON inventory contains 451 entries, preserves all 430 X03-B3 names and
   adds exactly the 21 admission cases.
3. `ctest --test-dir build/rewrite-debug -R
   'replacement.*(capture|path|acceptance)' --output-on-failure --output-junit …`
   independently passed **120/120**, with zero failures, errors or skips. This
   includes all new cases and existing capture/path/acceptance coverage.
4. Pinned formatting/Doxygen independently passed for 101 active sources. The
   final serialized `all planar_tests` build again reported no work.
5. Candidate source hashes were checked after each executed command and remained
   unchanged. Final protection/build checks and the reviewer artifact manifest
   bind the released evidence.

The author's final pass additionally retains passing canonical, record-v1 and
request-v2 vectors, four inherited runtime/CLI audits, exact forward/reverse
checks of five storage/export generated variants, and observer formatting.
Their source-bound final outputs and manifest hashes were checked, rather than
rerunning unrelated suites. The 451 inventory is not a full 451-test run claim.

## Read-observation and regression evidence

The test-only dynamic interposer observes explicit selected device/inode pairs
at read/pread/readv/mmap APIs. The executed read positive controls work for
profile, module, registry, observation and declaration files; metadata admission,
staged additions and repeated verification report zero observed attempts. Valid
bound capture produces attempts, and a replaced inode refuses without a read
attempt. The admission object's undefined-symbol audit contains no content-read
or directory-enumeration APIs. This establishes positive-controlled API
observation plus compiled/source inspection, not universal kernel tracing.

Both negative-control variants were independently recomputed from the exact
final native/admission sources and match their recorded original/variant hashes.
Each raw exact-one-test run fails: removing descriptor binding allows replacement
content reads; inserting a metadata read is detected. Exact restored source
hashes match the final candidate, whose final 120-case pass follows restoration.
The retained component-allocation and export-target-alias failing regressions
also match the final fixes and passing tests. Initial observer/build and fixture
failures remain disclosed. The two superseded successful-pass summaries have no
retained hash-bound raw outputs and were excluded as acceptance evidence.

## Limits carried forward

Only macOS ARM64 debug was independently executed. Linux O_PATH/syscall-forwarding
branches and release execution are unverified in A1. Cooperative receipts cannot
prevent hostile writes after a check or detect every change-and-revert.
Profile JSON/module closure, portable module policy, aggregate retained-content
budgets, external registration, CLI ordering, result authorization, provenance,
load acceptance and commit-seal wiring remain later work. A2 and subsequent
consumers must retain and recheck this original capability at their required
phases. No broader ordering or external-profile acceptance claim follows from
this native approval.
