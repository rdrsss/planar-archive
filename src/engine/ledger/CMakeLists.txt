planar_rewrite_module(planar_ledger_completion
  INTERFACE completion.cppm)

planar_rewrite_module(planar_ledger_sqlite
  INTERFACE sqlite.cppm SOURCES sqlite.cpp
  DEPENDS planar_sqlite TESTS sqlite.t.cpp reliability.t.cpp)

planar_rewrite_module(planar_ledger_types
  INTERFACE types.cppm DEPENDS planar_history_record planar_history_request_v2)

planar_rewrite_module(planar_ledger_storage_types
  INTERFACE storage_types.cppm
  DEPENDS planar_ledger_types planar_ledger_history_codec planar_value)

planar_rewrite_module(planar_ledger_storage
  INTERFACE storage.cppm SOURCES storage.cpp
  DEPENDS planar_ledger_storage_types planar_ledger_sqlite planar_json_codec planar_ledger_types planar_ledger_history_codec planar_history_execution_codec
    planar_history_tree_core planar_history_provenance_codec planar_history_interpretation_codec planar_history_recorded_interpretation planar_canonical
  TESTS storage.t.cpp)
# Test-only edge: the storage tests exercise the public format surface.
target_link_libraries(planar_ledger_storage_tests PRIVATE planar_ledger_format)

planar_rewrite_module(planar_ledger_acceptance
  INTERFACE acceptance.cppm acceptance_seam.cppm profile_acceptance.cppm
  SOURCES acceptance.cpp acceptance_seam.cpp profile_acceptance.cpp
  DEPENDS planar_runtime planar_ledger_storage planar_ledger_storage_types planar_acceptance_prepared planar_acceptance_intent
    planar_ledger_completion planar_ledger_types planar_ledger_history_codec planar_history_tree_core
  TESTS profile_acceptance.t.cpp profile_acceptance_seam.t.cpp TEST_TIMEOUT 60)
target_link_libraries(planar_ledger_acceptance_tests PRIVATE planar_ledger_format planar_acceptance_prepared planar_acceptance_intent
  planar_history_tree planar_history_read planar_source_export planar_json_codec)
target_compile_definitions(planar_ledger_acceptance_tests PRIVATE
  PLANAR_SCHEMA_FIXTURE="${PROJECT_SOURCE_DIR}/fixtures/render/quota")

planar_rewrite_module(planar_ledger_format
  INTERFACE format.cppm SOURCES format.cpp
  DEPENDS planar_ledger_completion planar_ledger_sqlite planar_json_codec planar_ledger_types planar_ledger_history_codec
    planar_ledger_storage planar_ledger_storage_types TESTS format.t.cpp snapshot.t.cpp history_verification.t.cpp acceptance.t.cpp)
# Test-only: fixtures prepare and complete acceptances; the format library itself links none of it.
target_link_libraries(planar_ledger_format_tests PRIVATE planar_ledger_acceptance planar_acceptance_prepared)
target_compile_definitions(planar_ledger_format_tests PRIVATE
  PLANAR_LEDGER_SCHEMA="${PROJECT_SOURCE_DIR}/fixtures/ledger/expected-schema.sql")

planar_rewrite_module(planar_ledger_execution_schema
  INTERFACE execution_schema.cppm SOURCES execution_schema.cpp
  DEPENDS planar_ledger_sqlite planar_ledger_storage planar_ledger_storage_types planar_json_codec planar_ledger_history_codec
  TESTS execution_schema.t.cpp)
# Test-only: the format1/format2 refusal and G1 guard open legacy ledgers through the existing format surface.
target_link_libraries(planar_ledger_execution_schema_tests PRIVATE planar_ledger_format)

planar_rewrite_module(planar_ledger_execution
  INTERFACE execution.cppm SOURCES execution.cpp execution_review.cpp
  DEPENDS planar_ledger_execution_schema planar_ledger_history_codec planar_history_request_v2 planar_history_request_v3 planar_history_execution_codec planar_hash planar_canonical planar_history_record planar_ledger_sqlite
    planar_history_tree_core planar_history_recorded_interpretation planar_json_codec
  TESTS execution.t.cpp execution_review.t.cpp TEST_TIMEOUT 120)
