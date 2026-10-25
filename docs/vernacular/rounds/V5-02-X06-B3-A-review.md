# V5-02-X06-B3-A independent review (profile invocation owner)

Disposition: **request-changes**. Four required changes block approval:
**X06B3A-R1** (the N1 fix changes the closed admission message vocabulary and
does not detect absence specifically), **X06B3A-R2** (the capture/registration
"counters" are constants, so no test can detect a second capture or
registration), **X06B3A-R3** (the evidence lacks the required
`extension_invocation` creation-site audit and the behavioural red-first runs, and
the round record does not own one edited file) and **X06B3A-R4** (inaccurate
documentation in the new API and edited tests).

Most of the owner itself is sound. `profile_invocation` is declared and
implemented in `planar.engine.runtime`. Copying and default construction are
deleted, and the only constructor that takes an implementation is private. Moves
transfer the heap implementation and refuse afterwards. `open_known_load` requires
a codec2 profile intent and takes the selector from it. It admits the workspace
root, the selected declaration file (role `declaration`), registry and ledger
before `register_vm`, registers once, then calls `capture.verify()`. `open_opaque`
has no route to load preparation. `into_extension` hands the same
`profile_capture` and `command_vm` to a private adoption constructor. The
active-frame refactor moves the phase body out unchanged: apart from passing the
frame, the moved body is identical to the old one, so public validation entry
behaviour is preserved by construction. The DAG has no cycle and no
runtime-to-ledger-acceptance edge. The full Debug gate passes for me too.

I reviewed in the shared checkout on `dev/planar-v2-vernacular`, HEAD
`d783c0b9514c50c8a2ad060281c4e75400d5b2dd`, against the uncommitted candidate. I
wrote only this file and new files under
`tests/native/v5-02-x06-b3-a-review-evidence/`. I did not edit, revert or reformat
any product, test, round-record or author-evidence file. I did not commit,
install, publish or merge anything, and I used no old Planar tool, queue, skill,
role or installed binary. Builds, ctest and lint in `build/rewrite-debug` ran one
at a time, with no other build process running.

## Exact candidate

`git diff` SHA-256 at review start and end:
`e21917490f51841de6b687388f02a2da0e5a8d1185534994521890bbaa231e64`. This equals the
gate runner's `raw/diff.sha256`, so the evidence was taken from this tree. The diff
covers 10 tracked files (+922/−59). Per-file hashes, including the untracked round
record and the gitignored `REPORT.md`, are in
[`candidate-files.sha256`](../../../tests/native/v5-02-x06-b3-a-review-evidence/candidate-files.sha256).
The candidate also edits `src/cmd/planar/CMakeLists.txt`, which the round record
does not list (see R3).

## Required change: X06B3A-R1 — N1 fix adds a new admission message and does not detect absence specifically

The round allowed N1 to be closed in either of two ways: detect absence
specifically (at admission, or by checking for ENOENT), or document the macOS
difference. The candidate did neither cleanly:

1. **New message at an existing native site.** `admission.cpp:54` adds
   `io_error_present()`, used at `:203` and `:646`. When `openat` fails after a
   successful `fstatat`, it now reports `io` / `Cannot open admitted input` instead
   of the existing `Cannot open confined input`. X-CAP in
   `external-profile-contract.md` says "Missing/no-follow/capture I/O uses existing
   native mapping" and "New native sites use this closed message table (existing
   propagated native errors keep their exact messages)". The new string is not in
   that table, and it replaces an existing message at an existing site. The change
   is in shared admission, not in the known-load adapter. Every admission caller
   (profile modules, extension late slots, known-load slots) now gets the new text
   for an unopenable present file. No reviewed amendment covers this.
2. **New macOS/Linux divergence.** On macOS the metadata open uses `O_EVTONLY`,
   which returns EACCES on a mode-000 file, so a mode-000 input now gets the new
   message at admission. On Linux `O_PATH` succeeds, and the later native read
   still reports `Cannot open confined input`. Before this change both platforms
   reported the same message for such inputs. Neither the code nor the evidence
   records the new difference. REPORT section (c) says "No statement in the coder
   evidence documents a macOS N1 difference".
