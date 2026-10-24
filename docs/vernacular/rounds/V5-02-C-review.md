# V5-02-C independent review

Disposition: **approve**. No blocking findings or unresolved product choices.

Reviewed the frozen candidate in the shared `dev/planar-v2-vernacular` checkout,
HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, against current CLAUDE.md,
implementation/roadmap, v5 direction, the approved exact command contract and
the C round brief. B2 approval is a dependency, not reopened by this review.
Reviewer writes are confined to this report and
`tests/native/v5-02-c-review-evidence/`. No candidate fixes, reversions, historical
evidence edits, Planar tools/skills/queue, installed binaries, operator data,
commits, installation, publication or merge occurred.

## Exact reviewed identity

Independently verified the wrapped manifest `files` maps using their stated path
bases, all current candidate bytes against snapshots, and regenerated the exact
baseline-relative no-index diff. These identities were unchanged after focused
verification:

| Artifact | SHA256 |
| --- | --- |
| `tests/native/v5-02-c-evidence/candidate.json` (23 repository-relative files) | `ce208885657cf57697259de2b64cda250020121a5151e49ba980c8678df06fb0` |
| `tests/native/v5-02-c-evidence/owned.diff` | `ad338ce56d68719d65d065a3f7911b04bb490fb56d9a9933e7fd39a8db7bb708` |
| `tests/native/v5-02-c-evidence/artifacts.json` (105 evidence-relative files) | `d07dba43bca93f06eb64a5230129cab31730fa429e7b3645fb36ec6ce0248802` |

Also independently checked all 19 baseline inputs, seven handoff identities,
12 debug/release binary hashes and 3,948 protected inputs. Historical evidence,
generators, fixtures/goldens/vectors, user v5 and runtime implementation/interface
match the protected manifest. The contract body from `## Catalog and structural
ABI` onward is byte-identical to baseline; the contract diff is status-only.
`AGENTS.md` remains a symlink to `CLAUDE.md`. See
[integrity audit](../../../tests/native/v5-02-c-review-evidence/integrity.json).

## Contract assessment

- `src/engine/capture/host_path.cpp:115` implements classification without
  filesystem/environment access: slash/absolute/case-sensitive `.lua` wins,
  otherwise exact ASCII 1–64-byte names are required. Invalid bare names are
  usage before path limits. Explicit paths validate text before size, ignoring
  the environment entirely (`:124`, `:165`). Colocated tests at
  `src/engine/capture/host_path.t.cpp:128` cover name boundaries and explicit
  punctuation/suffix exceptions, including an oversized invalid bare name.
- The named environment is read only on the name branch of
  `src/cmd/planar/invocation.cpp:247`, once through its injected reader; the
  production reader at `:334` borrows getenv bytes. The helper at
  `src/engine/capture/host_path.cpp:69` checks required absolute UTF-8/no-NUL text,
  then 64 MiB size, before the owning copy at `:160`. Expansion checks bounded
  separator/name/suffix joins before concatenation. No HOME, default or discovery
  path exists. The final borrowed-view/NUL-before-size regression is present at
  `src/cmd/planar/bootstrap.t.cpp:317` and passed independently in both builds.
- `prepare_host_path` at `src/engine/capture/host_path.cpp:131` validates inputs,
  checks relative joins and normalized output bounds, preserves absolute parent
  spelling and independently produces lexical intent without filesystem reads.
  The existing `normalize_intent_path` implementation/API is unchanged. The
  physical-link/parent regression at `host_path.t.cpp:160` proves different real
  files can result from physical render spelling versus lexical intent.
- `src/cmd/planar/invocation.cpp:278` validates all selected paths before services,
  reads cwd once only when a relative selected path requires it, and prepares
  every path from that snapshot. Absolute paths never require cwd. Production
  cwd acquisition uses the standard library's owning path and then validates
  derived preparation; no unbounded derived join was introduced. Injected tests
  at `bootstrap.t.cpp:239` and `:278` cover all command families and redacted
  missing/malformed cwd errors. Real processes launched from deleted cwd cover
  absolute explicit/name success and relative refusal.
