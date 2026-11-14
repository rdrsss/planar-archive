# Embed ordered originals and exact assembly bytes; never read product sources at runtime.
# Grammar: planar_embed_lua(target module source parts... [REMAP <abs> <logical>...] [LITERAL <from> <to>...]).
# Parts end at the first REMAP or LITERAL token; a bare REMAP with no pairs is a legal no-op. The module text is built in
# memory and written once by file(CONFIGURE), which leaves the file untouched when the bytes are unchanged. Each REMAP pair
# names the embedded file by the helper's own rule (relative to src/stdlib, else to PROJECT_SOURCE_DIR) and replaces that
# name with <logical>; then each LITERAL pair replaces <from> with <to> verbatim. Order: REMAP pairs in call order, then
# LITERAL pairs in call order. Without either option the bytes are the plain embedding.
function(planar_embed_lua target module source)
  set(parts)
  set(remap_pairs)
  set(literal_pairs)
  set(mode parts)
  foreach(arg IN LISTS ARGN)
    if(arg STREQUAL "REMAP")
      set(mode remap)
    elseif(arg STREQUAL "LITERAL")
      set(mode literal)
    elseif(mode STREQUAL "remap")
      list(APPEND remap_pairs "${arg}")
    elseif(mode STREQUAL "literal")
      list(APPEND literal_pairs "${arg}")
    else()
      list(APPEND parts "${arg}")
    endif()
  endforeach()
  set(bytes "")
  set(originals "")
  set(views "")
  set(index 0)
  foreach(part IN ITEMS "${source}" ${parts})
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
  set(content
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
")
  list(LENGTH remap_pairs remap_count)
  list(LENGTH literal_pairs literal_count)
  set(pair 0)
  while(pair LESS remap_count)
    list(GET remap_pairs ${pair} remap_path)
    math(EXPR logical_index "${pair} + 1")
    list(GET remap_pairs ${logical_index} remap_logical)
    file(RELATIVE_PATH remap_name "${PROJECT_SOURCE_DIR}/src/stdlib" "${remap_path}")
    if(remap_name MATCHES "^\\.\\.")
      file(RELATIVE_PATH remap_name "${PROJECT_SOURCE_DIR}" "${remap_path}")
    endif()
    string(REPLACE "${remap_name}" "${remap_logical}" content "${content}")
    math(EXPR pair "${pair} + 2")
  endwhile()
  set(pair 0)
  while(pair LESS literal_count)
    list(GET literal_pairs ${pair} literal_from)
    math(EXPR to_index "${pair} + 1")
    list(GET literal_pairs ${to_index} literal_to)
    string(REPLACE "${literal_from}" "${literal_to}" content "${content}")
    math(EXPR pair "${pair} + 2")
  endwhile()
  file(CONFIGURE OUTPUT "${output}" CONTENT "${content}" @ONLY)
  planar_rewrite_module(${target} INTERFACE "${output}")
endfunction()
