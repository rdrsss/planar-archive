# Shared setup is inherited by every engine component, including clean builds.
include("${PROJECT_SOURCE_DIR}/cmake/embed_lua.cmake")
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Components own their targets and colocated tests. Named link dependencies may
# refer to later components; CMake resolves them when generating the build graph.
add_subdirectory(value)
add_subdirectory(budget)
add_subdirectory(lua)
add_subdirectory(capture)
add_subdirectory(canonical)
add_subdirectory(hash)
add_subdirectory(json_codec)
add_subdirectory(history)
add_subdirectory(ledger)
add_subdirectory(acceptance)
add_subdirectory(export)
add_subdirectory(process)
add_subdirectory(runtime)
add_subdirectory(execution)
