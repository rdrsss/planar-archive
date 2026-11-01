# RED-CHECK V5-02-RA01-C-C01

Red-time identity (covers tracked and untracked bytes under src and tests/native/*-support):
- HEAD: 93053c42a4c68fd3ecc8911edab6464e3d1307f3
- sha256 candidate.diff (git diff HEAD -- src tests/native): c2a1fda122a9895be56b24811efbd76148c2efdbe235f38050d8b0f987b09f74
- sha256 SHA256SUMS: f914784422ae3b12751c8c411551300987458fd199a3014596b27d07f2a88b36
- Snapshot: red/snapshot/ (no untracked src or support files existed)

Build: cmake --build build/rewrite-debug --target planar_tests -> exit 0 (ninja: no work to do). Log raw/build.log.

Commands: ctest --test-dir build/rewrite-debug -R '<exact name>' --no-tests=error --output-on-failure

| test | invariants | expectRed | outcome | failing lines |
|---|---|---|---|---|
| RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | I1-I5, R1 | true | failed-assertion | authoring_orchestrator.t.cpp:750,751,752,755,759,761,765,767 (wrapper shape); 778 (sections order); 810 (every section mandatory); 861-873,890-892 (source pins). 28 of 79 assertions failed. Log raw/c01-wrapper.log |
| RA01-B2 orchestrator prompt carries five sections in order | I5b, I6 | false | passed | Log raw/b2-prompt.log |
| X08 C01 role preparation outputs-off boundary pins the request-record conversion charge | I8 | false | passed | Log raw/x08.log |

Non-TEST_CASE invariants (I7, I9-I11, G1-G9) are gates or reviews, not run here.
