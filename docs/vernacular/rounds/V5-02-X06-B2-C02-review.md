# V5-02-X06-B2-C02 independent review (combined B2+C01+C02)

Disposition: **approve**. The C02 documentation correction closes
**X06B2C01-R1**. The combined B2+C01+C02 candidate is approved. There are no
required changes. Four minor wording notes follow; none blocks approval.

The eight product and test sources are byte-identical to the C01 review's
identity table, so the code that review accepted is unchanged. The C01
`REPORT.md` now has the criteria-to-test map, the red-first account and the X-CAP
record. Every file:line it cites matches the files on disk, every claim about red
evidence matches the retained snapshots and raw logs, and the gap in the A-run
command lines is stated plainly rather than filled with a guessed command. The
original gate section is kept byte-exact. Every reference that the C01 review
found dangling now resolves. Advisories N1 to N3 are carried to the roadmap
next-work row.

I reviewed in the shared checkout on `dev/planar-v2-vernacular`, HEAD
`bcfe1ad7ed94a85985f1189deba3b12cc851e7bd`, against the uncommitted combined
candidate. I wrote only this file and new files under
`tests/native/v5-02-x06-b2-c02-evidence/review/`. The files the gate runner had
already written in that evidence directory (`REPORT.md`, `inventory.txt`, `raw/`)
are not mine, and I left them untouched. I did not edit, revert or reformat any
product, test, round or author-evidence file. I did not commit, install, publish
or merge anything, and used no old Planar tool, queue, skill, role or installed
binary. My one build command and one ctest run in `build/rewrite-debug` ran one
after the other. No other ninja, cmake or ctest process was running.

## Exact candidate

Recomputed with `shasum -a 256`. The values are recorded in
`tests/native/v5-02-x06-b2-c02-evidence/review/candidate-hashes.txt`.

| File | SHA-256 | Against C01 review table |
| --- | --- | --- |
| `src/engine/acceptance/source_capture.cpp` | `db0e6dfdfd7e6f03adfce25c87eaf15e71f8c5ddf15cc7de4dbadc0396545cc9` | equal |
| `src/engine/acceptance/source_capture.cppm` | `58865a5a328b0d157cc9bd0f6621a0601487e10feea2b3126cd8f353b2e2791e` | equal |
| `src/engine/acceptance/source_capture.t.cpp` | `3924ecc0c0ff9aed37a858d7d8d78908dbba961c2accc0c59aedfb2a1934e182` | equal |
| `src/engine/acceptance/load.cpp` | `caf038172725875e88e26800733467aa26cc1e06bd1286e11579c41bc25412be` | equal |
| `src/engine/CMakeLists.txt` | `74721d41994951236a70388730e7e30dcca424559f382b6765c653f2fc1adfca` | equal |
| `src/engine/capture/admission.cpp` | `7886d7407df2679a90b234da46b01e7f7f711fa6c3e3c418eb208b26c77bd109` | equal |
| `src/engine/capture/admission.cppm` | `b289112661a17394d083f3b853c4f5bf4c96efac3bc0dc46ec7897acfb34cf2f` | equal |
| `src/engine/capture/admission.t.cpp` | `75ffa7bed2bebc3f1c1d8f1141cfb812971447509f03d55b08805e1ef52b9ef1` | equal |
| `tests/native/v5-02-x06-b2-c01-evidence/REPORT.md` (C02 candidate) | `73381f423dfa75c766980256385add3418d6d0fc21b46d7ad9d6112368ef4b07` | changed, as intended |
| `docs/vernacular/rounds/V5-02-X06-B2-C02.md` | `f0af5611e6fd30b6bf843b1d9385d3e13ba05d2c4c0775dad0ccdadfe9be0fd7` | new |
| `docs/vernacular/roadmap.md` | `4d09289f4bcc1a66ceee383b3c5db882db14066583a5b34348ca02d111525af0` | changed (coordinator) |
| `docs/vernacular/rounds/V5-02-X06-B2.md` | `224ac11056e0635422324b5f4e7ee6360424e81f2d5adf4c1f4aa667f1e45589` | changed (coordinator outcome section) |
| `git diff` (tracked files) | `7dc155c0ac55e0c9b88bdb70341246d63c78e6413835b70d0618f12ddc404fb6` | differs from `5049…` only because of `roadmap.md` |

`git status` lists the same modified and untracked paths as at round start, plus
the new C02 record. The tracked-diff hash changed only because the coordinator
edited the roadmap. Every tracked source in the diff (`CMakeLists.txt`,
`load.cpp`, `admission.{cpp,cppm,t.cpp}`) still hashes to the C01 value. No
product or test byte changed in C02.

## Check of X06B2C01-R1

### Item 2 — criteria-to-test map: met

The new "Criteria to test map" lists the ten B2 criteria (wording checked against
`V5-02-X06-B2.md` lines 38-56) and the C01 rows: R1 diagnostic, R2 construction
refusal, N2 post-seal mutation and the registry/declaration hard link. I checked
every cited location with `grep -n` and `awk` against the current
`source_capture.t.cpp` and `admission.t.cpp`.

