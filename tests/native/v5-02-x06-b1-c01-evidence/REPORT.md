# X06-B1-C01 gate report

## Candidate identity

HEAD 6c30ecdbc63af1fae690774108eb7d37cd4ffbf8 on dev/planar-v2-vernacular, with the shared working tree uncommitted. The sha256 of `git diff` (tracked changes only, untracked files excluded) is e0b6aeca1b33a31d7a4874e69d61e36ac0622c9af7e4abee2999ba49dbba1be0. Raw logs are in `raw/`.

## Commands (run serially, unpiped, exit codes read directly)

1. `cmake --build build/rewrite-debug --target planar_tests`: exit 0 (ninja reported no work, targets were already current).
2. `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure -j6`: exit 0. "100% tests passed out of 928", zero failures, zero not-run, zero skipped, zero disabled. Real time 67.47 s.
3. `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`: exit 0. clang-format --Werror and Doxygen checked 133 active source files.
4. `cmake --build build/rewrite-debug`: exit 0 (ninja: no work to do).
5. `cmake --build build/rewrite-debug` again (no-change check): exit 0, "ninja: no work to do", zero steps. Pass.

Caveat: the build in step 4 was itself already a no-work build because step 1 had built everything, so the zero-work check followed a successful build but no build in this run performed compile steps. The earlier compile occurred before this run, so I did not independently observe a full rebuild of the candidate.

## Inventory

`inventory.txt` is the sorted `ctest -N` name list (928 names). The comparison baseline is the most recent committed full name list found: `tests/native/v5-02-x06-a3-evidence/inventory.json` (CMake file-API test list, 918 names, X06-A3). Removed names: 0. Added names: 10, all preserving the 918 prior.

- replacement.intent create refuses invalid arguments
- replacement.intent create returns exact immutable legacy data
- replacement.intent create_v2 carries the optional profile selector
- replacement.intent moved-from data refuses
- replacement.storage abort and destruction roll back
- replacement.storage begin_write refuses a missing ledger
- replacement.storage entry and precommit identity checks are separate and site specific
- replacement.storage failed operation poisons and rolls back the transaction
- replacement.storage request miss does not poison the transaction
- replacement.storage write transaction matches the record writer byte for byte

## Limitations

Debug configuration only; no RelWithDebInfo or Linux run. The diff hash excludes untracked new files (intent, storage, acceptance sources). The tree contains other pre-existing uncommitted changes, so the hash identifies the whole working tree diff, not only this round. No source was edited by this runner.
