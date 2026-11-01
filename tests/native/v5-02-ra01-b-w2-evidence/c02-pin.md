# c02_off_boundary re-pin (package PIN, D-29, operator decision 2026-10-30)

Status (V5-02-RA01-B-W2-C01, D-C01-20, D-C01-31): the `planar_hash` blocker below is closed and the
acceptance-5 values are re-recorded. Details: "Identity at C01 acceptance" and "C01 acceptance values".

Test: `X08 C01 role preparation outputs-off boundary pins the request-record conversion charge`
(`src/engine/runtime/external_registration.t.cpp`, window 8 unchanged, gap constants 4742 / 4737 unchanged).

## Values

| step | macOS | Linux |
|---|---|---|
| old pin (HEAD 3343ad38) | 227525 | 227551 |
| T0 interim (wave 1) | 227524 | not measured |
| post-wave-3 I0 / I-SC reports | not available to PIN (not in its brief); no intermediate pin recorded | not available |
| FINAL (measured after wave 4, this file) | 227275 (Debug and RelWithDebInfo) | 227301 (Debug) |
| C01 I-R, macOS Debug, checkout build (`build/rewrite-debug`, `planar_runtime_tests`) | 227275 | not run by I-R |
| C01 I-R, macOS RelWithDebInfo, checkout build (`build/rewrite-release`, `planar_runtime_tests`) | 227275 | not run by I-R |
| C01 gate stage, Linux container (Debug and RelWithDebInfo) | not applicable (container run) | 227301 pin held in Debug and RelWithDebInfo, within the +/-8 window (gate run 3, `raw/run3/linux`); no fresh measurement; see "C01 Linux gate rows" |

The Linux RelWithDebInfo value was not measured (one container run, Debug only); the prior pin comment recorded the two
build types as equal on Linux. Final deltas: macOS -250, Linux -250 (same shift, so a stdlib size effect, not a sizing difference).

## Identity of the measured bytes

Pathspec `src/stdlib src/engine/runtime/runtime.cpp`, output of identity.py, unchanged between measurement and the end of this step:

- HEAD `3343ad38304db57388bc7ac2438a40043eba1b7d`
- sha256(git diff --binary HEAD -- src/stdlib src/engine/runtime/runtime.cpp), tracked only: `70222fe5328f101932b089438b8a394c885bd46f2741a8f63943c64f0a004804`
- combined sha256 (with untracked): `9fc500ab5dbaa312f16acdcfe04eae00c0d0ec57f66993685c0b2a49f97322b5`

| file | state | sha256 |
|---|---|---|
| src/engine/runtime/runtime.cpp | modified | 9c8c15cff3bc42eaddb4066f26f32e8b9464be2426fef3b57ebee5f94448a71d |
| src/stdlib/planar/commands.lua | modified | f32c5be4b8ae3de2a36ded7b51081813bfe3c63ca3d3cfa0e58e702274cf10b7 |
| src/stdlib/planar/schema.lua | modified | e71fa6d969602e23ab9ee61c8838ccf3d7f8a0118ad27b35e1d0504927d86fe9 |
| src/stdlib/planar/semantic.lua | modified | 82bfe3dcad643d9eff7eb7d4c75108b40d2dcfd93dee69be711ba5e5f65f89aa |
| src/stdlib/planar/observe.lua | untracked | 00c852affb77a240922f746087602aa5296707ef1e5adce04f382fb491e15ca8 |
| src/stdlib/planar/orchestrator.lua | untracked | 3854209cc80b266b8ddee5d3e8b4397f2118dd0d80e3b1ccf2b731c5b0d40e5f |
| src/stdlib/planar/prompts.lua | untracked | b1524300da9bced721e329a5ebd64f8a68f3f902285afa8a2e1a1399b2ac9bed |

After the constants edit: identity.py for `src/engine/runtime/external_registration.t.cpp` gives tracked diff sha256
`0add2f2478e33174610389965c8922fe9409ab6fa5f86e2ab3922ef8db80ed3a`.

## Method and a blocker

Status (C01): CLOSED. `src/engine/CMakeLists.txt` now lists `planar_hash` in the `planar_capture` DEPENDS (line 36), and
`cmake --build build/rewrite-debug --target planar_tests` builds from the checkout (exit 0). The scratch-copy method and the
statement that the checkout build fails are historical; no scratch copy is needed for the C01 values below.

Historical text (wave 2 measurement):

The checkout build `cmake --build build/rewrite-debug --target planar_tests` FAILS: `src/engine/capture/capture.cpp:8`
`import planar.engine.hash;` but the `planar_capture` module in `src/engine/CMakeLists.txt` does not list `planar_hash` in
DEPENDS ("module 'planar.engine.hash' not found"). PIN does not own CMake, so it was not edited. The measurement used a
scratch copy of the checkout (rsync of the tree without build, .git, bin and *-evidence) outside the repository, with exactly one
change: `planar_hash` appended to the `planar_capture` DEPENDS. CMake wiring does not alter the embedded bytes. The
production-embedded files above are byte-identical in the copy (sha256 verified for runtime.cpp, commands.lua, schema.lua,
semantic.lua, prompts.lua).

