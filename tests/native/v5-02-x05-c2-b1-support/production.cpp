// Whole production-call allocation observer and independent Glaze/unchanged-codec parity.
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
std::size_t                  calls = 0, bytes = 0, fail_at = 0, live = 0;
std::array<void*, 256>       pointers{};
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
  if (auto p = std::malloc(n ? n : 1)) {
    if (observation::active) {
      observation::pointers[observation::calls - 1] = p;
      ++observation::live;
    }
    return p;
  }
  throw std::bad_alloc{};
}
void* operator new[](std::size_t n) {
  return ::operator new(n);
}
void operator delete(void* p) noexcept {
  if (observation::active)
    for (auto& ptr : observation::pointers)
      if (ptr == p && p) {
        ptr = nullptr;
        --observation::live;
        break;
      }
  std::free(p);
}
void operator delete[](void* p) noexcept {
  ::operator delete(p);
}
namespace probe {
using namespace planar::engine;
constexpr std::size_t mib = 1048576, limit = 16 * mib, threshold = 32768, scratch = 6325248;
using section = projection_section_view;
struct result {
  int                      version;
  std::span<const section> sections;
};
struct options : glz::opts {
  bool validate_trailing_whitespace = true, validate_utf8 = true, escape_control_characters = true;
};
constexpr options opts{};
std::size_t       assertions = 0, faults = 0, cases = 0;
void              require(bool b, const char* message) {
  ++assertions;
  if (!b) {
    std::fprintf(stderr, "FAIL %s\n", message);
    std::exit(1);
  }
}
struct charge {
  codec_projection_reservation           measured{};
  std::size_t                            calls  = 0;
  int                                    action = 0;
  diagnostic                             supplied{"resource_limit", std::string(200, 'm'), std::string(200, 'l'), 73};
  static std::expected<void, diagnostic> reserve(void* p, const codec_projection_reservation& m) {
    auto& s = *static_cast<charge*>(p);
    ++s.calls;
    s.measured = m;
    require(observation::calls == 0, "hook precedes every allocation");
    if (s.action == 1)
      return std::unexpected(std::move(s.supplied));
    if (s.action == 2)
      return std::unexpected(s.supplied);
    if (s.action == 3)
      throw 3;
    return {};
  }
  codec_projection_reserver hook() {
    return {this, reserve};
  }
};
struct counted {
  std::expected<std::size_t, codec_projection_error> output;
  std::size_t                                        allocations, bytes, live;
  std::array<std::size_t, 256>                       requests;
};
counted count(const result& r, codec_projection_reserver hook = {}, std::size_t fault = 0, std::size_t maximum = limit) {
  observation::calls = observation::bytes = observation::live = 0;
  observation::pointers                                       = {};
  observation::requests                                       = {};
  observation::fail_at                                        = fault;
  observation::active                                         = true;
  auto output                                                 = codec_projection_count(r.sections, hook, maximum);
  observation::active                                         = false;
  return {std::move(output), observation::calls, observation::bytes, observation::live, observation::requests};
}
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
void check(const char* name, const result& r, bool overflow = false) {
  charge owner;
  auto   c = count(r, owner.hook());
  require(owner.calls == 1, "exactly one charge");
  require(owner.measured.scratch_bytes == scratch, "exact scratch reservation");
  require(owner.measured.workspace_bytes > 0 && owner.measured.total_bytes == scratch + owner.measured.workspace_bytes,
          "checked logical workspace");
  require(c.requests[0] == scratch, "first request is exact fixed scratch");
  std::string output;
  require(!glz::write<opts>(r, output), "independent Glaze oracle");
  require((output.size() > limit) == overflow, "oracle overflow");
  require(bool(c.output) == !overflow, "production outcome");
  if (!overflow) {
    require(c.allocations == 1 && c.bytes == scratch && c.live == 0,
            "one transient scratch/no output retained for whole operation");
    require(*c.output == output.size(), "exact Glaze count");
  } else
    require(c.output.error().kind == codec_projection_failure::output_limit, "encoded limit typed");
  auto encoded = codec_encode(graph(r));
  require(bool(encoded) == !overflow, "unchanged codec outcome");
  if (encoded)
    require(encoded->size() == *c.output, "unchanged codec parity");
  auto f = count(r, {}, 1);
  ++faults;
  require(!f.output && f.output.error().kind == codec_projection_failure::resource, "actual scratch allocation refusal");
  if (overflow) {
    auto f = count(r, {}, 2);
    ++faults;
    require(!f.output && f.output.error().kind == codec_projection_failure::resource,
            "actual output diagnostic allocation refusal");
  }
  std::printf("case %s bytes=%zu allocations=%zu cumulative=%zu live=%zu workspace=%zu total=%zu\n", name, output.size(),
              c.allocations, c.bytes, c.live, owner.measured.workspace_bytes, owner.measured.total_bytes);
  ++cases;
}
void extra() {
  std::array<section, 2>     views{};
  std::array<std::string, 2> ids{std::string(64, 'a'), std::string(64, 'b')};
  views[0] = {ids[0], {}, false};
  views[1] = {ids[1], {}, true};
  std::string prefix;
  require(!glz::write<opts>(result{1, {views.data(), 1}}, prefix), "derive prefix through Glaze");
  const std::string large(mib, '\1');
  for (int delta = -9; delta <= 9; ++delta) {
    const std::string pending(threshold + delta - (prefix.size() - 1), 'x');
    views[0].text = pending;
    views[1].text = large;
    check("threshold-largest-section", {1, views});
  }
  auto exact = count({1, {}});
  require(bool(exact.output), "empty exact count");
  for (std::size_t max : std::array<std::size_t, 4>{0, 1, *exact.output - 1, *exact.output}) {
    auto r = count({1, {}}, {}, 0, max);
    require(bool(r.output) == (max == *exact.output), "tighter exact ceiling");
  }
  const std::array invalid{section{"Invalid", {}, false}};
  for (std::size_t fault : std::array<std::size_t, 2>{0, 1}) {
    charge owner;
    auto   r = count({1, invalid}, owner.hook(), fault);
    require(!r.output && owner.calls == 0, "invalid input before hook");
    require(r.requests[0] != scratch, "invalid input before scratch");
    require(r.output.error().kind == (fault ? codec_projection_failure::resource : codec_projection_failure::invalid_input),
            "invalid diagnostic allocation guarded");
    if (fault)
      ++faults;
  }
  {
    charge owner;
    owner.action = 1;
    auto pointer = owner.supplied.message.data();
    auto r       = count({1, {}}, owner.hook());
    require(!r.output && r.allocations == 0, "callback diagnostic moves without allocation");
    require(r.output.error().detail.detail.message.data() == pointer, "owned diagnostic pointer preserved");
  }
  {
    charge owner;
    owner.action = 2;
    auto r       = count({1, {}}, owner.hook(), 1);
    ++faults;
    require(!r.output && r.output.error().kind == codec_projection_failure::resource, "actual callback diagnostic copy failure");
    require(owner.supplied.message.size() == 200 && owner.supplied.limit.size() == 200,
            "failed copy preserves original callback diagnostic");
  }
  {
    charge owner;
    owner.action = 3;
    auto r       = count({1, {}}, owner.hook(), 1);
    ++faults;
    require(!r.output && r.output.error().kind == codec_projection_failure::resource, "diagnostic construction in catch guarded");
  }
  {
    // The inner one-shot scratch fault must not poison the enclosing owner.
    struct nested_owner {
      std::size_t                            prior_calls = 7, calls = 0;
      bool                                   inner_failed = false;
      static std::expected<void, diagnostic> reserve(void* p, const codec_projection_reservation&) {
        auto& owner = *static_cast<nested_owner*>(p);
        ++owner.calls;
        auto inner         = codec_projection_count({});
        owner.inner_failed = !inner && inner.error().kind == codec_projection_failure::resource;
        return {};
      }
    } owner;
    auto r = count({1, {}}, {&owner, nested_owner::reserve}, 1);
    ++faults;
    require(bool(r.output) && owner.inner_failed && owner.calls == 1 && owner.prior_calls == 7,
            "nested failure preserves prior and enclosing owner state");
    require(r.allocations == 3 && r.requests[0] == scratch && r.requests[2] == scratch && r.live == 0,
            "independent outer scratch succeeds after one-shot inner failure and diagnostic cleanup");
  }
  {
    auto r = count({1, {}}, {}, 2, 0);
    ++faults;
    require(!r.output && r.output.error().kind == codec_projection_failure::resource && r.allocations == 3,
            "outer diagnostic guard retry");
  }
}
} // namespace probe
int main() {
  using namespace probe;
#ifdef SCRATCH_DEFECT
  const std::string text(mib, '\1');
  const std::array  input{section{"a", text, false}};
  auto              r = count({1, input});
  require(!r.output && r.output.error().kind == codec_projection_failure::scratch_defect,
          "controlled physical scratch defect classification");
  require(r.output.error().detail.detail.code == "internal", "scratch defect codec vocabulary");
  std::printf("PASS controlled_scratch_defect first_request=%zu assertions=%zu\n", r.requests[0], assertions);
  return 0;
#else
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
  for (std::size_t i = 0; i < 64; ++i) {
    ids[i]   = std::string(60, 'a') + std::string(4 - std::to_string(i).size(), '0') + std::to_string(i);
    texts[i] = std::string(i % 2 ? mib / 2 - 1 : 1, i % 2 ? '\1' : 'a');
    views[i] = {ids[i], texts[i], bool(i % 2)};
  }
  check("64-max-id-alternating-tiny-escape", {1, views}, true);
  std::string corpus = "é世界😀\"\\\b\f\n\r\t";
  for (char c = 1; c < 32; ++c)
    corpus += c;
  for (std::size_t i = 0; i < 64; ++i) {
    texts[i].clear();
    for (std::size_t j = 0; j < (i * 997) % 1600; ++j)
      texts[i] += corpus;
    views[i].text = texts[i];
  }
  check("64-mixed-utf8-controls", {1, views});
  extra();
  std::printf("PASS cases=%zu assertions=%zu actual_fault_positions=%zu scratch=%zu logical_source_max=100669888\n", cases,
              assertions, faults, scratch);
#endif
}
