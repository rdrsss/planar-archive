# X05-C4-D2-A retained original supplied-file authority

Candidate for fresh independent review; not self-approved. Shared checkout at
HEAD3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a, branch
dev/planar-v2-vernacular. Coordinator owns the round record. The exact owned
candidate is frozen in `frozen-candidate.json`, copied under `candidate-source/`,
and compared to actual pre-edit bytes by `candidate.diff`. `artifact-manifest.json`
and `finalization.json` bind the completed evidence. Historical evidence is retained.

## Original authority and transitions

`capture_admission` remains move-only; moved-from wrappers refuse. Its private
implementation now has shared ownership. A move-only `retained_file_receipt`
retains that same state plus an immutable append-only selection index. No copied
mutable facts, host selector getter, profile pointer, reconstructed admission,
provider, filesystem discovery, content owner or ownership cycle is introduced.
`admit_file` is unchanged. `retain_file(slot)` verifies original receipts and
relationships, admits only an already admitted supplied-file selection and charges
each issuance; trusted/root/target/unknown/failed keys refuse and poison.

Native retained reads store the successful original receipt. Every later content
path verifies the actual opened FD against that receipt before the content buffer
or data syscall: plain cached reads, retained repeats, existing tree visits,
finish and acceptance rereads. Global original relationships, metadata and every
retained name edge remain authoritative. Token metadata checks precede inherited
content ceilings. Original content is compared exactly on reuse and reread.

The coordinator approved these transitions before broad edits: new entry plus
retained token becomes owned; an owned repeat requires the same original state
and selection; plain read uses stored authority; either borrowed overload on an
owned entry refuses; unbound-to-owned conversion and replacing owned authority
refuse. Original owned and proposed incoming authorities are poisoned on mixed-mode
refusal. Original borrowed/unbound entries preserve their prior behavior. Tree
visits protect existing owned entries; new bound discovery/load/reference wiring
belongs to D2-B. Old unbound discovery remains the existing native API.

Operation guards cover preflight/open/read/copy/publication/verification failures.
They poison original authority before fallible error adaptation; normal work is
not pre-poisoned. Failed retained reads publish no file, token or partial bytes.
The native map insertion commits its content-byte counter only after insertion.
Profile forwarding obtains tokens through its private original admission and
propagates terminal failures. `healthy()` and profile snapshot observe shared
poison without filesystem work. Trusted profile content sealing remains separate.
Native tokens keep original metadata descriptors after profile/admission wrapper
moves/destruction; acceptance native moves preserve receipts.

## Bounds and layouts

Existing ceilings remain separate: admission 100000 metadata receipts/selections,
10000 files and 64 MiB cumulatively charged metadata; native 10000 files, 64 MiB
retained content, and existing 100000 tree-entry accounting. No content-byte quota
is repurposed as a metadata ceiling, new quota or whole-invocation/RSS metric.

On pinned arm64 libc++, compiler layout logs measure admission implementation80,
retained native record176 (previously144), token24, optional token32 and admission
wrapper16 bytes. The private allocation control measures make_shared's first
allocation104 bytes: implementation80 plus control24. Factory charging reserves
sizeof(implementation)+128 =208 bytes, conservatively covering that allocation and
the wrapper. Each token issuance charges sizeof(token)+sizeof(optional token)=56
before issuance, covering its handle and potential native inline storage. Handles
move without allocation and retain no slot strings or new descriptors. Reissuance
charges cumulatively without refunds or an unbounded handle cache. Native's inline
32-byte record addition is bounded by 10000 entries (320000 bytes), independently
of the unchanged content budget. Transient stack frames are finite native call
frames, not claimed as allocator/RSS accounting.

## Verification and preserved development attempts

The baseline protection/preimages completed before product edits and recorded
812 tests. Final inventory is826, preserving every original name and adding14
colocated cases (admission4, native6, profile4). The focused Debug gate selects159
cases across capture/admission/native/profile/codec/host-path and C3-B2 original
owner coverage. Its raw XML, selected names, command results and LastTest log are
retained. No failures/errors/skips/disabled cases are allowed in that selection.

Three copied-dependency private cases exercise actual interposed production data
and metadata syscalls, with positive read controls. They cover FD-time mode
mutation after open, trusted hardlink substitution, ordinary/retained/bounded cache
refusal, original ancestor replacement, beyond-quota growth, mutation at the seal
hook and acceptance reread. All refused content routes have zero data reads;
positive initial/cache/seal/acceptance paths do read. Snapshot succeeds or observes
late shared poison with zero intercepted metadata/data operations. Scope teardown
returns descriptor counts to baseline, including surviving tokens and failed paths.

