# R07-A-C05 implementation evidence

Candidate only; independent review is required. Shared checkout preserved; writes
limited to the four assigned implementation/test/reference files and this evidence
directory. No Planar tools, queues, installed binary, commits or installation.

`candidate.sha256` identifies exactly the four candidate files. Its SHA256 is
`899c2e06450e20f2d55645407115a00fa67be3b452172a2821f3bb934d91ea99`.
Starting revision/status are retained separately; these four files were already
untracked parts of the ongoing rewrite, so ordinary git diff does not capture them.

## Changes

Runtime setup builds the exposed table object by copying the explicit allowlist
concat/create/pack/unpack from Lua's opened library. insert/remove/sort/move are
unreachable through the table global; no package/debug/require path exposes the
original library. The setup loop has only trivial locals across Lua allocation
and error boundaries. Fixed deterministic seed is unchanged. No consumer required
replacement of the removed helpers. README requires future schema sorting and
shifting to use metered Lua and removes the invalid storage-bound native claim.

Two new cases use tiny ordinary tables. One asserts insert/remove/sort calls fail;
the other checks exact allowlist membership and behavior of all four retained
helpers. Red-before-fix: 2 cases failed, 5 failing assertions (all three ordinary
calls succeeded; absence/allowlist false). No giant sparse range executes.
Green: 2 cases, 14 assertions. Earlier quota, finalizer, conversion, diagnostic,
wrapper, pattern, repetition and memory regressions remain unchanged.

## Remaining table audit

Pinned source: vendor/lua/0b10/src/ltablib.c. create (62-68) checks capacity and
calls lua_createtable, which is allocator-limited. concat/addfield (158-182)
requires each traversed value to be a string/number; missing/nonstring entries
error immediately. Even with a large explicit range, successful prefix traversal
requires populated input. pack (192-202) obtains its bound from lua_gettop and
allocates the result. unpack (205-218) rejects counts >= INT_MAX and reserves the
entire result stack with lua_checkstack before traversing. These are
input/allocation bounds, not instruction metering or wall-clock guarantees.

sort_comp (271-287) accepts native function comparators, whose execution need not
invoke a VM hook. Removing sort closes that path, rather than assuming callbacks
are metered. The other allowed table functions have no comparator callback path.
No metatables can be created by trusted Lua; captured inputs also refuse them.
The original table library remains only in inaccessible registry state; no
permitted capability retrieves it.

## Executed checks

Each JSON/log pair records actual command, return code, elapsed time and raw output.
Builds/tests were serialized in build/rewrite-debug with subprocess timeouts.
- red-build: targeted runtime test build passed.
- red: expected exit 42, 2 failed cases / 5 failed assertions.
- build: all + planar_tests passed.
- green: 2 cases / 14 assertions passed.
- runtime: 18 cases / 432 assertions passed.
- native: 42 selected runtime/loader/capture/canonical/hash CTest cases passed;
  native.xml confirms failures=0 and skipped=0.
- lint: pinned formatter and Doxygen passed over 21 active sources.
- nochange: all + planar_tests reported ninja: no work to do.

The pinned formatter was also run on both owned C++ files. Two exploratory command
mistakes were corrected before checks: the initial runner path used bin/ rather
than src/kernel/ (no test executed); an rg call had an unmatched cmake/toolchain*
zsh glob. Neither is counted as verification. The retained red record comes from
the corrected runner and final tests before changing runtime setup.

Limits: macOS arm64 debug only; no Linux/release/sanitizer or complete CLI gate.
This is bridge isolation evidence, not schema/renderer acceptance.
