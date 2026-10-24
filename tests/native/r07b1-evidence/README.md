# R07-B1 candidate evidence

Implementation candidate only; independent reviewer approval remains pending.
Shared checkout `dev/planar-v2-vernacular`, starting revision
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Existing changes preserved.
No Planar tools, installed binaries, real database, commits or installation.
The coordinator owns planning/round records.

`candidate.sha256` freezes all seven owned product/test/build/documentation
files. `evidence.sha256` freezes this note and the raw logs (excluding the
manifests themselves). Verify from repository root with `shasum -a 256 -c`.

## Executed verification

Commands ran directly, serialized in `build/rewrite-debug`:

1. `cmake --preset debug` — success, `r07b1-configure.log`.
2. `cmake --build build/rewrite-debug --target planar_runtime_tests` — initial
   success, `r07b1-build-initial.log`; intermediate/final focused build success,
   `build.log`. The final candidate rebuild is `product-build.log` below.
3. Initial production invocation before schema exports were connected:
   `build/rewrite-debug/src/kernel/planar_runtime_tests '[r07b1]'` — failed as
   expected for absent exports, `before-exports.log`. That initial test harness
   also had a temporary-root path alias mistake affecting fixture cases; this
   log is **not** proof that every initial fixture failure was missing product
   behavior. The harness now canonicalizes its own temporary root before capture.
4. Meaningful diagnostic regression, before correction:
   `build/rewrite-debug/src/kernel/planar_runtime_tests 'schema enforces job intrinsic*'`
   — 1 matched test, 73 assertions, 1 failure: a schema-invalid job ID was
   incorrectly reported as `reference` instead of `schema`.
   `diagnostic-beforefix.log` and `diagnostic-beforefix-build.log` preserve the
   failing execution. The fix only compares returned IDs after a valid ID exists,
   and keeps declaration presence separate from schema success.
5. `cmake --build build/rewrite-debug --target all planar_runtime_tests` — final
   candidate success, `product-build.log`.
6. `build/rewrite-debug/src/kernel/planar_runtime_tests '[r07b1]'` — final candidate
   success, 12 test cases, 861 assertions, zero skips, `schema.log`.
7. `build/rewrite-debug/src/kernel/planar_runtime_tests '[runtime]'` — 18 bridge
   cases, 432 assertions, zero skips, `bridge.log`. Final CTest repeats below
   also execute every bridge case against the final linked candidate.
8. `ctest --test-dir build/rewrite-debug -R '^replacement\.(schema|runtime)' --repeat until-fail:2 --output-on-failure`
   — final success, 30 matched cases, each run twice (60 executions), zero
   failures/skips, `ctest.log`.
9. `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` — final
   success, formatting/Doxygen checked 22 active C++ sources, `lint.log`.
10. `cmake --build build/rewrite-debug --target all planar_runtime_tests` — final
    no-change success, `ninja: no work to do.`, `nochange.log`.

## Scope and limitations

Production embedded exports parse strict bounded JSON, validate exact declaration
and registry schemas, normalize defaults/sets/facets, route all workspace entries,
and resolve exact text bytes with declaration-relative logical provenance from
retained values. Tests use production `run_trusted`, native capture of a per-test
copy of both quota jobs, malformed JSON and schema mutations, registry descriptor
bounds/uniqueness, deterministic diagnostics, and different job text directories.
API/result details are in `stdlib/planar/README.md`.

B2 still owns full references, shared ID namespaces, ownership/portable aliases,
check policy, assignment/reviewer coverage, graph and W. R08 owns observations,
projection and successful CLI behavior. This evidence does not assert those
semantics or check/approval success. Existing file-only capture snapshots cannot
prove empty-directory topology; that remains the coordinator's B2/R08 obligation.
Host verification is macOS with the pinned LLVM/libc++; Linux and release-mode
execution were not run for this bounded round. The full integrated renderer gate
remains R09. No native runtime/capture implementation or fixture/contract changed.
