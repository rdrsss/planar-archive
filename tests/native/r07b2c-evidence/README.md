# R07-B2C candidate evidence

Implementation evidence, pending independent review. Work occurred in the shared
checkout; no Planar tools, installed binaries, queue, commits, installation or
native/schema/semantic contract changes. The coordinator owns round records.
Five owned production/test files and the explicitly authorized five symbolic
manifest corrections are frozen in candidate.json, with exact SHA256 bytes and
a combined path/hash identity. Evidence hashes are separate from that candidate.

The new production export is `select_role`; its exact additive input/result API
is documented in stdlib/planar/README.md. It first validates the full raw bundle
and every job, validates retained raw byte/mode inventories against topology
and declaration captures, then builds exact scoped W and native identity records.
No I/O, Markdown, successful render CLI, observation applicability, check
execution, completion or approval is implemented here.

## Symbolic fixture discrepancy and correction

The first exact recursive comparison found integration.check.inputs spelled
`["own_output","output_vector"]` in five R03 W templates. Byte sorting puts
`output_vector` before `own_output` (`output` compares before `own`).
Declarations define registry check.inputs as a set; render-contract.md requires
normalized set ordering. Approved B1 schema.lua already normalizes this field
with `set(enum(...),1)`. Selection therefore correctly produced the sorted form.
`initial-tests.log` preserves the raw failing field comparisons and work hashes.
There were no other exact W fixture differences. This was a fixture expectation
error, not a runtime contract change.

The coordinator explicitly authorized changing only this array's order in:

- fixtures/render/quota/manifests/combined-builder.symbolic.json
- fixtures/render/quota/manifests/combined-reviewer.symbolic.json
- fixtures/render/quota/manifests/split-cli_worker.symbolic.json
- fixtures/render/quota/manifests/split-cli_worker-recovery.symbolic.json
- fixtures/render/quota/manifests/split-reviewer.symbolic.json

`fixture-correction.json` records exact before/after hashes by reconstructing
the original single-array ordering. No declaration, Markdown, other fixture
field or hash recipe changed. The library-only W template was already correct.
The final oracle performs **no expectation normalization**: it resolves symbols
with independently assembled native H records, then recursively compares exact
kinds, field sets, array lengths/order and primitive bytes. All six W templates,
base/observation trees, unit outputs, reviewer targets and full file provenance
(including exact bytes_hex) compare directly. Worker role outputs and available
facet identities have independent native-record oracles too.

## Verification

All commands were direct and serialized in build/rewrite-debug:

```
cmake --build build/rewrite-debug --target all planar_runtime_tests
build/rewrite-debug/src/kernel/planar_runtime_tests '[selection]'
ctest --test-dir build/rewrite-debug -R 'replacement.(runtime|schema|semantic|selection)' --output-on-failure --output-junit tests/native/r07b2c-evidence/ctest.xml
python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug
cmake --build build/rewrite-debug --target all planar_runtime_tests
git diff --check
```

CTest writes relative JUnit paths under its build directory; the exact report
was copied into this evidence directory after completion. `counts.json` checks
nonzero cases and zero skipped/disabled results. The focused selection run
passed 9 test cases with 17,265 assertions. CTest passed 60 distinct cases:
18 runtime/bridge, 17 schema, 16 semantic and 9 selection; zero skips/failures.
The pinned formatter and Doxygen checked 22 active C++ sources. The final build
reported `ninja: no work to do.` Diff whitespace validation passed.

Coverage includes exact scoped requirements and all W fields, native identities,
optional notes/preferences/budgets/checkpoint/provenance syntax and set ordering,
per-unit worker-added checks and selected/unselected registry definitions,
complete-policy versus observed values, external versus grouped/reviewer start
facets, capture absence versus captured emptiness, unavailable facets,
base-byte materiality, own/upstream output changes, executable modes, derived
pipeline obligations/stage reversal/producer assignment changes, retained input
after workspace deletion, directory-topology immateriality, malformed inventory
records, nonmatching roots/paths/bytes, per-file and total byte limits, file-count
limits, and optional observation-file absence. Every mutation uses independent
native immutable copies or a per-test temporary root. R08 still owns assertion
schema/applicability and actual packet projection; tests make no observation
quality or execution claims. The complete R09 integrated gate is deferred.
