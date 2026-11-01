# V5-02-RA01-B-W2-C01 mutation probes (focused re-probe after the survivor repair)

Prober run of 2026-10-31. The previous PROBES.md, raw/ and results.jsonl are preserved unchanged in `superseded-1/`. Only `tests/native/v5-02-ra01-b-w2-c01-evidence/probes/` was written in the checkout. The warm scratch copy `probe-warm` (outside the checkout, debug preset, kept in place) was synced with `rsync -a --delete --exclude build/ --exclude .git/ --exclude 'tests/native/*-evidence/'` and rebuilt incrementally. Baseline before the first mutant and after the last restore and rebuild: `ctest --no-tests=error -R "C01|RA01-B|X08 C01" -j8` gave 228/228 passed (`raw/FINAL-BASELINE.ctest.txt`, `raw/FINAL-BASELINE2.ctest.txt`; was 227 before the repair added one test). Driver `raw/driver/drv.py` (same driver as superseded-1, only TARGETS narrowed to `planar_capture_tests planar_capture_directory_tests`, the only executables holding the named tests; no generated variant target depends on directory.cpp or capture.cpp). Mutant definitions: `raw/driver/m5.json`, `m6.json`, `m7.json`. Per mutant: exact-anchor replacement (anchor occurs exactly once, a failed apply is `patch-failed`, not a kill), targets rebuilt, named test run with `ctest -R`, file restored by byte copy with sha256 verified (`verified=True` in each log), targets rebuilt after the last restore. A kill needs the named failing test in the ctest failure list. Platform: macOS, non-root (root-aware branches ran for real).

## Candidate identity (identity.py, verbatim; equal before the first mutant and after the last restore: `raw/identity-before.txt`, `raw/identity-after.txt`, diff empty)

| identity | value |
|---|---|
| HEAD | `3343ad38304db57388bc7ac2438a40043eba1b7d` |
| sha256(git diff --binary HEAD -- src tests/native), tracked only | `d7071fd2cdacee177cd4d447c707c11c5349541bb1fea790d91377912f454665` |
| combined sha256 (diff bytes, then each untracked path NUL bytes, sorted), pathspec src tests/native, evidence excluded | `1f30a26e7150d3de4795dfebaf924707beeadaf54bffbe8b93fe63ed03d67b55` |

The per-file table is `raw/identity-after.txt`. Compared with superseded-1 (combined `afc66baa...`), the only changed file is `src/engine/capture/directory.t.cpp` (`fa99733d...` to `c992a209...`): the repair added `RA01-B2 C01 observe maps a failed entry inspection after listing to cannot inspect`. No product file changed.

## Focused re-probe rows (run on this candidate)

| id | invariant | mutant (sha256 prefix of diff) | result | killing test | log |
|---|---|---|---|---|---|
| E01 | read() failure site in directory read_file is io | `read_file` read() error branch cannot_inspect to denied_kind (`fe41e685f1414b66`) | survived | - | `raw/E01.log` |
| E02 | fstatat failure after listing (scan_directory) is io | fstatat failure cannot_inspect to denied_name (`57f5722b20ae62f7`), named test is the new repair test | survived | - | `raw/E02.log` |
| E02b | fstatat failure after listing (scan_root) is io | same in scan_root (`f3cd3f85d73512fd`) | survived | - | `raw/E02b.log` |
| E02c | which site the new repair test actually reaches | list() `openat(dir, ".")` failure cannot_inspect to denied_name (`44cead6ef0a41350`) | killed | RA01-B2 C01 observe maps a failed entry inspection after listing to cannot inspect | `raw/E02c.log` |
| E03 | readdir errno after open is io | list() `if (errno)` cannot_inspect to denied_kind (`21fa0acbd7ba0a40`) | survived | - | `raw/E03.log` |
| E05 | capture_diagnostic post-read byte bound | post-read `bytes.size() > maximum` refusal deleted (`c34f8af6ce8e1808`), test filter `RA01-B2 capture_diagnostic` (2 tests) | survived | - | `raw/E05.log` |
| E11 | readdir errno site (root listing) is io | same list() errno branch cannot_inspect to denied_name (`b2504f0714f20915`), filter `RA01-B2 C01 observe` | survived | - | `raw/E11.log` |

Survivor analysis (the repair did not close E02, E02b):

