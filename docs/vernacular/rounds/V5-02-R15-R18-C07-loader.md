# C07 loader terminal-refusal correction

Product implementation complete; focused loader suite passes. A final test-only
anchoring sensitivity assertion is saved; its rerun and the unchanged runtime
large-Lua control are delegated to the policy coder's shared Release build. Independent
review/mutations remain pending. No approval or final calibration claim.

The declaration loader now uses one first-diagnostic latch for composed Lua
allocation refusals, instruction-hook diagnostics and native-constructor errors.
The allocator may still allow Lua's GC retry and its ordinary accounting cleanup,
but successful retry cannot erase the refusal. State creation, protected setup,
source parsing, protected execution and protected table anchoring inspect the
latch before returning or adapting another error. A later native/allocator
failure cannot replace the earlier typed diagnostic. Existing inherited chunk
limit precedence remains intact.

The change removes the separate `composed` optional, whose error was previously
consulted only under `LUA_ERRMEM` and could overwrite another composed failure.
All latch writes finish their C++ temporaries before an allocating Lua operation;
protected anchoring continues to catch Lua longjmp before native conversion
objects. No public hook, interface change, product-limit change or larger
large-Lua fixture was introduced.

## Regression evidence

Raw logs: `tests/native/r15-r18-c07-evidence/loader/`.

New deterministic test-only budget sink refuses recoverable growth and then
allows the immediate retry. It is intentionally not sticky, so success/failure
must be decided by the loader. Tests cover one and two distinct refusals, a
successful no-refusal control, released live charges, parsing followed by syntax
error, protected table anchoring, a later semantic error after allocation refusal,
and an earlier native semantic error followed by error-message allocation refusal.

- Before product edits, `cmake --build build/rewrite-release --target planar_lua_loader_tests -j3`
  succeeded (`red-build.log`, `red-build-2.log`).
- `planar_lua_loader_tests '[c07]'`: final assertion-bearing red run **2 cases,
  28 assertions, 6 failures** (`red-2.log`). Initial red1 preserved separately.
  Failures include returned success after GC retry, syntax replacing the owner
  refusal and later path error replacing the first allocation refusal. The
  earlier-native-error control passes before and after.
- Corrected Release build succeeded (`green-build.log`). Full loader suite:
  **21 cases, 185 assertions, zero failures/skips** (`green.log`).
- Pinned formatter dry-run/Werror passed both changed C++ files (`format.log`).
  Scoped Doxygen warnings-as-errors passed loader implementation/interface/tests
  (`doxygen.log`); the interface itself is unchanged.

Release lane was handed to the policy coder for its coherent product/test rebuild,
including runtime tests and the unchanged `X06 D C01 declaration Lua memory counts
against the composed live Lua ceiling` case. No no-work claim is made while the
shared candidate is changing. Full Debug/Release/Linux gates and independent
mutation sensitivity remain coordinator obligations. Only assigned loader source,
tests, report and evidence were edited; no installed/legacy Planar, operator data,
commit, publication or installation was used.

## Final handoff

After the shared loader test TU finished compiling, added a single test-only
`native_calls == 1` assertion in the recovered-anchor case. This prevents the
outer execution-boundary latch from masking removal of the immediate anchoring
check: no subsequent native conversion charge may cross that failed boundary.
Pinned format passes final files (`format-final.log`). The earlier21/185 result
predates this one assertion; final expected count is21/186 and is not claimed
until rerun. Product loader source is unchanged since the green suite apart
from formatting.

The policy coder owns the active Release lane and has accepted the final
incremental loader-tests build/full-suite rerun (`green-final.log`) and unchanged
large-Lua case (`biglua-green.log`). Coordinator integrates their actual outcomes.
No agent-owned build remains running here; there is no no-work claim.
