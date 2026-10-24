# R07-A-C02 terminal protected-call evidence

Candidate implementation; independent review pending. Owned changes are
src/kernel/runtime/runtime.cpp, runtime.t.cpp, tests/stdlib/bridge.lua,
stdlib/planar/README.md and this evidence directory. Shared changes preserved.
No Planar tools, installed binaries, commits, installation or publication.

## Red evidence

Before runtime.cpp changes, red-build.log built planar_runtime_tests and
red-regressions.log ran `[r07ac02]` (exit 42). caught_quota failed its required
resource diagnostic check (child exit 1). repeated_quota and handler_quota each
exceeded the 300 x 10 ms subprocess bound and were killed with SIGKILL. These
are the retained review's three infinite-loop bodies, without shortened loops.
The stock pcall caught the hook error; stock xpcall entered an unbounded handler.

The ordinary-pcall fixture also failed before the fix, because it expected
error(nil) to preserve nil. Lua 5.5 explicitly substitutes `<no error object>`
(vendor/lua/0b10/src/ldebug.c:849). green-regressions.log retains this fixture
failure after the runtime fix while the quota case passed; fixture-regressions.log
then identifies nil_error as the only failed ordinary assertion. The final
fixture checks the pinned Lua behavior. The correction does not weaken any quota
loop. A fourth nested-pcall quota subprocess was added after the initial red run.

## Implementation and protected-call audit

Curated pcall uses lua_pcall with errfunc zero and only trivial C++ locals. It
checks the monotonically latched instruction count before returning to Lua,
raising again through each enclosing curated pcall until the host boundary.
No trusted error handler runs with hooks disabled. xpcall is removed, documented
as unsupported and tested both as an absent capability and by attempted use of
the original infinite handler expression; refusal must complete within 3 seconds.
Normal zero/multiple/nil return arity, argument forwarding, object errors,
nil-error normalization, invalid/missing callable arguments and nested calls
are checked separately. Existing native-error recovery and old-token rethrow
checks remain unchanged and pass.

The host checks instruction exhaustion immediately after lua_pcall regardless
of return status. The only other internal lua_pcall anchors table tags during
native conversion: it runs native raw tag operations, no Lua callbacks, handlers
or instructions. It now checks terminal instruction status before classifying
an anchoring allocation failure. Callback temporaries still unwind before any
Lua error is raised. No native error-token representation changed.

## Executed verification

Commands were run sequentially from the repository root:

```sh
cmake --build build/rewrite-debug --target planar_runtime_tests
build/rewrite-debug/src/kernel/planar_runtime_tests '[r07ac02]'
cmake --build build/rewrite-debug --target all planar_tests
build/rewrite-debug/src/kernel/planar_runtime_tests '[runtime]'
ctest --test-dir build/rewrite-debug -R 'replacement.(runtime|loader|capture|canonical|hash)' --repeat until-fail:2 --output-on-failure --output-junit <repo>/tests/native/r07ac02-evidence/final-native.xml
python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug
cmake --build build/rewrite-debug --target all planar_tests
build/rewrite-debug/src/kernel/planar_runtime_tests '[r07ac02]'
```

Build logs: red-build, green-build, fixture-build and final-build. Final results:
13 runtime cases / 361 assertions; 2 focused regression cases / 25 assertions;
37 matched native cases passed twice in fresh processes, zero failures/skips;
pinned formatting and Doxygen passed for 21 active sources; no-change build
reported `ninja: no work to do.` Raw logs and native XML are retained here.

candidate.sha256 identifies the four owned candidate source/document files.
macOS debug only; Linux, release and sanitizers were not exercised. No approval
is claimed and R07-B remains outside this scope.