- Path preparation runs after successful CLI parsing/registration/catalog
  admission (`src/cmd/planar/bootstrap.cpp:97` and
  `src/cmd/planar/invocation.cpp:325`), before native services at `:347`.
  Existing source-mapping refusal remains ahead of routed spec/registry/root
  reads in `invocation.cpp:113`. The native options stay private: unchanged
  `src/engine/runtime/runtime.cpp:1174` creates handles/presence metadata rather
  than exporting host path strings to Lua. Source review and retained actual-CLI
  tests confirm no alternate parser, loader, policy, storage or public identity
  path was introduced.
- The historical directory-input adjustment at
  `tests/acceptance/bootstrap_paths.py:114` is the authorized single substitution
  to explicit `./workspace`. Its complete regular-file io refusal and all 60
  host-path checks remain. The historical render suite still runs 247 processes;
  the separate new suite supplies unset-name semantics. Leaf/root/captured alias
  refusal, retained capture receipts and physical parent semantics pass focused
  checks. Existing physical resolver/capture policies are unchanged.
- `src/engine/acceptance/prepared.t.cpp:350` proves pure source-bound future
  request framing: normalized name/explicit target equality, changed environment
  inequality and no required input existence. It does not implement or claim
  public load/replay. References consistently describe render names and pending
  load/full R14/external-profile work. Current init ledger syntax is unchanged.

## Raw evidence and independent verification

Reviewed final debug and release raw JUnit plus retained LastTest logs directly:
each contains 370 executed passing entries, zero skips/failures and all 361
baseline names. Six acceptance suites and 364 unit entries agree with inventory.
Full logs show pinned format/Doxygen over 89 active sources and zero-work final
builds. Historical actual CLI counts remain 247 render, 60 init, 115 history,
55 export/output and 99 retained dispatch; the new selector suite runs 41 actual
processes. These process counts are separate from CTest entry counts. The old
369-entry debug evidence remains explicitly superseded; final 370 results include
the environment ordering regression. The initial bad.name red/green logs retain
the pre-fix status4/io failure and corrected status2/usage behavior.

Ran `python3 tests/native/v5-02-c-review-evidence/review.py` with debug then release
serialized. It independently audits identities/raw gates, performs a no-change
`cmake --build BUILD --target all planar_tests`, inventories CTest, runs the
focused regex recorded in that script with raw JUnit/LastTest preservation, and
regenerates exact variants. Each configuration passed **23/23 selected entries,
zero skips/failures**: 20 native/unit checks and three actual CLI suites (247
render, 99 retained-dispatch and 41 selector processes). This focused selection
targets changed paths, intent, aliases/receipts and precedence; no blanket full
gate or new stress experiment was run.

Both exact audits regenerated all seven variant families plus fixed-export exact
runtime harness; all configured files matched, and the two audits match each
other. `python3 tests/vectors/generate.py --check` passed unchanged 12 hand byte,
20 upstream hash and 41 canonical/domain vectors. Both independent builds did no
work; recorded binary hashes and candidate/protected hashes still match afterward.
Raw commands/results are retained under
[review evidence](../../../tests/native/v5-02-c-review-evidence/summary.json),
including focused XML/logs and exact variant manifests. An initial reviewer
audit invocation used the wrong expected log substring `247 CLI processes`;
correcting it to the actual `247 real CLI processes` affected only the reviewer
script, before successful verification. It was not a product test failure.

Evidence remains macOS ARM64/Clang23.1.4 only (release is RelWithDebInfo); no Linux,
hardware power-loss or public load/replay completion is inferred. Remaining R14
and the mandatory subsequent external-profile gate retain their existing scope.

**Both `build/rewrite-debug` and `build/rewrite-release` are idle and explicitly
released by the reviewer.** Approval applies to the exact identities above;
edits affecting that candidate require renewed independent review.
