# R19 CMake verification tooling

The available ordinary model substitutes for the unavailable prescribed Claude
model. This is a tooling implementation report, not independent approval. The
checkout is shared; only the two assigned support scripts, this report and the
ignored verification evidence directory were changed. No build, configure,
CTest execution, candidate-source mutation probe, installed binary or operator
database was invoked by this coder.

## Executable tools

`tests/native/r19-cmake-support/verify.py` takes `--repo`, `--evidence`,
`--preset debug|release` and `--output`. It performs read-only comparisons of
the actual configured/built candidate against the retained baseline File API,
Ninja, generated-file and CTest records. It checks all target names/types,
per-source compile/module membership, definitions/options/includes, resolved
direct/transitive target edges, link/archive fragments, artifact paths, complete
registered test properties/arguments, normal-all and planar_tests closures,
configure dependencies, sixteen directory CMake files and immediate-parent
reachability. All original non-CMake source names and hashes must remain exact.

The relocation policy is derived from baseline ownership plus the reviewed map,
not candidate differences. It is pinned before the first candidate comparison in
`verification/{debug,release}-OWNER-ALLOWLIST.json`. Every candidate target must
be created in its exact reviewed owner. Artifact/source translations are specific
to each baseline output and all 155 baseline generated files; generated-directory
translations apply to known module bases and fixture path definitions, and the
shared-library rpath has one exact allowed relocation. Catch2 working-directory
rebasing is restricted to the test command's exact owning target. All generated
files remain inventoried and compared byte-for-byte after only those explicit
path-string substitutions. No generated source or path-bearing definition is
broadly omitted. Unknown owner/path/payload changes fail.

`check.py` is the trusted real-check adapter with `--repo`, `--evidence` and
`--preset`. It resolves CMake/CTest through the explicitly supplied registry
PATH, configures the preset, builds `all planar_tests` with four jobs, executes
equivalence validation, then all registered CTest cases into retained JUnit,
pinned formatting/Doxygen, and a strict actual no-work repeat. CTest stays
serial. The result counts come from executed JUnit cases and must match the
nonempty unique actual inventory, with no failures, disabled tests or skips.
Source bytes/names are frozen before and after the complete check.

Each actual check creates an exclusive timestamp/PID attempt directory under
`checks/<preset>/` and keeps command argv/exits/durations, stdout/stderr, JUnit,
source snapshots and the result manifest. It writes the strict result document
to `PLANAR_CHECK_RESULT` when provided, returns nonzero on any gate failure and
prints only a compact real result summary. Missing/malformed execution reports
never become successful counts. Earlier attempts are never overwritten.

A clean separate configuration can be run by the coordinator with
`cmake --preset debug -B <fresh-build-directory>` without a full duplicate build;
this is separate from the two actual host-gate adapter calls. No claim of a fresh
configure or candidate-equivalence result is made here before execution.

## Executed tooling checks

Python compiled both script bodies successfully without bytecode output.
Read-only baseline inspection resolved 442 targets, 200 engine-owned targets,
155 generated paths and 1,086 explicit path translations in each configuration.
The retained baseline Ninja closures resolve 95 normal-all target names and
441 planar_tests target names. Every retained generated baseline file's current
bytes matches its recorded hash in both configurations. Results and allowlist
hashes are retained in `verification/TOOL-READINESS.json`.

No synthetic candidate graph mutation, source mutant, runtime test, build or
configure ran as part of tooling development. Proposed invariants and any later
actual probe evidence must remain distinguished. Actual candidate checks and
independent review remain the coordinator's next step.

## First actual coordinator execution

The coordinator ran the frozen scripts through the real Planar check adapter.
Its first Debug configure and native build succeeded; the executed equivalence
result reports 442 targets, 200 engine owners, 1,844 actual registered tests,
155 generated payloads and no errors. Target/test semantic differences are empty,
and both default-all/planar_tests closures and configure-dependency checks pass.
All 155 generated payloads retain exact raw bytes; no payload normalization was
needed for this observed candidate, although its explicit allowed path policy
remains recorded. This is inspection of the coordinator's actual saved result,
not a separate execution by this tooling coder.

The real full CTest gate was still running at this checkpoint. Release and final
independent review are pending; no complete-gate or final approval is claimed.
The current equivalence implementation reconstructs its large literal path regex
for each string leaf. The baseline has 46,279 target-data and 16,711 test-data
string leaves, so each pass has avoidable multi-minute overhead; the successful
executed tool remains unchanged rather than introducing a mid-gate alteration.
