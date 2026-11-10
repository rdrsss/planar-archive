# R19-CMAKE no-work adapter repair

Status: **applied after explicit coordinator release; parser regression checks passed**.
The next genuine combined Planar check and independent final approval remain pending.
This is a bounded verification-tool repair after the first actual Debug gate.
It is not a product or CMake correction, a passing combined check, or approval.
The prescribed Claude model is unavailable; a fresh ordinary repair agent
performed this work instead. The checkout is shared and other edits are retained.

## Actual failure and authority

The Debug attempt `checks/debug/1794266799601479000-58896/` completed configure,
build, equivalence, inventory, all 1,844 CTest cases, lint and repeat-build commands
with exit 0. Its actual JUnit accounting reports 1,844 passed and zero failed.
The adapter nevertheless exited 1 because it compared the complete repeat-build
stdout to the single idle line. The saved actual stdout was:

```text
[0/2] Re-checking globbed directories...
ninja: no work to do.
```

The original approved C14 Debug gate retains exactly those two final lines, and
the pre-split baseline Ninja already contains `VERIFY_GLOBS` maintenance. The
current official gate in `scripts/rewrite-check.py` accepts the idle phrase in
this output. The new task adapter's stricter complete-output comparison caused
a false failure; this prefix was not introduced by directory ownership.
`no-work-repair/RED-EVIDENCE.json` and `BASELINE-CORROBORATION.json` bind the actual
failed attempt, historical output and pre-split Ninja. Neither record is relabeled
as a successful task check.

## Draft and scope

The ignored `candidate-check.py` adds `is_no_work_output()` and calls it at the
existing repeat-build boundary. It accepts only the plain Ninja idle line or
one exact known glob-verification line followed by the idle line. Either form
may have one terminal LF. It refuses compilation, linking, CMake regeneration,
unknown diagnostics, duplicated probe lines, additional lines and unexplained
whitespace. There is no broad substring or arbitrary progress-line exception.

All command execution, exit handling, CTest accounting, source freezing,
equivalence, lint and result-file behavior remain unchanged. The existing error
is retained for any other output. No engine CMake, C++, Lua,
ledger, installed binary or operator database was modified or executed.

During the initial draft checkpoint, only ignored drafts, evidence, a candidate
regression script and this report were written. The actual `check.py` remained
frozen until the original combined Planar check finished its required Release
attempt and the coordinator explicitly released the live file. The original
report bytes are preserved as `no-work-repair/REPORT-DRAFT.md`.

## Executed verification and limitations

The actual saved failing gate is the red regression; it already demonstrates
the old recognizer refusing this real idle build output after all seven commands
exited successfully. A retained unit invocation against the ignored candidate
passes three unittest cases with 19 protocol input vectors: four accepted idle
forms and 15 refusals, with no failures or skips. The regression script also ran
once successfully before the retained invocation. No native configure, build,
CTest, lint or Planar operation was run by the repair coder.

The ignored patch, candidate, tests, command, stdout/stderr, exit, source hashes
and evidence bindings remain under `no-work-repair/`. These parser checks do not
replace the coordinator's subsequent genuine check request and independent final
review. Original failed evidence remains intact. Existing verifier probes retain
their unchanged `verify.py` scope and do not certify this new recognizer.

## Applied outcome

After the original combined check completed with its correctly retained failed
disposition, the coordinator explicitly released application. The exact prepared
candidate was copied to `tests/native/r19-cmake-support/check.py`, and its scoped
regression script was added as `check_test.py`. The original adapter remains
byte-identical in `no-work-repair/check-before.py`; the original draft manifest
and all native gate records remain unchanged.

The actual live module passed the same three unittest cases and 19 vectors,
zero failures or skips. Its recognizer also accepted both exact saved Debug and
Release repeat outputs. Both original attempts still report adapter exit 1,
with all seven underlying commands at exit 0 and 1,844 actual cases passed.
These observations do not retroactively turn either original task check green.

All 287 first-party source files match the unchanged candidate source index,
including all 16 CMake files, and `verify.py` retains its original hash. Existing
verifier probes carry forward only through that unchanged component; their old
whole-input snapshot still names the earlier `check.py` hash. The new recognizer
is covered by its own focused regression inputs and subsequent actual gate.
`APPLIED-PROOF.json` and `APPLIED-MANIFEST.json` bind the applied candidate, live
tests, original backup and historical-report relocation. No native build, CTest,
lint or Planar command was executed by this repair coder.
