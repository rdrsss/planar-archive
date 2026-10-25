planar_rewrite_module(planar_ledger_completion
  INTERFACE ledger/completion.cppm)

planar_rewrite_module(planar_value
  INTERFACE value/value.cppm SOURCES value/value.cpp)
planar_rewrite_module(planar_lua_loader
  INTERFACE lua/loader.cppm SOURCES lua/loader.cpp
  DEPENDS planar_value planar_lua TESTS lua/loader.t.cpp)
planar_rewrite_module(planar_capture_host_path
  INTERFACE capture/host_path.cppm SOURCES capture/host_path.cpp
  DEPENDS planar_value TESTS capture/host_path.t.cpp)
planar_rewrite_module(planar_capture_admission
  INTERFACE capture/admission.cppm SOURCES capture/admission.cpp
  DEPENDS planar_value TESTS capture/admission.t.cpp)
planar_rewrite_module(planar_capture_native
  INTERFACE capture/native.cppm SOURCES capture/native.cpp
  DEPENDS planar_value planar_capture_admission TESTS capture/native.t.cpp)
add_library(planar_admission_syscalls SHARED EXCLUDE_FROM_ALL
  "${PROJECT_SOURCE_DIR}/tests/native/v5-02-x04-a1-support/syscalls.cpp")
planar_native_target(planar_admission_syscalls)
target_link_libraries(planar_capture_admission_tests PRIVATE planar_capture_native planar_admission_syscalls)
planar_rewrite_module(planar_capture_profile_codec
  INTERFACE capture/profile_codec.cppm SOURCES capture/profile_codec.cpp
  DEPENDS planar_value planar_glaze TESTS capture/profile_codec.t.cpp)
planar_rewrite_module(planar_capture_profile
  INTERFACE capture/profile.cppm SOURCES capture/profile.cpp
  DEPENDS planar_capture_native planar_capture_host_path planar_capture_profile_codec TESTS capture/profile.t.cpp)
target_link_libraries(planar_capture_profile_tests PRIVATE planar_admission_syscalls)
planar_rewrite_module(planar_capture
  INTERFACE capture/capture.cppm SOURCES capture/capture.cpp
  DEPENDS planar_capture_native planar_value planar_lua_loader TESTS capture/capture.t.cpp
  TEST_TIMEOUT 120)

planar_rewrite_module(planar_canonical
  INTERFACE canonical/canonical.cppm SOURCES canonical/canonical.cpp
  DEPENDS planar_value TESTS canonical/canonical.t.cpp)
planar_rewrite_module(planar_hash
  INTERFACE hash/hash.cppm SOURCES hash/hash.cpp
  DEPENDS planar_canonical planar_xxhash TESTS hash/hash.t.cpp)
target_link_libraries(planar_hash_tests PRIVATE planar_capture)
target_compile_definitions(planar_hash_tests PRIVATE
  PLANAR_VECTOR_DIR="${PROJECT_SOURCE_DIR}/tests/vectors")

planar_rewrite_module(planar_history_effective_catalog
  INTERFACE history/effective_catalog.cppm SOURCES history/effective_catalog.cpp
  DEPENDS planar_canonical TESTS history/effective_catalog.t.cpp)

planar_rewrite_module(planar_history_record
  INTERFACE history/record.cppm SOURCES history/record.cpp
  DEPENDS planar_canonical planar_xxhash TESTS history/record.t.cpp)
target_compile_definitions(planar_history_record_tests PRIVATE
  PLANAR_RECORD_VECTOR_DIR="${PROJECT_SOURCE_DIR}/tests/vectors/records")

planar_rewrite_module(planar_history_request_v2
  INTERFACE history/request_v2.cppm SOURCES history/request_v2.cpp
  DEPENDS planar_history_record planar_canonical planar_xxhash TESTS history/request_v2.t.cpp)
target_compile_definitions(planar_history_request_v2_tests PRIVATE
  PLANAR_RECORD_VECTOR_DIR="${PROJECT_SOURCE_DIR}/tests/vectors/records")

planar_rewrite_module(planar_json_codec
  INTERFACE json_codec/codec.cppm SOURCES json_codec/codec.cpp
  DEPENDS planar_value planar_glaze TESTS json_codec/codec.t.cpp)

include("${PROJECT_SOURCE_DIR}/cmake/embed_lua.cmake")
# Link deterministic libraries only to the trusted runtime, not declaration jobs.
add_library(planar_lua_trusted_libs STATIC EXCLUDE_FROM_ALL
  "${PLANAR_LUA_SOURCE_DIR}/src/lbaselib.c"
  "${PLANAR_LUA_SOURCE_DIR}/src/ltablib.c"
  "${PLANAR_LUA_SOURCE_DIR}/src/lstrlib.c"
  "${PLANAR_LUA_SOURCE_DIR}/src/lmathlib.c"
  "${PLANAR_LUA_SOURCE_DIR}/src/lutf8lib.c")