Exhaustive one-shot allocator refusal sweeps run until the first successful
initial read and cached read, retaining per-index requested size/read counts.
They exercise failures before content and after content, including result copying,
identity staging and file publication. Every failure preserves original immutable
profile bytes, poisons the profile, closes temporary descriptors and publishes no
new file. The non-SSO fixture exercises data/result allocations; successful controls
prove that the sweep actually reaches the production operation. Raw XML contains
the measured stage counts rather than relying on an arbitrary minimum.

A separately compiled preserved pre-edit native implementation reproduces the
historical borrowed-cache gap as exactly one expected assertion failure
(`baseline.xml`, exit42). This motivates explicit retained mode; it is not a claim
that the old borrowed API changed. Current retained cases must all pass.

The first private attempt passed behavioral cases but failed an arbitrary
allocation-count expectation (SSO yielded3, expected at least4). Its source, raw
XML, command/dependency manifests and verified cleanup are retained under attempt1.
The fixture was expanded and the expectation replaced by observed pre/post-content
stages. Passing intermediate attempts2/3 and their verified cleanup are retained.
Two early development builds overlapped development edits; their logs remain and
are not final consistency evidence. The frozen gate therefore performs one
serialized clean-first Debug all+planar_tests rebuild with fixed source/test/build
inputs before tests, audits and private compilation. No product edits overlap it.
The final no-change all+planar_tests build must run zero work.

The first frozen gate passed clean compilation,159 focused cases, all vectors/
audits/lint, private tests/cleanup and no-work, but its final protection helper
still excluded the historical uppercase D1 coordinator round. It therefore refused
the authorized D2-A coordinator-round edit. `attempt1-frozen-gate/` preserves its
complete output, frozen identity and support sources. Only the exact coordinator
round exclusion and this report were corrected; product/tests remained identical.
The candidate was refrozen and its gate rerun.

Six applicable vectors, seven source/runtime audits, pinned formatting and Doxygen
(118 active sources) are included. Private compilation recursively expands all
response files; build PCM/object/archive dependencies are copied and redirected,
with implicit module creation disabled. LLVM libc++/libc++abi real dylib files are
physically copied and their original hashes/mtimes protected, but resolved-path
copying omits libc++.dylib/libc++.1.dylib and corresponding ABI aliases. Actual
copied-runtime link/load selection and the macOS LC_LOAD_DYLIB chain were not
proved; execution may use original platform runtimes, consistent with the earlier
B0 platform-runtime limitation. No copied-runtime execution is claimed. Original/
copied membership, modes, full hashes, mtimes and Ninja journal contents are
checked before/after private compilation. Original state remains untouched.
Only owned verified disposable dependencies/products are removed, retaining source,
commands/XML/hashes and removal inventory. Python runs with -B.

Pre-edit/final protected inventories compare hash/mode for unrelated sources,
historical evidence, build configuration and install-release. Rewrite-release
remains absent; no release-mtime snapshot is claimed. Coordinator round is the
only additional allowed document mutation. Debug is released after finalization
for fresh independent review. No commit/install/publish/merge/operator-data action
occurred. No full test, release, Linux, role service, new bound discovery, public
CLI, profile-ledger acceptance, X06–X08 or mandatory external-profile completion
is claimed. Latest full independent approval remains676/configuration.

## Final bookkeeping identity

The redundant second frozen gate was dispatched immediately before the
coordinator's instruction to correct protection/bookkeeping only arrived. It was
allowed to finish without intervention; all159 focused and3 private cases,
vectors/audits/lint, cleanup/no-work/protection passed again. This repeated gate
was unnecessary for an exclusion/report-only fix. Product, tests and build inputs
stayed unchanged through both clean builds and both gates.

After that process completed, only this report and the finalizer changed. The
finalizer now parses the protection helper's actual exclusion assignment and
requires exactly the current D2-A coordinator round, explicitly retaining D1
protection. `tested-frozen-candidate.json`, `tested-REPORT.md` and `tested-support/`
preserve the second gate's exact support/report identity. `bookkeeping-identity.json`
compares that identity with final source/test/prose files and the first gate;
product/test/build inputs are identical. No further rebuild or test was run for
these bookkeeping changes. Final protection and identity bookkeeping alone were
repeated before finalization. The final artifact manifest covers both tested and
final support/report identities.

Before finalization, read-only inspection identified that inherited runtime-copy
alias limitation. Only this report/finalization metadata were corrected; no new
behavioral rerun was required. Exact commands, dependency manifests, copied-mode/
hash/mtime checks and cleanup evidence remain preserved. Review can independently
reconstruct the actual runtime link/load chain if needed.
