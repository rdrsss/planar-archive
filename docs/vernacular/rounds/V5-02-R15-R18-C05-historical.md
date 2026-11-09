# C05 historical catalog implementation handoff

Status: implemented and focused verification complete; independent review pending.
No commit, installation, merge or product-limit change performed.

## Policy and implementation

`validate_catalog` keeps strict current 17-command live admission. `decode_catalog`
recognizes only the exact complete shipped name sets for documented feature cuts
5, 10, 14, 16 and 17. A bounded owner-checked prepass selects the historical cut;
the shared validator then applies that cut's required commands, signatures,
capabilities, handler/response pairings and collision namespace. Arbitrary
subsets, hybrids, ownership spoofing and duplicate descriptors remain invalid.
Historical five-command history omits `--event`; ten-command preview omits
`--since`. An external name reserved only in a later cut remains readable in
its original cut. Current admission does not delegate to historical decoding.

Only effective_catalog.cpp, effective_catalog.cppm and effective_catalog.t.cpp
were edited by this package. The interface adds construction friendship and
clarifies policy documentation; public function signatures remain unchanged.
Existing C03 catalog corrections were preserved. Runtime implementation and
immutable historical fixtures were not edited.

## Tests-first evidence

Evidence directory: `tests/native/r15-r18-c05-evidence/historical/`.
Release build directory: `build/rewrite-release`; builds serialized in the
assigned lane. New tests were compiled before the implementation change.

- `red-build.log`: catalog test target built successfully.
- `red.log`: `planar_history_effective_catalog_tests 'C05*'` exited 42;
  2 cases failed, 4 assertions (2 passed, 2 failed). Valid old catalog and old
  external-name ownership were rejected by the former current-only decoder.
- `green-build.log`, `catalog-build-2.log`, `recorded-build.log`: targeted builds
  exited 0.

Final executed focused tests, all exit 0 and no skips:

| Executable / filter | Cases | Assertions | Evidence |
| --- | ---: | ---: | --- |
| planar_history_effective_catalog_tests | 13 | 1747 | catalog-green-2.log |
| planar_history_effective_provenance_tests | 12 | 514 | provenance-green.log |
| planar_history_interpretation_v2_codec_tests | 16 | 50675 | interpretation-green.log |
| planar_history_recorded_interpretation_tests | 18 | 14397 | recorded-green.log |
| planar_ledger_format_tests '[profile_history]' | 8 | 6325 | profile-history-green.log |

New tests independently assemble all documented cuts and exercise per-cut
capability, owner, handler/response/family, ordering, duplicate, unsupported-cut
and signature-hybrid refusal. Existing current omission/capability and resource
bounds coverage remains in the catalog suite.

The additional unfiltered `planar_ledger_format_tests` run exited 42:
104 cases, 103 passed and 1 failed; 35338 assertions, 35337 passed and 1 failed
(`history-green.log`). Its sole failure was the exact public-surface source pin
in `ledger/acceptance.t.cpp:1204`, expecting the pre-C04 `prepare_load` signature;
C04 added the approved execution snapshot argument. This unrelated assertion
was reported to the coordinator and was not changed under this ownership.
The filtered profile-history suite above passed independently.

Pinned clang-format `--dry-run --Werror` on all three changed files exited 0
(`format.log`); `git diff --check` exited 0.

## Integrity and remaining gates

`immutable-fixtures.sha256` records 20 tracked effective-provenance,
interpretation-v2 and R13C2 fixture files, each verified byte-for-byte against
HEAD using `git show`. No expected packet/catalog was regenerated.
`candidate.sha256` records the three source files at handoff; `base-head.txt`
records their base revision. These hashes identify the shared candidate, which
also contains prior C03 corrections.

The Release build lane was released to the coordinator after the last build.
Full integrated Debug/Release/Linux gates, independent review and any follow-up
fixes remain the coordinator's responsibility. This handoff does not claim the
full repository gate passed or that the candidate is approved.
