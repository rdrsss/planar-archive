# V5-02-X06-B3-A-C01 independent review (profile invocation owner correction)

Disposition: **request-changes**. This review covers the combined B3-A+C01
candidate. C01 closes the B3-A findings X06B3A-R1, R3 and R4, and half of R2.
Two required changes remain:

- **X06B3AC01-R1:** a second registration is still undetected. My scratch mutants
  that call `register_vm` twice pass every runtime test, and one test comment
  claims coverage the test does not give.
- **X06B3AC01-R2:** acceptance criterion 6 is not met as written. One of the 957
  current names was removed, and no recorded amendment allows that.

Everything else holds. Admission messages are byte-identical to HEAD. Absence is
an internal non-message signal that only the known-declaration adapter consumes.
EACCES in a directory that cannot be searched is not classified as missing. The
platform difference is documented. The creation-site audit and the N1/N3 reds are
in the evidence. The B3-A owner, the DAG and the full Debug gate are unchanged and
pass.

I reviewed in the shared checkout on `dev/planar-v2-vernacular` at HEAD
`d783c0b9514c50c8a2ad060281c4e75400d5b2dd`, against the uncommitted candidate. I
wrote only this file and new files under
`tests/native/v5-02-x06-b3-a-c01-review-evidence/`. Mutation probes ran in a
temporary detached worktree in my scratchpad, which I removed afterwards. I never
mutated the shared checkout. I did not commit, install, publish or merge, and I
used no old Planar tool, queue, skill, role or installed binary. Builds, ctest and
lint in `build/rewrite-debug` ran one at a time, and no other build process was
running.

## Exact candidate

- `git diff | shasum -a 256` (including `roadmap.md`) at review start:
  `37b200047abf0f6c879af01bc70feab26bf23d588b7920d5cd22589e9cf5835e`.
- Excluding `roadmap.md`:
  `84c68c2e287a9d240bef76d5e183a5f3a4365768cb7216f13fd454449029f2b6`.

Both equal the round record's final hashes and the gate runner's
`raw/diff-sha256.txt` ([`diff-at-review-start.sha256`](../../../tests/native/v5-02-x06-b3-a-c01-review-evidence/diff-at-review-start.sha256)).
The probe patch (`git diff HEAD -- src`) has the same 84c68c2e… hash. Both CMake
lists have the same bytes as at the B3-A review (`candidate-files.sha256`), so the
B3-A DAG audit still applies.

## Required change: X06B3AC01-R1 — a second registration is not detected

The round's R2 says: "The tests must fail on a second capture or registration,
and one red must be shown." C01 removed the constant `registrations`/`captures`
fields, which is acceptable, and added two tests:

- `factories capture exactly once and adoption reads nothing` counts
  trusted-module and profile reads against a `profile_capture::open` + `verify()`
  baseline. It detects a second capture. The coders' red covers `open_opaque`.
  My probe adds a second `profile_capture::open` in `open_known_load`, and the
  test fails three times with `10 == 6`. It also shows zero reads across
  `into_extension`.
- `factory registration counters are deterministic across factories` compares
  `instructions` and `native_bytes` between known, opaque and repeated owners. Its
  comment says "a repeated registration in one factory would diverge from the
  other", and `r2-r3/NOTES.txt` says "double registration in one would [be
  caught]". **This is false.** Each `register_vm` call builds a fresh `command_vm`
  with its own `state`. A factory that registers twice and keeps either VM reports
  exactly the same counters as one that registers once. `capture.snapshot()` reads
  no files, so the read-count test cannot see a second registration either.