Commands (scratch copy root `<scratch>/wt`):

- macOS Debug: `cmake --preset debug && cmake --build build/rewrite-debug --target planar_runtime_tests`, then
  `build/rewrite-debug/src/engine/planar_runtime_tests "X08 C01 role preparation outputs-off boundary pins the request-record conversion charge" -s`
  gave `outputs-off boundary 227275 pinned 227524`.
- macOS RelWithDebInfo: same with `--preset release` and `build/rewrite-release`: `outputs-off boundary 227275`.
- Linux (arm64 container, no tag, build without -t): `docker build -f scripts/measure.Dockerfile --target evidence --output type=local,dest=<out> .`
  where `scripts/measure.Dockerfile` is a scratch-only copy of `scripts/linux-gate.Dockerfile` whose gate RUN builds `planar_runtime_tests`
  (debug preset, -j4) and runs the same test with `-s`. Result: build exit 0, `outputs-off boundary 227301 pinned 227551`.
  `docker builder prune -f` was run afterwards (2.737GB reclaimed); no images were purged.

The explanatory comment block and operator decision reference in the test file are retained. If the CMake fix or any later edit
changes production-embedded Lua or runtime.cpp bytes, this step must be repeated.

## Identity at C01 acceptance (I-R, 2026-10-31)

Output of the workflow `identity.py`, pasted verbatim, pathspec `src/stdlib src/engine/runtime/runtime.cpp`:

| identity | value |
|---|---|
| HEAD | `3343ad38304db57388bc7ac2438a40043eba1b7d` |
| sha256(git diff --binary HEAD -- src/stdlib src/engine/runtime/runtime.cpp), tracked only | `70222fe5328f101932b089438b8a394c885bd46f2741a8f63943c64f0a004804` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src/stdlib src/engine/runtime/runtime.cpp, evidence excluded | `dc2916e2fea3326d224097be3de59193bfb67a4c48a1ee38354a3986ac4335b0` |

| file | state | sha256 |
|---|---|---|
| `src/engine/runtime/runtime.cpp` | modified | `9c8c15cff3bc42eaddb4066f26f32e8b9464be2426fef3b57ebee5f94448a71d` |
| `src/stdlib/planar/commands.lua` | modified | `f32c5be4b8ae3de2a36ded7b51081813bfe3c63ca3d3cfa0e58e702274cf10b7` |
| `src/stdlib/planar/schema.lua` | modified | `e71fa6d969602e23ab9ee61c8838ccf3d7f8a0118ad27b35e1d0504927d86fe9` |
| `src/stdlib/planar/semantic.lua` | modified | `82bfe3dcad643d9eff7eb7d4c75108b40d2dcfd93dee69be711ba5e5f65f89aa` |
| `src/stdlib/planar/observe.lua` | untracked | `bf4751a6f7c838d4a91c0f464cdd554be684ccd634a32abedcc1654826d0fce7` |
| `src/stdlib/planar/orchestrator.lua` | untracked | `a7f5ebb4e3b0b531656bb7dca97536b2a61524f9f5dacc68cae25bf30d147238` |
| `src/stdlib/planar/prompts.lua` | untracked | `c5bcba1dc9caef9721ad084538d9be9297d62fc42e34da8a999b82fa338590d8` |

The tracked-only hash equals the wave 2 value above (`70222fe5...`). The untracked Lua values differ from the wave 2 table
because the C01 R3 type check was added to `prompts.lua` and `observe.lua`. Both files are authoring-only and are not in
`planar_runtime_source` (`src/engine/CMakeLists.txt:82-92`), so the production boundary bytes do not move (D-C01-15).
`orchestrator.lua` is also authoring-only (`src/cmd/planar/CMakeLists.txt`, `authoring_lua_files`), and its hash is the R8
value required by the round record, from `identity.py src/stdlib/planar/orchestrator.lua`, verbatim:

| identity | value |
|---|---|
| HEAD | `3343ad38304db57388bc7ac2438a40043eba1b7d` |
| sha256(git diff --binary HEAD -- src/stdlib/planar/orchestrator.lua), tracked only | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src/stdlib/planar/orchestrator.lua, evidence excluded | `ae81f05e7e90e0dee2d37e027ae4ec3ccb8e2b6077df7e526a8f755e549d6ae9` |

| file | state | sha256 |
|---|---|---|
| `src/stdlib/planar/orchestrator.lua` | untracked | `a7f5ebb4e3b0b531656bb7dca97536b2a61524f9f5dacc68cae25bf30d147238` |

