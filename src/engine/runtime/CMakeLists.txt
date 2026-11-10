add_subdirectory(closure_guard)

planar_embed_lua(planar_runtime_source planar.engine.runtime_source
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/json.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/schema.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/semantic.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/selection.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/presentation.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/observations.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/render.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/dispatch.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/reconcile.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/checks.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/review.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/acceptance.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/commands.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/bootstrap.lua")

planar_rewrite_module(planar_runtime
  INTERFACE runtime.cppm SOURCES runtime.cpp
  DEPENDS planar_history_read planar_acceptance_source_capture planar_acceptance_intent planar_capture planar_capture_profile planar_history_effective_catalog planar_history_effective_provenance planar_history_interpretation_v2_codec planar_closure_guard planar_ledger_completion planar_history_provenance_codec planar_hash planar_json_codec planar_lua_trusted_libs planar_runtime_source
  TESTS runtime.t.cpp schema.t.cpp commands.t.cpp external_registration.t.cpp
    reconcile.t.cpp dispatch.t.cpp
  TEST_TIMEOUT 120)
planar_embed_lua(planar_runtime_test_source planar.engine.runtime_test_source
  "${PROJECT_SOURCE_DIR}/tests/stdlib/bridge.lua")
# Compile the exact bridge against test-only embedded exports, in its own module.
# Verification harnesses compile the exact runtime units under renamed modules; ceilings come from compile definitions.
set(planar_runtime_unit "${CMAKE_CURRENT_SOURCE_DIR}/runtime.cpp")
set(planar_runtime_interface_unit "${CMAKE_CURRENT_SOURCE_DIR}/runtime.cppm")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/runtime.cpp")
file(READ "${planar_runtime_unit}" test_bridge)
string(REPLACE "module planar.engine.runtime;" "module planar.engine.runtime_test;" test_bridge "${test_bridge}")
string(REPLACE "import planar.engine.runtime_source;" "import planar.engine.runtime_test_source;" test_bridge "${test_bridge}")
string(REPLACE "namespace planar::engine {" "namespace planar::engine::runtime_test {" test_bridge "${test_bridge}")
file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/runtime_test.cpp" CONTENT "${test_bridge}" @ONLY)
file(READ "${planar_runtime_interface_unit}" test_interface)
string(REPLACE "module planar.engine.runtime;" "module planar.engine.runtime_test;" test_interface "${test_interface}")
string(REPLACE "namespace planar::engine {" "namespace planar::engine::runtime_test {" test_interface "${test_interface}")
file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/runtime_test.cppm" CONTENT "${test_interface}" @ONLY)
planar_rewrite_module(planar_runtime_test_harness
  INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/runtime_test.cppm" SOURCES "${CMAKE_CURRENT_BINARY_DIR}/runtime_test.cpp"
  DEPENDS planar_history_read planar_acceptance_source_capture planar_acceptance_intent planar_capture planar_capture_profile planar_history_effective_catalog planar_history_effective_provenance planar_history_interpretation_v2_codec planar_closure_guard planar_ledger_completion planar_history_provenance_codec planar_hash planar_json_codec planar_lua_trusted_libs planar_runtime_test_source)
set_target_properties(planar_runtime_test_harness planar_runtime_test_source PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_link_libraries(planar_runtime_tests PRIVATE planar_runtime_test_harness planar_lua_loader)

target_link_libraries(planar_runtime_tests PRIVATE planar_capture planar_admission_syscalls)

# Small-ceiling variant: the exact runtime compiled with injected composed ceilings, so budget tests stay fast. Tests and
# variant read the same numbers through PLANAR_BUDGET_* definitions. Rule: every non-bracketing fixture keeps at least 25
# percent of each ceiling free (native bytes, live Lua, invocation instructions, registration instructions, nodes).
# Bracketing classes are excluded per column: bisects, confirmation pairs, deliberate refusals, ceiling-sized families
# and the tight variant. Peaks, margins and exclusions are in the Margin record section of
# docs/vernacular/rounds/V5-02-R15-R18-C07-calibration.md (authority: X06-D-C04 D9-D11).
set(PLANAR_BUDGET_NATIVE_MAX 53477376)
set(PLANAR_BUDGET_INSTRUCTION_MAX 29000000)
set(PLANAR_BUDGET_REGISTRATION_INSTRUCTION_MAX 1000000)
set(PLANAR_BUDGET_NODE_MAX 37500)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/runtime.cppm")
file(READ "${planar_runtime_unit}" budget_unit_text)
string(REPLACE "module planar.engine.runtime;" "module planar.engine.runtime_budget_test;" budget_unit_text "${budget_unit_text}")
string(REPLACE "namespace planar::engine {" "namespace planar::engine::runtime_budget_test {" budget_unit_text "${budget_unit_text}")
file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/runtime_budget_test.cpp" CONTENT "${budget_unit_text}" @ONLY)
file(READ "${planar_runtime_interface_unit}" budget_interface_text)
string(REPLACE "module planar.engine.runtime;" "module planar.engine.runtime_budget_test;" budget_interface_text "${budget_interface_text}")
string(REPLACE "namespace planar::engine {" "namespace planar::engine::runtime_budget_test {" budget_interface_text "${budget_interface_text}")
file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/runtime_budget_test.cppm" CONTENT "${budget_interface_text}" @ONLY)
planar_rewrite_module(planar_runtime_budget_test_harness
  INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/runtime_budget_test.cppm" SOURCES "${CMAKE_CURRENT_BINARY_DIR}/runtime_budget_test.cpp"
  DEPENDS planar_history_read planar_acceptance_source_capture planar_acceptance_intent planar_capture planar_capture_profile planar_history_effective_catalog planar_history_effective_provenance planar_history_interpretation_v2_codec planar_closure_guard planar_ledger_completion planar_history_provenance_codec planar_hash planar_json_codec planar_lua_trusted_libs planar_runtime_source)
set_target_properties(planar_runtime_budget_test_harness PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_compile_definitions(planar_runtime_budget_test_harness PRIVATE
  PLANAR_RUNTIME_MEMORY_MAX=${PLANAR_BUDGET_NATIVE_MAX}
  PLANAR_RUNTIME_INSTRUCTION_MAX=${PLANAR_BUDGET_INSTRUCTION_MAX}
  PLANAR_RUNTIME_NODE_MAX=${PLANAR_BUDGET_NODE_MAX}
  PLANAR_RUNTIME_REGISTRATION_INSTRUCTION_MAX=${PLANAR_BUDGET_REGISTRATION_INSTRUCTION_MAX})
target_link_libraries(planar_runtime_tests PRIVATE planar_runtime_budget_test_harness)
target_compile_definitions(planar_runtime_tests PRIVATE
  PLANAR_BUDGET_NATIVE_MAX=${PLANAR_BUDGET_NATIVE_MAX}
  PLANAR_BUDGET_INSTRUCTION_MAX=${PLANAR_BUDGET_INSTRUCTION_MAX}
  PLANAR_BUDGET_REGISTRATION_INSTRUCTION_MAX=${PLANAR_BUDGET_REGISTRATION_INSTRUCTION_MAX}
  PLANAR_BUDGET_NODE_MAX=${PLANAR_BUDGET_NODE_MAX})

# Tight instruction-ceiling variant: only the invocation instruction ceiling is injected, below the inherited per-chunk limit,
# so a pre-frame declaration chunk is refused by the composed sink alone. Used by exactly one test.
set(PLANAR_TIGHT_INSTRUCTION_MAX 500000)
file(READ "${planar_runtime_unit}" tight_unit_text)
string(REPLACE "module planar.engine.runtime;" "module planar.engine.runtime_tight_test;" tight_unit_text "${tight_unit_text}")
string(REPLACE "namespace planar::engine {" "namespace planar::engine::runtime_tight_test {" tight_unit_text "${tight_unit_text}")
file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/runtime_tight_test.cpp" CONTENT "${tight_unit_text}" @ONLY)
file(READ "${planar_runtime_interface_unit}" tight_interface_text)
string(REPLACE "module planar.engine.runtime;" "module planar.engine.runtime_tight_test;" tight_interface_text "${tight_interface_text}")
string(REPLACE "namespace planar::engine {" "namespace planar::engine::runtime_tight_test {" tight_interface_text "${tight_interface_text}")
file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/runtime_tight_test.cppm" CONTENT "${tight_interface_text}" @ONLY)
planar_rewrite_module(planar_runtime_tight_test_harness
  INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/runtime_tight_test.cppm" SOURCES "${CMAKE_CURRENT_BINARY_DIR}/runtime_tight_test.cpp"
  DEPENDS planar_history_read planar_acceptance_source_capture planar_acceptance_intent planar_capture planar_capture_profile planar_history_effective_catalog planar_history_effective_provenance planar_history_interpretation_v2_codec planar_closure_guard planar_ledger_completion planar_history_provenance_codec planar_hash planar_json_codec planar_lua_trusted_libs planar_runtime_source)
set_target_properties(planar_runtime_tight_test_harness PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_compile_definitions(planar_runtime_tight_test_harness PRIVATE
  PLANAR_RUNTIME_INSTRUCTION_MAX=${PLANAR_TIGHT_INSTRUCTION_MAX})
target_link_libraries(planar_runtime_tests PRIVATE planar_runtime_tight_test_harness)
target_compile_definitions(planar_runtime_tests PRIVATE PLANAR_TIGHT_INSTRUCTION_MAX=${PLANAR_TIGHT_INSTRUCTION_MAX})

target_compile_definitions(planar_runtime_tests PRIVATE PLANAR_SCHEMA_FIXTURE="${PROJECT_SOURCE_DIR}/fixtures/render/quota")

# Test-only altered captured bundles compile the exact runtime and public interface.
set(v502b1_support "${PROJECT_SOURCE_DIR}/tests/native/v5-02-b1-support")
set(v502b1_generated "${CMAKE_CURRENT_BINARY_DIR}/v5-02-b1-generated")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/runtime.cppm"
  "${v502b1_support}/generate.py" "${v502b1_support}/altered.lua")
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
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/dispatch.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/reconcile.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/checks.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/review.lua"
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
    DEPENDS planar_history_read planar_acceptance_source_capture planar_acceptance_intent planar_capture planar_capture_profile planar_history_effective_catalog planar_history_effective_provenance planar_history_interpretation_v2_codec planar_closure_guard planar_ledger_completion planar_history_provenance_codec planar_hash planar_json_codec planar_lua_trusted_libs planar_command_source_${variant})
  set_target_properties(planar_command_variant_${variant} planar_command_source_${variant} PROPERTIES EXCLUDE_FROM_ALL TRUE)
  target_link_libraries(planar_runtime_tests PRIVATE planar_command_variant_${variant})
endforeach()

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
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/dispatch.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/reconcile.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/checks.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/review.lua"
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
    DEPENDS planar_history_read planar_acceptance_source_capture planar_acceptance_intent planar_capture planar_capture_profile planar_history_effective_catalog planar_history_effective_provenance planar_history_interpretation_v2_codec planar_closure_guard planar_ledger_completion planar_history_provenance_codec planar_hash planar_json_codec planar_lua_trusted_libs planar_load_source_${variant})
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
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/dispatch.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/reconcile.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/checks.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/review.lua"
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
    DEPENDS planar_history_read planar_acceptance_source_capture planar_acceptance_intent planar_capture planar_capture_profile planar_history_effective_catalog planar_history_effective_provenance planar_history_interpretation_v2_codec planar_closure_guard planar_ledger_completion planar_history_provenance_codec planar_hash planar_json_codec planar_lua_trusted_libs planar_load_c01_source)
  set_target_properties(planar_load_c01_variant planar_load_c01_source PROPERTIES EXCLUDE_FROM_ALL TRUE)
  target_link_libraries(planar_runtime_tests PRIVATE planar_load_c01_variant)
