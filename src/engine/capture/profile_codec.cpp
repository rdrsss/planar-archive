module;
#include <glaze/glaze.hpp>
namespace planar_profile_detail {
/// Internal typed propagation of schema or resource refusals through Glaze.
struct refusal {
  bool resource = false; ///< Whether a fixed resource ceiling was exceeded.
};
// JSON's string reader does not implement context max_string_length. Decode
// through its ordinary string_t path into fixed scratch instead of allocating
// the raw escaped length. A 1024-byte decoded string has at most 6144 raw bytes.
/// Fixed decoding scratch; Glaze owns escape decoding and every write is size-checked.
struct decode_buffer {
  std::array<char, 6152> bytes{};    ///< Maximum raw expansion plus Glaze write padding.
  std::size_t            length = 0; ///< Current library-requested scratch length.
  /// View the current scratch bytes without allocation.
  operator std::string_view() const {
    return {bytes.data(), length};
  }
  /// @brief Return fixed writable scratch storage to the library.
  /// @return Fixed scratch pointer; callers obey the checked length.
  char* data() {
    return bytes.data();
  }
  /// @brief Return the current decoded or intermediate library length.
  /// @return Current byte length without allocation.
  std::size_t size() const {
    return length;
  }
  /// @brief Check the requested length before Glaze writes, never truncating.
  /// @param n Requested intermediate or decoded byte count.
  void resize(std::size_t n) {
    if (n > bytes.size())
      throw refusal{true};
    length = n;
  }
  /// @brief Copy Glaze's short-string decode through the same checked bound.
  /// @param source Library-owned temporary bytes.
  /// @param n Number of bytes to copy.
  void assign(const char* source, std::size_t n) {
    resize(n);
    std::copy_n(source, n, bytes.data());
  }
};
/// Retained decoded string, owned only after the fixed scratch length check.
struct text {
  std::string bytes; ///< Non-NUL decoded text of at most1024 bytes.
  /// View quoted object keys without making a temporary owned key copy.
  operator std::string_view() const {
    return bytes;
  }
};
/// Library container adapter enforcing element/node ceilings before vector growth.
/// @tparam T Library-parsed element type.
/// @tparam Maximum Maximum elements in this container.
template <class T, std::size_t Maximum> struct bounded_list : std::vector<T> {
  std::size_t* nodes = nullptr; ///< Borrowed per-read node counter; used only during parsing.
  /// @brief Construct the next element only when both resource ceilings allow it.
  /// @return True on construction; exhaustion propagates a typed resource refusal.
  bool try_emplace_back() {
    // Glaze calls this before constructing an element or growing the container.
    if (this->size() >= Maximum || (nodes && *nodes >= 132))
      throw refusal{true};
    this->emplace_back();
    return true;
  }
};
/// Library-owned structural value retaining raw numeric views and decoded duplicate keys.
struct node {
  using array  = bounded_list<node, 64>;                   ///< Bounded ordered array payload.
  using object = bounded_list<std::pair<text, node>, 132>; ///< Object payload preserving every decoded key.
  std::variant<std::nullptr_t, bool, glz::raw_json_view, text, array, object> data; ///< Parsed structural payload.
};
/// Per-read cumulative structure accounting with no shared mutable state.
struct context : glz::context {
  std::size_t nodes = 0; ///< Values already entered by the library adapter.
};
/// Strict complete-input UTF-8 parsing without comments or lossy conversion.
struct options : glz::opts {
  bool validate_trailing_whitespace = true; ///< Refuse any trailing non-whitespace.
  bool validate_utf8                = true; ///< Validate raw JSON scalar UTF-8.
};
constexpr options opts{};
} // namespace planar_profile_detail
namespace glz {
/// Decode one string through bounded fixed scratch before retaining owned bytes.
template <> struct from<JSON, planar_profile_detail::text> {
  /// @brief Parse a bounded profile value with Glaze-owned grammar.
  /// @param value Destination value.
  /// @param ctx Per-call library context and resource accounting.
  /// @param it Current input position, advanced by Glaze.
  /// @param end End of the original bounded input.
  template <auto Opts> static void op(auto& value, auto& ctx, auto& it, auto end) {
    using namespace planar_profile_detail;
    glz::raw_json_view raw;
    parse<JSON>::op<Opts>(raw, ctx, it, end);
    if (bool(ctx.error))
      return;
    if (raw.str.size() < 2 || raw.str.front() != '"')
      throw refusal{};
    if (raw.str.size() - 2 > 6144)
      throw refusal{true};
    decode_buffer decoded;
    auto          read = glz::read<planar_profile_detail::opts>(decoded, raw.str);
    if (read)
      throw refusal{};
    if (decoded.size() > 1024)
      throw refusal{true};
    std::string_view result = decoded;
    if (result.find('\0') != result.npos)
      throw refusal{};
    value.bytes.assign(result);
  }
};
/// Enforce cumulative nodes/depth around ordinary Glaze value readers.
template <> struct from<JSON, planar_profile_detail::node> {
  /// @brief Parse a bounded profile value with Glaze-owned grammar.
  /// @param value Destination value.
  /// @param ctx Per-call library context and resource accounting.
  /// @param it Current input position, advanced by Glaze.
  /// @param end End of the original bounded input.
  template <auto Opts> static void op(auto& value, auto& ctx, auto& it, auto end) {
    using namespace planar_profile_detail;
    if (++ctx.nodes > 132)
      throw refusal{true};
    if (skip_ws<Opts>(ctx, it, end) || it == end)
      return;
    if ((*it == '{' || *it == '[') && ctx.depth >= 4)
      throw refusal{true};
    // Dispatch selects library readers; Glaze owns all token/number grammar.
    switch (*it) {
    case '"':
      parse<JSON>::op<Opts>(value.data.template emplace<planar_profile_detail::text>(), ctx, it, end);
      break;
    case '{':
      value.data.template emplace<node::object>().nodes = &ctx.nodes;
      parse<JSON>::op<Opts>(std::get<node::object>(value.data), ctx, it, end);
      break;
    case '[':
      value.data.template emplace<node::array>().nodes = &ctx.nodes;
      parse<JSON>::op<Opts>(std::get<node::array>(value.data), ctx, it, end);
      break;
    case 'n':
      parse<JSON>::op<Opts>(value.data.template emplace<std::nullptr_t>(), ctx, it, end);
      break;
    case 't':
    case 'f':
      parse<JSON>::op<Opts>(value.data.template emplace<bool>(), ctx, it, end);
      break;
    default:
      parse<JSON>::op<Opts>(value.data.template emplace<glz::raw_json_view>(), ctx, it, end);
      break;
    }
  }
};
} // namespace glz
module planar.engine.capture.profile_codec;
import std;
namespace planar::engine {
namespace {
profile_error error(std::string_view code, std::string field = "profile") {
  std::string_view message = "Invalid external profile structure";
  if (code == "unsupported_version")
    message = "Unsupported external profile version";
  else if (code == "path")
    message = "Invalid external module path";
  else if (code == "resource_limit")
    message = "External profile resource limit exceeded";
  else if (code == "capture_changed")
    message = "Captured external profile changed";
  return {{std::string(code), std::string(message)}, std::move(field)};
}
std::string module_field(std::size_t i) {
  return std::format("profile.modules[{}]", i + 1);
}
std::string fold(std::string s) {
  for (auto& c : s)
    if (c >= 'A' && c <= 'Z')
      c += 'a' - 'A';
  return s;
}
} // namespace
std::expected<std::vector<std::string>, profile_error> decode_profile(std::string_view bytes) {
  using namespace planar_profile_detail;
  if (bytes.size() > 65536)
    return std::unexpected(error("resource_limit"));
  if (!valid_text(bytes))
    return std::unexpected(error("schema"));
  try {
    node    parsed;
    context ctx;
    auto    status = glz::read<opts>(parsed, bytes, ctx);
    if (status)
      return std::unexpected(error("schema"));
    // The pair-range reader preserves decoded duplicate keys but is permissive
    // about separators. A strict library validation pass owns their grammar.
    if (glz::validate_json(bytes))
      return std::unexpected(error("schema"));
    auto object = std::get_if<node::object>(&parsed.data);
    if (!object || object->size() != 2)
      return std::unexpected(error("schema"));
    const node* version = nullptr;
    const node* modules = nullptr;
    for (const auto& [key, value] : *object) {
      if (key.bytes == "version" && !version)
        version = &value;
      else if (key.bytes == "modules" && !modules)
        modules = &value;
      else
        return std::unexpected(error("schema"));
    }
    if (!version || !modules)
      return std::unexpected(error("schema"));
    auto number = std::get_if<glz::raw_json_view>(&version->data);
    auto list   = std::get_if<node::array>(&modules->data);
    if (!number || number->str.find_first_of(".eE") != number->str.npos || !list)
      return std::unexpected(error("schema"));
    if (number->str != "1")
      return std::unexpected(error("unsupported_version"));
    if (list->size() > 64)
      return std::unexpected(error("resource_limit"));
    std::vector<std::string>           result;
    std::map<std::string, std::string> spellings;
    std::set<std::string>              paths;
    for (std::size_t i = 0; i < list->size(); ++i) {
      auto item = std::get_if<text>(&(*list)[i].data);
      if (!item)
        return std::unexpected(error("schema", module_field(i)));
      const auto& path = item->bytes;
      auto        top  = fold(path.substr(0, path.find('/')));
      if (!valid_path(path) || !path.ends_with(".lua") || top == "planar" || top == "__planar__")
        return std::unexpected(error("path", module_field(i)));
      if (!paths.insert(path).second)
        return std::unexpected(error("path", module_field(i)));
      std::size_t at = 0;
      do {
        at                     = path.find('/', at);
        auto prefix            = path.substr(0, at);
        auto [found, inserted] = spellings.emplace(fold(prefix), prefix);
        if (!inserted && found->second != prefix)
          return std::unexpected(error("path", module_field(i)));
        if (at == path.npos)
          break;
        ++at;
      } while (true);
      result.push_back(path);
    }
    return result;
  } catch (const refusal& r) {
    return std::unexpected(error(r.resource ? "resource_limit" : "schema"));
  } catch (const std::bad_alloc&) {
    return std::unexpected(error("resource_limit"));
  }
}
} // namespace planar::engine
