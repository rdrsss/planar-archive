# R14-D01 documentation candidate evidence

Shared checkout `<repo>`; coder writes are limited
to `docs/testing.md` and this directory. Others' changes were preserved. The sole
product-reference edit replaces the current hardcoded registry sentence with
CTest inventory and gate-log guidance. The adjacent 247 CLI invocations,
historical counts, links and every other byte remain unchanged. Native discovery
is wired in `cmake/rewrite.cmake:33`; Python acceptance registration is in
`CMakeLists.txt:57`.

`docs-testing.before.md` and `docs-testing.after.md` are exact snapshots;
`candidate.diff` compares them. `capture.py` is the one-time capture/edit command
executed as `python3 tests/native/r14d01-evidence/capture.py` from repository root
(exit 0); do not rerun it against the edited file. `verification.json` records
exact replacement and preservation checks. No builds, tests, Planar tools,
commits, installation or operator-state operations were run.

`candidate.json` maps the updated ten-file A1 candidate with repository-root
paths to SHA256; its identity hashes sorted compact JSON of the files map.
Nine other candidate files match the approved A1 hashes. `protected.json` maps
all 95 files in the prior A1 coder/reviewer evidence directories, including their
manifests, with repository-root paths; each was checked before and after the
edit. Their published manifests additionally verify 60 coder and 33 reviewer
entries. No prior evidence was changed.

`evidence.json` maps all other files in this directory using directory-relative
paths and excludes itself. This is implementation evidence, not approval;
independent review remains required.