endforeach()

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
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/dispatch.lua"
    "${PROJECT_SOURCE_DIR}/src/stdlib/planar/reconcile.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/checks.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/review.lua"
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
    DEPENDS planar_history_read planar_acceptance_source_capture planar_acceptance_intent planar_capture planar_capture_profile planar_history_effective_catalog planar_history_effective_provenance planar_history_interpretation_v2_codec planar_closure_guard planar_ledger_completion planar_history_provenance_codec planar_hash planar_json_codec planar_lua_trusted_libs planar_x05_source_${variant} planar_x05_closure_audit)
  set_target_properties(planar_x05_runtime_${variant} planar_x05_source_${variant} PROPERTIES EXCLUDE_FROM_ALL TRUE)
  target_link_libraries(planar_x05_closure_audit_tests PRIVATE planar_x05_runtime_${variant})
endforeach()

# X05-B1 pure-helper seam: exact production runtime, distinct test-only exports.
planar_embed_lua(planar_shapes_test_source planar.engine.shapes_test_source
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/json.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/schema.lua"
  "${PROJECT_SOURCE_DIR}/src/stdlib/planar/semantic.lua"
  "${PROJECT_SOURCE_DIR}/tests/stdlib/x05-shapes.lua")
file(READ "${planar_runtime_unit}" shapes_bridge)
string(REPLACE "module planar.engine.runtime;" "module planar.engine.shapes_test;" shapes_bridge "${shapes_bridge}")
string(REPLACE "import planar.engine.runtime_source;" "import planar.engine.shapes_test_source;" shapes_bridge "${shapes_bridge}")
string(REPLACE "namespace planar::engine {" "namespace planar::engine::shapes_test {" shapes_bridge "${shapes_bridge}")
file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shapes_test.cpp" CONTENT "${shapes_bridge}" @ONLY)
file(READ "${planar_runtime_interface_unit}" shapes_interface)
string(REPLACE "module planar.engine.runtime;" "module planar.engine.shapes_test;" shapes_interface "${shapes_interface}")
string(REPLACE "namespace planar::engine {" "namespace planar::engine::shapes_test {" shapes_interface "${shapes_interface}")
file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shapes_test.cppm" CONTENT "${shapes_interface}" @ONLY)
planar_rewrite_module(planar_shapes_test_harness
  INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/shapes_test.cppm" SOURCES "${CMAKE_CURRENT_BINARY_DIR}/shapes_test.cpp"
  DEPENDS planar_history_read planar_acceptance_source_capture planar_acceptance_intent planar_capture planar_capture_profile planar_history_effective_catalog planar_history_effective_provenance planar_history_interpretation_v2_codec planar_closure_guard planar_ledger_completion planar_history_provenance_codec planar_hash planar_json_codec planar_lua_trusted_libs planar_shapes_test_source)
set_target_properties(planar_shapes_test_harness planar_shapes_test_source PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_sources(planar_runtime_tests PRIVATE shapes.t.cpp)
target_link_libraries(planar_runtime_tests PRIVATE planar_shapes_test_harness)
