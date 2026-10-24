# R14-C3-A native capture extraction candidate

This refactor moves the complete existing capture/receipt implementation into
`planar.engine.capture.native`, with only the value module as a dependency.
The facade re-exports native values and preserves `capture_session` using a
private composed native session. Its declaration load body is byte-identical.
Both private native implementations remain in one module, with the original
friendship and full validating factory. No unchecked receipt factory is exposed.

`baseline/` freezes owned inputs before editing. `protection-before.json` covers
all other source/evidence files including the entire original capture.t.cpp.
Only owned paths, this evidence directory, build/Git outputs and coordinator-owned
`docs/vernacular/` planning/status records are excluded, as recorded explicitly.
Candidate identity hashes owned product/test/reference files; the evidence
manifest is hashed separately to avoid recursive identity claims.

`audit.py` reconstructs the native implementation/interface from the baseline by
exact module/class rename, removal of load and its Lua import, followed by the
pinned formatter. Exact bytes must match the candidate. It also proves the load
body is unchanged, verifies protected hashes, recreates both mechanical ledger
variants with exact/reverse-identical output, inspects the actual pure-native
link command and checks old test names/counts. `audit-first.txt` is an early audit
before the focused suite completed: extraction/protection/generation/link checks
passed, then the missing pending focused XML stopped it. It is superseded by
`audit-final.txt` after all commands finish.

`run.py` serializes the all-plus-tests build, capture25 (all 21 original cases plus
four native-only cases), unchanged history-focused151, CLI247, both vector checks,
active formatting/Doxygen and zero-work rebuild. `commands.json` records actual
arguments and exits. `helper-lint.json/txt` records the four unchanged verification
helper sources under the pinned formatter. No failing regression replay applies:
this is dependency extraction with exact algorithm preservation, no bug fix.

The pure tests construct real workspace and standalone registry captures, check
full seal refusal, repeat verification, preserve immutable published values,
refuse postseal byte/parent changes and exercise moves/session destruction.
Existing facade tests continue covering the full confinement, alias, quota,
normalization, topology and descriptor ownership contract without any edits.

Limits: macOS debug execution only. Linux, release and the complete integrated
R14 gate remain outstanding. Cooperative receipt checks do not prevent writes
after comparison or guarantee detection of changed-and-reverted inputs.
No storage writer, schema, history behavior, Lua policy or CLI behavior changes
are included. This candidate awaits fresh independent review; it is not approved
by its author.
