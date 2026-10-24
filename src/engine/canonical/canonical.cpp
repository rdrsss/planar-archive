module planar.engine.canonical;
import std;
namespace planar::engine {
namespace {
[[noreturn]] void invalid() {
  throw diagnostic{"schema", "Invalid canonical value"};
}
[[noreturn]] void exhausted(std::string name, std::size_t maximum) {
  throw diagnostic{"resource_limit", "Canonical encoding limit exceeded", std::move(name), maximum};
}
struct encoder {
  canonical_limits                 limits;
  std::string                      out;
  std::size_t                      nodes = 0;
  std::unordered_set<const value*> active;
  void                             room(std::size_t size) {
    if (size > limits.bytes - out.size())
      exhausted("canonical_bytes", limits.bytes);
  }
  void byte(unsigned char b) {
    room(1);
    out += static_cast<char>(b);
  }
  void wide(std::uint64_t n) {
    room(8);
    for (int shift = 56; shift >= 0; shift -= 8)
      out += static_cast<char>((n >> shift) & 255);
  }
  void visit(std::size_t depth) {
    if (depth > limits.depth)
      exhausted("canonical_depth", limits.depth);
    if (++nodes > limits.nodes)
      exhausted("canonical_nodes", limits.nodes);
  }
  void string(std::string_view bytes) {
    byte(3);
    wide(bytes.size());
    room(bytes.size());
    out.append(bytes);
  }
  void encode(const value_ptr& p, std::size_t depth) {
    visit(depth);
    if (!p || !active.insert(p.get()).second)
      invalid();
    const auto& v = *p;
    // Refuse unused payloads so malformed native trees cannot acquire ambiguous identities.
    if ((v.kind != value_kind::boolean && v.boolean) || (v.kind != value_kind::integer && v.integer != 0) ||
        (v.kind != value_kind::string && !v.string.empty()) || (v.kind != value_kind::record && !v.fields.empty()) ||
        (v.kind != value_kind::list && !v.items.empty()))
      invalid();
    switch (v.kind) {
    case value_kind::boolean:
      byte(v.boolean ? 1 : 0);
      break;
    case value_kind::integer:
      if (v.integer < -2147483648LL || v.integer > 2147483647LL)
        invalid();
      byte(2);
      wide(static_cast<std::uint64_t>(v.integer));
      break;
    case value_kind::string:
      string(v.string);
      break;
    case value_kind::list:
      if (v.items.size() > limits.nodes - nodes)
        exhausted("canonical_nodes", limits.nodes);
      byte(4);
      wide(v.items.size());
      for (const auto& item : v.items)
        encode(item, depth + 1);
      break;
    case value_kind::record: {
      if (v.fields.size() > (limits.nodes - nodes) / 2)
        exhausted("canonical_nodes", limits.nodes);
      byte(5);
      wide(v.fields.size());
      std::vector<const std::pair<const std::string, value_ptr>*> fields;
      fields.reserve(v.fields.size());
      std::size_t key_bytes = 0;
      for (const auto& entry : v.fields) {
        // Bound text validation before scanning keys; the sorted view is bounded by the node ceiling.
        if (entry.first.size() > limits.bytes - key_bytes)
          exhausted("canonical_bytes", limits.bytes);
        key_bytes += entry.first.size();
        if (!valid_text(entry.first))
          invalid();
        fields.push_back(&entry);
      }
      std::ranges::sort(fields, [](const auto* a, const auto* b) {
        return std::lexicographical_compare(a->first.begin(), a->first.end(), b->first.begin(), b->first.end(),
                                            [](unsigned char x, unsigned char y) { return x < y; });
      });
      for (const auto* entry : fields) {
        visit(depth + 1);
        string(entry->first);
        encode(entry->second, depth + 1);
      }
      break;
    }
    default:
      invalid();
    }
    active.erase(p.get());
  }
};
struct decoder {
  std::string_view input;
  canonical_limits limits;
  std::size_t      position = 0;
  std::size_t      nodes    = 0;
  std::uint64_t    wide() {
    if (input.size() - position < 8)
      invalid();
    std::uint64_t n = 0;
    for (int i = 0; i < 8; ++i)
      n = (n << 8) | static_cast<unsigned char>(input[position++]);
    return n;
  }
  std::string_view string() {
    auto size = wide();
    if (size > input.size() - position)
      invalid();
    auto result = input.substr(position, static_cast<std::size_t>(size));
    position += static_cast<std::size_t>(size);
    return result;
  }
  value_ptr decode(std::size_t depth) {
    if (depth > limits.depth)
      exhausted("canonical_depth", limits.depth);
    if (++nodes > limits.nodes)
      exhausted("canonical_nodes", limits.nodes);
    if (position == input.size())
      invalid();
    auto tag = static_cast<unsigned char>(input[position++]);
    auto p   = std::make_shared<value>();
    switch (tag) {
    case 0:
    case 1:
      p->kind    = value_kind::boolean;
      p->boolean = tag == 1;
      break;
    case 2: {
      auto bits   = wide();
      auto number = std::bit_cast<std::int64_t>(bits);
      if (number < -2147483648LL || number > 2147483647LL)
        invalid();
      p->kind    = value_kind::integer;
      p->integer = number;
      break;
    }
    case 3:
      p->kind   = value_kind::string;
      p->string = string();
      break;
    case 4:
    case 5: {
      auto count      = wide();
      bool map        = tag == 5;
      auto multiplier = map ? 2u : 1u;
      if (count > (limits.nodes - nodes) / multiplier)
        exhausted("canonical_nodes", limits.nodes);
      // A list item needs at least one tag; a map pair needs a string header and a value tag.
      if (count > (input.size() - position) / (map ? 10u : 1u))
        invalid();
      p->kind = map ? value_kind::record : value_kind::list;
      std::string_view previous;
      for (std::uint64_t i = 0; i < count; ++i) {
        if (!map) {
          p->items.push_back(decode(depth + 1));
          continue;
        }
        if (depth + 1 > limits.depth)
          exhausted("canonical_depth", limits.depth);
        if (++nodes > limits.nodes)
          exhausted("canonical_nodes", limits.nodes);
        if (position == input.size() || input[position++] != 3)
          invalid();
        auto key = string();
        if (!valid_text(key) || (i != 0 && !std::lexicographical_compare(previous.begin(), previous.end(), key.begin(), key.end(),
                                                                         [](unsigned char x, unsigned char y) { return x < y; })))
          invalid();
        previous   = key;
        auto child = decode(depth + 1);
        p->fields.emplace(std::string(key), std::move(child));
      }
      break;
    }
    default:
      invalid();
    }
    return p;
  }
};
} // namespace
std::expected<std::string, diagnostic> canonical_encode(const value_ptr& input, canonical_limits limits) {
  limits.depth = std::min(limits.depth, std::size_t{256});
  limits.nodes = std::min(limits.nodes, std::size_t{4000000});
  limits.bytes = std::min(limits.bytes, std::size_t{268435456});
  try {
    encoder e{limits, {}, 0, {}};
    e.encode(input, 1);
    return std::move(e.out);
  } catch (const diagnostic& d) {
    return std::unexpected(d);
  } catch (const std::bad_alloc&) {
    return std::unexpected(diagnostic{"resource_limit", "Canonical allocation failed", "canonical_bytes", limits.bytes});
  } catch (const std::length_error&) {
    return std::unexpected(diagnostic{"resource_limit", "Canonical allocation failed", "canonical_bytes", limits.bytes});
  }
}
std::expected<value_ptr, diagnostic> canonical_decode(std::string_view bytes, canonical_limits limits) {
  limits.depth = std::min(limits.depth, std::size_t{256});
  limits.nodes = std::min(limits.nodes, std::size_t{4000000});
  limits.bytes = std::min(limits.bytes, std::size_t{268435456});
  try {
    if (bytes.size() > limits.bytes)
      exhausted("canonical_bytes", limits.bytes);
    decoder d{bytes, limits};
    auto    result = d.decode(1);
    if (d.position != bytes.size())
      invalid();
    return result;
  } catch (const diagnostic& d) {
    return std::unexpected(d);
  } catch (const std::bad_alloc&) {
    return std::unexpected(diagnostic{"resource_limit", "Canonical allocation failed", "canonical_bytes", limits.bytes});
  } catch (const std::length_error&) {
    return std::unexpected(diagnostic{"resource_limit", "Canonical allocation failed", "canonical_bytes", limits.bytes});
  }
}
} // namespace planar::engine
