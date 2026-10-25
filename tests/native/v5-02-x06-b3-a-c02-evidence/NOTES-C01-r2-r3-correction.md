# Correction to the C01 r2-r3 notes (X06B3AC01-R1)

Scope: this is a new note. The reviewed file
`tests/native/v5-02-x06-b3-a-c01-evidence/r2-r3/NOTES.txt` is left byte-identical.

## The claim that is wrong

The C01 `r2-r3/NOTES.txt` (Limitation bullet: "A double register_vm in both factories
identically would not be caught; double registration in one would") and the former
comment above the test "X06 B3 A factory registration counters are deterministic
across factories" in `src/engine/runtime/runtime.t.cpp` (about line 1237 at C01:
"a repeated registration in one factory would diverge from the other") state that
comparing cumulative `instructions` / `native_bytes` across owners catches a repeated
registration. That is false.

Every `extension_invocation::register_vm` call builds a fresh `command_vm` from the
snapshot. A second registration therefore yields a second, equally fresh VM whose
counters equal those of the first. The factory keeps only the VM it returns, so
equal counters are produced with one registration or with two. Counter equality only
shows deterministic cumulative work; it cannot detect a second `register_vm`. The
reviewer's probe confirmed this: all three double-registration mutants (opaque only,
known only, both) survived `[x06b3a]` at C01 (294 assertions / 16 cases passed; see
`tests/native/v5-02-x06-b3-a-c01-review-evidence/probes/run-opaque.log`, `run-known.log`,
`run-both.log` and `INDEX.txt`).

## What detects it

C02 counts registrations at the registration site:
`profile_capture::note_registration()` is called inside `register_vm` and is exposed as
`profile_invocation_observation::registrations`. The tests assert
`registrations == 1` after each factory (`open_opaque`, `open_known_load`) and
unchanged across `into_extension`. This is the detector; counter equality is not.

## Evidence that the observable detects it

The three red logs, run on the C02 candidate with `mutate.py` in a scratch worktree
outside the shared checkout, are in this directory:

- `probes/run-opaque.log`
- `probes/run-known.log`
- `probes/run-both.log`

Each fails the `registrations == 1` CHECKs (for example `2 == 1`); the mutated
`runtime.cpp` hashes and the restored-hash equality are in `probes/INDEX.txt`.
The corrected comment now sits in `runtime.t.cpp` above the same test case.