target_link_libraries(planar_lua_trusted_libs PUBLIC planar_lua)
planar_embed_lua(planar_runtime_source planar.engine.runtime_source
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/json.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/schema.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/semantic.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/selection.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/presentation.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/observations.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/render.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/acceptance.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/commands.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/bootstrap.lua")
planar_rewrite_module(planar_closure_guard
  INTERFACE runtime/closure_guard/closure_guard.cppm SOURCES runtime/closure_guard/closure_guard.cpp
  DEPENDS planar_lua_trusted_libs TESTS runtime/closure_guard/closure_guard.t.cpp)

planar_rewrite_module(planar_runtime
  INTERFACE runtime/runtime.cppm SOURCES runtime/runtime.cpp
  DEPENDS planar_history_read planar_capture planar_capture_profile planar_history_effective_catalog planar_history_effective_provenance planar_closure_guard planar_ledger_completion planar_history_provenance_codec planar_hash planar_json_codec planar_lua_trusted_libs planar_runtime_source
  TESTS runtime/runtime.t.cpp runtime/schema.t.cpp runtime/commands.t.cpp runtime/external_registration.t.cpp)
planar_embed_lua(planar_runtime_test_source planar.engine.runtime_test_source
  "${PROJECT_SOURCE_DIR}/tests/stdlib/bridge.lua")
# Compile the exact bridge against test-only embedded exports, in its own module.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/runtime/runtime.cpp")
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/runtime/runtime.cpp" test_bridge)
string(REPLACE "module planar.engine.runtime;" "module planar.engine.runtime_test;" test_bridge "${test_bridge}")
string(REPLACE "import planar.engine.runtime_source;" "import planar.engine.runtime_test_source;" test_bridge "${test_bridge}")
string(REPLACE "namespace planar::engine {" "namespace planar::engine::runtime_test {" test_bridge "${test_bridge}")
file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/runtime_test.cpp" CONTENT "${test_bridge}" @ONLY)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/runtime/runtime.cppm" test_interface)
string(REPLACE "module planar.engine.runtime;" "module planar.engine.runtime_test;" test_interface "${test_interface}")
string(REPLACE "namespace planar::engine {" "namespace planar::engine::runtime_test {" test_interface "${test_interface}")
file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/runtime_test.cppm" CONTENT "${test_interface}" @ONLY)
planar_rewrite_module(planar_runtime_test_harness
  INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/runtime_test.cppm" SOURCES "${CMAKE_CURRENT_BINARY_DIR}/runtime_test.cpp"
  DEPENDS planar_history_read planar_capture planar_capture_profile planar_history_effective_catalog planar_history_effective_provenance planar_closure_guard planar_ledger_completion planar_history_provenance_codec planar_hash planar_json_codec planar_lua_trusted_libs planar_runtime_test_source)
set_target_properties(planar_runtime_test_harness planar_runtime_test_source PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_link_libraries(planar_runtime_tests PRIVATE planar_runtime_test_harness planar_lua_loader)

target_link_libraries(planar_runtime_tests PRIVATE planar_capture planar_admission_syscalls)
target_compile_definitions(planar_runtime_tests PRIVATE PLANAR_SCHEMA_FIXTURE="${PROJECT_SOURCE_DIR}/fixtures/render/quota")

# Test-only altered captured bundles compile the exact runtime and public interface.
set(v502b1_support "${PROJECT_SOURCE_DIR}/tests/native/v5-02-b1-support")
set(v502b1_generated "${CMAKE_CURRENT_BINARY_DIR}/v5-02-b1-generated")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/runtime/runtime.cppm"
  "${v502b1_support}/generate.py" "${v502b1_support}/altered.lua")
find_package(Python3 COMPONENTS Interpreter REQUIRED)
execute_process(COMMAND "${Python3_EXECUTABLE}" "${v502b1_support}/generate.py"
  "${PROJECT_SOURCE_DIR}" "${v502b1_generated}" COMMAND_ERROR_IS_FATAL ANY)
foreach(variant RANGE 0 20)
  planar_embed_lua(planar_command_source_${variant} planar.engine.command_source_${variant}
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/json.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/schema.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/semantic.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/selection.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/presentation.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/observations.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/render.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/acceptance.lua"
    "${v502b1_generated}/commands_${variant}.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/bootstrap.lua")
  # The altered file still occupies the exact shipped logical module slot.
  set(source_module "${CMAKE_CURRENT_BINARY_DIR}/planar_command_source_${variant}.cppm")
  file(READ "${source_module}" source_text)
  file(RELATIVE_PATH generated_logical "${PROJECT_SOURCE_DIR}" "${v502b1_generated}/commands_${variant}.lua")
  string(REPLACE "${generated_logical}" "planar/commands.lua" source_text "${source_text}")
  file(CONFIGURE OUTPUT "${source_module}" CONTENT "${source_text}" @ONLY)
  planar_rewrite_module(planar_command_variant_${variant}
    INTERFACE "${v502b1_generated}/runtime_${variant}.cppm" SOURCES "${v502b1_generated}/runtime_${variant}.cpp"
    DEPENDS planar_history_read planar_capture planar_capture_profile planar_history_effective_catalog planar_history_effective_provenance planar_closure_guard planar_ledger_completion planar_history_provenance_codec planar_hash planar_json_codec planar_lua_trusted_libs planar_command_source_${variant})
  set_target_properties(planar_command_variant_${variant} planar_command_source_${variant} PROPERTIES EXCLUDE_FROM_ALL TRUE)
  target_link_libraries(planar_runtime_tests PRIVATE planar_command_variant_${variant})
