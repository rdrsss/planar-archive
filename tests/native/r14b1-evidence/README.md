# R14-B1 candidate evidence

Pure typed event/load-result codecs; candidate awaits independent review. No
commits, public history support, format/schema changes or installation occurred.

`candidate.json` identifies the exact six owned files; `candidate/` contains their
bytes and `candidate.diff` compares them to this round's baseline. `protected.json`
records 12,213 unchanged files including prior evidence, product format, schema,
SQLite, history, Lua, CLI, runtime and vendor bytes. Coordinator-owned R14 round
records and roadmap/implementation status, build/.git/.DS_Store and this round's
owned/evidence files are explicit exclusions. `audit.py` also rejects unexpected
unowned files and preserves the AGENTS.md symlink.

`commands.json` records actual argv, cwd and exits. Final checks:

- `build-final.log`: all + planar_tests build passed. A later documentation-only
  interface correction rebuilt successfully in `build-docs.log`.
- `focused-final.log` / `focused.xml`: 133 cases passed, zero skips; exact names
  include all 122 A2 baseline cases and 11 new codec cases. `codec-final.log` /
  `codec.xml` reran those 11 after the documentation-only correction: all passed.
- `cli.log` / `cli.xml`: one acceptance case, 247 real CLI processes, six unchanged
  goldens, zero skips.
- `vectors.log`, `record-vectors.log`: both independent vector checks passed.
- `lint-green.log`, `helper-lint.log`: pinned active formatting/Doxygen and
  supplementary collision-helper formatting/Doxygen passed.
- `no-work.log`: final all + planar_tests build ran zero Ninja steps.
- `audit.log`: unchanged protected bytes, exact init composition versus the
  existing format helper and independent JSON oracle, fresh byte-exact reversible
  generation of C2/B1 variants, isolated pure test linkage and candidate identity.

`history-codec-link.txt` excludes format, SQLite, history algorithms, capture,
runtime and Lua. Only the new codec, json_codec/value leaves and Catch2 link.
`r12c2-generation.json` and `r13b1-generation.json` record the audited unchanged
variants. They are not patched by this round.

Initial attempts remain visible: `build.log` failed on missing-field initialization
warnings and an incorrect test static_assert qualifier; both were corrected before
running tests. `focused.log` passed 124 cases because nine new test names initially
lacked the ledger prefix; the final names select all 133. Initial lint runs caught
missing Doxygen member/return documentation; `lint-green.log` is the corrected
zero-warning result. These are authoring failures, not claimed product regressions.

The fixed schema and bounded strings cannot form a valid v1 event/result near
one MiB. Tests establish exact-limit malformed refusal and over-limit resource
refusal without padding extensions. IDs permit positive signed-64 gaps; no-op
results keep the returned revision's stored parent. This pure unit does not check
SQL/cross-row agreement and does not enable nonempty accepted history. This is
macOS Apple Silicon debug evidence; release/Linux and the integrated R14 milestone
remain outstanding.
