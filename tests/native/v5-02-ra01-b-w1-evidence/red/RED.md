# RED check V5-02-RA01-B-W1

HEAD: 307026288615e317d145ba6846c0692929c50a17
Snapshot: tests/native/v5-02-ra01-b-w1-evidence/red/snapshot-2 (snapshot/ already existed). Identity covers tracked (git diff HEAD -- src tests/native) and untracked bytes (src and tests/native/*-support).
sha256 1e6dfd3aeb7204ba4809b56bbdd304031effe188c4fbcb7c975895b16a996326  candidate.diff
sha256 d23c1470893a405b1981ef677e0802b18486c2f695ab47b30e3c5a89e3ac0b02  SHA256SUMS

Build: cmake --build build/rewrite-debug --target planar_tests succeeded (raw/build.log).
Command per test: ctest --test-dir build/rewrite-debug -R '^replacement\.<escaped name>$' --no-tests=error --output-on-failure. Logs: raw/prefixed/NN.log (NN = line in raw/names.txt). Earlier logs raw/NN.log are from a first attempt that omitted the replacement. ctest name prefix (No tests found); kept, not deleted.

All 69 listed tests ran. All 55 expectRed tests registered and failed on an assertion (file:line in log). Guards that PASSED: I01,I08,I09,I26,I56,I57,I58,I53. Guards that FAILED an assertion (allowed, but noteworthy): I25 (bootstrap.t.cpp:3301), I44 (bootstrap.t.cpp:3368), I45 (bootstrap.t.cpp:3223), I46 (bootstrap.t.cpp:3223), I52 (external_registration.t.cpp:5290), I54 (external_registration.t.cpp:5313). Rows I12a-g, I30a-f, I38a/b, I40a-c, I41a-c map to the same-name test; each name ran once (one log). I59-I64 have no test.

Failing assertion lines per test are in the ctest output above in each raw/prefixed log; summary lines: 02 authoring_catalog.t.cpp:183, 03 :197, 04 :209, 05 :222, 06 :243, 09 :256, 10 :268, validators :144, 68 :336, authoring.t.cpp 25 :358 ... 50 :873, bootstrap.t.cpp 54 :3223, 64 :1000, external_registration.t.cpp 55-60 :5224/5225, 69 :5328.
