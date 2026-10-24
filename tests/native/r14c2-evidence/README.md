# R14-C2 retained acceptance capture candidate

Owned candidate: capture.cppm/.cpp/.t.cpp plus architecture/testing references.
The exact baseline is under `baseline/`; `candidate/`, `candidate.json` and
`candidate.diff` freeze the final files and their composite SHA256 identity.
No CMake, ledger/history, Lua, CLI, vendor or existing evidence changes belong to
this candidate. Planning under docs/vernacular is coordinator-owned and excluded
from the protection audit for that stated reason.

The capability owns the original sealed bytes/topology and original retained
root descriptors/identity receipts. Its repeat native verifier reuses the initial
finish comparisons; initial seal still invokes the existing seam and preserves
its diagnostic mapping. Postseal verification never invokes a callback, updates
alias state, or replaces sealed values. Mode comparison is the existing normalized
0644/0755 contract. Standalone inputs retain file and parent identity without
traversing parent membership. An fdopendir failure now leaves its descriptor in
RAII ownership rather than releasing it before successful adoption by DIR.

Final serialized command arguments/exits are in `commands.json`, run by `run.py`:
all/planar_tests build, all 21 capture cases (16 pre-existing exact-prefix cases,
five additive acceptance cases), the unchanged 151 ledger/history focused cases,
CLI acceptance (247 real processes), both vector checks, active source
formatting/Doxygen and a zero-step no-change build. `helper-lint.txt` records the
pinned formatter's zero-warning check of the four unchanged C++ verification
helpers. `audit.py` checks exact preservation, test counts/skips, no unexpected
nonplanning files and exact/reverse-identical R12-C2/R13-B1 mechanical generation.

`baseline-api-absent.txt/json` replay the additive tests against the exact baseline
capture sources: compilation fails because the acceptance capability/factory do
not exist. `regression.py` restores the candidate in finally; the successful final
build and checks follow restoration. A preliminary capture run (`capture.txt/xml`)
overlapped this baseline compile replay; it is superseded by the final serialized
capture run. The replay failed before linking, so it never changed that running
test binary. Initial build outputs are separately retained.

`before-doxygen-fix/` preserves the first serialized sequence: capture21,
focused151, CLI247 and both vectors passed; Doxygen required parameter/return docs
on the new move assignment declaration. The correction only adds those docs and
parameter names. The final serialized sequence repeats all checks against the
frozen final candidate. No earlier failures are hidden or replaced.

Limits: macOS debug host only. Linux, release and complete R14 milestone gates
remain outstanding. Capture detects cooperative changes while comparing; it does
not prevent writes after the check or promise detection of changes reverted before
comparison. This slice adds no writer; a later writer must require the capability
and invoke native verification immediately before every commit attempt. This is
an implementation report awaiting independent review, not self-approval.
