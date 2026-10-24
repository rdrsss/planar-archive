# R14-A1 independent review evidence

Fresh reviewer `/root/r14a1_reviewer` used the shared checkout at
`<repo>`, `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Product/candidate and prior evidence
were read-only. Writes are confined to this directory and the review report.
No Planar tools/skills/queue, operator data, installed binary, subagents, commits,
installation, publication or merge. Debug commands were exclusive and serial;
release was not used. Debug is released on report delivery.

`commands.json` records direct build, focused CTest, raw CLI, vectors, lint,
registry, final no-work and audit commands with cwd/exit status. `probe-commands.json`
records exact standalone compile/link/run commands. `run.py`, `probe.py`,
`audit.py`, and `finish.py` reproduce checks without writing coder/prior evidence.
All 113 focused cases pass, with all 109 prior names plus four core cases and zero
skips. CLI passes 247 processes, including 60 host-path checks and six goldens.
Both vectors, 54-source formatting/Doxygen, two-file helper lint and no-work pass.
The entire registry has 230 entries (229 native and one Python); only the agreed
focused tests and standalone probes were executed here, not a full milestone gate.

`core-probe.cpp` imports only the pure core and runs three cases/541 assertions:
all 14 exact ledger errors, long embedded-NUL messages, failures after different
numbers of pending records, synchronous callback lifetime/no retention,
reentrant digest calls, output lifetime, and original read/build exceptions.
`abort-alias-probe.cpp` retains the earlier 46-assertion allocation/abort probe and
adds four compile-time alias type-identity checks. Its one case passes. Object and
binary hashes are captured in `probe-artifacts.json`; temporary executables and
objects were removed. `core-link.txt` independently records the pure link graph.

Initial/final audits verify the ten-file live/snapshot candidate identity,
sixty coder evidence entries, 4,081 local protected files, 11,867 inherited files,
exact reconstructed diff against approved historical snapshots, complete moved
algorithm bodies and exact fresh reversible fault/collision generation. All paths
in candidate/protection maps are repository-relative. Reviewer `evidence.json`
uses paths relative to this directory and excludes itself.

One nonblocking editorial finding is recorded in the report: docs/testing.md's
current summary remains 226 although registry.json contains 230 after the four
new core cases. Product candidate was not changed. Checks are macOS/arm64 debug;
release, Linux and full integrated R14 gate remain unverified in this round.
