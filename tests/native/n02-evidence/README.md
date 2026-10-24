# N02 coder evidence

The product-only relocation moves all eight Lua implementation files and their
README from `stdlib/planar/` to `src/stdlib/planar/`. Each moved file is byte
identical. Only the eight CMake embed paths and scoped current documentation
change. No runtime APIs, C++ modules, Lua ordering or behavior change.

`before/` captures the seven assigned pre-edit build/reference/instruction files.
`scoped.diff` is their exact N02 diff. `product-before.json` hashes the nine moved
files at their prior paths; `preservation-check.json` compares destinations.
`protected-before.json` records 899 fixture, test-only Lua/harness, vector,
pre-existing native-evidence and historical round/design/map files. Their bytes
must remain unchanged. The coordinator-owned N02 record is excluded.
`pre-existing-status.txt` records the extensive prior shared changes.

`candidate-sha256.json` identifies the exact 16-file product/documentation
candidate. It excludes this evidence directory and the coordinator-owned round
record. `AGENTS.md` remains a symlink to `CLAUDE.md`; top-level `stdlib/` is absent.

Commands, serialized:

```sh
make test-all PRESET=debug
make test-all PRESET=release
```

Raw complete gates are `debug-gate.log` and `release-gate.log`; detailed CTest
output is in the corresponding `*-ctest.log`. Each gate must report 113 tests,
zero skips, formatting/Doxygen success and a no-work second build. Acceptance
output records 150 CLI processes and six native goldens. Local host is macOS
Apple Silicon; no Linux result is claimed. Release Doxygen hit its documented
signal-only SIGBUS flake on attempt 1 and succeeded on the approved wrapper
retry (attempt 2); raw output is retained. No source edit or additional
regression test is needed for this byte-identical move.

`build-input-audit.json` verifies configured graph dependencies at the moved
paths, absence of old embed dependencies, and generated Lua bytes against the
ordered source concatenation. `local-link-check.json` checks 45 relative links;
the moved README has no relative Markdown links requiring repair.

Coder evidence is submitted for independent review and is not approval.
