# C05 reduced test-budget calibration

Status: exploratory calibration handoff, **not green and not certified**. The
coordinator stopped final calibration pending C06 reconciliation source changes
and assigned the follow-up to a fresh agent. All numbers below describe the C05
source snapshot; they are not final acceptance pins.
Production runtime ceilings and budget charges are unchanged. This continuation
preserves the preceding C05 catalog/options, completion-backed reconcile and
format3 init fixture corrections.

## Authority and method

X06-D-C04 D6–D11 and D26–D29 govern the five measured columns. Exclusions remain
per column: an actual refusal/bisect or a deliberately ceiling-sized family does
not exclude the other columns. The existing manual instruction exclusion for
`extension owners created without a factory` remains the paired control of the
factory refusal. Native sweeps are included in the instruction margin.

The source was frozen after the separate C05 API correction restored the closed
`prepare_load(intent)` surface. Scratch instrumentation adds private high-water
counters and destruction-time logs only; no public fields or product hooks. The
external copies under `<scratch-root>/planar-c05-scratchpad-{debug,release}-{normal,raised}`
compile instrumented implementation/test objects, replace the runtime object in
a copied static archive, and relink existing stable Ninja inputs. All commands
run in an assigned build lane, serialized with the corresponding ordinary build.
Main build artifacts are never instrumented. Each case runs in its own process.

Raw commands, scripts, outcomes and RUN records are under
`tests/native/r15-r18-c05-evidence/calibration/`. `build_measure.py` records exact
compile/link commands and source hashes. `probe_measure.py` drives the historical
scratch-only probe cases. `aggregate.py` applies the existing five-column rule.
Raised Debug limits (256 MiB, 400 million instructions, 100 million registration
instructions, 4 million nodes) provide slopes only, never ordinary-margin rows.

## Baseline and sizing

The unmodified-limit Debug runtime suite ran **484 cases, 480 passing, 4 failing**,
84,146 assertions: the quarter-free control, old outputs-off pin, padded C03
registry control and N30 projection control. No fixture protection was removed.
Scratch baseline runs in both configurations found the same five-column peaks.
Two extra scratch source-inspection failures were missing copied source files;
copying `runtime.cppm` and `src/stdlib` and rerunning those cases passed. These
were instrumentation packaging defects, not product failures.

Measured ordinary padded native cost is 31,010,364 bytes; another 20,000 padding
bytes adds 460,000 native bytes and 440,000 instructions: slopes **23 and 22**.
Ordinary role preparation uses 20,565 nodes without outputs and 20,881 with them.
The 200,000-byte registry/observation controls cost 43,260,093/41,860,117 native
bytes. The unchanged large-Lua refusal fixture reaches **55,311,987 live bytes**
in the raised copy, above the proposed small ceilings and below the inherited
64 MiB loader bound.

The limits are coupled. The first derivation below omitted the projected N30
node peak; pass 2 rejected its 48 MiB conclusion. It is retained as exploratory
evidence, not a valid final margin table. For each candidate memory ceiling M, the native sweep
requires invocation instructions at least
`4/3 * (131200 + 22 * floor((M - 31010364)/23))`, rounded upward to one million.
The resulting declaration-family length sets its ordinary node use; that use
requires a node limit rounded upward to 500. The node-bisect passing side then
sets an ordinary native peak. Measured family slopes are 430,189 native bytes and
692 nodes per added job. The node fixture slope is 2,750 native bytes per entry,
with five nodes per entry. Optimistic candidate calculations (smaller instruction
ceilings than the old ceiling where applicable) already reject every MiB step:

| MiB | derived instructions | required nodes | node-boundary native | family native | native margin passes? |
|---:|---:|---:|---:|---:|:---|
| 40 | 15,000,000 | 24,500 | 32,777,470 | 33,095,066 | no |
| 41 | 16,000,000 | 25,500 | 33,327,470 | 33,525,255 | no |
| 42 | 17,000,000 | 26,500 | 33,877,470 | 33,955,444 | no |
| 43 | 19,000,000 | 28,000 | 34,702,470 | 34,815,822 | no |
| 44 | 20,000,000 | 29,000 | 35,252,470 | 35,246,011 | no |
| 45 | 21,000,000 | 30,000 | 35,802,470 | 35,676,200 | no |
| 46 | 23,000,000 | 32,000 | 36,902,470 | 36,536,578 | no |
| 47 | 24,000,000 | 33,000 | 37,452,470 | 36,966,767 | no |
| 48 | 25,000,000 | 33,500 | 37,727,470 | 37,396,956 | yes |

