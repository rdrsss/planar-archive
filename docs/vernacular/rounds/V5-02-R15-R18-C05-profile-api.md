# C05 profile preparation API correction

C04 review R1 identified an unintended public overload: the optional snapshot
argument made `prepare_load(intent, value_ptr)` callable. The pre-existing
compile-negative guard failed at runtime.t.cpp:1101, recorded in
`tests/native/r15-r18-c05-evidence/fixtures/debug-build-1.log`. The exact public
surface assertion also failed in `historical/history-green.log`; that filename
does not imply that its complete run passed.

The original `prepare_load(const acceptance::request_intent&) &&` is restored.
The distinct `prepare_execution_load(intent, snapshot)` method requires a nonnull
snapshot and delegates to a private common implementation. Null input consumes
the owner and refuses before preparation. Nonnull input follows the existing
structural validation, deep copy and trusted reconciliation in the original
cumulative frame. No factory, frame reset or prepared-VM reentry was introduced.
The command selects this method from the native ledger format flag, preserving
format2 preparation and the C04 frozen snapshot comparison, receipts and replay.

The existing runtime negative assertions are unchanged. With parent authorization,
the acceptance test's exact whole-public-surface string gained only the new named
method declaration; its equality and friend grants remain intact. The original
format3 alias-severing test now invokes the new method, and a new test checks null,
boolean and missing-fields snapshot refusals and consumed-owner retry rejection.

Validation evidence is under `tests/native/r15-r18-c05-evidence/profile-api/`.
The scoped pinned formatter passed (`format.log`). Scoped Doxygen passed
(`doxygen-2.log`); the first invocation failed solely because its temporary config
selected no output format (`doxygen.log`), corrected by enabling XML.
Build and behavioral checks are recorded below when complete. No commit or
self-approval; independent review and full host/Linux gates remain separate.

Completed Release checks:

- `cmake --build build/rewrite-release --target planar_command_tests planar_ledger_acceptance_tests planar_runtime_tests planar planar_x07_variant_3 -j3`
  passed (`build.log`). In particular, runtime/runtime.t.cpp compiled with the
  unchanged negative overload assertion at line1101.
- `planar_command_tests 'C04 profile preparation*,C05 execution profile*'` passed:
  3 cases, 57 assertions, no skips (`preparation.log`).
- `python3 tests/native/r15-r18-c04-profile-cli.py build/rewrite-release/bin/planar --variant build/rewrite-release/src/cmd/planar/planar_x07_variant_3`
  passed: 5 scenarios, zero failures/skips (`profile-cli.log`), covering workflow,
  history, reconciliation, no-op, early replay, late equal/conflicting replay,
  receipts and CAS.
- The first exact-surface attempt selected the wrong binary and matched zero
  (`surface.log`), which is not success. Built `planar_ledger_format_tests -j3`
  (`surface-build.log`) and ran its exact filter
  `'X06 D runtime public surface and resource messages stay closed'`: 1 case,
  53 assertions passed (`surface-2.log`).
- Repeated the build for the five initial targets plus `planar_ledger_format_tests`:
  `ninja: no work to do` (`no-change-build.log`). Source SHA256 values are in
  `source-sha256.txt`.

Release lane2 was released to the calibration agent after these checks. These
are focused correction checks, not a claim that the full gate is complete.
