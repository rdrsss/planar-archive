# V5-02-X05-C2-B2 independent review

Disposition: **approve**. No required changes or unresolved product choices were found in this bounded retained-projection bridge/raw-admission candidate, including its indexed shared-resolver extraction. This approves C2-B2 only; it does not close projection read services, composition, receipts, public admission or the external-profile gate.

The shared checkout is `dev/planar-v2-vernacular`, HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. I owned only this report, `tests/native/v5-02-x05-c2-b2-review-evidence/`, and exclusive serialized ordinary Debug verification. Product, author tests/support/prose/evidence, historical files and release were read-only. I did not revert others' work, use Planar tools/skills/roles/queues, delegate, commit, install, publish, merge or access operator data.

## Exact reviewed identity

The reviewed [round](V5-02-X05-C2-B2.md) includes the bounded bridge authorization. I read the active guide, implementation plan, roadmap, v5, amended [X-PROJ/X-LIMIT](../external-profile-contract.md), B0/B1 counting proof and C2-A identity evidence before completing this review. Current prose still says fresh review pending; this report supplies that disposition without changing the frozen prose.

All hashes are SHA256 of exact files. [raw-evidence-audit.json](../../../tests/native/v5-02-x05-c2-b2-review-evidence/raw-evidence-audit.json) records the audit.

| Identity | SHA256 |
| --- | --- |
| Author candidate manifest, 15 mode/hash entries | `ccdc2b51bf79fc939134c3a9b8a0336306b7c5d47b932e698c9db71ddb847e1a` |
| Frozen candidate, 231 entries | `0f10d1de1e900cff3a3d60497674a21db3b53b6a80f5491b7a0bae1812c5e2a2` |
| Author evidence manifest, 8,283 entries | `6d741370d33c49171852ac4c81fe184e7ac9bf3cf102a005033df2ce893072e0` |
| Scoped actual-preedit diff | `94848c19c6fa134e8faadd54be4b119a000ada635977863a09d42944e2b75052` |
| Author REPORT | `c817d85ef41c81ce60694b314758d63034434958751f10489d66626fa3c8a5af` |

## Contract and boundary assessment

- [runtime.cppm:139–145,190–197](../../../src/engine/runtime/runtime.cppm) explicitly makes the read-data discriminator/authenticity a trusted synchronous native-caller precondition. The bridge checks immutable transport and descriptor kind; it does not claim that arbitrary caller records are authenticated role/history service results. No generic invoke/provider/CLI capability is opened.
- [runtime.cpp:3234–3316](../../../src/engine/runtime/runtime.cpp) preserves prior terminal state before new guards, finds original external projection order, binds the approved owned definition, checks exact input kind and validates transport before one retained F call. Healthy protocol guard failures remain nonterminal. Fixed scanner capacity is charged before invocation action. Existing `begin_invocation` at 1711 shares cumulative invocation accounting after the one registration transition; no new instruction/native/node reset exists.
- [runtime.cpp:250–384](../../../src/engine/runtime/runtime.cpp) uses one authoritative GLOBAL18 union for both kinds. The scanner enforces integer version1, exact record fields and tagged list semantics; list traversal caps at65 and then requires every index1..count. Untagged records, explicit record tags and genuine known frozen backing resolution agree with existing `convert` at 394–499; unknown metatables refuse without calling them. Type checks precede `lua_tolstring`, so keys/numbers are not coerced. Exact schema bounds root/section traversal and prevents a valid cyclic graph. Uniqueness/ID grammar, UTF8/no NUL, 64 sections, 1MiB text and checked16MiB aggregate are applied before native graph copying.
- `scan_projection` runs inside `lua_pcall`, with fixed borrowed POD storage and checked32-slot capacity before pushes. Its helpers use raw Lua access, trivial locals and allocation-free text validation ([value.cpp:25](../../../src/engine/value/value.cpp)); no native owning destructor crosses a Lua longjmp. The result remains rooted in stack slot1 while scan returns, outside-Lua B1 count runs, and only then `convert` copies it ([runtime.cpp:3321–3348](../../../src/engine/runtime/runtime.cpp)). No AST copy or Glaze scratch allocation precedes successful raw admission. The existing B1 reservation callback charges before its sole scratch request; resource and encoded-output refusals remain distinct from malformed protocol adaptation.
- Existing alias-preserving `execute_shape` and `release_shape_temporaries` ([runtime.cpp:1643–1710](../../../src/engine/runtime/runtime.cpp)) are reused with the original retained F reference. Transient proxies stay backed through conversion; cleanup clears input/memo/stack and invocation tag/backing entries without publishing new permanent roots. The partial-first-baseline failure retains one inherited lazy bookkeeping map until terminal destruction; the author reports that limit accurately. It preserves the original registry/ref/tag/backing/sealed identities, rather than claiming both new bookkeeping keys are absent.
- The shared resolver extraction ([runtime.cpp:2894–3017](../../../src/engine/runtime/runtime.cpp)) changes only the failure-location parameter and wrapper. Public standalone lookup passes index0 and retains its prior broad location. Invocation passes original projection index before any identity resource failure. Existing first terminal errors win rather than being relabeled. The genuine pre-correction red at `development-probes-4/red.xml` has45 field assertions showing broad `catalog.projections` instead of index10; the frozen correction and all67 actual one-shot allocation positions preserve exact first code/message/limit/maximum/field across operations and moves.
- Completed restricted callback `LUA_ERRMEM` remains sticky through a caught nested `pcall`; successful emergency allocation retry remains healthy. I inspected the narrowly instrumented actual retained-F adapter and replayed both controls. These are controlled allocator refusals, not natural exhaustion. No serializer/parser/Lua/vendor/build/CLI bytes changed in this candidate.

