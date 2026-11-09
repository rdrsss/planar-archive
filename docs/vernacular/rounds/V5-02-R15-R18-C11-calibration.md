# V5-02 R15–R18 C11 actual calibration

Actual final-source macOS Debug and RelWithDebInfo measurements pass: **509 cases,
87,118 assertions, zero failures, expected failures, skips or missing selections
per configuration**. The exact integer checker accepts all **31 budget fixtures**
in both configurations. This closes the actual remeasurement obligation in D15;
it does not replace the integrated gates or independent final review.

The product tuple remains native/live Lua **53,477,376 bytes (51 MiB)**,
invocation **29,000,000 instructions**, registration **1,000,000 instructions** and
**37,500 nodes**. This work changes no product code, fixtures or ceilings. The
ordinary model substitution used by the parent also applies to this evidence
runner; it is disclosed here rather than represented as the originally requested
reviewer model.

## Scope and source identity

The measured source includes the final C11 changes to `ledger/execution.t.cpp`,
`runtime/runtime.t.cpp` and `runtime/external_registration.t.cpp`: sibling SECTION
discovery, the four ordinary q(0,20000) margin assertions, and the shared-tail
constant reduced from 27,500,000 to 27,400,000. The three test sources were frozen
before the input snapshot. Both private measurement builds compile the current
runtime implementation and both changed runtime test translation units; they do
not reuse stale main Release test objects.

