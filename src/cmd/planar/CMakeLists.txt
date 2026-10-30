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
planar_rewrite_module(planar_command_envelope
  INTERFACE envelope.cppm
  SOURCES envelope.cpp
  DEPENDS planar_json_codec CLI11::CLI11_Module)
planar_rewrite_module(planar_extension_command
  INTERFACE handlers/extension/command.cppm SOURCES handlers/extension/command.cpp
  DEPENDS planar_command_invocation)
planar_rewrite_module(planar_command
  INTERFACE bootstrap.cppm
  SOURCES bootstrap.cpp
  DEPENDS planar_command_response planar_command_envelope planar_extension_command planar_render_command planar_init_command planar_history_command planar_export_command planar_load_command CLI11::CLI11_Module
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
    DEPENDS planar_history_read planar_acceptance_source_capture planar_acceptance_intent planar_capture planar_capture_profile planar_history_effective_catalog planar_history_effective_provenance planar_history_interpretation_v2_codec planar_closure_guard planar_ledger_completion planar_history_provenance_codec planar_hash planar_json_codec planar_lua_trusted_libs planar_cli_bundle_source_${variant})
  planar_rewrite_module(planar_cli_invocation_${variant}
    INTERFACE "${generated}/invocation.cppm" SOURCES "${generated}/invocation.cpp"
    DEPENDS planar_command_response planar_cli_runtime_${variant} planar_capture planar_capture_host_path planar_ledger_format planar_ledger_acceptance planar_history_read planar_source_export planar_acceptance_load)
  foreach(family render init history export load extension)
    planar_rewrite_module(planar_cli_${family}_${variant}
      INTERFACE "${generated}/handlers/${family}/command.cppm" SOURCES "${generated}/handlers/${family}/command.cpp"
      DEPENDS planar_cli_invocation_${variant} CLI11::CLI11_Module)
    set_target_properties(planar_cli_${family}_${variant} PROPERTIES EXCLUDE_FROM_ALL TRUE)
  endforeach()
  planar_rewrite_module(planar_cli_bootstrap_${variant}
    INTERFACE "${generated}/bootstrap.cppm" SOURCES "${generated}/bootstrap.cpp"
    DEPENDS planar_command_envelope planar_cli_render_${variant} planar_cli_init_${variant} planar_cli_history_${variant} planar_cli_export_${variant} planar_cli_load_${variant} planar_cli_extension_${variant})
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
  DEPENDS planar_history_read planar_acceptance_source_capture planar_acceptance_intent planar_capture planar_capture_profile planar_history_effective_catalog planar_history_effective_provenance planar_history_interpretation_v2_codec planar_closure_guard planar_ledger_completion planar_history_provenance_codec planar_hash planar_json_codec planar_lua_trusted_libs planar_load_cli_probe_source)
planar_rewrite_module(planar_load_cli_probe_invocation
  INTERFACE "${d7b_generated}/invocation.cppm" SOURCES "${d7b_generated}/invocation.cpp"
  DEPENDS planar_command_response planar_load_cli_probe_runtime planar_capture planar_capture_host_path planar_ledger_format planar_ledger_acceptance planar_history_read planar_source_export planar_acceptance_load)
foreach(family render init history export load extension)
  planar_rewrite_module(planar_load_cli_probe_${family}
    INTERFACE "${d7b_generated}/handlers/${family}/command.cppm" SOURCES "${d7b_generated}/handlers/${family}/command.cpp"
    DEPENDS planar_load_cli_probe_invocation CLI11::CLI11_Module)
  set_target_properties(planar_load_cli_probe_${family} PROPERTIES EXCLUDE_FROM_ALL TRUE)
endforeach()
planar_rewrite_module(planar_load_cli_probe_bootstrap
  INTERFACE "${d7b_generated}/bootstrap.cppm" SOURCES "${d7b_generated}/bootstrap.cpp"
  DEPENDS planar_command_envelope planar_load_cli_probe_render planar_load_cli_probe_init planar_load_cli_probe_history planar_load_cli_probe_export planar_load_cli_probe_load planar_load_cli_probe_extension)
add_executable(planar_load_cli_probe EXCLUDE_FROM_ALL "${d7b_generated}/main.cpp")
planar_native_target(planar_load_cli_probe)
target_link_libraries(planar_load_cli_probe PRIVATE planar_load_cli_probe_bootstrap)
add_dependencies(planar_tests planar_load_cli_probe)
set_target_properties(planar_load_cli_probe_runtime planar_load_cli_probe_source planar_load_cli_probe_invocation planar_load_cli_probe_bootstrap PROPERTIES EXCLUDE_FROM_ALL TRUE)
add_test(NAME replacement.load_cli COMMAND "${Python3_EXECUTABLE}"
  "${PROJECT_SOURCE_DIR}/tests/acceptance/bootstrap_load.py" "$<TARGET_FILE:planar>" "$<TARGET_FILE:planar_load_cli_probe>")
set_tests_properties(replacement.load_cli PROPERTIES TIMEOUT 240 LABELS acceptance)

# X07 substitution variants: the embedded init, history or export handler is replaced by a distinguishable result and the
# real process is driven as with-profile, proving the sealed shipped callable (not native code) produces the response.
set(x07_support "${PROJECT_SOURCE_DIR}/tests/native/v5-02-x07-support")
set(x07_generated "${CMAKE_CURRENT_BINARY_DIR}/v5-02-x07-generated")
file(GLOB_RECURSE x07_command_inputs CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.cppm")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${x07_command_inputs}
  "${x07_support}/generate.py" "${PROJECT_SOURCE_DIR}/src/stdlib/planar/commands.lua")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${x07_support}/generate.py"
  "${PROJECT_SOURCE_DIR}" "${x07_generated}" COMMAND_ERROR_IS_FATAL ANY)
