# V5-02-X05-C2-B1 independent review

Disposition: **approve**. No blocking findings. Approval covers the native
projection-size prerequisite and the operator-approved Global18 X-PROJ amendment.
It does not approve projection result admission/dispatch, Lua string rooting,
read services, public profile admission or the mandatory external-profile gate.

The reviewer used the shared `dev/planar-v2-vernacular` checkout at
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, preserved the inherited dirty rewrite
and others' work, and owned only this report and
`tests/native/v5-02-x05-c2-b1-review-evidence/`, plus exclusive serialized ordinary
Debug verification. Product, registered tests, author support/evidence, current
prose, history and release remained read-only. No Planar tools/skills/roles/queues,
subagents, fixes, commits, installation, publication, merge, installed Planar
binary or operator data were used.

The review read the active guide, implementation plan, roadmap, v5 design,
complete B1/C2-B briefs and operator decision, approved B0 review/source/proof,
exact X-PROJ/X-LIMIT, author report, candidate diff, codec APIs/tests and pinned
Glaze writer paths. The pre-verification baseline is `baseline.json`; commands,
raw logs, private inputs and independent source are retained alongside it.

## Candidate and evidence identity

The exact 232-entry candidate manifest SHA256 is
`793584ecc965e2e006f1b56af925bd678789a0348734e1ec71e7c8ea88029b99`.
The 363-entry author artifact manifest SHA256 is
`6c3e835cc20d8624b888081e9e190d7a7adc8149ba96b6c0532e53686ab4b7b0`;
the author report SHA256 is
`fe631720909cba25d7eec01022af26af79318c8be05863910e1a109ae7f55de1`.
Independent `audit.py` checks every candidate mode/digest, archived candidate
byte, author artifact and all three nested 888-entry private compiled trees.
Each private tree maps exactly to its own captured shared inputs. The first
development tree predates the final rebuild and differs in 168 entries; the
second development and final trees equal the final shared compiled inputs.
Historical development inputs are not represented as final-candidate reruns.

The audit independently parses the raw XML: **432** focused, **11** supplemental,
**25** codec, **9** projection-identity and **46** production-render cases, all
without failure/error/skip. The **698** registered names preserve all **691**
prior names. Gate command exits, seven vectors, six source/runtime audits, pinned
formatting/Doxygen over 118 active sources, and the no-work log agree with the
author report. A line diff verifies all prior lines of the three codec files
remain, with only insertions. `candidate-audit.json` retains these results.

## Source assessment

`src/engine/json_codec/codec.cpp:415` validates the complete span before the hook
or scratch: <=64 sections, nonempty <=64-byte ASCII declaration IDs, bytewise
uniqueness, UTF-8 without NUL, <=1 MiB per string and subtraction-checked <=16 MiB
raw total. `value/value.cpp:25` rejects overlong encodings, surrogates, truncated
sequences and code points above U+10FFFF. Version1 and mandatory's boolean type
are fixed by the reflected native records. No malformed content can bypass the
preconditions supporting the scratch derivation. As with ordinary C++ views,
the caller must provide valid storage and keep views/bytes/context unchanged
through the synchronous call, including callbacks; no raw-Lua lifetime is proven.

`codec.cpp:131` and `:193` retain the approved bounded, nonresizable streaming
adapter. Source review includes every relevant unchecked dump/memcpy and its
caller padding, not just the checked string entry point. Pinned
`glaze/json/write.hpp:861` reserves `ix+10+6*n`; `:1239` skips array preallocation
for bounded storage; `:1160`/`:1395` flush separators; `:2469` writes fixed
reflected fields without intermediate flush. Empty-array and closing punctuation
dumps remain covered by the preceding root/field/string padding. Pending bytes
are <32768 after separators, a maximum section is <=6 MiB+101 bytes, and the
272-byte static-field preflight plus closing syntax fit the conservative 1024
allowance. Thus **6,325,248** physical scratch bytes cover the admitted types.

The buffer advances its logical offset only after consuming a chunk. The sink
at `codec.cpp:139` uses subtraction before addition and sticky overflow, never
throws, allocates or dereferences payload. Pinned `core/buffer_traits.hpp:108`
makes flush noexcept, so nonthrowing continuation is necessary. Writing finishes
the admitted input after overflow, with <=100,669,888 logical bytes of work; it
then returns output_limit. Pinned `core/write.hpp:19` propagates writer error
before finalization and finalizes success once. A writer-capacity failure remains
scratch_defect, distinct from encoded overflow and resource denial.

