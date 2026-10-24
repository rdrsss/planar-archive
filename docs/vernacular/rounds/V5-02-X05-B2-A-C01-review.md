# V5-02-X05-B2-A+C01 independent review

Disposition: **approve**. The combined native shape-definition foundation and
C01 correction satisfy the bounded authorization. **B2A-R1 is closed**. No new
blocking finding remains; this approval permits the coordinator to advance to
B2-B/C after checking that the reviewed candidate is still current.

Review used the shared `dev/planar-v2-vernacular` checkout at
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The intentional dirty rewrite tree,
prior failed candidate and all prior evidence were preserved. Candidate and
current reference files remained read-only. Reviewer writes are confined to this
report and `tests/native/v5-02-x05-b2-a-c01-review-evidence/`, apart from ordinary
verification outputs in the exclusively owned Debug build directory. No Planar
tools, legacy skills/roles/queue, installed binaries, operator data, commits,
installation, publication or merge were used. No full 606-case or release gate
was run.

## R1 closure and error semantics

The [complete lookup](../../../src/engine/runtime/runtime.cpp#L2317) now places
owner guards and already-exhausted diagnostic construction inside its typed
boundary. Constructing an embedded/moved-from protocol refusal or copying the
sticky resource diagnostic therefore reaches the allocation adapter when one
actual allocation fails. The adapter handles a null moved-from owner directly;
for a live owner it installs an external resource diagnostic only when no earlier
conversion failure exists. It preserves the first diagnostic's message, limit
and maximum instead of overwriting it with the generic external resource text.
The logical field remains `catalog.shapes`.

Ordinary unknown/nonshape/embedded/moved-from protocol refusals keep the existing
redacted internal vocabulary. Ordinary unknown-ID refusal leaves a valid profile
owner usable. A resource refusal leaves a profile owner terminal. Embedded and
moved-from retries remain protocol-first; the moved-from object has no native
owner to poison, and its moved-to owner remains usable. These distinctions match
the corrected report and reference prose. The correction changes neither public
admission nor the identity/hash/reservation policy.

The unchanged original review allocator cases are independently recompiled and
linked against production runtime, with no source variant, runtime injection hook
or private state manipulation. All **three cases / 23 assertions** now pass.
They cover embedded, moved-from and already-sticky resource adaptation. The
preserved fresh pre-fix run remains three failed cases, six failed assertions and
exit **42**; the earlier independent R1 evidence also remains intact.

The unchanged original adversarial suite passes **three cases / 377 assertions**,
including every observed allocation ordinal along a successful lookup, typed
resource refusal and terminal retry, independent exact-byte identity, same-ID
shape/projection namespace separation and redacted unknown IDs. C01's boundary
suite independently passes **three cases / 56 assertions**: ordinary protocol
recoverability, early resource categories/fields, moved-to independence and a
real cumulative `native_bytes` refusal followed by a second actual allocation
refusal while copying that diagnostic. Both the second refusal and the subsequent
ordinary retry preserve the exact first message, limit, maximum and field.

These runners fail one real native allocation and then permit subsequent
allocations; assertions run after observation ends. They do not establish a
persistent globally exhausted-heap guarantee or natural host exhaustion. Typed
string diagnostics can still allocate under that different condition.

## Combined identity and cumulative reservation review

The [owned metadata API](../../../src/engine/runtime/runtime.cppm#L127) returns
ID, final revision and digest without exporting retained descriptor/source aliases.
Lookup selects only registered external shape entries, uses their native ordinal
to locate privately copied original source, and hashes exactly final D,
`{tier:"external",module:"external/"+path}` and lowercase original source HEX in
`shape_definition_v1`. Ordinal itself, unrelated modules, profile bytes/modes and
host selectors do not enter this value. Same-module projection/comment changes
conservatively change the selected identity; separate unselected modules do not.
The six production cases cover final D mutation, source/path/revision changes,
ordinal independence, actual captured host selectors and caller alias mutation.

I independently compared the complete combined addition against the pre-B2-A
runtime/interface/tests and checked its reservation against the actual
[hash envelope](../../../src/engine/hash/hash.cpp#L17) and
[canonical encoder](../../../src/engine/canonical/canonical.cpp#L11).
The validated flat five-string D and admitted module/path bounds keep all size
arithmetic bounded. Nine new graph nodes, six borrowed D nodes and ten encoded
keys receive 25 existing 512-byte node charges; five new map entries receive
separate slots. Staging/graph HEX and module paths, new envelope/key/tier text,
one canonical output, digest and returned metadata copies are reserved before
their allocations. The hash helper builds one four-item domain frame and encodes
once; XXH3 reads those bytes. These are logical charged capacities under the
existing metric, not process RSS or a new quota. Repeated calls recompute and
consume cumulative native work; actual repeated-work exhaustion remains terminal.

The method has no F access, Lua call, provider/filesystem operation, reload,
invocation transition or budget reset. Generic profile invocation remains closed
before provider effects. Nothing here establishes future F dispatch, immutable
call input or complete shape-return transport.

## Independent verification and immutable evidence

[Exact commands](../../../tests/native/v5-02-x05-b2-a-c01-review-evidence/commands.json)
configure Debug, build `all planar_tests`, and pass **252/252** focused cases in
**133.34 seconds**: 245 unit/native and seven acceptance cases, zero failures,
errors, skips or disabled cases. All six production definition cases pass. The
606-name inventory preserves all 600 B1 and 582 A4 names; inventory is not a full
606-case run. [Raw audit](../../../tests/native/v5-02-x05-b2-a-c01-review-evidence/raw-evidence-audit.json)
checks both author and reviewer JUnit, unique selected names, console starts and
immediately saved LastTest records, plus red/green allocator counts.

The independent Python canonical/upstream XXH3 vector and all four inherited
vectors pass. The known answer remains 733 bytes and
`xxh3_128:b6d3718ddeb0cd1bc49b9e866a638b99`. All five inherited exact-source
audits and the B2-A lookup audit pass. Pinned formatting/Doxygen checks **118
active C++ sources**. The final `all planar_tests` build reports
`ninja: no work to do.`. Reconfiguration regenerated/rebuilt verification
variants; the product binary and runtime library match the author's recorded
hashes, and this review separately records the rebuilt runtime test binary's
identity. Source audits supplement behavioral tests and do not prove callability.

[Scope audit](../../../tests/native/v5-02-x05-b2-a-c01-review-evidence/scope-before.json)
verifies all ten C01 candidate files, exact owned-diff reconstruction, all 104
wrapped C01 artifacts, all 76 prior B2-A author artifacts and all 57 prior B2-A
review artifacts. All **381** scoped source/config/fixture/support/reference
identities match the author's frozen candidate before and after verification.
The prior 378-member inventory is retained; its seven changed existing files and
three new support files match C01 ownership, and every archived original matches
the prior snapshot. This is scoped membership/byte verification, not a whole-tree
freeze. AGENTS.md remains a symlink to CLAUDE.md.

| Reviewed author identity | SHA256 |
| --- | --- |
| Candidate manifest | `aacb5f86af9d6742d7c87fd796799b23ed879e0a58866ed8072fb6990d9eb6d6` |
| Owned diff | `e9a54290d506cc3b0ebab8696c0e0fd7cbcdb3856c4a76dac51674fb9605cdee` |
| Artifact manifest | `9a3924201089542da140f5d522ec976aa88de46f473ede6c1dcfd03dada52a57` |
| Author report | `3988be82461351b803857b6038db5324c1ef5f0ef7752c7adda809f0dcbc56a2` |

Final candidate/source identities, this report and all reviewer artifacts are
bound by [finalization](../../../tests/native/v5-02-x05-b2-a-c01-review-evidence/finalization.json).
Debug build ownership is released after final hashing; no reviewer edits follow
release. No extra adversarial test was needed beyond the nine independently
rerun native cases and existing focused production coverage.

B2-B codec accounting, B2-C retained F dispatch/immutable input/16 MiB complete
return transport, B3 selected identity/W, B4 rendering, X06 history/interpretation/
acceptance and orchestration, X07 public cutover, X08 mandatory external-profile
acceptance, execution/RA01 and Linux remain open. Public init creates format1.
This approval closes the bounded combined B2-A+C01 foundation, not B2 or the B
milestone. The latest full gate remains A4's 582 per configuration.
