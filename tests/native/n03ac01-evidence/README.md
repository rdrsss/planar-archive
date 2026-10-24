# N03-A-C01 strict Glaze validation correction

Shared checkout correction to the independently reviewed N03-A candidate. Only
`src/engine/json_codec/codec.cpp` and `codec.t.cpp` changed; this directory owns
the correction evidence. Prior N03-A and reviewer evidence remain untouched.
No vendor patch, handwritten grammar, API change, legacy/installed executable,
Planar tool, commit, installation, publication or delegation was used.

The raw-byte ceiling still runs first. The existing bounded structural Glaze
read retains duplicate keys and enforces 128 containers / 100,000 values before
the new `glz::validate_json` pass sees the exact same input bytes. Validation
uses Glaze's strict grammar without allocating another structural tree, and
conversion only occurs after it succeeds. Both library recursion-exhaustion
paths defensively map to `resource_limit/json_depth` with the effective decoder
ceiling. Strict validation errors use Glaze's zero-based byte count. Structural
read errors and schema/resource refusals retain precedence for already invalid
inputs; malformed input containing a disallowed value can therefore produce
`schema` before the additional syntax pass runs. Existing safe fixed diagnostic
text and public API are unchanged.

## Regression and verification

Commands and raw outputs are paired as `*-command.json` and `*.log`, and all
builds were serialized in `build/rewrite-debug`.

- `red-test.log` preserves the first actual failing run: 1 test / 1 failed
  assertion, accepted trailing object comma. The revised nonfatal regression
  then preserves all six failures in `red-all-test.log`: trailing and missing
  object commas at the top level, inside an object, and inside an array.
- `final-build.log`: `cmake --build build/rewrite-debug --target all
  planar_tests` passes, including affected products and all test executables.
- `final-tests.log`: codec filter executes **8 cases / 115 assertions**, all
  pass, zero skips. Regressions assert exact syntax offsets; valid whitespace,
  decoded duplicate keys, oversized malformed raw input, exact 128/129 depth
  and 100,000/100,001 node boundaries also pass. Original encoder bounds remain
  included. The first post-fix run is separately retained as `green-tests.log`.
- `review-probe.log`: original independent review probe rebuilt against the
  corrected codec, **243 checks / 0 failures**. It is an existing reviewer
  probe, not a new independent review. The executable was removed after running;
  its hash and exact reproducible compiler command remain here.
- `lint.log` retains a formatting failure in the new test table. Pinned
  clang-format corrected only the owned files; `final-lint.log` passes pinned
  formatting and Doxygen across **28 active source files**.
- `no-change.log`: repeated `all planar_tests` build reports
  `ninja: no work to do.`
- `registered-tests.json`: **121 registered cases**, not an execution claim.
  Existing product tests, complete product gates, release and Linux gates were
  not rerun in this correction; integration verification belongs to N03-B.

`candidate-sha256.txt` freezes the same six-file N03-A product candidate with
the two corrected files; other four entries are unchanged. `before-*.txt`
preserve the previous exact sources and `candidate-*.txt` preserve corrected
sources. `correction.diff` is the complete correction against those snapshots.
`evidence-sha256.txt` covers all evidence files except itself, independently of
the product candidate manifest. This report records implementation evidence and
does not approve the candidate.
