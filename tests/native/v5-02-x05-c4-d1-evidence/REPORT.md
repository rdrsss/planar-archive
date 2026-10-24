# X05-C4-D1 original admitted root descriptor binding

Candidate for fresh independent review; the coder does not approve this work.
Shared `dev/planar-v2-vernacular` at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, intentional dirty rewrite.
The coordinator owns the round brief. This candidate owns only nine admission/
native/profile source and test files, seven current reference/guide files, and
new D1 support/evidence. No runtime, commands, Lua, build configuration, historical
adapters or protected evidence was edited. No Planar tools, delegation, commit,
installation, publication, merge or operator-data access occurred.

## Behavior

`capture_admission::verify_root(slot, descriptor)` first checks every original
receipt and relationship, then accepts only an originally admitted present
supplied-directory slot. The opened descriptor must match the original device,
inode and mode using the existing directory comparison. Wrong/unknown/trusted/
file/target slots, invalid descriptors, changed ancestors/names/modes and moved
owners refuse `capture_changed`; an original failed owner remains poisoned.
Unrelated sibling additions do not invalidate directory receipts.

The new four-argument `native_capture_session::add_root` opens the actual absolute
physical spelling with no-follow directory traversal, then binds it to that
original receipt before overlap checks or vector publication. A bound-root-only
mode permits real `..` edges and is retained for later native root reopening.
Dot/repeated separator/root parents work; missing, symlink and regular-file
intermediates cannot disappear through lexical cancellation. The original
unbound overload keeps its existing parent refusal and overlap behavior.
Failed binding releases temporary descriptors and publishes no candidate root.
Its new allocation boundary returns typed native resource refusal.

`profile_capture::verify_root` and `bind_root(session, logical, directory, slot)`
forward only through the original private admission. There is no mutable admission
accessor or provider callback. The wrapper marks itself terminal before native
work and clears that flag only on success, so native overlap, opening, binding or
allocation refusal closes the owner. Existing immutable closure snapshots remain
valid and unchanged; original content/metadata receipt verification remains
mandatory. This does not add supplied-file reading or a role service.

## Verification

The final frozen gate is recorded by `gate-results.json`, `verification-summary.json`,
`focused.xml`, `private/probe.xml`, and the individual command logs. The gate asserts:

| Check | Result required and obtained at finalization |
| --- | --- |
| Inventory | 812 names; all 800 pre-edit names preserved; 12 additions |
| Focused Debug CTest | 145/145, zero failures/errors/skips/disabled |
| Private production probes | 3/3 native syscall/allocation cases, zero failures/errors/skips |
| Exact vectors | Six generators pass their applicable check mode |
| Static boundaries | Five historical audits, approved current C4-C2 runtime audit, new D1 audit |
| Pinned format/Doxygen | 118 active sources pass |
| Final ordinary build | `all planar_tests`; Ninja reports no work |

The focused selection contains every case in admission, native capture, profile,
profile_codec, host_path and generic capture (128), plus all 17 C3-B2 original-owner
cases. It is a scoped gate, not a rerun of the previous 439 runtime/history cases
or the historical full suite. The development 83-case run predates the final
host-text/root-parent additions and is not substituted for final evidence.

Real per-test scratch fixtures cover correct/wrong directory descriptors, files,
invalid descriptors, trusted/file/unknown/existing-target/absent-target slots,
moves, leaf/ancestor replacement, directory modes, unrelated siblings and exited
parent branches. Native tests cover repeated root-parent/dot/trailing/repeated
components, non-cancelable missing/link/file intermediates, absolute/text checks,
original mismatch priority over duplicate overlap, no publication and existing
physical overlap policy. Profile cases cover moved authority, terminal failures,
original closure lifetime and original trusted receipt refusal.

The private macOS dynamic syscall adapter observes actual `read`, `pread`,
`readv`, `mmap`, `readdir`, `fdopendir` and `opendir` calls during binding. Both
successful and mismatched binding perform zero observed data/enum calls. A real
subsequent file read and a real tree enumeration independently make the relevant
counters positive. A separate one-shot global allocation sweep refuses each
successive actual binding allocation until the first healthy run; every failed
point returns resource_limit, closes the profile owner, preserves original
snapshot bytes, leaves descriptor count unchanged and permits the session's
previously unbound logical root to be registered afterward. These are controlled
allocation probes, not a natural exhaustion or persistent exhausted-heap claim.

Private compilation recursively expands response files, disables implicit modules
and maps, and redirects every explicit compiled dependency to an owned copy.
All shared objects/PCMs/archives/dylibs plus external explicit module inputs and
LLVM libc++ dylibs are copied. Complete membership, modes, sizes, hashes and mtimes
are compared before/after execution, along with shared Ninja journals. The private
runner retains exact argv, sources, XML, dependency manifests and journal proofs.
Only owned, verified dependency copies/products are removed; removal inventories
and cleanup records remain. Ordinary build commands are fully awaited and
serialized; no private process overlaps a shared build.

## Preserved attempts and exact identity

`baseline.json`, source preimages and `inventory-before.json` were captured before
product edits. The baseline protects unrelated sources, vendor/build tooling,
fixtures, historical tests/evidence, docs, the four repaired adapters and installed
release. `build/rewrite-release` remains absent. Final protection excludes only
explicit owned files and the coordinator's D1 brief; it records zero other changes.
`AGENTS.md` remains the `CLAUDE.md` symlink.

Private attempt1's executable-local read interposition missed the real read
positive control. Attempt2 then failed compilation on its obsolete unused read
counter. Attempt3's established read dylib positive control passed, but its
executable-local enumeration positive control failed. All failed logs/XML,
commands, source snapshots where captured, and verified cleanup remain under
`development-private-attempt*` and `attempt*-*`. The first source was reconstructed
by reversing the exact watcher edits; its reconstruction is explicitly labeled,
and is not claimed to be a pre-failure snapshot. Final evidence uses a separate
owned dynamic adapter for all seven calls and requires both positive controls.
No failed harness attempt is counted as passing evidence.

The first frozen gate passed all145 focused cases and six vectors, then an
inapplicable C3-B2-era source audit rejected the already-approved later runtime's
Lua service code. That audit selected the entire remainder of runtime.cpp and
asserted registration-only assumptions superseded by C4. The failed command,
source/support snapshot and candidate remain in `attempt1-frozen-gate/`. The final
gate instead runs the approved current C4-C2 runtime audit plus the new exact D1
boundary audit; no historical helper or runtime byte changed. The final syscall
adapter compilation also explicitly disables implicit modules/maps, matching the
probe compilation. All final product/test/support/report bytes are freshly frozen.

`frozen-candidate.json` binds exact final product/test/prose/support bytes;
`candidate.diff` is against actual pre-edit source copies, not Git's historical
legacy baseline. `artifact-manifest.json` binds all retained evidence. The final
`finalization.json` binds those manifests, this report and Debug release for fresh
review. No further coder writes or live build/test/private processes remain at
handoff.

## Limits

This is internal metadata-only root binding. No enumeration/content capture at
binding, discovered-file facade, handler/role context, Lua execution, public CLI,
profile-ledger interpretation/acceptance, whole-invocation accounting or public
cutover is delivered. Existing capture accounting stays separate from the VM;
this is not RSS accounting. Original cooperative checks do not prevent hostile
swapping or change-and-revert. Latest full independently approved gate remains
676/configuration; no new full, release or Linux run is claimed. X06–X08,
mandatory external-profile acceptance, execution/RA01 remain open. Public init
continues to create format1. Independent approval is pending.
