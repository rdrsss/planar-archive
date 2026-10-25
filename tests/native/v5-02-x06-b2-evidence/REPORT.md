# V5-02-X06-B2 gate report

## Candidate identity

HEAD is bcfe1ad7ed94a85985f1189deba3b12cc851e7bd on dev/planar-v2-vernacular. The candidate is uncommitted: modified src/engine/CMakeLists.txt and src/engine/acceptance/load.cpp, plus untracked source_capture.cpp, source_capture.cppm, source_capture.t.cpp and docs/vernacular/rounds/V5-02-X06-B2.md.
sha256 of `git diff` (tracked changes only): 5ccff2eb0b6773dcb61abc4daeff650032c6f44500ed75856c07738fea3b7dbf.
sha256 of `git diff HEAD` plus the contents of untracked src/docs files (excluding evidence): 1839b1c01c31b21a797d54dc8bf418a7fb224c6e2cf3c2e147167acdc4f553c4. This second value is an ad hoc composite and not a standard git identity.

## Commands (Debug, build/rewrite-debug, run serially, unpiped)

1. `cmake --build build/rewrite-debug --target planar_tests`: exit 0.
2. `ctest --test-dir build/rewrite-debug --no-tests=error --output-on-failure`: exit 0. 100% tests passed out of 937, zero failed. 930 unit plus 7 acceptance labeled tests. Total test time 398.24 s. The only "skipped" text in the log is the name of test 504 ("...forged family order skipped duplicate..."), which passed; no tests were skipped or disabled.
3. `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`: exit 0. Formatting (clang-format --Werror) and Doxygen checked 136 active source files.
4. `cmake --build build/rewrite-debug`: exit 0.
5. `cmake --build build/rewrite-debug` (no-change check, after the successful build): exit 0, output "ninja: no work to do." (only the globbed-directories recheck step, zero build steps). Pass.

All criteria met; passed = true.

## Inventory

`ctest -N` reports 937 tests, saved as inventory.txt (sorted names). The comparison baseline is tests/native/v5-02-x06-b1-c01-evidence/inventory.txt (928 names), the most recent inventory list found. It is present in the working tree but not tracked by git (no committed inventory list exists for it), so it is not a committed baseline. Result: 9 new names, 0 removed names (937 = 928 + 9). All 928 prior names are preserved.

New names (all `replacement.source capture ...`, from source_capture.t.cpp, 9 cases):
admission precedes registration and reads no declaration content; factories require three original receipts and refuse moved and poisoned authority; refuses a declaration replaced between registration and first read; refuses a hard-linked trusted declaration before content or factory; refuses retained authority for another selection or admission; registry colocation binds standalone input and refuses trusted alias; retained flow survives moves and detects post-seal mutation; terminal failure after capture refuses later operations; vanished declaration reports admitted change beside embedded missing.
(The inventory.txt diff output lists exactly these; the grep above also showed 3 `replacement.acceptance prepared ...` names which are in both lists and not new. Full diff in raw/new.txt and raw/removed.txt.)

## Limitations

- Debug configuration only; no RelWithDebInfo, release, Linux or sanitizer run.
- Candidate is uncommitted working-tree state; the diff hashes identify it only if the tree is unchanged since.
- Baseline inventory is untracked, so "preserved names" is relative to that working-tree file.
- Lint covered formatting and Doxygen as scripted; clang-tidy is advisory and not claimed.

Raw logs are in tests/native/v5-02-x06-b2-evidence/raw/.