# Variants 0 to 2 substitute handlers; 3 has env-driven pause and fault points; 4 has a tiny response ceiling (DC56).
foreach(variant RANGE 0 4)
  set(generated "${x07_generated}/${variant}")
  planar_embed_lua(planar_x07_bundle_source_${variant} planar.engine.x07_bundle_source_${variant}
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
  set(source_module "${CMAKE_CURRENT_BINARY_DIR}/planar_x07_bundle_source_${variant}.cppm")
  file(READ "${source_module}" source_text)
  file(RELATIVE_PATH generated_logical "${PROJECT_SOURCE_DIR}" "${generated}/commands.lua")
  string(REPLACE "${generated_logical}" "planar/commands.lua" source_text "${source_text}")
  file(CONFIGURE OUTPUT "${source_module}" CONTENT "${source_text}" @ONLY)
  planar_rewrite_module(planar_x07_runtime_${variant}
    INTERFACE "${generated}/runtime.cppm" SOURCES "${generated}/runtime.cpp"
    DEPENDS planar_history_read planar_acceptance_source_capture planar_acceptance_intent planar_capture planar_capture_profile planar_history_effective_catalog planar_history_effective_provenance planar_history_interpretation_v2_codec planar_closure_guard planar_ledger_completion planar_history_provenance_codec planar_hash planar_json_codec planar_lua_trusted_libs planar_x07_bundle_source_${variant})
  planar_rewrite_module(planar_x07_invocation_${variant}
    INTERFACE "${generated}/invocation.cppm" SOURCES "${generated}/invocation.cpp"
    DEPENDS planar_command_response planar_x07_runtime_${variant} planar_capture planar_capture_host_path planar_ledger_format planar_ledger_acceptance planar_history_read planar_source_export planar_acceptance_load)
  foreach(family render init history export load extension)
    planar_rewrite_module(planar_x07_${family}_${variant}
      INTERFACE "${generated}/handlers/${family}/command.cppm" SOURCES "${generated}/handlers/${family}/command.cpp"
      DEPENDS planar_x07_invocation_${variant} CLI11::CLI11_Module)
    set_target_properties(planar_x07_${family}_${variant} PROPERTIES EXCLUDE_FROM_ALL TRUE)
  endforeach()
  planar_rewrite_module(planar_x07_bootstrap_${variant}
    INTERFACE "${generated}/bootstrap.cppm" SOURCES "${generated}/bootstrap.cpp"
    DEPENDS planar_command_envelope planar_x07_render_${variant} planar_x07_init_${variant} planar_x07_history_${variant} planar_x07_export_${variant} planar_x07_load_${variant} planar_x07_extension_${variant})
  add_executable(planar_x07_variant_${variant} EXCLUDE_FROM_ALL "${generated}/main.cpp")
  planar_native_target(planar_x07_variant_${variant})
  target_link_libraries(planar_x07_variant_${variant} PRIVATE planar_x07_bootstrap_${variant})
  add_dependencies(planar_tests planar_x07_variant_${variant})
  set_target_properties(planar_x07_runtime_${variant} planar_x07_bundle_source_${variant} planar_x07_invocation_${variant} planar_x07_bootstrap_${variant} PROPERTIES EXCLUDE_FROM_ALL TRUE)
endforeach()
foreach(case IN ITEMS init:0 history:1 export:2)
  string(REPLACE ":" ";" parts "${case}")
  list(GET parts 0 x07_family)
  list(GET parts 1 x07_index)
  add_test(NAME x07.substitute_${x07_family} COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/acceptance/x07_substitution.py" "$<TARGET_FILE:planar_x07_variant_${x07_index}>"
    "x07.substitute_${x07_family}" "$<TARGET_FILE:planar>")
  set_tests_properties(x07.substitute_${x07_family} PROPERTIES TIMEOUT 120 LABELS acceptance)
endforeach()

# X07 wave B acceptance: variant 3 pauses or faults at named anchors, variant 4 has a tiny response ceiling; every case
# drives real processes in scratch directories only. The planar binary is the production candidate.
foreach(group IN ITEMS services replay workflow)
  set(x07_cases_services x07.late_receipt_survives_to_seal x07.response_ceiling_extension)
  set(x07_cases_replay x07.replay_after_inputs_deleted x07.replay_conflict_matrix
    x07.replay_same_selector_changed_bytes_new_provenance x07.relookup_equal_replay_before_stale_cas
    x07.relookup_unequal_conflicts x07.two_process_cas_one_winner x07.receipt_races_refuse_before_authorization
    x07.final_race_rolls_back_before_commit x07.post_commit_allocation_failure_keeps_receipt
    x07.post_commit_encoding_failure_keeps_receipt x07.post_commit_stdout_failure_keeps_receipt
    x07.ledger_commit_after_admission_reports_revision_conflict)
  set(x07_cases_workflow x07.workflow_shape_load x07.workflow_render_external x07.workflow_verb_history_export)
  foreach(x07_case IN LISTS x07_cases_${group})
    add_test(NAME ${x07_case} COMMAND "${Python3_EXECUTABLE}"
      "${PROJECT_SOURCE_DIR}/tests/acceptance/x07_${group}.py" "$<TARGET_FILE:planar_x07_variant_3>"
      "${x07_case}" "$<TARGET_FILE:planar>" "$<TARGET_FILE:planar_x07_variant_4>")
    set_tests_properties(${x07_case} PROPERTIES TIMEOUT 240 LABELS acceptance)
  endforeach()
endforeach()

# The acceptance tests above run the production binary, so building the test aggregate must also link it.
add_dependencies(planar_tests planar)
