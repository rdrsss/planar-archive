# V5-02-X08 test registration and the E12 generated variant (Q3 A, D8-2; DECISIONS rows 2, 11, 27, 29). Included last from the
# root CMakeLists.txt, so the planar, engine and cmd targets already exist. Test-only: the variant and its test executable stay
# out of default `all` and are built by planar_tests. Registration contract: tests/native/v5-02-x08-evidence/plan/INVARIANTS.md.

set(x08_support "${PROJECT_SOURCE_DIR}/tests/native/v5-02-x08-support")
set(x08_scripts "${PROJECT_SOURCE_DIR}/tests/acceptance")
set(x08_generated "${CMAKE_BINARY_DIR}/x08-generated")
set(x08_variant "${x08_generated}/0")

# Regenerate when the generator, the shipped Lua chunk, the runtime sources or any command source changes.
file(GLOB_RECURSE x08_command_inputs CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/cmd/planar/*.cpp" "${PROJECT_SOURCE_DIR}/src/cmd/planar/*.cppm")
set(x08_lua_inputs)
foreach(x08_name json schema semantic selection presentation observations render acceptance commands bootstrap)
  list(APPEND x08_lua_inputs "${PROJECT_SOURCE_DIR}/src/stdlib/planar/${x08_name}.lua")
endforeach()
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${x08_command_inputs} ${x08_lua_inputs}
  "${x08_support}/generate.py"
  "${PROJECT_SOURCE_DIR}/src/engine/runtime/runtime.cpp" "${PROJECT_SOURCE_DIR}/src/engine/runtime/runtime.cppm")

execute_process(COMMAND "${Python3_EXECUTABLE}" "${x08_support}/generate.py" "${PROJECT_SOURCE_DIR}" "${x08_generated}"
  COMMAND_ERROR_IS_FATAL ANY)

# The embed order and every file path come from generation.json, the same file the acceptance audit reads.
file(READ "${x08_variant}/generation.json" x08_generation)
string(JSON x08_count LENGTH "${x08_generation}" embedded)
set(x08_paths)
set(x08_logicals)
math(EXPR x08_last "${x08_count} - 1")
foreach(x08_index RANGE ${x08_last})
  string(JSON x08_path GET "${x08_generation}" embedded ${x08_index} path)
  string(JSON x08_logical GET "${x08_generation}" embedded ${x08_index} logical)
  list(APPEND x08_paths "${x08_path}")
  list(APPEND x08_logicals "${x08_logical}")
endforeach()
set(x08_all_paths "${x08_paths}")
list(POP_FRONT x08_paths x08_first)
planar_embed_lua(planar_x08_bundle_source_0 planar.engine.x08_bundle_source_0 "${x08_first}" ${x08_paths})

# Logical-path fix: planar_embed_lua names each file by its path relative to the source tree, which for generated files is a
# build path. Rewrite that name to the shipped logical name (planar/json.lua, ...) in the generated module, exactly as X07 does.
set(x08_module "${CMAKE_BINARY_DIR}/planar_x08_bundle_source_0.cppm")
file(READ "${x08_module}" x08_module_text)
foreach(x08_index RANGE ${x08_last})
  list(GET x08_all_paths ${x08_index} x08_path)
  list(GET x08_logicals ${x08_index} x08_logical)
  file(RELATIVE_PATH x08_portable "${PROJECT_SOURCE_DIR}/src/stdlib" "${x08_path}")
  if(x08_portable MATCHES "^\\.\\.")
    file(RELATIVE_PATH x08_portable "${PROJECT_SOURCE_DIR}" "${x08_path}")
  endif()
  string(REPLACE "${x08_portable}" "${x08_logical}" x08_module_text "${x08_module_text}")
endforeach()
file(CONFIGURE OUTPUT "${x08_module}" CONTENT "${x08_module_text}" @ONLY)

planar_rewrite_module(planar_x08_runtime_0
  INTERFACE "${x08_variant}/runtime.cppm" SOURCES "${x08_variant}/runtime.cpp"
  DEPENDS planar_history_read planar_acceptance_source_capture planar_acceptance_intent planar_capture planar_capture_profile
    planar_history_effective_catalog planar_history_effective_provenance planar_history_interpretation_v2_codec
    planar_closure_guard planar_ledger_completion planar_history_provenance_codec planar_hash planar_json_codec
    planar_lua_trusted_libs planar_x08_bundle_source_0)
planar_rewrite_module(planar_x08_invocation_0
  INTERFACE "${x08_variant}/invocation.cppm" SOURCES "${x08_variant}/invocation.cpp"
  DEPENDS planar_command_response planar_x08_runtime_0 planar_capture planar_capture_host_path planar_ledger_format
    planar_ledger_acceptance planar_history_read planar_source_export planar_acceptance_load)
foreach(x08_family render init history export load extension)
  planar_rewrite_module(planar_x08_${x08_family}_0
    INTERFACE "${x08_variant}/handlers/${x08_family}/command.cppm" SOURCES "${x08_variant}/handlers/${x08_family}/command.cpp"
    DEPENDS planar_x08_invocation_0 CLI11::CLI11_Module)
  set_target_properties(planar_x08_${x08_family}_0 PROPERTIES EXCLUDE_FROM_ALL TRUE)
endforeach()
planar_rewrite_module(planar_x08_bootstrap_0
  INTERFACE "${x08_variant}/bootstrap.cppm" SOURCES "${x08_variant}/bootstrap.cpp"
  DEPENDS planar_command_envelope planar_x08_render_0 planar_x08_init_0 planar_x08_history_0 planar_x08_export_0
    planar_x08_load_0 planar_x08_extension_0)
add_executable(planar_x08_variant_0 EXCLUDE_FROM_ALL "${x08_variant}/main.cpp")
planar_native_target(planar_x08_variant_0)
target_link_libraries(planar_x08_variant_0 PRIVATE planar_x08_bootstrap_0)
set_target_properties(planar_x08_runtime_0 planar_x08_bundle_source_0 planar_x08_invocation_0 planar_x08_bootstrap_0
  PROPERTIES EXCLUDE_FROM_ALL TRUE)
add_dependencies(planar_tests planar_x08_variant_0 planar)

# Registration: `python3 <script> <case> <production planar> [extra...]`, LABELS acceptance, TIMEOUT 240 unless stated.
function(x08_acceptance case script timeout)
  add_test(NAME x08.${case} COMMAND "${Python3_EXECUTABLE}" "${script}" ${case} "$<TARGET_FILE:planar>" ${ARGN})
  set_tests_properties(x08.${case} PROPERTIES TIMEOUT ${timeout} LABELS acceptance)
endfunction()

x08_acceptance(e01_no_discovery "${x08_scripts}/x08_cli.py" 240)
x08_acceptance(e01_registration_policy "${x08_scripts}/x08_cli.py" 240)
x08_acceptance(e02_envelope_rows_binary "${x08_scripts}/x08_cli.py" 900)
x08_acceptance(e03_closure_separation_binary "${x08_scripts}/x08_cli.py" 240)
x08_acceptance(e07_old_goldens_unselected_profile "${x08_scripts}/x08_cli.py" 240)
x08_acceptance(e07_old_w_vectors_binary "${x08_scripts}/x08_cli.py" 240)
x08_acceptance(e08_projection_vector_binary "${x08_scripts}/x08_cli.py" 240)
x08_acceptance(e09_history_id_binary "${x08_scripts}/x08_cli.py" 240)
x08_acceptance(e10_removed_profile_replay_binary "${x08_scripts}/x08_cli.py" 240)
x08_acceptance(adv_hardlink_module_refuses_before_chunk "${x08_scripts}/x08_adversarial.py" 240)
x08_acceptance(adv_missing_declaration_initial_admission "${x08_scripts}/x08_adversarial.py" 240)
x08_acceptance(adv_opaque_ordering_binary "${x08_scripts}/x08_adversarial.py" 240)
x08_acceptance(adv_invalid_unselected_job_refuses_all "${x08_scripts}/x08_adversarial.py" 240)
x08_acceptance(adv_job_mix_and_phase_failures "${x08_scripts}/x08_adversarial.py" 240)
x08_acceptance(adv_registry_colocation_binary "${x08_scripts}/x08_adversarial.py" 240)
x08_acceptance(adv_limits_cli "${x08_scripts}/x08_adversarial.py" 240)
x08_acceptance(timeout_headroom_531 "${x08_scripts}/x08_timeout.py" 240 "${CMAKE_BINARY_DIR}")
x08_acceptance(linux_recipe_pins "${x08_scripts}/x08_recipe.py" 240 "${PROJECT_SOURCE_DIR}")

# E12: the normal binary, the poisoned variant and the generated tree (the generation.json and the variant sources).
x08_acceptance(e12_variant_reads_identically "${x08_scripts}/x08_e12.py" 240
  "$<TARGET_FILE:planar_x08_variant_0>" "${x08_variant}")
x08_acceptance(e12_variant_exports_identically "${x08_scripts}/x08_e12.py" 240
  "$<TARGET_FILE:planar_x08_variant_0>" "${x08_variant}")
x08_acceptance(e12_variant_poison_is_live "${x08_scripts}/x08_e12.py" 240
  "$<TARGET_FILE:planar_x08_variant_0>" "${x08_variant}")
x08_acceptance(e12_variant_poison_is_live_load "${x08_scripts}/x08_e12.py" 240
  "$<TARGET_FILE:planar_x08_variant_0>" "${x08_variant}")
x08_acceptance(e12_variant_poison_is_live_profile_render "${x08_scripts}/x08_e12.py" 240
  "$<TARGET_FILE:planar_x08_variant_0>" "${x08_variant}")
x08_acceptance(e12_variant_audit "${x08_scripts}/x08_e12.py" 240
  "$<TARGET_FILE:planar_x08_variant_0>" "${x08_variant}")
x08_acceptance(e12_sources_and_profile_removed "${x08_scripts}/x08_e12.py" 240
  "$<TARGET_FILE:planar_x08_variant_0>" "${x08_variant}")

add_test(NAME x08.validator_selftest COMMAND "${Python3_EXECUTABLE}" "${x08_support}/validate.py" --self-test)
set_tests_properties(x08.validator_selftest PROPERTIES TIMEOUT 240 LABELS acceptance)
