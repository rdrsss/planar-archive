# R10 — Operator evaluation package

**Operator response pending.** This is a guided inspection of actual render-only
packets, with measured identity examples. It records no operator assessment,
completion of R10, or approval to implement persistence. The coordinator records
feedback and the disposition in [R10](R10.md).

## Read the six actual packets

These are fresh native CLI captures, byte-for-byte equal to the approved R08-B
goldens. Each JSON link contains the full packet and recomputable manifest;
Markdown is the exact `packet.markdown` extracted from that envelope.

| Read in this order | Actual Markdown | Full JSON | What to inspect |
| --- | --- | --- | --- |
| Split library worker | [library](../../../tests/native/r10-evidence/debug/split-library_worker.md) | [envelope](../../../tests/native/r10-evidence/debug/split-library_worker.json) | Atomic API requirement, exclusive library grant, contract check and own source/base difference. |
| Split command worker | [command](../../../tests/native/r10-evidence/debug/split-cli_worker.md) | [envelope](../../../tests/native/r10-evidence/debug/split-cli_worker.json) | CLI requirement, contract-first start, upstream facet, independent command grant and integration obligation. |
| Combined builder | [builder](../../../tests/native/r10-evidence/debug/combined-builder.md) | [envelope](../../../tests/native/r10-evidence/debug/combined-builder.json) | Both grants and per-unit checks in one session; integration remains an internal obligation. |
| Split reviewer | [reviewer](../../../tests/native/r10-evidence/debug/split-reviewer.md) | [envelope](../../../tests/native/r10-evidence/debug/split-reviewer.json) | Full exact target vector, complete selected old/new source, unverified assertions and no worker grant. |
| Combined reviewer | [reviewer](../../../tests/native/r10-evidence/debug/combined-reviewer.md) | [envelope](../../../tests/native/r10-evidence/debug/combined-reviewer.json) | Same unit targets/check applicability, with combined worker mapping. |
| Split command recovery | [recovery](../../../tests/native/r10-evidence/debug/split-cli_worker-recovery.md) | [envelope](../../../tests/native/r10-evidence/debug/split-cli_worker-recovery.json) | Current captured source plus bounded unverified checkpoint; identical work/output identities to ordinary command preview. |

The same spec supplies both jobs. Library exclusively owns `src/core/` and
`tests/core/`; command exclusively owns `src/cli/` and `tests/cli/`. Grouping both
under builder preserves these unit boundaries. Reviewers receive target
boundaries rather than permission to write them. `contracts/` is frozen,
`generated/` excluded, and the remainder read-only. Both groupings preserve the
[requirement/check coverage](../../../fixtures/render/quota/coverage.md).

For example, command requirements include this exact declaration:

> Write one remaining=&lt;integer&gt; line on success or exhaustion; write invalid arguments to stderr only.

Check applicability in both reviewer packets is explicit:

```text
work:command: contract, integration.
work:library: contract.
```

The trusted registry describes `python3 -B -m unittest discover -s tests -p
test_contract.py -v` and, for integration, `-s tests/cli -p test_integration.py -v`,
from the source root. The full packets retain exact argv, environment, timeout,
nonempty-selection and coverage descriptors. Contract checks use expected
contract and own output; integration uses the exact command/library output
vector. A future driver must stage the applicable own-output tests and provide
a reviewed Python environment. These renders execute no checks.

Both reviewers target exactly command
`xxh3_128:26251223d01bac7281172de5abd61dc4` and library
`xxh3_128:799d60775f9f630bf87d9f3fee5c8bd4`. Their supplied inconclusive check and
finding are labeled `driver assertion, unverified`; those assertions are not
receipts. Recovery states:

> Session note supplied for this preview; history unavailable; no run is active.

Its note proposes “Inspect actual library/CLI integration and independently
verify selected tests.” It supplies no durable execution history.

## Measured identity demonstrations

`W` below is the work fingerprint, distinct from fragment/render, unit output,
role output and whole-checkout observation identities. Digest prefixes are
abbreviations for reading; the [raw before/after table](../../../tests/native/r10-evidence/debug/comparisons.json)
contains every complete digest, expected changed/stable set, changed section and
envelope filename. Each linked diff shows the concrete changed Markdown.
Section comparison ignores only trailing structural LF separators.

