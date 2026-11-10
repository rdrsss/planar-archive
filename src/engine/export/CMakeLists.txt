planar_rewrite_module(planar_source_export
  INTERFACE source_export.cppm SOURCES source_export.cpp
  DEPENDS planar_history_tree TESTS source_export.t.cpp TEST_TIMEOUT 120)
target_link_libraries(planar_source_export_tests PRIVATE planar_ledger_acceptance planar_acceptance_prepared planar_history_provenance_codec planar_canonical)
# Test-only edge (R15-C P4): the format3 export case commits its revision through the execution ledger API.
target_link_libraries(planar_source_export_tests PRIVATE planar_ledger_execution)
target_compile_definitions(planar_source_export_tests PRIVATE
  PLANAR_PREPARED_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures"
  PLANAR_EXPORT_FIXTURE="$<TARGET_FILE:planar_history_fixture>")

# R14-F1 redirects only the exact export source at native filesystem boundaries.
set(r14f1_support "${PROJECT_SOURCE_DIR}/tests/native/r14f1-support")
set(r14f1_generated "${CMAKE_CURRENT_BINARY_DIR}/r14f1-generated")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/source_export.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/source_export.cppm"
  "${r14f1_support}/generate.py")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${r14f1_support}/generate.py"
  "${CMAKE_CURRENT_SOURCE_DIR}" "${r14f1_generated}"
  COMMAND_ERROR_IS_FATAL ANY)
planar_rewrite_module(planar_export_posix_test
  INTERFACE "${r14f1_support}/posix.cppm" SOURCES "${r14f1_support}/posix.cpp")
planar_rewrite_module(planar_export_fault_test
  INTERFACE "${r14f1_generated}/source_export.cppm" SOURCES "${r14f1_generated}/source_export.cpp"
  DEPENDS planar_history_tree planar_export_posix_test TESTS source_export.t.cpp)
target_compile_definitions(planar_export_fault_test_tests PRIVATE PLANAR_EXPORT_FAULT_TEST
  PLANAR_PREPARED_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures")
target_link_libraries(planar_export_fault_test_tests PRIVATE planar_ledger_acceptance planar_acceptance_prepared planar_history_provenance_codec planar_canonical)
set_target_properties(planar_export_posix_test planar_export_fault_test PROPERTIES EXCLUDE_FROM_ALL TRUE)