# Test-only: the format1/format2 refusals and the legacy-ledger fixtures open ledgers through the existing format surface.
target_link_libraries(planar_ledger_execution_tests PRIVATE planar_ledger_format planar_history_read)

# Test-only current-observation reader for the real execution workflow regression.
add_executable(planar_execution_snapshot_probe EXCLUDE_FROM_ALL snapshot_probe.t.cpp)
planar_native_target(planar_execution_snapshot_probe)
target_link_libraries(planar_execution_snapshot_probe PRIVATE planar_ledger_execution)
add_dependencies(planar_tests planar_execution_snapshot_probe)

add_test(NAME replacement.execution_sections_c11_coverage
  COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tests/native/r15-r18-c11-sections.py"
    "$<TARGET_FILE:planar_ledger_execution_tests>")
set_tests_properties(replacement.execution_sections_c11_coverage PROPERTIES
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}" LABELS integration TIMEOUT 120)

target_link_libraries(planar_ledger_storage PUBLIC planar_history_execution_codec)

planar_rewrite_module(planar_ledger_recovery
  INTERFACE recovery.cppm SOURCES recovery.cpp
  DEPENDS planar_ledger_execution)

# R12-C2 redirects only publication POSIX calls in an audited exact-source variant.
set(r12c2_support "${PROJECT_SOURCE_DIR}/tests/native/r12c2-support")
set(r12c2_generated "${CMAKE_CURRENT_BINARY_DIR}/r12c2-generated")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/format.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/format.cppm"
  "${CMAKE_CURRENT_SOURCE_DIR}/storage.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/storage.cppm"
  "${CMAKE_CURRENT_SOURCE_DIR}/acceptance.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/acceptance.cppm"
  "${CMAKE_CURRENT_SOURCE_DIR}/acceptance_seam.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/acceptance_seam.cppm"
  "${r12c2_support}/generate.py")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${r12c2_support}/generate.py"
  "${CMAKE_CURRENT_SOURCE_DIR}" "${r12c2_generated}"
  COMMAND_ERROR_IS_FATAL ANY)
planar_rewrite_module(planar_ledger_posix_test
  INTERFACE "${r12c2_support}/posix.cppm" SOURCES "${r12c2_support}/posix.cpp")
planar_rewrite_module(planar_ledger_fault_test
  INTERFACE "${r12c2_generated}/format.cppm" SOURCES "${r12c2_generated}/format.cpp"
  DEPENDS planar_ledger_completion planar_ledger_sqlite planar_json_codec planar_ledger_types planar_ledger_history_codec
    planar_history_tree_core planar_history_provenance_codec planar_history_interpretation_codec planar_history_recorded_interpretation planar_canonical planar_ledger_storage planar_ledger_storage_types planar_ledger_posix_test
  TESTS publication_fault.t.cpp)
set_target_properties(planar_ledger_posix_test planar_ledger_fault_test PROPERTIES EXCLUDE_FROM_ALL TRUE)

# R13-B1 forces only record digests in a reversible verification-only format variant.
set(r13b1_support "${PROJECT_SOURCE_DIR}/tests/native/r13b1-support")
set(r13b1_generated "${CMAKE_CURRENT_BINARY_DIR}/r13b1-generated")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${r13b1_support}/generate.py")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${r13b1_support}/generate.py"
  "${CMAKE_CURRENT_SOURCE_DIR}" "${r13b1_generated}"
  COMMAND_ERROR_IS_FATAL ANY)
planar_rewrite_module(planar_record_digest_test
  INTERFACE "${r13b1_support}/digest.cppm" DEPENDS planar_history_record)
planar_rewrite_module(planar_record_storage_test
  INTERFACE "${r13b1_generated}/storage.cppm" SOURCES "${r13b1_generated}/storage.cpp"
  DEPENDS planar_ledger_storage_types planar_ledger_sqlite planar_json_codec planar_ledger_types planar_ledger_history_codec planar_history_execution_codec
    planar_history_tree_core planar_history_provenance_codec planar_history_interpretation_codec planar_history_recorded_interpretation planar_canonical planar_record_digest_test)
