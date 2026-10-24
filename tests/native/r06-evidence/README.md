# R06 coder verification

Candidate is for independent review; no self-approval, commit, installation or
publication is claimed. Shared checkout preserves earlier R01–R05 changes.

Implemented native APIs:

- `planar.kernel.canonical`: `canonical_encode(value_ptr, canonical_limits)`
  returns exact binary canonical_v1 bytes or `diagnostic` through `std::expected`.
- `planar.kernel.hash`: `hash_value(domain, value_ptr)` implements the exact H
  envelope and seed-zero/canonical XXH3 representation; `file_digest(bytes)` and
  `fragment_digest(bytes)` select their reviewed primitive domains.

The future trusted Lua bridge can call those APIs after producing normalized
values. No new Lua capabilities or schema fields are exposed in this round.
Private constructor tags are refused. R07 owns normalization, graph resolution,
selection and material work records; R08 owns actual packet/manifest production,
canonical JSON transport and CLI integration. Identity recipes are unchanged.
Selected base/observation/unit/role records exist only in native tests/oracle.

Encoder resource ceilings are depth 256, four million visited values and 256 MiB
encoded output. Optional caller settings are clamped to those maxima. Object-key
text scanning is also bounded by the byte ceiling, and sorting allocation by
the node ceiling. Repeated DAG references count at every occurrence; cycles fail.
Hash helper input-size checks precede copying. The H envelope counts toward
encoder limits. Source bytes have no declaration-string cap. Maximum capture is
64 MiB and 10,000 files; its hexadecimal inventory is 128 MiB plus bounded
metadata, comfortably under these native ceilings. Maximum single source files
(16 MiB) are tested. These limits do not narrow the supported captured inventory;
trusted recipe selection must continue to respect bounded resource failures.
No work-materiality or output-boundary selection is implemented natively.

Executed on macOS Apple silicon with the pinned LLVM 23.1.0/libc++ and CMake
4.4.2 debug preset, serially in `build/rewrite-debug`:

```sh
cmake --preset debug
cmake --build build/rewrite-debug --target all planar_tests
ctest --test-dir build/rewrite-debug -R 'replacement.(canonical|hash)' --repeat until-fail:2 --output-on-failure
python3 tests/vectors/generate.py --check
make cpp-lint
cmake --build build/rewrite-debug --target all planar_tests
```

Final result: product/native test build succeeds. Seven selected Catch2 tests
match, each passes in two fresh processes (14 successful executions), with zero
skips. They compare against literal vectors every time; fresh processes cannot
silently agree with one another on an incorrect digest. Oracle replay verifies
12 hand-encoded byte examples, 20 published upstream hash known answers, and 41
canonical/domain vectors. Active clang-format/Doxygen passes over 18 sources.
The subsequent unchanged build reports `ninja: no work to do.` Logs are adjacent.

The initial compilation attempt failed because tests compared two
`std::expected` instances whose diagnostic type has no equality operator. The
comparisons were corrected to compare their valid values. Raw initial compiler
output is retained as `build-initial-failure.log`; subsequent `build.log` is the
successful final candidate build. No product regression was being fixed, so
there is no claimed pre-fix behavior regression test.

Vendoring provenance and independent oracle details are in
`tests/vectors/README.md`. The downloaded upstream archive matched the approved
SHA256. Only unmodified `xxhash.c`, `xxhash.h`, and BSD-2-Clause `LICENSE` were
copied. Runtime, configure, test and vector replay need no network.

Limitations: focused debug-host verification only; Linux/release and integrated
render gates belong to later rounds. Rendering is still honestly unimplemented.
No full repository gate, legacy binary, Planar tool or host queue was invoked.
`candidate-files.txt` lists the coder-owned delivery and `candidate-sha256.txt`
binds those files (excluding the two manifest files themselves) for review.
