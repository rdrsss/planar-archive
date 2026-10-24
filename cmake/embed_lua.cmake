# Embed ordered originals and exact assembly bytes; never read product sources at runtime.
function(planar_embed_lua target module source)
  set(bytes "")
  set(originals "")
  set(views "")
  set(index 0)
  foreach(part IN ITEMS "${source}" ${ARGN})
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${part}")
    file(READ "${part}" part_hex HEX)
    string(APPEND bytes "${part_hex}0a")
    string(REGEX REPLACE "(..)" "0x\\1," part_bytes "${part_hex}")
    file(RELATIVE_PATH portable "${PROJECT_SOURCE_DIR}/src/stdlib" "${part}")
    if(portable MATCHES "^\\.\\.")
      file(RELATIVE_PATH portable "${PROJECT_SOURCE_DIR}" "${part}")
    endif()
    string(APPEND originals "constexpr unsigned char original_${index}[] = {${part_bytes}0};\n")
    string(APPEND views "{\"${portable}\", {reinterpret_cast<const char*>(original_${index}), sizeof(original_${index}) - 1}},")
    math(EXPR index "${index} + 1")
  endforeach()
  string(REGEX REPLACE "(..)" "0x\\1," bytes "${bytes}")
  set(output "${CMAKE_CURRENT_BINARY_DIR}/${target}.cppm")
  file(CONFIGURE OUTPUT "${output}" CONTENT
"export module ${module};
import std;
export namespace planar::engine::embedded {
struct original_view { std::string_view path; std::string_view bytes; };
${originals}
constexpr unsigned char source_bytes[] = {${bytes}0};
std::string_view source() { return {reinterpret_cast<const char*>(source_bytes), sizeof(source_bytes) - 1}; }
std::string_view separator() { return \"\\n\"; }
std::array<original_view, ${index}> originals() { return {{${views}}}; }
}
" @ONLY)
  planar_rewrite_module(${target} INTERFACE "${output}")
endfunction()
