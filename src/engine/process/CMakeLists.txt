planar_rewrite_module(planar_process_resolve
  INTERFACE resolve.cppm SOURCES resolve.cpp TESTS resolve.t.cpp)

planar_rewrite_module(planar_process_runner
  INTERFACE runner.cppm SOURCES runner.cpp TESTS runner.t.cpp TEST_TIMEOUT 60)
