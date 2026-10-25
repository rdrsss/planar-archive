planar_rewrite_module(planar_command_response
  INTERFACE response.cppm
  SOURCES response.cpp
  DEPENDS planar_json_codec planar_ledger_types)
planar_rewrite_module(planar_command_invocation
  INTERFACE invocation.cppm SOURCES invocation.cpp
  DEPENDS planar_command_response planar_runtime planar_capture planar_capture_host_path planar_ledger_format planar_ledger_acceptance planar_history_read planar_source_export planar_acceptance_load)
planar_rewrite_module(planar_render_command
  INTERFACE handlers/render/command.cppm
  SOURCES handlers/render/command.cpp
  DEPENDS planar_command_invocation CLI11::CLI11_Module)
planar_rewrite_module(planar_init_command
  INTERFACE handlers/init/command.cppm
  SOURCES handlers/init/command.cpp
  DEPENDS planar_command_invocation CLI11::CLI11_Module)
planar_rewrite_module(planar_history_command
  INTERFACE handlers/history/command.cppm SOURCES handlers/history/command.cpp
  DEPENDS planar_command_invocation CLI11::CLI11_Module)
planar_rewrite_module(planar_export_command
  INTERFACE handlers/export/command.cppm SOURCES handlers/export/command.cpp
  DEPENDS planar_command_invocation CLI11::CLI11_Module)
planar_rewrite_module(planar_load_command
  INTERFACE handlers/load/command.cppm SOURCES handlers/load/command.cpp
  DEPENDS planar_command_invocation CLI11::CLI11_Module)
planar_rewrite_module(planar_command
  INTERFACE bootstrap.cppm
  SOURCES bootstrap.cpp
  DEPENDS planar_command_response planar_render_command planar_init_command planar_history_command planar_export_command planar_load_command CLI11::CLI11_Module
  TESTS bootstrap.t.cpp)
target_compile_definitions(planar_command_tests PRIVATE
  PLANAR_B2_WORKSPACE_FIXTURE="${PROJECT_SOURCE_DIR}/fixtures/render/quota/workspace/workspace.lua")
add_executable(planar main.cpp)
planar_native_target(planar)
target_link_libraries(planar PRIVATE planar_command)
set_target_properties(planar PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

# Test-only reversible command variant; production native export stays unchanged.
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(r14f2_support "${PROJECT_SOURCE_DIR}/tests/native/r14f2-support")
set(r14f2_generated "${CMAKE_CURRENT_BINARY_DIR}/r14f2-generated")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  invocation.cppm invocation.cpp "${r14f2_support}/generate.py")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${r14f2_support}/generate.py"
  "${CMAKE_CURRENT_SOURCE_DIR}" "${r14f2_generated}"
  COMMAND_ERROR_IS_FATAL ANY)
planar_rewrite_module(planar_export_response_fault
  INTERFACE "${r14f2_generated}/invocation.cppm" SOURCES "${r14f2_generated}/invocation.cpp"
  DEPENDS planar_command_invocation CLI11::CLI11_Module
  TESTS "${r14f2_support}/boundary.t.cpp")
target_compile_definitions(planar_export_response_fault_tests PRIVATE
  PLANAR_HISTORY_FIXTURE="$<TARGET_FILE:planar_history_fixture>")
add_dependencies(planar_export_response_fault_tests planar_history_fixture)

# Test-only bundles run the same parser, family bindings, native services and process I/O.
set(v502b2_support "${PROJECT_SOURCE_DIR}/tests/native/v5-02-b2-support")
set(v502b2_generated "${CMAKE_CURRENT_BINARY_DIR}/v5-02-b2-generated")
file(GLOB_RECURSE v502b2_command_inputs CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.cppm")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${v502b2_command_inputs}
  "${v502b2_support}/generate.py" "${PROJECT_SOURCE_DIR}/src/stdlib/planar/commands.lua")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${v502b2_support}/generate.py"
  "${PROJECT_SOURCE_DIR}" "${v502b2_generated}" COMMAND_ERROR_IS_FATAL ANY)