I tested this directly ([`probes/INDEX.txt`](../../../tests/native/v5-02-x06-b3-a-c01-review-evidence/probes/INDEX.txt),
`probes/mutate.py`). With a second `register_vm` call whose VM is kept, inserted
in `open_opaque` only, in `open_known_load` only, or in both, `planar_runtime_tests
'[x06b3a]'` passes 294/294 assertions every time. With the mutant in both
factories, the whole `planar_runtime_tests` binary passes (59120 assertions, 352
cases). The adoption half of R2 ("instruction and native-byte counts before and
after adoption") is also missing, because `extension_invocation` has no
observation. The coders record this limitation, but it means that clause has no
substitute.

Required: give the corrective round an observable that changes on each
registration attempt. Two options: a real count kept where registration happens,
such as a snapshot-issued count on `profile_capture` or a count incremented inside
`register_vm` and carried into the owner; or a test-only hook in the style of
`planar_admission_read_attempts`. Then assert exactly one registration per factory
and none across `into_extension`. Show a red against the double-`register_vm`
mutant in at least one factory. Correct the test comment and the evidence note to
match. If an adoption-time counter comparison stays unobservable, say so in the
evidence and cover adoption with the registration observable instead.

## Required change: X06B3AC01-R2 — one current name removed against criterion 6

My inventory ([`inventory-review.txt`](../../../tests/native/v5-02-x06-b3-a-c01-review-evidence/inventory-review.txt),
961 names, identical to the evidence `inventory.txt`) keeps all 940 approved B2-C01
names. Against the 957-name B3-A list, one name is removed:
`replacement.source capture N1 unreadable declaration is not reported as missing`.
Five names are added. The removed test pinned the forbidden `Cannot open admitted
input` text, so its assertions had to change. The new `an unreadable mode 0000
declaration is never reported as missing` section covers the same behaviour. But
criterion 6 says "All 957 current names are preserved", and nothing in the round
permits a rename. The gate report also leaves this to reviewer disposition. A
corrective round is needed anyway, so the cheapest exact fix is to keep the old
name: rename the test that now holds the unreadable-file case back to it, or keep
a test under that name. The alternative is a recorded coordinator amendment of
criterion 6. Either is acceptable. Silent removal is not.

## Prior findings

| Finding | Status | Basis |
| --- | --- | --- |
| X06B3A-R1 message vocabulary / absence | **Closed** | See below |
| X06B3A-R2 constant counters | **Partly closed** | Constants removed; capture detected; registration not detected (X06B3AC01-R1) |
| X06B3A-R3 audit, reds, ownership | **Closed** | See below |
| X06B3A-R4 documentation | **Closed** | See below |

**R1.** `git diff HEAD` of `admission.cpp`, `admission.cppm`, `profile.cpp` and
`profile.cppm` adds no string literal. Every changed `throw diagnostic{…}` is gone
from the diff, and `io_error()` and the metadata-flag code are unchanged.
`Cannot open admitted input` and `io_error_present` appear nowhere in `src`. Across
the whole `src` diff, the only added message literals are existing strings
(`Cannot open/inspect confined input`, `Captured external profile changed`) plus
synthetic test diagnostics.

Absence is the private `refused_absent` bit. It is set only from `errno == ENOENT
&& last && type == kind::supplied_file`, at the `fstatat` failure and at the
vanish-race `openat` failure. In both places it is read immediately after the
syscall and before `io_error()`. It is reset at `open()` entry and at `admit_file`
entry. `capture_admission::last_refusal_was_absent()` exposes it and
`profile_capture::last_admission_refusal_was_absent()` forwards it. The only
consumer is `known_declaration_error(diagnostic, bool absent)`, called from one
site in `open_known_load`, which passes the signal through without branching on
it. The adapter still also requires the unchanged io/message pair. That is an
extra conjunct; it does not replace the signal.

Direct tests: admission level (missing final component, missing intermediate
directory, hard link, directory with mode 0444, moved-from, and reset on the next
admission); adapter level (missing, EACCES directory, mode 0000 file split by
platform); runtime level (missing → `Cannot inspect`, unsearchable directory →
`Cannot open`). The coders' red is against B3-A behaviour with compile stubs
(snapshot `7f605280…`, command in `r1/NOTES.md`). It fails on the EACCES-directory
adaptation and on the `Cannot open admitted input` text. My permissive mutants all
fail: the signal ignoring errno, the signal ignoring `last`, the missing reset, the
adapter ignoring the signal (B3-A behaviour, which also turns the runtime
unsearchable-directory test red, the runtime-level red the coders did not record),
and the factory passing a constant true or false. The platform difference
(`O_EVTONLY` EACCES at admission on macOS; `O_PATH` admits on Linux and the content
read fails later; neither is absence) is documented in `admission.cppm`,
`source_capture.cppm` and `source_capture.cpp`.

**R3.** `r2-r3/creation-site-audit.txt` and `audits.txt` list the product creation
sites. My grep confirms there are exactly two: `extension_invocation::create`
(`runtime.cpp:4809`, through the private pointer constructor) and
`profile_invocation::into_extension` (`runtime.cpp:4969`, through the private
capture+VM constructor). Test-only `create` calls are at `runtime.t.cpp:447` and
`external_registration.t.cpp:3107/3307/3345/3349`. No handler or `src/cmd` source
names either owner, and the generated CLI runtime variants copy the same
`runtime.cpp`. The N1 red is as above. The N3 red (`r2-r3/red-n3.txt`) is a mutant
that removes the moved-from → `invalid_state` branch, which restores HEAD's
behaviour for that check. It records the command, the pre-mutation file and diff
hashes, matching hashes after restore, and fails at `source_capture.t.cpp:674`
(`3 == 0`). Neither red ran against the literal `e2191749…` snapshot, because the
new accessors cannot compile there. I accept the stubbed and mutant reds as
equivalent; each records its own hash. The round record now owns
`src/cmd/planar/CMakeLists.txt`.

**R4.** `runtime.cppm` now says each factory "captures the profile once and runs
the shipped plus external registration chunks exactly once" (accurate in code, but
see X06B3AC01-R1 for the test). The section title now says `invalid_state`. The N1
comments in `source_capture.cpp`/`.cppm` describe the signal and the platform
note. N5 is done (the `@file` brief names the owner entry points). For N1,
`begin_frame` stays public and is documented as a composition seam with no product
caller, which the round allows.

