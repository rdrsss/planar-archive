# C07 reduced test-budget calibration

Status: final host calibration complete. Debug and RelWithDebInfo each pass 509 cases / 87,114 assertions, with 31 budget fixtures and zero exact five-column checker problems. This is calibration evidence, not independent review approval. Production ceilings are unchanged. This package owns only reduced test variants and dependent verification fixtures; coordinator final gates, Linux validation and independent mutations are tracked separately.

Authority: X06-D-C04 D9–D11 and D26–D29. Use smallest qualifying MiB native ceiling below64MiB and below the unchanged deliberate big-Lua refusal; derive invocation and node ceilings from measured coupled families. Every ordinary column must have at least25% headroom in Debug and RelWithDebInfo, with documented per-column exclusions only. Final instruction pins are measured, never inferred from another platform. The minimum native search begins at 40 MiB because the ordinary control alone requires at least 41,495,952 bytes for one-quarter headroom.

Inherited C05/C06 measurement evidence remains exploratory. C06 exposed the loader GC-retry terminal-error bug now corrected by C07; the unchanged big-Lua refusal passed in the integrated Release package. C06 improved exact-integer measurement checker and immutable inventory runner are copied into this evidence folder; no prior outputs are overwritten. Private instrumentation uses copied coherent objects/PCMs and an external scratch archive. Source identity before measurement is source-before.json.

Execution starts with coherent Debug runtime/loader build, focused shipped ordinary control and raised scratch family/node/route sizing. Full normal-limit per-case measurements follow the chosen constants in both configurations. Independent budget mutations and root-owned Linux exact pin/full gates follow; no final claim until their evidence exists.

## Focused sizing and tooling correction

Coherent Debug baseline build passed. The49MiB quarter-free test correctly failed its derived instruction margin (1case36assertions,35pass/1fail). C07 ordinary control costs31,121,964 native bytes and131,200 instructions; open costs22,643,664 native bytes and5,000 instructions. Raised probes reproduce these exact values and slopes23native/22instructions per padding byte.

Coupled candidate51MiB/29M/37,500: measured node-boundary peak39,998,112bytes versus40,108,032 allowance; sink/first family27,873nodes and39,229,152/312bytes.50MiB needs28M and36,500nodes whose boundary39,448,112 exceeds39,321,600 allowance. Lower candidates fail the same coupled calculation (coupled-model.json). Full normal-limit measurements must still establish all other columns.

Dependent fixtures use25 sink/native extra jobs and24 first-error jobs. Raised first-error capture total28,107,300 at last100,000 puts exact equality at992,700, below the inherited1M chunk bound. Across-stage loop8,170,000 and role tail27,500,000 retain their declared discrimination. Route900 uses896,590workspace bytes and36,595,300 instructions; the far confirmation becomes1.25 times boundary to stay below the independent1MiB text quota while still refusing the29M instruction limit.

An additional evidence-tool defect was corrected: inherited aggregate used only non-refused RUN peaks, potentially omitting other-column consumption in refusing fixtures. D9 requires those columns too. The C07 checker includes all RUN peaks and available PRE columns, with exclusions only in each bracketed column. Four synthetic integrity tests pass, including refused-native/high-Lua and refused-instruction/high-node cases; these are checker tests, not product measurements.

Focused normal51MiB Debug ran12cases/10,195assertions:11passed, only the old absolute macOS pin failed. Measured off/on379,083/374,341 retains exactgap4,742; macOS pin updated to379,083 with unchanged window8. C03base374,888, drops3,770/3,768/200,000/175,864 retain all four floors. N30plain379,412/projected272,180 retains its strict transport-gap assertions. Final rebuild passed. Actual Linux pin preflight is root-owned; source held unchanged for its copied context before full final tables.

Raised big-Lua peak is55,316,843, above51MiB53,477,376. The raised test intentionally fails its refusal assertion because those scratch ceilings are256MiB; this is scaling evidence only. Normal-limit unchanged big-Lua already passed under the C07 loader correction and is included again in final measurements.

The corrected all-refusal checker also requires remeasuring the node bisect's broad upper probes. Offline C06 raw data has a node-only refusal with40,738,588 native peak, excluded by the old passing-only aggregation. That is not ordinary51MiB margin evidence. Coordinator approved measuring a tighter node upper bracket while retaining adjacent pass/fail checks and doubled-node mutation sensitivity. The measured bracket correction below retains every non-excluded column of refusing runs in the final tables.

## Resume and toolchain coherence (2026-11-09)

Linux pin preflight completed before the usage interruption: configure/build exit0; two cases/737assertions, one passing gap control and one failure solely from the obsolete off pin. Actual Linux off379,109/on374,372 retains gap4,737. Linux pin is now379,109/window8; root retains raw evidence under final-evidence/linux-pin.

After resume, an isolated Release sizing compile failed before measurement with a libc++ ABI tag mismatch. Current Homebrew compiler/headers resolve LLVM23.1.5, while Release cache retained23.1.4 scanner/archive paths and old PCMs. This failed sizing attempt is excluded. Per toolchain-parity.md and coordinator authorization, Release generated artifacts were cleaned and configured with `cmake --fresh --preset release` before rebuilding. Toolchain identity and raw clean/configure/build logs are retained. No product or operator installation/data was touched. Final host measurements must use coherent current-toolchain artifacts, not inherited binaries.

## Tight node upper bracket

