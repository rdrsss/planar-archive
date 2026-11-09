# R15–R18 integrated acceptance: C13 requires C14

Disposition: **request-changes** for GA-C13-1. The completed C13 Release gate
has one source-audit assertion failure. The Debug gate was coordinator-cancelled
after that deterministic failure was known and receives no full-gate pass credit.
C13's bounded source correction, descriptor fault proofs and fresh calibration
remain separately valid. No integrated acceptance approval is granted.

This reviewer modified only this report and owned ignored audit records. No
product/test/build output, installed binary or operator database was modified or
executed. An available ordinary agent substituted for the unavailable prescribed
specialist model. The integrated pass stopped at the confirmed gate finding;
it does not claim a fresh exhaustive line-by-line review of every unchanged
implementation or a completed publication audit.

## GA-C13-1 — blocking gate failure: audited platform count is stale

The completed macOS Release gate exited **1** after CTest exited **8**. Independent
parsing finds exactly **1,844 actual result rows: 1,843 passed, one failed, zero
skips**. The sole failure is:

`replacement.X08 platform branches are exactly the audited set`

The failing assertion is in `src/cmd/planar/bootstrap.t.cpp:3164`, comparing
`x08::platform_counts(scanned)` with `x08::audited_platform_counts()`.
The expected-map entry in **`x08::audited_platform_counts()`** still assigns
**1** to `engine/process/runner.t.cpp`.

An independent reproduction of the exact source-token scan visits 271 source
files after its documented exclusion. Every expected-map entry matches except
`engine/process/runner.t.cpp`: **actual 2, expected 1**. Its existing macOS guard
creates the intentionally inheritable subject. C13 adds a second macOS guard
immediately before spawn to assert that this subject has not been accidentally
marked CLOEXEC by late fixture capture. That assertion is required by the
approved C13 plan and independently killed the late-recapture fault. This is a
stale exact audit inventory, with no runtime defect established by the failure.

C14 must map and independently review the additional test-only platform branch
and reconcile the exact audited inventory. Preserve the required pre-spawn
assertion and exact scanner; do not disable the test, ignore the new token or
replace equality with a tolerant bound. Then run the required final host and
Linux gates on the corrected candidate. This reviewer makes no source edit.

Raw completed evidence is
`tests/native/r15-r18-c13-evidence/host-release/gate-1.log` and its exit sidecar.
The full helper stops at CTest failure, so its final formatting/Doxygen and
repeat-build stages were **not reached**. Earlier focused formatter/no-work
passes are separate evidence. Debug cancellation is explicitly recorded in
`host-debug/gate-1-cancellation.json`; partial successes are retained without
complete-gate credit. No final C13-source full Linux gate was started.

## Scoped source and descriptor evidence retained

The source-review chain reaches the last production correction at C08 and later
bounded verification corrections. C12's final request-changes disposition remains
correct historical evidence: its two Linux descriptor tests failed. C13's exact
pinned diagnostic identifies descriptor 3 as CTest's temporary log, inherited
through CTest, native test and check child with the same target/inode/flags and
without CLOEXEC. Planar-owned cwd and pipe descriptors have CLOEXEC and are absent
from the child. Direct clean runs pass. R17-2 rule 2 promises CLOEXEC for
Planar-owned Linux descriptors; the correction introduces no runtime close-all
policy.

The C13 bounded source reviewer approves fixture isolation before newly created
subjects, original-flag restoration on ordinary/partial-error/unwinding paths,
observer exclusion and descriptor identity checks, a dynamic non-3 observed-range
marker and the immediate macOS inheritable-subject assertion. Original exact
empty-output and `9\n` assertions and all 28 test names remain intact.

Current `runner.t.cpp` SHA256 is
`70ea1208b8a70020523a94e1e94a1c6d52b2aee23cbe67263f6c96bdf804c65f`.
Its 145 insertions preserve all original lines; the other 271 source files are
unchanged from the diagnostic candidate. This pass independently rehashes all
272 current source files and all 190 recorded production inputs. It also checks
the nine bounded-review audit bindings, the complete 394-file proof index and
both bound reports. The bounded independent source review semantically verifies
all six compiled faults and eight intended assertion failures:

