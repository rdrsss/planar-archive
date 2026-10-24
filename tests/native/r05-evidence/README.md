# R05 native verification evidence

Candidate implementation evidence, not independent approval. All commands ran
serially in `build/rewrite-debug` on macOS arm64 with the configured pinned LLVM.
Linux code paths compile conditionally but have not been executed here.

- `cmake --preset debug`: configured offline committed dependencies.
- `cmake --build build/rewrite-debug --target all planar_tests`: passed.
- `ctest --test-dir build/rewrite-debug -R 'replacement.(loader|capture)' --output-on-failure`:
  15 matched/executed cases, all passed, zero skipped.
- `ctest --test-dir build/rewrite-debug -R '^replacement.cli$' --output-on-failure`:
  one matched/executed CLI acceptance case, passed (22 existing subprocess cases).
- `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`:
  pinned formatter and Doxygen passed on 12 active compilation sources.
- Repeating `cmake --build build/rewrite-debug --target all planar_tests`:
  `ninja: no work to do.`

`gc-regression-before-fix.log` demonstrates the new discarded-constructor test
failing against the initial candidate: storing raw table identities without
retaining the corresponding Lua objects allowed GC address reuse to appear shared.
The candidate now anchors converted tables in a private registry table. Registry
allocation runs in its own `lua_pcall`, so allocation longjmp returns before it
can cross the outer native conversion's C++ temporaries. The final focused log
shows this regression passing together with real shared/cyclic rejection.

Other native tests exercise actual Lua instruction and allocation exhaustion,
100,001 attempted converted nodes, 70-level values, >1 MiB declaration input,
64 MiB native scalar-copy amplification, >1/16 MiB text/source input, five 16 MiB
files exhausting the 64 MiB aggregate quota, 10,001 files exhausting file count,
immutable existing/new key assignment, nested lookup, private tags, unsupported
values, UTF-8/NUL, portable paths/case aliases, symlinks, FIFO, hardlinks, nested
root aliases, and exact reference bytes. Capture changes use a deterministic
pre-verification callback, covering bytes, modes, identity and membership. No
race test relies on sleeps. Standalone registry/observation-style files preserve
logical input.json provenance without a tree capability.

The coordinator authorized the narrow render-contract clarification of a
separate 64 MiB native conversion allocation-accounting ceiling. Fixed portable
charges are documented beside the hard limits. No schema, graph, canonical hash,
trusted Lua projection or successful rendering is claimed by this round.
