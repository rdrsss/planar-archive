# R14-F1 native source export candidate evidence

This is implementation evidence, not independent approval. The checkout is shared;
all pre-existing rewrite changes were retained. No legacy Planar tool, installed
binary, operator database, install, commit, publication or merge was invoked.

The frozen nine-file candidate is identified by `candidate-files.json` and
`candidate.sha256`. `before.json`, `baseline-inventory.json` and the pre-existing
Git status preserve the baseline inventory. The audit verifies 4992 protected
files unchanged, the AGENTS symlink, all 317 prior test names, ten added cases,
and all four exact/reversible generated variants (three unchanged older variants
and the new export-only variant). Planning records are coordinator-owned and
excluded from the protected-file inventory.

Commands executed serially in the debug build directory:

- `cmake --preset debug`, then `cmake --build build/rewrite-debug --target all planar_tests`.
  The initial build failed on missing explicit SDK C declarations and one unused
  test-only variable; those were corrected. Later builds pass without warnings.
- Focused `ctest --test-dir build/rewrite-debug -R 'replacement.source export'
  --output-on-failure`: initial eight cases passed, then all ten expanded cases
  passed, followed by ten final cases after strengthening the arrival race.
  `focused-final.xml` records ten matches, ten passes, zero skips.
- `python3 scripts/rewrite-check.py gate --build-dir build/rewrite-debug`: all 327
  registered cases passed with zero skips; 84 active C++ files passed pinned
  formatting and Doxygen, then the no-change build did no work. Raw full CTest
  output remains in `full-LastTest.log`. This includes all existing ledger/tree
  checks and three CLI suites: 247 render processes, 60 init processes and 115
  history processes, with unchanged native goldens and scratch-state protections.
- `python3 tests/vectors/generate.py --check` and
  `python3 tests/vectors/records/generate.py --check`: independent render and ten
  record vectors verified unchanged.
- `python3 tests/native/r14f1-evidence/mutation.py`: the exact generated test
  variant alone temporarily uses flags zero at the platform rename. The arriving
  empty-directory case fails at `REQUIRE_FALSE(result)` (exit 42), proving this
  test detects unsafe overwrite publication. `finally` restores the exact bytes,
  rebuilds, and the original targeted case passes. `mutation.json` and the patch
  preserve source/mutant/restored hashes. No product source was mutated.
- `python3 tests/native/r14f1-evidence/audit.py`, final active-source lint,
  test-helper formatting/Doxygen and final all+planar_tests no-work build pass.

The full gate preceded a test-only strengthening from nonempty arriving directory
to empty arriving directory. `gate-candidate-files.json` preserves that earlier
candidate. Product implementation, module interface, CMake and documentation are
identical between the two snapshots; only the test and syscall helper changed.
The complete ten-case export suite and both active/helper lint gates were rerun
on the strengthened final tests; the full 327 gate was not repeated without a
product change. The restored race case also passed after mutation verification.

The native suite checks exact arbitrary bytes, normalized modes under umask 0077,
source-only membership and unchanged ledger bytes; existing/unsafe/relative target
spellings; chronological old/change/revert reads; unsupported historical policy;
and a valid empty source root. The empty-root fixture narrowly adapts a scratch
accepted ledger through raw SQLite, restores exact triggers, then requires the
production verified opener before export. It adds no bypass to production.

Breadth reaches 4096 files and 8191 directories, while a separate case reaches
path depth 64; both child exports run under RLIMIT_NOFILE 128. Existing tree gate
cases cover exact/excessive byte, file, directory, depth and path budgets.
The fault suite covers 13 prepublication refusals, short-write/EINTR success,
three postpublication sync/close refusals, six cleanup/replacement/unowned-entry
cases, the real empty arriving-target no-replace race, and subprocess exit before
rename/at parent sync. All prepublication clean refusals leave no stage; explicit
cleanup-incomplete cases preserve unowned/replaced entries. Postpublication
failures preserve the complete target. Crash leftovers are not adopted.

`sdk-primitive.log` pins the actual macOS SDK declaration and RENAME_EXCL value.
Tests ran on macOS ARM64 only. The guarded Linux renameat2 syscall branch has
not been compiled or executed here. No hardware power-loss, hostile concurrent
owner, all possible syscall permutations, or actual allocator exhaustion is
claimed; allocation cleanup uses an injected std::bad_alloc at a write boundary.
Native F1 adds no CLI export, load policy, response encoding/stdout recovery,
installation, or release build evidence. Debug build ownership is released for
independent review after this report.