endforeach()

planar_rewrite_module(planar_ledger_sqlite
  INTERFACE ledger/sqlite.cppm SOURCES ledger/sqlite.cpp
  DEPENDS planar_sqlite TESTS ledger/sqlite.t.cpp ledger/reliability.t.cpp)

planar_rewrite_module(planar_ledger_types
  INTERFACE ledger/types.cppm DEPENDS planar_history_record planar_history_request_v2)

planar_rewrite_module(planar_ledger_storage_types
  INTERFACE ledger/storage_types.cppm
  DEPENDS planar_ledger_types planar_ledger_history_codec)

planar_rewrite_module(planar_ledger_storage
  INTERFACE ledger/storage.cppm SOURCES ledger/storage.cpp
  DEPENDS planar_ledger_storage_types planar_ledger_sqlite planar_json_codec planar_ledger_types planar_ledger_history_codec
    planar_history_tree_core planar_history_provenance_codec planar_history_interpretation_codec planar_history_recorded_interpretation planar_canonical
  TESTS ledger/storage.t.cpp)
# Test-only edge: the storage tests exercise the public format surface.
target_link_libraries(planar_ledger_storage_tests PRIVATE planar_ledger_format)

planar_rewrite_module(planar_acceptance_intent
  INTERFACE acceptance/intent.cppm SOURCES acceptance/intent.cpp
  DEPENDS planar_value planar_history_record planar_history_request_v2 TESTS acceptance/intent.t.cpp)

planar_rewrite_module(planar_ledger_acceptance
  INTERFACE ledger/acceptance.cppm SOURCES ledger/acceptance.cpp
  DEPENDS planar_ledger_storage planar_ledger_storage_types planar_acceptance_intent planar_acceptance_prepared planar_ledger_completion planar_ledger_types planar_ledger_history_codec)

planar_rewrite_module(planar_ledger_format
  INTERFACE ledger/format.cppm SOURCES ledger/format.cpp
  DEPENDS planar_ledger_completion planar_ledger_sqlite planar_json_codec planar_ledger_types planar_ledger_history_codec
    planar_ledger_storage planar_ledger_storage_types TESTS ledger/format.t.cpp ledger/snapshot.t.cpp ledger/history_verification.t.cpp ledger/acceptance.t.cpp)
# Test-only: fixtures prepare and complete acceptances; the format library itself links none of it.
target_link_libraries(planar_ledger_format_tests PRIVATE planar_ledger_acceptance planar_acceptance_prepared)
target_compile_definitions(planar_ledger_format_tests PRIVATE
  PLANAR_LEDGER_SCHEMA="${PROJECT_SOURCE_DIR}/tests/native/r12b-evidence/expected-schema.sql")

planar_rewrite_module(planar_history_tree_core
  INTERFACE history/tree_core.cppm SOURCES history/tree_core.cpp
  DEPENDS planar_ledger_types planar_value TESTS history/tree_core.t.cpp)

planar_rewrite_module(planar_history_tree
  INTERFACE history/tree.cppm SOURCES history/tree.cpp
  DEPENDS planar_ledger_format planar_history_tree_core TESTS history/tree.t.cpp)

# R12-C2 redirects only publication POSIX calls in an audited exact-source variant.
set(r12c2_support "${PROJECT_SOURCE_DIR}/tests/native/r12c2-support")
set(r12c2_generated "${CMAKE_CURRENT_BINARY_DIR}/r12c2-generated")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/ledger/format.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ledger/format.cppm"
  "${CMAKE_CURRENT_SOURCE_DIR}/ledger/storage.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ledger/storage.cppm"
  "${CMAKE_CURRENT_SOURCE_DIR}/ledger/acceptance.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ledger/acceptance.cppm"
  "${r12c2_support}/generate.py")
find_package(Python3 COMPONENTS Interpreter REQUIRED)
execute_process(COMMAND "${Python3_EXECUTABLE}" "${r12c2_support}/generate.py"
  "${CMAKE_CURRENT_SOURCE_DIR}/ledger" "${r12c2_generated}"
  COMMAND_ERROR_IS_FATAL ANY)
planar_rewrite_module(planar_ledger_posix_test
  INTERFACE "${r12c2_support}/posix.cppm" SOURCES "${r12c2_support}/posix.cpp")
planar_rewrite_module(planar_ledger_fault_test
  INTERFACE "${r12c2_generated}/format.cppm" SOURCES "${r12c2_generated}/format.cpp"
  DEPENDS planar_ledger_completion planar_ledger_sqlite planar_json_codec planar_ledger_types planar_ledger_history_codec
    planar_history_tree_core planar_history_provenance_codec planar_history_interpretation_codec planar_history_recorded_interpretation planar_canonical planar_ledger_storage planar_ledger_storage_types planar_ledger_posix_test
  TESTS ledger/publication_fault.t.cpp)
