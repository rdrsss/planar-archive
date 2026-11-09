# C13 actual final-source resource calibration

Actual post-C13 macOS Debug and RelWithDebInfo measurements pass: **509 cases /
87,118 assertions per configuration**, zero failures, expected failures, skips or
missing selections. Both exact-integer five-column checks accept **31 budget
fixtures**. Fresh raised Debug scaling and both role-tail controls also execute
and verify. This is scoped measurement evidence, not independent source or
integrated acceptance approval.

## Source and build scope

The coordinator froze the last engine edit before any of these build/check/run
commands. All **272 src files**, including **179 engine files**, match the
before, private-build, post-Debug, post-Release, post-raised and final dictionaries.
The sole source delta from C11 is `src/engine/process/runner.t.cpp`, SHA256
`70ea1208b8a70020523a94e1e94a1c6d52b2aee23cbe67263f6c96bdf804c65f`.
Runtime/loader implementations, budget assertions and trusted Lua are unchanged.
The current whole-source snapshot is `SOURCE-FROZEN.json`; the old C11 identity
is historical and has not been relabelled as current.

Both current host measurement targets (`planar_runtime_tests` and
`planar_lua_loader_tests`) build/check with exit 0 and `ninja: no work to do.`
before capture. The bounded immutable snapshots contain **1,502 local files and
101 explicitly collected PCMs per configuration**: Debug 1,619,387,587 bytes and
Release 1,413,254,793 bytes. The driver also records 2,044/2,043 current external
compiler-discovered files. Ninja's inputs tool omits PCM outputs, so the driver
adds every imported PCM named in the represented module maps. Source dictionaries
and all copied hashes agree before/after capture.

The private instrumentation is **freshly compiled and linked** for normal Debug,
normal Release and raised Debug from those same captured inputs. No C11 private
runtime binary is reused. The instrumentation and helper fragments retain the
existing C04/C11 implementation; the builder changes only its private C13 output
namespace. All three builds exit 0. The loader binary comes from the validated
current host target snapshot. Current compiler, archive tool and linked libc++
remain hash-identical to their C11 signatures; the captured compiler is Homebrew
LLVM **23.1.5**, arm64 macOS.

After actual execution, all local/external snapshot hashes still match. Fresh
compiler depfiles additionally reconcile **828 Debug-normal, 827 Release-normal
and 828 Debug-raised unique dependencies**, including instrumentation includes,
against the captured headers/modules/current sources. SDK and temporary path
aliases are canonicalized for lookup, while exact byte hashes remain required.
No historical complete-SDK closure claim is used to avoid fresh compilation.

## Executed measurements

From the repository root, with `SNAP` denoting the recorded bounded snapshot:

```sh
python3 tests/native/r15-r18-c13-evidence/calibration/snapshot_inputs.py debug "$SNAP/rewrite-debug"
python3 tests/native/r15-r18-c13-evidence/calibration/snapshot_inputs.py release "$SNAP/rewrite-release"
python3 tests/native/r15-r18-c13-evidence/calibration/run_measurements.py
python3 tests/native/r15-r18-c13-evidence/calibration/verify_finalize.py
```

The final commands exit 0. Exact compiler/link commands, per-case commands, return
codes, XML, RUN/PRE counters, source/input/tool manifests and actual stage results
are retained under `tests/native/r15-r18-c13-evidence/calibration/`. The final
verifier log is `verification-strict.log`. Two preliminary verifier-only attempts
failed on `/var` versus `/private/var` and SDK alias lookup; both failed logs and
explicit setup-exclusion records remain. They changed no measured case, source
or resource counter and receive no successful-verification credit.

| Configuration | Runtime cases / assertions | Loader cases / assertions | Total | Budget rows / problems |
| --- | ---: | ---: | ---: | ---: |
| Debug | 487 / 86,924 | 22 / 194 | 509 / 87,118 | 31 / 0 |
| RelWithDebInfo | 487 / 86,924 | 22 / 194 | 509 / 87,118 | 31 / 0 |