`calibration/source-before.json`, `post-debug-source-hashes.json`,
`post-release-source-hashes.json`, `post-raised-source-hashes.json`,
`source-after-measurement.json` and `source-after-raised.json` describe identical
contents for all **272 files under src/**, including all 179 engine files. JSON
formatting/order can differ; dictionary contents are checked for equality. This
was an actual build and measurement after the last engine edit, not attribution
of the previous C07 table.

An APFS copy window captured both coherent build trees and the source tree under
`<temp-root>/planar-c11-measure-inputs-n4okru75`.
The two input artifact manifests each hash 2,476 compiled/module/archive/build
inputs. Normal and raised identity files hash the exact new objects, harness
archives and executed binaries. `dynamic-and-tool-identities.json` additionally
hashes the admission dylib in both snapshots and main trees (equal), the linked
LLVM libc++ dylib and compiler/archive tools. Both caches use Homebrew LLVM
**23.1.5**, arm64 macOS; no mixed compiler cache was used. Main build trees became
writable again once copies completed. All later compilation was private.

## Executed checks

The private script `calibration/build_measure.py` is the C07 builder adapted only
for a supplied immutable build path and unique C11 output names. Instrumentation
and helper source fragments remain the existing X06 C04 private measurement
implementation. No instrumentation hook was added to the product.

From the repository root, with `SNAP` denoting the snapshot above:

```sh
PLANAR_MEASURE_BUILD="$SNAP/rewrite-debug" python3 tests/native/r15-r18-c11-evidence/calibration/build_measure.py debug normal c11d1
PLANAR_MEASURE_BUILD="$SNAP/rewrite-release" python3 tests/native/r15-r18-c11-evidence/calibration/build_measure.py release normal c11r1
python3 tests/native/r15-r18-c11-evidence/calibration/run_post.py
PLANAR_MEASURE_BUILD="$SNAP/rewrite-debug" python3 tests/native/r15-r18-c11-evidence/calibration/build_measure.py debug raised c11scale1
python3 tests/native/r15-r18-c11-evidence/calibration/verify_raised.py
python3 tests/native/r15-r18-c11-evidence/calibration/finalize.py
```

All these commands exit zero. The exact compiler/link commands, per-case
invocations, return codes, XML, RUN counters and five-column summaries are under
`tests/native/r15-r18-c11-evidence/calibration/`. `run_post.py` runs the ordinary
probe, each complete runtime inventory and each complete loader inventory in
separate case processes, then invokes `aggregate.py` for each configuration.
There is no zero-selection success path.

| Configuration | Runtime cases / assertions | Loader cases / assertions | Total | Margin rows / problems |
| --- | ---: | ---: | ---: | ---: |
| Debug | 487 / 86,924 | 22 / 194 | 509 / 87,118 | 31 / 0 |
| RelWithDebInfo | 487 / 86,924 | 22 / 194 | 509 / 87,118 | 31 / 0 |

The ordinary runtime inventory is **484 cases**. The instrumented inventory adds
exactly `SCRATCH family2 probe`, `SCRATCH probe` and `SCRATCH role probe`; no
ordinary case is missing in either configuration. These three helpers are private
measurement cases and do not increase product coverage. Inventory set differences
are preserved in `inventory-differences.json`.

## Actual ordinary margins

The exact rule is `4 * peak <= 3 * ceiling`. Exclusions apply only to their
specified refusal/bracket column; other columns and native-boundary instruction
samples remain checked. The compact record contains every row and exclusion
reason. Maximum eligible peaks are the same in both configurations:

| Column | Peak | Ceiling | Free | Free percent |
| --- | ---: | ---: | ---: | ---: |
| Native bytes | 39,999,031 | 53,477,376 | 13,478,345 | 25.203826% |
| Live Lua bytes | 16,527,445 | 53,477,376 | 36,949,931 | 69.094510% |
| Invocation instructions | 21,516,200 | 29,000,000 | 7,483,800 | 25.806207% |
| Registration instructions | 5,000 | 1,000,000 | 995,000 | 99.500000% |
| Nodes | 27,873 | 37,500 | 9,627 | 25.672000% |

There are real per-case differences from C07. The plain-owner role control's
instruction peak is now 27,721,000 instead of 27,821,000 in both configurations,
matching the tail reduction; its instruction column retains its documented
paired-control exclusion. Debug direct-shape Lua peak is 2,759,555 instead of
3,286,616; Debug N30 role capture Lua peak is 11,533,651 instead of 12,124,275.
The only current Debug/Release per-row difference is direct-shape Lua
2,759,555 versus 3,286,616. All applicable columns still pass. The raw delta and
configuration-difference files preserve these observations. The C07 evidence has
not been overwritten.

## D27 raised scaling and changed-tail confirmation

The tail constant is a calibration input, so D27's repeat-R requirement applies.
A fresh Debug raised binary was compiled from the same final source, with
268,435,456 bytes, 400,000,000 invocation instructions, 100,000,000 registration
instructions and 4,000,000 nodes. These are scaling samples, excluded from normal
margin/acceptance rows.

**16 fresh probes** each select one helper case, use the exact raised tuple and
have zero resource refusals:

```text
padded:0:0, padded:0:20000, padded:0:40000
nodes:4780, nodes:5080, nodes:5280, nodes:5480, nodes:5481
sinkp:24:100000:985000, sinkp:25:100000:985000
firstp:23:100000:999000, firstp:24:100000:999000
firstp:24:200000:999000
stagesp:10:100000:900000:8170000
nativep:25:985000, route:900
```

The node helpers exercise their semantic path (`semantic=true`,
`prepared=false`); that metadata is intentional. Route 900 retains its syntax
failure (`lua_syntax`) with no resource refusal and is only a scaling sample.
The fresh ordinary p(0,0) result is native **31,121,964**, instructions **131,200**,
open-native **22,643,664**, open-instructions **5,000**, matching D29. Added padding
costs exactly **23 native bytes and 22 instructions per byte** at both 20,000 and
40,000. Adjacent sink and first-prefix probes reproduce **692 nodes per job**,
with 27,181 and 27,873 nodes.

Both role cases were also executed separately with the raised binary. The
plain-owner case passes. The shared-owner case reports exactly one failed
normal-limit assertion (`REQUIRE_FALSE(subject.ok)`, exit 42): a 400M ceiling lets
the normal 29M refusal subject finish. Its raw RUN values provide the required
current-tail evidence:

| Shared-owner workload | Actual instructions |
| --- | ---: |
| Control | 27,719,200 |
| One 900k prefix | 28,620,200 |
| Two 900k prefixes | 29,521,200 |

Thus one-prefix < 28,680,000 < 29,000,000 < two-prefix. That deliberate raised-case
assertion failure is recorded explicitly and is not hidden within the green
normal-limit counts. `post-raised-summary.json` and its verifier retain all probe
RUN values, role XML and hashes.

The prior C07 coupled model remains the derivation for alternative tuples; no
40–50MiB acceptance binaries are claimed here. Fresh current probes confirm each
node-native input used by those rows: 4,780→38,073,112; 5,080→38,898,112;
5,280→39,448,112; 5,480→39,998,112. The optimistic lower-tuple derivation therefore
still rejects 40–48MiB at the 4,780-entry floor, 49MiB at 5,080, and 50MiB at
5,280. At 50MiB the 39,448,112-byte demand exceeds the 39,321,600-byte allowance;
at 51MiB 39,998,112 fits within 40,108,032. The added ordinary q guard makes the
low-memory constraint stricter and does not weaken this conclusion. The selected
51MiB tuple also passes both actual normal tables. `coupled-model-confirmed.json`
labels historical derivation and current measured confirmation separately.

## Durable evidence and limits

`tests/native/r15-r18-c11-evidence/CALIBRATION-PEAKS.json` is **45,917 bytes**,
SHA256 `0eae45bb0dffaf3ce01a6bc56488ca9dd139c0697d1cf9c34fece08c2dca81e0`.
It was regenerated with the parametric C10 generator, `--base` set to C11,
`--debug-dir post-debug`, `--release-dir post-release`, and `--model` set to
`calibration/coupled-model-confirmed.json`. It includes 31 rows per configuration,
five integer peaks/ceilings/free values, all per-column exclusions, actual source
and artifact manifest hashes, and the explicitly sourced lower-candidate model.
`CALIBRATION-MANIFEST.json` binds this report, compact record, source identities,
commands, raw tree inventory and normal/raised artifact identities.

This evidence is host macOS calibration. Linux gates, the prescribed D28 actual
old-pair experiment, derived-fault inventory/SECTION coverage, complete product
gates and independent final review belong to their assigned owners. They are not
inferred from this measurement run. No integrated acceptance or self-approval is
claimed here.