- Linux pipe and devnull CLOEXEC omissions violate exact empty output.
- A hollow probe violates the original exact descriptor-9 control.
- Failed original-flag restoration violates three restoration checks.
- The macOS default-flag omission exposes the intentionally inheritable subject.
- Late fixture capture violates the immediate pre-spawn inheritable-flag check.

Healthy/restored native suites are **28 cases / 248 assertions on Linux** and
**28 / 252 on macOS**, zero skips. Four macOS-only assertions explain that
difference. Linux registered process tests pass 28/28 before and after faults.
The positive probe still observes only descriptors 3 and 9; its marker-23 pass
receives no leak-detection credit. Setup/matcher attempts remain preserved and
excluded. These scoped results do not substitute for the missing complete gates.

## Fresh C13 calibration independently checked

Executed the owned read-only checker:

```sh
python3 tests/native/r15-r18-final-evidence/final-review-c13/audit_calibration.py
```

Exit **0**. It independently parses every normal XML and RUN record, verifies
exact unique inventories and case selection, recomputes all five peak columns,
column-specific exclusions, integer margins, native-boundary instruction checks
and every compact cell. Each configuration reproduces **509 cases / 87,118
assertions**, zero failures, expected failures or skips, **569 RUN records** and
**31 budget fixtures**. The inventory comprises 484 ordinary runtime cases,
three explicitly named private helpers and 22 loader cases; private helpers add
no product-coverage claim.

Eligible maxima in both configurations are native **39,999,031**, Lua
**16,527,445**, invocation **21,516,200**, registration **5,000** and nodes
**27,873**. Every eligible cell satisfies `4 * peak <= 3 * ceiling`; minimum free
margin is **25.203826%**. The 51 MiB / 29M / 1M / 37,500 tuple is a test variant;
production limits are unchanged. Debug direct-shape Lua differs from C11 at
3,286,616 versus 2,759,555, while eligible maxima remain unchanged; these new
rows are not represented as byte-identical old measurements.

Independent hashing verifies all **44** calibration evidence bindings and
**2,266** raw files, every **1,502** local input per configuration, all
**2,044/2,043** external input/header entries and private compiled artifacts.
The snapshots include 101 explicit PCMs per configuration. Fresh compile-depfile
reconciliation and its bound records are retained by the calibrator; this pass
independently rehashes the declared closure and actual artifacts rather than
claiming reuse of old C11 binaries. The builder differs from the retained C11
instrumentation driver only in the private output namespace.

Independent raised checks parse all **16** actual XML/raw samples, reproduce
native/instruction padding slopes **23/22** and **692 nodes per job**, and verify
both role controls. The shared role has exactly the intentional `REQUIRE_FALSE`
failure with original `!(subject.ok)` and expansion `!true` under the raised
400M ceiling. Its measured instruction tail is **27,719,200 / 28,620,200 /
29,521,200**. This raised refusal assertion is excluded from normal green
acceptance. Preliminary path-alias verifiers are preserved exclusions. No new
calibration defect was found in this scoped check.

## Remaining acceptance and evidence boundaries

C11's literal old-tuple build, instruction isolation, section discovery proof
and earlier execution/budget campaigns retain their original scoped evidence;
this stopped pass does not broaden their carry to all current source or every
SDK dependency. C13's fresh actual normal and raised measurements close its
post-engine-edit measurement obligation. Later source changes need an accurate
current overlay and review of their affected scope.

The coordinator's retention indexes record reversible compression of owned C11
scratch inputs, with original repository logs/manifests unchanged. The archives
and per-file indexes resolve the original paths; this pass has not independently
restreamed every retained historical archive member. No data-loss finding is
inferred from those archived paths.

Final current references, compact publication selection, reversible path
redaction, staged candidate inspection and complete final platform gates remain
unfinished. No publication approval follows this request-changes disposition.
R19's real previously used project/spec remains unselected; fixture success
cannot certify it. Conditional human policy and deferred composition or
controlled model/grouping experiments remain unaccepted. Linux coverage is
aarch64; x86_64 is not covered. Installation, pushing, publication and merging
remain unauthorized.

Owned audit records are under
`tests/native/r15-r18-final-evidence/final-review-c13/`, including
`platform-branch-failure.json`, `host-gate-failure.json`,
`independent-c13-calibration-audit.json` and `review-probe-bindings.json`.
A fresh C14 correction and final integrated reviewer must close GA-C13-1 and the
remaining acceptance work before commit.