set_target_properties(planar_ledger_posix_test planar_ledger_fault_test PROPERTIES EXCLUDE_FROM_ALL TRUE)

# R13-B1 forces only record digests in a reversible verification-only format variant.
set(r13b1_support "${PROJECT_SOURCE_DIR}/tests/native/r13b1-support")
set(r13b1_generated "${CMAKE_CURRENT_BINARY_DIR}/r13b1-generated")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${r13b1_support}/generate.py")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${r13b1_support}/generate.py"
  "${CMAKE_CURRENT_SOURCE_DIR}/ledger" "${r13b1_generated}"
  COMMAND_ERROR_IS_FATAL ANY)
planar_rewrite_module(planar_record_digest_test
  INTERFACE "${r13b1_support}/digest.cppm" DEPENDS planar_history_record)
planar_rewrite_module(planar_record_storage_test
  INTERFACE "${r13b1_generated}/storage.cppm" SOURCES "${r13b1_generated}/storage.cpp"
  DEPENDS planar_ledger_storage_types planar_ledger_sqlite planar_json_codec planar_ledger_types planar_ledger_history_codec
    planar_history_tree_core planar_history_provenance_codec planar_history_interpretation_codec planar_history_recorded_interpretation planar_canonical planar_record_digest_test)
planar_rewrite_module(planar_record_format_test
  INTERFACE "${r13b1_generated}/format.cppm" SOURCES "${r13b1_generated}/format.cpp"
  DEPENDS planar_record_storage_test planar_ledger_storage_types planar_ledger_completion planar_ledger_sqlite planar_json_codec planar_ledger_types planar_ledger_history_codec
    planar_record_digest_test
  TESTS "${r13b1_support}/collision.t.cpp")
set_target_properties(planar_record_digest_test planar_record_storage_test planar_record_format_test PROPERTIES EXCLUDE_FROM_ALL TRUE)

planar_rewrite_module(planar_history_provenance_codec
  INTERFACE history/provenance_codec.cppm SOURCES history/provenance_codec.cpp
  DEPENDS planar_history_tree_core planar_canonical TESTS history/provenance_codec.t.cpp)

planar_rewrite_module(planar_history_interpretation_codec
  INTERFACE history/interpretation_codec.cppm SOURCES history/interpretation_codec.cpp
  DEPENDS planar_canonical TESTS history/interpretation_codec.t.cpp)
