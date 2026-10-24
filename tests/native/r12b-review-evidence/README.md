# R12-B independent reviewer evidence

Disposition: request-changes; see `docs/vernacular/rounds/R12-B-review.md`.
Candidate and original evidence are unchanged. Reviewer owns only this directory
and that report. Every database probe uses explicit temporary paths, never an
installed executable or operator database.

`focused.xml`/`focused.log`/`focused-counts.json`: 26 passed, zero failed/skipped.
`build.log`: no work to do. `compile.log`/`link.log`: exact native probe commands.
`probe.cpp`, `probe.o`, `probe`: native explicit-path initializer/opener/snapshot
probe against frozen debug candidate modules and libraries.

`adversarial.py`/`adversarial.json`/`adversarial.log`: 24 independent checks;
15 pass and 9 RED destination-sidecar cases. Successful init must refuse an
orphan -journal/-wal/-shm entry of file/link/directory type without publication.
`sidecar-publication.json` preserves the original smaller reproduction.

The same script records an embedded-NUL identity with SQLite character length 32
but 33,554,465 bytes. Native refusal follows the complete metadata copy;
`nul-metadata-memory.log` and `baseline-memory.log` preserve memory observations.
This is a missing pre-copy byte guard, not a measured out-of-memory crash.

`identity-before.json`/`identity-after.json` verify all 7 candidate and 24 original
evidence entries. `primitive-identity.json` verifies approved primitive source
and tests against the protected baseline. `manifest.json` freezes this directory
and the review report, excluding itself. No candidate fixes are included.