- E02 and E02b still survive. The repair test makes a directory readable but not searchable (`owner_read`), expecting the fstatat after a successful listing to fail. It never reaches fstatat: `list()` first does `openat(dir_fd, ".", O_DIRECTORY)`, which needs search permission on that directory, so the listing itself fails with EACCES at that openat and returns cannot_inspect. E02c proves it: mutating that openat failure to denied_name kills the new test, while mutating the fstatat sites does not. The test therefore guards the list() openat site (already killed by `observe maps an unreadable entry to cannot inspect` only for the mode-000 case; E02c is the first kill of the read-only case), but its comment ("kills the mutant that maps the fstatat failure ... to denied_name") is wrong. The fstatat failure after a successful listing needs a race (entry removed or directory made unsearchable between listing and stat, since a listable directory is always searchable for the opener's own descriptor chain) or a syscall fault seam, which D-C01-13a forbids. This is a limit, not producible by a fixture: the record should say so and correct the test comment, or accept it as an equivalent-by-unreachability survivor.
- E01 (read() error other than EINTR on a regular file, for example EIO), E03 and E11 (readdir errno after a successful opendir), E05 (file grows between the gate and the read; the bound is a defence against a race, and the Linux/macOS read size is already bounded by the gate size in `read()`): each needs a fault or race not producible by a fixture, unchanged from superseded-1. All survive with no killing test.

## Carried-forward rows re-run because the repair changed their test file (directory.t.cpp)

All 10 killed again on this candidate (`raw/driver/run2.out`).

| id | mutant diff sha256 prefix | killing test (failing list) |
|---|---|---|
| C01-01 | `799924acf8a0e434` | RA01-B2 C01 observe maps an unreadable entry to cannot inspect |
| C01-02 | `855db61ed68bdb5c` | RA01-B2 C01 observe maps an unreadable entry to cannot inspect |
| C01-03 | `909cd755208716a3` | RA01-B2 C01 observe maps an unreadable entry to cannot inspect |
| C01-05 | `1a5a5cc31ceb8267` | RA01-B2 C01 observe refuses a symlink root |
| C01-06 | `f4750820a6e6bf92` | RA01-B2 C01 observe bad_alloc refusal carries observation_total_bytes |
| C01-07 | `8a9de6f6c4a1911e` | RA01-B2 C01 observe applies precedence in sorted name order (macOS, no Linux fallback needed) |
| C01-08 | `49880f62c0dd9462` | RA01-B2 C01 observe applies precedence in sorted name order |
| C01-38 | `3d029392d654182b` | RA01-B2 C01 observe recheck bad_alloc is a limit and not a membership change |
| C01-39 | `5e5e953214564e91` | RA01-B2 C01 observe recheck growth past the total bound stays membership changed |
| E10 | `45ebe2a7e09716c3` | RA01-B2 C01 observe recheck bad_alloc is a limit and not a membership change |

## Rows carried forward unchanged from `superseded-1/PROBES.md`

Every other row (C01-04, C01-09 to C01-14, C01-18 to C01-25, C01-33 to C01-37, C01-44, C01-45a to c, E04, E06r, E07, E08r, E09, and the build-failed first forms) keeps its result from superseded-1: neither the mutated file nor the killing test file changed (identity comparison above; only directory.t.cpp changed, and every row whose named test lives in it was re-run). Acceptance 2 summary (X16, X17, X03, X04, X05, X07, W2-51 and W2-06n) therefore stands as recorded in `superseded-1/PROBES.md`: X17 by C01-05, W2-51 by C01-07 and C01-08, both re-confirmed above; X16, X03, X04, X05, X07 carried. C01-04 and C01-33 (authoring_observe.t.cpp, directory.cpp file unchanged) carried, not re-run.

## Red-first

Unchanged from superseded-1 for the 20 earlier tests. For the added test `RA01-B2 C01 observe maps a failed entry inspection after listing to cannot inspect`: the mutant that fails it is E02c (list() openat failure to denied_name), run and killed. The mutants named for it by the repair (fstatat sites, E02, E02b) do not fail it. A test that fails under a mutant of the fstatat site does not exist, so its stated purpose is not met.

## Skipped

Exploratory budget: 3 extra mutants used (E02b, E02c, and E02 re-pointed) of 20. Not run: Linux container re-run of root-aware rows (as root they SUCCEED), RelWithDebInfo (Debug scratch build only), wave-2 survivor rows W2-23, W2-67, X01, X11, X13 to X15 (unchanged, see superseded-1).

## C02 annotation (records only, added by C02 package D1, R2)

The rows above are unchanged history. Reading them with the C02 correction: the test `RA01-B2 C01 observe maps a failed entry inspection after listing to cannot inspect` guards the `list()` `openat(dir, ".")` failure on a readable but unsearchable directory, which is mutant E02c (killed). E02 and E02b (fstatat after a successful listing, `scan_directory` and `scan_root`) are STATED LIMITS: they need a race or fault seam, none may be added (D-C01-13a), and no test kills them. The test comment was corrected by C02 (comment only, no rename). C01-46 has no red-first record, recorded as a limit. The C02 probe stage re-runs the E02c row under C02-37 in `tests/native/v5-02-ra01-b-w2-c02-evidence/probes/`.