3. **Absence is still not detected specifically.** `fstatat` failures still go
   through `io_error()` for every errno other than ELOOP/ENOTDIR. A present
   declaration inside a directory that is readable but not searchable (mode 0444)
   gives EACCES from `fstatat`. Admission then reports `Cannot open confined input`,
   and `known_declaration_error` turns that into the "missing" diagnostic
   `Cannot inspect confined input`. I confirmed the syscall behaviour with a scratch
   probe ([`fstatat-eacces-probe.txt`](../../../tests/native/v5-02-x06-b3-a-review-evidence/fstatat-eacces-probe.txt),
   script `fstatat_probe.py`): `open(dir, O_RDONLY|O_DIRECTORY)` succeeds, and
   `fstatat` on a present child returns EACCES. This is the same misreport class N1
   was about. The comment at `source_capture.cpp:103` ("Admission reports only
   genuine absence as io/'Cannot open confined input'") is therefore false. The
   `source_capture.cppm` doc for `known_declaration_error` makes the same claim.
   (I did not establish what the embedded path reports for this case. The point is
   that the adapter's absence claim is wrong.)

What I ran directly: on macOS the N1 test takes its `else` branch and asserts
`Cannot open admitted input`
([`n1-success-trace.log`](../../../tests/native/v5-02-x06-b3-a-review-evidence/n1-success-trace.log)),
so the test pins the new vocabulary. Its `if (admitted)` branch asserts nothing on
platforms where the metadata open succeeds.

Required: either (a) detect absence specifically without changing any existing
native message, for example a non-diagnostic absence classification from
admission, set only on ENOENT at `fstatat`/`openat` for the declaration slot (the
existing `allow_absent` mechanism is a precedent), and apply the adapter only then;
or (b) keep admission unchanged and document the residual macOS behaviour at the
adapter and in the evidence, as the round allows. Then correct the comments and
docs to match, and show the chosen behaviour by test, including the no-search
directory case or a documented reason why it cannot be tested.

## Required change: X06B3A-R2 — capture/registration counters are constants; criterion 3 is not evidenced by a detecting test

The exported `profile_invocation_observation` documents `registrations` and
`captures` as counters: "One for a live owner; registration is never repeated"
(`runtime.cppm:397–398`). `observe()` sets both to the literal `1` whenever the
owner is live (`runtime.cpp:4988`). No code counts captures or registrations. The
checks `seen.registrations == 1` and `seen.captures == 1` (`runtime.t.cpp:1079–1080`)
would still pass if a factory or `into_extension` captured or registered twice.
Criterion 3 requires that "no second capture or registration happens", and the
architecture's matrix asks for a "registration side-effect test counter". Code
inspection shows a single `profile_capture::open` and a single `register_vm` per
factory, and no capture in `into_extension`. But the tests the report cites cannot
detect a regression, and the public API presents fabricated counts as
measurements. The adoption test also does not compare instructions or native bytes
before and after `into_extension`; only the catalog address is compared.

Required: either make these real counters incremented at the capture and
registration sites, or remove them and add detecting side-effect tests. For
example: watch the trusted module inode and assert an exact read count equal to a
single capture for each factory, assert zero module reads across `into_extension`,
and compare cumulative counters across adoption where a read-only observation is
available.

## Required change: X06B3A-R3 — evidence and record gaps

- **Creation-site audit missing.** B3-A scope item 5 says: "Every actual
  `extension_invocation` creation site is audited and listed in the evidence."
  `REPORT.md` has no such list. My audit (grep of all `src` for `create(`,
  `make_unique<implementation>` and the new constructor): the product has exactly
  two sites, `extension_invocation::create` (`runtime.cpp`, about line 4814) and
  `profile_invocation::into_extension` (`runtime.cpp:4963`, local `adopted`). The
  only other calls are test calls to `create` (`runtime.t.cpp:447`,
  `external_registration.t.cpp:3107/3307/3345/3349`). No CLI or handler code
  constructs one. The candidate is clean here, but the evidence must carry the
  audit.
- **Behavioural red-first not shown.** CLAUDE.md requires the failing regression
  to be shown before the fix, and criterion 5 requires red-first runs to be kept in
  evidence. The only red is a compile failure of the new accessor test
  (`logs/p1-red.log`: `no member named 'moved_from'`). It masks every behavioural
  red. N1 (the old admission message on macOS) and N3 (the three existing sections
  flipped from `capture_changed` to `invalid_state`) are behaviour changes whose
  tests would fail at runtime against the old product. Neither red was run. The red
  command and pre-fix snapshot identity are also missing, as the gate report itself
  says ("Red-first is incomplete").
- **Ownership.** `src/cmd/planar/CMakeLists.txt` was edited: two generated runtime
  variant `DEPENDS` lists gain `planar_acceptance_source_capture` and
  `planar_acceptance_intent`. The edit is needed (`logs/p3-red.log` shows `module
  'planar.engine.acceptance.intent' not found` without it). But the round record's
  owned files and the candidate list name only `src/engine/CMakeLists.txt`. The
  record must assign this file before commit.

## Required change: X06B3A-R4 — inaccurate documentation

- `runtime.cppm:408`: "@note Thread-confined. No Lua runs in the factories". Both
  factories call `register_vm`, which runs the shipped and external chunks.
- `source_capture.cpp:103` and the `known_declaration_error` doc (see R1).
- `source_capture.t.cpp:113`: the section is still titled "moved declaration
  receipt is refused as a capture error", but it now asserts `invalid_state`.

## Criteria assessment

| # | Criterion | Finding |
| --- | --- | --- |
| 1 | Known construction admits root, declaration, registry and ledger before any chunk; hard-link refusal; zero content reads | **Met.** Order at `runtime.cpp:4857–4887` precedes `register_vm`; `capture.verify()` follows registration. Tests: zero read attempts for declaration, registry and ledger, with a positive module-read control; hard links in all three slots refuse `unsafe_path`; an invalid-module hard link refuses before registration would fail. Missing declaration refuses at admission. |
| 2 | Opaque cannot prepare built-in load | **Met** by surface inspection of `runtime.cppm` (no `prepare_load`, VM accessor or upgrade). The `has_prepare_load` concept probes only `prepare_load(0)`, so it would not catch a `prepare_load(intent)`; my verdict rests on the interface, not on that assertion. |
| 3 | Copy deleted, moves preserve addresses and counters, moved-from refuses, failed/consumed publishes nothing, adoption keeps capture/VM/counters with no second capture or registration | **Partly met.** Copy, move, self-move, moved-from and consumed behaviour are correct and tested. "No second capture/registration" holds by inspection but has no detecting test (R2). |
| 4 | One active frame per owner; public validation entry unchanged; names preserved | **Met.** `begin_frame` is once-only and blocks adoption. The phase body is moved unchanged (my diff of the old and new bodies differs only in `active`→`nested` and the forwarded frame). The public and extension paths cannot carry a profile frame. All 940 names are preserved. |
| 5 | Role checks, N1 and N3 with direct tests; red-first kept | **Not met.** N2 (explicit `declaration`/`registry` role checks, swapped and ledger/observation roles) and N3 (moved-from → `invalid_state` before the liveness check; poisoned and foreign stay `capture_changed`) are implemented and tested. N1 fails R1. Behavioural reds are missing (R3). |
| 6 | Fresh profile acceptance refusal unchanged; no public CLI change; no cycle; no runtime→ledger-acceptance edge | **Met.** No ledger, acceptance-load or `src/cmd` source changes. My DAG audit of every `planar_rewrite_module` ([`dag-audit.txt`](../../../tests/native/v5-02-x06-b3-a-review-evidence/dag-audit.txt)) finds no cycle; runtime and every generated variant reach `ledger.format`/`storage` only through the existing `history.read` edge, and never reach `ledger_acceptance`, `acceptance_load` or `acceptance_prepared`. `source_capture` and `intent` do not reach runtime. |
| 7 | Typed failures, no runtime `assert`, docs | **Partly met.** No `assert(` was added; factories and adoption return `std::expected` and catch allocation failures. Lint passes. Documentation accuracy fails R4. |

No `prepared_profile`, `prepare_load`, transaction or `complete_profile_acceptance`
code exists in the diff (the only matches are the negative test concept).

## Advisory notes (non-blocking)

- **X06B3A-N1 — `begin_frame` is public.** The round describes the frame as
  private. In B3-A nothing can use the frame, so a caller can only move the owner
  into a frame-begun state that refuses adoption. B3-B should make it private and
  begin the frame inside `prepare_load`, or explain why it stays public.
- **X06B3A-N2 — The profile-frame branches are not yet exercised.** No product
  path passes a non-null `profile_owner`. `profile_frame_valid`, the skipped
  `begin_invocation` in `invoke_shape_impl` and the profile entry checks in
  `validate_profile_phases` are dead until B3-B. B3-B must cover them with the
  planned cumulative-exhaustion test.
- **X06B3A-N3 — `into_extension` accepts known-load owners.** The resulting
  extension keeps the known `declaration`/`registry`/`ledger` admissions in its
  capture, unused. The architecture says the mode "cannot be changed by parsing or
  an adoption API", and known-load owners exist only for shipped load. Decide in
  B3-B/X07 whether to refuse adoption of known owners.
- **X06B3A-N4 — Accounting inventory.** `register_vm` charges
  `sizeof(extension_invocation::implementation)+64` once. The new
  `profile_invocation::implementation` heap allocation is not separately charged.
  Record this in the metric inventory the architecture asks for, ahead of later
  shared-budget composition. No RSS claim is involved.
- **X06B3A-N5 — Module brief.** The `runtime.cppm` `@file` brief does not mention
  the new profile owner entry points.

## Independent gate (macOS Debug, `build/rewrite-debug`)

| # | Command | Result |
| --- | --- | --- |
| 1 | `cmake --build build/rewrite-debug --target planar_tests` | exit 0, `ninja: no work to do.` ([log](../../../tests/native/v5-02-x06-b3-a-review-evidence/01-build-tests.log)) |
| 2 | `ctest --test-dir build/rewrite-debug --no-tests=error -j1` | exit 0, `100% tests passed out of 957`, no failed, not-run, skipped or disabled lines; 323.12 s ([log](../../../tests/native/v5-02-x06-b3-a-review-evidence/02-ctest.log)) |
| 3 | `ctest -N` inventory against `tests/native/v5-02-x06-b2-c01-evidence/inventory.txt` (940) | 957 registered; 0 removed; 17 added (4 source_capture, 13 `X06 B3 A`) ([compare](../../../tests/native/v5-02-x06-b3-a-review-evidence/inventory-compare.txt)) |
| 4 | `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | exit 0, `Formatting and Doxygen checked 136 active source files` |
| 5 | `cmake --build build/rewrite-debug`, twice | both exit 0, `ninja: no work to do.` |
| — | N1 test with `-s` | else branch taken on macOS; 14 assertions pass |

Command 1 was already up to date, so this gate does not include a fresh compile of
the tree. The binaries match the hashed diff because the build reported no work.

## Limits

macOS Debug only. No RelWithDebInfo or Linux run, so the Linux `O_PATH` side of R1
is reasoned from source, not run. The no-search-directory case in R1 is shown by a
syscall probe plus source reading, not through the product. I did not reproduce
the missing behavioural reds myself. No RSS, whole-invocation budget or X06
preparation/transaction claim is made.
