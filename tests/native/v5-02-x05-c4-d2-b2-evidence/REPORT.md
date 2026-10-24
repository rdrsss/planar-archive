# X05-C4-D2-B2 implementation evidence

Candidate: internal rooted original-authority discovery. Independent review pending.
Shared checkout dev/planar-v2-vernacular, HEAD 3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a;
pre-existing rewrite changes preserved. No commits, install, release, publication,
merge, Planar tooling or operator-data access. Exclusive serialized Debug only.

## Implementation and boundaries

`capture_admission::retain_root` issues the same shared original admission and
immutable supplied-root index. It admits only original supplied roots. The new
move-only root token verifies original complete receipts and the actual root FD.
It discovers only `valid_path` portable relative files, opening each physical
component from the retained original root without links or parent traversal.
Metadata-only descriptors and named/opened comparisons precede file authority
issuance. Declaration separation rejects selected trusted hardlink aliases.

The root/path index returns the original selection on every repeat, so changed
metadata refuses without recapture. Before new publication it maps existing
explicit supplied-file name/parent identities back to their original selection,
including files admitted before a native cache entry exists. New selections use
a separate private kind/namespace and decimal append-only selection ordinals
(maximum five digits under 100000 selections); caller keys cannot collide with
that namespace. Public retain_file cannot issue private discoveries by key.

Original shared implementation size is charged at creation. Every root handle
and possible native optional is charged before issuance. Discovery charges
handle/optional, 256 bytes plus twice the <=1024-byte path for staging/index,
512 bytes plus original chain storage for a new selection, then component,
chain and original receipt allowances before allocation/opening. Issuance and
temporary work charge cumulatively without refunds. ASCII case comparison
allocates no temporary folded paths. Existing 64 MiB metadata/100000 receipts,
10000 file-selection and native separate content/tree ceilings remain distinct;
these are charged bounds, without process RSS or X06 shared invocation claims.

Native roots retain the token; read and enumeration verify it before use.
Ordinary initial reads discover and move the original file token into the entry.
Explicit owned file entries retain their selection, and borrowed modes refuse a
bound-root bypass. Existing caches cannot acquire root authority by registration:
duplicate binding fails terminally. Existing unbound and standalone APIs retain
their previous policies. Final verification checks original root metadata,
compares tree membership/topology before file rereads, and never calls discovery
or reads newly appeared files. Original tokens survive profile/admission wrapper
moves/destruction and native acceptance moves without cycles or profile pointers.

The profile facade binds roots through its private original admission and exposes
a bounded root-capability factory. `capture_session` forwards only that native
capability. Its ordinary load/reference flow is unchanged. The dependency remains
capture -> native and Lua declaration loader; profile -> native. No profile import,
runtime edge, provider, callback authority or role service is introduced.
Failures poison shared admission and bound native ownership before error adaptation.
All opened descriptors use RAII; partial failed admission/index publication remains
terminal and cannot be reused as a replacement authority.

## Verification

Final frozen raw results are recorded in gate-results.json, focused.xml and
private/commands.json. No full/release/Linux or mandatory external-profile gate
is claimed; latest approved full gate remains 676/configuration.

The development focused pass ran 183/183 in 42.89 seconds; inventory 850 preserves
all 838 baseline names, adding 12 B2 cases. All six capture-related binaries and
existing C3-B2 selection are included. Final frozen verification repeats this
selection after the allocation-free case-comparison refinement and count-boundary fix.

Preserved failures: development-new.log/XML records one failed path-limit fixture.
Its std::ofstream absolute host spelling exceeded macOS PATH_MAX and created no
file; no product defect was found. attempt1-admission.t.cpp preserves that source.
The corrected fixture builds a legal 1024-byte relative file through openat/mkdirat;
development-new2 passes 12/12. development-focused.log preserves an invocation that
failed to locate a relative --tests-from-file path before running any tests; it is
not a successful zero-test result. development-focused2 uses the absolute file and
passes 183/183. No failure is counted as success; no before-fix product regression
claim is made for these harness corrections.