target_compile_definitions(planar_history_interpretation_codec_tests PRIVATE
  PLANAR_INTERPRETATION_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures")

planar_rewrite_module(planar_history_provenance
  INTERFACE history/provenance.cppm SOURCES history/provenance.cpp
  DEPENDS planar_history_tree planar_history_provenance_codec planar_runtime_source TESTS history/provenance.t.cpp)
target_compile_definitions(planar_history_provenance_tests PRIVATE
  PLANAR_STDLIB_DIR="${PROJECT_SOURCE_DIR}/src/stdlib")

planar_rewrite_module(planar_history_interpretation
  INTERFACE history/interpretation.cppm SOURCES history/interpretation.cpp
  DEPENDS planar_history_provenance planar_history_interpretation_codec planar_runtime TESTS history/interpretation.t.cpp)
target_link_libraries(planar_history_interpretation_tests PRIVATE planar_capture planar_json_codec)
target_compile_definitions(planar_history_interpretation_tests PRIVATE
  PLANAR_STDLIB_DIR="${PROJECT_SOURCE_DIR}/src/stdlib"
  PLANAR_INTERPRETATION_FIXTURE="${PROJECT_SOURCE_DIR}/fixtures/render/quota"
  PLANAR_INTERPRETATION_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures")

planar_rewrite_module(planar_ledger_history_codec
  INTERFACE ledger/history_codec.cppm SOURCES ledger/history_codec.cpp
  DEPENDS planar_json_codec TESTS ledger/history_codec.t.cpp)

# Higher wrappers are fixture preparation only; production format links pure validators.
target_link_libraries(planar_ledger_format_tests PRIVATE planar_history_provenance planar_history_request_v2 planar_history_read planar_source_export)
target_compile_definitions(planar_ledger_format_tests PRIVATE
  PLANAR_PREPARED_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures"
  PLANAR_HISTORY_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures"
  PLANAR_HISTORY_FIXTURE="${PROJECT_SOURCE_DIR}/fixtures/render/quota"
  PLANAR_V2_VECTORS="${PROJECT_SOURCE_DIR}/tests/vectors/interpretation-v2")

planar_rewrite_module(planar_acceptance_prepared
  INTERFACE acceptance/prepared.cppm SOURCES acceptance/prepared.cpp
  DEPENDS planar_capture_native planar_history_tree_core planar_history_provenance_codec
    planar_history_interpretation_codec planar_history_record planar_history_request_v2 planar_acceptance_intent TESTS acceptance/prepared.t.cpp)
target_link_libraries(planar_acceptance_prepared_tests PRIVATE planar_capture_host_path)
target_compile_definitions(planar_acceptance_prepared_tests PRIVATE
  PLANAR_PREPARED_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures")

# R14-C3-B2 selectively forces one record kind; every other hash and validator remains real.
set(r14c3b2_support "${PROJECT_SOURCE_DIR}/tests/native/r14c3b2-support")
set(r14c3b2_generated "${CMAKE_CURRENT_BINARY_DIR}/r14c3b2-generated")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${r14c3b2_support}/generate.py")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${r14c3b2_support}/generate.py"
  "${CMAKE_CURRENT_SOURCE_DIR}/ledger" "${r14c3b2_generated}"
  COMMAND_ERROR_IS_FATAL ANY)
planar_rewrite_module(planar_atomic_digest_test
  INTERFACE "${r14c3b2_support}/digest.cppm" DEPENDS planar_history_record planar_history_request_v2)
planar_rewrite_module(planar_atomic_storage_test
  INTERFACE "${r14c3b2_generated}/storage.cppm" SOURCES "${r14c3b2_generated}/storage.cpp"
  DEPENDS planar_ledger_storage_types planar_ledger_sqlite planar_json_codec planar_ledger_types planar_ledger_history_codec
    planar_history_tree_core planar_history_provenance_codec planar_history_interpretation_codec planar_history_recorded_interpretation planar_canonical planar_atomic_digest_test)
planar_rewrite_module(planar_atomic_acceptance_test
  INTERFACE "${r14c3b2_generated}/acceptance.cppm" SOURCES "${r14c3b2_generated}/acceptance.cpp"
  DEPENDS planar_atomic_storage_test planar_ledger_storage_types planar_acceptance_intent planar_acceptance_prepared planar_ledger_completion planar_ledger_types planar_ledger_history_codec)
planar_rewrite_module(planar_atomic_format_test
  INTERFACE "${r14c3b2_generated}/format.cppm" SOURCES "${r14c3b2_generated}/format.cpp"
  DEPENDS planar_atomic_storage_test planar_ledger_storage_types planar_ledger_completion planar_ledger_sqlite planar_json_codec planar_ledger_types planar_ledger_history_codec
    planar_atomic_digest_test
  TESTS "${r14c3b2_support}/atomic.t.cpp")
target_link_libraries(planar_atomic_format_test_tests PRIVATE planar_atomic_acceptance_test planar_atomic_storage_test planar_acceptance_prepared)
target_compile_definitions(planar_atomic_format_test_tests PRIVATE
  PLANAR_PREPARED_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures")
set_target_properties(planar_atomic_digest_test planar_atomic_storage_test planar_atomic_acceptance_test planar_atomic_format_test PROPERTIES EXCLUDE_FROM_ALL TRUE)

planar_rewrite_module(planar_history_read
  INTERFACE history/read.cppm SOURCES history/read.cpp
  DEPENDS planar_ledger_format planar_history_interpretation_codec planar_history_recorded_interpretation planar_history_tree planar_json_codec
  TESTS history/read.t.cpp)

add_executable(planar_history_fixture EXCLUDE_FROM_ALL
  "${PROJECT_SOURCE_DIR}/tests/native/r14e2-support/fixture.cpp")
planar_native_target(planar_history_fixture)
target_link_libraries(planar_history_fixture PRIVATE planar_ledger_format planar_ledger_acceptance planar_acceptance_prepared planar_history_provenance_codec planar_canonical planar_sqlite)
target_compile_definitions(planar_history_fixture PRIVATE
  PLANAR_PREPARED_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures")
add_dependencies(planar_tests planar_history_fixture)

planar_rewrite_module(planar_source_export
  INTERFACE export/source_export.cppm SOURCES export/source_export.cpp
  DEPENDS planar_history_tree TESTS export/source_export.t.cpp TEST_TIMEOUT 120)
target_link_libraries(planar_source_export_tests PRIVATE planar_ledger_acceptance planar_acceptance_prepared planar_history_provenance_codec planar_canonical)
target_compile_definitions(planar_source_export_tests PRIVATE
  PLANAR_PREPARED_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures"
  PLANAR_EXPORT_FIXTURE="$<TARGET_FILE:planar_history_fixture>")

# R14-F1 redirects only the exact export source at native filesystem boundaries.
set(r14f1_support "${PROJECT_SOURCE_DIR}/tests/native/r14f1-support")
set(r14f1_generated "${CMAKE_CURRENT_BINARY_DIR}/r14f1-generated")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/export/source_export.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/export/source_export.cppm"
  "${r14f1_support}/generate.py")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${r14f1_support}/generate.py"
  "${CMAKE_CURRENT_SOURCE_DIR}/export" "${r14f1_generated}"
  COMMAND_ERROR_IS_FATAL ANY)
planar_rewrite_module(planar_export_posix_test
  INTERFACE "${r14f1_support}/posix.cppm" SOURCES "${r14f1_support}/posix.cpp")
planar_rewrite_module(planar_export_fault_test
  INTERFACE "${r14f1_generated}/source_export.cppm" SOURCES "${r14f1_generated}/source_export.cpp"
  DEPENDS planar_history_tree planar_export_posix_test TESTS export/source_export.t.cpp)
target_compile_definitions(planar_export_fault_test_tests PRIVATE PLANAR_EXPORT_FAULT_TEST
  PLANAR_PREPARED_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures")
target_link_libraries(planar_export_fault_test_tests PRIVATE planar_ledger_acceptance planar_acceptance_prepared planar_history_provenance_codec planar_canonical)
set_target_properties(planar_export_posix_test planar_export_fault_test PROPERTIES EXCLUDE_FROM_ALL TRUE)

planar_rewrite_module(planar_acceptance_load
  INTERFACE acceptance/load.cppm SOURCES acceptance/load.cpp
  DEPENDS planar_acceptance_prepared planar_capture planar_capture_host_path planar_canonical
  TESTS acceptance/load.t.cpp)
target_link_libraries(planar_acceptance_load_tests PRIVATE planar_runtime planar_history_provenance planar_json_codec)
target_compile_definitions(planar_acceptance_load_tests PRIVATE
  PLANAR_LOAD_FIXTURE="${PROJECT_SOURCE_DIR}/fixtures/render/quota")

# D6 private admission; production keeps exactly four mandatory entries.
set(r14d6_support "${PROJECT_SOURCE_DIR}/tests/native/r14-d6-support")
set(r14d6_generated "${CMAKE_CURRENT_BINARY_DIR}/r14-d6-generated")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${r14d6_support}/generate.py" "${r14d6_support}/load.lua")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${r14d6_support}/generate.py"
  "${PROJECT_SOURCE_DIR}" "${r14d6_generated}" COMMAND_ERROR_IS_FATAL ANY)
foreach(variant RANGE 0 5)
  planar_embed_lua(planar_load_source_${variant} planar.engine.load_source_${variant}
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/json.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/schema.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/semantic.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/selection.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/presentation.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/observations.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/render.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/acceptance.lua"
    "${r14d6_generated}/commands_${variant}.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/bootstrap.lua")
  set(source_module "${CMAKE_CURRENT_BINARY_DIR}/planar_load_source_${variant}.cppm")
  file(READ "${source_module}" source_text)
  file(RELATIVE_PATH generated_logical "${PROJECT_SOURCE_DIR}" "${r14d6_generated}/commands_${variant}.lua")
  string(REPLACE "${generated_logical}" "planar/commands.lua" source_text "${source_text}")
  file(CONFIGURE OUTPUT "${source_module}" CONTENT "${source_text}" @ONLY)
  planar_rewrite_module(planar_load_variant_${variant}
    INTERFACE "${r14d6_generated}/runtime_${variant}.cppm" SOURCES "${r14d6_generated}/runtime_${variant}.cpp"
    DEPENDS planar_history_read planar_capture planar_capture_profile planar_history_effective_catalog planar_history_effective_provenance planar_closure_guard planar_ledger_completion planar_history_provenance_codec planar_hash planar_json_codec planar_lua_trusted_libs planar_load_source_${variant})
  set_target_properties(planar_load_variant_${variant} planar_load_source_${variant} PROPERTIES EXCLUDE_FROM_ALL TRUE)
  target_link_libraries(planar_runtime_tests PRIVATE planar_load_variant_${variant})
endforeach()
target_link_libraries(planar_runtime_tests PRIVATE planar_acceptance_load)
target_compile_definitions(planar_runtime_tests PRIVATE
  PLANAR_LOAD_FIXTURE="${PROJECT_SOURCE_DIR}/fixtures/render/quota"
  PLANAR_LOAD_GENERATED="${r14d6_generated}")

# C01 provenance-only defect: exact bytes, non-command logical path is invalid.
set(r14d6c01_support "${PROJECT_SOURCE_DIR}/tests/native/r14-d6-c01-support")
set(r14d6c01_generated "${CMAKE_CURRENT_BINARY_DIR}/r14-d6-c01-generated")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${r14d6c01_support}/generate.py")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${r14d6c01_support}/generate.py"
  "${PROJECT_SOURCE_DIR}" "${r14d6c01_generated}" COMMAND_ERROR_IS_FATAL ANY)
foreach(variant RANGE 0 0)
  planar_embed_lua(planar_load_c01_source planar.engine.load_c01_source
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/json.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/schema.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/semantic.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/selection.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/presentation.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/observations.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/render.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/acceptance.lua"
    "${r14d6c01_generated}/commands_${variant}.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/bootstrap.lua")
  set(source_module "${CMAKE_CURRENT_BINARY_DIR}/planar_load_c01_source.cppm")
  file(READ "${source_module}" source_text)
  file(RELATIVE_PATH generated_logical "${PROJECT_SOURCE_DIR}" "${r14d6c01_generated}/commands_${variant}.lua")
  string(REPLACE "${generated_logical}" "planar/commands.lua" source_text "${source_text}")
  string(REPLACE "\"planar/json.lua\"" "\"../json.lua\"" source_text "${source_text}")
  file(CONFIGURE OUTPUT "${source_module}" CONTENT "${source_text}" @ONLY)
  planar_rewrite_module(planar_load_c01_variant
    INTERFACE "${r14d6c01_generated}/runtime_${variant}.cppm" SOURCES "${r14d6c01_generated}/runtime_${variant}.cpp"
    DEPENDS planar_history_read planar_capture planar_capture_profile planar_history_effective_catalog planar_history_effective_provenance planar_closure_guard planar_ledger_completion planar_history_provenance_codec planar_hash planar_json_codec planar_lua_trusted_libs planar_load_c01_source)
  set_target_properties(planar_load_c01_variant planar_load_c01_source PROPERTIES EXCLUDE_FROM_ALL TRUE)
  target_link_libraries(planar_runtime_tests PRIVATE planar_load_c01_variant)
endforeach()

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
  "${CMAKE_CURRENT_SOURCE_DIR}/ledger" "${x03b2bc01_generated}"
  COMMAND_ERROR_IS_FATAL ANY)
