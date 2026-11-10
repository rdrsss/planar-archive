planar_rewrite_module(planar_lua_loader
  INTERFACE loader.cppm SOURCES loader.cpp
  DEPENDS planar_value planar_lua planar_budget TESTS loader.t.cpp)
target_compile_definitions(planar_lua_loader_tests PRIVATE PLANAR_SOURCE_ROOT="${PROJECT_SOURCE_DIR}")
target_link_libraries(planar_lua_loader_tests PRIVATE planar_capture_admission)

# Link deterministic libraries only to the trusted runtime, not declaration jobs.
add_library(planar_lua_trusted_libs STATIC EXCLUDE_FROM_ALL
  "${PLANAR_LUA_SOURCE_DIR}/src/lbaselib.c"
  "${PLANAR_LUA_SOURCE_DIR}/src/ltablib.c"
  "${PLANAR_LUA_SOURCE_DIR}/src/lstrlib.c"
  "${PLANAR_LUA_SOURCE_DIR}/src/lmathlib.c"
  "${PLANAR_LUA_SOURCE_DIR}/src/lutf8lib.c")
target_link_libraries(planar_lua_trusted_libs PUBLIC planar_lua)
