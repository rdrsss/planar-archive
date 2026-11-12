# R19-ACCEPT-A red check

Red-time identity (covers tracked and untracked bytes):
- HEAD: 91e0df24f2167471e49ec057c7159bce5dff3608
- snapshot-4/candidate.diff (git diff HEAD -- src tests/native; empty, no tracked edits) sha256: e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
- snapshot-4/SHA256SUMS (candidate.diff plus 13 untracked files under tests/native/r19-accept-support) sha256: 5a91fbc5489c7894f200febcc4d81f5c02c90c1ca60b4a6a0e69d0eb3b6687ac
- snapshot/ and snapshot-2/ pre-existed. snapshot-3/ is defective (my pathspec 'tests/native/*-support' matched no files, so it holds an empty untracked set); kept, superseded by snapshot-4.

Commands: cmake --build build/rewrite-debug --target planar_tests (exit 0, log raw/build.log).
Finding: the target tests are Python unittest classes under tests/native/r19-accept-support, not registered in ctest (ctest -N lists 1851 tests, none matching). So ctest -R would be not-registered; instead each test was run by exact name:
  cd tests/native/r19-accept-support && python3 -B -m unittest -v <module>.<Class>.<test>
Raw logs: raw/<Class>.<test>.log. raw/AcceptLibTest.test_{a,b,error,fail,skip}.log are stray logs from a name-extraction slip (nonexistent tests); ignore, not deleted.

Caveat: 45 of the 86 fail on "AssertionError: PLANAR_CANDIDATE must name the D28 candidate binary" (live tests without a candidate binary), so those are red for a missing environment rather than missing behaviour. Others fail on AcceptError not raised, False is not true, list differences.

Summary: 86 run, 86 failed-assertion, 0 passed, 0 crash. Full per-test table (test, outcome, failing line) in table.tsv; invariants map in order I1-I35, Q1-Q19, F1-F25, A1-A7 = the listed order; all expectRed=true.