planar_rewrite_module(planar_metadata_fixture
  INTERFACE "${x03b2bc01_support}/fixture.cppm")
planar_rewrite_module(planar_metadata_storage_test
  INTERFACE "${x03b2bc01_generated}/storage.cppm" SOURCES "${x03b2bc01_generated}/storage.cpp"
  DEPENDS planar_ledger_storage_types planar_ledger_sqlite planar_json_codec planar_ledger_types planar_ledger_history_codec
    planar_history_tree_core planar_history_provenance_codec planar_history_interpretation_codec planar_history_recorded_interpretation planar_canonical planar_metadata_fixture)
planar_rewrite_module(planar_metadata_acceptance_test
  INTERFACE "${x03b2bc01_generated}/acceptance.cppm" SOURCES "${x03b2bc01_generated}/acceptance.cpp"
  DEPENDS planar_metadata_storage_test planar_ledger_storage_types planar_acceptance_intent planar_acceptance_prepared planar_ledger_completion planar_ledger_types planar_ledger_history_codec)
planar_rewrite_module(planar_metadata_format_test
  INTERFACE "${x03b2bc01_generated}/format.cppm" SOURCES "${x03b2bc01_generated}/format.cpp"
  DEPENDS planar_metadata_storage_test planar_ledger_storage_types planar_ledger_completion planar_ledger_sqlite planar_json_codec planar_ledger_types planar_ledger_history_codec
    planar_metadata_fixture
  TESTS "${x03b2bc01_support}/metadata.t.cpp")
