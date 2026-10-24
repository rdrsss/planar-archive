# Quota render fixture

This is a newly authored R03 example. The historical fixture described in the
[v4 design](../../../docs/vernacular/Planar%20design%20spec%20-%20v4.md) was absent;
none of its code, fake identities, protocol traces or claimed evidence was
recovered. This example follows the approved [declarations](../../../docs/vernacular/declarations.md)
and [render contract](../../../docs/vernacular/render-contract.md). The original `expected/` and symbolic manifests remain the R03 authored
expectations. [Computed native packets](native/README.md) now record the six
actual R08-B renders and document their comparison with those expectations.

## Layout and authority

`workspace/`, `current/`, `base/`, `registry/`, and `observations/` are sibling,
physically separate trees. No links or special files are intentional. Both jobs
load the single [spec.lua](workspace/spec.lua), the same contract and the same
base. Both use `fanout_review`; only unit grouping and resulting role mapping
vary. Library owns `src/core/` and `tests/core/`; command owns `src/cli/` and
`tests/cli/`. Their reviewer owns no source paths. The source `contracts/` frozen
boundary and `generated/` excluded boundary are intentionally absent, disjoint
from work paths, and remain explicit policy even when absent. Workspace
`text/quota-api.md` supplies the contract; no source copy is implicitly loaded.
No common test package file outside a unit's boundaries is needed.

The trusted [registry](registry/quota-tools.json) is driver-selected using the
CLI, not loaded by a workspace-provided path. It describes commands; render
executes none. Its Python/PATH assumptions are illustrative: a future execution
driver must review the Python 3.9+ executable and replace this trusted registry
for hosts where `/usr/bin:/bin` has no suitable Python. The current shell's `python3` was tested, and the exact registry command was
also tested with its replacement environment: Python 3.9.6 resolved successfully
on this macOS host. This is local source evidence, not a portable executable pin. Checks run from the captured source root.
A future adapter stages selected own-output test directories; the contract
check selects 4 library tests or 2 command tests when scoped, 6 when combined.
Consumer argument tests defer library import; the integration check explicitly
requires the actual command/library vector. Test examples cover the written
contract, but do not mechanically prove arbitrary changes to its prose.

`current/` is a concrete implemented example, not a claim of dispatched work.
`base/` contains the library and library tests only. The library module's initial
docstring differs; the rest of the library snapshot is unchanged. All command
files are added. There are no deletions. Tests use standard-library Python only;
bytecode is disabled so captured snapshots stay clean.

## Expected packets and symbolic manifests

| Invocation | Expected Markdown | Manifest template | Observation input |
| --- | --- | --- | --- |
| combined / builder | [builder](expected/combined-builder.md) | [M](manifests/combined-builder.symbolic.json) | [empty](observations/empty.json) |
| combined / reviewer | [reviewer](expected/combined-reviewer.md) | [M](manifests/combined-reviewer.symbolic.json) | [symbolic](observations/reviewer.symbolic.json) |
| split / library_worker | [library](expected/split-library_worker.md) | [M](manifests/split-library_worker.symbolic.json) | [empty](observations/empty.json) |
| split / cli_worker | [command](expected/split-cli_worker.md) | [M](manifests/split-cli_worker.symbolic.json) | [empty](observations/empty.json) |
| split / reviewer | [reviewer](expected/split-reviewer.md) | [M](manifests/split-reviewer.symbolic.json) | [symbolic](observations/reviewer.symbolic.json) |
| split / cli_worker recovery | [recovery](expected/split-cli_worker-recovery.md) | [M](manifests/split-cli_worker-recovery.symbolic.json) | [checkpoint](observations/recovery.json) |

The Markdown fixes proposed phrasing and contains all mandatory sections in
projection order. R08 must compare actual outputs with these expectations,
review differences, and pin final bytes. Editorial expectation metadata belongs
in this README. Target-bearing observation and manifest files ending in
`.symbolic.json` are **non-executable templates**, despite being valid JSON.
Their `$...` strings intentionally fail the native digest grammar. Never pass
them to a renderer as valid inputs or report them as native results.

Symbol legend (all exact native identities remain uncomputed):

- `$F_NNN`: file-byte digest slots. Equal byte strings share one symbol even
  across roots; distinct captured byte strings have different symbols. Numbers
  label slots, not hash values. Every file record includes actual size/hex bytes.
- `$BASE_INITIAL`: base identity for the supplied library-only snapshot.
- `$OBS_CURRENT`: complete non-excluded current-tree observation identity.
- `$O_COMMAND`, `$O_LIBRARY`: separate exact unit outputs, identical across
  grouping variants for this fixed root/base/current input.
- `$R_<JOB>_<WORKER>`: worker role-output digest. Recovery reuses
  `$R_SPLIT_CLI_WORKER`. No reviewer role_output is invented.
- `$C_QUOTA_API`: exact contract semantic identity; `$S_LIBRARY_FILES`: observed
  facet semantic identity. The complete-only facet is absent from W.start_facets.
- `$W_<JOB>_<ROLE>`: scoped work fingerprint; the recovery packet reuses
  `$W_SPLIT_CLI_WORKER` because checkpoint/observations do not enter W.
