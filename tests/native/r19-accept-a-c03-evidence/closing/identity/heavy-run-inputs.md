# Heavy run inputs (closing heavy run, started 2026-11-12T18:59:57Z)

sha256 of the support files the closing heavy run executed, against the committed bytes. Run-time copies were taken before the run (fault_driver.py, run_heavy.py: 2026-11-12T13:59 local, the moment the run was started) and before the first post-run support edit (run_acceptance.py); they are scratch files, not committed.

| file | at run time | committed | equal |
|---|---|---|---|
| `fault_driver.py` | `f6d02ceb9ad3a1447a313259cb0fcb0a5f576dd1dcc20914fa6c8bb6364114d0` | `f6d02ceb9ad3a1447a313259cb0fcb0a5f576dd1dcc20914fa6c8bb6364114d0` | yes |
| `fault/cmake_rerun/run_heavy.py` | `956acd6119c9483c0a32c63ccc817ef9456a4d099506dedd78667492a388e602` | `956acd6119c9483c0a32c63ccc817ef9456a4d099506dedd78667492a388e602` | yes |
| `run_acceptance.py` | `2ff48c14f2bb7daff546583f1e58888220378a3f26e70255201cf17ec038c1a8` | `4e034aa7f28e2080c83761b595916d822010f464a0e233df9dc3353dd8103781` | no: the committed file adds only the two commit-list rule edits (C4-D4 `COMMIT_LIST_INDEX_ALLOWED`, C4-D7 snapshot rule on directories); the heavy run does not call `commit_list_path_rules` |
| `accept_lib.py` | unchanged since before the run (mtime 2026-11-11) | `6ef4e069c2da203d508f3695482b5ea9883b71fc47ee8519a6f910f99e223e68` | yes |
