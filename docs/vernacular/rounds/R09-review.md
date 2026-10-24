# R09 independent review

Disposition: **approve**. No blocking correctness, preservation, dependency or
verification findings. The two editorial observations below are nonblocking and
do not require changing this frozen candidate to close R09.

## Reviewed identity and authority

Reviewer `/root/r09_reviewer`, fresh independent ordinary reviewer; shared checkout
`dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`.
Read the current agent guide, implementation plan, roadmap, v4 design, R09 brief,
coder report, active references/build graph and raw verification evidence.
No product changes, Planar tools, installed binaries, delegation, commits,
installation, publication or merge. Writes are limited to this report and
`tests/native/r09-review-evidence/` plus an isolated temporary snapshot.

Candidate manifest `tests/native/r09-evidence/candidate.sha256` has SHA256
`bb915f01a4231a28acbaa44489f02a3d72d558ba13873f98c13a2d4f5f958d67`.
All 1,296 files matched both before and after review. The evidence manifest has
SHA256 `fc1f6b23e8490e69f2698763e230983a6d01da3f84034109f09f9c7d3d19d9b3`;
all 33 recorded artifacts matched. Coordinator-owned R09/roadmap records and
reviewer artifacts are excluded as documented. No additional retained product
files outside the candidate manifest were found.

## Acceptance assessment

- Compared the 9,072-path initial inventory, 7,775 recorded deletions and 25
  supplemental ignored legacy vendor deletions. All 7,800 removed paths remain
  absent. Deletions cover the obsolete engine, library, command fleet, migrations,
  operational agents/skills/workflows, installation/release/CI surfaces and eight
  unused vendor families; no permanent legacy source tree remains.
- Independently compared all 1,117 protected initial product, test, fixture and
  retained dependency files with their original SHA256 values: unchanged.
  `AGENTS.md` remains a symlink to `CLAUDE.md`. Existing local ignored outputs
  and operator installations/data were outside the deletion manifest.
- Reviewed the downward native module graph and three retained dependencies.
  `cmake/dependencies.cmake:7` and `:29` use committed sources; there is no
  configure-time dependency fetching or old CMake helper reference. The only
  product executable is `planar` (`src/cmd/planar/CMakeLists.txt:6`). The source
  graph has no implicit home/legacy database access or queue path.
- README, CLI, architecture, declaration, testing, distribution and contributor
  references describe the actual explicit-input render-only product. Capture
  limitations, unverified driver assertions, absent execution/state and deferred
  distribution are stated. Historical vernacular source links remain historical
  evidence rather than active workflow instructions. All 46 checked relative
  links in the product/reference document set resolve.
- Raw debug and release gate logs each contain 113 passing CTest entries:
  112 native cases plus one acceptance case; zero failures/skips. The acceptance
  output records 150 real CLI processes and six computed goldens. Both gates
  report formatting/Doxygen over 25 active sources and a final no-work build.
  Registry names/counts match execution. Product/schema/adversarial tests were
  preserved, rather than replaced with cleanup-only checks.

## Independent verification

Created a source snapshot using only the 1,296 frozen retained paths, preserving
`AGENTS.md` as a symlink. No existing build cache, generated output, root binary
or ignored local file was copied. Within this clean source snapshot, serialized:

| Command | Result |
| --- | --- |
| `cmake --preset debug` | Exit 0; fresh configure; only expected experimental import-std warning. |
| `cmake --build build/rewrite-debug --target all` | Exit 0; 92 steps, renderer produced; test executables excluded. |
| `python3 tests/acceptance/bootstrap_cli.py <fresh-planar>` | Exit 0; 150 CLI processes, six native goldens, 60 host-path checks, zero skips. |
| `python3 tests/vectors/generate.py --check` | Exit 0; vectors unchanged. |
| Repeated default build | Exit 0; `ninja: no work to do.` |
| Fresh `planar --help` | Exit 0; documented usage, empty stderr. |
| Exact README combined captured render | Exit 0; JSON stdout, empty stderr. |
| Exact CLI reference split recovery render | Exit 0; JSON stdout, empty stderr. |
| `make install`, `make release`, `make dist`, `make run` | Each refuses with status 2; no output artifact or installation. |

Commands, timing, outputs, configure/build logs, link checks and independent
hash/preservation/count checks are retained in
`tests/native/r09-review-evidence/`. Full debug/release gates were examined from
raw coder evidence rather than gratuitously repeated; the fresh build and
acceptance run specifically verify deletion safety and current documented use.

## Nonblocking editorial observations

1. `docs/testing.md:52` calls the 150 CLI invocations the “path matrix.” The
   complete acceptance suite runs 150 processes; its host-path submatrix runs
   60 checks, as the raw output records. A future prose cleanup should call 150
   the complete CLI acceptance count. The total registry/pass counts are correct.
2. `scripts/cpp-lint-doxygen.sh:42` says the rewrite gate passes a higher retry
   bound. `scripts/rewrite-check.py` does not set one, so the default is three
   unless the caller provides `DOXYGEN_MAX_ATTEMPTS`. Current product toolchain
   documentation correctly states three. This is a stale explanatory comment;
   actual diagnostic failures still fail immediately.

## Limits and next gate

Evidence is native macOS ARM64 only. Linux execution/CI remains unvalidated.
The exact 16 MiB serializer boundary has native unit evidence; large CLI inputs
hit the trusted instruction budget first. Cooperative capture is not an atomic
or hostile-filesystem snapshot. These limitations are disclosed and do not
invalidate the approved render slice. Approval permits the coordinator to close
R09 after its candidate-current check and advance to R10 operator evaluation of
actual packets. It does not authorize persistence, distribution, installation,
publication or merge.