- Test case starts: SC:110, 154, 221, 240, 254, 269, 285, 342, 388, 423 and 482 are
  the exact `TEST_CASE` lines with the quoted names. SECTIONs at 113, 126, 137, 156,
  178, 193, 206, 287, 305, 328, 389, 409, 433, 454 and 465 are exact.
- Detail lines: arity `static_assert` at 105-108. Hard-link `path_denied` /
  "Hard-link input alias" at 234-235. `spec_captured` at 353-356, with the
  comment on 352. Mutation 3 at 376-377. The `capture_changed` checks at 380-384.
  The raw diagnostic and adapter at 433-452, passthrough at 454-464, and
  removed-after-admission at 465-479. AD:86 starts
  `capture admission receipts report same admission and liveness`, and the case
  ends at 162; the cited range 86-161 covers its assertions. AD:148 is the comment
  and AD:149 the inverted `CHECK`. All are correct.
- Limits are stated per row and overall: Debug macOS only, the adapter is not
  wired, real registration ordering is left to the runtime row, and no role check
  is made. The map agrees with the C01 review's "Criteria assessment" table.

### Item 1 — red-first section: met

- **Regression B.** The two commands in REPORT.md match the `$ cmake --build ...
  planar_acceptance_source_capture_tests` (exit 0) and `$ ctest ... -R "source
  capture"` (exit 8) lines in `raw/B-red.log`. The `red-snapshot-B/` files hash to
  `db5feedb…` (cppm), `4f4c4290…` (cpp) and `83aedf8f…` (t.cpp), matching
  `raw/B-red-snapshot-hashes.txt` and `raw/B-red.log`. The five failing lines 146,
  171, 323, 445 and 450 match `raw/B-red-lines.txt` and the `grep -n` output in
  `raw/red-line-verify.txt`, and they fall in the cases and SECTIONs the table
  names. `raw/red-line-verify.txt` confirms that the red `source_capture.cpp` has
  the three-line `known_declaration_error` stub and no construction predicate
  (count 0). My own `diff` of `red-snapshot-B/source_capture.t.cpp` against the
  current file shows only lines 225-226 (alignment of `auto p` and `auto r`), as
  the report says. Guards are labelled as guards: moved-from root, poisoned
  registry and poisoned root (already refused at `add_root`), the
  registry/declaration hard link (`raw/B-hardlink-probe.log` shows `code=3
  detail=path_denied`, `sealed=0`, `p.verify=0`) and N2. This matches the "Sections
  PASSING at red" line of `raw/B-red-lines.txt`.
- **Regression A.** The `red-snapshot-A/` files hash to `72520ca6…` (cpp),
  `b2891126…` (cppm) and `64819c70…` (t.cpp), matching `raw/A-red-hashes.txt`.
  `raw/A-red.log` reports test #52 Failed in 0.02 s, with "assertions: 60 | 43
  passed | 17 failed", and its failure lines are exactly 115-120, 123-126, 132,
  133, 145, 146, 147, 156 and 157, as listed. Snapshot line 147 is
  `CHECK_FALSE(admission_root->same_admission(*admission_file))`. My `diff` of
  `red-snapshot-A/admission.t.cpp` against the current file shows it replaced by
  the comment at 148 and the `CHECK` at 149. The report labels it a wrong
  expectation rather than a real red and counts 16 real reds. That is the
  correction R1 item 4 required. Attempt 1 is correctly set aside as a fixture
  bug: `raw/A-red-attempt1-fixture-bug.log` fails `REQUIRE( admission_file )` at
  `admission.t.cpp:142`, and its test hash `fd244831…` matches
  `raw/A-red-attempt1-hashes.txt`. `raw/A-green.log` shows the test passing 1/1,
  and `raw/A-green-hashes.txt` gives the final admission hashes.