planar_rewrite_module(planar_record_format_test
  INTERFACE "${r13b1_generated}/format.cppm" SOURCES "${r13b1_generated}/format.cpp"
  DEPENDS planar_record_storage_test planar_ledger_storage_types planar_ledger_completion planar_ledger_sqlite planar_json_codec planar_ledger_types planar_ledger_history_codec
    planar_record_digest_test
  TESTS "${r13b1_support}/collision.t.cpp")
set_target_properties(planar_record_digest_test planar_record_storage_test planar_record_format_test PROPERTIES EXCLUDE_FROM_ALL TRUE)

planar_rewrite_module(planar_ledger_history_codec
  INTERFACE history_codec.cppm SOURCES history_codec.cpp
  DEPENDS planar_json_codec TESTS history_codec.t.cpp)

# Higher wrappers are fixture preparation only; production format links pure validators.
target_link_libraries(planar_ledger_format_tests PRIVATE planar_history_provenance planar_history_request_v2 planar_history_read planar_source_export)
target_compile_definitions(planar_ledger_format_tests PRIVATE
  PLANAR_PREPARED_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures"
  PLANAR_HISTORY_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures"
  PLANAR_HISTORY_FIXTURE="${PROJECT_SOURCE_DIR}/fixtures/render/quota"
  PLANAR_V2_VECTORS="${PROJECT_SOURCE_DIR}/tests/vectors/interpretation-v2")

# R14-C3-B2 selectively forces one record kind; every other hash and validator remains real.
set(r14c3b2_support "${PROJECT_SOURCE_DIR}/tests/native/r14c3b2-support")
set(r14c3b2_generated "${CMAKE_CURRENT_BINARY_DIR}/r14c3b2-generated")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${r14c3b2_support}/generate.py")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${r14c3b2_support}/generate.py"
  "${CMAKE_CURRENT_SOURCE_DIR}" "${r14c3b2_generated}"
  COMMAND_ERROR_IS_FATAL ANY)
planar_rewrite_module(planar_atomic_digest_test
  INTERFACE "${r14c3b2_support}/digest.cppm" DEPENDS planar_history_record planar_history_request_v2)
planar_rewrite_module(planar_atomic_storage_test
  INTERFACE "${r14c3b2_generated}/storage.cppm" SOURCES "${r14c3b2_generated}/storage.cpp"
  DEPENDS planar_ledger_storage_types planar_ledger_sqlite planar_json_codec planar_ledger_types planar_ledger_history_codec planar_history_execution_codec
    planar_history_tree_core planar_history_provenance_codec planar_history_interpretation_codec planar_history_recorded_interpretation planar_canonical planar_atomic_digest_test)
planar_rewrite_module(planar_atomic_acceptance_test
  INTERFACE "${r14c3b2_generated}/acceptance.cppm" "${r14c3b2_generated}/acceptance_seam.cppm"
  SOURCES "${r14c3b2_generated}/acceptance.cpp" "${r14c3b2_generated}/acceptance_seam.cpp"
  DEPENDS planar_atomic_storage_test planar_ledger_storage_types planar_acceptance_intent planar_acceptance_prepared planar_ledger_completion planar_ledger_types planar_ledger_history_codec planar_history_tree_core)
planar_rewrite_module(planar_atomic_format_test
  INTERFACE "${r14c3b2_generated}/format.cppm" SOURCES "${r14c3b2_generated}/format.cpp"
  DEPENDS planar_atomic_storage_test planar_ledger_storage_types planar_ledger_completion planar_ledger_sqlite planar_json_codec planar_ledger_types planar_ledger_history_codec
    planar_atomic_digest_test
  TESTS "${r14c3b2_support}/atomic.t.cpp")