- `$FRAGMENT_<PACKET>`: fragment digest for that authored Markdown; recovery has
  its own fragment symbol. No relation between distinct symbols asserts collision
  resistance or substitutes for exact bytes.

Manifest JSON expands every required M field (and role_output only for workers),
including normalized material W, per-unit effective checks, tree entries, full
capture provenance, assertions, prerequisites and section accounting. Counts
are actual UTF-8 sizes of the authored expectations, not measured native output.
W omits optional prose/checkpoints/observations, and all source output bytes.
The base identity remains material. These templates specify expected identities
symbolically and cannot establish canonical encoding, hashing, validation,
confinement or successful rendering. A future envelope P uses version 1,
mode preview, workspace/spec quota, the table's job/role/kind, sorted work IDs,
and the matching Markdown; these M files are not whole render envelopes.

[empty.json](observations/empty.json) and [recovery.json](observations/recovery.json)
are real schema-shaped inputs with no target claims. The reviewer template has
an explicitly inconclusive driver assertion and a note, never a pass receipt or
approval. Substitute all target/vector symbols only after native capture computes
matching current unit identities. The manifest's observation file size/bytes/hash
must then be regenerated from those substituted bytes too; do not simply swap
strings in a manifest and call it verified. Recovery uses a valid checkpoint-only
input, shows current source and the actual base differences, and states that
history is unavailable and no run is active.

## Explicit replacement invocations

`REPLACEMENT` must be an explicit absolute path to the built replacement
(for example `build/rewrite-debug/bin/planar` resolved from the repository),
not an installed legacy `planar`.
Run from this fixture directory; relative CLI host paths are supported. The
source/base/workspace are disjoint. Write envelopes outside captured trees.

```sh
REPLACEMENT=/absolute/path/to/built/replacement/planar
"$REPLACEMENT" render --workspace workspace/workspace.lua --registry registry/quota-tools.json --root source=current --capture-outputs --base-root source=base --job combined --role builder --observations observations/empty.json > /tmp/quota-combined-builder.json
"$REPLACEMENT" render --workspace workspace/workspace.lua --registry registry/quota-tools.json --root source=current --capture-outputs --base-root source=base --job split --role library_worker --observations observations/empty.json > /tmp/quota-library.json
"$REPLACEMENT" render --workspace workspace/workspace.lua --registry registry/quota-tools.json --root source=current --capture-outputs --base-root source=base --job split --role cli_worker --observations observations/empty.json > /tmp/quota-command.json
"$REPLACEMENT" render --workspace workspace/workspace.lua --registry registry/quota-tools.json --root source=current --capture-outputs --base-root source=base --job split --role cli_worker --observations observations/recovery.json > /tmp/quota-recovery.json
```

Reviewer previews can use `observations/empty.json` immediately when the native
command exists (they will have explicit assertion absence). To obtain the authored
reviewer assertion expectations, first capture outputs, produce a valid externally
stored observation JSON by replacing `$O_COMMAND` and `$O_LIBRARY` in the symbolic
reviewer template with computed exact identities, then invoke:

```sh
"$REPLACEMENT" render --workspace workspace/workspace.lua --registry registry/quota-tools.json --root source=current --capture-outputs --base-root source=base --job combined --role reviewer --observations /tmp/quota-reviewer-resolved.json > /tmp/quota-combined-reviewer.json
"$REPLACEMENT" render --workspace workspace/workspace.lua --registry registry/quota-tools.json --root source=current --capture-outputs --base-root source=base --job split --role reviewer --observations /tmp/quota-reviewer-resolved.json > /tmp/quota-split-reviewer.json
```

A context-only preview drops both capture flags and reports output/base absence;
it cannot consume target assertions. The captured packet examples above deliberately
exercise more content than that minimal mode.

## Present source verification

From `fixtures/render/quota/current`:

```sh
PYTHONDONTWRITEBYTECODE=1 python3 -B -m unittest discover -s tests -p 'test_contract.py' -v
PYTHONDONTWRITEBYTECODE=1 python3 -B -m unittest discover -s tests/cli -p 'test_integration.py' -v
```

R03 author execution: **6 contract tests and 1 integration test passed**, zero
skips. The integration test contains three real CLI subprocess scenarios; library
contention tests 100 calls against 17 units. These results are separate local
source evidence, never fabricated as Planar observations. The same 6 + 1 tests also passed with the registry
replacement environment (`env -i PATH=/usr/bin:/bin PYTHONDONTWRITEBYTECODE=1`).
Isolated own-output staging passed 4 library and 2 command contract tests.
Static verification covered 4 Lua syntax checks, 10 JSON parses, 10 Python AST
parses, 6 packet/manifest pairs and 27 Markdown links, including section byte
sums, captured content, fences, symbol consistency, recovery stability, regular
file/alias checks and whitespace. The round record preserves verification details. Python checks
are fixture tests, not a product build. There is no fake renderer/schema validator
or protocol model here. Lua syntax, JSON parsing, manual schema/reference/phase
review, packet inclusion, fence/link/whitespace and manifest accounting checks
cannot prove native isolation or hash correctness. See [coverage](coverage.md)
and [mutation expectations](mutations.md); all native mutation results are deferred.
