# V5-02-X05-B2-B independent review

Disposition: **request-changes**. **B2B-R1** blocks the bounded codec foundation:
encode failure adapters allocate outside their protected boundary. The capacity
and phase-order review found no separate blocking issue. B2-C must wait for a
fresh corrective round and independent approval.

The review used shared `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The intentional dirty rewrite and
others' work were preserved. Candidate/current reference files were read-only;
writes are confined to this report, new
`tests/native/v5-02-x05-b2-b-review-evidence/`, and ordinary verification outputs
in the exclusively owned Debug build directory. No Planar tools, legacy
skills/roles/queue, installed binaries, operator data, commits, installation,
publication or merge were used. No full 612-case or release gate was run.
The B2-B round's recorded search disposition is the retained authorization;
there is no separate repository search report.

## B2B-R1 — encode diagnostic construction escapes allocation adaptation

At [the encode handlers](../../../src/engine/json_codec/codec.cpp#L357),
`catch (refusal)` calls `error(...)`, which allocates the diagnostic message.
If that allocation fails, the exception leaves the handler; sibling
`catch (bad_alloc)`, `catch (length_error)` and `catch (...)` clauses do not
protect handler bodies. The callback exception handlers at lines 359–364 have
the same problem. This violates the round's explicit requirement to protect
early error construction and callback adaptation, and the new interface's
typed-refusal claim.

The independent [actual-allocation probe](../../../tests/native/v5-02-x05-b2-b-review-evidence/adversarial.cpp#L111)
fails one `operator new` request and permits every later request. Both default
null input and null input with `{0,0,0}` limits escape as `std::bad_alloc`, for
ordinary and reserved entry points: four reproductions, each with exactly one
observed request and no charge-hook call. The zero-node case reaches the
resource refusal before the null-input check, so this covers both internal and
resource validation diagnostics.

Three additional reproductions supply preconstructed callback exceptions
(`bad_alloc`, `length_error`, unexpected `runtime_error`), rethrow them without
allocating in the callback, then fail the first actual diagnostic allocation.
Each calls the hook once, has zero prehook allocations and escapes
`std::bad_alloc`. The exception objects are controlled callback inputs; the
diagnostic allocation refusal is real. These are one bounded encode-adapter
defect, not three separate findings or a claim of persistent host exhaustion.

The [final red JUnit](../../../tests/native/v5-02-x05-b2-b-review-evidence/adversarial.xml)
records five cases, **two failed cases / seven failed assertions**, zero errors
and skips, exit **42**. The first discovery run is preserved separately in
`attempt01/` with its exact source, executable, object, commands and JUnit; it
reported one uncaught-allocation testcase error. The later explicit catches
make all seven escape controls observable without aborting their loops.

The original ordinary encode already constructed refusal diagnostics in its
catch handler: see [archived baseline lines 302–305](../../../tests/native/v5-02-x05-b2-b-evidence/baseline/codec.cpp#L302).
This is an inherited ordinary-codec gap exposed through the newly promised
reserved boundary, together with its new callback adapters. It is not claimed
as a new ordinary-encoding regression. Decode was not fault-tested or assigned
a separate finding.

Required correction: protect encode failure-to-diagnostic construction so a
one-shot allocation refusal in any of these adapters returns the typed resource
refusal. Preserve ordinary no-fault diagnostic precedence and supplied callback
diagnostics. Retain the real allocation regression, valid-copy failures and
ordinary controls. No persistent exhausted-heap guarantee or decode expansion
is required by this finding. Use the prescribed fresh search/coder/reviewer
correction process; do not amend the reviewed candidate in this round.

## Reservation and successful-path assessment

Ordinary encoding delegates to the same validator, graph builder and fixed-span
Glaze writer as reserved encoding. No separate JSON grammar/serializer, runtime
consumer, quota or global allocator policy was added. The native hook/context
does not allocate by itself; stable input across the synchronous call is an
explicit trusted C++ precondition, not protection against a violating caller.

The fixed active path stores only containers and checks depth before indexing.
Containers at depths 0–255 and a scalar leaf at 256 fit; a 257th container
refuses. Path membership detects cycles while repeated aliases are counted and
copied repeatedly. Integer sizing uses bounded stack `to_chars`. Existing byte,
node, int32, UTF-8 and error-order policies remain shared. Checked arithmetic
derives reservation before graph/output allocation; supported hard caps make
size overflow unreachable on this 64-bit target without invented larger-limit
knobs.

The graph allowance counts the root, array nodes and object key/node pair slots;
embedded string objects are not charged twice as separate heap objects.
Payload/terminator/capacity slack follows the pinned libc++ alignment helper.
Each vector reserves its exact count before filling, sorting moves existing
objects, and Glaze writes the fixed buffer. The output string moves into the
result with its capacity retained and is charged once. Workspace is explicitly
conservative logical capacity, with overlapping moved slots; it is not a claim
of independent allocations, full native stack size or RSS.

The unchanged author phase observer passes **2 cases / 16 assertions** against
production codec libraries. The independent successful controls pass **513**
string-size fixtures across inline/alignment boundaries, repeated objects and
control-character expansion, with zero prehook allocations. Their actual
aggregate requests fit graph + string + output storage even after excluding
the conservative workspace allowance. All **eight** observed copy/string/output
allocation ordinals independently return typed resource failures when failed
one at a time; ordinary retries pass. Long preowned callback diagnostics move
without allocation, preserve all fields, and null hooks retain ordinary behavior.
These finite probes supplement the source derivation; they do not establish a
universal allocator/RSS guarantee.

## Verification and exact evidence

[Recorded commands](../../../tests/native/v5-02-x05-b2-b-review-evidence/commands.json)
configure Debug, build `all planar_tests`, and pass **346/346** focused cases in
**184.104 seconds**, followed by the separate **11/11** ledger-history-codec
selection. Both have zero failures, errors, skips or disabled cases. The
612-name inventory preserves all 606 B2-A and 582 A4 names; inventory is not a
full run. Four canonical/source vectors, six exact-source audits, pinned
formatting/Doxygen over **118** active C++ sources and the no-work build pass.
The passing standard gate does not supersede R1's actual allocation failures.
Probe compile/link/run argv and statuses are in `probe-commands.json`; final
probe source links production codec/value/Catch libraries without runtime variants.

[Scope comparison](../../../tests/native/v5-02-x05-b2-b-review-evidence/scope-diff.json)
reconstructs the exact owned diff and every candidate-source copy. Against the
previous approved 381-member scoped freeze, the current 384-member freeze has
only nine authorized existing changes (three codec files and six references)
and three new support files, with no removals. All **12** candidate identities
and **90** author artifact identities match before and after the gate. AGENTS
remains a symlink to CLAUDE. This is scoped verification, not a whole-tree claim.
The stopped superseded gate remains preserved as exit -2 before its correction;
the initial test-only compile failure is also preserved. Neither is represented
as a successful gate or hidden by the later passing standard results.

| Reviewed author identity | SHA256 |
| --- | --- |
| Candidate manifest | `48e1fceb881ce55fcdd655b0f64a5d2b42b01dc419c21d0413f1e19dc081c91e` |
| Owned diff | `9a7c5e46f37423cdc3972cb4e12a7c7ac1bd5e4b07f3039efae434d3064c6525` |
| Artifact manifest | `2f3991abd0d0ab2ee852c35fce355ddd1a7592ee20b8b316807a402389ceb394` |
| Author report | `3168f0afc4c952885a88a35fefbf1fc55feef9f0c2b605d9fc48061e822fb7fb` |

Finalization binds this report, reviewer artifacts, candidate and final scoped
source identities. Debug ownership is released after final hashing; no reviewer
edits follow release. B2-B remains unapproved. B2-C retained F dispatch/raw
conversion/immutable input, B3 identity/W, B4 render, X06–X08, execution/RA01 and
Linux remain open. Codec 16 MiB/+1 transport checks do not prove shape semantic
acceptance or whole-invocation accounting. Public init creates format1; the
latest full gate remains A4's 582 per configuration.
