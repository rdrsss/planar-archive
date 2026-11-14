# R20-CARRY-A red check

Red-time identity (covers tracked diff and untracked bytes under src and tests/native/*-support):
- HEAD: dec6f178daf10e2a1219ae56dc71485d42d839ca
- sha256 candidate.diff: 136cb6973278e9cb8d86892c39eaa182c8ae49eeba680c6343123804c9b9be6e
- sha256 SHA256SUMS: 108d7585cfdb577ea0722b6ca34cb0c6c5b398d6ecc9b4e6c5948018d34e9b64
- snapshot: red/snapshot-2 (snapshot/ was incomplete: its untracked glob matched nothing; kept, superseded)

Build: cmake --build build/rewrite-debug --target planar_tests, rc 0 (raw/build.log).

All expectRed tests failed with AssertionError (failed-assertion); guards passed. Raw logs in raw/<name>.log; failing line is the AssertionError in each.
Evidence-only invariants (I14-I18, I22, I23, I25-I28) have no test and are not run here.
- build: passed
- configure_rebuilds_on_content_change: passed
- configure_zero_work:     raise AssertionError(message)
- embed_callsite_audit:     raise AssertionError(message)
- embed_content_change_rewrites: passed
- embed_unchanged_mtime:     raise AssertionError(message)
- generator_content_change_rewrites_b1: passed
- generator_content_change_rewrites_d6_c01: passed
- generator_content_change_rewrites_d6: passed
- generator_content_change_rewrites_x05_a0: passed
- generator_unchanged_mtime_b1:     raise AssertionError(message)
- generator_unchanged_mtime_d6_c01:     raise AssertionError(message)
- generator_unchanged_mtime_d6:     raise AssertionError(message)
- generator_unchanged_mtime_x05_a0:     raise AssertionError(message)
- manifest_content_change_rewrites: passed
- manifest_unchanged_mtime:     raise AssertionError(message)
- timeout_headroom_above4096:     raise AssertionError(message)
- timeout_headroom_v2_allocation:     raise AssertionError(message)
- timeout_names_registered: passed

## Failing assertion messages
- configure_zero_work: AssertionError: reconfigure over the unchanged tree made ninja do work: '[1/1] Generating stamp'
- embed_callsite_audit: AssertionError: embed call-site audit failed:
- embed_unchanged_mtime: AssertionError: reconfigure over the unchanged tree moved the module mtime (the helper wrote it more than once)
- generator_unchanged_mtime_b1: AssertionError: B1 generator rewrote 63 of 63 unchanged outputs (mtime moved), first: ['commands_0.lua', 'commands_1.lua', 'commands_10.lua']
- generator_unchanged_mtime_d6_c01: AssertionError: D6-C01 generator rewrote 3 of 3 unchanged outputs (mtime moved), first: ['commands_0.lua', 'runtime_0.cpp', 'runtime_0.cppm']
- generator_unchanged_mtime_d6: AssertionError: D6 generator rewrote 18 of 18 unchanged outputs (mtime moved), first: ['commands_0.lua', 'commands_1.lua', 'commands_2.lua']
- generator_unchanged_mtime_x05_a0: AssertionError: X05-A0 generator rewrote 7 of 7 unchanged outputs (mtime moved), first: ['commands_0.lua', 'commands_1.lua', 'generation.json']
- manifest_unchanged_mtime: AssertionError: manifest generators not write-if-different: r12c2: rewrote ['generation.json'] (mtime moved) of 3 outputs; r14f1: rewrote ['generation.json'] (mtime moved) of 3 outputs; r14f2: rewrote ['generation.json'] (mtime moved) of 3 outputs; v5-02-b2: rewrote ['generation.json'] (mtime moved) of 265 outputs; r14-d7-b: rewrote ['generation.json'] (mtime moved) of 45 outputs; v5-02-x07: rewrote ['generation.json'] (mtime moved) of 221 outputs
- timeout_headroom_above4096: AssertionError: replacement.recorded effective history selects wider fixed tree profile for valid closure above4096 files: TIMEOUT 30 < 120
- timeout_headroom_v2_allocation: AssertionError: replacement.v2 every native allocation failure remains a redacted typed resource refusal: TIMEOUT 30 < 120
