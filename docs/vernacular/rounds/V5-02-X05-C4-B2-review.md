# V5-02-X05-C4-B2 independent native history-binding review

Disposition: **approve**. No required changes were found in the exact bounded
candidate. This approves original receipt/descriptor binding and synchronous
native history preparation. It does not expose a Lua read service or complete
X05-C, X06, X07 or X08.

The review used the shared `dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Ownership was limited to this report
and `tests/native/v5-02-x05-c4-b2-review-evidence/`, with exclusive serialized
Debug. Product, configuration, author artifacts and historical evidence were
read-only. No Planar tools, skills, roles, queues, delegation, commits,
installation, publication, merge, installed binaries or operator data were used.

## Identity and authority

The review considered AGENTS.md's CLAUDE.md target, implementation/roadmap and
v5 baseline, the exact external-profile contract and implementation path,
approved C4-B1 review, and the complete C4-B2 authorization, native API decision,
review obligations and same-round verification amendments. The bounded native
API decision controls over historical future-service wording.

| Reviewed author identity | SHA256 |
| --- | --- |
| `frozen-candidate.json` | `b2569f339ad872b5a8dc9ac92aa881b976c75e0fee30e09878676b3361ffc2ba` |
| `artifact-manifest.json` | `93ead0bc5ab4645b8c6594c7947092389858fb719a8a92777e8f8d82ed331c19` |
| `REPORT.md` | `61ea33a4366733eb5f332003c24d88d75b22f8d5b39ff584de6942f3accc99b3` |

The path-keyed candidate has 23 entries. The schema1 artifact manifest has 221
entries rooted at the repository, including evidence directories; it excludes
itself and the finalization record. All entries, exact artifact membership,
modes, hashes and symlink targets were independently checked. Support files are
bound by the candidate. The finalization record agrees with those identities.
All nine product/test/CMake entries in the tested identity equal the final frozen
candidate. The reviewer independently reran the complete relevant 412-case
selection on these final sources rather than inheriting the author's earlier
412-case result across its correction.

The protection snapshot contains 72,661 entries, including product/reference
files, vendor, author/support artifacts, historical evidence and install-release.
Only the coordinator-owned round and reviewer-owned paths are excluded. Final
comparison finds no changes. AGENTS.md remains the CLAUDE.md symlink and
rewrite-release remains absent.

## Source assessment

`format.cpp:1210` opens the actual no-follow regular descriptor and invokes
`guard->bind(held.fd)` before `header(held.fd)`. Binding refusal, invalid status
or exception returns before raw header content access. The separate gated opener
always selects read-only access. The same shared guard object enters
`connection::open_gated` before SQLite opens, so the approved VFS guards its
open-time header read as well as later xRead/xFetch. Ungated opening and its
writable transition retain their original behavior.

`format.cpp:1182` checks the retained SQLite status before original verification;
format reads check both boundaries even for cached success. A first refusal is
stored without allocation and makes later reads unavailable. Gate status remains
inspectable without allocation. Cached metadata intentionally remains readable
and is documented as insufficient authorization. The generic format module has
no upward capture/runtime dependency.

`runtime.cpp:4304` binds the original `extension.ledger` receipt against that
actual descriptor, duplicates it with F_DUPFD_CLOEXEC without reopening, and
rechecks the same original receipt and complete original capture on every gate.
The adapter owns its duplicate and borrows capture only inside synchronous
preparation. Format retains the original descriptor through snapshot/connection
lifetime; SQLite retains shared guard ownership through its last state owner.
There is no borrowed capture escape into the retained history.

`runtime.cpp:4372` takes only the private admitted history selector and the
matching private path established from the original moved capture. No caller
query, path, replacement capture or mutable service result enters the helper.
The history reader is the unchanged native recorded-history reader, with no
current or recorded semantic Lua. Its typed misses, storage categories and
unsupported interpretation behavior remain intact; profile interpretation is
still unsupported pending X06.

The inner read scope destroys format, transaction and connection before the
post-read original receipt seal on success, typed error and caught exceptions.
Final duplicate verification and destruction occur before helper return. No Lua
call occurs in this helper. Real writer commits after return, observed SQLite
open/close counts and an independent descriptor census support the lifetime
assessment. Authenticated history is privately retained; the test accessor exists
only in the copied interface/implementation. Read-capability execute still refuses.

First registration errors move into retained native history error ownership
before fallible copying. `terminal` never replaces that owned error. Adapter-owned
original profile failures outrank SQL/query failures and allocation adaptation;
existing admit/verify/execute error paths borrow the same original registration
error. Native format/history errors stay retained in the history variant across
retry/move. All failures poison admission/execution, and once-only preparation,
unadmitted/repeated/moved calls, empty failed catalog and move ownership are
explicitly guarded. The guarantee covers controlled one-shot allocation refusal,
not persistent exhausted-host-memory recovery.

The runtime's new dependency descends through history-read to format/codecs;
there is no cycle. Production, runtime-test, shape, command/load/closure and CLI
variant declarations propagate the dependency. Outside engine wiring, only the
two authorized CLI DEPENDS clauses change. CMakeLists.txt's candidate entry is
unchanged from its preimage. There is no public CLI or shipped Lua change.

## Independent verification

Exact argv, cwd, timing, raw logs, JUnit, source copies and dependency inventories
are retained in the reviewer evidence directory. Python used `python3 -B`.

| Check | Result |
| --- | --- |
| Final-source focused production gate | 412/412; zero failures/errors/skips/disabled; 170.19 seconds |
| Runtime; format/history/acceptance; SQLite/reliability; history-read; CLI | 287; 82; 33; 3; 7 registered cases |
| Reconstructed author private probes | 7/7 green |
| Additional reviewer adversarial probes | 3/3 green |
| Deliberately removed pre-header binding control | One expected failing case, exit42 |
| Six unchanged contract vectors | Pass |
| Historical and current exact source/generated-variant audits | Pass |
| Pinned formatting/Doxygen | 118 active sources pass |
| Focused repeat build | Ninja reports no work |
| Inventory | 773 names unchanged; all original 767 preserved |
| Shared/copied compiled inputs | 888 members; exact membership/modes/hashes unchanged |
| Final author/protection identity | Unchanged |

The reconstructed probes use real captures, original admission, actual raw
header pread and SQLite delegate counters. Refusal during original binding has
zero ledger header/delegate reads; first SQLite header refusal permits the earlier
raw read but zero delegates. Positive controls read real content. Cached success
is discarded after a post-query gate failure and subsequent calls remain sticky.
After-read profile/ledger mutations outrank healthy results and typed misses.
Both embedded formats preserve exact state/page selection, ledger identity and
last_event. Actual allocation refusals record a nonzero failed request, preserve
original/resource diagnostics and close real storage resources.

The reviewer adds three independent cases: original capture_changed survives
actual allocation refusal through prepare_history/verify/admit/execute and a
subsequent move; a throwing generic verifier refuses a cached read without new
content delegates and remains sticky; and a descriptor census returns to its
pre-call count after success, typed missing-revision error and actual native
allocation refusal. This last case covers the duplicate adapter descriptor in
addition to SQLite's observed opens/closes. The first private attempt passed its
nine green cases; its sources/logs/XML are preserved before adding the census.
The final private run passes ten green cases in total. The red control is a
sensitivity test, not a historical product regression claim.

Private builds recursively expand response files, copy every object/PCM/archive/
dylib input, redirect generated outputs and disable implicit module/module-map
compilation. Complete shared/copied inventories are compared before removal.
The copied executable loads macOS system libc++/libSystem; these remain platform
prerequisites. Only reviewer-owned disposable copies and generated binaries are
removed after hash proof; raw sources, commands, logs, XML and exact removal
identities remain. No shared or author input is removed.

The author's actual first-error red regression and its first snapshot/raw
attempts remain intact. Its 412-case pre-correction and 299-case final gates are
kept distinct. The historical B2-A broad string-split failure reproduces on the
unchanged pre-edit span; the new exact-method audit preserves those original
invariants without editing historical evidence. Neither issue is concealed as a
successful historical gate.

## Limits and handoff

This is focused macOS Debug evidence. It claims no new full-suite, release or
Linux execution. The latest complete approved gate remains 676 per configuration.
Public init creates format1. Lua capability/service order and response equality,
role/projection services, X06 recorded profile interpretation/acceptance and
shared budgets, X07 public cutover, X08 mandatory external-profile acceptance,
execution/RA01 and Linux remain open. Cooperative filesystem stability limits,
including change-and-revert and writes after the final seal, remain unchanged.

All reviewer verification has completed. Exclusive Debug and reviewer ownership
are released to the coordinator after finalization; no subsequent edits/builds
will follow this handoff. The finalization record binds this report and evidence.
