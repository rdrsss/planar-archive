# R14-D6-C01 independent review

Disposition: **approve** the correction and inherited R14-D6 runtime candidate.
**D6-R1 is closed.** No remaining blocking finding was identified in this scope.
Reviewed on 2026-10-21 in the shared `dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The historical D6 request-changes
review and its evidence remain unchanged. This approval permits the coordinator
to advance to D7 after verifying that this candidate remains current; it is not
public load or full R14 acceptance.

## Finding closure and implementation

[Pure registration](../../../src/engine/runtime/runtime.cpp#L859) retains immutable
original-file views, separator and assembly from this VM's imported compiled
bundle and still seals exact original boundaries/bytes with catalog and handler
bindings. The generated source's backing arrays are static immutable storage;
VM moves preserve their validity. Registration performs no `prepare_stdlib`,
manifest construction or owning acceptance-provenance preparation. Retained view
storage is charged before allocation.

[Fresh accept](../../../src/engine/runtime/runtime.cpp#L1412) checks service order,
bounded converted arguments and exact B/M map/type shape, then cumulatively
charges owning source/assembly/manifest allowance and calls `prepare_stdlib` on
those retained views before invoking the native provider. A refusal poisons the
bridge and prevents provider/completion work. There is no ambient source lookup,
source path recapture, new Lua capability, runtime switch or native budget reset.
D5/provider still owns full B/M correspondence to retained capture. This closes
the defect identified in [D6-R1](R14-D6-review.md#finding-d6-r1--defer-acceptance-provenance-assembly-until-a-fresh-load)
and preserves the approved registration-before-early-lookup ordering. Actual early
request replay remains D7 work.

The new private variant retains all original payloads, separator and assembly,
commands.lua logical identity and callable line ownership. Its sole non-command
original metadata change, `planar/json.lua` to `../json.lua`, makes provenance
preparation deterministically refuse. The contemporaneously frozen pre-fix source
matches the correction baseline runtime; unchanged regression and CMake bytes
produce two red VM-creation failures. Green proves registration plus ordinary init
succeed, while a separate fresh load performs real D5 open/capture and refuses
before accept, with no prepared acceptance and phase `not_completed`. The existing
successful normal/altered callable cases still prove actual same-VM interpretation,
exact original commands.lua bytes and changed provenance digest. The fixture tests
preparation timing without a product test hook.

Inherited D6 source and fresh checks preserve:

- Exactly four production catalog entries and private fixed five-entry admission;
  no shipped load CLI or optional fifth production command. Typed expected-token/
  numeric-head, request and actor checks precede native services.
- Invocation-local opaque identities and exactly three ordered, once-only load
  services, with cross-family, forged, skipped, duplicate and malformed refusal.
  Native failure poisoning/precedence and caught allocation, instruction and
  cumulative native-byte exhaustion remain terminal. Native C++ owners unwind
  before allocating Lua response/error operations can longjmp.
- The shared four-phase native receipt is passed by reference before provider
  acceptance; phase and owned saved bytes survive provider exceptions, conversion,
  handler errors, terminal Lua cleanup and VM moves. Native owner moves/phase
  assignment do not allocate. Success requires committed/replayed plus exact
  equality with retained canonical saved JSON; uncertain completion cannot succeed.
  Provider completion in these tests is explicitly simulated.

Real ledger receipt production, ledger identity recheck, locked/early replay,
preallocated diagnostics, public load parsing and saved-byte stdout transport are
D7 integration obligations; none is inferred from this approval.

## Candidate and preservation

The wrapped repository-relative [candidate](../../../tests/native/r14-d6-c01-evidence/candidate.json)
contains 13 files, SHA256
`2aca1e4f34612574c53e895c2cd5949ca4a9f706b04a9cb8de407f7d5cfa60c2`.
The reconstructed `owned.diff` SHA256 is
`e8bd41c7ce466b0ae1b2d14b958f4633734b76c5e1705448d119b5f794446baf`.
The 67-artifact wrapped evidence-relative manifest SHA256 is
`d4540cf7fcbed5622e4f10389bbd1d01b30a14bedc2375b60fe378eb92dd67b9`;
its paths resolve under `tests/native/r14-d6-c01-evidence/`.

The independent [audit](../../../tests/native/r14-d6-c01-review-evidence/audit.json)
verifies every current candidate file, frozen snapshot, reconstructed diff and
artifact; all inherited D6 candidate snapshots/67 artifacts; and the exact D6
candidate-to-C01 baseline relationship. All 14,143 contemporaneous baseline paths
are preserved outside candidate/coordinator ownership. No unexplained new path
appears outside those and reviewer ownership. The exact contract body retains
SHA256 `a842901180c15691cda24a0ddafcc708dcb88add85717b0080b2a1313e6f4ab8`;
AGENTS.md remains a symlink to CLAUDE.md. Product Lua, storage/codecs, D5, prior
generators, fixtures, goldens and old evidence are preserved.

## Verification

Fresh independent commands used only the exclusively assigned debug directory,
serialized to completion:

1. `cmake --build build/rewrite-debug --target all planar_tests`: exit 0, no work.
2. `ctest --test-dir build/rewrite-debug -R 'replacement\.(command |runtime |load capture )' --output-on-failure --output-junit <review-evidence>/focused.xml`:
   **59/59 passed**, zero failures/skips, including all 11 private load cases and
   seven D5 capture cases. JUnit and LastTest were preserved before inventory.
3. `ctest --test-dir build/rewrite-debug --show-only=json-v1`: 391 names; all 390
   baseline names preserved, only the expected C01 name added.
4. Exact-source audits for all six D6 variants and the C01 variant: passed native,
   interface, original metadata/payload, separator and assembly checks.
5. Independent preservation/evidence audit: passed.
6. Final all/planar_tests build: exit 0, no work.

Raw argv, UTC timing and exits are in [commands](../../../tests/native/r14-d6-c01-review-evidence/commands.json);
[JUnit](../../../tests/native/r14-d6-c01-review-evidence/focused.xml),
[LastTest](../../../tests/native/r14-d6-c01-review-evidence/LastTest.log) and
[summary](../../../tests/native/r14-d6-c01-review-evidence/summary.json) retain results.
The coder's verified broader 134/134 result includes all six CLI suites, zero
skips, lint/Doxygen for 93 active files, vectors 12/20/41 and seven historical
variant families plus fixed runtime. These are reused evidence, not claimed as
fresh reviewer runs. The initial regression compile typo, two red failures and
read-only exploratory audit overlap are preserved; final audits follow testing,
and no build/test overlap occurred. No redundant full milestone was run.

Debug is released idle; release was untouched and remains idle. Scope is macOS
ARM64 debug on the pinned toolchain. No release/Linux, full R14, hardware power
loss, actual completion or public load result is claimed. Reviewer wrote only
this report and `tests/native/r14-d6-c01-review-evidence/`; no implementation fix,
Planar tools/skills/queue, installed binary, operator data, commit, installation,
publication or merge was used.
