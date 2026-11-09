# Final reduced-budget mutation probes (pre-C11 candidate)

The independent campaign is complete for the frozen C08/C07 budget candidate. It is **not final integrated approval**, and it does **not claim literal D28 named-red/isolation compliance**: two exact historical proof obligations need the C11 test-only disposition described below. No candidate source or candidate build artifact was modified by this prober.

## Results and isolation

- **26 distinct unit/source/fixture faults are assertion-killed in their complete registered-test scope.** B17 and B20 survive their initially named scopes; those raw results remain intact, and existing supplementary cases kill the same unchanged mutant artifacts.
- One additional distinct node-margin fault is rejected by an independent exact D9 cell checker. The same checker additionally rejects the old-pair native margin; that is an additional proof of B17, not a second distinct source fault.
- Five original plus five privately recompiled no-op cases pass; the rebuilt cases contain2177assertions. The26fault campaign has37passing original baseline cases and37mutant-case executions, including supplementary scope. Four privately instrumented functional cases pass. All XML is independently checked for nonzero selection and zero skips.
- No compiler/link failure, crash, timeout, zero-match result or stale tuple counts as a kill. There are no unexplained full-scope survivors.

The base is a coherent LLVM23.1.5 Debug APFS copy outside the checkout. The prober independently cloned its build/source tree, recorded compiler/archive/stdPCM hashes, validated all14required initial exact names in the484-case runtime inventory, and proved object/archive substitution with a no-op. Each mutant has its own external source/object/archive/executable directory; imported PCMs and baseline link inputs remain read-only. Native capture changes replace a copied capture archive member; runtime changes replace only the copied budget-harness member. Limit faults recompile the budget implementation and the two test objects with consistent variant macros. Product512MiB/100M/etc. limits are never edited.

The first no-op compile succeeded but link failed because resolving `clang++` to `clang-23` changed argv0 driver mode and omitted automatic C++ libraries. The failed attempt is preserved/excluded. The corrected tool pins the versioned parent while retaining `clang++`. No-op XML separately confirms zero skips after the future-run parser was hardened. Executed script versions and exact raw data remain retained.

## Unit faults

| ID | Fault | Actual result |
|---|---|---|
| B01 | request-record conversion charge removed | Assertion-killed |
| B02 | snapshot/directory vector reservation removed | Assertion-killed |
| B03 | exclusion-vector and copied-string charges removed | Assertion-killed |
| B04 | capture-outputs ignored | Assertion-killed |
| B05 | current tree enumeration omitted | Assertion-killed |
| B06 | base tree enumeration omitted | Assertion-killed |
| B07 | registry read native charge skipped | Assertion-killed |
| B08 | observation read native charge skipped | Assertion-killed |
| B09 | projection counted transport reservation omitted | Assertion-killed |
| B10 | prepare-role capture session loses factory sink | Assertion-killed |
| B11 | conversion node comparison permits twice final node ceiling | Assertion-killed |
| B12 | node fixture upper bracket becomes measured passing5480 | Assertion-killed |
| B13 | instruction ceiling strictly between E and4E/3, from final slope data | QF derived-margin assertion killed; two dependent controls also fail (D28 isolation not claimed) |
| B14 | native ceiling strictly between ordinary P and4P/3, from final data | Assertion-killed |
| B15 | registration ceiling strictly between ordinary R and4R/3 | Assertion-killed |
| B16 | registration ceiling lowered to300000 (margin passes, retained pin must catch) | Assertion-killed |
| B17 | exact D28 oldC03 pair41943040/18000000 | QF+node functional scopes survive; unchanged stages+sink cases kill at required prepared controls |
| B18 | native/live-Lua test variant raised to67108864 above unchanged bigLua peak | Assertion-killed |
| B19 | native charge boundary comparison disabled (counter still advances) | Assertion-killed |
| B20 | factory sink invocation instruction comparison disabled on all declaration paths | Sink-alone scope survives; existing first-failure case kills at upper-refuses prerequisite; preframe scope survives |
| B21 | role handler tail restored to18400000 below new composed requirement | Assertion-killed |
| B22 | across-stage fixture extra jobs reduced10to4 | Assertion-killed |
| B23 | sink fixture extra jobs reduced25to4 | Assertion-killed |
| B24 | first-error fixture extra jobs reduced24to4 | Assertion-killed |
| B25 | native family extra jobs reduced25to4 | Assertion-killed |
| B26 | route far bracket restored1.5x causing independent text quota masking at29M | Assertion-killed |