foreach(variant RANGE 0 5)
  set(generated "${v502b2_generated}/${variant}")
  planar_embed_lua(planar_cli_bundle_source_${variant} planar.engine.cli_bundle_source_${variant}
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/json.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/schema.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/semantic.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/selection.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/presentation.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/observations.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/render.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/acceptance.lua"
    "${generated}/commands.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/bootstrap.lua")
  set(source_module "${CMAKE_CURRENT_BINARY_DIR}/planar_cli_bundle_source_${variant}.cppm")
  file(READ "${source_module}" source_text)
  file(RELATIVE_PATH generated_logical "${PROJECT_SOURCE_DIR}" "${generated}/commands.lua")
  string(REPLACE "${generated_logical}" "planar/commands.lua" source_text "${source_text}")
  file(CONFIGURE OUTPUT "${source_module}" CONTENT "${source_text}" @ONLY)
  planar_rewrite_module(planar_cli_runtime_${variant}
    INTERFACE "${generated}/runtime.cppm" SOURCES "${generated}/runtime.cpp"
    DEPENDS planar_history_read planar_capture planar_capture_profile planar_history_effective_catalog planar_history_effective_provenance planar_closure_guard planar_ledger_completion planar_history_provenance_codec planar_hash planar_json_codec planar_lua_trusted_libs planar_cli_bundle_source_${variant})
  planar_rewrite_module(planar_cli_invocation_${variant}
    INTERFACE "${generated}/invocation.cppm" SOURCES "${generated}/invocation.cpp"
    DEPENDS planar_command_response planar_cli_runtime_${variant} planar_capture planar_capture_host_path planar_ledger_format planar_ledger_acceptance planar_history_read planar_source_export planar_acceptance_load)
  foreach(family render init history export load)
    planar_rewrite_module(planar_cli_${family}_${variant}
      INTERFACE "${generated}/handlers/${family}/command.cppm" SOURCES "${generated}/handlers/${family}/command.cpp"
      DEPENDS planar_cli_invocation_${variant} CLI11::CLI11_Module)
    set_target_properties(planar_cli_${family}_${variant} PROPERTIES EXCLUDE_FROM_ALL TRUE)
  endforeach()
  planar_rewrite_module(planar_cli_bootstrap_${variant}
    INTERFACE "${generated}/bootstrap.cppm" SOURCES "${generated}/bootstrap.cpp"
    DEPENDS planar_cli_render_${variant} planar_cli_init_${variant} planar_cli_history_${variant} planar_cli_export_${variant} planar_cli_load_${variant})
  add_executable(planar_cli_variant_${variant} EXCLUDE_FROM_ALL "${generated}/main.cpp")
  planar_native_target(planar_cli_variant_${variant})
  target_link_libraries(planar_cli_variant_${variant} PRIVATE planar_cli_bootstrap_${variant})
  add_dependencies(planar_tests planar_cli_variant_${variant})
  set_target_properties(planar_cli_runtime_${variant} planar_cli_bundle_source_${variant} planar_cli_invocation_${variant} planar_cli_bootstrap_${variant} PROPERTIES EXCLUDE_FROM_ALL TRUE)
endforeach()
add_test(NAME replacement.retained_cli COMMAND "${Python3_EXECUTABLE}" "${v502b2_support}/cli.py"
  "$<TARGET_FILE:planar>" "$<TARGET_FILE:planar_history_fixture>"
  "$<TARGET_FILE:planar_cli_variant_0>" "$<TARGET_FILE:planar_cli_variant_1>"
  "$<TARGET_FILE:planar_cli_variant_2>" "$<TARGET_FILE:planar_cli_variant_3>" "$<TARGET_FILE:planar_cli_variant_4>" "$<TARGET_FILE:planar_cli_variant_5>")
set_tests_properties(replacement.retained_cli PROPERTIES TIMEOUT 120 LABELS acceptance)

# Explicit workspace-name acceptance uses only scratch directories and the candidate binary.
add_test(NAME replacement.workspace_selector_cli COMMAND "${Python3_EXECUTABLE}"
  "${PROJECT_SOURCE_DIR}/tests/native/v5-02-c-support/cli.py" "$<TARGET_FILE:planar>"
  "$<TARGET_FILE:planar_cli_variant_1>" "$<TARGET_FILE:planar_cli_variant_2>")