`codec.cpp:408` holds the fixed logical workspace. `:758` preflights, then invokes
one synchronous hook with a checked **6,325,248 + 200 = 6,325,448** logical charge
before the vector resize. No generic AST, string copy or retained output is
created. The workspace sizeof is a pinned-target logical reservation, not a full
native stack-frame, allocator-overhead or RSS measurement. There is no refund,
reset, detached plan or global allocation routing. Independent nested/concurrent
owners are supported under the documented stability precondition.

The denial at `codec.cpp:771` moves all owned callback diagnostic fields unchanged.
The inner and outer guards cover bad_alloc/length_error, unexpected exceptions
and diagnostic construction in handlers. Final fallback may allocate; persistent
host heap exhaustion is explicitly outside the guarantee. No new public profile
diagnostic or runtime mapping is introduced.

`external-profile-contract.md:363` records the settled global union of all 18
names for both input kinds, including conditional `shape_policy`/`preferences`,
optional role sections, generated `omissions` and `recorded_history`. The audit
checks the exact amendment against the operator decision and shipped role names.
The codec intentionally accepts reserved names for counting: semantic collision
admission and a shared reserved-name authority remain the later native scanner's
duty. No new policy decision is required here.

## Independent verification

The reviewer copied and rehashed all **888** shared .o/.pcm/.a inputs privately,
expanded response files, redirected all module inputs and object/dependency
outputs, disabled implicit modules/module maps, and linked private archives.
No private compile wrote shared compiled files. `commands.json`,
`corrected-command.json`, `copied-compiled.json` and named raw logs bind execution.

- Fresh submitted whole-operation probe: **35 cases, 474 assertions, 45 actual
  allocation fault positions**, passing, including moved/copy-failed diagnostics,
  nested inner failure/outer success, maximum raw/escaped data and threshold±9.
- Fresh independent `adversarial.cpp`: **67 fixtures, 332,510 assertions, 3 actual
  fault positions**, passing. It checks every leading/trailing ID byte, all
  **65,536 two-byte text combinations**, valid UTF-8 scalar extremes, malformed
  UTF-8 at the end of a full 1 MiB string, duplicate IDs backed by distinct
  strings, count/string/raw-total bounds and refusal before hook/scratch.
  Actual Glaze output supplies the size oracle for threshold−12..+12, 32
  deterministic mixed 64-section fixtures, maximum escaping and exact16MiB±1.
  Tightened ceilings, exact logical precharge, one fixed scratch request freed
  before return, fault cleanup and independent nested owners are exercised.
- Fresh controlled physical-scratch adapter: exactly one substitution changes
  resize to 512 bytes; it returns scratch_defect/internal, passing. This is a
  controlled adapter, not natural exhaustion of the proven production bound.
- Direct registered codec tests: **25/25**, zero failures/errors/skips. Corrected
  ordinary Debug all+planar_tests build reports **no work to do**.

No full suite or release binary was run. The latest independently approved full
integrated B4-C gate remains **676/676 per configuration**; this review adds no
full-milestone, release or Linux acceptance. Future scanner/dispatch must establish
Lua root lifetime and immutable source views, native schema/semantic admission,
existing cumulative budgets and external diagnostic mapping. Read services,
receipt/whole-invocation orchestration, X06–X08, public cutover, execution/RA01
and Linux remain open. Public init still creates format1.

Reviewer harness mistakes are preserved: `attempt-audit/` initially compared
the historical development compiled snapshot to final inputs, and the no-work
preset attempt ran from the build directory without CMakePresets.json. The
corrected audit uses each tree's captured original, and `final_check.py` uses the
explicit build path. `attempt-final-check/` preserves the first final audit's
overstrict refusal of a changed Debug `.ninja_log`; this ordinary verification
metadata is allowed explicitly in the corrected audit. These failures are not
product evidence or counted as success.

Final `protected-final.json` checks membership/modes/symlinks and bytes of all
**52,378** baseline entries, including author/history/support and both build
trees, plus the reviewer's private dependencies. All **4,238** release entries
and **888** shared compiled inputs remain unchanged. Of **4,304** Debug entries,
the only byte change is permitted `.ninja_log` metadata; both hashes are retained
in `protected-differences.json`. No compiled input or build graph changed.
Candidate and author manifests, HEAD and AGENTS.md -> CLAUDE.md are rechecked.
The reviewer artifact manifest binds this report and evidence, with private
dependencies transitively bound by `copied-compiled.json`; it excludes itself.

All Debug use and all reviewer edits are released. **No later edits are planned.**
