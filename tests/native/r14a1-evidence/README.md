# R14-A1 candidate and raw verification evidence

Shared checkout: `<repo>`, branch
`dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The intentional uncommitted rewrite
was preserved. No Planar tools, installed state, commits, installation or
publication were used. The coordinator owns round and planning records.

The ten-file candidate is frozen in `candidate/`; `candidate.json` maps paths
relative to the repository root to SHA256, with an identity computed from sorted
compact JSON of that map. `candidate.diff` compares exactly against the previous
approved snapshots copied into `baseline/`. `baseline.json` identifies their
repository-relative historical sources and hashes. New interfaces/core tests have
an empty baseline. Candidate identity:
`f8fc71887c97f775d40556e1a6f15177759620f348f42d7f3a188621315305be`.
The final evidence manifest uses paths relative to this evidence directory and
excludes itself to avoid a recursive hash.

The change extracts inert DTOs/errors into `ledger.types` and exact shared tree
algorithms into `history.tree_core`. `ledger.format` retains explicit fully
qualified aliases and all storage ownership/policy. Existing `history.tree`
signatures remain owner adapters, and re-export the core DTOs/digest. Each writer
adapter constructs the abort guard before callback conversion or allocation;
callbacks invoke `intern`/`read_record` on the same original owner. Pinned readers
therefore retain sticky storage failures. The core has no SQL, raw handles,
retained callbacks, retries, hidden commit or storage cleanup. Callbacks return
codec-validated records/exact identities; core algorithm state is per call.

Module graph: `ledger.types -> history.record -> canonical/value/xxhash`;
`history.tree_core -> ledger.types + value`; `ledger.format -> ledger.types +
ledger.sqlite + json_codec`; `history.tree -> ledger.format + history.tree_core`.
The exact core test link is in `core-link.txt`; it contains no format, SQLite,
runtime, embedded source or Lua library. Algorithm audit compares every budget,
error helper, node DTO, construction, reconstruction and digest body to the
approved source, allowing only whitespace, leaf qualification and explicit reader
callback signature/invocation changes.

`commands.json` records exact argv, cwd and exit for direct serialized debug
build/check commands. `probe-commands.json` records independent probe compile,
link and execution. Builds used `all planar_tests`; test executables remain out
of default `all`. Focused original and final runs pass 113 cases: all 109 exact
prior case names plus four pure core cases, zero skips. CLI acceptance passes one
CTest case containing the existing 247 CLI invocations. Both vectors pass. Active
formatter/Doxygen checks cover 54 files; supplementary two-file helper lint
passes. The unchanged R13-C1 allocation/abort probe was copied, compiled against
the current wrapper link and passed 46 assertions; object/binary hashes were
recorded before those temporary artifacts were removed. Existing storage-prefix
abort, typed failures, sticky readers, profile/resource bounds, portable digests
and unsupported populated-history refusal tests remain included.

`protected.json` maps 4,081 current unowned source/vendor/fixture/vector/helper/
prior-evidence files relative to the repository root, captured after initial
owned edits. `inherited-protected.json` independently verifies 11,867 prior
approved R13 hashes; explicit exclusions cover owned candidate paths, Finder
metadata and coordinator planning. `coordinator-baseline-differences.json` records
older/current hashes for implementation, ledger contract and roadmap: the
coordinator intentionally updated these after R13 approval and before A1.
No such file was edited here. C2/B1 exact fresh reversible generation equals the
configured source/interface/manifests; fully qualified leaf aliases survive the
unchanged namespace substitutions. Generator/helper sources and prior evidence
remain byte-identical. `no-work.log` requires exactly zero Ninja steps.

Audit corrections are retained: `freeze-initial.log` rejected an outdated B1-C01
format baseline; the exact B2 snapshot matches the later approved protected hash.
`freeze-protected-initial.log` rejected the coordinator's intentional newer
planning files; their current hashes and explicit exclusion are recorded.
`audit-initial.log` selected C2's initial 108-case XML; the corrected audit uses
its final 109-case XML and verifies all names. These were evidence-path/baseline
corrections, with no product failure or historical evidence rewrite.

Verification is host macOS debug only. Release/Linux were not run for this
behavior-preserving unit; integrated R14 milestone gates remain outstanding.
No accepted-history support, schema, SQLite/format implementation, codec,
provenance, interpretation, Lua or CLI behavior changed. Debug is released for
independent review when the coder report is delivered. This report does not
approve the candidate.