Private probes count actual production read/pread/readv/mmap and directory
operations with both positive read/tree controls. They cover metadata discovery,
repeats, trusted hardlinks/links, actual opened-leaf mode changes, root mode changes,
final added/removed membership, explicit selection precedence and cached mutation.
One-shot allocation sweeps exercise direct discovery, native read publication and
root registration, terminal state and FD closure after complete owner destruction.
Controlled failures do not claim natural or persistent exhausted-heap behavior.

Private compilation recursively expands response files, disables implicit modules,
copies complete compiled object/PCM/archive/dylib inputs, redirects compile/link
inputs/outputs, and verifies shared/copied membership, modes, bytes, hashes and
mtimes plus both Ninja journals before/after. Actual copied runtime loading is not
proved or claimed; linkage selection is documented by otool only. Only verified
owned disposable dependencies/products are removed after proof.

Protection freezes all source/vendor/tooling/fixtures/prior evidence and both
release paths except named owned files and the exact coordinator B2 round. B1 and
older rounds remain protected. AGENTS.md remains the CLAUDE.md symlink.
Rewrite-release is absent; protected install-release is never invoked.
Final manifests bind the exact candidate and all retained evidence.

## File-count correction chronology

The complete first frozen gate passed and is preserved in first-frozen-gate/,
including its candidate snapshot, diff/source copies, report, focused/private
results and protection. Final inspection found that the new discovery code checked
selected-file count before mapping an explicitly admitted original selection.
A full count would wrongly refuse reuse, even though no new file slot was needed.

quota-before-fix-admission.cpp preserves actual product before correction.
quota-regression/controlled-before.cpp changes only root issuance for private slot
quota to set the existing files counter to max_files. This controlled copied
production seam avoids claiming a real 10000-file fixture. The exact commands,
source/compiled dependencies and Ninja journals are retained. The before run exits 42
with exactly one failing original-selection-reuse case and one passing exact numeric
key collision case. This failure preceded the product fix.

The product moves the selected-file/selection-count ceiling check after original
explicit selection reuse and before retaining a genuinely new leaf selection.
Reuse still incurs its original handle, path/index and metadata staging charges;
it does not consume a second supplied-file count. The identically controlled
corrected copy passes 2/2: original reuse/repeat succeeds at the full count, a new
selection refuses resource_limit and poisons, and a caller key exactly equal to
private ordinal 4 cannot replace or collide with the private selection.

quota-regression/controlled-after.cpp, before.xml/after.xml, commands and
before/after dependency/journal proofs preserve the regression and fix. Only
verified owned copied dependencies and compiled products were removed, with
quota-removal-inventory.json and quota-cleanup.json. Final ordinary Debug build,
184-case focused selection, six normal private cases, boundary audits, pinned lint
and no-work check execute after this correction; inventory remains 850/838 preserved.

## Caller-slot namespace correction chronology

second-frozen-gate/ preserves the complete passing 183/6 candidate after the count
correction. A subsequent deliberate public-slot audit found verify_file accepted
private discovered kinds during string-key lookup. If a caller slot was admitted
after an equal private ordinal, the public operation selected the private record
first; without a caller slot, guessing that ordinal could verify the private file.

namespace-before-fix-admission.cpp and namespace-regression-admission.t.cpp preserve
the exact source/test before the fix. namespace-red-build.log and namespace-red.log/
XML demonstrate the actual ordinary Debug regression before correction: one case
fails with three assertions. It covers equal numeric caller keys both before and
after discovery, plus guessed private-key refusal. The public-before-private
control passes; the after-order and guess assertions expose the defect.

verify_file now excludes discovered_file from caller-slot lookup. The deliberate
complete boundary audit checks every string-slot path: key uniqueness ignores the
private namespace, verify_root and retain_root require supplied_root, retain_file
requires supplied_file, and verify_file excludes private discoveries. Owned root
and file tokens still use immutable indices, with no string lookup or replacement.
The same final ordinary regression passes; final inventory is 851 with all 838 names
preserved, and focused 184 includes all 13 new B2 cases. The final frozen gate uses
this candidate. Both earlier passing gates retain their own identities and scope.
