# C03 independent vector workflow verification

This verification author changed only the standalone regression script and this
report/evidence. No product source, builds, installed binaries or operator data
were used. The shared checkout and other agents' changes were preserved.

`tests/native/r15-r18-c03-vectors-cli.py` accepts the source-built binary as its
required positional argument. It runs three disposable-ledger scenarios:

1. Grouped worker owns `a`, `b`, `c`; `b` integrates internal producer `a`, while
   `c` integrates external producer `z`. External `s` is a start-only facet
   dependency of `b`. Producers `s` and `z` complete first.
2. The same declarations with separate workers for `a`, `b` and `c`.
3. The grouped lifecycle entirely through `with-profile` and an explicitly
   supplied empty external profile, including init, load, execution and review.

The registry runs actual POSIX arithmetic assertions over the declared files,
then writes the check-result document. Each native scenario checks ten evidence
vectors exactly: own-only checks and boundary rows remain singletons, `b` is
`[a,b]`, and `c` is `[c,z]`. This covers grouped internal producers, unrelated
producer exclusion and start-only facet exclusion. Observation membership is
checked separately. Read-only SQLite queries and a small test-only decoder of
the documented canonical format inspect retained vectors; public history does
not expose those vector records directly. No rows are changed.

Each native scenario also verifies three nonempty facet payloads through the
retained tree edges and exact file bytes/modes, completes all five workers,
claims independent review, submits approval, verifies all five units accepted,
and checks byte-identical check and verdict replay. The fixture's base and
current directories contain the same initial files, so unrelated unchanged files
do not violate boundary ownership. Every subprocess has a 30-second timeout.
Tokens remain in memory and are absent from logs.

## Results

Evidence directory: `tests/native/r15-r18-c03-evidence/vectors/`.

- `first.log`: initial two-scenario script on Debug binary
  `cc560a22e93e8cb56b28a41b9f158140674eb58b2e97d00fea58b8a3b1af12fb`.
  Both scenarios failed the first facet-producing check with exit 4, invalid
  embedded command protocol. This independently reproduced the nonempty facet
  publication failure being investigated by the C03 coder; it did not reach
  vector assertions and is not red proof of vector membership by itself.
- `release-before-facet.log`: expanded three-scenario script on Release binary
  `929792330ce96c0c750845dc4bbbaab5a3db576ca8fe0f7af816ca1b19101f58`.
  Native scenarios still failed the then-unfixed facet publication. The selected
  profile initialized its ledger, then load failed with exit 3,
  `unsupported_ledger_version`.
- `debug-facet-fixed.log` and final script run `final-current.log`: Debug binary
  `e7d9ed31fe343374e3cc55d6efc0121e126bf51c53dbba521e2966af7ef79db1`.
  **Grouped and split pass**, with 10 exact vector assertions and five accepted
  units each, including the retained nonempty facet bytes. **Profile fails at
  load** with the same unsupported-ledger error. Total: **3 scenarios, 1 failure,
  0 skips; script exit 1**. The complete script is not green.

Commands used the explicit positional binary paths
`build/rewrite-debug/bin/planar` and `build/rewrite-release/bin/planar` from the
repository root. `python3 -m py_compile` passed during script development.
The final script/raw evidence hashes are generated in `manifest.json`.

## Profile blocker

`src/cmd/planar/invocation.cpp`'s profile-load path explicitly rejects format3
before request lookup (around line 1861), whereas public init now creates
format3. This is an incomplete profile cutover, not a fixture assuming an
unauthorized feature: `execution-contract.md:497` explicitly retains the
profile argument and effective-stdlib path of request_v2 on format3, and
`:728–730` requires profile execution to use the accepting revision's exact
trusted-code digest. The parent and catalog coder were notified. A separate
corrective package must repair it; this verification author made no product
fix. The coordinator will rerun all three scenarios and register the script in
CTest on the corrected candidate. No full gate or approval is claimed here.
