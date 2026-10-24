# Independent N03-A-C01 review evidence

Only this directory and `docs/vernacular/rounds/N03-A-C01-review.md` were written
by this reviewer. Product candidate, coder evidence, and prior review evidence
were read-only; no other builder ran. Commands use argument arrays and explicit
working directories in `*-command.json`; raw outputs are adjacent logs.

Final results: new probe 1,054 checks / zero failures; original reviewer probe
243 checks / zero failures; codec tests 8 cases / 115 assertions / zero skips;
all + planar_tests build reports no work. No full-suite execution is claimed.
The report records scope, source assessment, hashes and limitations.

The initial new probe had an off-by-one error in its raw-byte-boundary input;
its source and three failed harness checks remain as `initial-probe.cpp.txt`
and `initial-probe.log`. Final `probe.cpp` computes padding from the prefix
length and asserts the exact length. This needed no product change.

Before/after identity records verify the six-file candidate, 37 frozen coder
evidence files, 663 HEAD-exact Glaze files and current branch/revision. Executable
hashes are retained; reviewer-owned executables and debug bundles were removed.
`evidence-sha256.txt` covers this directory's final evidence except itself.