Exact failed expressions, all primary outcomes and all supplementary outcomes are in `unit-summary.json`; original individual records are never overwritten. Particularly important direct kills are the request-record off-pin window, outputs gap, current/base tree floors, registry/observation floors, projection gap, and the tightened node upper-refuses prerequisite under node-doubling.

## Private checker proofs

Private scratch counters are the approved D6 observation method, with no public field or product hook. The independent checker reads raw RUN lines, verifies that the functional case passed with one match/zero skips, excludes no column except one actually bracketed by a refusal bit, and recomputes the exact integer condition `4 * peak <= 3 * ceiling`.

| Case | Functional test | Measured ordinary cell | Checker exit |
|---|---|---|---:|
| Control node cell | sink passes |27873nodes at37500;25.672%free |0|
| Node ceiling fault | same sink passes |same27873nodes at32500;14.237%free |1|
| Control native cell | node bisect passes |39999031native at53477376;25.203826%free |0|
| Old C03 pair native cell | same node bisect passes |same39999031native at41943040;4.635%free |1|

There is no node-refusal bit in the judged node-cell runs and no native-refusal bit in the judged native-cell runs. Refusal in another dimension does not hide its ordinary column (D9). The checker exits are actual executed commands, recorded in `raw/checker-exits.json`; `check_cell.py` independently recomputes from raw counters rather than trusting the measurement harness's precomputed booleans.

Both final ordinary measurement tables have509passing cases,31budgetfixtures and0problems; their hashes are in DERIVED-FACTS.json. The derived margin faults are native35800000, instruction28600000 and registration5800. Each is above observed usage and below its required four-thirds margin. The registration300000fault is caught only by the retained equality pin, as intended. Mutations are Debug only; this does not claim Linux mutation or Release mutation coverage.

## Exact D28 clauses versus current observations

The governing [X06-D-C04 D28](V5-02-X06-D-C04.md) asks for the final tree with the old C03 native41943040/instruction18000000 pair and a failing **named quarter-free test**. On this newer source, the ordinary unpadded control nativeP31121964 remains below75%of41943040, and its derived old-pairE10481782 is below75%of18M. The named test therefore passes; the node functional case also passes. Existing fixed-prefix stages/sink tests reject that configuration, and the private native-cell checker proves its insufficient node-flow headroom. These observations are real proof of the old pair's unsuitability, but are **not relabelled as the exact named D28 red**.

D28 also describes an instruction fault betweenEand4E/3 for which other fixtures pass so only the derivation notices. Current guardE21514628 gives upper threshold28686170.67. The actual28600000fault fails precisely that QF assertion, but also rejects first-error`vb.prepared` and role single-prefix`middle.ok`. The latter requires28720200instructions, above every possible fault below28686170.67. Thus literal global isolation cannot hold for the current fixed tail. Four other selected dependent controls pass. No broader all-fixture isolation claim is made.

## Source-ready proposal for review (not applied)

The coordinator accepted this as input to a fresh C11 test-only plan review; no exception is assumed:

1. QF already measures ordinary`q=padded_probe(0,20000)`. Add the same four observable25%checks forq as forp. Its native31581964 gives126327856>125829120under the old pair, so the named QF red is restored by a legitimate existing ordinary sample under D9, without new padding or observer fields.
2. Change only the role fixture`c02_tail`27500000→27400000, and use a28680000instruction fault. The middle requires28620200after that change; first-errorvb requires28632300. Both are below28680000, which remains below the guard threshold28686170.67. The two-prefix role subject remains above the normal29M ceiling. This is a source-ready prediction, **not an executed post-change proof**.
3. If reviewed/applied, run affected probes again and follow D15's final-source Debug/Release measurements and required gates. The current exact-source results remain preserved; no future-candidate approval is implied.

## Evidence and remaining boundary

Evidence directory: `tests/native/r15-r18-final-evidence/budget-probes/`. SUMMARY.json, unit-summary.json, PLAN.json, DERIVED-FACTS.json, snapshot/toolchain/inventory identities and compact manifest record the source tuple, commands, measurements, assertions and hashes. External bulky artifacts remain local and are hashed by the manifest. At report time all eight critical source/configuration files match the copied candidate exactly. No installed/legacy Planar or operator database was used. All probe processes ended and lane1 was explicitly released. C11 proof repairs, full platform gates and final independent integrated approval remain coordinator work.
