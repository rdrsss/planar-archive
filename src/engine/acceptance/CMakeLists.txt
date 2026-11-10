planar_rewrite_module(planar_acceptance_intent
  INTERFACE intent.cppm SOURCES intent.cpp
  DEPENDS planar_value planar_history_record planar_history_request_v2 TESTS intent.t.cpp)

planar_rewrite_module(planar_acceptance_prepared
  INTERFACE prepared.cppm SOURCES prepared.cpp
  DEPENDS planar_capture_native planar_history_tree_core planar_history_provenance_codec
    planar_history_interpretation_codec planar_history_record planar_history_request_v2 planar_acceptance_intent TESTS prepared.t.cpp)
target_link_libraries(planar_acceptance_prepared_tests PRIVATE planar_capture_host_path)
target_compile_definitions(planar_acceptance_prepared_tests PRIVATE
  PLANAR_PREPARED_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures")

planar_rewrite_module(planar_acceptance_source_capture
  INTERFACE source_capture.cppm SOURCES source_capture.cpp
  DEPENDS planar_capture planar_capture_host_path planar_canonical planar_value planar_capture_native
  TESTS source_capture.t.cpp)
target_link_libraries(planar_acceptance_source_capture_tests PRIVATE planar_runtime planar_capture_profile planar_admission_syscalls planar_json_codec)
target_compile_definitions(planar_acceptance_source_capture_tests PRIVATE
  PLANAR_LOAD_FIXTURE="${PROJECT_SOURCE_DIR}/fixtures/render/quota")

planar_rewrite_module(planar_acceptance_load
  INTERFACE load.cppm SOURCES load.cpp
  DEPENDS planar_acceptance_prepared planar_capture planar_capture_host_path planar_canonical planar_acceptance_source_capture
  TESTS load.t.cpp)
target_link_libraries(planar_acceptance_load_tests PRIVATE planar_runtime planar_history_provenance planar_json_codec)
target_compile_definitions(planar_acceptance_load_tests PRIVATE
  PLANAR_LOAD_FIXTURE="${PROJECT_SOURCE_DIR}/fixtures/render/quota")

# D6 private admission; production keeps exactly four mandatory entries.
