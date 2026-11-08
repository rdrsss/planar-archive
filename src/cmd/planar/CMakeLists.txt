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
# R15-C activate (DD27): one real handler module, outside the generated handler prefix, shared by every composition.
planar_rewrite_module(planar_activate_command
  INTERFACE handlers/activate/command.cppm SOURCES handlers/activate/command.cpp
  DEPENDS planar_command_invocation CLI11::CLI11_Module)
planar_rewrite_module(planar_command_envelope
  INTERFACE envelope.cppm
  SOURCES envelope.cpp
  DEPENDS planar_json_codec CLI11::CLI11_Module)
planar_rewrite_module(planar_extension_command
  INTERFACE handlers/extension/command.cppm SOURCES handlers/extension/command.cpp
  DEPENDS planar_command_invocation)
# RA-1 authoring core and the six verb families (wave 1 skeleton); the product entry points do not import them without
# the PLANAR_AUTHORING_BUNDLE definition.
planar_rewrite_module(planar_command_authoring
  INTERFACE authoring.cppm SOURCES authoring.cpp
  DEPENDS planar_command_response planar_capture_host_path)
set(planar_authoring_families spec_prompt:spec-prompt critique:critique ingest:ingest explain_rejection:explain-rejection
  observe:observe orchestrator:orchestrator)
set(planar_authoring_family_targets)
foreach(pair IN LISTS planar_authoring_families)
  string(REPLACE ":" ";" pair_parts "${pair}")
  list(GET pair_parts 0 family_module)
  list(GET pair_parts 1 family_directory)
  planar_rewrite_module(planar_authoring_${family_module}
    INTERFACE handlers/${family_directory}/command.cppm SOURCES handlers/${family_directory}/command.cpp
    DEPENDS planar_command_authoring CLI11::CLI11_Module)
  list(APPEND planar_authoring_family_targets planar_authoring_${family_module})
endforeach()
planar_rewrite_module(planar_command
  INTERFACE bootstrap.cppm
  SOURCES bootstrap.cpp
  DEPENDS planar_command_response planar_command_envelope planar_extension_command planar_render_command planar_init_command planar_history_command planar_export_command planar_load_command planar_activate_command CLI11::CLI11_Module
    planar_command_authoring ${planar_authoring_family_targets}
  TESTS bootstrap.t.cpp authoring.t.cpp authoring_prompts.t.cpp authoring_observe.t.cpp authoring_orchestrator.t.cpp
    activate.t.cpp load_format3.t.cpp)
# Draft (R15-C P3): the format3 execution modules reach every invocation copy through the shared response target, and the
# real activate handler reaches every bootstrap copy through the shared envelope target; the audited generators keep
# their fixed dependency lists (DD27).
target_link_libraries(planar_command_response PUBLIC planar_ledger_execution planar_execution_snapshot_value
  planar_execution_change_value planar_history_provenance planar_canonical)
target_link_libraries(planar_command_envelope PUBLIC planar_activate_command)
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
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/dispatch.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/reconcile.lua"
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
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/dispatch.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/reconcile.lua"
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
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/dispatch.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/reconcile.lua"
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

# ---------------------------------------------------------------------------------------------------------------------
# RA-1 authoring bundle, compile-time selection (D2, D10, D18). The product targets never receive PLANAR_AUTHORING_BUNDLE.
# The inline renamed stack below is built exactly like the X07 stack from the shipped sources, with the definition set on
# its runtime and bootstrap copies only. Every generated copy is written only when its bytes change.
set(authoring_src_cmd "${CMAKE_CURRENT_SOURCE_DIR}")
set(authoring_src_runtime "${PROJECT_SOURCE_DIR}/src/engine/runtime")
set(authoring_stack_generated "${CMAKE_CURRENT_BINARY_DIR}/authoring-stack-generated")
set(authoring_variant_generated "${CMAKE_CURRENT_BINARY_DIR}/authoring-variant-generated")

