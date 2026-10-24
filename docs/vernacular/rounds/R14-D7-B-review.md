# R14-D7-B independent review

Disposition: **approve**. No blocking findings or requested changes in the frozen
public load integration. This approves D7-B only; full R14 remains D8, followed by
the mandatory external-profile gate.

Reviewed the shared `dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, against the rewrite guide, implementation
plan, roadmap, v5 direction, approved exact load amendment, D7-A review, D7-B brief
and raw coder evidence. Writes were limited to this report and
`tests/native/r14-d7-b-review-evidence/`. No product or prior evidence was changed,
and no Planar planning tools, skills, queue, installed binary, operator data,
commit, install, publish or merge was used.

## Exact candidate and preservation

The 26-file repository-relative wrapped candidate manifest has SHA256
`c73a1faa5270bd57e6c6f69fc8d973cac1e139fe7781820ffb268ea8ebaec0dd`;
`owned.diff` has SHA256
`f68d11a51ab8cbb54ad66e97a149bfd1cce61996a4e51595164296cf4b63733c`.
The 176-artifact evidence-relative manifest has SHA256
`977eb5012f16afd451503e82f86976ce5955911bf8adf7f36c18c6526be76a3d`.
All current candidate files, frozen snapshots and manifested artifacts matched.
Independently rebuilt the owned unified diff byte for byte.

Verified all 14,301 contemporaneous baseline archive entries against the baseline
manifest, including symlink targets, and all 20 extracted owned baseline files.
Current protected paths have no unexpected changes; added paths are the six owned
candidate additions plus this review's permitted outputs. The coordinator's round
record is an explicit exception. Historical evidence, fixtures and goldens remain
protected by the complete baseline. `AGENTS.md` remains the symlink to `CLAUDE.md`.
The contract body beginning at `Fixed catalog admission and typed intent` matches
both the baseline and D3-approved snapshot, SHA256
`9360af4d6d38983f36e01e0019fd723c7ac01b0a23f6e8d3db1a5cfb4eea2634`.
The independent checks are reproducible through `review.py` and recorded in
`identity-audit.json`; candidate identities were rechecked at completion.

## Implementation assessment

`handlers/load/command.cpp:22` attaches six required single text values through
CLI11. Canonical positive signed-64 head tokens, request keys and UTF-8 actors
retain native validation. `bootstrap.cpp:66` composes the fifth family into parsed
metadata checks, fixed help, whole catalog comparison and retained dispatch.
`runtime.cpp:16` changes admission from exactly four to exactly five; existing
sealed signature/family/capability checks require load atomically. The real
`commands.lua:16` routes and interprets the whole captured bundle before calling
accept; its mandatory descriptor is registered at line 77.

`invocation.cpp:296` uses one relevant environment/cwd snapshot, lexical workspace
and registry targets, and preserves ledger parent spelling. It performs no input
existence check while framing intent. `invocation.cpp:439` creates immutable
request intent, opens the explicit ledger read-only, compares exact verified body
bytes and publishes early replay before invoking Lua, capturing inputs, assembling
acceptance provenance or checking current head. The scoped read owner closes
before fresh work. Changed actor/head/workspace/registry intent conflicts even
when inputs disappeared; ledger location remains outside the body.

`invocation.cpp:122` uses the real D5 capture/preparation owner. At line 143 it
borrows the stable VM completion receipt before preparation/completion or the
first decoded result allocation. At line 149 it passes that same receipt and the
verified prelude ledger identity into D7-A's identity-bound overload. Runtime
`call_service_native` supplies provenance from that retained VM's exact originals,
separator and assembly; the real shipped callable performs all-job interpretation.
The existing native capture membership/correspondence checks remain authoritative.
There is no new Lua, diagnostic encoder or provider callback under the native
write lock, and no storage/schema/codec or W-recipe amendment.

`invocation.cpp:432` prepares both load and stdout fallback bytes before replay or
acceptance. Early replay and successful fresh/late completion return the native
saved string plus one LF at lines 466 and 488. They do not serialize a replacement
Lua success. Both adapter and VM observe the same retained completion phase even
if the provider throws before returning its first service value. All later error,
invalid-return, conversion, encoding and exception paths consult that phase;
confirmed noncompletion retains ordinary native diagnostics. Existing checked
process writes honor the prebuilt stdout fallback. The final request-bound branch
at line 443 preserves `resource_limit` before any ledger access and emits the
exact redacted envelope exercised by the appended regression.

## Independent verification and evidence adequacy

Build directories were exclusively assigned and all operations serialized per
directory. Exact argv, UTC times and exits are in `commands.json`.

- `cmake --build build/rewrite-{debug,release} --target all planar_tests` each
  passed with `ninja: no work to do.`.
- The focused CTest selection recorded in `commands.json` passed **30/30 debug**
  in 21.83 seconds and **30/30 release** in 12.90 seconds, zero failures/skips.
  It covers native load capture/all-job/membership/receipt bounds, retained load
  protocol/provenance/terminal failures, identity-bound native completion and
  final capture refusal, CLI11/path/resource units, and the retained CLI,
  workspace-selector CLI and public load CLI suites.
- Each public load suite reported 119 checked invocations. It verified accepted,
  noop, early/late replay and native saved bytes; deleted inputs/advanced head;
  changed intent; name/explicit equivalence and environment conflict; preserved
  ledger traversal; concurrent equal/conflicting/CAS outcomes; prepared capture
  mutation; postcompletion Lua/codec/synthetic C++ faults; early transport and
  closed stdout fallback/recovery. Exact known recovery-required transients were
  logged (one debug, three release); bounded retry retained final history-row,
  conflict and CAS assertions.
- Both configurations passed independently rerun exact-source audits for all
  seven historical variant families plus fixed-export harness, six D6 variants,
  C01, and the entire new load probe graph/originals/separator/assembly. Generator
  amendments preserve every prior mode under current five-entry admission.
- JUnit and LastTest were archived before subsequent inventories. Both inventories
  contain **400** names and preserve all **396** D7-A names; `verification.json`
  records the four additions and exact executed selections.

Inspected the coder's raw final full JUnit/LastTest: 399/399 debug and release,
zero failures/skips, after the ledger-path correction. Those runs precede the
final small request-resource mapping change. Verified `final-gate-source.json`,
`final-source.json` and the exact two-file `intent-mapping.diff`: the sole product
delta is the error-code branch, with one appended regression. Final 69/69 affected
tests in each configuration include that regression and all seven CLI suites.
The independent 30/30 runs also exercise the final branch. This is adequate
applicable final retesting of the isolated correction; another broad repeat is
not needed for this review. No final 400-case full-suite pass is claimed.

Both red regressions retain failing JUnit/LastTest and source provenance. Verified
that the ledger pre-fix source is literally extracted from the retained
contemporaneous generator artifact, whose later copy is explicitly disclosed;
the resource pre-fix source matches the full-gate identity. The red logs show lost
ledger parent spelling and `schema` instead of `resource_limit`, respectively.
Earlier formal failed attempts remain retained. Reviewed the final pinned
format/Doxygen result for 95 active files, 12/20/41 unchanged vectors plus ten
independent record vectors, all final audits and both zero-work builds.

## Limits and handoff

Verification is macOS ARM64, LLVM 23.1.4, debug and RelWithDebInfo release. It does
not establish Linux, hardware power loss or actual C++ allocator exhaustion.
Generated `bad_alloc` probes inject synthetic exceptions; real Lua allocation/
instruction exhaustion and bounded codec refusal are separately exercised.
Native uncertain-COMMIT receipt behavior is inherited from independently approved
D7-A and exercised here through receipt/provider tests; no new public CLI VFS
commit-fault injection is claimed. Complete integrated R14 acceptance remains D8.

Debug and release are released idle. Approval applies while the exact reviewed
candidate remains current. Review artifacts have an explicit evidence-relative
manifest; `handoff-identities.json` uses repository-relative report/manifest paths.
