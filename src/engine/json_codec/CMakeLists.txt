planar_rewrite_module(planar_json_codec
  INTERFACE codec.cppm SOURCES codec.cpp
  DEPENDS planar_value planar_glaze TESTS codec.t.cpp)
