# R10-P01 independent review

Decision: **approve**. No blocking or nonblocking product findings.

Reviewed in the shared `dev/planar-v2-vernacular` checkout. Product and candidate
inputs remained read-only. Reviewer writes are confined to this report and
`tests/native/r10p01-review-evidence/`; no other agent's edits were reverted.
No Planar operational tools, installed binary, commits, installation, publishing,
check execution or persistence operation was used. Coordinator-owned planning
records are outside this approval's frozen candidate.

## Exact reviewed candidate

The 21-file candidate manifest is
`tests/native/r10p01-evidence/candidate.sha256`, SHA256
`1780d0070543754fe4afee9e503cd47099b1fb0772018f029d8c8691165ddaa8`.
All entries matched before and after independent verification. Also verified:

| Manifest | Entries | SHA256 |
| --- | ---: | --- |
| `evidence.sha256` | 192 | `53e173c8af9aefd8db69fb3dca51ebfc8fb609be2c4cd5c54c2d69b11bddb392` |
| `protected-source.sha256` | 3,032 | `a26fe2ad56316e0eb8d9e5f3539c9296c67317f0229aa84b48390beb5628722c` |

See reviewer evidence `initial-manifests.json` and `final-manifests.json`.
The protected audit accurately distinguishes the 2,312 historical matches, the
known coordinator-finalized N04 record difference, and 719 paths with only a
current freeze; it does not fabricate a missing P01 protected prehash.

## Contract and implementation assessment

Read repository instructions, implementation plan, roadmap, assigned round,
v4 baseline, current render contract, projection and framing helper, changed
native cases, CLI/evaluation verification and actual six before/current packets.

- `src/stdlib/planar/render.lua:62` presents each dependency once, retaining
  source/reference, producer/consumer and roles, phase/quality, internal versus
  cross-role obligation and prerequisite status. Referenced facet policy and
  availability, pipeline predecessors and blocking questions remain visible.
- `src/stdlib/planar/render.lua:84` retains effective checks by work unit and all
  eight check fields. Toolchains occur once with command, environment and full
  description. Exact argv boundaries use Glaze JSON data; coverage and description
  are framed prose. No executable shell quoting or stronger evidence claim is
  invented.
- `src/stdlib/planar/render.lua:100` retains full selected source and old/new,
  deleted/binary and mode/byte information. The reviewer target vector stays
  ordered and complete, without duplicate unit-output lines. One work fingerprint
  remains in authority. Global capture/base and file hashes remain in M.
- The existing `presentation_data` helper dynamically fences content and labels
  display control escaping. Free text stays in data blocks; inline IDs/paths use
  the validated restricted grammars. Assertion target/vector records remain
  visible and unverified. Preview, missing output, dependency quality and recovery
  labels make no execution/history/approval claim.
- `src/stdlib/planar/render.lua:159` preserves byte accounting, mandatory refusal,
  omission order and notices. `:195` appends only `projection_v2` to M versions;
  W and all identity domains retain their prior content/versions.

An independent six-pair audit confirms exactly `render`, `sections`, `versions`
change in M; every other manifest value and all packet metadata are exactly
equal. It checks full applicable requirement/acceptance text, check coverage,
toolchain descriptions, dependency policy/mappings and reviewer ordering. The
original R03 inputs/expectations and prior evidence remain protected and intact.
See `independent-preservation.json`. All six Markdown packets are smaller than
their frozen predecessors while retaining those obligations.

## Verification and raw evidence

Inspected coder raw debug/release gates: each ran 124 registered tests with zero
skips, formatting/Doxygen on 29 source files, and no-work final incremental build.
CLI evidence ran 247 processes and six goldens, including 60 host-path checks.
Each evaluation ran 19 invocations, six goldens and 11 identity comparisons;
79 nonprovenance artifacts match across builds. The regression probe records a
failure against frozen old CLI output and success against current output,
accurately described as an output-level comparison rather than rebuilding old
source. These raw results agree with the handoff summaries.

Independently executed, serialized in debug:

```sh
cmake --build build/rewrite-debug --target all planar_tests
build/rewrite-debug/src/engine/planar_runtime_tests '[render]'
python3 tests/acceptance/render_evaluation.py build/rewrite-debug/bin/planar tests/native/r10p01-review-evidence/evaluation
python3 tests/native/r10p01-review-evidence/independent_probe.py
```

- Incremental build: `ninja: no work to do` (`build-debug.log`).
- Native render: 18 cases / 28,070 assertions passed (`render-tests.log`).
- Fresh real CLI evaluation: 19 invocations / six exact goldens / 11 identity
  comparisons, independent canonical/hash checks, inputs/HOME unchanged and zero
  skips (`evaluation.log`, `evaluation/`). Unrelated command output leaves the
  library packet/render stable while its global observation changes.
- Independent adversarial CLI probe: five invocations, exact argv/environment
  roundtrip with spaces, quotes, slash, newline, nine-backtick runs and controls;
  forged headings remain fenced; full selected source, binary hex, deleted old
  content and changed executable mode survive. Section sums match UTF-8 bytes.
  Exact full budget succeeds; exact mandatory budget 7,611 succeeds with declared
  optional omissions; 7,610 refuses. Inputs and scratch HOME remain unchanged
  (`independent-probe.json`, `adversarial.json`).

The probe's first two runs exposed reviewer-oracle mistakes: counting JSON-escaped
newlines as raw headings, then expecting unchanged source twice. The oracle was
corrected to the documented framing and once-only unchanged content contracts;
failure logs are retained as `independent-probe-first.log` and
`independent-probe-second.log`. No product change resulted.

Limitations: independent execution and coder full gates are macOS Apple Silicon;
Linux was not executed. This review approves presentation only. Ledger/history,
finite subprocess checks, claims and durable recovery remain later work and gain
no validation from render-only success. No broader rerun was needed after the
bounded checks passed without product findings. Coordinator must verify the same
candidate manifest before advancing and obtain renewed review for edits affecting
this scope.
