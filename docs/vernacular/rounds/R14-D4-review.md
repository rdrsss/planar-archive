# R14-D4 independent review

Disposition: **approve**. No blocking findings or requested changes in the frozen
D4 candidate. Approval covers the owned native diagnostic amendment and its
tests/documentation; it does not claim load service/CLI admission or full R14.

Fresh ordinary reviewer, Sol-high, in the shared checkout on
`dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The reviewer owned only this report
and `tests/native/r14-d4-review-evidence/`. Other work was preserved. No product
fix, author-evidence edit, Planar tool/skill/queue, installed binary, operator
database, commit, installation, publication or merge was used.

## Exact candidate

The independently recomputed identities match the handoff:

| Frozen item | SHA256 |
| --- | --- |
| Wrapped `candidate.json`, 17 repository-relative paths | `d5b39b5f92411b6bce0cd26ec26b67974c4eea10c2663066a5f19242b0946e4f` |
| Baseline-relative `owned.diff` | `08f9790ab662b25d608b307136476bebfb96d5567aacc1762c4c9df1f609a0be` |
| Wrapped `artifacts.json`, 69 evidence-relative artifacts | `7c4faf40d4b657aee6feeb9c750c3106622585c2752086cd0ff315167f2facd7` |

The independent [audit](../../../tests/native/r14-d4-review-evidence/audit.json)
checks all 17 current files and candidate/baseline snapshots, regenerates the
exact diff in memory, verifies every author artifact and the 13,867-entry
protection inventory with approved owned changes excluded, and finds no new
out-of-scope files.
The AGENTS symlink, v5 SHA256 and exact approved load-contract body remain intact.
The two extra support assertion files are expressly authorized in the
[round brief](R14-D4.md#scoped-verification-extension); generators are unchanged.
`format.t.cpp` remains byte-identical. Repeating the audit after reviewer
verification confirms the candidate is still current.

## Contract and implementation assessment

The implementation satisfies the approved
[typed authoritative diagnostic contract](../load-command-contract.md#typed-authoritative-diagnostics):

- [types.cppm](../../../src/engine/ledger/types.cppm:27) adds an owned optional
  variant of two finite native structs. Heads are optional signed-64 integers;
  collision identity is a stable record kind and optional fixed-size digest.
  No string, Lua value, request body, arbitrary map or borrowed reference enters
  the detail. The pre-existing history-record reexport supplies these types;
  there is no new module edge.
- [complete_acceptance](../../../src/engine/ledger/format.cpp:1437) retains the
  existing native write transaction and request relookup precedence. Its sole
  locked head comparison now copies requested and actual heads into the refusal
  at [line 1470](../../../src/engine/ledger/format.cpp:1470). There is no diagnostic
  head reread or message parsing. Existing equal replay and unequal saved-intent
  paths precede CAS, and request conflict retains no detail.
- [intern_record](../../../src/engine/ledger/format.cpp:1317) preserves exact
  encoded-record comparison and storage ordering. The unequal bucket refusal
  adds kind and candidate digest at [line 1347](../../../src/engine/ledger/format.cpp:1347),
  with a disengaged digest for request records. The detail itself never retains
  a request digest. Existing writer reset, transaction RAII and
  [tree error ownership](../../../src/engine/history/tree_core.cpp:26) copy/move
  the whole error; the native callback return at
  [line 1432](../../../src/engine/ledger/format.cpp:1432) preserves it after rollback.
- [ledger_error](../../../src/cmd/planar/response.cpp:36) checks category/variant,
  positive heads, all four kind names and exact kind/digest pairing before
  building detail values. Invalid combinations return the fixed redacted
  internal protocol error. The existing Glaze-backed
  [response encoder](../../../src/cmd/planar/response.cpp:162) supplies canonical
  JSON, one LF, empty stdout and status 3/4. Native message bytes are ignored.
  Public digests are 32 lowercase hex digits, heads are canonical decimal or
  `none`, and request collision output has only `kind:"request"`.

No storage/schema/codec/tree algorithm, runtime, Lua, parser, public catalog or
command admission changed. Existing service/runtime composition is preserved;
new load-specific service propagation remains D6/D7 work. Status documentation
accurately retains that boundary and the mandatory external-profile gate.

## Test and raw evidence assessment

The author red log and XML show the added old-API response case failing twice
because `details` is missing, before the type/population implementation. The
wrapper did not separately retain CTest's exit code; the raw failed assertions,
one failed test and zero-percent summary are explicit, so this does not obscure
the regression demonstration. The intermediate immutable-value compilation
failure is retained and corrected in the final candidate.

The [locked-head regression](../../../src/engine/ledger/acceptance.t.cpp:445)
obtains actual errors from complete acceptance on both empty and nonempty
ledgers, copies them after rollback, advances the head and checks the original
heads remain. It also verifies unchanged bytes on refusal and saved-request
conflict precedence. Existing replay/precedence tests remain unchanged.

The [writer collision case](../../../tests/native/r13b1-support/collision.t.cpp:46)
tests all four kinds, including an inserted prefix, failed subsequent writer
operations, exact bytes and reopened-state checks. Its error copy outlives the
writer. The [atomic collision case](../../../tests/native/r14c3b2-support/atomic.t.cpp:121)
selectively forces each kind through complete acceptance, including file and
directory tree callbacks, then inspects the error after transaction unwinding.
All existing unchanged-database/head/request assertions remain. Request digest
absence is checked at both native boundaries.

The [response cases](../../../src/cmd/planar/bootstrap.t.cpp:373) pin full canonical
envelopes and status/output for valid heads, maximum signed-64, public kinds and
request redaction. Forty malformed payload combinations must yield the exact
internal envelope without detail. The
[generic mapper case](../../../src/cmd/planar/bootstrap.t.cpp:82) pins all 17
pre-existing no-detail native mappings byte-for-byte, including existing I/O
operation detail. No native-message/private-key/body/path/digest fallback exists.

I inspected the retained final JUnit/LastTest and commands, lint, vectors,
variant audit, protection and no-work evidence. The final author run passed
153/153 (147 unit cases and all six current CLI suites), zero failures/skips,
after final formatting and rebuild. Inventory is 373 and retains all 370 prior
names. Pinned formatting/Doxygen passed 89 active sources; all nine owned C++
paths separately passed formatting. Unchanged vectors report 12/20/41. The exact
seven generated variant families and fixed runtime implementation/interface
harness were audited against final bytes. These checks are reviewed author
evidence, not additional reviewer executions.

## Independent execution and limits

Reviewer build/test commands and exits are in
[commands.json](../../../tests/native/r14-d4-review-evidence/commands.json):

1. `python3 tests/native/r14-d4-review-evidence/audit.py` — passed before and
   after verification, preserving all frozen identities.
2. `cmake --build build/rewrite-debug --target all planar_tests` — exit 0;
   `ninja: no work to do.`
3. Direct CTest selection recorded in `commands.json` — exit 0, six matched/passed
   cases, 634 assertions, zero failures/skips. Selection covers locked heads,
   both four-kind collision paths, valid/malformed response details and all
   generic mappings. Reviewer [JUnit](../../../tests/native/r14-d4-review-evidence/focused.xml)
   and [LastTest](../../../tests/native/r14-d4-review-evidence/LastTest.log) are retained.

Debug ownership is released and idle after these serialized commands. Release
was untouched and idle. No full debug/release milestone or Linux run was
performed or inferred; the approved prior 370/370 debug/release checkpoint is
baseline evidence. No hardware power-loss, actual allocator exhaustion, public
load or complete R14 acceptance claim follows from this review. D7/D8 retain
their integrated gate obligations.
