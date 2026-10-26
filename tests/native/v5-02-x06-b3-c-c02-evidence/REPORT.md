# V5-02-X06-B3-C-C02 gate report (comment-only repair of C01B3C-R1)

Gate runner, 2026-10-26, shared checkout on `dev/planar-v2-vernacular`. I edited no source file. Raw logs and exit
codes are under `raw/` (none piped; each exit code is in its own `.exit` file).

## Candidate identity

- HEAD: `2af5f742498181a0f390c97f71db708bbe4b1fb3` (raw/head.txt).
- `git diff HEAD -- src` sha256: `d7dcc96c6e2552f7e60d62979e859d0a3cb35f212bd8f62909473238a67a9f09` (raw/diff-src.sha,
  patch in raw/diff-src.patch). This matches the "after" value in the C02 round record.
- `git diff HEAD` (unrestricted) sha256: `74fdf145969065ebb39b1eb22c818811a583869b9315620a6ec96aea0899c762`.
- `runtime.t.cpp` sha256 now: `ec153c6953e8add5133e6df59841feeaea8546a11aa58e41e99c595a80674273` (raw/rt-now.sha).

## Commands

| # | Command | Exit | Result |
|---|---|---|---|
| 1 | Pre-edit snapshot and hashes | n/a | **Limitation.** The edit was already applied when this gate started, so I could not snapshot before editing. The C01 bytes were recovered from the prober's scratch copy; `raw/runtime.t.cpp.c01-snapshot` has sha256 `8fedecbc2874f3280dbac17c8c6609a8e0b4e668fb5b9ec7f4b621834416433e`, which equals the C01 `hash-rt-r2.txt` value and the "before" value in the C02 record (raw/snapshot.sha256). The per-file hashes of the other ten src files come from the C01 review list `tests/native/v5-02-x06-b3-c-c01-evidence/review/00-files.sha256`. |
| 2 | Comment-stripped comparison | 0 | `raw/strip_comments.py` (token-level: strings, char literals and raw strings preserved, `//` and `/* */` removed, whitespace collapsed) applied to the snapshot and to the current file; `cmp` of the outputs exits 0 (raw/cmp-stripped.exit), 2897 lines each. A plain `diff` of the two files shows only the three comment hunks (raw/runtime.t.cpp.c01-to-c02.diff). The other ten src files hash identically to the C01 review list (`diff` exit 0; raw/hash-other10-now.txt, raw/ref10.txt). A first attempt with `clang -E -fdirectives-only` exited 1 (Catch2 header not found) and is kept as raw/strip-*.exit and `.err`; it was not used. |
| 3 | `cmake --build build/rewrite-debug --target planar_tests` | 0 | "ninja: no work to do." The repair coder had already built the edited file (raw/build.log). |
| 4 | `ctest --test-dir build/rewrite-debug --no-tests=error` | 0 | "100% tests passed out of 1003", 0 failed. The log has no "Skipped" or "Not Run" result; the only grep hit for "skipped" is the test name `replacement.command load refuses forged family order skipped duplicate and malformed arguments`, which passed. Real time 355.96 s (raw/ctest.log). |
| 5 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | 0 | pinned `clang-format --dry-run --Werror` clean; "Formatting and Doxygen checked 136 active source files" (raw/lint.log). |
| 6 | `cmake --build build/rewrite-debug`, twice | 0, 0 | First: "ninja: no work to do." (exit 0, raw/all1.log). Second: "ninja: no work to do." (raw/all2.log). Zero steps. Caveat: the first full build also did no work because earlier builds were current, so it is a successful build only in the sense that it exited 0 with everything up to date. |
| 7 | Hashes | 0 | recorded above (raw/diff-src.sha, raw/diff-all.sha, raw/rt-now.sha). |
| - | `scripts/redact-local-paths.py check` without file arguments | 1 | Reports local paths in older committed-to-be evidence logs (e.g. the B3-C `repair-lint.log`); not meaningful (raw/redact-nofiles.log). |
| - | same, on the files from `git status` plus every file in this evidence directory | 1 | The only leaks are three raw logs of this gate (`raw/ctest-N.log`, `raw/ctest.log`, `raw/lint.log`), which contain absolute paths printed by the tools. `src/engine/runtime/runtime.t.cpp` and the C02 record are clean (exit 0, raw/redact-owned.log). The commit step's `redact` then `check` will clear the raw logs; I did not run `redact` because it rewrites files. |

Verdict: every command in the agreed list met its criterion. The redaction check on this gate's own raw logs is the
only failing item, and it is a pre-commit step, not one of the seven agreed commands.

## Test inventory

`inventory.txt` holds the 1003 sorted names from `ctest -N` (raw/ctest-N.log, "Total Tests: 1003"). I compared it with
`tests/native/v5-02-x06-b3-c-c01-evidence/inventory.txt` (1003 names), the most recent inventory I found. No evidence
inventory file is tracked in git, so that on-disk C01 list is the baseline. New names: none. Removed names: none.

## (a) Acceptance criteria and findings to test names

C01B3C-R1 (the finding this round closes) concerns the comments of two tests, both in `src/engine/runtime/runtime.t.cpp`.

| Criterion or finding | Evidence |
|---|---|
| C01B3C-R1 / AC1: comments state the measured behaviour and make no claim on the mtime or on a single skipped reverification widening the window | Comment hunks at lines 2705-2707 and 2760-2761 (rename sweep, test `X06 B3 C a file replaced at any allocation inside prepare_load is never published as verified`, line 2665) and 2772-2775 (test `X06 B3 C an in place same size edit at any allocation inside prepare_load is never published as verified`, line 2770). Diff: raw/runtime.t.cpp.c01-to-c02.diff. The new text says the mtime restore is not load-bearing and that a single skipped source reverification is masked, which agrees with C01 probes IP-P01 (survives) and IP-hook-nomtime (survives) and with IP-P03, IP-PB1, IP-PB2 (the in-place test kills all three). |
| AC2: only comments changed; other ten files keep hashes | Step 2 above: stripped files equal, ten hashes equal. |
| AC3: all 1003 names, 0 failures, 0 skips | Step 4 and the inventory comparison. The two edited tests run in the ctest run under their unchanged names. |
| AC4: formatting and lint pass; no-change build does no work | Steps 5 and 6. |

## (b) Red-first

No regression test was added or changed, and no assertion, constant, helper or name changed, so there is no red-first
evidence for this round and none is required: the change is comments only, and the proof of that is the token-level
comparison in step 2. The C02 record states that no mutation probe was required. The red/probe record for the in-place
test belongs to C01 (`tests/native/v5-02-x06-b3-c-c01-evidence/probes/PROBES.md`). I did not invent any command.

## (c) Deliberate behaviour differences

None. Product behaviour, test behaviour and diagnostics are unchanged.

## Limitations

- Step 1 was done after the edit (see the table); the C01 bytes come from the prober's scratch copy, which hashes to the recorded C01 value.
- The comparison strips comments with a purpose-written script, not a compiler, because the compiler preprocessing mode could not resolve the test headers.
- macOS Debug only; no Linux run. Carried items N1 (phase coverage not measured) and N4 (see the C02 record) are unchanged.
- The ctest run took 356 s with the full suite and passed, including the #531 timing-sensitive test.

## Files this gate wrote

Everything under `tests/native/v5-02-x06-b3-c-c02-evidence/` (`REPORT.md`, `inventory.txt`, `raw/*`). No source or record file was edited.
