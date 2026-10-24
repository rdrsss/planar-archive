# R14-C2 independent review

Verdict: **approve**. No blocking findings or requested changes.

Reviewed the shared checkout on `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, without product edits. The reviewed
five-file candidate is exactly composite SHA256
`41b4ef54273ae2c1a3c0efc956563d4809748e71e4f6ed6fb526225258234652`.
The coder's `evidence.json` file SHA256 is
`3b319f50b6163e53b368cf5fbb38bb48352d4895834c63372023fcd293cfeb33`;
this is the hash of that manifest file, distinct from its candidate identity field.
Independent checks validated all five live/frozen file hashes, the composite
identity encoding, all 59 manifest-listed raw hashes, and 12,238 protected hashes.
No unexpected nonplanning files were found after allowing this review's owned
folder. The instruction symlink remains preserved.

## Contract and implementation assessment

- `capture.cppm:80` exposes a non-default-constructible, noncopyable capability
  with private construction and a sole session factory. Ownership transfers the
  original session implementation, rather than reopening substitute receipts
  (`capture.cpp:478`). Snapshot/topology access returns the original immutable
  shared values. Moved-from methods refuse safely, and transferred session paths
  guard their null implementation. Move assignment releases previous ownership;
  self-move preserves it.
- Native verification reopens each root through the existing no-link path walk,
  compares retained root identity, re-enumerates registered tree membership and
  directory identities, then rereads retained bytes and compares file identity
  and normalized mode (`capture.cpp:298`). Separate input parents remain retained
  roots; their unrelated membership is not traversed. Existing exclusions remain
  in force. The contract intentionally compares 0644/0755 normalization between
  checks; readable raw chmod changes with unchanged normalization are allowed.
- Postseal verification runs no Lua or callback. The sealing callback is cleared
  before transfer (`capture.cpp:483`), and sealed guards prevent alias-map writes
  in both native discovery helpers (`capture.cpp:158`, `capture.cpp:266`). Local
  verification maps, budgets and fresh bytes are discarded. Success and failure
  preserve the published values and original receipts; retries can succeed after
  restoration. Detected postseal refusals map to fixed, host-path-free
  `capture_changed` (`capture.cpp:498`).
- Existing `finish()` retains its initial seam and diagnostic mapping
  (`capture.cpp:434`). The extraction preserves the original root/tree/file
  comparison algorithms. All sixteen old capture cases are an exact source
  prefix and passed independently. CLI247 passed unchanged. The fdopendir
  adjustment correctly retains descriptor ownership until DIR adoption succeeds
  (`capture.cpp:239`). Expected diagnostic failures and nonrecoverable exceptions
  unwind through descriptor/DIR ownership.
- Architecture/testing references describe the implemented boundary and its
  limits accurately. No writer is introduced. Possession of this capability is
  not proof of a current check: the later writer must require it and invoke
  native verification immediately before every commit attempt. That integration
  remains a mandatory later acceptance criterion.

## Independent verification

Exact argv, cwd, exit codes and raw output are in
`tests/native/r14c2-review-evidence/{commands,probe-commands,final-commands}.json`.
All executed verification commands exited zero; no test skips were observed.

| Check | Independent result |
| --- | --- |
| Debug `all planar_tests` build | Zero steps; candidate already current |
| `replacement.capture` | 21/21 cases, zero skips; original16 plus additive5 |
| Ledger/canonical/hash/record/tree/provenance/interpretation focus | 151/151 cases, zero skips |
| `replacement.cli` verbose | One CTest entry; 247 real processes, zero skips |
| Both vector generators `--check` | Pass |
| Active formatting/Doxygen | 64 source files, pass |
| Four existing C++ verification helpers, pinned formatter | Pass, zero warnings |
| Mechanical R12-C2/R13-B1 generation | Exact configured outputs; reverse-identical manifests |
| Final debug `all planar_tests` build | `ninja: no work to do.` |
| Reviewer-only exact-library adversarial probes | 830 checks, pass |

The reviewer-only harness links against the existing exact candidate static
capture library and module artifacts without editing product or CMake files.
It independently rejects replacement of a nested directory even when the original
captured file inode is moved into the replacement, nested directory/file links,
new hardlink membership, and symlink replacement of an ancestor of the workspace
or standalone-registry parent. Repeated failures preserve original snapshot and
topology pointers and bytes.

The harness directly interposes `fdopendir` at link time and confirms 100 actual
injected enumeration failures. Every refusal returns `capture_changed`; root fd
counts remain stable and verification succeeds after the fault is disabled.
Forty allocation-failure positions during repeated verification preserve values,
release transient descriptors, and allow successful retry. Sixty allocation-failure
positions during initial sealing produce eleven caught `bad_alloc` exceptions;
all owner lifetimes close their roots. A throwing initial callback propagates its
original exception and releases both closure and root when its session dies.
Existing candidate tests independently exercise capability/session moves, closure
release after successful sealing, root identity counts, retry restoration,
exclusions, normalized modes and immutable values. Compiled reviewer probe/object
hashes are recorded; the reproducible build products were removed afterward.

The coder's prior API-absence replay was inspected and hash-validated: it fails
against baseline capture sources because the new API does not exist. Earlier
Doxygen failure and corrected final rerun remain preserved; the final reviewed
candidate passes independently. This is evidence for the added API, not a claim
that the baseline implemented postseal checks incorrectly.

## Limits and handoff

Executed on macOS arm64 debug only. Linux, release, and the complete R14 milestone
gates remain outstanding. Fault injection covers the stated deterministic failure
positions, not every possible platform or allocator failure. Capture is cooperative:
it neither prevents writes after comparison nor detects every change-and-revert.
No operator-state access, Planar tools, commits, installation, publication or merge
occurred. Product candidate stayed unchanged throughout review. Reviewer ownership
was limited to this report and `tests/native/r14c2-review-evidence/`.

The debug build directory is released to the coordinator after this report and
its evidence manifest are frozen. Completed reviewer is retired from dispatch.