target_link_libraries(planar_atomic_format_test_tests PRIVATE planar_atomic_acceptance_test planar_atomic_storage_test planar_acceptance_prepared)
target_compile_definitions(planar_atomic_format_test_tests PRIVATE
  PLANAR_PREPARED_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures")
set_target_properties(planar_atomic_digest_test planar_atomic_storage_test planar_atomic_acceptance_test planar_atomic_format_test PROPERTIES EXCLUDE_FROM_ALL TRUE)

# D8 native-only acceptance faults reuse the real engine with a scoped test VFS.
target_sources(planar_ledger_format_tests PRIVATE
  "${PROJECT_SOURCE_DIR}/tests/native/r14-d8-support/acceptance_fault.t.cpp")

# X03-B1 extends the existing reversible publication fault variant to native format2 lifecycle.
target_sources(planar_ledger_fault_test_tests PRIVATE
  "${PROJECT_SOURCE_DIR}/tests/native/v5-02-x03-b1-support/publication.t.cpp")
add_executable(planar_format2_fixture EXCLUDE_FROM_ALL
  "${PROJECT_SOURCE_DIR}/tests/native/v5-02-x03-b1-support/fixture.cpp")
planar_native_target(planar_format2_fixture)
target_link_libraries(planar_format2_fixture PRIVATE planar_ledger_format)
add_dependencies(planar_tests planar_format2_fixture)

# X03-B2-B-C01 supplies only verified metadata in a reversible exact-source test variant.
set(x03b2bc01_support "${PROJECT_SOURCE_DIR}/tests/native/v5-02-x03-b2-b-c01-support")
set(x03b2bc01_generated "${CMAKE_CURRENT_BINARY_DIR}/v5-02-x03-b2-b-c01-generated")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${x03b2bc01_support}/generate.py")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${x03b2bc01_support}/generate.py"
  "${CMAKE_CURRENT_SOURCE_DIR}" "${x03b2bc01_generated}"
  COMMAND_ERROR_IS_FATAL ANY)
planar_rewrite_module(planar_metadata_fixture
  INTERFACE "${x03b2bc01_support}/fixture.cppm")
planar_rewrite_module(planar_metadata_storage_test
  INTERFACE "${x03b2bc01_generated}/storage.cppm" SOURCES "${x03b2bc01_generated}/storage.cpp"
  DEPENDS planar_ledger_storage_types planar_ledger_sqlite planar_json_codec planar_ledger_types planar_ledger_history_codec planar_history_execution_codec
    planar_history_tree_core planar_history_provenance_codec planar_history_interpretation_codec planar_history_recorded_interpretation planar_canonical planar_metadata_fixture)
planar_rewrite_module(planar_metadata_acceptance_test
  INTERFACE "${x03b2bc01_generated}/acceptance.cppm" "${x03b2bc01_generated}/acceptance_seam.cppm"
  SOURCES "${x03b2bc01_generated}/acceptance.cpp" "${x03b2bc01_generated}/acceptance_seam.cpp"
  DEPENDS planar_metadata_storage_test planar_ledger_storage_types planar_acceptance_intent planar_acceptance_prepared planar_ledger_completion planar_ledger_types planar_ledger_history_codec planar_history_tree_core)
planar_rewrite_module(planar_metadata_format_test
  INTERFACE "${x03b2bc01_generated}/format.cppm" SOURCES "${x03b2bc01_generated}/format.cpp"
  DEPENDS planar_metadata_storage_test planar_ledger_storage_types planar_ledger_completion planar_ledger_sqlite planar_json_codec planar_ledger_types planar_ledger_history_codec
    planar_metadata_fixture
  TESTS "${x03b2bc01_support}/metadata.t.cpp")
target_link_libraries(planar_metadata_format_test_tests PRIVATE planar_metadata_acceptance_test planar_metadata_storage_test planar_acceptance_prepared)
target_compile_definitions(planar_metadata_format_test_tests PRIVATE
  PLANAR_PREPARED_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures")
set_target_properties(planar_metadata_fixture planar_metadata_storage_test planar_metadata_acceptance_test planar_metadata_format_test PROPERTIES EXCLUDE_FROM_ALL TRUE)

target_link_libraries(planar_ledger_execution_tests PRIVATE planar_history_read)
