# R10 independent preparation review

Disposition: **approve — evaluation preparation only**. No blocking findings.
The operator's evaluation of actual fragments remains pending. This review does
not complete R10, establish product usefulness, or permit persistence work.

## Candidate and authority

Fresh independent reviewer `/root/r10_reviewer`, shared checkout
`dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Read the active agent guide,
implementation plan, roadmap, v4 design, R10 brief/evaluation guide and raw
evidence. Preserved existing changes. No product edits, Planar tools, legacy or
installed binaries, delegation, product builds, commits, installation or publication.
Reviewer writes are confined to this report and
`tests/native/r10-review-evidence/`.

Candidate manifest `tests/native/r10-evidence/candidate.sha256` has SHA256
`5660d992f6927c820f63011cba09ce67a2b43f0786e4380dd039e40abdf59d0c`.
Its four files matched before and after review. Evidence manifest SHA256 is
`37f236eb24bf30e2f81f6122cc53703952264736e29081acebf1ce7ba97f6275`;
all 166 entries matched before and after review. Of R09's 1,296 frozen paths,
1,294 remain byte-identical; only the intended `docs/testing.md` count correction
and Doxygen retry comment differ. Product, original fixtures and goldens remain
unchanged. The count correction agrees with R09's raw 150-process/60-host-path
results; the retry comment agrees with the script's default of three attempts.

## Assessment

- The guide links all six real combined/split worker, reviewer and recovery
  packets. Their JSON bytes equal the approved native goldens, and each Markdown
  file is exactly its envelope's `packet.markdown`. Ownership, frozen/excluded
  paths, reviewer target boundaries and exact command/library targets agree with
  the packets. Checks retain per-unit applicability, argv, environment, timeout,
  nonempty selection, input vectors and limited coverage; the guide explicitly
  leaves execution and staging to a future driver.
- Optional notes move renders without moving work/output identities. The command
  requirement moves command W/render while library W/render remains stable. Own
  command source moves its outputs without moving either worker W. The guide
  truthfully discloses that library Markdown still changes its global checkout
  line; the raw diff and helper's normalized-text comparison confirm that this
  is the only library Markdown difference.
- Original complete/done upstream facets leave consumer W stable after the
  library edit. The separate valid start/available declaration moves consumer W
  after that same source edit while producer W remains stable. Before/after
  patches preserve the shared policy change from integration to contract, so
  the demonstration does not confound policy changes with source changes.
- Reviewer observations are resolved from actual output identities and remain
  unverified assertions. Recovery supplies current captured data and an
  explicitly unverified session note, without claiming execution history,
  approval or an active run. Operator questions address missing/distracting
  context, boundaries, checks, grouping and recovery; no response is fabricated.
- The helper imports only the side-effect-free vector module, compiles pinned
  upstream XXH3 C in scratch, and recomputes the recorded identity domains. It
  invokes an explicit binary with a minimal environment and scratch HOME/database
  sentinel. Mutations affect temporary input copies; each render checks input
  and HOME preservation, and final checks preserve original fixtures and binary.
  Exact envelopes, patches, commands, input hashes and provenance permit replay.

## Independent verification

Executed serially, without building:

```sh
python3 -B tests/acceptance/render_evaluation.py build/rewrite-debug/bin/planar tests/native/r10-review-evidence/debug
```

Exit 0: **19 actual CLI invocations, six exact native golden comparisons,
11 identity comparisons, zero skips**, independent hashes verified, input/HOME
unchanged, fresh-process repeat equal. Binary SHA256:
`35a8bc2095f014087f7e1771d3b136d078449291ea0aafca447679133bd483f5`.
All 79 artifacts other than provenance match both recorded configurations;
independent debug provenance also matches exactly. Independently checked all
35 relative guide links, six Markdown extractions/golden envelopes, reviewer
grant/check wording and helper syntax compilation to scratch. Results are in
`tests/native/r10-review-evidence/verification.json` and the reproduced outputs.

The approved R09 full build/test gates were not repeated for this helper,
documentation and comment scope. Independent execution here covers debug on
macOS ARM64; release evidence was inspected and byte-compared. Linux remains
unvalidated. This package measures deterministic preview behavior of a small
trusted fixture, not model task performance or quota correctness. Cooperative
capture, size-limit and declared registry/toolchain limitations are accurately
retained. The coordinator may present this package for actual operator feedback;
R10 acceptance and the persistence boundary remain open.
