planar_rewrite_module(planar_closure_guard
  INTERFACE closure_guard.cppm SOURCES closure_guard.cpp
  DEPENDS planar_lua_trusted_libs TESTS closure_guard.t.cpp)
