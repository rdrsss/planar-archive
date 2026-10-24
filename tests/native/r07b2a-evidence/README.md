# R07-B2A coder evidence

Candidate implementation only; no self-approval. Shared checkout; prior rewrite
edits preserved. No Planar tools, queue, commit, installation or legacy binary.
Round records remain coordinator-owned.

## Candidate changes

- Add separate `captured_directory` / `capture_topology` and
  `capture_session::directories() const`. Getter returns retained shared immutable
  sorted logical root/path entries only after successful `finish()`, with no I/O.
  Tree roots, exclusions and `.git` directories are omitted. File snapshots and
  file identity/provenance recipes remain file-only.
- Record directory identity during discovery; compare it before verification
  descent and compare complete retained directory membership before sealing.
  Detect empty directory addition/removal/replacement.
- Bound both enumeration passes separately across all trees before copying any
  non-dot entry name. Count every staged name, even excluded and `.git` entries.
  Preserve 10,000 files, 64 MiB raw input and per-file limits.
- Exact metadata charge: `512 + 6*(relative_path_bytes+1) + 2*(basename_bytes+1)`.
  `enumeration_entries` maximum 100000 is checked first; `enumeration_bytes`
  maximum 67108864 follows. The larger conservative charge covers staging,
  spelling maps and retained/verified/sealed topology. Bytes include full relative
  prefix/separators, no trailing slash. No limit overrides or synthetic entries.
- Staging resource exhaustion is refused before complete membership comparison.
  Already observed directory identity/membership changes and retained-file quota
  growth keep `capture_changed`; excluded membership mutations may exhaust the
  enumeration budget without changing retained topology.
- Narrow CMake expansion: optional `TEST_TIMEOUT` helper argument, default30;
  capture registration selects120 seconds. The real count-boundary fixture took
  27.79 seconds, leaving inadequate margin under the previous30-second default.
  No other test registration changes.

## Commands and results

All build operations serialized in `build/rewrite-debug`; explicit from-source
replacement test executables only. Logs are raw command output.

1. Add empty-directory regression test before implementation.
   `cmake --build build/rewrite-debug --target planar_capture_tests`
   succeeded (`red-build.txt`).
   `build/rewrite-debug/src/kernel/planar_capture_tests 'capture detects empty directory membership and identity changes'`
   exited42: one matched test, all3 add/remove/identity mutation assertions failed
   (`red-empty-directory.txt`). Those assertions now pass unchanged.
2. `cmake --build build/rewrite-debug --target planar_capture_tests` succeeded
   (`build.txt`). Initial `ctest --test-dir build/rewrite-debug -R 'replacement.capture' --output-on-failure`
   succeeded:15 matched cases, zero skips (`capture-tests.txt`). Subsequent source
   adds one file-provenance comparison and ignored-subtree mutations.
3. Pinned LLVM `clang-format -i` applied only to the3 owned capture sources.
4. Final `cmake --build build/rewrite-debug --target planar_capture_tests planar_hash_tests planar_lua_loader_tests`
   succeeded (`final-build.txt`), including timeout reconfiguration.
5. `ctest --test-dir build/rewrite-debug -R 'replacement.(capture|hash|loader)' --output-on-failure`
   succeeded:26 matched cases (16 capture,7 loader,3 hash), zero skips,36.62 seconds
   (`final-tests.txt`). Includes existing confinement/aliases/raw-byte quotas,
   retained-growth classification and native captured identity recipes.
6. `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`
   succeeded: pinned formatting and Doxygen over22 active C++ files (`lint.txt`).
7. Repeat exact final build command: `ninja: no work to do.`
   (`no-change-build.txt`).

## Finite resource boundary fixtures

Entry fixture creates50000 short-named empty directories in each of two roots.
Exactly100000 entries succeeds in discovery and verification; one extra refuses
in discovery, and one excluded extra added by the verification seam refuses in
verification. This exercises accumulated cross-tree budgets and staging before
exclusions without hitting the regular-file quota.

Metadata fixture creates two128-byte parent components, charging1544+2318=3862.
Each128-byte child basename has386-byte root-relative path and charge3092.
21702 children charge67106446 total bytes and succeed in both passes;
21703 children charge67109538 and refuse. Verification's added child is excluded,
so that test proves filtered names still count. All names meet portable path limits.

## Limits

Executed on macOS with the configured pinned LLVM debug toolchain. Linux and
release builds were not run in this bounded round. Capture remains cooperative:
change-and-revert and atomic OS snapshots are outside the existing contract.
No broad product/render integration gate was run; this is the generic capture
prerequisite, with Lua policy and rendering unchanged. Independent review pending.

`candidate-sha256.txt` freezes every changed source, CMake and reference document.
`evidence-sha256.txt` freezes this report and raw logs (excluding hash manifests).
