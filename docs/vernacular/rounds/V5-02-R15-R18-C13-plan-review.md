# C13 independent plan review

Disposition: **approve the conditional diagnostic-first plan**. The focused
exact-image diagnostic may proceed. The proposed fixture repair is released only
after its recorded external-descriptor provenance condition is satisfied. This
is plan approval; source correctness, fault sensitivity, measurements, platform
results and integrated acceptance remain to be reviewed. No build, product/test
edit, installed binary or operator data was used by this reviewer. An ordinary
available model substituted for the unavailable prescribed specialist model.

Read the C13 brief, source map and complete decision/invariant tables, the
implementation/roadmap/v5 authority, R17-2 rule 2 and R17-11, the runner interface,
implementation and colocated tests, test registration, prior C11/C12 review and
measurement obligations, and the archived-input retention record. Input hashes
are recorded under `tests/native/r15-r18-c13-evidence/plan-review/`.

## Contract and diagnostic boundary

R17-2 explicitly requires close-on-exec for Planar-owned Linux descriptors and
adds `POSIX_SPAWN_CLOEXEC_DEFAULT` on macOS. It supplies no Linux policy for
closing arbitrary descriptors inherited from an external driver. The existing
Linux `pipe2(O_CLOEXEC)` and `/dev/null` open, borrowed cwd and spawn actions
therefore need ownership evidence before any runtime defect is inferred.

The two full Linux assertion failures are valid red evidence. Cached alternative
Ubuntu shell controls and the four green macOS ambient-marker runs are useful
diagnostics with their stated scope; they do not identify the actual Linux gate
descriptor. D01/D04 and I01 correctly require the current pinned source/toolchain
and existing runner target, clean direct invocation, deliberate ambient-marker
invocations, CTest comparison and passive procfs observations. No diagnostic C++
instrumentation or official-runner change is needed.

The provenance record must identify the native test process, its CTest parent
and check child, match the external descriptor target and inheritance flags,
and distinguish cwd/pipe/devnull ownership. A capture during an existing slower
case must be described as such; it is not a before-call capture. Direct green
and CTest red results alone do not establish ownership. Missing or racy captures
remain incomplete attempts. Contradictory or Planar-owned provenance returns to
mapping and plan review rather than entering the fixture-only branch.

## Conditions for the fixture-only branch

The colocated `runner.t.cpp` proposal is smaller than a new execution launcher
and preserves the product boundary. The coder and source reviewer must enforce
all of the following:

- Capture only pre-existing test-process descriptors before creating cwd and
  intentional test subjects. Preserve all original assertions, exact empty
  output and exact `9\n` control. Do not filter reported descriptors or special
  case descriptor 3.
- Restore each original flag and descriptor validity after the fixture, including
  partial setup failure and assertion unwinding. Any enumeration handle owned
  by the helper must be distinguished from pre-existing descriptors; restoration
  must not act on a later reused descriptor number.
- Exercise an intentionally inheritable marker at a dynamically available
  non-3 descriptor. Ensure that the child probe actually observes its range,
  check normalization while isolation is active, and check original flags and
  validity after isolation ends.
- Prove that the intentional macOS subject created after isolation remains
  inheritable immediately before spawn. This makes the late-capture fault in
  I06 observable and prevents a helper from silently masking the
  `CLOEXEC_DEFAULT` fault in I04.
- Run the same final leak test against the Linux pipe and devnull CLOEXEC faults
  and the macOS default-flag fault, with green no-op baselines and actual
  assertion failures. The always-empty probe fault must fail the fd9 positive
  control. Preserve failed setup attempts and masked faults without kill credit.

These are implementation acceptance checks within D03/D05 and I02–I07, not new
runtime behavior. Product files and the 190-product-input identity remain
unchanged in this branch. A proven runtime defect requires a revised scope.

## Measurement, gates and evidence

Any `runner.t.cpp` edit triggers D15: actual normal Debug and RelWithDebInfo
measurements after the last engine edit, complete runtime/loader inventories,
all five resource columns and existing fixture exclusions, exact integer margins
and cited-number reproduction, matching before/after/final source identities,
and regenerated compact bindings. The earlier 272-file identity becomes
historical; current source and actual runs must be bound anew.

D15 does not require an otherwise unnecessary relink when the actual measurement
targets have an unchanged complete compiled-input closure. Retained normal
artifacts may be reused only with a current full-source snapshot/hash binding,
matching compiler and flags, instrumented source/harness, modules, archives and
other transitive inputs, and a valid no-change build/check of those measurement
targets. If that closure or check cannot be established, compile/link anew.
Every normal case and column still executes after the last edit in both
configurations and generates new records. Describe reused artifacts accurately;
neither an earlier table nor a claim of newly compiled binaries may substitute
for those actual runs. The concrete driver and evidence procedure remains
subject to verification.

D27 makes the raised scaling repeat conditional on changed calibration
constants. A carry must identify unchanged constants, sample/assertion bodies,
compiled measurement inputs and compiler/flags against the final C13 scope.
That is a scoped evidence disposition, not permission to carry normal D15 rows
or all older budget faults by assertion. A changed calibration input requires
the applicable actual raised work. The C11 D28 method amendment remains exact;
this review grants no further amendment or measurement waiver.

Both complete host gates remain the default, with pinned formatting/Doxygen and
no-work checks. This review grants no narrower host carry; C12's Python-only
permission does not cover a native test change. The final official Linux Debug
and Release gates must execute final bytes with a frozen unignored context,
nonzero exact inventories, zero skips, every command exit checked and matching
host-before/after/container manifests. The first two 1,842/1,844 failures remain
preserved, and Docker exit 0 is not a CTest result.

The verified archive of owned C11 copied build inputs is a reversible retention
format, not replacement calibration. Its archive hash, complete member inventory
and original-root mapping must remain available for streaming verification or
restoration of earlier hash-bound artifacts; original raw logs/manifests remain
intact. Final independent review must account for the archive when resolving
their original scratch paths.

No installation, publication, push, merge or commit is authorized by this plan
review. R19's real-project exercise remains separate and unselected.