set_tests_properties(replacement.workspace_selector_cli PROPERTIES TIMEOUT 120 LABELS acceptance)

# Exact-source isolated load transport/race probes; never linked into production.
set(d7b_support "${PROJECT_SOURCE_DIR}/tests/native/r14-d7-b-support")
set(d7b_generated "${CMAKE_CURRENT_BINARY_DIR}/r14-d7-b-generated")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  ${v502b2_command_inputs} "${d7b_support}/generate.py" "${PROJECT_SOURCE_DIR}/src/stdlib/planar/commands.lua")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${d7b_support}/generate.py"
  "${PROJECT_SOURCE_DIR}" "${d7b_generated}" COMMAND_ERROR_IS_FATAL ANY)
planar_embed_lua(planar_load_cli_probe_source planar.engine.load_cli_probe_source
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/json.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/schema.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/semantic.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/selection.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/presentation.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/observations.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/render.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/acceptance.lua"
  "${d7b_generated}/commands.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/bootstrap.lua")
set(d7b_source_module "${CMAKE_CURRENT_BINARY_DIR}/planar_load_cli_probe_source.cppm")
file(READ "${d7b_source_module}" d7b_source_text)
file(RELATIVE_PATH d7b_logical "${PROJECT_SOURCE_DIR}" "${d7b_generated}/commands.lua")
string(REPLACE "${d7b_logical}" "planar/commands.lua" d7b_source_text "${d7b_source_text}")
file(CONFIGURE OUTPUT "${d7b_source_module}" CONTENT "${d7b_source_text}" @ONLY)
planar_rewrite_module(planar_load_cli_probe_runtime
  INTERFACE "${d7b_generated}/runtime.cppm" SOURCES "${d7b_generated}/runtime.cpp"
  DEPENDS planar_history_read planar_capture planar_capture_profile planar_history_effective_catalog planar_history_effective_provenance planar_closure_guard planar_ledger_completion planar_history_provenance_codec planar_hash planar_json_codec planar_lua_trusted_libs planar_load_cli_probe_source)
planar_rewrite_module(planar_load_cli_probe_invocation
  INTERFACE "${d7b_generated}/invocation.cppm" SOURCES "${d7b_generated}/invocation.cpp"
  DEPENDS planar_command_response planar_load_cli_probe_runtime planar_capture planar_capture_host_path planar_ledger_format planar_ledger_acceptance planar_history_read planar_source_export planar_acceptance_load)
foreach(family render init history export load)
  planar_rewrite_module(planar_load_cli_probe_${family}
    INTERFACE "${d7b_generated}/handlers/${family}/command.cppm" SOURCES "${d7b_generated}/handlers/${family}/command.cpp"
    DEPENDS planar_load_cli_probe_invocation CLI11::CLI11_Module)
  set_target_properties(planar_load_cli_probe_${family} PROPERTIES EXCLUDE_FROM_ALL TRUE)
endforeach()
planar_rewrite_module(planar_load_cli_probe_bootstrap
  INTERFACE "${d7b_generated}/bootstrap.cppm" SOURCES "${d7b_generated}/bootstrap.cpp"
  DEPENDS planar_load_cli_probe_render planar_load_cli_probe_init planar_load_cli_probe_history planar_load_cli_probe_export planar_load_cli_probe_load)
add_executable(planar_load_cli_probe EXCLUDE_FROM_ALL "${d7b_generated}/main.cpp")
planar_native_target(planar_load_cli_probe)
target_link_libraries(planar_load_cli_probe PRIVATE planar_load_cli_probe_bootstrap)
add_dependencies(planar_tests planar_load_cli_probe)
set_target_properties(planar_load_cli_probe_runtime planar_load_cli_probe_source planar_load_cli_probe_invocation planar_load_cli_probe_bootstrap PROPERTIES EXCLUDE_FROM_ALL TRUE)
add_test(NAME replacement.load_cli COMMAND "${Python3_EXECUTABLE}"
  "${PROJECT_SOURCE_DIR}/tests/acceptance/bootstrap_load.py" "$<TARGET_FILE:planar>" "$<TARGET_FILE:planar_load_cli_probe>")
set_tests_properties(replacement.load_cli PROPERTIES TIMEOUT 240 LABELS acceptance)