target_link_libraries(planar_metadata_format_test_tests PRIVATE planar_metadata_acceptance_test planar_metadata_storage_test planar_acceptance_prepared)
target_compile_definitions(planar_metadata_format_test_tests PRIVATE
  PLANAR_PREPARED_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures")
set_target_properties(planar_metadata_fixture planar_metadata_storage_test planar_metadata_acceptance_test planar_metadata_format_test PROPERTIES EXCLUDE_FROM_ALL TRUE)

planar_rewrite_module(planar_history_effective_provenance
  INTERFACE history/effective_provenance.cppm SOURCES history/effective_provenance.cpp
  DEPENDS planar_history_provenance_codec planar_capture_profile_codec planar_history_effective_catalog planar_canonical
  TESTS history/effective_provenance.t.cpp)
target_compile_definitions(planar_history_effective_provenance_tests PRIVATE
  PLANAR_EFFECTIVE_VECTORS="${PROJECT_SOURCE_DIR}/tests/vectors/effective-provenance")

# X05-A0 is verification-only: pinned metadata audit and reversible runtime variants.
set(x05a0_support "${PROJECT_SOURCE_DIR}/tests/native/v5-02-x05-a0-support")
set(x05a0_generated "${CMAKE_CURRENT_BINARY_DIR}/v5-02-x05-a0-generated")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${x05a0_support}/generate.py")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${x05a0_support}/generate.py"
  "${PROJECT_SOURCE_DIR}" "${x05a0_generated}" COMMAND_ERROR_IS_FATAL ANY)
planar_rewrite_module(planar_x05_closure_audit
  INTERFACE "${x05a0_support}/audit.cppm" SOURCES "${x05a0_support}/audit.cpp"
  DEPENDS planar_lua_trusted_libs TESTS "${x05a0_support}/audit.t.cpp" "${x05a0_support}/shipped.t.cpp")
