# Render-only scoped role context compiler.
cmake_minimum_required(VERSION 4.3)

# --- In-source-build rejection, before project() ---------------------------
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR
    "Planar does not support in-source builds. Configure into build/<preset>, "
    "e.g.:\n  cmake --preset debug\nRemove CMakeCache.txt and CMakeFiles/ "
    "from the source root before retrying.")
endif()

# --- `import std;` experimental gate ----------------------------------------
#
# CMake gates `import std;` support behind a per-release UUID that must be
# set before CXX is enabled by project() below. The UUID is not a secret or
# a stable API — it changes between CMake feature releases as the underlying
# P1689/modules-json format changes, and using the wrong one degrades to a
# CMake authoring warning followed by a hard configure failure ("Experimental
# `import std` support not enabled"), not a silent fallback. Select the gate
# per the installed CMake minor (adapted from a sibling project's CMakeLists.txt);
# fail loudly rather than guess for an unrecognized minor. The 4.4.x row is
# empirically verified against this machine's CMake 4.4.2 — see
# docs/toolchain-parity.md § "Derived import-std / embed flag set".
if(CMAKE_VERSION VERSION_LESS 4.4)
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
      "451f2fe2-a8a2-47c3-bc32-94786d8fc91b") # CMake 4.3.x
elseif(CMAKE_VERSION VERSION_LESS 4.5)
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
      "f35a9ac6-8463-4d38-8eec-5d6008153e7d") # CMake 4.4.x — verified, docs/toolchain-parity.md
else()
  message(FATAL_ERROR
    "Unsupported CMake ${CMAKE_VERSION}: this project's import-std gate "
    "table (CMakeLists.txt) only covers 4.3.x and 4.4.x. Discover the new "
    "minor's gate UUID (grep the cmake binary for CxxImportStd, or read "
    "Source/cmExperimental.cxx from that CMake's own release tag), add a "
    "row here, and record the finding in docs/toolchain-parity.md before "
    "building Planar with it.")
endif()

project(planar VERSION 0.1.0 DESCRIPTION "Scoped role context compiler" LANGUAGES C CXX)
if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
  message(FATAL_ERROR "Planar requires Ninja for named module scanning")
endif()
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(PLANAR_WARNINGS_AS_ERRORS "Treat first-party warnings as errors" ON)
enable_testing()
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(dependencies)
include(rewrite)
add_custom_target(planar_tests)
add_subdirectory(src)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_test(NAME replacement.cli COMMAND "${Python3_EXECUTABLE}"
  "${PROJECT_SOURCE_DIR}/tests/acceptance/bootstrap_cli.py" "$<TARGET_FILE:planar>")
set_tests_properties(replacement.cli PROPERTIES TIMEOUT 30 LABELS acceptance)
add_test(NAME replacement.init_cli COMMAND "${Python3_EXECUTABLE}"
  "${PROJECT_SOURCE_DIR}/tests/acceptance/bootstrap_init.py" "$<TARGET_FILE:planar>")
set_tests_properties(replacement.init_cli PROPERTIES TIMEOUT 30 LABELS acceptance)

add_test(NAME replacement.history_cli COMMAND "${Python3_EXECUTABLE}"
  "${PROJECT_SOURCE_DIR}/tests/acceptance/bootstrap_history.py" "$<TARGET_FILE:planar>" "$<TARGET_FILE:planar_history_fixture>")
set_tests_properties(replacement.history_cli PROPERTIES TIMEOUT 120 LABELS acceptance)

add_test(NAME replacement.export_cli COMMAND "${Python3_EXECUTABLE}"
  "${PROJECT_SOURCE_DIR}/tests/acceptance/bootstrap_export.py" "$<TARGET_FILE:planar>" "$<TARGET_FILE:planar_history_fixture>")
set_tests_properties(replacement.export_cli PROPERTIES TIMEOUT 120 LABELS acceptance)
include("${PROJECT_SOURCE_DIR}/tests/native/v5-02-x08-support/x08.cmake")
