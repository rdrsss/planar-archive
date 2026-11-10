planar_rewrite_module(planar_hash
  INTERFACE hash.cppm SOURCES hash.cpp
  DEPENDS planar_canonical planar_xxhash TESTS hash.t.cpp)
target_link_libraries(planar_hash_tests PRIVATE planar_capture)
target_compile_definitions(planar_hash_tests PRIVATE
  PLANAR_VECTOR_DIR="${PROJECT_SOURCE_DIR}/tests/vectors")