set_target_properties(planar_x05_closure_audit PROPERTIES EXCLUDE_FROM_ALL TRUE)
foreach(variant RANGE 0 1)
  if(variant EQUAL 0)
    set(x05a0_commands "${PROJECT_SOURCE_DIR}/src/stdlib/planar/commands.lua")
  else()
    set(x05a0_commands "${x05a0_generated}/commands_${variant}.lua")
  endif()
  planar_embed_lua(planar_x05_source_${variant} planar.engine.x05_a0_${variant}_source
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/json.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/schema.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/semantic.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/selection.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/presentation.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/observations.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/render.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/acceptance.lua"
    "${x05a0_commands}"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/bootstrap.lua")
  if(variant EQUAL 1)
    set(source_module "${CMAKE_CURRENT_BINARY_DIR}/planar_x05_source_${variant}.cppm")
    file(READ "${source_module}" source_text)
    file(RELATIVE_PATH generated_logical "${PROJECT_SOURCE_DIR}" "${x05a0_commands}")
    string(REPLACE "${generated_logical}" "planar/commands.lua" source_text "${source_text}")
    file(CONFIGURE OUTPUT "${source_module}" CONTENT "${source_text}" @ONLY)
  endif()
  planar_rewrite_module(planar_x05_runtime_${variant}
    INTERFACE "${x05a0_generated}/x05_a0_${variant}_runtime.cppm" SOURCES "${x05a0_generated}/x05_a0_${variant}_runtime.cpp"
    DEPENDS planar_history_read planar_capture planar_capture_profile planar_history_effective_catalog planar_history_effective_provenance planar_closure_guard planar_ledger_completion planar_history_provenance_codec planar_hash planar_json_codec planar_lua_trusted_libs planar_x05_source_${variant} planar_x05_closure_audit)
  set_target_properties(planar_x05_runtime_${variant} planar_x05_source_${variant} PROPERTIES EXCLUDE_FROM_ALL TRUE)
  target_link_libraries(planar_x05_closure_audit_tests PRIVATE planar_x05_runtime_${variant})
endforeach()

# X05-B1 pure-helper seam: exact production runtime, distinct test-only exports.
planar_embed_lua(planar_shapes_test_source planar.engine.shapes_test_source
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/json.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/schema.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/semantic.lua"
  "${PROJECT_SOURCE_DIR}/tests/stdlib/x05-shapes.lua")
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/runtime/runtime.cpp" shapes_bridge)
string(REPLACE "module planar.engine.runtime;" "module planar.engine.shapes_test;" shapes_bridge "${shapes_bridge}")
string(REPLACE "import planar.engine.runtime_source;" "import planar.engine.shapes_test_source;" shapes_bridge "${shapes_bridge}")
string(REPLACE "namespace planar::engine {" "namespace planar::engine::shapes_test {" shapes_bridge "${shapes_bridge}")
file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shapes_test.cpp" CONTENT "${shapes_bridge}" @ONLY)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/runtime/runtime.cppm" shapes_interface)
string(REPLACE "module planar.engine.runtime;" "module planar.engine.shapes_test;" shapes_interface "${shapes_interface}")
string(REPLACE "namespace planar::engine {" "namespace planar::engine::shapes_test {" shapes_interface "${shapes_interface}")
file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shapes_test.cppm" CONTENT "${shapes_interface}" @ONLY)
planar_rewrite_module(planar_shapes_test_harness
  INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/shapes_test.cppm" SOURCES "${CMAKE_CURRENT_BINARY_DIR}/shapes_test.cpp"
  DEPENDS planar_history_read planar_capture planar_capture_profile planar_history_effective_catalog planar_history_effective_provenance planar_closure_guard planar_ledger_completion planar_history_provenance_codec planar_hash planar_json_codec planar_lua_trusted_libs planar_shapes_test_source)
set_target_properties(planar_shapes_test_harness planar_shapes_test_source PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_sources(planar_runtime_tests PRIVATE runtime/shapes.t.cpp)
target_link_libraries(planar_runtime_tests PRIVATE planar_shapes_test_harness)

planar_rewrite_module(planar_history_interpretation_v2_codec
  INTERFACE history/interpretation_v2_codec.cppm SOURCES history/interpretation_v2_codec.cpp
  DEPENDS planar_history_effective_provenance planar_history_effective_catalog planar_hash
  TESTS history/interpretation_v2_codec.t.cpp)
target_compile_definitions(planar_history_interpretation_v2_codec_tests PRIVATE
  PLANAR_V2_VECTORS="${PROJECT_SOURCE_DIR}/tests/vectors/interpretation-v2"
  PLANAR_EFFECTIVE_VECTORS="${PROJECT_SOURCE_DIR}/tests/vectors/effective-provenance"
  PLANAR_V1_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures")

planar_rewrite_module(planar_history_recorded_interpretation
  INTERFACE history/recorded_interpretation.cppm SOURCES history/recorded_interpretation.cpp
  DEPENDS planar_history_interpretation_codec planar_history_interpretation_v2_codec planar_canonical
  TESTS history/recorded_interpretation.t.cpp TEST_TIMEOUT 120)
target_compile_definitions(planar_history_recorded_interpretation_tests PRIVATE
  PLANAR_V2_VECTORS="${PROJECT_SOURCE_DIR}/tests/vectors/interpretation-v2"
  PLANAR_V1_GOLDENS="${PROJECT_SOURCE_DIR}/tests/native/r13c2-fixtures")
