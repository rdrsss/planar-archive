# X05-C3-A pure native invocation preparation candidate

Status: implemented and verified, awaiting fresh independent review. No self-approval.
Shared starting HEAD 3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a on
`dev/planar-v2-vernacular`. No commits, installation, publishing, merging, Planar
operations or operator-data access. Debug and all owned edits are released with
this completed handoff.

`command_vm::prepare_extension_invocation` selects an actual retained sealed kind 0
external command and accepts already parsed borrowed records. Names omit `--`;
omission means absent, explicit absence cannot contain a token, present flags
contain no token, and present scalars contain exactly one. Unknown/duplicate names,
required absence and inconsistent records refuse. The closed error enum distinguishes
usage, protocol and resource outcomes without echoing any supplied value.

The owned result has every canonical descriptor option and authoritative presence.
Its scalar variant carries strings, signed32 integers and booleans; history IDs
remain exact strings, including the absent string "0". A separate variant alternative
carries an optional typed pending-slot index. Only supplied capability-bound hosts
have a native pending selector; absent host options have empty indices, not scalar
path substitutes. Slot order follows canonical descriptor order. Capability-free
recognized path spellings remain ordinary strings under the approved X04-B2 rule.
No value_ptr, JSON graph, Lua invocation, argv parser, filesystem normalization,
handle, receipt, handler or provider is introduced.

History IDs use canonical positive INT64 grammar/from_chars; ordinary decimal tokens
are bounded to 10 bytes and signed32, history tokens to 19 bytes and signed64. History
selectors exclude one another and paging, while after-event+limit page reads remain
valid. Native effective cursor 0 / limit 50 are distinct from absent option string "0"/
integer 0 and false presence. Request IDs enforce ASCII 1..128. Role intent retains
root/base first-equals syntax and prefix matching, base/capture presence matching,
and budget 1..1048576. Broader path and semantic admission remains future work.

Preparation does not start invocation or reset a budget. Healthy owners remain
usable after usage/protocol refusal; resource refusal is terminal and preserves
the existing shared first error across APIs and moves. Embedded/moved/failed/
consumed owners and unknown external commands refuse. Caller-mutated prepared
results are not later dispatch authority. Future dispatch must bind original
same-owner identity, capabilities, intent, admitted slots and receipts.

## Verification and preservation

The actual preedit baseline is in `preedit/`, `owned-before.json`, `git-before.txt`,
`head-before.txt`, `protected-before.json`, `release-before.json` and the 706-name
`inventory-before.json`. These record source/ownership/identity and inventory;
no preedit test-suite execution is claimed. The large pre-existing Git rewrite
was preserved, and candidate.diff compares only actual owned preedit bytes.
This is a new seam, without a pre-existing failing API regression.

Frozen candidate: 227 entries, SHA256
`c8dd36cd6d7dffef2139211be017529587f59f8ceb11af0027837fd30f92b138`.
Scoped diff SHA256
`8a0d0357b17022a117dd91b6933f2aac0eb57f4cc95a8779fea349123c6f9c17`.
`candidate-source/` preserves the eight owned product/test/prose files;
`frozen-candidate.json` also identifies support and unchanged verification inputs.
Product/test/support/prose stayed frozen through the final gate and final identity
verification. Only this evidence report and finalization manifests were added later.

Final serialized focused Debug gate passes **275/275** runtime/command cases
(87.368 seconds), with supplemental history/codec **11/11**, zero failures/errors/
skips/disabled cases. Inventory 716 preserves every prior 706 name and adds ten
production cases. Pinned formatting/Doxygen checks 118 active sources; direct
all+planar_tests build and final no-work build pass. Executed command arguments,
working directories, timings and exit codes are in `final-gate/commands.json`;
JUnit, selected names, CTest LastTest copies, inventory preservation and raw output
are adjacent. Entrypoints were:

- `python3 tests/native/v5-02-x05-c3-a-support/freeze.py freeze`
- `python3 tests/native/v5-02-x05-c3-a-support/gate.py`
- `python3 tests/native/v5-02-x05-c3-a-support/freeze.py verify`

The ten cases cover all three history-ID boundary/malformed tables, complete
selector exclusion, paging/defaults, bounded request IDs, typed general scalars,
parsed-record invariants, all capability-free host-spelled text, native host kinds/
indices/order/absence, role matching/budget, owner guards, ownership/moves, earlier
failure dominance and cumulative accounting. Handler F throws if executed; valid
preparation succeeds without entering it. Native result types are tested, not
future Lua value types/presence materialization.

Each prospective output reserves complete vector capacity and every string copy
before allocation under the existing cumulative native metric. The string charge
includes a separate sizeof(string) allowance plus length+1; that conservative
allowance also covers pinned libc++ constructor capacity rounding for these bounded
strings. No incremental string growth occurs. The real native-budget test performs
actual 65536-byte preparations until the existing 512 MiB charge boundary refuses,
then checks identical first failure across other APIs and moves. No allocator
fault injection, private compilation, instruction reset, natural host exhaustion
or process-RSS measurement is claimed. Details are in
`../v5-02-x05-c3-a-support/accounting.md`.

Development build/test logs are retained; the first gate attempt stopped at a
support-only inventory-name assertion because CTest prefixes names with
`replacement.`. It ran build/inventory only, never focused tests. Original support
bytes/hash, initial frozen manifest and raw attempt remain under
`final-gate-attempt1/`, `final-gate-attempt1.log` and
`frozen-candidate-attempt1.json`. Only the support prefix assertion was corrected;
product/test/prose bytes were unchanged, support was refrozen, and the complete
focused gate then passed. No product test failure or weakened assertion is hidden.

The protected snapshot covers 60,978 existing entries outside owned files and the
coordinator-owned rounds, and 4,238 release entries. Final verification checks full
membership, modes, bytes and symlinks against the preedit snapshots; AGENTS.md
remains the symlink to CLAUDE.md. See `protected-final.json` and finalization/artifact
manifests for exact identities.

No full milestone, release or Linux gate is claimed; the latest independently
approved full gate remains 676 per configuration. Receipt-backed host admission,
Lua string/presence ABI proof, native read services, capability/order/once-only/
poisoned-service behavior, authenticated C2 input composition and response
correspondence remain open. Generic profile invocation remains closed. Public
profile CLI/cutover, X06–X08 and mandatory external-profile acceptance, execution/
RA01 and Linux remain open. Public init creates format1.
