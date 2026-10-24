# Release archive provenance for committed, offline dependency sources.
# Catch2 v3.15.3: https://codeload.github.com/catchorg/Catch2/tar.gz/refs/tags/v3.15.3
# SHA256 b0299ae552918220a7a6e21e7de5b714777f4e8c883fb70c4bb23fe01df8c6e3
# Lua 5.5.0 (R05): https://www.lua.org/ftp/lua-5.5.0.tar.gz
# SHA256 57ccc32bbbd005cab75bcc52444052535af691789dba2b9016d5c50640d68b3d
# Never download at configure time. Missing vendor sources are an incomplete checkout.
set(PLANAR_CATCH2_SOURCE_DIR "${PROJECT_SOURCE_DIR}/vendor/catch2/c601")
set(PLANAR_LUA_SOURCE_DIR "${PROJECT_SOURCE_DIR}/vendor/lua/0b10")
if(NOT EXISTS "${PLANAR_CATCH2_SOURCE_DIR}/CMakeLists.txt" OR
   NOT EXISTS "${PLANAR_LUA_SOURCE_DIR}/src/lua.h")
  message(FATAL_ERROR "Required committed vendor sources are missing")
endif()
set(CATCH_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
set(CATCH_INSTALL_EXTRAS OFF CACHE BOOL "" FORCE)
add_subdirectory("${PLANAR_CATCH2_SOURCE_DIR}" "${PROJECT_BINARY_DIR}/vendor/catch2" EXCLUDE_FROM_ALL SYSTEM)
list(APPEND CMAKE_MODULE_PATH "${PLANAR_CATCH2_SOURCE_DIR}/extras")
include(Catch)

# Restricted loader links the C core only, with no interpreter, compiler or libraries.
set(planar_lua_core lapi lcode lctype ldebug ldo ldump lfunc lgc llex lmem
  lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio lauxlib)
list(TRANSFORM planar_lua_core PREPEND "${PLANAR_LUA_SOURCE_DIR}/src/")
list(TRANSFORM planar_lua_core APPEND ".c")
add_library(planar_lua STATIC EXCLUDE_FROM_ALL ${planar_lua_core})
target_include_directories(planar_lua SYSTEM PUBLIC "${PLANAR_LUA_SOURCE_DIR}/src")

# xxHash v0.8.3: https://codeload.github.com/Cyan4973/xxHash/tar.gz/refs/tags/v0.8.3
# SHA256 aae608dfe8213dfd05d909a57718ef82f30722c392344583d3f39050c7f29a80
set(PLANAR_XXHASH_SOURCE_DIR "${PROJECT_SOURCE_DIR}/vendor/xxhash/0.8.3")
if(NOT EXISTS "${PLANAR_XXHASH_SOURCE_DIR}/xxhash.c" OR
   NOT EXISTS "${PLANAR_XXHASH_SOURCE_DIR}/xxhash.h" OR
   NOT EXISTS "${PLANAR_XXHASH_SOURCE_DIR}/LICENSE")
  message(FATAL_ERROR "Required committed xxHash sources are missing")
endif()
add_library(planar_xxhash STATIC EXCLUDE_FROM_ALL "${PLANAR_XXHASH_SOURCE_DIR}/xxhash.c")
target_include_directories(planar_xxhash SYSTEM PUBLIC "${PLANAR_XXHASH_SOURCE_DIR}")

# Glaze v8.1.0: https://codeload.github.com/stephenberry/glaze/tar.gz/refs/tags/v8.1.0
# SHA256 8cc479b53e4612fad2b4b74a079421d5ea897944e54a9ab5d1207cd68cf52b9b
set(PLANAR_GLAZE_SOURCE_DIR "${PROJECT_SOURCE_DIR}/vendor/glaze/a4a7")
if(NOT EXISTS "${PLANAR_GLAZE_SOURCE_DIR}/include/glaze/glaze.hpp" OR
   NOT EXISTS "${PLANAR_GLAZE_SOURCE_DIR}/LICENSE")
  message(FATAL_ERROR "Required committed Glaze sources are missing")
endif()
add_library(planar_glaze INTERFACE)
target_include_directories(planar_glaze SYSTEM INTERFACE "${PLANAR_GLAZE_SOURCE_DIR}/include")

# CLI11 v2.7.2: https://codeload.github.com/CLIUtils/CLI11/tar.gz/refs/tags/v2.7.2
# SHA256 46eef3101da70852ec7af026e09d485ccee81813331c8c6052d39344443b83da
set(PLANAR_CLI11_SOURCE_DIR "${PROJECT_SOURCE_DIR}/vendor/cli11/9d6a")
if(NOT EXISTS "${PLANAR_CLI11_SOURCE_DIR}/src/modules/CLI11.cppm")
  message(FATAL_ERROR "Required committed CLI11 sources are missing")
endif()
set(CLI11_MODULES ON CACHE BOOL "" FORCE)
set(CLI11_PRECOMPILED ON CACHE BOOL "" FORCE)
set(CLI11_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CLI11_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CLI11_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(CLI11_INSTALL OFF CACHE BOOL "" FORCE)
set(CLI11_FULL_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory("${PLANAR_CLI11_SOURCE_DIR}" "${PROJECT_BINARY_DIR}/vendor/cli11" EXCLUDE_FROM_ALL SYSTEM)

# SQLite 3.53.3: https://sqlite.org/2026/sqlite-amalgamation-3530300.zip
# SHA256 646421e12aac110282ef8cc68f1a62d4bb15fc7b8f09da0b53e29ee690500431
# Public domain (notice in sqlite3.c). Compile only the amalgamation, never shell.c.
set(PLANAR_SQLITE_SOURCE_DIR "${PROJECT_SOURCE_DIR}/vendor/sqlite/b9c7")
if(NOT EXISTS "${PLANAR_SQLITE_SOURCE_DIR}/sqlite3.c" OR
   NOT EXISTS "${PLANAR_SQLITE_SOURCE_DIR}/sqlite3.h")
  message(FATAL_ERROR "Required committed SQLite sources are missing")
endif()
add_library(planar_sqlite STATIC EXCLUDE_FROM_ALL "${PLANAR_SQLITE_SOURCE_DIR}/sqlite3.c")
target_include_directories(planar_sqlite SYSTEM PUBLIC "${PLANAR_SQLITE_SOURCE_DIR}")
# Serialized library internals; wrapper objects remain confined to one caller thread.
# Reject double-quoted string literals and enable foreign keys on every connection.
target_compile_definitions(planar_sqlite PRIVATE
  SQLITE_THREADSAFE=1 SQLITE_DQS=0 SQLITE_DEFAULT_FOREIGN_KEYS=1)