# Write text to path only when the bytes differ, so a no-change build runs zero steps.
function(planar_authoring_emit path text)
  get_filename_component(directory "${path}" DIRECTORY)
  file(MAKE_DIRECTORY "${directory}")
  file(WRITE "${path}.tmp" "${text}")
  file(COPY_FILE "${path}.tmp" "${path}" ONLY_IF_DIFFERENT)
  file(REMOVE "${path}.tmp")
endfunction()

# Apply the D10 rename table to one source. kind is cmd, invocation, main or runtime; the order is part of the contract.
function(planar_authoring_stack_file source destination kind)
  file(READ "${source}" text)
  if(kind STREQUAL "main")
    string(REPLACE "planar.cmd.bootstrap" "planar.cmd.authoring_stack.bootstrap" text "${text}")
    string(REPLACE "planar::cmd::invoke" "planar::cmd::authoring_stack::invoke" text "${text}")
  elseif(kind STREQUAL "runtime")
    string(REPLACE "planar.engine.runtime;" "planar.engine.authoring_stack;" text "${text}")
    string(REPLACE "planar.engine.runtime_source;" "planar.engine.authoring_stack_source;" text "${text}")
    string(REPLACE "namespace planar::engine {" "namespace planar::engine::authoring_stack {" text "${text}")
  else()
    string(REPLACE "import planar.engine.runtime; // X07-REAL-RUNTIME" "import planar.engine.authoring_real_runtime; // X07-REAL-RUNTIME" text "${text}")
    string(REPLACE "planar.cmd.bootstrap" "planar.cmd.authoring_stack.bootstrap" text "${text}")
    string(REPLACE "planar.cmd.invocation" "planar.cmd.authoring_stack.invocation" text "${text}")
    string(REPLACE "planar.cmd.planar.handlers." "planar.cmd.authoring_stack.handlers." text "${text}")
    string(REPLACE "namespace planar::cmd {" "namespace planar::cmd::authoring_stack {" text "${text}")
    string(REPLACE "namespace planar::cmd::handlers {" "namespace planar::cmd::authoring_stack::handlers {" text "${text}")
    string(REPLACE "planar.engine.runtime;" "planar.engine.authoring_stack;" text "${text}")
    string(REPLACE "engine::command_" "engine::authoring_stack::command_" text "${text}")
    string(REPLACE "engine::profile_invocation" "engine::authoring_stack::profile_invocation" text "${text}")
    string(REPLACE "engine::registration_error" "engine::authoring_stack::registration_error" text "${text}")
    string(REPLACE "engine::extension_" "engine::authoring_stack::extension_" text "${text}")
    string(REPLACE "engine::prepared_profile" "engine::authoring_stack::prepared_profile" text "${text}")
    string(REPLACE "import planar.engine.authoring_real_runtime; // X07-REAL-RUNTIME" "import planar.engine.runtime; // X07-REAL-RUNTIME" text "${text}")
    if(kind STREQUAL "invocation")
      string(CONCAT bound "using namespace engine;\n"
        "using engine::authoring_stack::command_services;\n"
        "using engine::authoring_stack::command_service_result;\n"
        "using engine::authoring_stack::command_render_options;\n"
        "using engine::authoring_stack::command_init_options;\n"
        "using engine::authoring_stack::command_history_options;\n"
        "using engine::authoring_stack::command_history_selector;\n"
        "using engine::authoring_stack::command_export_options;\n"
        "using engine::authoring_stack::command_load_options;\n"
        "using engine::authoring_stack::command_authoring_options;\n"
        "using engine::authoring_stack::command_authoring_verb;")
      string(REPLACE "using namespace engine;" "${bound}" text "${text}")
    endif()
  endif()
  planar_authoring_emit("${destination}" "${text}")
endfunction()

