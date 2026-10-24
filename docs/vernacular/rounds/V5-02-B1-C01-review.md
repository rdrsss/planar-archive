# V5-02-B1-C01 independent review

Disposition: **approve**. Both technical findings from the completed
V5-02-B1-R02 review are corrected. No blocking finding remains in this scope.

Reviewed the frozen six-file candidate in the shared checkout on
`dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Ownership was limited to this report
and `tests/native/v5-02-b1-c01-review-evidence/`; no product fixes or reversions
were made. This was an ordinary source/evidence review and rerun of existing
focused tests. No new test program, stress experiment or prior deep probe was
created or executed. No Planar tools/skills/queue, installed binary, operator
database, network, commit, installation or publication was used.

## Exact reviewed candidate

- Candidate manifest SHA256:
  `33f4bf46a3a9722c7bacff3d30a34e01d09a48d2375a0e8b0c6fec734742a89d`.
- Owned diff SHA256:
  `e4e7374782dfdbe9b3463bb33d8e4774190576f1e364de73a5e36f31eace91ad`.
- Coder artifact manifest SHA256:
  `48df735af6b78ef66c641cae64aaa1bd7704318375c14d44bb1011d6328597b8`.

Independent byte checks matched every current candidate file to its snapshot and
all 53 frozen artifacts to their manifest. They also verified 13,487 protected
files unchanged and the `AGENTS.md -> CLAUDE.md` symlink. The sole additional
baseline difference is the coordinator-owned C01 round record's frozen-candidate
handoff, explicitly confirmed by the coordinator during review. Its exact input
hash is recorded separately; it is not a product-candidate change. Historical
fixtures, prior review/probe evidence, shipped Lua and render goldens remain
protected.

## Correction assessment

**Closure and table graph.** `runtime.cpp:1031` supplies registration/global roots
and every retained callable reference to one identity memo. The iterative walk at
line 933 memoizes both tables and Lua functions before descent. It follows all
Lua closure upvalues (line 980), table keys (988) and values (994), including
helper-only captured tables and shared/cyclic graphs. Opaque C closure
implementation state is deliberately excluded. Emptying each original table and
installing private backing access preserves object identity while routing both
existing-key replacement and new-key writes to refusal. Captured aliases reach
that same original object; a newly allocated invocation scratch table has no
backing and remains mutable.

**Traversal bounds and ownership.** The walk uses a fixed 256-element array of
trivial three-integer frames, without native recursion. It refuses a new object
at depth 257 through the retained typed conversion diagnostic. `freeze_stack`
(line 907) checks the actual live stack against 2048 and reserves twelve temporary
slots before bounded pushes; at most four retained slots per active table are
needed. Each edge, including aliases and scalar leaves, and each entry cleared
from an original table consumes the cumulative visit/node/native-byte budget.
Private memo, backing and metatable allocations use the existing 64 MiB
registration allocator. These checks operate directly in native traversal and do
not depend on Lua hooks firing. `freeze_visit` ends its C++ exception/catch scope
before raising a Lua error; no nontrivial native owner survives the new allocating
protected frames. Registration failure destroys the owning VM, including any
partly sealed graph.

**Reads and conversion.** Backing-aware rawget/next/pairs wrappers are installed
before bundle evaluation (`runtime.cpp:1078`), so captured aliases work across
sealing. Indexed reads, length and ipairs use the backing through the installed
metamethods. Conversion at line 229 reads the private backing but keeps tags and
cycle identity on the original table; unrelated metatables still refuse.
Existing scalar/type/shape checks remain in force. The new deterministic tests
exercise direct/new/nested/key/helper-only mutation refusal, shared/cyclic and
callable-key reads, saved iterator/read aliases, scratch mutation, frozen tagged
array codec conversion and a returned frozen error table.

**Preserved surrounding behavior.** Native service error capture and precedence,
exact retained registry references, move/one-shot ownership and terminal release
are unchanged. Lua closes before native command state is cleared. Native
conversion charges continue from registration into invocation; only the
instruction budget restarts. Export publication is still recorded before later
validation and remains observable after terminal cleanup. Existing focused tests
cover native failure suppression/retry, caught exhaustion, cumulative transfers,
same-bundle dispatch, real retained render equality and export uncertainty. No
CLI adapter, write-lock callback or service implementation was added here.

## Regression and verification evidence

The red runtime exactly matches the baseline runtime. Its separately frozen
JUnit contains three failures: the direct captured write succeeded, the read
compatibility invocation failed, and the over-bound graph was accepted. This
demonstrates the pre-fix failures without rerunning historical experiments.
Final helper-only and returned-table strengthening is visible in the candidate;
the original red evidence remains intact. Red-manifest SHA256:
`03a2bbfbc0bff721173d230ef618ec7b16755200e6986e082b66b272a0ad8aed`.

Variant 0's generation differs from its prior form only by removing `rawget or`
from the historical fixture's capability-absence assertion. The historical
`altered.lua` itself is unchanged, and rawset/getmetatable/registration absence
plus all write-denial assertions remain. Positive frozen and scratch rawget
checks accompany that change. Independent audits confirm all 21 generated native
implementations/interfaces and embedded sources, plus the fixed-export native
variant, match their intended exact source transformations.

The following commands executed directly and serially with exclusive debug
ownership; raw results are retained in the new review-evidence directory:

- `cmake --build build/rewrite-debug --target all planar_tests`: passed, no work.
- `ctest --test-dir build/rewrite-debug -R 'replacement.(command|runtime|loader|provenance|interpretation|acceptance prepared|cli$|init_cli$|history_cli$|export_cli$)' --output-on-failure --output-junit <repo>/tests/native/v5-02-b1-c01-review-evidence/verified.xml`:
  **87 passed, zero failures/skips**, 53.34 seconds. This exactly preserves the
  coder's focused test-name set, including the previous 83 and four new cases.
  LastTest was archived before querying inventory.
- `ctest --test-dir build/rewrite-debug --show-only=json-v1`: 358 names,
  preserving all 354 earlier names; this is inventory, not a full test run.
- `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`:
  pinned formatting and Doxygen passed over 87 active source files.
- `python3 tests/vectors/generate.py --check`: 12 byte, 20 hash and 41 domain
  vectors passed.
- Repeated build of `all planar_tests`: `ninja: no work to do.`
- `python3 tests/native/v5-02-b1-c01-review-evidence/audit.py`: candidate,
  protected input, frozen artifact, variant and test-count checks passed.

Retained coder final JUnit SHA256:
`b014e3edd9b6c09c33fb37248bbf6fcbaf88615c25fba89a4690bfb814e7c003`.
Independent reviewer JUnit SHA256:
`10cf0b2682383149e3d302a1258fdd2ce6dab152ca37c132cd7d510d22b99b5c`.
`input-sha256.json` records review inputs; `output-sha256.json` hashes this report
and the independent raw verification/audit files.

This approval covers the frozen B1 correction on macOS debug. It does not claim a
full 358-test execution, release/Linux coverage, B2 CLI composition, workspace
naming, full R14/load or external-profile support. B2 owns the integrated
debug/release gate. **Debug build-directory ownership is explicitly released.**
