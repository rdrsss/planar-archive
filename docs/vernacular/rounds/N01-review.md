# N01 independent review

Disposition: **approve**. No findings requiring changes.

Reviewed by fresh ordinary reviewer `/root/engine_rename_reviewer`, gpt-6.1-sol
with high reasoning, in the shared `dev/planar-v2-vernacular` checkout. Starting
Git revision: `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Product/source files
were read-only; reviewer writes are this report and
`tests/native/n01-review-evidence/`. Existing shared changes were preserved.
No Planar tooling, installed binary, commit, installation or publication was used.

## Exact reviewed candidate

`tests/native/n01-evidence/candidate-files.json` SHA256:
`161b5f6a02be6a1e26258dde2e412b084e9a66659a3ae236d9614c69630239ae`.

The manifest covers 46 current files and the removal of 22 `src/kernel/` paths.
The 25-file estimate in the initial N01 ownership prose is inaccurate; the
baseline contents and final manifest establish the actual 22 moved files.
This is a record correction for the coordinator, not a product defect.

## Review and preservation

The independent audit compared every owned baseline content entry in
`before-files.json` against its original hash in `before.json`, then compared
its new path/content against the candidate. Every candidate is byte-identical
to the baseline after the requested case-preserving kernel-to-engine naming
substitution. There are no additional code, protocol-version, hash-domain,
fixture or golden edits. All candidate hashes still match; old source paths
are absent. The evidence manifest hashes also match.

All 871 protected files outside the live coordinator-owned N01 record remain
unchanged, including prior round evidence, original v4 design, historical
repository map, fixtures and vectors. `AGENTS.md` remains a symlink to
`CLAUDE.md`. Local file links in changed current Markdown documents resolve.
Active source/current reference searches found no remaining kernel spelling;
historical records and inactive previous build outputs intentionally retain it.

Both debug and release `compile_commands.json` and `build.ninja` contain no
old native source path or module name. Generated product/test runtime source
modules declare `planar.engine.runtime_source` and
`planar.engine.runtime_test_source`; the separate generated test bridge declares
`planar.engine.runtime_test` and `planar::engine::runtime_test`. Existing CMake
target names and their test-only exclusions are preserved.

The trusted binding in `src/engine/runtime/runtime.cpp` is now `engine`; its
consumers in the embedded stdlib and test bridge match. The separate declaration
loader still exposes only the immutable `planar` constructor namespace; it does
not install the trusted engine table. The rename changes no isolation behavior.

## Verification examined and executed

Raw coder evidence was independently inspected and its hashes checked:

- Debug and release complete gates each contain 113 raw passing CTest records,
  zero failures and the explicit zero-skip gate result.
- Each raw acceptance record reports 150 real CLI processes and six exact native
  goldens, with independent canonical/hash/prose/section checks.
- Both gates record pinned formatting and Doxygen checks on 25 active source
  files and end with `ninja: no work to do.`

Reviewer verification commands, executed serially from the repository root:

```sh
python3 tests/native/n01-review-evidence/audit.py > tests/native/n01-review-evidence/audit.json
ctest --test-dir build/rewrite-debug -R '^replacement\.(cli|loader)' --output-on-failure --output-junit "$PWD/tests/native/n01-review-evidence/focused-tests.xml"
```

The audit exited zero, with no candidate, baseline, protected-artifact, evidence,
active-registry or current-document-link mismatch. The focused CTest rerun passed
**8/8 selected tests with zero skips**: one real CLI acceptance case and seven
restricted loader cases. The raw LastTest output independently records another 150 CLI
processes and six native goldens. Raw rerun output is `focused-tests.log`,
`focused-tests.xml` and `focused-last-test.log` beside the audit; CTest truncates
the successful CLI output in JUnit, so the complete LastTest log is retained. An initial identical focused run also
passed; its JUnit output was relocated into the owned evidence directory, then
the run above refreshed both final evidence files with an explicit output path.
No full gate rerun was necessary after exact mechanical equality and intact raw
full-gate evidence were established.

Coverage remains macOS Apple Silicon. Linux was not executed. Old inactive
`build/rewrite-*/src/kernel/` files are build leftovers, excluded from the active
registries; they do not constitute product source or a compatibility surface.
R10 operator packet evaluation remains pending and is outside this naming round.