set(authoring_stack_sources
  bootstrap.cpp bootstrap.cppm invocation.cppm
  handlers/render/command.cpp handlers/render/command.cppm handlers/init/command.cpp handlers/init/command.cppm
  handlers/history/command.cpp handlers/history/command.cppm handlers/export/command.cpp handlers/export/command.cppm
  handlers/load/command.cpp handlers/load/command.cppm handlers/extension/command.cpp handlers/extension/command.cppm)
set(authoring_stack_inputs "${authoring_src_cmd}/main.cpp" "${authoring_src_cmd}/invocation.cpp"
  "${authoring_src_runtime}/runtime.cpp" "${authoring_src_runtime}/runtime.cppm")
foreach(file IN LISTS authoring_stack_sources)
  list(APPEND authoring_stack_inputs "${authoring_src_cmd}/${file}")
  planar_authoring_stack_file("${authoring_src_cmd}/${file}" "${authoring_stack_generated}/${file}" cmd)
endforeach()
planar_authoring_stack_file("${authoring_src_cmd}/invocation.cpp" "${authoring_stack_generated}/invocation.cpp" invocation)
planar_authoring_stack_file("${authoring_src_cmd}/main.cpp" "${authoring_stack_generated}/main.cpp" main)
planar_authoring_stack_file("${authoring_src_runtime}/runtime.cpp" "${authoring_stack_generated}/runtime.cpp" runtime)
planar_authoring_stack_file("${authoring_src_runtime}/runtime.cppm" "${authoring_stack_generated}/runtime.cppm" runtime)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${authoring_stack_inputs}
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/commands.lua")

