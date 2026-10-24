// Test-only pinned Glaze counting feasibility; fixture ownership keeps borrowed strings alive.
#include <array>
#include <cstdio>
#include <cstdlib>
#include <glaze/core/ostream_buffer.hpp>
#include <glaze/glaze.hpp>
#include <new>
#include <span>
#include <string>
#include <string_view>
#include <vector>
import planar.engine.json_codec;
namespace observation {
bool                         active = false;
std::size_t                  calls = 0, bytes = 0, fail_at = 0;
std::array<std::size_t, 256> requests{};
void                         request(std::size_t n) {
  if (!active)
    return;
  if (calls < requests.size())
    requests[calls] = n;
  ++calls;
  bytes += n;
  if (fail_at && calls == fail_at)
    throw std::bad_alloc{};
}
} // namespace observation
void* operator new(std::size_t n) {
  observation::request(n);
  if (auto p = std::malloc(n ? n : 1))
    return p;
  throw std::bad_alloc{};
}
void* operator new[](std::size_t n) {
  return ::operator new(n);
}
void operator delete(void* p) noexcept {
  std::free(p);
}
void operator delete[](void* p) noexcept {
  std::free(p);
}
namespace probe {
constexpr std::size_t mib = 1024 * 1024, limit = 16 * mib, threshold = 32768;
// Preconditions: <=64 sections, ASCII declaration IDs <=64 bytes, each text
// <=1MiB, cumulative raw text <=16MiB. These are fixture guarantees, not admission.
// After each array separator flush pending<threshold; a full section requires
// <=6MiB+101 bytes. Reflected field preflight padding is round16(9+256)=272;
// all post-string/section syntax plus that padding adds <374 bytes. Four times
// the pinned 256-byte writer padding conservatively covers every such position.
constexpr std::size_t scratch = threshold + 6 * mib + 1024;
struct section {
  std::string_view id, text;
  bool             mandatory;
};
struct result {
  int                      version;
  std::span<const section> sections;
};
struct owned_section {
  std::string id, text;
  bool        mandatory;
};
struct owned_result {
  int                        version;
  std::vector<owned_section> sections;
};
struct options : glz::opts {
  bool validate_trailing_whitespace = true;
  bool validate_utf8                = true;
  bool escape_control_characters    = true;
};
constexpr options opts{};
enum class status { ok, overflow, allocation, writer };
struct sink {
  std::size_t count = 0, writes = 0, last = 0, first_overflow = 0, maximum = limit;
  bool        overflow = false;
  void        write(const char*, std::streamsize n) noexcept {
    ++writes;
    last = std::size_t(n);
    if (overflow)
      return;
    if (n < 0 || std::size_t(n) > maximum - count) {
      overflow       = true;
      first_overflow = writes;
      return;
    }
    count += std::size_t(n);
  }
};
// No resize: bounded-capacity Glaze paths must check this physical scratch.
// This adapts stream storage only. Glaze owns all JSON grammar and escaping.
struct buffer {
  using value_type = char;
  std::vector<char> storage;
  sink*             output;
  std::size_t       offset = 0, finalizations = 0;
  explicit buffer(sink& s, std::size_t size = scratch) : storage(size), output(&s) {
  }
  char& operator[](std::size_t i) noexcept {
    if (i < offset || i - offset >= storage.size())
      std::abort();
    return storage[i - offset];
  }
  char* begin() noexcept {
    return storage.data();
  }
  char* end() noexcept {
    return storage.data() + storage.size();
  }
  std::size_t size() const noexcept {
    return offset + storage.size();
  }
  void flush(std::size_t n) noexcept {
    if (n - offset >= threshold) {
      output->write(storage.data(), std::streamsize(n - offset));
      offset = n;
    }
  }
  void finalize(std::size_t n) noexcept {
    ++finalizations;
    if (n > offset) {
      output->write(storage.data(), std::streamsize(n - offset));
      offset = n;
    }
  }
};
} // namespace probe
namespace glz {
template <> struct buffer_traits<probe::buffer> {
  static constexpr bool is_resizable = false, has_bounded_capacity = true, is_output_streaming = true;
  static std::size_t    capacity(const probe::buffer& b) noexcept {
    return b.size();
  }
  static void flush(probe::buffer& b, std::size_t n) noexcept {
    b.flush(n);
  }
  static void finalize(probe::buffer& b, std::size_t n) noexcept {
    b.finalize(n);
  }
};
} // namespace glz
namespace probe {
struct counted {
  status                       state;
  std::size_t                  count, writes, finalizations, first_overflow, allocations, bytes;
  std::array<std::size_t, 256> requests;
};
counted count(const result& r, bool bounded = true, std::size_t fault = 0, std::size_t capacity = scratch,
              std::size_t maximum = limit) noexcept {
  sink s;
  s.maximum             = maximum;
  status      state     = status::ok;
  std::size_t finalized = 0;
  observation::calls    = 0;
  observation::bytes    = 0;
  observation::fail_at  = fault;
  observation::active   = true;
  try {
    if (bounded) {
      buffer b(s, capacity);
      auto   ec = glz::write<opts>(r, b);
      finalized = b.finalizations;
      if (ec)
        state = status::writer;
    } else {
      glz::basic_ostream_buffer<sink> b(s);
      auto                            ec = glz::write<opts>(r, b);
      finalized                          = !ec;
      if (ec)
        state = status::writer;
    }
    if (s.overflow)
      state = status::overflow;
  } catch (const std::bad_alloc&) {
    state = status::allocation;
  } catch (...) {
    state = status::writer;
  }
  observation::active = false;
  return {state, s.count, s.writes, finalized, s.first_overflow, observation::calls, observation::bytes, observation::requests};
}
std::size_t assertions = 0, faults = 0;
void        require(bool b, const char* message) {
  ++assertions;
  if (!b) {
    std::fprintf(stderr, "FAIL %s\n", message);
    std::exit(1);
  }
}
using namespace planar::engine;
value_ptr text(std::string_view t) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = t;
  return v;
}
value_ptr integer(int x) {
  auto v     = std::make_shared<value>();
  v->kind    = value_kind::integer;
  v->integer = x;
  return v;
}
value_ptr boolean(bool x) {
  auto v     = std::make_shared<value>();
  v->kind    = value_kind::boolean;
  v->boolean = x;
  return v;
}
value_ptr graph(const result& r) {
  auto list  = std::make_shared<value>();
  list->kind = value_kind::list;
  for (const auto& s : r.sections) {
    auto v    = std::make_shared<value>();
    v->kind   = value_kind::record;
    v->fields = {{"id", text(s.id)}, {"mandatory", boolean(s.mandatory)}, {"text", text(s.text)}};
    list->items.push_back(v);
  }
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::record;
  v->fields = {{"sections", list}, {"version", integer(r.version)}};
  return v;
}
std::size_t cases = 0;
void        check(const char* name, const result& r, bool expected_overflow = false) {
  // Counting happens before construction of the equivalent generic graph.
  auto c      = count(r);
  auto normal = count(r, false);
  require(c.allocations == 1 && c.bytes == scratch, "bounded writer allocated beyond one scratch");
  require(c.state == (expected_overflow ? status::overflow : status::ok), "bounded count status");
  require(c.finalizations == 1, "exactly one finalization");
  std::string output;
  auto        ec = glz::write<opts>(r, output);
  require(!ec, "independent real output");
  require((output.size() > limit) == expected_overflow, "reference overflow");
  if (!expected_overflow)
    require(c.count == output.size() && normal.count == output.size(), "count equals actual output");
  auto encoded = codec_encode(graph(r));
  if (expected_overflow)
    require(!encoded && encoded.error().detail.code == "resource_limit", "unchanged codec refusal");
  else {
    require(bool(encoded), "unchanged codec success");
    auto a = glz::read_json<owned_result>(*encoded);
    auto b = glz::read_json<owned_result>(output);
    require(a && b && a->version == b->version && a->sections.size() == b->sections.size(), "unchanged codec equivalent graph");
    for (std::size_t i = 0; i < a->sections.size(); ++i)
      require(a->sections[i].id == b->sections[i].id && a->sections[i].text == b->sections[i].text &&
                  a->sections[i].mandatory == b->sections[i].mandatory,
              "unchanged codec exact sections");
    require(encoded->size() == c.count, "unchanged codec count equality");
  }
  std::printf("case %s bytes=%zu count=%zu status=%d writes=%zu first_overflow=%zu basic_allocations=%zu basic_cumulative=%zu "
              "bounded_allocations=%zu bounded_cumulative=%zu\n",
              name, output.size(), c.count, int(c.state), c.writes, c.first_overflow, normal.allocations, normal.bytes,
              c.allocations, c.bytes);
  std::printf("requests %s basic", name);
  for (std::size_t i = 0; i < normal.allocations; ++i)
    std::printf(" %zu", normal.requests[i]);
  std::printf(" bounded %zu\n", c.requests[0]);
  for (std::size_t i = 0; i < c.allocations; ++i) {
    ++faults;
    auto f = count(r, true, i + 1);
    require(f.state == status::allocation && f.allocations == i + 1, "bounded scratch fault controlled");
  }
  for (std::size_t i = 0; i < normal.allocations; ++i) {
    ++faults;
    auto f = count(r, false, i + 1);
    require(f.state == status::allocation && f.allocations == i + 1, "basic growth fault controlled");
  }
  ++cases;
}
} // namespace probe
struct throwing_sink {
  void write(const char*, std::streamsize) {
    throw std::bad_alloc{};
  }
};
int main(int argc, char** argv) {
  using namespace probe;
  if (argc > 1 && std::string_view(argv[1]) == "--throw-flush") {
    throwing_sink                            s;
    glz::basic_ostream_buffer<throwing_sink> b(s);
    glz::flush_buffer(b, 32768);
    return 99;
  }
  std::array<section, 64>     views{};
  std::array<std::string, 64> ids, texts;
  for (std::size_t i = 0; i < 64; ++i) {
    ids[i]   = "section_" + std::to_string(i);
    views[i] = {ids[i], {}, bool(i % 2)};
  }
  check("empty", {1, {}});
  std::string all = "ASCII \" \\ \b\f\n\r\t";
  for (char c = 1; c < 32; ++c)
    all += c;
  all += " café 世界 😀";
  views[0].text = all;
  check("escapes-controls-utf8", {1, {views.data(), 1}});
  views[0].text = {};
  check("empty-text-false", {1, {views.data(), 1}});
  views[0].mandatory = true;
  check("empty-text-true", {1, {views.data(), 1}});
  check("64-empty-sections", {1, views});
  check("integer-min-format-only", {-2147483647 - 1, {}});
  check("integer-max-format-only", {2147483647, {}});
  texts[0]      = std::string(mib, '\1');
  views[0].text = texts[0];
  check("1MiB-controls", {1, {views.data(), 1}});
  texts[0]      = std::string(mib, '"');
  views[0].text = texts[0];
  check("1MiB-quotes", {1, {views.data(), 1}});
  ids[0]        = std::string(64, 'a');
  views[0].id   = ids[0];
  texts[0]      = std::string(threshold - 1, 'a');
  views[0].text = texts[0];
  texts[1]      = std::string(mib, '\1');
  views[1].text = texts[1];
  check("threshold-max-id-max-escape", {1, {views.data(), 2}});
  // 16 one-MiB raw strings: encoded framing makes the full result exceed 16MiB.
  for (std::size_t i = 0; i < 16; ++i) {
    texts[i]      = std::string(mib, 'a');
    views[i].text = texts[i];
  }
  check("16MiB-raw", {1, {views.data(), 16}}, true);
  std::string temporary;
  auto        e = glz::write<opts>(result{1, {views.data(), 16}}, temporary);
  require(!e, "boundary fixture");
  auto excess = temporary.size() - limit;
  for (int delta = -1; delta <= 1; ++delta) {
    texts[15].resize(mib - excess + delta, 'a');
    views[15].text = texts[15];
    check(delta < 0 ? "encoded-below" : delta == 0 ? "encoded-exact" : "encoded-above", {1, {views.data(), 16}}, delta > 0);
  }
  for (std::size_t i = 0; i < 64; ++i) {
    texts[i]      = std::string(mib / 4, '\1');
    views[i].text = texts[i];
  }
  check("64-sections-16MiB-control-raw", {1, views}, true);
  for (std::size_t i = 0; i < 16; ++i) {
    texts[i]      = std::string(mib, '\1');
    views[i].text = texts[i];
  }
  check("16-max-sections-16MiB-control-raw", {1, {views.data(), 16}}, true);
  auto smallfinal = count(result{1, {}}, true, 0, scratch, 26);
  require(smallfinal.state == status::overflow && smallfinal.writes == 1 && smallfinal.finalizations == 1 &&
              smallfinal.first_overflow == 1,
          "real writer finalization overflow");
  for (std::size_t i = 0; i < 64; ++i) {
    ids[i]   = std::string(60, 'a') + std::string(4 - std::to_string(i).size(), '0') + std::to_string(i);
    texts[i] = std::string(i % 2 ? mib / 2 - 1 : 1, i % 2 ? '\1' : 'a');
    views[i] = {ids[i], texts[i], bool(i % 2)};
  }
  check("64-max-id-alternating-tiny-escape", {1, views}, true);
  auto too_small = count(result{1, {views.data() + 1, 1}}, true, 0, 512);
  require(too_small.state == status::writer && too_small.finalizations == 0, "bounded capacity safe refusal");
  // Finalization overflow: first/only sink write can exceed the actual limit only
  // when threshold is bypassed; directly exercise the same nonthrow sink adapter.
  sink final;
  final.count = limit - 1;
  final.write("xx", 2);
  require(final.overflow && final.first_overflow == 1, "final overflow sticky");
  final.write("x", 1);
  require(final.count == limit - 1, "later writes cannot mask overflow");
  require(noexcept(glz::flush_buffer(std::declval<buffer&>(), 0)), "Glaze flush noexcept");
  std::printf("PASS cases=%zu assertions=%zu allocation_faults=%zu scratch=%zu source_raw_max=%zu no_product_integration=true\n",
              cases, assertions, faults, scratch, limit);
}
