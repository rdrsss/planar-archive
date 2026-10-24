module;
#include <xxhash.h>
module planar.engine.history.request_v2;
import std;
import planar.engine.canonical;
namespace planar::engine::history {
namespace {
constexpr std::size_t      maximum = 64 * 1024 * 1024;
constexpr std::string_view magic   = "PLANAR-REQUEST\x02";
[[noreturn]] void          invalid() {
  throw record_error{record_error_code::invalid_record};
}
[[noreturn]] void exhausted() {
  throw record_error{record_error_code::resource_limit};
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
    if (n > 9223372036854775807ULL)
      return false;
  }
  return true;
}
void validate(const request_v2& v) {
  std::size_t size    = magic.size() + 5 * 8 + 1;
  auto        account = [&](std::string_view part) {
    if (part.size() > maximum - size)
      exhausted();
    size += part.size();
  };
  for (std::string_view part : {std::string_view(v.operation), std::string_view(v.actor), std::string_view(v.expected_head),
                                std::string_view(v.workspace_argument), std::string_view(v.registry_argument)})
    account(part);
  if (v.profile_argument) {
    if (8 > maximum - size)
      exhausted();
    size += 8;
    account(*v.profile_argument);
  }
  if (v.operation != "load" || v.actor.empty() || v.actor.size() > 256 || !valid_text(v.actor) || !head(v.expected_head) ||
      v.workspace_argument.empty() || !valid_text(v.workspace_argument) || v.registry_argument.empty() ||
      !valid_text(v.registry_argument))
    invalid();
  if (v.profile_argument) {
    const auto& s = *v.profile_argument;
    if (s.empty() || !valid_text(s))
      invalid();
    const std::filesystem::path path(s);
    if (!path.is_absolute() || path.lexically_normal().native() != s)
      invalid();
  }
}
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
std::string encode(const request_v2& v) {
  validate(v);
  std::string out(magic);
  auto        bytes = [&](std::string_view part) {
    for (int shift = 56; shift >= 0; shift -= 8)
      out += static_cast<char>((static_cast<std::uint64_t>(part.size()) >> shift) & 255);
    out.append(part);
  };
  bytes(v.operation);
  bytes(v.actor);
  bytes(v.expected_head);
  bytes(v.workspace_argument);
  bytes(v.registry_argument);
  out += v.profile_argument ? '\x01' : '\0';
  if (v.profile_argument)
    bytes(*v.profile_argument);
  return out;
}
struct reader {
  std::string_view input;
  std::size_t      position = 0;
  std::string_view take(std::size_t size) {
    if (size > input.size() - position)
      invalid();
    auto result = input.substr(position, size);
    position += size;
    return result;
  }
  std::string bytes(std::size_t bound = maximum) {
    std::uint64_t size = 0;
    for (unsigned char c : take(8))
      size = (size << 8) | c;
    if (size > bound)
      exhausted();
    return std::string(take(static_cast<std::size_t>(size)));
  }
};
} // namespace
std::expected<std::string, record_error> encode_request_v2(const request_v2& input) {
  return guarded([&] { return encode(input); });
}
std::expected<request_v2, record_error> decode_request_v2(std::string_view bytes) {
  return guarded([&] {
    if (bytes.size() > maximum)
      exhausted();
    reader r{bytes};
    if (r.take(magic.size()) != magic)
      invalid();
    request_v2 v;
    v.operation          = r.bytes();
    v.actor              = r.bytes(256);
    v.expected_head      = r.bytes(19);
    v.workspace_argument = r.bytes();
    v.registry_argument  = r.bytes();
    const auto tag       = static_cast<unsigned char>(r.take(1)[0]);
    if (tag == 1)
      v.profile_argument = r.bytes();
    else if (tag != 0)
      invalid();
    if (r.position != bytes.size())
      invalid();
    validate(v);
    return v;
  });
}
std::expected<record_digest, record_error> digest_request_v2(const request_v2& input) {
  return guarded([&] {
    auto               bytes = encode(input);
    XXH128_canonical_t canonical;
    XXH128_canonicalFromHash(&canonical, XXH3_128bits(bytes.data(), bytes.size()));
    record_digest result;
    std::ranges::copy(canonical.digest, result.begin());
    return result;
  });
}
} // namespace planar::engine::history
