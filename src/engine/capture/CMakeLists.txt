planar_rewrite_module(planar_capture_host_path
  INTERFACE host_path.cppm SOURCES host_path.cpp
  DEPENDS planar_value TESTS host_path.t.cpp)
planar_rewrite_module(planar_capture_admission
  INTERFACE admission.cppm SOURCES admission.cpp
  DEPENDS planar_value planar_budget TESTS admission.t.cpp)
planar_rewrite_module(planar_capture_native
  INTERFACE native.cppm SOURCES native.cpp
  DEPENDS planar_value planar_capture_admission TESTS native.t.cpp)
add_library(planar_admission_syscalls SHARED EXCLUDE_FROM_ALL
  "${PROJECT_SOURCE_DIR}/tests/native/v5-02-x04-a1-support/syscalls.cpp")
planar_native_target(planar_admission_syscalls)
target_link_libraries(planar_capture_admission_tests PRIVATE planar_capture_native planar_admission_syscalls)
planar_rewrite_module(planar_capture_profile_codec
  INTERFACE profile_codec.cppm SOURCES profile_codec.cpp
  DEPENDS planar_value planar_glaze TESTS profile_codec.t.cpp)
planar_rewrite_module(planar_capture_profile
  INTERFACE profile.cppm SOURCES profile.cpp
  DEPENDS planar_capture_native planar_capture_host_path planar_capture_profile_codec TESTS profile.t.cpp)
target_link_libraries(planar_capture_profile_tests PRIVATE planar_admission_syscalls)
planar_rewrite_module(planar_capture
  INTERFACE capture.cppm SOURCES capture.cpp
  DEPENDS planar_capture_native planar_value planar_lua_loader planar_hash TESTS capture.t.cpp
  TEST_TIMEOUT 120)

planar_rewrite_module(planar_capture_directory
  INTERFACE directory.cppm directory_recheck.cppm SOURCES directory.cpp
  DEPENDS planar_capture_native planar_value TESTS directory.t.cpp)

planar_rewrite_module(planar_capture_tree_digest
  INTERFACE tree_digest.cppm SOURCES tree_digest.cpp TESTS tree_digest.t.cpp
  DEPENDS planar_value planar_canonical planar_xxhash)