Previous orchestrator.lua hash in the wave 2 table (`3854209c...`) is superseded by the value above.

## C01 acceptance values (I-R, acceptance 5, D-C01-31)

Test: `X08 C01 role preparation outputs-off boundary pins the request-record conversion charge`, run unchanged. Old pin
(constants in `src/engine/runtime/external_registration.t.cpp:4570,4573`): macOS 227275, Linux 227301. New measured values:

| build | command | output (verbatim lines) | result |
|---|---|---|---|
| macOS Debug | `build/rewrite-debug/src/engine/planar_runtime_tests "X08 C01 role preparation outputs-off boundary pins the request-record conversion charge" -s` | `outputs-off boundary 227275 pinned 227275` (two lines) | `All tests passed (237 assertions in 1 test case)`, exit 0 |
| macOS RelWithDebInfo | `build/rewrite-release/src/engine/planar_runtime_tests "X08 C01 role preparation outputs-off boundary pins the request-record conversion charge" -s` | `outputs-off boundary 227275 pinned 227275` (two lines) | `All tests passed (237 assertions in 1 test case)`, exit 0 |
| Linux container (Debug and RelWithDebInfo) | `scripts/linux-gate.sh` full gate, gate run 3 (`raw/run3/linux/container/debug-ctest.log`, `release-ctest.log`) | ctest lines only, no `-s` output: `replacement.X08 C01 role preparation outputs-off boundary pins the request-record conversion charge` Passed (5.92 s Debug, 1.91 s RelWithDebInfo); `100% tests passed out of 1437` in each | Passed within the +/-8 window of 227301, both build types; no fresh measurement |

Old and new are equal on macOS (227275 in both configurations). The Linux rows are filled by C02 (section "C01 Linux gate rows" at the end of this file). The pin is unchanged: no edit to `runtime.cpp`, `commands.lua`, `schema.lua` or `semantic.lua` in C01.

## Other pins of the embedded stdlib

None observed to move in the runs above: only the one test was run in each environment, so other digest pins were not exercised.
They must be checked by the full gate.

## C01 Linux gate rows (filled by C02, D-C02-14, C02-14)

Source: `tests/native/v5-02-ra01-b-w2-c01-evidence/raw/run3/linux/` (`linux-gate.stdout`, `manifest-compare.txt`, `container/debug-ctest.log`, `container/release-ctest.log`); the container ctest exit files are 0 and both logs end `100% tests passed out of 1437`.

- The pin test `X08 C01 role preparation outputs-off boundary pins the request-record conversion charge` PASSED in the Linux container in Debug and in RelWithDebInfo. The test checks a window of plus or minus 8 around the pin (Linux 227301), so the result is "within the +/-8 window", not equality.
- No production-embedded byte changed since the wave 2 measurement: `runtime.cpp`, `commands.lua`, `schema.lua` and `semantic.lua` are byte-identical (output of the workflow `identity.py` for those four pathspecs, pasted verbatim below, matching the per-file table above).
- The Linux value 227301 is the wave 2 Debug measurement. Linux RelWithDebInfo was never separately measured by a fresh measurement; the gate-3 run only passed the pin test in that configuration.
- Pin values unchanged: macOS 227275, Linux 227301.

| identity | value |
|---|---|
| HEAD | `3343ad38304db57388bc7ac2438a40043eba1b7d` |
| sha256(git diff --binary HEAD -- src/engine/runtime/runtime.cpp src/stdlib/planar/commands.lua src/stdlib/planar/schema.lua src/stdlib/planar/semantic.lua), tracked only | `70222fe5328f101932b089438b8a394c885bd46f2741a8f63943c64f0a004804` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src/engine/runtime/runtime.cpp src/stdlib/planar/commands.lua src/stdlib/planar/schema.lua src/stdlib/planar/semantic.lua, evidence excluded | `70222fe5328f101932b089438b8a394c885bd46f2741a8f63943c64f0a004804` |

| file | state | sha256 |
|---|---|---|
| `src/engine/runtime/runtime.cpp` | modified | `9c8c15cff3bc42eaddb4066f26f32e8b9464be2426fef3b57ebee5f94448a71d` |
| `src/stdlib/planar/commands.lua` | modified | `f32c5be4b8ae3de2a36ded7b51081813bfe3c63ca3d3cfa0e58e702274cf10b7` |
| `src/stdlib/planar/schema.lua` | modified | `e71fa6d969602e23ab9ee61c8838ccf3d7f8a0118ad27b35e1d0504927d86fe9` |
| `src/stdlib/planar/semantic.lua` | modified | `82bfe3dcad643d9eff7eb7d4c75108b40d2dcfd93dee69be711ba5e5f65f89aa` |
