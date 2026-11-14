# R20-CARRY-B: carried items closed directly

Done directly by the orchestrator on 2026-11-14 at the operator's direction ("finish group 2 directly"; no round workflow, no full gate). Starting revision `1c2fe300`.

## Changes

| item | disposition |
|---|---|
| CL-1 `call_retained_projection` | Fixed: `bridge_error` (by construction the first terminal failure, see `command_failure`) is tested before exhaustion. Tests `CL-1 retained projection keeps a caught illegal registration over later exhaustion` and `CL-1 extension projection callback keeps a caught illegal registration over later exhaustion` (`runtime.t.cpp`); red before the fix (instruction and allocation tails reported `resource_limit`), green after. |
| CL-2 `register_profile` module call | Fixed the same way; a present `bridge_error` also wins over a later `LUA_ERRMEM`. Test `CL-2 profile module chunk keeps a caught malformed registration over later exhaustion` (`external_registration.t.cpp`); red then green. |
| CL-4 `validate_profile_phases` phase pcall | Not reachable: that pcall runs only trusted embedded phase functions; untrusted shapes run through `call_retained_value`, which already records the first failure, and `setmetatable` is absent so no finalizer can run inside a phase. Guard test `CL-4 public validate_profile keeps a caught illegal registration in a shape over later exhaustion` passes before and after. |
| CL-3, CL-5, CL-6 | Unchanged: not reachable by untrusted callers or not injectable, as recorded. |
| Accounting audit, embedded `accept_load` | Fixed: the load site now charges the same full `prepare_stdlib` reservation as `bind_embedded_execution`, shared as `reserve_stdlib_preparation` (3x payload, 7x manifest, per-file and per-tree reserves, overflow guard). Test `command load acceptance reserves the full stdlib preparation before the codec` (`commands.t.cpp`, test-only pad modes in `tests/native/r14-d6-support/load.lua`); red (old charge 535,815 bytes for a 469,228-byte payload reached `accept`), green after. |
| Accounting audit, profile-load phase 4 | Carried (bounded limit). The codec chain does about 22 copies of the embedded payload, 8 of the profile and modules, 10 of the effective manifest and two catalog trees; the site charges 4 copies. Worst case about 55-65 MB uncharged against the 512 MiB native ceiling, bounded by the input limits. A correct charge breaks two calibration pins (`X06 B3 C each derivation charge scales exactly with the bytes it covers`, `X06 D C03 ordinary fixtures keep a quarter of every observable ceiling free`), so it needs a budget recalibration decision; not applied. |
| Near-limit timeouts (D28) | `replacement.cli` 30 s to 120 s; `planar_command` target 30 s to 120 s (covers the 1048576-byte prompt test); `planar_runtime` target 120 s to 300 s (X06 D C03 bisection tests exceed 120 s under `ctest -j8`). Hang detection in those targets is correspondingly slower. |
| Reviewer-packet wording | Carried as accepted wording: "No check receipt exists." is the render statement required by `render-contract.md` (checks section: render did not execute checks and no receipt exists) and is pinned in 982 expected-output files. It is true of render; in a reviewer packet the observations section lists receipts separately. Changing it needs a render-contract amendment. |
| Captured-preview instruction limit | Carried as a named limit: `render --capture-outputs` over the whole engine source exceeds the 100,000,000 Lua instruction ceiling and refuses with the typed `resource_limit` (R19-CMAKE). Raising ceilings is a contract change; revisit with real-use data. |
| Other carried limits | Unchanged and recorded: P01 survivor, N1 phase coverage unmeasured, x86_64 Linux never run, R20 configure residual (607 dyndep steps). |

## Verification (targeted)

- CL work: the 493 `planar_runtime_tests` cases under `ctest -j8`: 492 passed; the one failure was the X06 D C03 bisection timeout at 120 s, which led to the 300 s limit.
- Accounting work: whole registry under `ctest -j8` (1,877 tests): 1,876 passed, 0 skipped; the one failure was the same X06 D C03 timeout before the limit change.
- After the timeout change: `ctest -j8` over the new tests, the timeout guards, `replacement.cli`, the 1048576-byte prompt test and the X06 D C03 test: 12 of 12 passed. `rewrite-check.py lint`: format and Doxygen clean over 251 files.
- Not run: Release, Linux, Python acceptance suites. Red logs are local in the session scratchpad.
