# R14-G documentation and command composition evidence

Shared checkout HEAD is recorded in `starting-head.txt`. This bounded unit owns
only eight assigned documents and this new evidence directory. `baseline.json`
freezes the pre-edit non-build/cache files (11,590); `owned-before.json` records
the eight initial document identities. `preexisting-status.txt` records the dirty
rewrite status during this unit; it is not a clean-tree claim. Coordinator-owned
vernacular planning files are excluded except the assigned implementation plan.
All prior evidence is protected. No product/build/test/vendor input was changed.
The coder does not approve this candidate.

`candidate-docs.json` hashes the eight final documents. `composition-inputs.json`
hashes 1,862 unchanged product, build, test, fixture and vendored dependency inputs,
excluding generated builds, evidence and caches. The broader protected audit also
covers other unchanged repository files. `audit.py` verifies all 11,581 protected
files, the AGENTS.md symlink, both 331-name inventories, 43 local links/anchors,
trailing whitespace and composition identity. Its initial and final logs are
retained. Current declarations of source/module paths were checked against the
existing source tree; no load service/CLI was invented.

Executed serially with exclusive debug/release build-directory ownership:

- `make test-all PRESET=release`: exit 0; release RelWithDebInfo all+planar_tests
  build, all 331 tests passed, zero skips, pinned formatting/Doxygen over 86 active
  sources and zero-work rebuild. Full raw command/build/test output is retained
  in `release-gate.log` and the status in `release-gate.exit`.
- `ctest --test-dir build/rewrite-release -V -R
  '^replacement\.(cli|init_cli|history_cli|export_cli)$' --output-junit
  tests/native/r14g-evidence/release-cli.xml`: four matched, four passed, zero
  skips; 247 render, 60 init, 115 history and 55 export/output actual CLI processes.
  Retained verbose output, JUnit and LastTest log give complete process evidence.
  CTest resolved the relative JUnit path under the build directory; it was copied
  unchanged into this evidence directory.
  This focused rerun was needed because the existing gate deletes its temporary
  JUnit, and an inventory query replaced LastTest before it could be copied.
  No redundant complete gate was run.
- `cmake --build build/rewrite-debug --target all planar_tests`: exit 0, zero work.
- `python3 tests/native/r14g-evidence/audit.py`: passed at entry/final candidate;
  invokes CTest JSON inventory for each preset without executing tests.
- Tool version commands and exact CMake compiler/libc++/preset fields are recorded
  in `host-toolchain.json` (macOS 26.5.2 ARM64; LLVM 23.1.3; CMake 4.4.4;
  Ninja 1.13.2; Doxygen 1.18.0; Python 3.15.0).

Approved debug evidence is reused, not newly executed: `reused-debug.json`
identifies the F2 full331 gate, preserved full LastTest, strengthened export suite
and independent review. All 13 non-document F2 candidate members still match the
approved manifest. Both inventories retain exactly its 331 names; the composition
manifest identifies all current test/build/product inputs. The approved debug
post-gate export-harness strengthening and review are recorded in F2 evidence.

The full suites already compose scratch native acceptance fixtures, historical
reads and exact export, including ledger-byte identity. No extra ad hoc composition
or persistent test was added. Only explicit source-built paths and scratch state
were exercised; no old Planar tools/queue/claims, installed binary, operator data,
commit, install, publish or merge was used. No fixes were needed for the gate.

This is a composition checkpoint, not full R14 closure. Load membership policy,
load service/CLI, final R14 acceptance, accepted-revision rendering and execution
remain pending. Linux runtime/compilation and hardware power loss are unverified.
Build-directory ownership is released to the coordinator for independent review.