- **The A command gap.** `raw/A-red.log` and `raw/A-green.log` both start at
  `Test project ...`, and the A build logs are ninja output only, so no command
  line was recorded. The report says exactly that. It lists what is known (test
  #52, the build directory, the `Filters:` line, the timings), notes that the
  output looks like a `ctest -R` run but that this is unverified, and does not
  invent a command. R1 item 1 originally asked for the exact A command. That
  information does not exist in the retained evidence, and the C02 brief rightly
  asks for an honest statement instead. I accept the gap as a documented and
  permanent limit of the A red-first record. The hashed snapshot, the failing-line
  match and the single-test filter are enough to establish that the red state was
  real.

### Item 3 — X-CAP record: met

The record quotes X-CAP correctly. Line 172 of `external-profile-contract.md`
reads "Missing supplied paths retain existing family capture errors". The
`capture_changed` sentence begins on line 203 and ends on 204. The anchor
`#x-cap--strict-capture-and-trust` resolves. Case (a): a declaration removed after
admission stays `capture_changed` / "Captured external profile changed", with zero
reads and a poisoned admission (SC:465-479). Case (b): a declaration missing when
supplied gets the raw `io` / "Cannot open confined input" / `profile` diagnostic,
which `known_declaration_error` (`source_capture.cpp:103-107`, verified) adapts to
`io` / "Cannot inspect confined input", with an empty limit and maximum 0, equal
to `open_embedded`. The record states that the adapter is not yet wired. Of the
two readings in the C01 review, this is the one I find correct.

### Item 4 — line 147 correction: met (see Regression A above)

### References: met

- The comment at `source_capture.t.cpp:466` is unchanged, as the hash shows. It
  now points to a REPORT.md that contains an X-CAP record naming SC:465-479 and
  466.
- `V5-02-X06-B2.md` line 250 says REPORT.md holds "exact commands, snapshot
  hashes, verified failing lines, and the criteria table", and line 147 refers to
  the criteria map. All four now exist. Line 251's "Four R2/R1 regressions failed"
  agrees with the report: four regressions (moved registry, registry from a second
  admission, declaration from a second admission, R1 adapter) produce the five
  red lines.
- I parsed every Markdown link in REPORT.md, the C02 record, the B2 record and the
  roadmap with a script (45 links in the first three, plus the roadmap's links),
  including the heading anchors. All resolve. Every `raw/…`, `red-snapshot-*/…`
  and inventory path named in backticks in REPORT.md exists.
- The original gate section is preserved byte-exact. REPORT.md lines 1-33 plus
  118-120 hash to `cb1674887b68…`, which is the REPORT.md identity in the C01
  review table. The three new sections sit between "Zero-work result" and
  "Limitations".

## Advisories carried (acceptance criterion 5): met

The roadmap next-work row "X06 remaining code rounds" carries N1 (macOS
`O_EVTONLY` EACCES on a mode-000 declaration, rewritten to "Cannot inspect
confined input"; detect absence or document it), N2 (no receipt role check) and
N3 (`capture_changed` for moved-from, poisoned and foreign receipts; moved-from
may merit `invalid_state`). It also carries the known-load obligation to apply
`known_declaration_error`. REPORT.md's X-CAP record repeats all three. The next
runtime round record does not exist yet, so the roadmap row is the carrier until
it does. The roadmap "Pending" status describes C02 accurately as awaiting this
re-review.

## Gate and spot check

No source byte changed, so the independent Debug 940/940 gate in the C01 review
stands. The C02 gate runner's raw logs are consistent with this:
`raw/ctest.log` shows "100% tests passed out of 940", `raw/gate2-ctest-N.log`
shows "Total Tests: 940", `raw/gate2-nochange.log` shows "ninja: no work to do.",
and `raw/sha256.log` equals the table above. Its `inventory.txt` is identical to
the C01 inventory: 0 names added, 0 removed.

My own commands, unpiped, with logs under
`tests/native/v5-02-x06-b2-c02-evidence/review/`:

1. `cmake --build build/rewrite-debug`: exit 0, "ninja: no work to do."
   (`1-build-nochange.log`). The built tree is current for these bytes.
2. `ctest --test-dir build/rewrite-debug --no-tests=error -R "source capture|capture admission receipts" --output-on-failure`:
   exit 0, "100% tests passed out of 12". That is 11 source-capture cases plus the
   admission receipt case (`2-focused-ctest.log`).
3. `python3 scripts/redact-local-paths.py --repo-path "$PWD" --home "$HOME" check`
   on the C01 REPORT.md, the C02 record, the roadmap, the B2 record, the B2 and C01
   reviews, the B2 and C02 evidence REPORT.md files and the eight sources: exit 0.
   A `grep` found no absolute home or repository path in these documents. This
   review file contains none either.

## Advisory notes (non-blocking)

- **X06B2C02-N1.** The B2-2 row says the read count becomes `> 0` "after capture".
  In the test, SC:251 checks it right after `open_bound` (SC:248-250), before any
  `capture()` call. The wording is loose, but the cited line is correct.
- **X06B2C02-N2.** The X-CAP record cites "line 203" for a sentence that runs from
  203 to 204 of `external-profile-contract.md`.
- **X06B2C02-N3.** Regression A does not say that `red-snapshot-A/admission.t.cpp`
  also differs from the final file in clang-format line wrapping at snapshot lines
  139 and 150. Regression B states its formatting-only difference. The pre-format
  copy and format-check logs are linked, so nothing is hidden.
- **X06B2C02-N4.** The C02 record's "Verification run" section points to "the
  final coder report" rather than recording results. This review records the
  checks independently, so the gap has no effect.

## Limits

This is a documentation and evidence review. Debug on macOS only. I did not run
RelWithDebInfo, Linux, sanitizers or a full ctest pass; the full 940/940 result
comes from the C01 review and the C02 gate runner, on identical source bytes. I
did not re-run the red states, and verified them from the retained hashed
snapshots and logs. The A-run command lines cannot be recovered from the evidence.
The report's statement of that gap is accepted as a permanent limit, not
reconstructed. No claim is made about X06 runtime preparation, wiring of
`known_declaration_error`, known-load admission timing, transactions, shared
budgets or RSS. Commit, redaction `redact` and staging remain the coordinator's
duty under the orchestration rules.
