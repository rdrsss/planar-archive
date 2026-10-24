# V5-02-X05-B2-A independent review

Disposition: **request-changes**. One blocking finding, **B2A-R1**, concerns
allocation failure in the lookup's early refusal adapters. The candidate remains
unchanged. The passing focused gate does not close this finding or authorize
advancement to B2-B/C.

Review used the shared `dev/planar-v2-vernacular` checkout at
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, preserving the intentional dirty
rewrite tree and others' work. Writes are confined to this report and
`tests/native/v5-02-x05-b2-a-review-evidence/`. Candidate/current reference files,
round authorizations and historical evidence were read-only. No Planar tools,
legacy skills/roles/queue, installed binaries, operator data, commits,
installation, publication or merge were used. Review held sole serialized Debug
build ownership. No full 606-case or release suite was run.

## B2A-R1 — Early refusal adapters escape the typed resource boundary

At [runtime.cpp:2319](../../../src/engine/runtime/runtime.cpp#L2319), embedded,
moved-from, failed-sealing, registering and invoked owners return
`failure(external_protocol())` before the method's `try` begins. At
[runtime.cpp:2324](../../../src/engine/runtime/runtime.cpp#L2324), an exhausted
owner similarly returns `failure(command_resource(s))` before that boundary.
Constructing the protocol diagnostic's message or copying the saved resource
diagnostic can allocate. A `std::bad_alloc` there escapes
`resolve_shape_definition` instead of returning its declared
`std::expected<shape_definition, registration_error>` resource refusal.

Three independent [native cases](../../../tests/native/v5-02-x05-b2-a-review-evidence/adapter.cpp)
link directly against the unchanged production runtime. A replacement global
`operator new` fails exactly one real native allocation after the caller enables
it immediately before lookup; it automatically permits subsequent allocations.
Observation is disabled before Catch assertions. The cases demonstrate:

- An embedded VM queried for `pipeline` throws `std::bad_alloc` while constructing
  the early protocol refusal.
- A moved-from profile VM queried for `custom` throws in the same early branch.
- A valid profile VM first receives an ordinary typed resource refusal from a
  one-shot allocation failure inside the lookup. A second one-shot failure while
  adapting its already-sticky resource state throws from the pre-`try` path.

All three record a real allocation request and an escaped exception. The
[JUnit](../../../tests/native/v5-02-x05-b2-a-review-evidence/adapter.xml) records
three failed cases, with six failed assertions; the executable exits **42**.
[Exact compile/link/run commands](../../../tests/native/v5-02-x05-b2-a-review-evidence/adapter-commands.json)
are retained. These are controlled one-shot allocation failures, not natural host
memory exhaustion or a claim about process RSS. No source variant or production
test knob is used.

A fresh corrective round must include these early branches in the typed error
boundary and preserve the first resource failure, owner terminality, redacted
logical location and ordinary recoverable protocol refusals. Existing failures
and this candidate must remain available. This review does not prescribe a
broader API or a new diagnostic vocabulary.

## Identity, accounting and ordinary behavior

The bounded implementation otherwise matches the authorized identity foundation.
It selects the external shape namespace from retained native entries, reads final
D and privately copied original module bytes, and hashes exactly descriptor,
`{tier:"external",module:"external/"+path}` and lowercase source HEX in
`shape_definition_v1`. Ordinal selects the original module but is absent from the
hashed owner. No unrelated module/profile/mode/host selector enters the object.
The result owns its ID, revision and digest; neither retained source nor a
writable descriptor alias escapes.

The native source has no Lua call, F access, provider/filesystem operation,
reload, invocation transition or budget reset. Generic profile invocation remains
closed. Every lookup recomputes, and successful work consumes cumulative native
charges. Final flat five-string D and admitted source/path bounds justify the
bounded size arithmetic. The reservation enumerates nine new graph nodes, six
borrowed D nodes and ten encoded map keys; five new map entries receive separate
slots. Staging/graph HEX and module-path strings, envelope/key/tier text, one
canonical output, digest and returned metadata copies are reserved before their
allocations. The existing hash helper constructs one four-item domain frame and
encodes once; XXH3 reads it directly. These are logical charged capacities under
the existing metric, not RSS or a new quota. B2A-R1 is specifically the omitted
early error adaptation boundary.

Three additional [passing controls](../../../tests/native/v5-02-x05-b2-a-review-evidence/adversarial.cpp)
pass **377 assertions**. They compare a maximal ID/revision/help descriptor,
UTF-8/comment/quote/backslash/tab/CRLF source and a nested logical module path
against independent native object composition. A same-ID projection remains a
separate namespace; a NUL-suffixed unknown ID refuses without disclosing it.
Every observed one-shot allocation failure along a successful lookup returns a
resource refusal and leaves same-owner retries terminal. An unknown-ID protocol
adaptation failure inside the existing `try` also returns a typed resource
refusal. The passing test whose name mentions sticky adapters verifies this
inside-`try` unknown-ID control; the separate failing adapter cases cover the
actual early exhausted branch.

## Independent gate and immutable evidence

[Commands](../../../tests/native/v5-02-x05-b2-a-review-evidence/commands.json)
rerun **252/252** focused Debug cases in **130.43 seconds**: 245 unit/native and
seven acceptance cases, zero failures/errors/skips/disabled cases. All six new
production lookup cases pass. The 606-name inventory preserves every B1 name
(600) and A4 name (582); this is not a full 606-case run.
[Raw evidence audit](../../../tests/native/v5-02-x05-b2-a-review-evidence/raw-evidence-audit.json)
matches unique selected names to JUnit, console starts and immediately saved
LastTest records for both author and reviewer gates.

All four existing vectors and the independent Python canonical/upstream XXH3
shape vector pass. The fixed 733-byte known answer remains
`xxh3_128:b6d3718ddeb0cd1bc49b9e866a638b99`. All five inherited audits and the
new native lookup audit pass. Pinned formatting/Doxygen passes for **118 active
C++ sources**, and `all planar_tests` reports `ninja: no work to do.`. Source
audits complement runtime controls; they do not prove callable execution.

[Scope audit](../../../tests/native/v5-02-x05-b2-a-review-evidence/scope-before.json)
verifies all 12 candidate files, all 76 wrapped author-manifest artifacts, exact
owned-diff reconstruction and all 378 frozen entries. The 375-member B1 scope is
preserved; its nine changed existing files match the authorized ownership, all
nine archived originals match B1, and the three additions are support scripts.
This is scoped membership/byte verification, not a whole-tree freeze. AGENTS.md
remains a symlink to CLAUDE.md. Author exploratory failures remain preserved and
are distinct from final accepted test runs.

A reviewer helper initially stopped after successful adapter compilation because
its link-command loader used a nonexistent earlier-review filename. The initial
helper and its compile command are retained as `adapter-attempt01.py` and
`adapter-attempt01-commands.json`. Correcting that review-only path allowed the
unchanged controls to link and expose R1; no candidate edit or product rebuild
was needed.

| Author identity | SHA256 |
| --- | --- |
| Candidate manifest | `b80a540b4c0db0a79b9acf560d039f2935e1c65c8d8ada9b6cfa98db36376a14` |
| Owned diff | `edb204aca17f35691af73155e2f4de4ac18026d879991b1a4933de7064bc1959` |
| Artifact manifest | `17aecdc1eb098b6949339a735aa14ca7554f446f120a3805b7b9c35e4bbaf21f` |
| Author report | `20c88b35324c8825df0618c135ee3450cc6638244d5ee4df6dfb29088b76354c` |

Final candidate, artifact and scoped source identities are checked again after
all verification; review artifacts and this report are bound by
[finalization](../../../tests/native/v5-02-x05-b2-a-review-evidence/finalization.json).
Debug build ownership is released after final hashing; no reviewer edits follow
release. A fresh search/coder/reviewer corrective round is required for B2A-R1.
B2-B codec accounting, B2-C retained F dispatch/immutable input/complete-return
transport, B3 identity/W, B4 render, X06–X08, execution/RA01 and Linux remain open.
Public init creates format1. The latest full gate remains A4's 582 per build.
