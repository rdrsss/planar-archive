planar_rewrite_module(planar_history_effective_catalog
  INTERFACE effective_catalog.cppm SOURCES effective_catalog.cpp
  DEPENDS planar_canonical TESTS effective_catalog.t.cpp)

planar_rewrite_module(planar_history_record
  INTERFACE record.cppm SOURCES record.cpp
  DEPENDS planar_canonical planar_xxhash TESTS record.t.cpp)
target_compile_definitions(planar_history_record_tests PRIVATE
  PLANAR_RECORD_VECTOR_DIR="${PROJECT_SOURCE_DIR}/tests/vectors/records")

planar_rewrite_module(planar_history_request_v2
  INTERFACE request_v2.cppm SOURCES request_v2.cpp
  DEPENDS planar_history_record planar_canonical planar_xxhash TESTS request_v2.t.cpp)
target_compile_definitions(planar_history_request_v2_tests PRIVATE
  PLANAR_RECORD_VECTOR_DIR="${PROJECT_SOURCE_DIR}/tests/vectors/records")

planar_rewrite_module(planar_history_request_v3
  INTERFACE request_v3.cppm SOURCES request_v3.cpp
  DEPENDS planar_history_record planar_canonical planar_xxhash TESTS request_v3.t.cpp)
planar_rewrite_module(planar_history_execution_codec
  INTERFACE execution_codec.cppm SOURCES execution_codec.cpp
  DEPENDS planar_history_record planar_canonical planar_hash planar_json_codec planar_xxhash TESTS execution_codec.t.cpp)

planar_rewrite_module(planar_history_tree_core
  INTERFACE tree_core.cppm SOURCES tree_core.cpp
  DEPENDS planar_ledger_types planar_value TESTS tree_core.t.cpp)

planar_rewrite_module(planar_history_tree
  INTERFACE tree.cppm SOURCES tree.cpp
  DEPENDS planar_ledger_format planar_history_tree_core TESTS tree.t.cpp TEST_TIMEOUT 120)

planar_rewrite_module(planar_history_provenance_codec
  INTERFACE provenance_codec.cppm SOURCES provenance_codec.cpp
  DEPENDS planar_history_tree_core planar_canonical TESTS provenance_codec.t.cpp)

planar_rewrite_module(planar_history_interpretation_codec
  INTERFACE interpretation_codec.cppm SOURCES interpretation_codec.cpp
  DEPENDS planar_canonical TESTS interpretation_codec.t.cpp)
target_compile_definitions(planar_history_interpretation_codec_tests PRIVATE
  PLANAR_INTERPRETATION_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures")

planar_rewrite_module(planar_history_provenance
  INTERFACE provenance.cppm SOURCES provenance.cpp
  DEPENDS planar_history_tree planar_history_provenance_codec planar_runtime_source TESTS provenance.t.cpp)
target_compile_definitions(planar_history_provenance_tests PRIVATE
  PLANAR_STDLIB_DIR="${PROJECT_SOURCE_DIR}/src/stdlib")

planar_rewrite_module(planar_history_interpretation
  INTERFACE interpretation.cppm SOURCES interpretation.cpp
  DEPENDS planar_history_provenance planar_history_interpretation_codec planar_runtime TESTS interpretation.t.cpp)
target_link_libraries(planar_history_interpretation_tests PRIVATE planar_capture planar_json_codec)
target_compile_definitions(planar_history_interpretation_tests PRIVATE
  PLANAR_STDLIB_DIR="${PROJECT_SOURCE_DIR}/src/stdlib"
  PLANAR_INTERPRETATION_FIXTURE="${PROJECT_SOURCE_DIR}/fixtures/render/quota"
  PLANAR_INTERPRETATION_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures")

planar_rewrite_module(planar_history_read
  INTERFACE read.cppm SOURCES read.cpp
  DEPENDS planar_ledger_format planar_history_interpretation_codec planar_history_recorded_interpretation planar_history_tree planar_json_codec
    planar_history_execution_codec
  TESTS read.t.cpp)
# Test-only edge (R15-C P4): the format3 history cases create their ledger with the format3 initializer.
target_link_libraries(planar_history_read_tests PRIVATE planar_ledger_execution_schema)

add_executable(planar_history_fixture EXCLUDE_FROM_ALL
  "${PROJECT_SOURCE_DIR}/tests/native/r14e2-support/fixture.cpp")
planar_native_target(planar_history_fixture)
target_link_libraries(planar_history_fixture PRIVATE planar_ledger_format planar_ledger_acceptance planar_acceptance_prepared planar_history_provenance_codec planar_canonical planar_sqlite)
target_compile_definitions(planar_history_fixture PRIVATE
  PLANAR_PREPARED_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures")
add_dependencies(planar_tests planar_history_fixture)

planar_rewrite_module(planar_history_effective_provenance
  INTERFACE effective_provenance.cppm SOURCES effective_provenance.cpp
  DEPENDS planar_history_provenance_codec planar_capture_profile_codec planar_history_effective_catalog planar_canonical
  TESTS effective_provenance.t.cpp)
target_compile_definitions(planar_history_effective_provenance_tests PRIVATE
  PLANAR_EFFECTIVE_VECTORS="${PROJECT_SOURCE_DIR}/tests/vectors/effective-provenance")

# Limit 1680 s is D32: max(120, ceil60(2 x 813 s loaded direct time)); its 16 v2 codec test cases share it, so a hung test is detected later.
planar_rewrite_module(planar_history_interpretation_v2_codec
  INTERFACE interpretation_v2_codec.cppm SOURCES interpretation_v2_codec.cpp
  DEPENDS planar_history_effective_provenance planar_history_effective_catalog planar_hash
  TESTS interpretation_v2_codec.t.cpp TEST_TIMEOUT 1680)
target_compile_definitions(planar_history_interpretation_v2_codec_tests PRIVATE
  PLANAR_V2_VECTORS="${PROJECT_SOURCE_DIR}/tests/vectors/interpretation-v2"
  PLANAR_EFFECTIVE_VECTORS="${PROJECT_SOURCE_DIR}/tests/vectors/effective-provenance"
  PLANAR_V1_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures")

planar_rewrite_module(planar_history_recorded_interpretation
  INTERFACE recorded_interpretation.cppm SOURCES recorded_interpretation.cpp
  DEPENDS planar_history_interpretation_codec planar_history_interpretation_v2_codec planar_canonical
  TESTS recorded_interpretation.t.cpp TEST_TIMEOUT 120)
target_compile_definitions(planar_history_recorded_interpretation_tests PRIVATE
  PLANAR_V2_VECTORS="${PROJECT_SOURCE_DIR}/tests/vectors/interpretation-v2"
  PLANAR_V1_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures")
