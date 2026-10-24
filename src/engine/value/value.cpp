module planar.engine.value;
import std;
namespace planar::engine {
bool valid_path(std::string_view p, bool directory) {
  if (p.empty() || p.size() > 1024 || (p.back() == '/') != directory)
    return false;
  if (directory)
    p.remove_suffix(1);
  while (!p.empty()) {
    auto end  = p.find('/');
    auto part = p.substr(0, end);
    if (part.empty() || part.size() > 128 || part == "." || part == ".." ||
        (part.size() == 4 && part[0] == '.' && (part[1] == 'g' || part[1] == 'G') && (part[2] == 'i' || part[2] == 'I') &&
         (part[3] == 't' || part[3] == 'T')))
      return false;
    for (char c : part)
      if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '.' || c == '-'))
        return false;
    if (end == std::string_view::npos)
      return true;
    p.remove_prefix(end + 1);
  }
  return false;
}
bool valid_text(std::string_view s) {
  for (std::size_t i = 0; i < s.size();) {
    auto c = static_cast<unsigned char>(s[i++]);
    if (c == 0)
      return false;
    if (c < 128)
      continue;
    unsigned n = c >= 0xc2 && c <= 0xdf ? 1 : c >= 0xe0 && c <= 0xef ? 2 : c >= 0xf0 && c <= 0xf4 ? 3 : 99;
    if (n == 99 || s.size() - i < n)
      return false;
    unsigned cp = c & ((1u << (6 - n)) - 1);
    for (unsigned j = 0; j < n; ++j) {
      c = static_cast<unsigned char>(s[i++]);
      if ((c & 0xc0) != 0x80)
        return false;
      cp = (cp << 6) | (c & 63);
    }
    if (cp < (n == 1 ? 128u : n == 2 ? 2048u : 65536u) || cp > 0x10ffff || (cp >= 0xd800 && cp <= 0xdfff))
      return false;
  }
  return true;
}
} // namespace planar::engine
