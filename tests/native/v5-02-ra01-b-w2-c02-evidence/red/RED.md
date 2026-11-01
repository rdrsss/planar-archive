# RED check V5-02-RA01-B-W2-C02
Red-time identity (covers tracked and untracked bytes): HEAD 3343ad38304db57388bc7ac2438a40043eba1b7d;
candidate.diff sha256 c2a9be9914d91d735087a4683930319d5daeeadc588e0fa64ce8f2f8ea04ea4a;
SHA256SUMS sha256 b48d790bfdc579e84c64cdd84eaaa6976a93b81ad777a772ea5b5cd97f8fb7c8 (snapshot/).
Build: cmake --build build/rewrite-debug --target planar_tests, exit 0 (raw/build.log).
Command: ctest --test-dir build/rewrite-debug -R '^replacement\.<escaped name>$' --no-tests=error --output-on-failure (raw/testN.log; the first attempt without the replacement. prefix found no tests and was overwritten by the corrected run).

| # | test | invariants | expectRed | outcome | failing line |
|---|------|-----------|-----------|---------|--------------|
| 1 | capture_diagnostic post-gate failures are remapped at every native call | C02-01,03 (red); 35,05 (guard) | true/false | failed-assertion | capture.t.cpp:1097, :1100 |
| 2 | observe digest allocation failure is a bound refusal | C02-02,04,21 (red); 36 (guard) | true/false | failed-assertion | authoring_observe.t.cpp:785, :789 |
| 3 | capture_diagnostic maps a post-gate add_input failure to file changed | C02-06 | false | passed | |
| 4 | observe maps a failed entry inspection after listing to cannot inspect | C02-09 | false | passed | |
| 5 | X08 C01 role preparation outputs-off boundary pins the request-record conversion charge | C02-15 | false | passed | |
Other invariants have no test (grep/diff/gate checked).

Deviation (review R3): the first red-first attempt matched zero tests (missing `replacement.` prefix) and its raw logs were overwritten by the corrected run, which breaks the never-delete-raw-logs rule. The behavioural red-first evidence of the corrected run is intact.