## B3-A criteria re-check

| # | Criterion | Finding |
| --- | --- | --- |
| 1 | Known construction admits root, declaration, registry and ledger before any chunk; hard-link refusal; zero content reads | **Met.** Order unchanged (`runtime.cpp:4868–4885` precede `register_vm` at :4887). Zero-read, hard-link and admission-before-registration tests pass. |
| 2 | Opaque cannot prepare built-in load | **Met.** Surface unchanged; no `prepare_load`, VM accessor or upgrade. |
| 3 | Owner semantics; no second capture or registration | **Partly met.** Moves, consumption and adoption are correct. A second capture is detected; a second registration is not (X06B3AC01-R1). |
| 4 | One active frame; public validation unchanged; names preserved | **Met** for the frame and validation. Names: see X06B3AC01-R2. |
| 5 | Role checks, N1, N3 with direct tests; red-first kept | **Met.** N2 swapped/ledger/observation roles, N3 moved-from `invalid_state` versus poisoned/foreign `capture_changed`, and N1 per R1, all with reds. |
| 6 | No profile acceptance change, no public CLI change, no cycle, no runtime→ledger-acceptance edge | **Met.** No ledger, `acceptance/load`, `prepared` or handler change. The only `src/cmd` change is the owned CMake `DEPENDS`, byte-identical to B3-A, whose DAG audit stands. The coders' `dag-audit.txt` agrees. |
| 7 | Typed failures, no runtime `assert`, docs | **Met** apart from the R1 test comment. No `assert(` added; lint passes. |

## Advisory notes (non-blocking)

- **X06B3AC01-N1 — Embedded and known diagnostics differ for an unsearchable
  directory.** In a scratch probe, `source_capture::open_embedded` on a declaration
  under a mode-0400 directory returns capture/io/`Cannot inspect confined input`,
  because `native.cpp:283–284` maps every final-component `fstatat` failure to that
  text. The known path now returns io/`Cannot open confined input` for the same
  tree. This follows the round's explicit R1 instruction, but it shows that
  `Cannot inspect` means "cannot stat" on the embedded route, not only "missing".
  X07 public-cutover parity must decide whether the known route should match.
  This is a product choice, not a C01 defect.
- **X06B3AC01-N2 — `SKIP` in a root-only branch.** The runtime unsearchable-directory
  test uses `SKIP` when run as root, while its sibling admission and source_capture
  tests use `SUCCEED`. Under a root Linux container gate, this would count as a
  skipped case. Use `SUCCEED` for consistency before any Linux gate.
- **X06B3AC01-N3 — Signal scope wording.** `last_refusal_was_absent` keeps its
  value across later non-`admit_file` refusals on a poisoned handle. The doc's
  "False … after any other refusal" is accurate only for `admit_file`. The known
  factory reads the signal immediately, so nothing is affected.
- Carried duties N2 to N4 from the round record (dead profile-frame branches until
  B3-B, known-owner adoption policy, uncharged `profile_invocation::implementation`
  heap) remain for B3-B/X07.

## Independent gate (macOS Debug, `build/rewrite-debug`)

| # | Command | Result |
| --- | --- | --- |
| 1 | `cmake --build build/rewrite-debug --target planar_tests` | exit 0, `ninja: no work to do.` ([log](../../../tests/native/v5-02-x06-b3-a-c01-review-evidence/01-build-tests.log)) |
| 2 | `ctest --test-dir build/rewrite-debug --no-tests=error -j1` | exit 0, `100% tests passed out of 961`, 330.22 s; zero failed, not-run, skipped or disabled lines ([log](../../../tests/native/v5-02-x06-b3-a-c01-review-evidence/02-ctest.log)) |
| 3 | `ctest -N` inventory | 961 names; all 940 approved B2-C01 names kept; against 957 B3-A names, 1 removed and 5 added (X06B3AC01-R2) |
| 4 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | exit 0, `Formatting and Doxygen checked 136 active source files` |
| 5 | `cmake --build build/rewrite-debug`, twice | both exit 0, `ninja: no work to do.` |

Exit codes are in [`exit-codes.txt`](../../../tests/native/v5-02-x06-b3-a-c01-review-evidence/exit-codes.txt). The tree was already built,
so command 1 did no work. The binaries correspond to the hashed diff because
ninja reported nothing out of date. The diff hashes at review end equal those at
the start ([`diff-at-review-end.sha256`](../../../tests/native/v5-02-x06-b3-a-c01-review-evidence/diff-at-review-end.sha256)). The scratch
probe build compiled the same patch from a clean configure without errors.

## Limits

macOS Debug only. No RelWithDebInfo or Linux run, so the Linux `O_PATH` behaviour
is reasoned from source. Mutation probes and the embedded-parity probe ran in a
scratch worktree built from the hashed candidate patch, not in the shared build
directory. Each mutant was restored, and the restored file hashes are recorded. I
did not rerun the coders' reds against their exact snapshots. No RSS,
whole-invocation budget, preparation or transaction claim is made.
