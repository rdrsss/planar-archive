# V5-02-X04-A2 — Strict profile and selected closure capture

Status: search complete; implementation in progress after independently approved A1. Shared branch
`dev/planar-v2-vernacular`, HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`.
Preserve the intentional rewrite. Fresh Luna-high search, Sol-medium coder,
Sol-high reviewer. No Planar tools/skills/queue, installed binary/operator data,
commit/install/publish/merge. Coordinator owns this round. P1 A/P2 A approved.

Read exact X-CAP/X-LIMIT/X-CLI and A1 APIs/review. Map strict Glaze JSON profile
schema, ordered portable module paths, exact original bytes/modes, per-file/total
budgets and no bytecode. Compose A1 metadata admission and native bound reads,
retained closure/receipts and staged supplied-input admission. No external Lua
execution, registration, CLI flag, provenance/interpretation v2, or ledger wiring.
Known/late native seams are required; actual phase wiring remains X05/X07.

Fresh search is read-only: map exact reusable JSON/value/path/capture helpers,
new profile module ownership/tests and requirements. Check integer-version error
classification beyond INT32 against exact X-CAP, duplicate/escaped keys, unknown
fields, depth/node limits, path segment case-fold aliases, profile Unicode filename,
module bytecode/text bounds, aggregate quotas, physical aliases before reading,
receipt transfer/lifetime and unchanged siblings. Identify real contract issues
without silently narrowing approved behavior or adding handwritten JSON parsing.
Recommend one bounded implementation or justified split, exact tests and commands.
Current451 inventory must be retained; A1 is native prerequisite proof only.

## Search disposition and implementation scope

Fresh search complete. Coordinator corrects its admission-order suggestion:
X-CAP explicitly captures the trusted closure, then checks known shipped slots
after replay miss and before external evaluation. Do not move missing-input or
overlap refusals ahead of profile capture. A2 exposes open/capture then staged
admission methods; late slots remain post-registration/post-parse before handlers
in future X05/X07. No newly supplied untrusted contents are read by admission.
Metadata-select every listed trusted module before reading any module content.

Native bound reads need an explicit byte ceiling and remaining module-byte budget
checked from the bound descriptor before allocation/read. Keep old overloads and
budgets exact; tighten profile to65536 and modules to1MiB each/8MiB aggregate.
The module list/order and exact profile/raw bytes/modes are retained separately
from sorted capture snapshots. A2 must retain both A1 admission and native content
receipts for repeated original verification and later transfer to provenance/runtime.
No Lua/CLI/ledger integration. Fresh profile semantic acceptance remains X06.

Use a profile-specific Glaze adapter. Do not widen canonical JSON/int32 codecs.
X-CAP says unsupported integer versions are unsupported_version, without a
machine-integer cap: preserve that for any syntactically valid integer token
within the bounded input. Glaze may retain a validated raw number view; classify
integer versus fractional/exponent and exact supported spelling after library
validation, without implementing JSON/number grammar. Existing node/context
pattern is in json_codec/codec.cpp. Pinned Glaze provides raw_json_view and
runtime context max_string_length (core/context.hpp); inspect its semantics for
bounded decoded strings. No manual parser, grammar scan or serialization.
Schema/duplicate/unknown/container errors stay schema; resources use X-LIMIT.
Use tests for INT32/INT64/UINT64 overflow and long integer tokens to avoid an
accidental narrowing. If the library cannot meet exact policy, report the concrete
constraint before changing a reviewed contract; no new operator choice assumed.

## Ownership and verification

Coder owns new src/engine/capture/profile.cppm/.cpp/.t.cpp; narrow native.cppm/
native.cpp/native.t.cpp for bounded-read seam; engine/CMakeLists.txt wiring.
Do not alter admission semantics or canonical/json_codec/vendor source. New
 tests/native/v5-02-x04-a2-support/ and evidence/; status/reference prose in
CLAUDE.md, docs/architecture.md, and vernacular implementation/roadmap/alignment/
acceptance-matrix/external-profile-implementation.md. Coordinator owns round.
Send API/bounded-parser proposal before product edits; routine interface review.

Meaningful tests cover all profile/schema/path/byte/count limits; full-list
validation and metadata selection before module reads; arbitrary-size integer
classification through Glaze; bytecode/NUL/UTF-8/source-only; raw whitespace/modes/
module order; Unicode host filename; aliases/symlinks/ancestor/mode changes and
unrelated siblings; staged input checks, retained snapshot/receipts, moved/failed
owners, exact diagnostic privacy. Reuse approved positive-controlled read observer
via narrow test wiring or new support wrapper, without editing frozen A1 evidence.
Verify profile65536/65537 and remaining module budget before content allocation/read.
Report aggregate64MiB/10000 composition boundaries truthfully; native-only profile
capture cannot prove later whole-invocation sharing before that integration exists.

Full pre-edit baseline/symlinks/root-only exclusions and owned snapshots required.
Serialize debug all+planar_tests, focused profile/capture/path/acceptance nonzero
zero skips, relevant vectors/source audits/lint/no-work, preserve451 names.
Snapshot final source before commands; no source edits during commands. Use unique
per-attempt evidence paths so earlier raw outputs are never overwritten. Freeze
all reports/candidate/diff/manifests before release, then no edits. Independent
review and integrated X04-A gate follow; no public profile or Linux claim.

## Coordinator interface disposition

Approved the move-only profile capture owner, immutable ordered closure,
staged supplied-slot admission and retained metadata/content verification.
A profile-specific error wrapper carries only logical fields alongside existing
native diagnostics. Bounded native reads bind the reopened descriptor before
checking per-read/remaining-total limits and before allocation or content reads.

The coder inspected pinned Glaze and found its JSON reader does not consume
runtime max_string_length/max_array_size context fields. Their declarations alone
are not evidence of JSON enforcement. Approved a profile-only fixed-capacity
string adapter: Glaze captures/validates raw tokens, a 6144-byte raw string-body
ceiling bounds the scratch required for at most 1024 decoded bytes (each decoded
byte requires at most six JSON escape bytes), and Glaze decodes escapes into
fixed scratch before the decoded bound permits retained allocation. The coder
must verify actual library resize/padding requirements and safe overflow refusal;
boundary, escape/surrogate and invalid-token tests must support this approach.
No handwritten JSON grammar, vendor changes or generic codec widening authorized.
This is an implementation proposal, not executed verification evidence.

Coordinator additionally approved metadata-only `verify_file(slot, descriptor)`
forwarding on the profile owner so later supplied-input readers can bind their
opened descriptor to the retained admission receipt without exposing mutable
admission state. Test original/mismatched descriptors and terminal failure;
this does not implement a read service or its later CLI/runtime phase wiring.
The coder also identified bounded-entry verification rereads using the old
larger raw limit. Preserve the observed failing regression and narrow correction
that checks the original charged size before rereading bounded entries, including
cached reads, finish and acceptance verification. Existing unbounded overloads
retain their behavior. Final evidence/review must establish the correction.
