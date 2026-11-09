# C11 independent sensitivity experiments

Status: **C11 sensitivity obligations complete**: prescribed old-tuple red/restored green, full instruction-fault isolation, registered section-discovery sensitivity, affected fault reruns and scoped carry are verified. This record is independent verification, not integrated acceptance. Candidate sources and build directories are read-only to this prober. All variants use an external throwaway source/build tree; raw evidence is under `tests/native/r15-r18-c11-evidence/experiments/`.

## Prescribed D28(b) old-tuple experiment

Followed the exact approved D6 amendment: substitute only `PLANAR_BUDGET_NATIVE_MAX` to 41943040 and `PLANAR_BUDGET_INSTRUCTION_MAX` to 18000000 in the final C11 `src/engine/CMakeLists.txt`. No production ceiling, node limit, registration limit or test expectation changed. The initial snapshot contains3,083 files, including the otherwise ignored required `tests/native/r12b-evidence/expected-schema.sql`. Its preserved old-tuple source copy has no missing input and exactly one changed file, containing those two substitutions.

Executed, unpiped with separate logs/exit files, from that throwaway source root:

```sh
cmake --preset debug
cmake --build <scratch>/source/build/rewrite-debug --target planar_tests -j3
ctest --test-dir <scratch>/source/build/rewrite-debug -R 'X06 D C03 ordinary fixtures keep a quarter of every observable ceiling free' --no-tests=error --output-on-failure
```

Configure and the complete `planar_tests` build exited 0. Named CTest exited 8: one matched case, 40 assertions, 39 passed and exactly one failure at the ordinary-q native margin:

```text
CHECK( q.native_bytes * 4 <= budget_native_max * 3 )
126327856 <= 125829120
```

Thus q native 31,581,964 cannot retain 25% of 41,943,040. The old instruction derivation remains green; this is the explicitly approved change of failure expression, not a claim that the historical instruction expression became red. Restoring 51 MiB/29M, configuring and rebuilding the affected native targets yields named CTest exit 0.

The exact source identity command was:

```sh
find src -type f | LC_ALL=C sort | xargs shasum -a 256 | shasum -a 256
```

Its old-tuple result is `1b254b78062db1af5aefad3b417d6e8da2b063046060b1e36967780f7c40ff3e`. Homebrew LLVM 23.1.5 arm64 compiler and archiver hashes match the prior budget campaign. Source-copy audits, compiler identity, original and substituted manifests, full commands, source diffs and old/restored binary/archive/cache/compile-command hashes are retained separately. This proof uses an actual fresh configure/build/CTest, not object substitution.

## Independent instruction derivation isolation

Changed only the private CMake invocation ceiling to 28,680,000, preserving 51 MiB native/live Lua, 1M registration and 37,500 nodes. Configured and built `planar_runtime_tests` successfully. Ran the entire combined runtime/external-registration binary with explicit `*`, lexical order and XML output, using the same exact inventory for baseline and variant.

The ordinary inventory has **484 cases**, and explicit `*` and default listings are identical: no hidden cases are omitted. The private calibration inventory adds only `SCRATCH probe`, `SCRATCH family2 probe` and `SCRATCH role probe`; those are measurement helpers rather than additional product coverage.

Baseline passes all **484 cases / 86,924 assertions**, zero failures/skips. The derived configuration completes all 484 cases with **483 passing cases and exactly one failed assertion**, zero skips. Only quarter-free fails:

```text
CHECK( budget_instruction_max * 3 >= 4 * extrapolated )
86040000 >= 86058512
```

E is 21,514,628, so 28,680,000 lies strictly above E and below 4E/3. The one-prefix role preparation and first-error controls pass, along with every other ordinary fixture. This is full-inventory isolation, not an inference from six controls. The fault exits 42 normally with complete valid XML; crashes, compilation failures and incomplete inventories do not count.

## Registered section-coverage sensitivity

