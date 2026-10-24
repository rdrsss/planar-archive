# V5-02-X05-C3-B2 independent review

Disposition: **approve** for the bounded original-owner and initial extension-slot
metadata-admission slice. No blocking findings or requested changes.

Reviewed shared checkout `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. This review owns only this report
and `tests/native/v5-02-x05-c3-b2-review-evidence/`. Product, author evidence,
history and release files were read-only. No repairs, delegation, Planar tools,
legacy workflows, commit, installation, publication, merge or operator-data access.
Exclusive Debug verification was serialized and is released at this handoff.

## Exact candidate and authority

Read the active repository instructions, implementation/roadmap and v5 direction,
the full C3-B2 brief including its workspace distinction, pure-preflight, scoped
status/audit and comment-only amendments, X-CLI/X-CAP/X-LIMIT, and exact workspace
contract lines289–332. The review accepts no future public CLI, handler, read-service
or whole-invocation accounting behavior from this initial admission seam.

The reviewer independently recomputed every final candidate entry, every author
artifact entry and the actual-preedit diff. All identities match the handoff:

| Artifact | SHA256 |
| --- | --- |
| Final236-entry `frozen-candidate.json` | `a0ce3d520a4dc5992bd2f1a04355a0ba7400bca9a4f7818cc0bda35d84b78c8e` |
| Actual-preedit `candidate.diff` | `1467b53c526a0ed58b9b32f77ff0cf633db7fb0fee5eaabe17a289ec088ed92f` |
| Author200-entry `artifact-manifest.json` | `5e8300dcab0d3fe05f474c28bba8d97f0d869dc2017f25b149bb9319772aa11c` |
| Author `finalization.json` | `99ce568049b28e2602f9bf33d64588f3601f1e67347bd605492fc30900e693ad` |

`review-evidence/identity-verification.json` records the independent comparison.
Here and below `review-evidence/` means
`tests/native/v5-02-x05-c3-b2-review-evidence/`.

## Boundary assessment

- **Original authority and receipts.** `runtime.cppm:304` exposes a move-only owner
  with creation, healthy immutable catalog borrowing, one admission and receipt
  verification. Its private state retains the original moved `profile_capture`,
  VM, prepared result, mapping strings and normalized paths. `runtime.cpp:3915`
  obtains that capture's immutable snapshot and constructs the VM internally;
  callers cannot substitute a VM or mutable C3-A preparation. Failed creation
  publishes no owner. The private pair retains the existing receipt owner rather
  than opening a replacement capture.
- **Admission ordering and completeness.** `runtime.cpp:3947` privately prepares
  the actual sealed external command and normalizes the complete pending host
  list before entering the metadata loop. All supplied optional slots apply even
  without a later service. Workspace parent/root and selected file, source/base
  roots, registry, observation and ledger use the existing distinct native roles.
  Capability-free same-spelled options remain data. The original capture verifies
  after all admissions, including zero-host commands; subsequent verification
  uses the same owner. There is no handler/provider argument, dispatch method,
  late-content reader or ambient cwd/environment lookup. Generic profile invoke
  remains closed.
- **Physical spelling and approved names.** Admission uses `prepared_host_path`
  spelling rather than its lexical intent. Explicit symlink/parent paths cannot
  erase the symlink before metadata traversal. The named workspace environment
  deliberately normalizes lexically first, as command-contract298–324 requires;
  the resulting selected path then follows the same admission policy. Relative
  paths use the provided cwd, and root/base mappings split only the first '='.
  Original traversed edges, including exited ancestors, remain verifiable.
- **Pure helper amendment.** `host_path.cpp:131–171` shares the existing workspace
  checker and extracts path/relevant-cwd text, size and absolute validation.
  Success is nonallocating; irrelevant malformed snapshots remain ignored. The
  lexical cwd check still precedes joined-length refusal. No normalization logic,
  filesystem action or policy was duplicated into the wrapper.
- **Terminal lifecycle and diagnostics.** First refusal poisons the wrapper;
  admission retry after success also closes it. Moves transfer the complete
  state and moved-from calls refuse. Failed catalog borrowing returns empty.
  `runtime.cpp:3934` adopts an already published VM first failure before any
  fallible return copy, retaining original code/message/limit/maximum/field.
  Controlled allocator retries and moves preserve the first diagnostic, including
  the published preparation failure. No persistent exhausted-host-heap guarantee
  is inferred.
- **Cumulative path accounting.** The wrapper precharges its owned implementation,
  retained path-vector storage, workspace parent and each allocating helper pass.
  Preparation's original vector/string charges remain in force. I checked the
  archived LLVM23.1.4 path implementation: reserve32 component pairs, doubling
  vector capacities, output reservation, same-char copies and string growth.
  The24-byte component layout is compiled into the private check; archived path,
  vector and string headers exactly match the installed pinned headers. The
  conservative `4*max(32,B+1)` component allowance and separate `16*(B+1)` plus
  eight-string allowance cover cumulative helper requests. Relative cwd and joined
  passes are separately reserved. Relevant snapshot lengths participate, products
  and additions are bounded before allocation, and no refund/reset is introduced.
  Capture subsystems retain their separate bounded accounting; shared capture/VM
  accounting remains X06 and is not claimed as delivered or as RSS accounting.

## Independent verification

All results below use the final candidate, with zero failing/error/skipped cases.
Raw commands for private compilation/link/run are in
`review-evidence/private-probes/commands.json`; other exact commands are recorded
in `review-evidence/commands.json`.

| Check | Independent result |
| --- | --- |
| CTest owner/path/profile/admission selection |87/87,5.63s; `focused.xml` and `focused.log` |
| Copied-production private controls |7/7,2.05s; four existing controls independently rerun plus three reviewer-authored adversarial cases |
| Current inventory |743 names; all723 preedit names preserved;20 additions |
| Comment-only correction |35,742 identical non-comment libclang kind/spelling tokens; token SHA `6505bef1eb8ea3b69fe3873894a696bd3f8cc9751ab950010fc4821ff5e39b47` |
| Current source-boundary audit |Pass; original B2-A lookup invariants retained, plus new owner/preflight invariants |
| Pinned formatting/Doxygen |Pass over118 active C++ sources |
| Ordinary all+planar_tests build |Pass; Ninja reports no work |

The CTest selection was
`ctest --test-dir build/rewrite-debug --output-on-failure -R '(C3 B2|host path|profile capture|capture admission)'`
with an explicit review-local JUnit output. It covers owner lifecycle, all supplied
slots, overlap/aliases, strict parent traversal, original receipt mutation,
preflight precedence and real cumulative reservation refusal. This was a focused
review run, not the complete suite or a repeat of the author's409-case gate.

`adversarial.inc` adds checks for every one-shot allocation position in a terminal
retry, first-equals mapping with a later '=' in the physical path, exited workspace
ancestor replacement, and malformed later workspace environment taking precedence
over an earlier physical-link refusal. Private reused controls independently
observe all87 actual scalar-new admission positions, creation first/middle/final
requests out of6097, published preparation first-error double/triple faults, and
zero observed workspace/registry/observation/ledger data reads with positive read
controls. These are controlled allocator/inode-read observations, not exhaustive
aligned/Lua allocation interception or natural host exhaustion.

Private compilation expands response files, redirects object/PCM/dependency
outputs, supplies owned explicit module copies and disables implicit modules/maps.
All888 compiled object/PCM/archive/dylib dependencies, including external inputs,
were copied with modes/hashes and checked unchanged in both locations. The private
fixture links copied production runtime objects; no product implementation or
public API is modified. Disposable compiled outputs/copies were removed only after
comparison; sources, commands and raw results remain.

## Author evidence and amended gate assessment

The raw author XML independently confirms409/409 focused cases before the authorized
Doxygen comments and20/20 final-source new cases afterward. The original tested
freeze remains separate from the final freeze. Independent libclang token equality
confirms that only comments changed executable source between those runs. I also
checked the two final status corrections and the source-audit amendment against the
actual-preedit diff.

The old B2-A script's textual endpoint includes later preparation code and its
`function_reference`; its raw failure and original script remain. The replacement
uses the actual shape-lookup function boundary and preserves every original
forbidden/required invariant, then separately audits the owner. No original
invariant is silently dropped.

The author preserves failed baseline/fixture/edit/cache/lint/private-link/interposer
attempts. Failed positive read controls are not claimed as successful zero-read
proof. The final reconstructed pre-hardening triple-fault raw XML fails exactly
on `profile` versus `catalog.commands`; the earlier two-fault green is disclosed.
This is a controlled post-audit red/green reconstruction, not a claim that the red
ran chronologically before hardening or that natural exhaustion occurred. Author
full/release claims remain absent; six vectors and six final audits are supporting
scoped evidence, not a new full milestone gate.

## Preservation, release and remaining scope

Independent before/after membership, mode, byte and symlink manifests protect
71,689 entries, including candidate, historical/author evidence and installed
release entries. They record rewrite-release's continued absence and preserve
`AGENTS.md -> CLAUDE.md`. `preservation.json` records zero changes outside the
assigned review report/prefix. The author236-entry candidate and200-entry evidence
manifest remain exact. Review evidence is bound by its own `artifact-manifest.json`
and `finalization.json`, including this report's final hash.

No build/test/private process remains. Exclusive Debug and both reviewer-owned
paths are released; no further edits are planned. Approval covers original owner
pairing and initial metadata admission only. Receipt-bound read services, actual
handler/Lua invocation ABI, public final parsing/profile admission, early shipped
load replay integration, X06 whole-invocation accounting/history/acceptance,
X07–X08, mandatory external-profile acceptance, execution/RA01 and Linux remain
open. Public init still creates format1; latest approved full gate remains676 per
configuration. No full/release/Linux gate is inferred from this review.