set(authoring_lua_files
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/json.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/schema.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/semantic.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/selection.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/presentation.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/observations.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/render.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/dispatch.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/reconcile.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/prompts.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/observe.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/orchestrator.lua")
set(authoring_runtime_depends planar_history_read planar_acceptance_source_capture planar_acceptance_intent planar_capture
  planar_capture_profile planar_history_effective_catalog planar_history_effective_provenance
  planar_history_interpretation_v2_codec planar_closure_guard planar_ledger_completion planar_history_provenance_codec
  planar_hash planar_json_codec planar_lua_trusted_libs)
set(authoring_invocation_depends planar_command_response planar_capture planar_capture_host_path planar_ledger_format
  planar_ledger_acceptance planar_history_read planar_source_export planar_acceptance_load planar_capture_directory)

# The stack embeds the shipped, unedited commands.lua.
planar_embed_lua(planar_authoring_stack_source planar.engine.authoring_stack_source
  ${authoring_lua_files}
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/acceptance.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/commands.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/bootstrap.lua")
planar_rewrite_module(planar_authoring_stack_runtime
  INTERFACE "${authoring_stack_generated}/runtime.cppm" SOURCES "${authoring_stack_generated}/runtime.cpp"
  DEPENDS ${authoring_runtime_depends} planar_authoring_stack_source)
planar_rewrite_module(planar_authoring_stack_invocation
  INTERFACE "${authoring_stack_generated}/invocation.cppm" SOURCES "${authoring_stack_generated}/invocation.cpp"
  DEPENDS ${authoring_invocation_depends} planar_authoring_stack_runtime)
set(authoring_stack_family_targets)
foreach(family render init history export load extension)
  planar_rewrite_module(planar_authoring_stack_${family}
    INTERFACE "${authoring_stack_generated}/handlers/${family}/command.cppm"
    SOURCES "${authoring_stack_generated}/handlers/${family}/command.cpp"
    DEPENDS planar_authoring_stack_invocation CLI11::CLI11_Module)
  set_target_properties(planar_authoring_stack_${family} PROPERTIES EXCLUDE_FROM_ALL TRUE)
  list(APPEND authoring_stack_family_targets planar_authoring_stack_${family})
endforeach()
planar_rewrite_module(planar_authoring_stack_bootstrap
  INTERFACE "${authoring_stack_generated}/bootstrap.cppm" SOURCES "${authoring_stack_generated}/bootstrap.cpp"
  DEPENDS planar_command_envelope ${authoring_stack_family_targets} planar_command_authoring ${planar_authoring_family_targets})
add_executable(planar_authoring_stack EXCLUDE_FROM_ALL "${authoring_stack_generated}/main.cpp")
planar_native_target(planar_authoring_stack)
target_link_libraries(planar_authoring_stack PRIVATE planar_authoring_stack_bootstrap)
add_dependencies(planar_tests planar_authoring_stack)
set_target_properties(planar_authoring_stack_runtime planar_authoring_stack_source planar_authoring_stack_invocation
  planar_authoring_stack_bootstrap PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_compile_definitions(planar_authoring_stack_runtime PRIVATE PLANAR_AUTHORING_BUNDLE=1)
target_compile_definitions(planar_authoring_stack_invocation PRIVATE PLANAR_AUTHORING_BUNDLE=1)
target_compile_definitions(planar_authoring_stack_bootstrap PRIVATE PLANAR_AUTHORING_BUNDLE=1)
# The bootstrap tests drive the authoring entry (STACK_INVOKE) and spawn the production binary (I25).
target_link_libraries(planar_command_tests PRIVATE planar_authoring_stack_bootstrap)
add_dependencies(planar_command_tests planar)
target_compile_definitions(planar_command_tests PRIVATE PLANAR_PRODUCTION_BINARY="$<TARGET_FILE:planar>")

# Malformed-bundle variants (D18). Each is a renamed copy of runtime.cpp and runtime.cppm embedding one edited commands.lua.
# Index table (definition PLANAR_AUTHORING_BUNDLE on every variant except 18):
#   0 control, unedited                              10 observe family mispaired with ingest
#   1 spec-prompt row missing                        11 explain-rejection handler id spelled with a hyphen
#   2 critique row missing                           12 observe response kind observe_v2
#   3 ingest row missing                             13 observe capability authoring_capture_v1
#   4 explain-rejection row missing                  14 orchestrator without --job
#   5 observe row missing                            15 orchestrator --jobs instead of --job
#   6 orchestrator row missing                       16 spec-prompt --registry typed decimal
#   7 shipped load row missing                       17 explain-rejection --diagnostic optional
#   8 extra twelfth row status                       18 gate if true, built WITHOUT the definition (leaky default)
#   9 critique registered twice                      19 gate if false (five rows registered, eleven required)
#  20 status registered AND sealed as a twelfth catalog row (label flip: status uses render_capture_v1)
#  21 prompts.lua include binding swapped from workspace to spec (include-mismatch refusal)
function(planar_authoring_once variable old new)
  set(text "${${variable}}")
  string(FIND "${text}" "${old}" first)
  if(first EQUAL -1)
    message(FATAL_ERROR "authoring variant anchor is absent: ${old}")
  endif()
  string(LENGTH "${old}" length)
  math(EXPR after "${first} + ${length}")
  string(SUBSTRING "${text}" ${after} -1 rest)
  string(FIND "${rest}" "${old}" second)
  if(NOT second EQUAL -1)
    message(FATAL_ERROR "authoring variant anchor occurs more than once: ${old}")
  endif()
  string(REPLACE "${old}" "${new}" text "${text}")
  set(${variable} "${text}" PARENT_SCOPE)
endfunction()

function(planar_authoring_variant_commands index variable)
  file(READ "${PROJECT_SOURCE_DIR}/src/stdlib/planar/commands.lua" text)
  set(verbs spec-prompt critique ingest explain-rejection observe orchestrator)
  if(index GREATER_EQUAL 1 AND index LESS_EQUAL 6)
    math(EXPR slot "${index} - 1")
    list(GET verbs ${slot} verb)
    planar_authoring_once(text "registration.add(authoring_descriptor(\"${verb}\")," ";(function() end)(authoring_descriptor(\"${verb}\"),")
  elseif(index EQUAL 7)
    planar_authoring_once(text "registration.add(descriptor(\"load\"," ";(function() end)(descriptor(\"load\",")
  elseif(index EQUAL 8 OR index EQUAL 20)
    set(anchor "    registration.add(authoring_descriptor(\"orchestrator\"),orchestrator_command)\n")
    planar_authoring_once(text "${anchor}" "${anchor}    registration.add(descriptor(\"status\",\"render_capture_v1\",{}),orchestrator_command)\n")
  elseif(index EQUAL 9)
    set(anchor "    registration.add(authoring_descriptor(\"critique\"),critique_command)\n")
    planar_authoring_once(text "${anchor}" "${anchor}${anchor}")
  elseif(index EQUAL 10)
    planar_authoring_once(text "[\"observe\"]={family=\"observe\"," "[\"observe\"]={family=\"ingest\",")
  elseif(index EQUAL 11)
    planar_authoring_once(text "handler=\"shipped.explain_rejection.v1\"" "handler=\"shipped.explain-rejection.v1\"")
  elseif(index EQUAL 12)
    planar_authoring_once(text "kind=\"observe_v1\"" "kind=\"observe_v2\"")
  elseif(index EQUAL 13)
    planar_authoring_once(text "capability=\"observation_directory_v1\"" "capability=\"authoring_capture_v1\"")
  elseif(index EQUAL 14)
    planar_authoring_once(text "option(\"registry\",\"text\",true),option(\"job\",\"text\",true)}" "option(\"registry\",\"text\",true)}")
  elseif(index EQUAL 15)
    planar_authoring_once(text "option(\"job\",\"text\",true)}" "option(\"jobs\",\"text\",true)}")
  elseif(index EQUAL 16)
    set(row "[\"spec-prompt\"]={family=\"spec-prompt\",handler=\"shipped.spec_prompt.v1\",kind=\"spec_prompt_v1\",capability=\"authoring_capture_v1\",options={option(\"workspace\",\"text\",true),option(\"registry\",\"text\",true)}}")
    string(REPLACE "option(\"registry\",\"text\",true)}}" "option(\"registry\",\"decimal\",true)}}" changed "${row}")
    planar_authoring_once(text "${row}" "${changed}")
  elseif(index EQUAL 17)
    planar_authoring_once(text "option(\"diagnostic\",\"text\",true)" "option(\"diagnostic\",\"text\",false)")
  elseif(index EQUAL 18)
    planar_authoring_once(text "if registration.authoring then" "if true then")
  elseif(index EQUAL 19)
    planar_authoring_once(text "if registration.authoring then" "if false then")
  endif()
  set(${variable} "${text}" PARENT_SCOPE)
endfunction()

set(authoring_variant_objects)
foreach(variant RANGE 0 21)
  set(generated "${authoring_variant_generated}/${variant}")
  planar_authoring_variant_commands(${variant} variant_commands)
  planar_authoring_emit("${generated}/commands.lua" "${variant_commands}")
  foreach(name runtime.cpp runtime.cppm)
    file(READ "${authoring_src_runtime}/${name}" text)
    string(REPLACE "planar.engine.runtime_source;" "planar.engine.authoring_source_${variant};" text "${text}")
    string(REPLACE "module planar.engine.runtime;" "module planar.engine.authoring_variant_${variant};" text "${text}")
    string(REPLACE "namespace planar::engine {" "namespace planar::engine::authoring_variant_${variant} {" text "${text}")
    if(variant EQUAL 20 AND name STREQUAL "runtime.cpp")
      planar_authoring_once(text "std::array<bundle_row, 12> bundle_table{{" "std::array<bundle_row, 13> bundle_table{{")
      set(orchestrator_row [=[    {"orchestrator", "shipped.orchestrator.v1", "orchestrator_v1", "orchestrator_capture_v1", true},
]=])
      set(status_row [=[    {"status", "shipped.status.v1", "status_v1", "render_capture_v1", true},
]=])
      planar_authoring_once(text "${orchestrator_row}" "${orchestrator_row}${status_row}")
      set(load_signature [=[  if (name == "load")
]=])
      set(status_signature [=[  if (name == "status")
    return {};
]=])
      planar_authoring_once(text "${load_signature}" "${status_signature}${load_signature}")
    endif()
    planar_authoring_emit("${generated}/${name}" "${text}")
  endforeach()
  # Variant 21 swaps the spec-prompt include binding in a generated copy of prompts.lua, same technique as commands.lua.
  set(variant_lua_files)
  set(prompts_logical)
  foreach(lua_file IN LISTS authoring_lua_files)
    if(variant EQUAL 21 AND lua_file STREQUAL "${PROJECT_SOURCE_DIR}/src/stdlib/planar/prompts.lua")
      file(READ "${lua_file}" prompts_text)
      planar_authoring_once(prompts_text [=[local prompts_spec_prompt_include="workspace"]=]
        [=[local prompts_spec_prompt_include="spec"]=])
      planar_authoring_emit("${generated}/prompts.lua" "${prompts_text}")
      file(RELATIVE_PATH prompts_logical "${PROJECT_SOURCE_DIR}" "${generated}/prompts.lua")
      list(APPEND variant_lua_files "${generated}/prompts.lua")
    else()
      list(APPEND variant_lua_files "${lua_file}")
    endif()
  endforeach()
  planar_embed_lua(planar_authoring_source_${variant} planar.engine.authoring_source_${variant}
    ${variant_lua_files}
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/acceptance.lua"
    "${generated}/commands.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/bootstrap.lua")
  # The altered file still occupies the exact shipped logical module slot.
  set(source_module "${CMAKE_CURRENT_BINARY_DIR}/planar_authoring_source_${variant}.cppm")
  file(READ "${source_module}" source_text)
  file(RELATIVE_PATH generated_logical "${PROJECT_SOURCE_DIR}" "${generated}/commands.lua")
  string(REPLACE "${generated_logical}" "planar/commands.lua" source_text "${source_text}")
  if(variant EQUAL 21)
    string(REPLACE "${prompts_logical}" "planar/prompts.lua" source_text "${source_text}")
  endif()
  file(CONFIGURE OUTPUT "${source_module}" CONTENT "${source_text}" @ONLY)
  planar_rewrite_module(planar_authoring_variant_${variant}
    INTERFACE "${generated}/runtime.cppm" SOURCES "${generated}/runtime.cpp"
    DEPENDS ${authoring_runtime_depends} planar_authoring_source_${variant})
  set_target_properties(planar_authoring_variant_${variant} planar_authoring_source_${variant} PROPERTIES EXCLUDE_FROM_ALL TRUE)
  if(NOT variant EQUAL 18)
    target_compile_definitions(planar_authoring_variant_${variant} PRIVATE PLANAR_AUTHORING_BUNDLE=1)
  endif()
  list(APPEND authoring_variant_objects planar_authoring_variant_${variant})
endforeach()
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/commands.lua" "${authoring_src_runtime}/runtime.cpp" "${authoring_src_runtime}/runtime.cppm")

add_executable(planar_authoring_catalog_tests EXCLUDE_FROM_ALL authoring_catalog.t.cpp authoring_orchestrator_variants.t.cpp)
planar_native_target(planar_authoring_catalog_tests)
target_link_libraries(planar_authoring_catalog_tests PRIVATE Catch2::Catch2WithMain planar_command_response planar_runtime
  ${authoring_variant_objects})
add_dependencies(planar_tests planar_authoring_catalog_tests)
catch_discover_tests(planar_authoring_catalog_tests TEST_PREFIX "replacement." PROPERTIES LABELS unit TIMEOUT 60)