Restored only the first old outer return in a private copy of `execution.t.cpp`. The selected native R17 case still passes **1 case / 2,125 assertions**, zero skips. The unchanged C11 helper exits 1 and the registered `replacement.execution_sections_c11_coverage` CTest exits 8, identifying exactly 12 omitted required paths. The first facet and its three nested paths execute; later siblings and the common corruption tail do not.

Default baseline and restored final helper runs pass **1 case / 5,816 assertions / all 16 required paths**, zero failures/skips. The registered baseline and restored CTests also pass. Source diffs, each native XML, both direct and registered helper commands, exits and the return-mutant executable are retained. No product source changed.

## Affected faults and unchanged scope

A source/dependency scope audit identifies 19 prior fault rows whose selected case chunks and all 190 non-test product C++/Lua source files are byte-identical. B20's first-failure supplementary kill and pre-frame survivor are retained separately. B13 and B17 are independently closed above. Fresh original and recompiled no-op executions each pass five controls / 2,181 assertions, zero skips. All five remaining affected faults are assertion-killed:

| ID | Fault | Exact failed checks |
|---|---|---|
| B10 | Role capture session loses factory sink | Required two-prefix refusal: `!(subject.ok)` |
| B14 | Native/live Lua ceiling 35,800,000 | Both p and q native quarter-free inequalities |
| B15 | Registration ceiling 5,800 | Both p and q registration quarter-free inequalities, plus retained equality pin |
| B16 | Registration ceiling 300,000 | Retained equality pin only |
| B21 | Role tail reduced to 18,400,000 | Required two-prefix refusal: `!(subject.ok)` |

Each original baseline passes exactly its named case. Both role-case baselines have 37 assertions; each quarter-free baseline has 40. All five variants exit 42 normally with valid XML, a nonzero matched case and assertion failure, and zero skips. No compile/link error, crash or timeout is counted. Original selected-scope survivors from the prior campaign remain preserved; this report does not relabel them independent kills. Both private checker cases use unchanged sink/node fixture scopes; checker classification remains distinct from a functional assertion kill. Their exact source and prior-record hashes are retained in `carry-scope-audit.json`.

## Final identity and boundary

All 272 files under `src/` match the initial C11 snapshot in both the candidate checkout and the restored private source tree. The coverage helper and required schema fixture also match. Candidate source identity, using the same executed fallback command above, is `eaa6c2b4360e073e560a6018dcdc4b386be518def9e6a9e25baa643676325f04`. Compiler SHA256 is `89f9a1735eb8c596efb614c3af472303d2feddbed30495bca94e164a0bbab271`.

| Artifact | SHA256 |
|---|---|
| Prescribed old-tuple runtime binary | `85d7e27f0128c2b13c73eb1130df83cf4274495f296156535104465fcd7af1cd` |
| Default full-inventory runtime binary | `4f0fbf9acba95495783f7127840634af86d7322158b3ad66727cca815167bda6` |
| Derived-limit full-inventory runtime binary | `ebfd26f1516549846a952770c39fbe9bcbacf6c5490c52231e80ebbdf865989f` |
| Restored-return ledger binary | `3aee6787dd5ccd1f9f729d57c1b643eb2a6202db69fdf5d36de443ee9303a926` |
| Final restored ledger binary | `0a45f69f25fa450aad311d12d1855fdb788e10ab0325c24190f8da77c05f2099` |

`validated-results.json`, `rerun-validated.json`, source/tuple audits and `MANIFEST.json` retain the exact commands, XML totals, failed expressions, source/cache/compiler identities and artifact hashes. The final manifest indexes 137 retained evidence files and 96 external variant files; full throwaway source/build trees also remain local. The prior sparse source copy omits vendor inputs, so the attempted vendor comparison is explicitly marked `NOT_COMPARABLE` and excluded from proof. No dependency pin or vendor source changed in this round.

Sensitivity runs here are macOS arm64 Debug. All compile work ended and lane 2 was returned. The prober used only replacement binaries and disposable test data. Full platform gates, actual post-edit calibration and final independent acceptance belong to the coordinator; no integrated approval is claimed here. R19's unknown real project remains separate.
