# C06 diagnostic correction implementation

Implemented F-FINAL-1/F-FINAL-2 and I1–I6 within assigned ownership. No approval or integrated acceptance claimed.

`check_error` is an execution-owned typed diagnostic with field, first mismatching producer work, and a contract-delivery marker. It wraps unchanged lower-level diagnostics. Both preparation and execution return it; registry and scratch errors carry their approved fields. Delivery failures carry a marker instead of trusting a Lua-supplied revision; the command adapter reads the already verified execution snapshot revision. Producer prechecks explicitly sort by work, retaining the first mismatch and skipping producers without completion. Generic engine diagnostic and response serialization APIs are unchanged.

Command-local check/verdict adapters preserve the existing resource and capture error shape, add approved execution context, and retain both parser and normalizer verdict field paths. The distinct-request open-check guard now uses the exact check_in_progress message. Existing execution_code call sites for history_unavailable, drifted, no_active_job, interpretation_mismatch and state_conflict retain their fixed messages.

## Evidence

Raw logs and exact source hashes: `tests/native/r15-r18-c06-evidence/diagnostics/`.

- Before edits, the new assertion-bearing CLI harness ran against existing source-built Release: five scenarios, five failures, zero skips (`cli-red.log`), matching malformed verdict, missing verdict field, scratch overlap, reserved registry name and overlapping check message.
- `cmake --build build/rewrite-debug --target planar planar_execution_check_tests planar_execution_verdict_tests -j3` passed (`build-final.log`). Earlier bounded builds also passed.
- Check native suite: **8 cases, 109 assertions, all passed**, zero skips (`native-final.log`). New case covers scratch/registry context, missing/unequal/inline contracts, reverse-ordered two-producer mismatch, no-completion producers, and deterministic scratch-name collision without touching the existing file.
- Existing verdict suite: **9 cases, 241 assertions, all passed**, zero skips (`verdict-final.log`).
- Final serialized registered CTest: **1/1 passed**, containing **12 CLI scenarios, zero failures/skips** (`ctest-final.log`, complete output in `ctest-final-details.log`). It covers grammar, top-level and nested shape, normalize decision path, scratch overlap, registry name, overlapping requests, deterministic post-reserve scratch collision and recovery, three delivery forms, and two simultaneous producer mismatches naming the lexically first work. Refusals before reservation compare complete SQLite logical dumps unchanged. The scratch collision verifies a durable open reservation and recovers it. A file-gated real child keeps overlap tests deterministic; same-ID in-progress and changed-actor conflict controls pass, then the original check completes normally.
- Pinned clang-format dry-run/Werror passed four owned C++ files (`format.log`); scoped Doxygen with warnings-as-errors passed (`doxygen.log`).

Expanded scenarios and the new native typed-context assertions have green evidence only; the coherent pre-C06 binary was unavailable after parallel rebuilds. Do not describe them as independently red-first. Fresh final mutation probes must establish their sensitivity against the final coherent candidate. Initial expanded CLI attempts failed due test fixture setup: wrong check-recover option, missing required contract-start edge, applying output_vector to producers without dependencies, and dirty base causing unrelated boundary failure. Logs green-2 through green-5 preserve those setup errors; final assertions were not weakened.

A no-work attempt instead rebuilt after concurrent shared source/CMake updates; no no-work claim is made. One single Python CTest was accidentally started while that rebuild remained active; its `ctest.log` is excluded. The rebuild finished successfully before serialized reruns (`ctest-serialized.log`, final `ctest-final.log`). Full integrated no-work/Debug/Release/Linux verification remains with the coordinator. No installed or legacy Planar was used; all ledgers were disposable. No commit or mutation probe performed by this coder.
