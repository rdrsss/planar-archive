# X06-B1-C01 dependency cycle and forbidden-reachability manifest

Round: V5-02-X06-B1-C01 (package B1C01-ALL), criterion 5.
Build directory of record: `build/rewrite-debug` (Debug, macOS). This manifest uses a
separate scratch configure so the gated build tree is not touched.

## Commands and exit codes (unpiped)

1. Target graph (scratch configure only; `-S . -B <scratch>/graphbuild`, preset `debug`):
   `cmake --graphviz=<scratch>/graph.dot -S . -B <scratch>/graphbuild --preset debug > raw/graph-configure.log 2>&1` -> exit 0.
   Copy: `raw/target-graph.dot` (254 nodes, 1321 edges in the dot file).
   Check that the gated tree was not reconfigured: `build/rewrite-debug/build.ninja` mtime 20:05:44 and
   `CMakeCache.txt` mtime 19:23:36 both predate the graph run (20:12); `CMAKE_CACHEFILE_DIR` differs (scratch versus gated).
2. Checker (python3 -I, scratch copy `raw/check_graph.py`):
   `python3 -I <scratch>/check_graph.py <scratch>/graph.dot > <scratch>/check_graph.out 2>&1` -> exit 0.
   Output copied to `raw/check_graph.out` and reproduced below verbatim.

The checker parses node labels and `"nodeN" -> "nodeM"` edges (legend nodes skipped), runs an iterative
Tarjan SCC over all nodes, checks forbidden reachability from `planar_ledger_storage` and
`planar_ledger_storage_types` (must not reach `planar_ledger_format`, `planar_ledger_acceptance`,
`planar_acceptance_prepared`), and checks that `planar_ledger_format` non-test closure (test targets
matching `_test`/`_tests` are excluded from traversal) does not contain `planar_ledger_acceptance`.
It exits 1 on any cycle or violation; it exited 0.

### Checker output (verbatim)

```
nodes=254 edges=1321
SCC_total=254 SCC_size_gt1=0 self_loops=41
planar_ledger_storage reaches planar_ledger_format: False
planar_ledger_storage reaches planar_ledger_acceptance: False
planar_ledger_storage reaches planar_acceptance_prepared: False
planar_ledger_storage direct deps: ['@cmake_cxx_std', 'planar_canonical', 'planar_history_interpretation_codec', 'planar_history_provenance_codec', 'planar_history_recorded_interpretation', 'planar_history_tree_core', 'planar_json_codec', 'planar_ledger_history_codec', 'planar_ledger_sqlite', 'planar_ledger_storage_types', 'planar_ledger_types']
planar_ledger_storage full closure size: 22
planar_ledger_storage_types reaches planar_ledger_format: False
planar_ledger_storage_types reaches planar_ledger_acceptance: False
planar_ledger_storage_types reaches planar_acceptance_prepared: False
planar_ledger_storage_types direct deps: ['@cmake_cxx_std', 'planar_ledger_history_codec', 'planar_ledger_types']
planar_ledger_storage_types full closure size: 10
planar_ledger_format non-test closure contains planar_ledger_acceptance: False
planar_ledger_format full closure contains planar_ledger_acceptance: False
planar_ledger_format direct deps: ['@cmake_cxx_std', 'planar_json_codec', 'planar_ledger_completion', 'planar_ledger_history_codec', 'planar_ledger_sqlite', 'planar_ledger_storage', 'planar_ledger_storage_types', 'planar_ledger_types']
planar_ledger_acceptance direct deps: ['@cmake_cxx_std', 'planar_acceptance_intent', 'planar_acceptance_prepared', 'planar_ledger_completion', 'planar_ledger_history_codec', 'planar_ledger_storage', 'planar_ledger_storage_types', 'planar_ledger_types']
planar_ledger_format_tests direct deps: ['@cmake_cxx_std', 'Catch2WithMain\\n(Catch2::Catch2WithMain)', 'planar_acceptance_prepared', 'planar_history_provenance', 'planar_history_read', 'planar_history_request_v2', 'planar_ledger_acceptance', 'planar_ledger_format', 'planar_ledger_format_tests', 'planar_source_export']
cycles: 0 violations: []
```

## Interpretation

- SCC over 254 nodes: 0 components of size >1. No dependency cycle.
- 41 self-loops: every one is a `*_tests` node pointing at itself (for example
  `planar_canonical_tests -> planar_canonical_tests`). This is the CMake graphviz artifact for test
  targets (the TESTS helper edge), not a source dependency. None is on a module library node.
- `planar_ledger_storage` (direct deps, matches CMake DEPENDS at src/engine/CMakeLists.txt:159-162):
  `planar_canonical`, `planar_history_interpretation_codec`, `planar_history_provenance_codec`,
  `planar_history_recorded_interpretation`, `planar_history_tree_core`, `planar_json_codec`,
  `planar_ledger_history_codec`, `planar_ledger_sqlite`, `planar_ledger_storage_types`, `planar_ledger_types`.
  No edge to acceptance, intent, prepared or format.
- `planar_ledger_storage_types` (direct deps, matches CMake DEPENDS at lines 155-157):
  `planar_ledger_history_codec`, `planar_ledger_types`.
- `planar_ledger_acceptance` (CMake DEPENDS at line 173) has the expected downward edges to
  `planar_acceptance_intent`, `planar_acceptance_prepared`, `planar_ledger_storage`, `planar_ledger_storage_types`, and others.
  Its DEPENDS list is one-directional: no storage, storage_types or format node reaches acceptance.
- `planar_ledger_format` (CMake DEPENDS at lines 175-178) depends on `planar_ledger_storage` and
  `planar_ledger_storage_types` (correct direction). Its non-test closure and full closure do not contain
  `planar_ledger_acceptance`. `planar_ledger_acceptance` appears only as a direct dependency of
  `planar_ledger_format_tests` (test-only linkage), as the brief expected.

Result: no cycle, no forbidden reachability, no violation. The checker stop condition was not triggered.