After coherent Release23.1.5 rebuild, normal51MiB instrumentation measures5480entries/37,498nodes with39,998,112native;5481entries refuses exactly nodes at37,501 with39,999,031native. Wider6250 and7500 upper probes already consume40,144,582 and40,382,082native, exceeding the40,108,032 ordinary-column allowance despite refusing only in nodes. The original broad37500-entry bisect passes its functional test but violates this independent margin requirement (raw node-broad logs).

The fixture upper bracket is now5481: the first refusing entry, a one-entry minimum cushion above the measured last passing5480. `last_passing` still requires its upper bound to refuse, so a doubled-node comparison is directly detected there; all adjacent pass/fail, exact error/limit, cleanup and retry assertions remain. No exclusion or product ceiling changed. Independent prober must run the doubled-node mutation against this final fixture. All refusing RUN peaks remain in the other columns of the final margin table.

## Final Release measurement

Coherent LLVM23.1.5 RelWithDebInfo instrumentation (`build_measure.py release normal finalr1`) and immutable per-case inventory run passed487runtime cases plus22loader cases,509total, with no skips or failures. The exact integer checker found31budget fixtures and zero problems. Raw XML, run logs, inventories and five-column per-case data are retained in `calibration/final-release`; `final-release-maxima.json` summarizes the nonbracketing maxima, including refusing executions.

| Column | Ceiling | Maximum ordinary peak | Free percent |
| --- | ---: | ---: | ---: |
| Native bytes | 53,477,376 | 39,999,031 | 25.203826% |
| Live Lua bytes | 53,477,376 | 16,527,445 | 69.094510% |
| Invocation instructions | 29,000,000 | 21,516,200 | 25.806207% |
| Registration instructions | 1,000,000 | 5,000 | 99.5% |
| Conversion nodes | 37,500 | 27,873 | 25.672% |

Native maximum is the adjacent node refusal; invocation maximum is the native-boundary derivation; node maximum is the composed sink. Exclusions remain per column and are enumerated per case in summary.json. Debug measurements below now reproduce these maxima; independent budget mutations and coordinator final gates remain separately tracked. Owned runtime fixture formatting and repository diff whitespace checks passed after the fixture freeze.

## Final Debug measurement and closure

Coherent LLVM 23.1.5 Debug instrumentation (`build_measure.py debug normal finald1`) passed the same 487 runtime and 22 loader cases, 87,114 assertions, zero skips/failures. The exact integer checker found 31 budget fixtures and zero problems. Every maximum in the preceding five-column table reproduces in Debug. The only differing per-case peak is N30 role-capture live Lua memory: 12,124,275 Debug versus 11,533,651 Release; both independently satisfy the margin. The unchanged deliberate large-Lua refusal passes in both final inventories.

Both full `src/` SHA256 manifests match each other and the source after measurement. Durable manifests, exact private compile/link commands and executable identities are retained as `final-{debug,release}-source-hashes.json`, `final-{debug,release}-commands.txt` and `final-{debug,release}-identity.json`. C08's saved-response ledger-reader correction is included in this final identity; the measured budget fixtures do not exercise that reader. The ordinary source citation now reproduces 31,121,964 bytes rather than C06's 31,116,830 because it is measured against the final corrected source, not carried forward from the previous product snapshot. The native maximum in the node fixture changed because the measured upper bracket now probes the adjacent refusal rather than unnecessarily larger node-refusing inputs; all other-column consumption remains included.

Final commands, each run once per configuration after its coherent instrumented build:

```sh
python3 tests/native/r15-r18-c07-evidence/calibration/probe_measure.py SCRATCH_BINARY OUTPUT/ordinary padded:0:0
python3 tests/native/r15-r18-c07-evidence/calibration/run_all.py SCRATCH_BINARY OUTPUT/runtime 3
python3 tests/native/r15-r18-c07-evidence/calibration/run_all.py LOADER_BINARY OUTPUT/loader 3
python3 tests/native/r15-r18-c07-evidence/calibration/aggregate.py CONFIG OUTPUT . OUTPUT/summary.json
```

`CONFIG` is `debug` or `release`; `OUTPUT` is the corresponding `calibration/final-CONFIG` directory. Scratch binaries are `<scratch-root>/planar-c07-finald1-scratchpad-debug-normal/planar_runtime_tests` and `<scratch-root>/planar-c07-finalr1-scratchpad-release-normal/planar_runtime_tests`; loader binaries are `build/rewrite-CONFIG/src/engine/planar_lua_loader_tests`. Inventories and results retain every exact test command. Runtime counts include three isolated sizing helpers; the unchanged product runtime inventory contributes 484 cases. Each configuration has 86,920 runtime assertions plus 194 loader assertions.

The final measured host result supports the reduced 51 MiB / 29M invocation / 1M registration / 37,500-node tuple. The coupled lower-candidate model remains explicitly a derivation, not a substitute for measurements: 50 MiB's required node boundary is measured above its native allowance, while the final 51 MiB tuple has full actual Debug/Release tables. Independent doubled-node and other budget mutations, the root-owned final Linux pin gate and complete host acceptance gates must be consulted before overall acceptance. This agent does not self-approve those gates.

Limitations for independent review: the I1/I2 extension-owner control retains only its documented instruction-column paired-control exclusion; it is not evidence of ordinary instruction headroom. The ordinary runtime guard reproduces the padded source model and measured slopes; it does not execute every proposed lower-limit candidate as a complete old/new runtime pair. Lower-limit minimality is therefore supported by the disclosed coupled derivation and retained focused measurements, while full five-column acceptance measurements are actual runs of the final tuple. Historical pair results and pre-C07 artifacts are not promoted to final-source mutation evidence; the independent prober owns that evidence.