## Evidence and independent verification

I audited final command exits, nonzero XML case counts and no failures/errors/skips, candidate/evidence identities, exact raw private transformations, and source-policy correspondence. Author evidence supports inventory706 preserving698 names; frozen focused440/440 plus11/11 supplement; codec25, projection identity9 and render46; seven vectors, six source/runtime audits, pinned formatter/Doxygen118 and no-work build. This is the author's focused Debug gate, independently examined, not a new full/release gate.

I independently executed:

| Check | Result |
| --- | --- |
| Ordinary Debug all+planar_tests before/after private checks | Pass; `ninja: no work to do.` |
| Direct production `[projection-admission],[projection-definition]` | 17/17 cases, zero failures/errors/skips |
| New independent production bridge checks | 3/3 cases, zero failures/errors/skips |
| GLOBAL18 contract/source correspondence | Pass |
| Author actual raw/count/GC observer replay | 2/2 cases pass |
| All sampled actual one-shot native allocation positions | 1 case,67 positions pass |
| Actual partial-baseline allocator failure replay | 1/1 case pass |
| Actual retained-F completed-memory and emergency-retry controls | 1/1 each pass; status4/two nulls and status0/one null respectively |

The new [adversarial source](../../../tests/native/v5-02-x05-c2-b2-review-evidence/adversarial.cpp) calls unchanged production `create(profile)`/`invoke_projection`, using existing fixture helpers. It independently constructs the16-section JSON framing overhead: encoded16,777,215 and16,777,216-byte results succeed,16,777,217 refuses with terminal resource/index1 while all raw per-text/count/aggregate limits still hold. Other controls cover explicit record tags, float version refusal, INT64_MAX/sparse list keys, insertion-order-independent dense lists, non-string record keys, surrogate/out-of-range UTF8 refusal, U+10FFFF success, repeated immutable registration-frozen returns and an empty native proxy-root/list response. These do not claim final read-service authenticity.

[commands.json](../../../tests/native/v5-02-x05-c2-b2-review-evidence/commands.json), JUnit files and [replay-commands.json](../../../tests/native/v5-02-x05-c2-b2-review-evidence/replay-commands.json) retain exact argv/cwd/exit/timing; `run.py` and `replay.py` retain driver/environment logic. Every private compile recursively expands response files, redirects module/object/dependency inputs/outputs, disables implicit module rebuilding, and links copied archives. All888 shared and copied object/PCM/archive members and bytes remain exact (`compiled-final.json`). Final independent fixture source was pinned-formatted and recompiled/reexecuted before finalization.

Two reviewer fixture mistakes are preserved, not candidate failures: the first private compile rejected a signed/unsigned comparison in my independent assertion; I corrected only the review fixture. The first memory replay supplied `C2B2_MODE` but omitted that adapter's separate `C2B2_MEMORY`; its null environment view caused the private adapter to exit−11. The corrected replay supplies both variables and passes. Initial logs/command records/XML remain under review evidence; no product fix was required.

## Preservation and remaining limits

A fresh complete review-start snapshot covers64,042 protected entries, including all4,238 release directories/files/modes/symlinks, author evidence and history. Final comparison reports zero changes; AGENTS.md remains the CLAUDE.md symlink. The author's separate actual-preedit protection also remains45,036 protected/4,238 release entries unchanged. `protected-final.json` records my independent comparison. `review-manifest.json` hashes this report and all final review artifacts except the manifest/finalization files themselves; `review-final.json` binds that manifest and this report.

No full/release/Linux gate, natural host-exhaustion guarantee, service input authentication, renderer/budget composition, provider/receipt integration, public cutover, X06–X08 or execution/RA01 acceptance is inferred. The current full676/configuration evidence remains its separate checkpoint. Public init still creates format1. Approval applies only while the exact reviewed candidate remains current. Exclusive Debug and all review edits are released; no further edits are planned.
