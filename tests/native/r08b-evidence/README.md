# R08-B coder evidence

Shared checkout `dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. No commits, Planar tools,
installation, publication, delegation or changes to approved Lua/capture/loader/
runtime/value/canonical/hash implementations. Coordinator owns round records.

`candidate.sha256` freezes the complete contents of the 25 owned candidate files.
Its SHA256 is `c8171f623a8426e377ffa33e8e03fa62a4eb32306c63fd3343142e1ff28f4889`.
This is an uncommitted candidate, not self-approval. Existing tracked diffs and
untracked earlier rewrite files were present on entry. `prior-approved-check.log`
shows all 20 R08-A-C01 manifest entries still match. That check complements rather
than replaces review of the new command and JSON module wiring.

## Implementation and verification

The command loads the explicit workspace, routes every declaration and registered
text reference through one capture session, retains registry/optional observation
inputs, registers the required current root even without scanning, optionally
captures both source trees with exclusions, seals once, and passes the raw bundle,
retained files and topology to the trusted `render` export. No additional native
schema policy or normalize/parse round-trip was introduced. Native diagnostics
map to canonical buffered errors with safe field/details and the prescribed exit
classes. Host filesystem exceptions are sanitized.

The new generic `planar.kernel.json` serializer has independent byte/depth/node
bounds, sorted unsigned-byte keys, signed decimal integers, all C0 lower-case
Unicode escapes, literal UTF-8/slashes, and invalid-type/text/cycle refusal.
Complete success payloads are bounded to 16 MiB before stdout; LF is appended
outside that bound. Main uses unbuffered POSIX writes, ignores SIGPIPE, retries
EINTR and refuses short/failed writes. There is no deferred stream-buffer flush.

Executed directly, serially per build directory:

- `cmake --build --preset debug --target all planar_tests`: success in
  `build-final.log` and `build-extra.log` (the latter builds the final JSON tests).
  Earlier `build-initial.log` preserves a missing help initializer compile failure;
  `build-second.log` resolves it. No approved lower-level module was edited.
- `python3 tests/acceptance/bootstrap_cli.py build/rewrite-debug/bin/planar`:
  **71 real CLI processes, six native goldens, zero skips**, `cli-frozen.log`.
  Six envelopes and Markdown were originally generated with the explicit
  `--write-goldens` option; final acceptance only compares them. The fixture's
  native README documents presentation differences and exact source coverage.
- `build/rewrite-debug/src/kernel/planar_json_tests --reporter junit --out tests/native/r08b-evidence/json.xml`:
  **4 cases / 31 assertions**, zero failures/errors/skips. `json.log` is empty
  because the reporter writes to XML.
- `build/rewrite-debug/src/cmd/planar/planar_command_tests --reporter junit --out tests/native/r08b-evidence/command.xml`:
  **3 cases / 11 assertions**, zero failures/errors/skips; `command.log` is empty.
- `ctest --test-dir build/rewrite-debug -R 'replacement.(json|help|missing explicit|legacy commands|cli|render|runtime|schema|semantic|selection)' --output-on-failure --output-junit <repo>/tests/native/r08b-evidence/ctest-frozen.xml`:
  **83 matched tests** (82 unit / 1 acceptance), zero failures/skips,
  `ctest-frozen.log` and XML. `counts.json` records parsed counts; Catch's JUnit
  suite `tests` field counts assertions, unlike CTest's testcase records.
- `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`:
  formatter and Doxygen pass for **25 active sources**, `lint-final.log`.
- `cmake --build --preset debug --target all planar_tests`:
  **ninja: no work to do**, `no-change-build.log`.
- `git diff --check`: success, `diff-check-final.log`.
- `shasum -a 256 -c tests/native/r08ac01-evidence/candidate.sha256`:
  all entries match, `prior-approved-check.log`.

Earlier CLI logs remain: the first test expected the manifest provenance digest
key to be `content` instead of `digest`; another expected a valid worker rename
to be invalid; another attempted a same-root case alias on a case-insensitive
filesystem. These were test expectation/setup errors, not claimed product bug
regressions. The final tests use an unknown work reference and cross-root aliases.
The remaining earlier green logs record progressive coverage; final evidence is
`cli-frozen.log` and `ctest-frozen.*`.

## Coverage and limits

CLI tests cover explicit help/usage, canonical JSON and sanitized diagnostic
code/field/details, malformed registry/observations/Lua, unsupported versions,
resource input-size errors, forbidden declaration capabilities, unselected-job
validation and globally invalid observations before scoping, stale/absent targets,
wrong check/vector, checkpoint scope/size, budget boundaries/omissions, logical
root mismatch and current/base overlap, no-capture/empty/deleted/binary/upstream
files, fence/C0/C1 injection, exclusions, unowned captured content, symlink/
hardlink/case aliases/special files, broken pipe/closed stdout/closed stderr and
forced short output through a nonblocking pipe. An unavailable configured
executable still renders as a descriptor. Fresh-process determinism, host-path
relocation, input/state preservation and hostile HOME/config/database sentinels
are checked. No code/check command is run by the replacement.

The independent Python encoder plus scratch upstream XXH3 library recomputes
file, W, contract, base, observation, unit output, role output and fragment hashes;
canonical JSON and included section byte counts/joins are independently checked.
Each of six packets is compared with original R03 goal/requirements/acceptance/
contract/source prose and ownership paths, and exact check/toolchain descriptors.

The exact 16 MiB success payload and one-byte overflow are **serializer unit
boundaries**, not an end-to-end CLI output-limit experiment. The real CLI
backpressure case adds a 1 MiB unowned file to force a partial nonblocking pipe
write and exit 4; it does not establish the 16 MiB refusal. Production CLI wiring
uses the serializer's default fixed 16 MiB bound before any output. Capture race
seams remain covered by prior native capture work; this round introduces no new
filesystem race hook or kernel change. Verification is native debug macOS;
Linux/release and the full `make test-all` integrated gate remain R09. Operator
packet evaluation remains R10.
