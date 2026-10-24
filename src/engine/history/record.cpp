module;
#include <xxhash.h>
module planar.engine.history.record;
import std;
import planar.engine.canonical;
namespace planar::engine::history {
namespace {
constexpr std::size_t      record_max  = 64 * 1024 * 1024;
constexpr std::size_t      payload_max = 16 * 1024 * 1024;
constexpr std::uint64_t    id_max      = 9223372036854775807ULL;
constexpr std::string_view magic       = "PLANAR-RECORD\x01";
[[noreturn]] void          invalid() {
  throw record_error{record_error_code::invalid_record};
}
[[noreturn]] void exhausted() {
  throw record_error{record_error_code::resource_limit};
}
bool valid_kind(record_kind kind) {
  auto n = static_cast<unsigned>(kind);
  return n >= 1 && n <= 4;
}
bool ascii(std::string_view s) {
  return !s.empty() && std::ranges::all_of(s, [](unsigned char c) { return c > 0 && c < 128; });
}
bool head(std::string_view s) {
  if (s == "none")
    return true;
  if (s.empty() || s.front() < '1' || s.front() > '9' || s.size() > 19)
    return false;
  std::uint64_t n = 0;
  for (char c : s) {
    if (c < '0' || c > '9')
      return false;
    n = n * 10 + static_cast<unsigned>(c - '0');
    if (n > id_max)
      return false;
  }
  return true;
}
void validate(const record& input) {
  std::visit(
      [](const auto& v) {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, file_record>) {
          if (v.mode != 420 && v.mode != 493)
            invalid();
          if (v.payload.size() > payload_max)
            exhausted();
        } else if constexpr (std::is_same_v<T, directory_record>) {
          // Each child requires at least 34 bytes. Bound auxiliary alias storage first.
          if (v.children.size() > (record_max - magic.size() - 9) / 34)
            exhausted();
          std::unordered_set<std::string> aliases;
          std::string_view                previous;
          std::size_t                     stored_size = magic.size() + 9;
          for (const auto& c : v.children) {
            if (c.name.size() > 128 || c.name.find('/') != std::string::npos || !valid_path(c.name) || !valid_kind(c.kind) ||
                c.id == 0 || c.id > id_max || (!previous.empty() && previous >= c.name))
              invalid();
            if (c.name.size() + 33 > record_max - stored_size)
              exhausted();
            stored_size += c.name.size() + 33;
            auto folded = c.name;
            for (char& ch : folded)
              if (ch >= 'A' && ch <= 'Z')
                ch += 'a' - 'A';
            if (!aliases.insert(std::move(folded)).second)
              invalid();
            previous = c.name;
          }
        } else if constexpr (std::is_same_v<T, value_record>) {
          if (v.codec.empty() || !valid_text(v.codec))
            invalid();
          if (v.codec != "canonical_v1")
            throw record_error{record_error_code::unsupported_codec};
          if (v.encoded.size() > payload_max)
            exhausted();
          auto decoded = canonical_decode(v.encoded, {.bytes = payload_max});
          if (!decoded) {
            if (decoded.error().code == "resource_limit")
              exhausted();
            invalid();
          }
        } else {
          std::size_t stored_size = magic.size() + 1 + 5 * 8;
          for (std::string_view part :
               {std::string_view(v.operation), std::string_view(v.actor), std::string_view(v.expected_head),
                std::string_view(v.workspace_argument), std::string_view(v.registry_argument)}) {
            if (part.size() > record_max - stored_size)
              exhausted();
            stored_size += part.size();
          }
          if (!ascii(v.operation) || v.actor.empty() || v.actor.size() > 256 || !valid_text(v.actor) || !head(v.expected_head) ||
              v.workspace_argument.empty() || !valid_text(v.workspace_argument) || v.registry_argument.empty() ||
              !valid_text(v.registry_argument))
            invalid();
        }
      },
      input);
}
struct writer {
  std::string out;
  void        append(std::string_view bytes) {
    if (bytes.size() > record_max - out.size())
      exhausted();
    out.append(bytes);
  }
  void number(std::uint64_t n, unsigned width) {
    if (width > record_max - out.size())
      exhausted();
    for (int shift = static_cast<int>(width * 8) - 8; shift >= 0; shift -= 8)
      out += static_cast<char>((n >> shift) & 255);
  }
  void bytes(std::string_view s) {
    number(s.size(), 8);
    append(s);
  }
};
std::string encode(const record& input, bool portable) {
  validate(input);
  writer w;
  w.append(magic);
  w.number(static_cast<unsigned>(kind_of(input)), 1);
  std::visit(
      [&](const auto& v) {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, file_record>) {
          w.number(v.mode, 4);
          w.bytes(v.payload);
        } else if constexpr (std::is_same_v<T, directory_record>) {
          w.number(493, 4);
          w.number(v.children.size(), 4);
          for (const auto& c : v.children) {
            w.bytes(c.name);
            w.number(static_cast<unsigned>(c.kind), 1);
            if (!portable)
              w.number(c.id, 8);
            w.append(std::string_view(reinterpret_cast<const char*>(c.digest.data()), c.digest.size()));
          }
        } else if constexpr (std::is_same_v<T, value_record>) {
          w.bytes(v.codec);
          w.bytes(v.encoded);
        } else {
          w.bytes(v.operation);
          w.bytes(v.actor);
          w.bytes(v.expected_head);
          w.bytes(v.workspace_argument);
          w.bytes(v.registry_argument);
        }
      },
      input);
  return std::move(w.out);
}
struct reader {
  std::string_view input;
  std::size_t      position = 0;
  std::string_view take(std::size_t n) {
    if (n > input.size() - position)
      invalid();
    auto result = input.substr(position, n);
    position += n;
    return result;
  }
  std::uint64_t number(std::size_t width) {
    auto          bytes = take(width);
    std::uint64_t n     = 0;
    for (unsigned char c : bytes)
      n = (n << 8) | c;
    return n;
  }
  std::string bytes(std::size_t maximum = record_max) {
    auto size = number(8);
    if (size > maximum)
      exhausted();
    return std::string(take(static_cast<std::size_t>(size)));
  }
};
template <class F> auto guarded(F&& f) -> std::expected<decltype(f()), record_error> {
  try {
    return f();
  } catch (record_error e) {
    return std::unexpected(e);
  } catch (const std::bad_alloc&) {
    return std::unexpected(record_error{record_error_code::resource_limit});
  } catch (const std::length_error&) {
    return std::unexpected(record_error{record_error_code::resource_limit});
  }
}
} // namespace
record_kind kind_of(const record& input) {
  return static_cast<record_kind>(input.index() + 1);
}
std::expected<std::string, record_error> encode_record(const record& input) {
  return guarded([&] { return encode(input, false); });
}
std::expected<record, record_error> decode_record(std::string_view bytes) {
  return guarded([&]() -> record {
    if (bytes.size() > record_max)
      exhausted();
    reader r{bytes};
    if (r.take(magic.size()) != magic)
      invalid();
    auto   kind = static_cast<record_kind>(r.number(1));
    record result;
    switch (kind) {
    case record_kind::file: {
      auto mode = static_cast<std::uint32_t>(r.number(4));
      if (mode != 420 && mode != 493)
        invalid();
      result = file_record{mode, r.bytes(payload_max)};
      break;
    }
    case record_kind::directory: {
      if (r.number(4) != 493)
        invalid();
      auto count = r.number(4);
      if (count > (r.input.size() - r.position) / 34)
        invalid();
      directory_record directory;
      for (std::uint64_t i = 0; i < count; ++i) {
        record_child child;
        child.name  = r.bytes(128);
        child.kind  = static_cast<record_kind>(r.number(1));
        child.id    = r.number(8);
        auto digest = r.take(16);
        std::ranges::transform(digest, child.digest.begin(), [](unsigned char c) { return c; });
        directory.children.push_back(std::move(child));
      }
      result = std::move(directory);
      break;
    }
    case record_kind::value: {
      auto codec = r.bytes();
      result     = value_record{std::move(codec), r.bytes(payload_max)};
      break;
    }
    case record_kind::request: {
      request_record request;
      request.operation          = r.bytes();
      request.actor              = r.bytes(256);
      request.expected_head      = r.bytes(19);
      request.workspace_argument = r.bytes();
      request.registry_argument  = r.bytes();
      result                     = std::move(request);
      break;
    }
    default:
      invalid();
    }
    if (r.position != bytes.size())
      invalid();
    validate(result);
    return result;
  });
}
std::expected<record_digest, record_error> digest_record(const record& input) {
  return guarded([&] {
    auto               bytes = encode(input, true);
    XXH128_canonical_t canonical;
    XXH128_canonicalFromHash(&canonical, XXH3_128bits(bytes.data(), bytes.size()));
    record_digest result;
    std::ranges::copy(canonical.digest, result.begin());
    return result;
  });
}
} // namespace planar::engine::history
