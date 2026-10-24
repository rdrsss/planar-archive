# R12-A independent review evidence

Frozen candidate manifest SHA256: `510e0f5453f66a6973462989aa51e02a1923171e1fa27a536674e15ebc01006d`.
Frozen implementation evidence manifest SHA256: `3987f7f6706ef98ed737cff96eece4079942bfc1698e95af0749521a636068b0`.
Both verified before and after review. Product/reference/vendor files were read-only throughout review.

- `focused.log`: independent `ctest --test-dir build/rewrite-debug -R 'replacement.ledger' --output-on-failure`, 9 passed, zero skipped.
- `no-work.log`: independent `cmake --build build/rewrite-debug --target all planar_tests`, no work.
- `probe.cpp`, `probe-command.sh`, `probe-build.log`, `probe.log`: additional scratch-only C++ probe linked against exact candidate module/static libraries. Build/run both exit 0. The script reproduces macOS flags from the existing build; no installed Planar is invoked.
- `vendor-audit.json`: independently compared all seven vendor files against Git HEAD; every file matches.
- `protected-audit.json`: independently hashed 3189 protected files; no changes.
- `candidate-after.log`, `evidence-after.log`: all manifest entries still match.

The probe checks that an active INSERT RETURNING causes commit to fail with typed SQLITE_BUSY, rollback removes the inserts, and destroying an obsolete guard cannot roll back a newer transaction. Those checks pass. It also records the issue: bare `:memory:` succeeds in existing-file modes with no persistent file, and read-write mode can create a table. Explicit `./:memory:` in the scratch root creates a real file, confirming the distinction. All real databases and temporary files use canonical unique scratch roots removed on exit.

Only focused debug/macOS verification ran. Full debug/release, Linux, VFS fault injection, backup, disk-full and termination/restart gates remain later R12 scope.