Below 40 MiB the ordinary padded control alone lacks 25% margin. The initial
48 MiB / 25 million / 33,500 candidate passed the ordinary/native sweep checks
but **failed** the N30 projection node margin. Its measured projection peak is
25,418 nodes, requiring at least 34,000; that node boundary costs 38,002,470 native
bytes, above 75% of 48 MiB (37,748,736). Thus 48 MiB is also rejected.

The next **unverified** coupled candidate is 49 MiB (51,380,224 bytes),
26,000,000 invocation instructions and 34,500 nodes. Raised probes measured
sink/first-family native peaks 37,826,985/37,827,145 and 25,791 nodes. The node
boundary is predicted at 38,277,470 native bytes (75% allowance 38,535,168).
Only final normal-limit Debug/Release runs on frozen C06 source can establish
whether these predictions hold. Registration stays at 1,000,000 (measured use
5,000), and the tight 500,000-instruction variant remains unchanged.

## Dependent controls

The proposed sink uses 22 extra jobs, the first-error family 21, and the
refusing capture family 22. The across-stage family retains ten extra jobs and
uses a 6,670,000-iteration shape loop. The first-error prefix is 999,000: raised
controls have capture marks 25,106,300/25,206,300 for last chunks of
100,000/200,000, putting equality at `100000 + 26000000 - 25106300 = 993700`.
Earlier arithmetic for prefix997,000 was incorrect; the pass-2 failing bisect
caught it. Prefix999,000 still opens/prepares below inherited per-chunk limits
in the raised controls. The role-tail loop becomes 24,500,000.

The route fixture's former twice-boundary probe reached 1,228 jobs and crossed
the text quota, masking instruction refusal. The new probe is 1.5 times the
boundary, with an explicit workspace-text size assertion. Raised route921 uses
917,506 workspace bytes and 37,447,400 instructions, demonstrating separation.

The 200,000-byte padding, two role-padding files, 262,144-byte C03 search window,
262,144-byte projection section and request-record pin window of eight are
retained. At the rejected 48 MiB candidate, measured C03 base boundary is 321,916; drops are current tree
3,770, base tree 3,768, registry 200,000 and observations 175,862. Tree floors are
90% of their drops; registry/observation floors retain the measured half-mutant
separation formula `floor(M * (1 - f/2))`, with f=1/58 and f=1/51 respectively.
The 48 MiB macOS off/on boundaries were 326,112/321,370 (gap4742); N30 plain
326,441/projected205,390. These absolute pins require remeasurement at49 MiB and
after C06. Linux's old pin remains unchanged and must be measured independently.

## Executed evidence and handoff

- `cmake --build build/rewrite-debug --target planar_runtime_tests -j3`: baseline
  and 48 MiB candidate builds passed. `runtime-baseline-debug.log` records
  484cases/84146assertions, 480pass/4fail, zero skips.
- `build_measure.py debug normal`, `release normal`, `debug raised`: passed;
  exact compiler/link arguments and source hashes are retained in the respective
  `pass1-release/instrumentation/` and `pass2-debug/instrumentation/` directories.
- Baseline per-case instrumented Debug and Release: 486cases,480pass,6fail each;
  four budget failures plus the two corrected scratch packaging failures above.
  `scratch-path-recheck.log` records those two cases passing. Debug results JSON
  names were recovered from each output's `Filters:` line after an exploratory
  live-summary script mistakenly paired a second randomized test listing with
  the existing outputs. Raw per-case outputs and the original runner exit summary
  remain authoritative; no measurement was relabeled from guessed ordering.
- Pass2 instrumented Debug:487cases,484pass,3fail: old off-pin, first-error family
  inherited-limit masking, route far-probe text quota. Aggregate additionally
  exposes the N30 node margin failure. Raw records are under `pass2-debug/`.
- 48 MiB boundary trio:3cases,2pass,1old-pin failure. C03 and N30 pass with the
  measured controls above; the separate outputs-gap case passes. These do not
  establish the rejected candidate's five-column margin.
- Pinned `clang-format --dry-run --Werror` passed on the three owned test files
  before the final proposed49MiB edits; rerun noted in the final handoff log.

No production runtime source or budget changed. No mutation was applied. D28
requires a separate independent prober for budget mutations after final pins.
The fresh continuation must first read C06 source-freeze instructions, rebuild
Debug/Release, run the proposed49MiB controls, remeasure all five columns, adjust
both platform pins from actual measurements, and verify the existing tight,
large-Lua, native/node/instruction, registry, tree and projection refusal controls.
Do not reuse the 48 MiB absolute pin or the optimistic earlier margin claim.
