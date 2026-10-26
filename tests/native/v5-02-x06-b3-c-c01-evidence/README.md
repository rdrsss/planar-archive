# V5-02-X06-B3-C-C01 evidence

Final candidate: `git diff HEAD -- src` sha256 `ea5f45376e9cd9473a57060f99a1b7adc41c38ddb165265693ba30a6c9c5daae`;
`src/engine/runtime/runtime.t.cpp` sha256 `8fedecbc2874f3280dbac17c8c6609a8e0b4e668fb5b9ec7f4b621834416433e`.
The round record is `docs/vernacular/rounds/V5-02-X06-B3-C-C01.md`.

- `REPORT.md`: gate report, second run (1003 tests, PASS). `raw/*-r2.*` are its logs.
- `raw/REPORT-r1-superseded.md` and the logs without the `-r2` suffix: first gate run (1002 tests, before the repair test); kept.
- `inventory.txt`: the 1003 sorted test names.
- `probes/PROBES.md`: mutation probes on the final candidate (83 ids; P01 survives the new test, classified masked). `probes/raw/` has per-mutant logs and measurements (`MEAS-*`), `probes/scripts/` the driver.
- `probes/superseded-1/`: the earlier probe run against the candidate before the repair test; kept unchanged.
- `raw/post-c01-hashes.txt`, `raw/post-c01-src.diff`: intermediate post-P1 hashes and `src` diff (`e5dfafab...`, superseded by the final candidate above); kept as evidence.
