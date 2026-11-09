# C01 runtime corrective implementation

Implementation complete; independent review and parent full gates remain required. No commits, installed binaries, legacy tools or operator data were used. The shared checkout's Lua policy and ledger changes belong to the other corrective lanes.

## Changes

- RT1: `invocation.cpp` forwards native recovery evidence and findings rather than constructing empty arrays.
- RT2: request lookup falls back to an open reservation, returning exactly `request_id`, `operation:"check"`, `outcome:"in_progress"`, `reservation`, and `run:{id,generation}`. Generation is the run generation's ordinal. A read transaction keeps both lookup branches on the same SQLite snapshot, so completion between the queries cannot produce a false not-found. No token or secret is read. Runtime service admission validates the additional exact shape, positive decimal identifiers and operation; completed requests retain their result shape.
- RT3: digest capture reads no-follow metadata before excluding an exact `.git` directory, then applies the existing shared portable path and file-kind policy. Regular `.git` files, links and special files refuse; mixed-case `.GiT` files/directories refuse, matching render.
- Updated owned exported declarations with field, parameter and result documentation required by the Doxygen gate. Added the colocated digest tests to their existing CMake module; parent owns CLI CTest registration.

The original review's claim that render includes regular `.git` files was incorrect: `value.cpp::valid_path` rejects that component case-insensitively, and render invokes it after skipping exact `.git` directories. The corrective preserves that approved existing behavior. It does not widen path grammar. The original review remains unchanged as historical evidence.

## Verification

Raw evidence is under `tests/native/r15-r18-c01-evidence/runtime/`.

| Command | Result | Evidence |
| --- | --- | --- |
| `python3 tests/native/r15-r18-c01-runtime.py` against existing baseline source binary | 1 passed, 4 expected failures: active and abandoned request lookup, evidence recovery, rejection findings recovery | `cli-red.log` |
| `cmake --build build/rewrite-debug --target planar_capture_tree_digest_tests -j3` then `build/rewrite-debug/src/engine/planar_capture_tree_digest_tests`, with original blanket exclusion restored for the red run | 3 cases: 1 passed, 2 expected failures; 4 failed assertions across regular-file/link/special-file cases | `tree-red-final-build.log`, `tree-red-final.log` |
| `cmake --build build/rewrite-debug --target planar_capture_tree_digest_tests planar planar_review_lua_tests -j3` | Success; subsequent product rebuild adds read-transaction consistency | `green-build.log`, `green-rebuild.log` |
| `cmake --build build/rewrite-debug --target planar_capture_tree_digest_tests planar -j3` | Success | `green-final-build.log` |
| `build/rewrite-debug/src/engine/planar_capture_tree_digest_tests` | 4/4 cases, 15 assertions passed, zero skips | `tree-green-final.log` |
| `python3 tests/native/r15-r18-c01-runtime.py` | 5/5 assertions passed, zero skips | `cli-green-final.log` |
| Pinned `/opt/homebrew/opt/llvm/bin/clang-format --dry-run --Werror` on six owned C++ source/interface/test files | Success | `format.log` |
| `bash scripts/cpp-lint-doxygen.sh <scratch-root>/planar-runtime-doxygen-config` with both owned interfaces as input and normal warning/error settings | Success | `doxygen.log` |
| `git diff --check` | Success | terminal output |

The CLI exercise uses fresh temporary roots and a command rendezvous, checks a reservation during active execution, kills the Planar process and the separate check child process group, confirms the child exited before `check-recover --confirm-stopped true`, then completes a worker and records a rejecting reviewer verdict. Recovery assertions cover failed evidence and exact carried finding content including its native verdict ID. Completed-request status remains covered. Logs contain no run tokens.

Earlier exploratory logs are retained: `tree-red.log` and `tree-green.log` used the incorrect include-regular-file expectation before the render path policy was inspected; `cli-green.log` exposed a test expectation missing the required carried finding `verdict` member. The final tests and logs supersede those attempts. The added mixed-case test is parity coverage rather than a demonstrated baseline failure. CLI red and final differ only by the corrected carried-finding expectation (empty baseline arrays fail either expectation) and process cleanup tightening.

## Limits

Focused macOS Debug checks only. Full suite, release/Linux, malformed-service protocol injection and race stress are parent verification scope. The read snapshot closes the lookup inter-query race by construction; no concurrent completion stress result is claimed. The runtime lane does not approve itself.