| Isolated change | Measured W before → after (prefix) | Other measured effects | Concrete section diff |
| --- | --- | --- | --- |
| Optional workspace note | command `915ccd27 → 915ccd27`; library `56eb9006 → 56eb9006` | Both render identities change; all outputs, role outputs and checkout observation stay equal. | [workspace_notes](../../../tests/native/r10-evidence/debug/notes-command.markdown.diff) |
| Add “Preserve a trailing newline.” to command_rule | command `915ccd27 → 94bff3ad`; library `56eb9006 → 56eb9006` | Command render changes; library render and all output/observation identities stay equal. | [authority, requirements](../../../tests/native/r10-evidence/debug/requirement-command.markdown.diff); [unchanged library](../../../tests/native/r10-evidence/debug/requirement-library.markdown.diff) |
| Add comment to command implementation | command `915ccd27 → 915ccd27`; library `56eb9006 → 56eb9006` | Command unit/role output, observation and render change; library unit/role output stay equal. Library render changes only its global checkout line. | [command outputs](../../../tests/native/r10-evidence/debug/own-command.markdown.diff); [library checkout line](../../../tests/native/r10-evidence/debug/own-library.markdown.diff) |
| Add comment to upstream library, original complete/done facet | command `915ccd27 → 915ccd27`; library `56eb9006 → 56eb9006` | Library output/role output and global observation change; command output/role output stay equal. Command render shows changed facet/source bytes; no start facet enters W. | [dependencies, outputs](../../../tests/native/r10-evidence/debug/complete-command.markdown.diff) |
| Same upstream comment, start/available facet variant | command `e9c714a1 → 1ecf1c72`; library `56eb9006 → 56eb9006` | Captured start-facet semantic identity and consumer W change; producer W stays equal. Command output/role output stay equal; library output and both renders change. | [authority, dependencies, outputs](../../../tests/native/r10-evidence/debug/start-command.markdown.diff) |
| Recovery checkpoint instead of empty observations | command `915ccd27 → 915ccd27` | Render changes; unit/role outputs and observation stay equal. Checkpoint is unverified session data. | [checkpoint](../../../tests/native/r10-evidence/debug/recovery.markdown.diff) |

The start-facet variant is a deliberately different valid scratch declaration:
it changes integrate to start/available and changes command's explicit check
from integration to contract. The integration check requires a completion facet,
so retaining it with only start facets would be invalid. The table compares the
same valid start declaration before/after source edits; it does not attribute
policy changes to a source edit. Exact [before patch](../../../tests/native/r10-evidence/debug/start-before.patch)
and [after patch](../../../tests/native/r10-evidence/debug/start-after.patch) are retained.
The [complete-only patch](../../../tests/native/r10-evidence/debug/upstream-complete.patch)
changes source alone. No fixture, product schema, contract or golden was edited.

## Reproduce and inspect evidence

From the repository root, choose new output directories (the helper refuses an
existing directory). The binary argument resolves to an explicit executable;
there is no installed `planar` lookup.

```sh
python3 -B tests/acceptance/render_evaluation.py build/rewrite-debug/bin/planar /tmp/planar-r10-debug-new
python3 -B tests/acceptance/render_evaluation.py build/rewrite-release/bin/planar /tmp/planar-r10-release-new
```

Recorded runs: [debug log](../../../tests/native/r10-evidence/debug.log),
[release log](../../../tests/native/r10-evidence/release.log),
[debug provenance](../../../tests/native/r10-evidence/debug/provenance.json),
[release provenance](../../../tests/native/r10-evidence/release/provenance.json),
[verification](../../../tests/native/r10-evidence/verification.json).
Each run performs **19 real CLI invocations, six exact golden comparisons and
11 identity comparisons, zero skips**. Each also repeats a command render in a
fresh process. Debug/release packet, patch and comparison bytes agree.

The [helper](../../../tests/acceptance/render_evaluation.py) copies only invocation
inputs into isolated temporary sibling trees. It uses a minimal explicit
environment and a scratch HOME/database sentinel, verifies inputs and HOME
unchanged after every render, and checks that no host paths enter payloads.
[Commands](../../../tests/native/r10-evidence/debug/commands.json) record argv and
case working directories. [Input hashes](../../../tests/native/r10-evidence/debug/fixture-inputs.json),
all mutation patches, and exact captured file bytes in every envelope identify
the inputs. Reviewer observations are [resolved from actual outputs](../../../tests/native/r10-evidence/debug/reviewer-resolved.json).
The independent vector encoder and scratch-compiled pinned upstream XXH3 C
recompute file, base, observation, unit/role output, contract, work, fragment and
start-facet identities. The helper never imports the acceptance runner, which
executes at import time.

## Operator questions and limits

Please assess these actual packets:

1. What context is missing for the library or command worker to act? Which
   repeated source, identity or authority content distracts from that work?
2. Are exclusive write boundaries and read-only upstream dependencies clear,
   including the builder's internal integration obligation?
3. Are per-unit checks, exact input vectors, environment assumptions and partial
   coverage descriptions actionable for an external driver and reviewer?
4. Does combined versus split grouping improve useful context for this example?
   Does recovery give enough current context without suggesting past execution?

Coder assessment for discussion, **not operator judgment**: the global checkout
identifier causes fragment churn outside a worker's selected source, as the
own-output example shows. Exact full source and compact registry JSON can be
verbose. These may deserve presentation changes if the operator finds them
costly; no such change is proposed as already approved.

This package evaluates a small trusted quota fixture and deterministic preview
behavior. It does not measure model task performance or prove quota correctness.
Render has no checks execution, model calls, active run, history, authenticated
approval, scheduler or ledger. Native execution is validated on macOS ARM64;
Linux remains unvalidated. Cooperative capture is not an atomic snapshot.
Native serializer tests cover the exact 16 MiB boundary, while large CLI inputs
hit trusted instruction limits first; this package adds no end-to-end size-limit
claim. Registry test/toolchain coverage remains declared and limited. Persistence
requires the operator's actual evaluation and resolution of required changes.
