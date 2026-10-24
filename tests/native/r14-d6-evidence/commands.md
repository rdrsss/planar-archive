# D6 command record

`<absolute-evidence>` is `<repo>/tests/native/r14-d6-evidence`.

All builds/tests used the shared checkout and debug only, strictly serialized.
No command used an installed binary, operator data, Planar queue, install, commit,
publish or merge. Build invocation for build-01 through build-05:
`cmake --build build/rewrite-debug --target all planar_tests`.

- build-01 exit 1: generated CLI runtime variants lacked new module dependencies.
- build-02 exit 1: new test digest assignment and virtual overload visibility errors.
- build-03 exit 0: initial implementation and eight new cases.
- load-01 exit 0: `ctest --test-dir build/rewrite-debug -R 'replacement.command load' --output-on-failure --output-junit <absolute-evidence>/load-01.xml`, 8/8, zero skips. LastTest archived before next test command.
- build-04 exit 0: expanded private admission, completion and cumulative tests.
- focused-01 exit 8: `ctest --test-dir build/rewrite-debug -R 'replacement\.(capture |acceptance prepared |prepared |load capture |interpretation |atomic |command |runtime |.*cli$)' --output-on-failure --output-junit <absolute-evidence>/focused-01.xml`, 132/133 passed, zero skips. One new cumulative fixture used deliberately unavailable string.rep, causing four expected-resource assertions to fail. Raw JUnit/LastTest retained before correction.
- build-05 exit 0: fixture corrected to bounded Lua string doubling.

These initial command exits are recorded from completed tool sessions; exact UTC
start/end timing was not captured for those invocations. Later checks are recorded
with exact argv, UTC start/end, exit and raw log in checks.json by run_check.py.
The first successful eight-case selection and failed 133-case selection are
superseded by the final source checks; they remain part of the evidence.

Formatter preparation used the pinned
`/opt/homebrew/opt/llvm/bin/clang-format -i` for the owned C++ sources before
build-03, then commands.t.cpp before build-04. Final lint checks those exact bytes;
no reconstructed or comment-only proof is used.
