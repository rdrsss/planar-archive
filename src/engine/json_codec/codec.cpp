module;
#include <glaze/glaze.hpp>
#include <unordered_set>
namespace planar_codec_detail {
/// Erased synchronous request edge, installed before any reader growth.
struct allocation_state {
  void* context;                      ///< Native adapter alive for the complete parse.
  void (*charge)(void*, std::size_t); ///< Charge before entering the ordinary allocator.
};
/// Stateful allocator routing each actual parser/checker request before allocation.
template <class T> struct allocator {
  using value_type                             = T;              ///< Allocator element type.
  using propagate_on_container_move_assignment = std::true_type; ///< Preserve request binding on moves.
  allocation_state* state                      = nullptr;        ///< Nonowning request binding.
  allocator()                                  = default;
  /// Bind the request owner without allocation.
  /// @param s Request owner alive until container destruction.
  explicit allocator(allocation_state* s) : state(s) {
  }
  /// Preserve the request pointer across standard allocator rebinds.
  /// @param other Allocator retaining the same owner.
  template <class U> allocator(const allocator<U>& other) : state(other.state) {
  }
  /// Charge checked byte size synchronously, then allocate the requested elements.
  /// @param count Number of elements requested by the library.
  /// @return Ordinary allocated storage after successful charge.
  T* allocate(std::size_t count) {
    if (!state)
      throw std::logic_error("unbound parser allocator");
    if (count > std::numeric_limits<std::size_t>::max() / sizeof(T))
      throw std::bad_array_new_length();
    state->charge(state->context, count * sizeof(T));
    return std::allocator<T>{}.allocate(count);
  }
  /// Release storage without refunding cumulative requests.
  /// @param p Previously allocated storage.
  /// @param count Original element count.
  void deallocate(T* p, std::size_t count) noexcept {
    std::allocator<T>{}.deallocate(p, count);
  }
  /// Compare request identity for standard container ownership rules.
  /// @param other Allocator to compare.
  /// @return Whether both allocators retain the same request.
  template <class U> bool operator==(const allocator<U>& other) const noexcept {
    return state == other.state;
  }
};
/// String whose Glaze reader binds its request before any growth.
struct text : std::basic_string<char, std::char_traits<char>, allocator<char>> {
  using base =
      std::basic_string<char, std::char_traits<char>, allocator<char>>; ///< Ordinary allocator-aware container implementation.
  using base::base;
  /// Replace the destination with an empty container bound to this request.
  /// @param state Owner alive until destination destruction.
  void bind(allocation_state* state) {
    static_cast<base&>(*this) = base(allocator<char>{state});
  }
};
/// Vector whose Glaze reader binds its request before any growth.
template <class T> struct sequence : std::vector<T, allocator<T>> {
  using base = std::vector<T, allocator<T>>; ///< Ordinary allocator-aware container implementation.
  using base::base;
  /// Replace the destination with an empty container bound to this request.
  /// @param state Owner alive until destination destruction.
  void bind(allocation_state* state) {
    static_cast<base&>(*this) = base(allocator<T>{state});
  }
};
/// Decode-only graph preserving all seven original ordered alternatives.
struct decode_node {
  using array  = sequence<decode_node>;                  ///< Ordered array values.
  using object = sequence<std::pair<text, decode_node>>; ///< Ordered object pairs retaining duplicates.
  using variant =
      std::variant<std::nullptr_t, bool, std::int64_t, double, text, array, object>; ///< Unchanged Glaze alternative order.
  variant data;                                                                      ///< Library-selected parsed value.
};
/// Request-local parser policy and allocator binding; never shared between calls.
struct decode_context : glz::context {
  std::size_t       nodes = 0;                  ///< Values visited including speculative alternatives.
  std::size_t       max_nodes;                  ///< Maximum visited values.
  std::size_t       max_depth;                  ///< Maximum nested containers.
  const char*       failure_position = nullptr; ///< Furthest failure before variant rewind.
  allocation_state* allocations;                ///< Request owner alive through graph destruction.
};

/// Library-owned recursive structural value, retaining every object key.
struct node {
  /// Ordered array payload.
  using array = std::vector<node>;
  /// Object payload preserving duplicate keys until validation.
  using object = std::vector<std::pair<std::string, node>>;
  /// Library-discriminated JSON alternatives.
  using variant = std::variant<std::nullptr_t, bool, std::int64_t, double, std::string, array, object>;
  variant data; ///< Parsed or converted value.
};
/// Internal typed propagation through Glaze callbacks.
struct refusal {
  std::string_view code;               ///< Stable refusal category.
  std::string_view limit;              ///< Optional resource name.
  std::size_t      maximum  = 0;       ///< Optional resource maximum.
  const char*      position = nullptr; ///< Input position for decoding failures.
};
/// Per-call parser state; no shared mutable state.
struct context : glz::context {
  std::size_t nodes = 0;                  ///< Values visited by this read.
  std::size_t max_nodes;                  ///< Maximum visited values.
  std::size_t max_depth;                  ///< Maximum nested containers.
  const char* failure_position = nullptr; ///< Furthest library failure before variant rewind.
};
/// Strict JSON reading and library-controlled escaping.
struct options : glz::opts {
  bool validate_trailing_whitespace = true; ///< Require complete input consumption.
  bool validate_utf8                = true; ///< Reject malformed scalar UTF-8.
  bool escape_control_characters    = true; ///< Escape C0 string bytes.
};
constexpr options opts{};
// Pinned libc++ aligns (size+1) to max(new alignment, pointer size), with
// at most two extra bytes for its exceptional short/long capacity boundary.
// Match the pinned libc++ short-flag capacity encoding on both supported layouts.
#if defined(_LIBCPP_ABI_ALTERNATE_STRING_LAYOUT) != defined(_LIBCPP_BIG_ENDIAN)
constexpr std::size_t string_endian_factor = 1;
#else
constexpr std::size_t string_endian_factor = 2;
#endif
constexpr std::size_t string_capacity_slack = std::max<std::size_t>(__STDCPP_DEFAULT_NEW_ALIGNMENT__, sizeof(void*)) + 2;
// The approved B0 bound relies on exactly version1, <=64 ASCII IDs <=64 bytes,
// <=1 MiB per text and <=16 MiB aggregate. Array separators flush at 32768;
// reflected section fields do not flush. One whole section needs <=6 MiB+101,
// and round16(9+256)=272 field preflight plus closing syntax fits four 256-byte
// padding units. Hence 32768 + 6 MiB + 1024. Glaze owns all JSON/escaping.
constexpr std::size_t projection_scratch_bytes = 32768 + 6 * 1048576 + 1024;
/// Nonthrowing sticky sink: never dereferences bytes or cancels a noexcept flush.
struct projection_sink {
  std::size_t count    = 0;        ///< Successfully counted bytes before sticky refusal.
  std::size_t maximum  = 16777216; ///< Tightened output ceiling.
  bool        overflow = false;    ///< Once set, all subsequent chunks are ignored.
  /// Consume a complete chunk without accessing its bytes.
  /// @param n Logical byte count supplied by the bounded buffer.
  void write(std::size_t n) noexcept {
    if (overflow)
      return;
    if (n > maximum - count) {
      overflow = true;
      return;
    }
    count += n;
  }
};
/// Fixed streaming storage; Glaze bounded checks precede every write, without resize.
struct projection_buffer {
  using value_type = char;            ///< Pinned Glaze character buffer trait.
  std::vector<char> storage;          ///< One fixed allocation, never resized after writing starts.
  projection_sink*  output = nullptr; ///< Workspace-owned sink alive throughout writing.
  std::size_t       offset = 0;       ///< Already consumed logical prefix.
  /// Access an index after Glaze has checked bounded capacity.
  /// @param i Absolute logical index.
  /// @return Corresponding scratch byte.
  char& operator[](std::size_t i) noexcept {
    return storage[i - offset];
  }
  /// @return Physical scratch beginning.
  char* begin() noexcept {
    return storage.data();
  }
  /// @return Physical scratch end.
  char* end() noexcept {
    return storage.data() + storage.size();
  }
  /// @return Logical capacity including the consumed prefix.
  std::size_t size() const noexcept {
    return offset + storage.size();
  }
  /// Consume a pending chunk at the pinned separator threshold.
  /// @param n Absolute logical end of written bytes.
  void flush(std::size_t n) noexcept {
    if (n - offset >= 32768) {
      output->write(n - offset);
      offset = n;
    }
  }
  /// Consume the final partial chunk after successful writing.
  /// @param n Absolute logical end of written bytes.
  void finalize(std::size_t n) noexcept {
    if (n > offset) {
      output->write(n - offset);
      offset = n;
    }
  }
};
} // namespace planar_codec_detail
namespace glz {
/// Bounded storage prevents the stock stream adapter's absolute-index growth.
template <> struct buffer_traits<planar_codec_detail::projection_buffer> {
  static constexpr bool is_resizable         = false; ///< No growth API; Glaze must use bounded checks.
  static constexpr bool has_bounded_capacity = true;  ///< Fixed physical scratch.
  static constexpr bool is_output_streaming  = true;  ///< Separators may flush consumed prefixes.
  /// @param b Call-local buffer.
  /// @return Absolute logical capacity.
  static std::size_t capacity(const planar_codec_detail::projection_buffer& b) noexcept {
    return b.size();
  }
  /// Consume a chunk at an array separator.
  /// @param b Call-local buffer.
  /// @param n Absolute written byte end.
  static void flush(planar_codec_detail::projection_buffer& b, std::size_t n) noexcept {
    b.flush(n);
  }
  /// Consume final bytes after writer success.
  /// @param b Call-local buffer.
  /// @param n Absolute written byte end.
  static void finalize(planar_codec_detail::projection_buffer& b, std::size_t n) noexcept {
    b.finalize(n);
  }
};
/// Bind a string before delegating its complete grammar to the pinned reader.
template <> struct from<JSON, planar_codec_detail::text> {
  /// @brief Read with the original grammar and a bound request owner.
  /// @param value Destination value.
  /// @param ctx Request-local parser and allocation state.
  /// @param it Current library input iterator.
  /// @param end Input end.
  template <auto Opts> static void op(auto& value, auto& ctx, auto& it, auto end) {
    value.bind(ctx.allocations);
    from<JSON, planar_codec_detail::text::base>::template op<Opts>(static_cast<planar_codec_detail::text::base&>(value), ctx, it,
                                                                   end);
  }
};
/// Bind a vector before delegating its complete grammar to the pinned reader.
template <class T> struct from<JSON, planar_codec_detail::sequence<T>> {
  /// @brief Read with the original grammar and a bound request owner.
  /// @param value Destination value.
  /// @param ctx Request-local parser and allocation state.
  /// @param it Current library input iterator.
  /// @param end Input end.
  template <auto Opts> static void op(auto& value, auto& ctx, auto& it, auto end) {
    value.bind(ctx.allocations);
    using base = typename planar_codec_detail::sequence<T>::base; ///< Ordinary allocator-aware container implementation.
    from<JSON, base>::template op<Opts>(static_cast<base&>(value), ctx, it, end);
  }
};
/// Preserve original structural checks around the request-aware Glaze variant.
template <> struct from<JSON, planar_codec_detail::decode_node> {
  /// @brief Read with the original grammar and a bound request owner.
  /// @param value Destination value.
  /// @param ctx Request-local parser and allocation state.
  /// @param it Current library input iterator.
  /// @param end Input end.
  template <auto Opts> static void op(auto& value, auto& ctx, auto& it, auto end) {
    using namespace planar_codec_detail;
    if (++ctx.nodes > ctx.max_nodes)
      throw refusal{"resource_limit", "json_nodes", ctx.max_nodes, it};
    if (skip_ws<Opts>(ctx, it, end))
      return;
    if (it == end)
      return;
    if ((*it == '{' || *it == '[') && ctx.depth >= ctx.max_depth)
      throw refusal{"resource_limit", "json_depth", ctx.max_depth, it};
    const char* start = it;
    parse<JSON>::op<Opts>(value.data, ctx, it, end);
    if (bool(ctx.error) && ctx.error != error_code::end_reached) {
      const auto position = std::max(start, it);
      if (!ctx.failure_position || position > ctx.failure_position)
        ctx.failure_position = position;
      return;
    }
    if (std::holds_alternative<std::int64_t>(value.data)) {
      const std::string_view spelling(start, it - start);
      if (spelling.find_first_of(".eE") != spelling.npos)
        throw refusal{"schema", {}, 0, start};
      auto number = std::get<std::int64_t>(value.data);
      if (number < INT32_MIN || number > INT32_MAX)
        throw refusal{"schema", {}, 0, start};
    }
    if (std::holds_alternative<double>(value.data) || std::holds_alternative<std::nullptr_t>(value.data))
      throw refusal{"schema", {}, 0, start};
    if (auto s = std::get_if<planar_codec_detail::text>(&value.data); s && s->find('\0') != s->npos)
      throw refusal{"schema", {}, 0, start};
    if (auto object = std::get_if<decode_node::object>(&value.data)) {
      using keys_type = std::unordered_set<std::string_view, std::hash<std::string_view>, std::equal_to<std::string_view>,
                                           allocator<std::string_view>>;
      keys_type keys(0, std::hash<std::string_view>{}, std::equal_to<std::string_view>{},
                     allocator<std::string_view>{ctx.allocations});
      for (const auto& [key, ignored] : *object)
        if (key.find('\0') != key.npos || !keys.insert(std::string_view(key)).second)
          throw refusal{"schema", {}, 0, start};
    }
  }
};

/// Enforce structural ceilings around Glaze value parsing.
template <> struct from<JSON, planar_codec_detail::node> {
  /// @brief Parse one library value and apply native structural policy.
  /// @param value Destination node.
  /// @param ctx Per-call resource context.
  /// @param it Current input position.
  /// @param end Input end.
  template <auto Opts> static void op(auto& value, auto& ctx, auto& it, auto end) {
    using namespace planar_codec_detail;
    if (++ctx.nodes > ctx.max_nodes)
      throw refusal{"resource_limit", "json_nodes", ctx.max_nodes, it};
    if (skip_ws<Opts>(ctx, it, end))
      return;
    if (it == end)
      return;
    if ((*it == '{' || *it == '[') && ctx.depth >= ctx.max_depth)
      throw refusal{"resource_limit", "json_depth", ctx.max_depth, it};
    const char* start = it;
    parse<JSON>::op<Opts>(value.data, ctx, it, end);
    if (bool(ctx.error) && ctx.error != error_code::end_reached) {
      const auto position = std::max(start, it);
      if (!ctx.failure_position || position > ctx.failure_position)
        ctx.failure_position = position;
      return;
    }
    if (std::holds_alternative<std::int64_t>(value.data)) {
      const std::string_view spelling(start, it - start);
      if (spelling.find_first_of(".eE") != spelling.npos)
        throw refusal{"schema", {}, 0, start};
      auto number = std::get<std::int64_t>(value.data);
      if (number < INT32_MIN || number > INT32_MAX)
        throw refusal{"schema", {}, 0, start};
    }
    if (std::holds_alternative<double>(value.data) || std::holds_alternative<std::nullptr_t>(value.data))
      throw refusal{"schema", {}, 0, start};
    if (auto text = std::get_if<std::string>(&value.data); text && text->find('\0') != text->npos)
      throw refusal{"schema", {}, 0, start};
    if (auto object = std::get_if<node::object>(&value.data)) {
      std::unordered_set<std::string_view> keys;
      for (const auto& [key, ignored] : *object)
        if (key.find('\0') != key.npos || !keys.insert(key).second)
          throw refusal{"schema", {}, 0, start};
    }
  }
};
/// Delegate structural output to the ordinary Glaze variant writer.
template <> struct to<JSON, planar_codec_detail::node> {
  /// @brief Write one structural variant with Glaze.
  /// @param value Source node.
  /// @param ctx Library output context.
  /// @param buffer Bounded output span.
  /// @param index Current output position.
  template <auto Opts> static void op(const auto& value, auto& ctx, auto& buffer, auto& index) {
    serialize<JSON>::op<Opts>(value.data, ctx, buffer, index);
  }
};
} // namespace glz
module planar.engine.json_codec;
import std;
namespace planar::engine {
/// Fixed reflected projection envelope; named linkage is required by pinned Glaze.
struct projection_record {
  int                                      version = 1; ///< Sole supported projection output version.
  std::span<const projection_section_view> sections;    ///< Stable caller-owned section views.
};
namespace {
using planar_codec_detail::node;
using planar_codec_detail::refusal;
codec_error error(std::string_view code, std::string_view limit = {}, std::size_t maximum = 0, std::size_t offset = 0) {
  return {{std::string(code), "JSON codec refused input", std::string(limit), maximum}, offset};
}
bool text_with_nul(std::string_view text) {
  std::size_t start = 0;
  for (;;) {
    auto end = text.find('\0', start);
    if (!valid_text(text.substr(start, end == text.npos ? end : end - start)))
      return false;
    if (end == text.npos)
      return true;
    start = end + 1;
  }
}
value_ptr convert(const node& source) {
  auto result = std::make_shared<value>();
  std::visit(
      [&](const auto& item) {
        using T = std::decay_t<decltype(item)>;
        if constexpr (std::same_as<T, bool>) {
          result->kind    = value_kind::boolean;
          result->boolean = item;
        } else if constexpr (std::same_as<T, std::int64_t>) {
          result->kind    = value_kind::integer;
          result->integer = item;
        } else if constexpr (std::same_as<T, std::string>) {
          if (!valid_text(item))
            throw refusal{"schema", {}};
          result->kind   = value_kind::string;
          result->string = item;
        } else if constexpr (std::same_as<T, node::array>) {
          result->kind = value_kind::list;
          for (const auto& child : item)
            result->items.push_back(convert(child));
        } else if constexpr (std::same_as<T, node::object>) {
          result->kind = value_kind::record;
          for (const auto& [key, child] : item) {
            if (!valid_text(key))
              throw refusal{"schema", {}};
            result->fields.emplace(key, convert(child));
          }
        } else
          throw refusal{"schema", {}};
      },
      source.data);
  return result;
}
using decode_node = planar_codec_detail::decode_node;
// All owned fixed writer state is represented here for its sizeof reservation.
// Borrowed views themselves belong to the caller and are not copied or charged twice.
struct projection_workspace {
  projection_record                      input;
  planar_codec_detail::projection_sink   sink;
  planar_codec_detail::projection_buffer buffer;
  glz::context                           context{};
  codec_projection_reservation           reservation{};
};
bool projection_input_valid(std::span<const projection_section_view> sections) {
  if (sections.size() > 64)
    return false;
  std::size_t total = 0;
  for (std::size_t i = 0; i < sections.size(); ++i) {
    const auto& s = sections[i];
    if (s.id.empty() || s.id.size() > 64 || s.id.front() < 'a' || s.id.front() > 'z')
      return false;
    for (char c : s.id)
      if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'))
        return false;
    for (std::size_t j = 0; j < i; ++j)
      if (sections[j].id == s.id)
        return false;
    if (s.text.size() > 1048576 || s.text.size() > 16777216 - total || !valid_text(s.text))
      return false;
    total += s.text.size();
  }
  return true;
}
struct decode_hook_refusal {
  codec_error error;
};
struct decode_charge {
  codec_decode_reserver hook;
  static void           allocate(void* context, std::size_t bytes) {
    auto& self = *static_cast<decode_charge*>(context);
    if (self.hook.allocate) {
      auto result = self.hook.allocate(self.hook.context, bytes);
      if (!result)
        throw decode_hook_refusal{std::move(result.error())};
    }
  }
};
// Each frame owns one partially constructed ordinary node. The same fixed
// workspace is used by sizing and construction, with no detached mutable plan.
struct decode_workspace {
  struct frame {
    const decode_node*     source = nullptr;
    std::size_t            next   = 0;
    std::shared_ptr<value> result;
  };
  std::array<frame, 129>   frames{};
  std::size_t              depth = 0;
  codec_decode_reservation measured{};
  value_ptr                completed;
  static void              add(std::size_t& target, std::size_t n) {
    if (n > std::numeric_limits<std::size_t>::max() - target)
      throw refusal{"resource_limit", "input_bytes", 1048576};
    target += n;
  }
  static std::size_t multiply(std::size_t n, std::size_t size) {
    if (n && size > std::numeric_limits<std::size_t>::max() / n)
      throw refusal{"resource_limit", "input_bytes", 1048576};
    return n * size;
  }
  static std::size_t string_storage(std::size_t n) {
    // libc++ string __align_allocation_size; empty-string reserve selects this
    // exact allocation rather than the assignment amortized-growth path.
    const auto short_capacity = std::string{}.capacity();
    if (n <= short_capacity)
      return 0;
    constexpr auto alignment = std::max<std::size_t>(__STDCPP_DEFAULT_NEW_ALIGNMENT__, sizeof(void*));
    add(n, 1);
    add(n, alignment - 1);
    auto capacity = n & ~(alignment - 1);
    if (capacity == short_capacity + 2)
      add(capacity, planar_codec_detail::string_endian_factor);
    return capacity;
  }
  static std::size_t count(const decode_node& n) {
    if (auto a = std::get_if<decode_node::array>(&n.data))
      return a->size();
    if (auto o = std::get_if<decode_node::object>(&n.data))
      return o->size();
    return 0;
  }
  static const decode_node* child(const decode_node& n, std::size_t i) {
    if (auto a = std::get_if<decode_node::array>(&n.data))
      return &(*a)[i];
    return &std::get<decode_node::object>(n.data)[i].second;
  }
  codec_decode_reservation measure(const decode_node& root) {
    frames[0].source = &root;
    for (;;) {
      auto& f = frames[depth];
      if (f.next == 0) {
        add(measured.visits, 1);
        if (auto t = std::get_if<planar_codec_detail::text>(&f.source->data)) {
          if (!valid_text(*t))
            throw refusal{"schema", {}};
          add(measured.string_bytes, string_storage(t->size()));
        }
        if (auto a = std::get_if<decode_node::array>(&f.source->data))
          add(measured.list_slots, a->size());
        if (auto o = std::get_if<decode_node::object>(&f.source->data)) {
          add(measured.map_entries, o->size());
          for (const auto& [key, ignored] : *o) {
            if (!valid_text(key))
              throw refusal{"schema", {}};
            add(measured.string_bytes, string_storage(key.size()));
          }
        }
      }
      if (f.next < count(*f.source)) {
        const auto* next = child(*f.source, f.next++);
        ++depth;
        frames[depth].source = next;
        frames[depth].next   = 0;
      } else {
        if (!depth)
          break;
        --depth;
      }
    }
    // Pinned libc++ make_shared and map allocate precisely one of these rebound types.
    measured.node_bytes = multiply(measured.visits, sizeof(std::__shared_ptr_emplace<value, std::allocator<value>>));
    measured.map_bytes =
        multiply(measured.map_entries, sizeof(std::__tree_node<std::__value_type<std::string, value_ptr>, void*>));
    measured.list_bytes      = multiply(measured.list_slots, sizeof(value_ptr));
    measured.workspace_bytes = sizeof(decode_workspace);
    for (auto n : {measured.node_bytes, measured.map_bytes, measured.list_bytes, measured.string_bytes, measured.workspace_bytes})
      add(measured.total_bytes, n);
    return measured;
  }
  static void copy_string(std::string& target, std::string_view source) {
    target.reserve(source.size());
    target.assign(source.data(), source.size());
  }
  value_ptr build(const decode_node& root) {
    depth            = 0;
    frames[0].source = &root;
    frames[0].next   = 0;
    for (;;) {
      auto& f = frames[depth];
      if (!f.result) {
        f.result = std::make_shared<value>();
        std::visit(
            [&](const auto& item) {
              using T = std::decay_t<decltype(item)>;
              if constexpr (std::same_as<T, bool>) {
                f.result->kind    = value_kind::boolean;
                f.result->boolean = item;
              } else if constexpr (std::same_as<T, std::int64_t>) {
                f.result->kind    = value_kind::integer;
                f.result->integer = item;
              } else if constexpr (std::same_as<T, planar_codec_detail::text>) {
                f.result->kind = value_kind::string;
                copy_string(f.result->string, item);
              } else if constexpr (std::same_as<T, decode_node::array>) {
                f.result->kind = value_kind::list;
                f.result->items.reserve(item.size());
              } else if constexpr (std::same_as<T, decode_node::object>)
                f.result->kind = value_kind::record;
              else
                throw refusal{"schema", {}};
            },
            f.source->data);
      }
      if (f.next < count(*f.source)) {
        const auto* next = child(*f.source, f.next++);
        ++depth;
        frames[depth].source = next;
        frames[depth].next   = 0;
      } else {
        completed = std::move(f.result);
        if (!depth)
          return completed;
        --depth;
        auto& parent = frames[depth];
        if (parent.result->kind == value_kind::list)
          parent.result->items.push_back(std::move(completed));
        else {
          // Direct construction uses libc++ __init's exact rounded capacity,
          // including the initially empty/SSO case, with no intermediate key copy.
          const auto& key = std::get<decode_node::object>(parent.source->data)[parent.next - 1].first;
          parent.result->fields.emplace(std::piecewise_construct, std::forward_as_tuple(key.data(), key.size()),
                                        std::forward_as_tuple(std::move(completed)));
        }
      }
    }
  }
};
struct encoder {
  codec_limits                  limits;
  std::size_t                   nodes         = 0;
  std::size_t                   minimum_bytes = 0;
  codec_reservation             measured;
  std::array<const value*, 256> active{};
  static void                   add(std::size_t& target, std::size_t amount) {
    if (amount > std::numeric_limits<std::size_t>::max() - target)
      throw refusal{"resource_limit", "output_bytes", 16777216};
    target += amount;
  }
  static std::size_t multiply(std::size_t count, std::size_t size) {
    if (count && size > std::numeric_limits<std::size_t>::max() / count)
      throw refusal{"resource_limit", "output_bytes", 16777216};
    return count * size;
  }
  codec_reservation reservation() {
    measured.visits        = nodes;
    measured.minimum_bytes = minimum_bytes;
    measured.graph_bytes   = sizeof(node);
    add(measured.graph_bytes, multiply(measured.array_entries, sizeof(node)));
    add(measured.graph_bytes, multiply(measured.object_entries, sizeof(node::object::value_type)));
    measured.string_bytes = measured.key_bytes;
    add(measured.string_bytes, measured.value_bytes);
    auto strings = measured.object_entries;
    add(strings, measured.value_strings);
    add(measured.string_bytes, multiply(strings, planar_codec_detail::string_capacity_slack));
    // Conservative per-depth result/return node slots and local container metadata.
    // These overlap moved graph objects; they are logical allowance, not heap copies.
    measured.workspace_bytes = sizeof(encoder);
    add(measured.workspace_bytes, multiply(257, 2 * sizeof(node) + sizeof(node::object)));
    measured.output_capacity = multiply(minimum_bytes, 6);
    add(measured.output_capacity, 1024);
    measured.output_storage_bytes = measured.output_capacity;
    add(measured.output_storage_bytes, planar_codec_detail::string_capacity_slack);
    measured.total_bytes = measured.graph_bytes;
    add(measured.total_bytes, measured.string_bytes);
    add(measured.total_bytes, measured.workspace_bytes);
    add(measured.total_bytes, measured.output_storage_bytes);
    return measured;
  }
  void charge(std::size_t n) {
    if (n > limits.bytes - minimum_bytes)
      throw refusal{"resource_limit", "output_bytes", limits.bytes};
    minimum_bytes += n;
  }
  void validate(const value_ptr& v, std::size_t depth) {
    if (++nodes > limits.nodes)
      throw refusal{"resource_limit", "json_nodes", limits.nodes};
    if (!v)
      throw refusal{"internal", {}};
    switch (v->kind) {
    case value_kind::boolean:
      charge(v->boolean ? 4 : 5);
      return;
    case value_kind::integer:
      if (v->integer < INT32_MIN || v->integer > INT32_MAX)
        throw refusal{"internal", {}};
      {
        std::array<char, 12> spelling{};
        auto                 converted = std::to_chars(spelling.data(), spelling.data() + spelling.size(), v->integer);
        charge(converted.ptr - spelling.data());
      }
      return;
    case value_kind::string:
      charge(2);
      charge(v->string.size());
      add(measured.value_strings, 1);
      add(measured.value_bytes, v->string.size());
      if (!text_with_nul(v->string))
        throw refusal{"internal", {}};
      return;
    case value_kind::record:
    case value_kind::list:
      break;
    default:
      throw refusal{"internal", {}};
    }
    if (depth >= limits.depth)
      throw refusal{"resource_limit", "json_depth", limits.depth};
    if (std::find(active.begin(), active.begin() + depth, v.get()) != active.begin() + depth)
      throw refusal{"internal", {}};
    active[depth] = v.get();
    charge(2);
    bool first = true;
    auto comma = [&] {
      if (!first)
        charge(1);
      first = false;
    };
    if (v->kind == value_kind::record) {
      for (const auto& [key, child] : v->fields) {
        comma();
        add(measured.object_entries, 1);
        add(measured.key_bytes, key.size());
        charge(3);
        charge(key.size());
        if (!text_with_nul(key))
          throw refusal{"internal", {}};
        validate(child, depth + 1);
      }
    } else
      for (const auto& child : v->items) {
        comma();
        add(measured.array_entries, 1);
        validate(child, depth + 1);
      }
    active[depth] = nullptr;
  }
  node build(const value_ptr& v) {
    node result;
    switch (v->kind) {
    case value_kind::boolean:
      result.data = v->boolean;
      break;
    case value_kind::integer:
      result.data = v->integer;
      break;
    case value_kind::string:
      result.data = v->string;
      break;
    case value_kind::list: {
      node::array items;
      items.reserve(v->items.size());
      for (const auto& child : v->items)
        items.push_back(build(child));
      result.data = std::move(items);
      break;
    }
    case value_kind::record: {
      node::object fields;
      fields.reserve(v->fields.size());
      for (const auto& [key, child] : v->fields)
        fields.emplace_back(key, build(child));
      std::ranges::sort(fields, [](const auto& a, const auto& b) {
        return std::lexicographical_compare(a.first.begin(), a.first.end(), b.first.begin(), b.first.end(),
                                            [](unsigned char x, unsigned char y) { return x < y; });
      });
      result.data = std::move(fields);
      break;
    }
    default:
      throw refusal{"internal", {}};
    }
    return result;
  }
};
} // namespace
std::expected<std::size_t, codec_projection_error> codec_projection_count(std::span<const projection_section_view> sections,
                                                                          codec_projection_reserver                reserver,
                                                                          std::size_t maximum_bytes) {
  using enum codec_projection_failure;
  maximum_bytes   = std::min<std::size_t>(maximum_bytes, 16777216);
  const auto fail = [&](codec_projection_failure kind, std::string_view code) {
    return std::unexpected(codec_projection_error{
        kind, error(code, code == "resource_limit" ? "output_bytes" : "", code == "resource_limit" ? maximum_bytes : 0)});
  };
  // Both handlers and diagnostic construction remain inside allocation adaptation.
  try {
    try {
      if (!projection_input_valid(sections))
        return fail(invalid_input, "schema");
      projection_workspace state;
      state.input.sections = sections;
      state.sink.maximum   = maximum_bytes;
      state.buffer.output  = &state.sink;
      state.reservation    = {planar_codec_detail::projection_scratch_bytes, sizeof(state), 0};
      if (state.reservation.scratch_bytes > std::numeric_limits<std::size_t>::max() - state.reservation.workspace_bytes)
        return fail(internal, "internal");
      state.reservation.total_bytes = state.reservation.scratch_bytes + state.reservation.workspace_bytes;
      if (reserver.reserve) {
        auto reserved = reserver.reserve(reserver.context, state.reservation);
        if (!reserved)
          return std::unexpected(codec_projection_error{resource, {std::move(reserved.error()), 0}});
      }
      state.buffer.storage.resize(planar_codec_detail::projection_scratch_bytes);
      const auto status = glz::write<planar_codec_detail::opts>(state.input, state.buffer, state.context);
      if (status)
        return fail(scratch_defect, "internal");
      if (state.sink.overflow)
        return fail(output_limit, "resource_limit");
      return state.sink.count;
    } catch (const std::bad_alloc&) {
      return fail(resource, "resource_limit");
    } catch (const std::length_error&) {
      return fail(resource, "resource_limit");
    } catch (...) {
      return fail(internal, "internal");
    }
  } catch (const std::bad_alloc&) {
    return fail(resource, "resource_limit");
  } catch (const std::length_error&) {
    return fail(resource, "resource_limit");
  }
}
std::expected<value_ptr, codec_error> codec_decode(std::string_view input, codec_limits limits) {
  limits.bytes = std::min<std::size_t>(limits.bytes, 1048576);
  limits.nodes = std::min<std::size_t>(limits.nodes, 100000);
  try {
    try {
      if (input.size() > limits.bytes)
        return std::unexpected(error("resource_limit", "input_bytes", limits.bytes));
      // The library also has a hard recursion ceiling of 256; callers may only tighten it.
      limits.depth = std::min<std::size_t>(limits.depth, 128);
      node                         parsed;
      planar_codec_detail::context ctx{};
      ctx.max_nodes = limits.nodes;
      ctx.max_depth = limits.depth;
      auto status   = glz::read<planar_codec_detail::opts>(parsed, input, ctx);
      if (status.ec == glz::error_code::exceeded_max_recursive_depth)
        return std::unexpected(error("resource_limit", "json_depth", limits.depth, status.count));
      if (status)
        return std::unexpected(error("syntax", {}, 0, ctx.failure_position ? ctx.failure_position - input.data() : status.count));
      // The range-of-pairs reader preserves duplicate keys but accepts loose object
      // separators. Validate the same bytes with Glaze's strict grammar before
      // conversion. Structural/schema refusals above retain precedence, and bound
      // this second pass below Glaze's independent recursion ceiling.
      auto syntax = glz::validate_json(input);
      if (syntax.ec == glz::error_code::exceeded_max_recursive_depth)
        return std::unexpected(error("resource_limit", "json_depth", limits.depth, syntax.count));
      if (syntax)
        return std::unexpected(error("syntax", {}, 0, syntax.count));
      return convert(parsed);
    } catch (const refusal& r) {
      return std::unexpected(error(r.code, r.limit, r.maximum, r.position ? r.position - input.data() : 0));
    } catch (const std::bad_alloc&) {
      return std::unexpected(error("resource_limit", "input_bytes", limits.bytes));
    } catch (const std::length_error&) {
      return std::unexpected(error("resource_limit", "input_bytes", limits.bytes));
    } catch (...) {
      return std::unexpected(error("internal"));
    }
  } catch (const std::bad_alloc&) {
    return std::unexpected(error("resource_limit", "input_bytes", limits.bytes));
  } catch (const std::length_error&) {
    return std::unexpected(error("resource_limit", "input_bytes", limits.bytes));
  }
}
std::expected<value_ptr, codec_error> codec_decode_reserved(std::string_view input, codec_decode_reserver reserver,
                                                            codec_limits limits) {
  limits.bytes = std::min<std::size_t>(limits.bytes, 1048576);
  limits.nodes = std::min<std::size_t>(limits.nodes, 100000);
  try {
    try {
      if (input.size() > limits.bytes)
        return std::unexpected(error("resource_limit", "input_bytes", limits.bytes));
      // The library also has a hard recursion ceiling of 256; callers may only tighten it.
      limits.depth = std::min<std::size_t>(limits.depth, 128);
      decode_charge                         charge{reserver};
      planar_codec_detail::allocation_state allocations{&charge, decode_charge::allocate};
      decode_node                           parsed;
      planar_codec_detail::decode_context   ctx{};
      ctx.allocations = &allocations;
      ctx.max_nodes   = limits.nodes;
      ctx.max_depth   = limits.depth;
      auto status     = glz::read<planar_codec_detail::opts>(parsed, input, ctx);
      if (status.ec == glz::error_code::exceeded_max_recursive_depth)
        return std::unexpected(error("resource_limit", "json_depth", limits.depth, status.count));
      if (status)
        return std::unexpected(error("syntax", {}, 0, ctx.failure_position ? ctx.failure_position - input.data() : status.count));
      // The range-of-pairs reader preserves duplicate keys but accepts loose object
      // separators. Validate the same bytes with Glaze's strict grammar before
      // conversion. Structural/schema refusals above retain precedence, and bound
      // this second pass below Glaze's independent recursion ceiling.
      auto syntax = glz::validate_json(input);
      if (syntax.ec == glz::error_code::exceeded_max_recursive_depth)
        return std::unexpected(error("resource_limit", "json_depth", limits.depth, syntax.count));
      if (syntax)
        return std::unexpected(error("syntax", {}, 0, syntax.count));
      decode_workspace workspace;
      const auto       measured = workspace.measure(parsed);
      if (reserver.convert) {
        auto reserved = reserver.convert(reserver.context, measured);
        if (!reserved)
          return std::unexpected(std::move(reserved.error()));
      }
      return workspace.build(parsed);
    } catch (decode_hook_refusal& r) {
      return std::unexpected(std::move(r.error));
    } catch (const refusal& r) {
      return std::unexpected(error(r.code, r.limit, r.maximum, r.position ? r.position - input.data() : 0));
    } catch (const std::bad_alloc&) {
      return std::unexpected(error("resource_limit", "input_bytes", limits.bytes));
    } catch (const std::length_error&) {
      return std::unexpected(error("resource_limit", "input_bytes", limits.bytes));
    } catch (...) {
      return std::unexpected(error("internal"));
    }
  } catch (const std::bad_alloc&) {
    return std::unexpected(error("resource_limit", "input_bytes", limits.bytes));
  } catch (const std::length_error&) {
    return std::unexpected(error("resource_limit", "input_bytes", limits.bytes));
  }
}
std::expected<std::string, codec_error> codec_encode(const value_ptr& input, codec_limits limits) {
  return codec_encode_reserved(input, {}, limits);
}
std::expected<std::string, codec_error> codec_encode_reserved(const value_ptr& input, codec_reserver reserver,
                                                              codec_limits limits) {
  limits.bytes = std::min<std::size_t>(limits.bytes, 16777216);
  limits.depth = std::min<std::size_t>(limits.depth, 256);
  limits.nodes = std::min<std::size_t>(limits.nodes, 4000000);
  // Handler bodies may allocate while adapting a refusal or callback exception.
  // Keep their diagnostic construction inside the same typed allocation boundary.
  try {
    try {
      encoder state{limits, 0, 0, {}, {}};
      state.validate(input, 0);
      const auto capacity = state.reservation();
      if (reserver.reserve) {
        auto reserved = reserver.reserve(reserver.context, capacity);
        if (!reserved)
          return std::unexpected(codec_error{std::move(reserved.error()), 0});
      }
      auto            parsed = state.build(input);
      std::string     output(capacity.output_capacity, '\0');
      std::span<char> buffer(output.data(), output.size());
      auto            status = glz::write<planar_codec_detail::opts>(parsed, buffer);
      if (status || status.count > limits.bytes)
        return std::unexpected(error("resource_limit", "output_bytes", limits.bytes));
      output.resize(status.count);
      return output;
    } catch (const refusal& r) {
      return std::unexpected(error(r.code, r.limit, r.maximum));
    } catch (const std::bad_alloc&) {
      return std::unexpected(error("resource_limit", "output_bytes", limits.bytes));
    } catch (const std::length_error&) {
      return std::unexpected(error("resource_limit", "output_bytes", limits.bytes));
    } catch (...) {
      return std::unexpected(error("internal"));
    }
  } catch (const std::bad_alloc&) {
    return std::unexpected(error("resource_limit", "output_bytes", limits.bytes));
  } catch (const std::length_error&) {
    return std::unexpected(error("resource_limit", "output_bytes", limits.bytes));
  }
}
} // namespace planar::engine