The ordinary runtime inventory is **484 cases**. The instrumented inventory adds
exactly three private helpers (`SCRATCH probe`, `SCRATCH role probe`,
`SCRATCH family2 probe`), which add no product-coverage claim. Each selected normal
case executes in its own process with exact XML name/count matching. Every
normal case and all five columns are measured after the last engine edit.

The **test budget tuple** is 51 MiB native/live Lua, 29,000,000 invocation
instructions, 1,000,000 registration instructions and 37,500 nodes. Product limits
are unchanged. Both configurations have the following eligible maxima:

| Column | Peak | Ceiling | Free | Free percent |
| --- | ---: | ---: | ---: | ---: |
| Native bytes | 39,999,031 | 53,477,376 | 13,478,345 | 25.203826% |
| Live Lua bytes | 16,527,445 | 53,477,376 | 36,949,931 | 69.094510% |
| Invocation instructions | 21,516,200 | 29,000,000 | 7,483,800 | 25.806207% |
| Registration instructions | 5,000 | 1,000,000 | 995,000 | 99.500000% |
| Nodes | 27,873 | 37,500 | 9,627 | 25.672000% |

Every eligible cell satisfies `4 * peak <= 3 * ceiling`. Refusal/bracket exclusions
remain specific to their named column; other-column and native-boundary
instruction costs remain checked. The cited ordinary source values reproduce.
The only per-row change from C11 is Debug direct-shape live Lua 3,286,616 rather
than 2,759,555; its other columns and every eligible maximum above are unchanged.
The actual delta file is retained, with no unsupported causal explanation.

## Fresh raised Debug and lower-model confirmation

The raised tuple is 268,435,456 bytes, 400,000,000 invocation instructions,
100,000,000 registration instructions and 4,000,000 nodes. **16 fresh private
probe invocations** each select exactly one helper case, return 0, retain their
XML and produce nonempty checked counters. They cover the existing padded,
node-boundary, sink, first-prefix, staged, native-prefix and route samples.
All RUN records use the raised tuple and have no resource refusal. The route
sample retains its expected `lua_syntax` outcome; it is scaling evidence.

Fresh samples reproduce native/instruction padding slopes **23/22 per byte**,
**692 nodes per job**, and ordinary p(0,0) **31,121,964 native bytes / 131,200
instructions**. Both role cases execute separately. The plain-owner case passes;
the shared-owner case returns exactly 42 with one `REQUIRE_FALSE` failure whose
XML original is `!(subject.ok)` and expansion is `!true`. That normal-limit
refusal is intentionally false under the raised ceiling. It is excluded from
normal green acceptance counts, rather than accepted as an unrelated failure.

The shared-tail instruction values are **27,719,200 / 28,620,200 / 29,521,200**:
one-prefix < 28,680,000 < 29,000,000 < two-prefix. Fresh node-native samples
reconfirm the historical lower-candidate model, including 50 MiB rejection and
51 MiB fit. Those alternative tuple rows remain derived predictions, not actual
alternative-limit acceptance binaries. The current model links the fresh C13
raised records explicitly; no D27 raised carry is needed.

## Durable records and limits

`CALIBRATION-PEAKS.json` contains the current 31-fixture rows, integer peaks,
ceilings/free values, exclusions, source/artifact bindings and historical-model
provenance: **45,821 bytes**, SHA256
`59fdc6eee329a22483b15fbe5a395d78020d8ce68b14bcad4461cc148db3052d`. `CALIBRATION-MANIFEST.json` binds this report, compact record,
current source/input/artifact manifests, scripts, commands and raw tree inventory.

The measurements are macOS host resource evidence. Full host/Linux gates,
descriptor-fault sensitivity, the C11 old-tuple/budget-probe carry dispositions
and final independent acceptance remain with their assigned owners. This report
claims no new SDK identity for old C11 experiments. The archived C11 scratch
copies retain their own original mappings/hashes and do not replace these fresh
C13 runs. R19's real project remains unselected. No operator database, installed
Planar binary, host installation, publication, push or merge was used.
