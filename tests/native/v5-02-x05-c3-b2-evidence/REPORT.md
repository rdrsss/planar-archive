# X05-C3-B2 original captured owner and initial extension admission

Status: candidate for independent review; no self-approval. Shared checkout
`dev/planar-v2-vernacular`, HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`.
The coordinator owns the round; this implementation owns only the eleven paths
in `owned-before.json` and this round's support/evidence prefixes. Other changes
were not reverted. No Planar tools/skills/roles/queue, delegation, commits,
installation, publication, merge or operator-data access was used.

## Implemented boundary

`runtime.cppm:304` exports a move-only internal `extension_invocation` with only
create, immutable catalog borrow, one admission attempt and receipt verification.
`runtime.cpp:3915` consumes the actual original `profile_capture`, calls its
snapshot guard, then internally constructs its VM. No independently made VM or
mutable C3-A result can enter this pairing. Failed creation publishes no owner.
The prepared invocation, original mapping strings, normalized paths, receipt owner
and VM stay private. Generic profile invocation remains closed.

Admission selects the actual retained sealed extension command and privately runs
C3-A on exact parsed records. All supplied host selectors normalize before the
first metadata action. Relative paths use explicit caller cwd; named workspaces
use one caller workspace-directory snapshot. First-equals root/base RHS handling
retains mapping IDs privately. Metadata uses `prepare_host_path.spelling`, including
physical parent traversal; it never uses lexical intent for admission. The approved
name rule first lexically normalizes the environment, then appends the name and
`workspace.lua`. Explicit paths retain their physical spelling even when containing
a symlink followed by '..'. No ambient snapshot reacquisition occurs.

Every supplied slot applies even if a handler would omit its service: workspace
containing root and selected declaration file, source/base roots, registry,
observations and ledger with their exact native roles. Pure same-spelled options
remain data. After all admissions, the same original capture verifies trusted and
late receipts, including capability-free commands with no late slots. Later verify
rechecks these originals and grants no execution/output authority. It requires a
successful admission; no admission retry follows partial work or success. Moves
transfer receipts/state and first failure; moved-from operations refuse. Failed
catalog borrowing returns empty. Errors preserve closed native classifications,
messages/limits and logical fields without selectors.

The authorized helper amendment exports pure workspace-directory validation and
extracts shared path/relevant-cwd text, size and absolute validation before helper
allocation. Existing APIs delegate to these checks. Lexical cwd validation remains
before joined-length overflow, matching the old helper. Malformed oversized
relevant snapshots retain usage/path_denied; valid oversized snapshots retain
path_bytes; irrelevant malformed snapshots remain ignored.

The [accounting proof](../v5-02-x05-c3-b2-support/accounting.md) binds exact pinned
libc++ path/vector/string sources and conservatively reserves component growth,
string copies, joined buffers and temporary helper work before allocation. Wrapper
allocation and retained path vector/parent storage charge the VM's existing512MiB
cumulative native metric; charges are never refunded or reset. Caller snapshots
are borrowed, not recopied. Existing capture metrics remain separate; this is not
RSS accounting or completed whole-invocation shared capture/VM budgeting.

## Actual baseline and candidate identities

Before product/test/prose mutation, owned bytes were copied to `preedit/` and the
protected snapshot completed. The current filesystem had neither rewrite build
directory. The first snapshot attempt failed on absent rewrite-release, without
product edits; the recovery recorded absence and protected `build/install-release`
without executing it. Fresh Debug configure/build then produced exactly723 names,
identical to the historical approved C3-B1 inventory. `baseline-complete.json`
records this completed preedit baseline. Newly authorized helper preedit bytes
match the original protected baseline; `ownership-extension.json` and the unchanged
`protected-before.json` preserve that ownership amendment.

| Exact SHA256 artifact | Value |
| --- | --- |
| Final `frozen-candidate.json`,236 mode/hash entries | `a0ce3d520a4dc5992bd2f1a04355a0ba7400bca9a4f7818cc0bda35d84b78c8e` |
| Final actual-preedit `candidate.diff` | `1467b53c526a0ed58b9b32f77ff0cf633db7fb0fee5eaabe17a289ec088ed92f` |
| Original409-test `tested-frozen-candidate.json` | `0f99cbcdeef3188a645a787341c0d904681d2e53afd007e8ed6a95c3f88f3f0d` |
| Original409-test `tested-candidate.diff` | `433c84f16f29ba781cfcec331d1681ac54e8366629171ff65423207b2b437166` |

The final candidate is intentionally distinct from the original all-source freeze.
The coordinator authorized two stale status prose corrections, evidence-local
source-audit/import/link corrections and Doxygen-only C++ comments after the first
focused run. `support-before-amendment/`, `tested-candidate-source/`, intermediate
candidate manifests and raw failed attempts remain. No test byte or executable
C++ token changed after409 passed. `comment-token-equivalence.json` compares all
35,742 non-comment libclang token kind/spellings, preserving literals exactly:
tested runtime SHA `a19e154cb52f3b8194ab8ce05ed6b5d4bc69a548451afb332ffddc0777878979`
and final `404cc6e643923d8a30e6fa803b6ec18d5a419174cdd1e289bcd1e070997d62ec`
have the same token hash `6505bef1eb8ea3b69fe3873894a696bd3f8cc9751ab950010fc4821ff5e39b47`.
Final twenty-case and private checks run against the exact final product bytes.

## Executed verification

Commands and durations are in `final-gate/commands.json`,
`final-resume/commands.json` and `private-probes/commands.json`. All builds/tests
were serialized for Debug. No full suite or release suite was run.

| Check | Result and limits |
| --- | --- |
| Fresh baseline Debug configure/build | Pass;723 exact prior names before mutation |
| Final inventory preservation |743 names, all723 preserved,17 owner and3 helper additions |
| Initial frozen focused CTest selection |409/409 in159.97s; zero failures/errors/skips/disabled;408 relevant runtime/command/capture/path cases plus workspace-selector CLI |
| Final comment-corrected build | Pass;34.73s |
| Final exact-source new cases |20/20 in2.98s; zero failures/errors/skips/disabled |
| Common deterministic vectors | Six pass; source inputs unchanged through finalization |
| Current generated/source audits | Six pass, including every original B2-A invariant on the exact current shape-lookup method plus separate owner/preflight assertions |
| Pinned formatting/Doxygen | Pass over118 active C++ sources |
| Final ordinary all+planar_tests build | Pass; no work |
| Private production-object allocator/read controls |4/4 cases; zero failures/errors/skips |
| Private pre-hardening control | Expected red, one case/one failure: field `profile` instead of original `catalog.commands` |
| Shared/copied compiled dependency identity | All888 members, modes and bytes unchanged; response files expanded and implicit modules/maps disabled |

Seventeen real scratch owner cases cover pure, role and history commands; every
supplied/optional slot; bare names and explicit snapshots; parent traversal;
symlink/missing/nonregular refusal; nesting both ways; selected aliases and registry
colocation; trusted/late-file/root/exited-ancestor mutation; original-capture final
verification; moved/poisoned creation; one-attempt/terminal lifecycle; relevant
malformed/valid oversized snapshots and irrelevant malformed snapshots; and actual
native cumulative reservation refusal before large helper allocation. Three helper
cases preserve shared preflight parity, text-before-size and lexical-cwd-before-join
precedence. Existing capture/profile tests retain original before-read binding
coverage. Future services must bind actual descriptors through this same capture's
original private receipt keys; no independent recapture or service seam is added.

Private checks compile a fixture against copied production dependencies, not a
modified public runtime API. All87 observed scalar-new admission positions are
failed once individually; each produces a terminal redacted native refusal and
preserves exact first diagnostics through moves/retries. Creation samples first,
middle and final observed requests out of6097. These are controlled allocator
failures, not natural host exhaustion or exhaustive aligned/Lua allocator coverage.

The first-error probe uses the real protected adapters. An initial failed allocation
publishes C3-A's resource diagnostic. Two faults are recovered by its nested
adapter; the preserved initial pre-hardening run is green and is not presented as
a reproduced defect. One authorized third-fault control at requests1/4/5 escapes
the outer adapter after publication. A private implementation reconstructed by
reverting only the wrapper's first-error adoption then fails exactly on logical
field (`profile` versus `catalog.commands`). Final unchanged production objects
pass; the wrapper adopts the original published first error before fallible copies.
This controlled red/green demonstration was made after the code-audit hardening;
it is not claimed as a naturally occurring failure or an earlier chronological run.

The read controls watch workspace, registry, observations and ledger inodes.
Admission and verification produce zero observed late-data reads, while explicit
read positive controls increment once. The existing syscall dylib is copied and
loaded through a private rpath. Original trusted content may be rechecked by the
capture API; no provider or handler parameter/API exists and the registered test
handlers would fail if executed. No service, userdata, Lua invocation ABI or
response-correspondence behavior is inferred.

Private compilation expands all responses, redirects objects/PCMs/dependency
outputs, maps every explicit module input to owned copies and disables implicit
module rebuilding. All object/PCM/archive/shared-library dependencies, including
external explicit inputs and pinned dylibs, are manifested and checked before and
after. Disposable compiled copies/current outputs are removed after comparison;
raw sources, commands, manifests, JUnit and logs remain. Failed-attempt disposable
outputs are hashed in `disposable-attempt-outputs.json` before removal.

## Preserved failed attempts

- Initial baseline helper required an absent rewrite-release directory; no product
  edit preceded the corrected, complete preedit baseline.
- Two exploratory fixture runs failed on expected native classifications: bare
  workspace alias versus explicit path, and symlink/nonregular path_denied versus
  io. Product behavior was correct; raw XML/logs remain.
- One editing-script guard failed to match a formatted call site after helper
  extraction; intended edits were completed before its subsequent successful build.
- The first focused driver passed409 and six vectors, then an old B2-A audit
  failed because its old create_impl endpoint included later C3-A code. The old
  script stays intact. The new evidence-local audit checks every original invariant
  on the actual method boundary and checks the new owner separately.
- Resume attempts preserve an import-generated cache/freeze mismatch, its narrow
  amendment allowlist correction and genuine Doxygen missing private-field docs.
  `-B` is now used at runner startup. Required comments pass robust token equality,
  rebuild, final20 and lint;409 is not claimed on byte-identical final comments.
- Private attempt1 expected a nonexistent static syscall archive; attempt2 reused
  copied read-only dylibs and failed overwrite. Full copied hashes/modes were
  checked before cleanup. Attempt3's executable-linked interposer failed positive
  read controls; zero-read evidence was withheld until the copied dylib was loaded.
- Private attempt4 passed all four controls but its pre-hardening two-fault variant
  remained green. The single bounded third-fault control supplies the actual final
  red. No product/test behavior changed during these harness corrections.

`attempt-notes.json`, each driver log/attempt directory and intermediate manifests
retain the exact limits and failures. None was retried into an unreported pass.

## Preservation and remaining scope

Final full membership/mode/byte/symlink comparison verifies62,767 protected entries
and8,248 installed-release/absent-release entries unchanged. The helper ownership
amendment filters only its three exact baseline paths; coordinator round records
and this round's owned prefix are excluded as documented by the snapshot helper.
`AGENTS.md` remains the `CLAUDE.md` symlink. Release was neither rebuilt nor run.

This is initial metadata admission only. No argv parser, public profile admission,
Lua invocation/slot ABI, handler/provider/read service, early load replay,
interpretation/history acceptance, completed whole-invocation shared budgets,
full/release/Linux gate, public cutover or mandatory X08 external-profile acceptance
is claimed. X06–X08, execution/RA01 and Linux stay open; public init creates format1.

Exclusive Debug and all implementation-owned files are released at final handoff.
No build/test/private process remains at that handoff, no further edits are planned,
and independent review must decide disposition against the final exact identity.
`artifact-manifest.json` binds evidence modes/sizes/hashes; `finalization.json`
binds final candidate/diff/report/artifact identities and release.
