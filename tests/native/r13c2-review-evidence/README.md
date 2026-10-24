# Independent C2 and integrated R13 review evidence

Fresh reviewer `/root/r13c2_reviewer` used shared
`dev/planar-v2-vernacular`, HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`.
Only this directory and `docs/vernacular/rounds/R13-C2-review.md` were written.
No product fixes, unrelated reverts, Planar tools/skills/queue, operator data,
installed binary, subagents, commits, installation, publication or merge.

`run.py` saves exact argv, exit status, elapsed time and complete combined output.
Build directories were exclusively owned; build/test operations ran serially.

| Check | Raw evidence | Result |
| --- | --- | --- |
| `make test-all` | `gate-debug.{json,log}`, `debug-LastTest.log`, `debug-registry.json` | 226/226 passed, zero skips, 50-source formatting/Doxygen, no-change build does no work. |
| `make test-all PRESET=release` | `gate-release.{json,log}`, `release-LastTest.log`, `release-registry.json` | 226/226 passed, zero skips, 50-source formatting/Doxygen, no-change build does no work. |
| Saved-history adversarial harness | `adversarial.cpp`, `build-probe.py`, `probe-commands.json`, `probe-{compile,link,run}.log` | Three cases, 13,703 assertions, all pass; never invokes live Lua/capture. |
| Separate focused CTest `replacement.interpretation` | `interpretation.{json,log,xml}` | 10/10 passed, zero skips. |
| Both vector generators, `--check` | `vectors.{json,log}`, `record-vectors.{json,log}` | 12 hand-byte, 20 upstream hash, 41 canonical/domain and ten independent record vectors. |
| Independent identity/generation audit | `audit.py`, `audit-before.{json,log}`, `audit-final.{json,log}` | Candidate13/evidence61/protected11863 and integrated latest36 paths exact; both variants and all nine embedded original/chunk byte arrays exact in both presets. |
| Independent counts/log/JUnit verification | `verify.py`, `verify.{json,log}` | Registry226, all226 passing statuses, focused109 included, interpretation10, CLI1; zero skips, lint50, no-work confirmed. |
| Supplementary pinned source formatting | `probe-format.{json,log}` | Independent harness passes `clang-format --dry-run --Werror`. |

The decoder probe checks all 13,114 proper interpretation truncations and all
449 proper versions truncations, trailing bytes, nested recorded container/
field corruption, duplicate and reversed paired jobs, ID and unsafe file-path
mismatches, differing historical policy values and stdlib digest, exact separate
versions equality, and distinct future-version refusal. It links the current
candidate modules without source changes. Exact compile/link/run argv is retained.

`audit.py` is independent of coder scripts. It reconstructs the C2 diff and
candidate identity, verifies every live candidate and snapshot, all 61 coder
evidence entries and 11,863 protected inputs, then composes latest reviewed
A/B1/C01/B2/C1/C2 candidates (36 paths). Protection excludes OS `.DS_Store`
metadata and only active coordinator R13/C/C2 records. No protected difference
was found; `protected-differences.json` is empty. Fresh temporary R12-C2/R13-B1
generation equals configured sources/manifests byte-for-byte in both presets;
reversibility and both digest redirections remain intact. Generated arrays are
independently decoded to verify exact nine original bytes, paths and ordered LF
assembly in both presets. Generation snapshots are retained here.

Coder intermediate unused-helper compilation, absent-golden, raw-versus-normalized
registry set-order assertion and Doxygen diagnostic-free SIGBUS/retry evidence
were inspected and remain unchanged. Their final goldens and complete existing
normalization/validation comparisons pass. Own read-only log inspection initially
failed on strict UTF-8 decoding of synthetic binary diagnostic bytes; replacement
decoding corrected the inspection (`inspection-note.txt`). Own `verify.py`
initially expected older CTest summary spelling; it failed after already counting
all 226 passing statuses. `initial-verify.{py,json,log}` preserves that failure;
the corrected checker accepts the observed `100% tests passed out of 226` spelling
and retains every count/status/skip/lint/no-work requirement. No product or test
assertion changed for either inspection/checker issue.

Executed platform: macOS arm64, pinned LLVM/libc++ toolchain. Linux, physical
device exhaustion, hostile concurrent filesystem writers and R14 acceptance/
history activation/export are not established by this review. Full gates exercise
the existing configured VFS and capacity failures, concurrency and process death;
no stronger hardware durability or exhaustive allocation-fault claim is made.
Both build directories are released on delivery. `evidence.json` maps every
file in this directory relative to this directory, excluding itself and explicit
OS `.DS_Store` metadata only. The review report is separate.
