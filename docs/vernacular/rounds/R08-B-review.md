# R08-B independent native CLI review

Disposition: **request-changes**. One reproducible P2 input-routing defect blocks
approval. Product files were read-only throughout this review.

Reviewed the shared `dev/planar-v2-vernacular` checkout at starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The frozen 25-file candidate manifest
`tests/native/r08b-evidence/candidate.sha256` has SHA256
`c8171f623a8426e377ffa33e8e03fa62a4eb32306c63fd3343142e1ff28f4889`.
All entries matched before and after review, as did the coder evidence manifest
and all 20 prior R08-A-C01 approved entries. Existing unrelated changes were
preserved. No Planar tools, commits, installation, publication or delegation.

## Finding — P2: canonicalize host paths without changing their referent

`src/cmd/planar/bootstrap.cpp:72`–`74`, `host_file()`, calls
`absolute(...).lexically_normal()` before canonicalizing the parent. Removing
`..` lexically before resolving a preceding symlink changes normal filesystem
path semantics. The contract requires resolving relative host paths from cwd
and then canonicalizing before capture (`render-contract.md:31`–`32`). The
helper is used by workspace, registry, observations, current-root and base-root
routing, so the issue can select different declaration/input/source bytes while
returning success.

The independent reproducer is
`tests/native/r08b-review-evidence/canonical-path.py`; raw results are in
`canonical-path.log`. In its scratch layout:

- `fixture/hop` is a symlink to `external/subdir`.
- `fixture/current` and `external/current` contain different CLI source bytes.
- Both normal file reads and `Path.resolve()` resolve `hop/../current` to
  `external/current`.
- `--root source=hop/../current` exits 0 but captures `fixture/current`. Its
  observation digest is `xxh3_128:0e3a8cc3ccfddc9884b7b0b4955e8a9c`, identical to
  `--root source=current`. Passing the actual canonical external path gives
  `xxh3_128:ce1509621d599a20a467bceb6aaed9c1` and captures the external marker
  bytes. The log records exact retained source bytes and render/work digests.

The same helper gives inconsistent final-root symlink refusal:
`--root source=root-link` returns `path_denied`, while `root-link/`,
`root-link/.` and `root-link/src/..` all exit 0. See `path-spelling.log`.
This bypasses the final-root symlink rejection already exercised by the
candidate acceptance test. Canonicalization/refusal should preserve one rule
across equivalent directory spellings.

Required correction: preserve filesystem resolution semantics for explicit host
paths, and apply the intended final-root link refusal consistently. Add native
CLI regressions for the demonstrated symlink/parent routing case and equivalent
root spellings; demonstrate failure on this frozen candidate before fixing.
The lower-level approved capture/runtime policy need not be changed to address
the command helper. Start a fresh corrective round under the repository protocol.

## Verification and review scope

Independent commands, serialized per build directory:

- `cmake --build --preset debug --target all planar_tests`: success, **ninja: no
  work to do** (`tests/native/r08b-review-evidence/build.log`).
- `ctest --test-dir build/rewrite-debug -R 'replacement.(json|help|missing explicit|legacy commands|cli|render|runtime|schema|semantic|selection)' --output-on-failure --output-junit <repo>/tests/native/r08b-review-evidence/ctest.xml`: **83 tests**, 82 unit and 1 acceptance, zero failures/errors/skips (`ctest.log`, `ctest.xml`). The acceptance reruns all 71 real CLI processes and the six native golden/oracle comparisons.
- `python3 tests/native/r08b-review-evidence/probes.py`: **20 independent real
  CLI processes** for explicit option rejection, missing workspace/registry/
  observations, context-only workspace/source overlap, unselected traversal
  reference, absolute/relative/dot path equivalence, final registry symlink and
  four large unowned source files. All expected checks pass; raw typed errors
  are retained in `probes.json`/`probes.log`.
- Six independent path-spelling processes and three independent canonical-path
  processes establish the finding above: **29 additional CLI processes** total.
- SHA256 checks for candidate, coder evidence and prior R08-A-C01 all match;
  final checks are saved under `tests/native/r08b-review-evidence/`.
- Original R03 fixture manifest checked with the explicitly documented README
  extension excluded: **29 original entries match**; the remaining **five symbolic
  manifests match the approved R07-B2C corrections** recorded in that round. All
  34 files preserve their approved baseline. See `original-fixture-check.log`.

Read the implementation/roadmap, declaration/render contracts, v4 render scope,
round brief and approved trusted-stdlib README. Inspected the owned native
command/module wiring and JSON implementation/tests, CLI acceptance oracle and
native-vs-R03 comparison. The command otherwise routes all jobs and registered
text references through one capture session, seals before trusted render, and
passes the raw bundle; no fresh projection reads or ambient configuration access
were found. The native serializer independently buffers bounded canonical JSON,
sorts unsigned-byte keys, escapes all C0 controls as lowercase Unicode escapes,
and rejects cycles/private types/invalid UTF-8 with depth/node bounds. Main
handles EINTR, SIGPIPE, failed and short writes with exit 4. The native golden
checks preserve original source prose/boundaries and independently recompute
identities rather than treating regenerated JSON alone as an oracle.

## Limits

The 4, 7, 8 and 9 MiB unowned-file CLI probes all refuse safely with exit 3,
empty stdout, and `resource_limit` / `instructions` / maximum 100,000,000 in
about 2.4 seconds each. They reach the trusted runtime instruction limit before
native JSON encoding. Therefore this review does **not** establish an end-to-end
16 MiB CLI envelope boundary; exact 16 MiB/one-byte-over evidence remains the
serializer unit test plus inspection of its direct default-limit CLI wiring.
No unexplained skipped tests. Native debug macOS only; release/Linux and the
full integrated R09 gate remain deferred. No corrective changes were made.
