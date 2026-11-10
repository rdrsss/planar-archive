# R18-2 verdict document reader: capture step 1, bounds, Glaze grammar, field table and normalization.
planar_rewrite_module(planar_execution_verdict
  INTERFACE verdict.cppm SOURCES verdict.cpp
  DEPENDS planar_history_execution_codec planar_capture planar_capture_host_path planar_json_codec planar_value
  TESTS verdict.t.cpp)

# R15-C native execution services (EX-9 placement): the read_execution snapshot value, execution_change_v1 decoding with
# native request framing, and the read_provenance single-file capture.
planar_rewrite_module(planar_execution_snapshot_value
  INTERFACE snapshot_value.cppm SOURCES snapshot_value.cpp
  DEPENDS planar_ledger_execution planar_value planar_canonical TESTS snapshot_value.t.cpp)
planar_rewrite_module(planar_execution_change_value
  INTERFACE change_value.cppm SOURCES change_value.cpp
  DEPENDS planar_ledger_execution planar_value planar_canonical TESTS change_value.t.cpp)
# R15-D P1 (DD-D6): the status drafts comparison of a captured declaration closure with the recorded source tree.
planar_rewrite_module(planar_execution_drafts
  INTERFACE drafts.cppm SOURCES drafts.cpp
  DEPENDS planar_history_tree_core TESTS drafts.t.cpp)
planar_rewrite_module(planar_execution_provenance_input
  INTERFACE provenance_input.cppm SOURCES provenance_input.cpp
  DEPENDS planar_capture planar_json_codec planar_history_execution_codec planar_value
  TESTS provenance_input.t.cpp)

planar_rewrite_module(planar_execution_check
  INTERFACE check.cppm SOURCES check.cpp
  DEPENDS planar_value planar_capture_native planar_capture_tree_digest planar_process_runner planar_process_resolve planar_hash planar_json_codec planar_glaze TESTS check.t.cpp)

planar_rewrite_module(planar_execution_recovery_capture
  INTERFACE recovery_capture.cppm SOURCES recovery_capture.cpp
  DEPENDS planar_ledger_execution planar_capture_native planar_capture_host_path planar_hash
  TESTS recovery_capture.t.cpp)
