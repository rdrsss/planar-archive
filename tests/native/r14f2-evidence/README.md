# R14-F2 implementation evidence

Shared checkout, HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`.
The intentional pre-existing rewrite is captured in `preexisting-status.txt`;
`baseline.json` freezes the pre-edit inventory. No legacy tools, installed binary,
operator data, commit, install, publish or merge was used. Coder does not approve.

Owned production changes: export handler, bootstrap registration, optional buffered
stdout-error bytes and main selection, additive CMake wiring and product references.
Tests add CLI acceptance, command units and a reversible command-only fault variant.
The one old acceptance expectation change is exactly the generic usage message;
`audit.py` reverses it and proves the baseline hash. Native F1 and old helpers,
fixtures and evidence remain unchanged.

Executed commands (source-built debug binary only):

- `cmake --preset debug`
- `cmake --build build/rewrite-debug --target all planar_tests`
- `ctest --test-dir build/rewrite-debug --output-on-failure -R 'export|help is explicit|ledger error adapter|replacement\.(cli|init_cli|history_cli)$' --output-junit tests/native/r14f2-evidence/focused.xml`
- `ctest --test-dir build/rewrite-debug --output-on-failure -R 'export postpublication|export binds|export adapters' --output-junit tests/native/r14f2-evidence/new-units.xml`
- `python3 tests/acceptance/bootstrap_export.py build/rewrite-debug/bin/planar build/rewrite-debug/src/engine/planar_history_fixture`
- `python3 tests/vectors/generate.py --check`
- `python3 tests/vectors/records/generate.py --check`
- `python3 scripts/rewrite-check.py gate --build-dir build/rewrite-debug`
- `python3 tests/native/r14f2-evidence/audit.py`
- `/opt/homebrew/opt/llvm/bin/clang-format --dry-run --Werror tests/native/r14f2-support/boundary.t.cpp`

An early focused CTest run overlapped correction/configuration and building of only
the new fault variant. Those exploratory results are supplementary; the final
full gate runs serially against the completed candidate and is authoritative.

Initial logs preserve implementation mistakes and corrected harness mistakes:
C++ `export` cannot be a module-name segment (resolved as `handlers.source_export`),
Python executable lookup needed command-directory scope, and read-only oracle
column names were corrected to `source_id`. These were not product regressions.
`build-variant-second.log`, `cli-third.log` and subsequent focused/gate evidence
supersede those initial failed runs.

The post-publication variant transforms only module/namespace and two exact handler
sites: throw bad_alloc after native success for the allocation target, or encode
the success envelope with a one-byte ceiling for the encoding target. Native F1
publishes the real tree first; both controlled failures must return the prebuilt
uncertainty response with empty stdout, unchanged ledger and preserved complete
target. `r14f2-generation.json` records source/generated hashes, exact substitutions
and byte-identical reversal. No public injection flag exists and production source
is never mutated by this helper.

Actual process output tests close the read end before launching the child, proving
there is no reader race. Successful export publishes then returns the exact
operation-stdout guidance; unknown revision retains its semantic refusal without
uncertainty. Init/history output behavior and existing render output checks remain
pinned. The actual CLI fixture is unchanged and contains a single nonexecuting
source file; binary/empty/executable/multidirectory coverage remains native F1.

No Linux runtime, release-preset, actual allocator exhaustion, hardware power-loss,
or fake CLI native-publication-fault claim. Release preset is deferred to later
integrated verification per coordinator. Cleanup and uncertain-publication typed
adapters are direct unit evidence; native F1 owns actual fault behavior.

Final verification: the serialized full debug gate passed all 331 registered
cases with zero skips, formatting/Doxygen checked 86 active sources, and the
no-change all+planar_tests build ran zero steps. Focused exploratory selection
passed 20 cases; all three new command/fault unit cases separately passed. All
327 prior registered names remain present. The independent vectors verified
12 byte vectors, 20 upstream hashes, 41 canonical/domain vectors and 10 record
vectors. All 5046 protected baseline files are unchanged; five configured variants
match fresh reversible generation exactly. The new support test passes pinned
formatter checks.

The full-gate candidate is retained in `gate-candidate-files.json` and
`gate-candidate.sha256`. After that gate, only `tests/acceptance/bootstrap_export.py`
changed: a Unix-domain socket target refusal was added, and success ledger
identity is now compared with the read-only metadata oracle. `cli-final.log/xml`
reruns exactly the affected acceptance case (55 actual processes), zero skips;
`cli-final-process.log` preserves a direct rerun's process output. The audit's
CTest inventory listing reset `cli-final-LastTest.log` to an empty test session. Production and other test
sources match the full gate. `audit-final.log` freezes the final candidate, and
`no-work-final.log` again proves zero build steps. No broad gate repeat was needed.

`candidate-files.json` and `candidate.sha256` identify the final review candidate.
`evidence-sha256.json` and `evidence.sha256` identify these frozen raw records;
the evidence manifest excludes itself and its own hash. Debug build ownership is
released for independent review. No self-approval is claimed.
