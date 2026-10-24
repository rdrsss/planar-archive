# R14-C3-A independent review evidence

Verdict: `docs/vernacular/rounds/R14-C3-A-review.md`.

`check.py` independently validates the eight live/frozen candidate hashes and
composite identity, coder evidence-manifest file hash and every raw member,
12,326 protected hashes and scope, exact approved C2 capture/reference baseline,
exact mechanically reconstructed native implementation/interface, byte-identical
facade load, both generated variants, actual native-only archive link and raw
coder counts/exits/skips. It does not execute or rewrite coder evidence scripts.
It excludes only its own evidence folder in addition to the coder's documented
scope exclusions. `check-initial.txt` and `check-final.txt` retain both runs.

`run.py` serially builds all plus tests, independently runs capture25 and CLI247,
compiles/links/runs the exact-library `probe.cpp`, checks two expected negative
compilations, runs active formatting/Doxygen and checks a zero-work rebuild.
`commands.json` records the exact argv, cwd, actual exit and expected outcome.
`probe.cpp` imports both modules simultaneously and compares full result/error
transcripts (including diagnostic code/message/limit/maximum), immutable file
bytes/modes and topology over fifteen native/facade scenario pairs. It checks
seal/transfer refusals, repeated verification, byte/directory changes, callback
lifetime and exception behavior, exclusions/normalized modes, portable paths,
aliases, standalone registry and parent replacement, overlap, symlinks/FIFO,
empty captures, cached reads, text/binary bounds, move/self-move and API traits.
185 runtime assertions and compile-time API checks pass. Negative compile logs
specifically identify the private receipt constructor and private native state.

The probe uses current configured module maps and exact candidate archives;
`probe-artifacts.json` freezes archive/executable/object hashes. Its executable
and object were removed after passing. Verification-only sources remain here.
Existing native test link independently excludes loader/Lua/runtime/history/ledger.

The unchanged 151 history-focused tests, vector checks and four-helper lint were
validated from frozen raw coder evidence rather than rerun: product sources,
old tests, generators and helpers are hash-identical, and exact extraction plus
capture/CLI and parity checks focus on the changed boundary. No release/Linux
or complete R14 milestone validation is claimed.

Run `python3 tests/native/r14c3a-review-evidence/check.py`, then `run.py`, then
`check.py` again, directing audit stdout to new evidence files as appropriate.
`evidence.json` freezes all retained reviewer members except itself and records
the report SHA256, exact candidate identity and coder manifest file SHA256.
