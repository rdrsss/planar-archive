# R15–R18 C02 native check corrections

Scope: C01 native-review N1 and N4. N2/N3 ledger integrity moved to the separate
C02 integrity coder before any ledger edits. Shared checkout, no commit or
self-approval. C01 fencing and lineage changes remain intact.

`prepare_check` now compares the opened working/base directory descriptors by
physical device/inode identity while walking parents. Identical roots, ancestry
in either direction and aliases through intermediate symlinks refuse with
`path_denied`, `Capture roots overlap`, before capture/reservation. The same
physical overlap validation covers scratch against both checked roots. Existing
canonical spelling checks remain as early scratch refusals. Parent descriptors
use close-on-exec and are closed on every return path; failed descriptor probes
refuse rather than treating an unavailable identity as disjoint.

`classify` now retains the runner's actual leader termination status independently
of timeout/output-limit cause: a nonzero signal becomes its negative value;
otherwise the normal exit code is retained. Not-spawned remains absent. The
reason precedence and runner behavior are unchanged.

## Regression evidence

Evidence directory: `tests/native/r15-r18-c02-evidence/native/`.

Before product edits, `cmake --build build/rewrite-debug --target
planar_execution_check_tests -j3` succeeded. Then
`build/rewrite-debug/src/engine/planar_execution_check_tests 'C02*'` exited 42:
2 cases failed, 26 assertions, 6 failed. Four failures covered identical roots,
ancestor overlap in both directions and an intermediate symlink alias. Two
failures used actual shell children whose TERM handlers exit 7, separately
triggering timeout and output limit; reason assertions already passed while exit
status assertions failed. See `check-red-build.log` and `check-red.log`.

Post-fix the same focused build succeeded, then
`build/rewrite-debug/src/engine/planar_execution_check_tests` exited 0:
**7 cases, 63 assertions, all passed, no skips reported**. See
`check-green-build.log` and `check-green.log`. Pinned
`/opt/homebrew/opt/llvm/bin/clang-format --dry-run --Werror` on the two changed
files exited 0 (`check-format.log`). Debug build ownership was then released.
Parent coordinator owns integrated Debug/Release/Linux,
format/Doxygen and no-change gates and fresh independent review. No installed
binary or operator data was used; tests use per-case temporary directories.
