module planar.engine.ledger.history_codec;
import std;
import planar.engine.json_codec;
namespace planar::engine::ledger {
namespace {
constexpr std::size_t maximum_bytes = 1048576;
using error                         = history_codec_error;
value_ptr text(std::string s) {
  value v{};
  v.kind   = value_kind::string;
  v.string = std::move(s);
  return std::make_shared<value>(std::move(v));
}
value_ptr object(std::map<std::string, value_ptr> fields) {
  value v{};
  v.kind   = value_kind::record;
  v.fields = std::move(fields);
  return std::make_shared<value>(std::move(v));
}
value_ptr version() {
  value v{};
  v.kind    = value_kind::integer;
  v.integer = 1;
  return std::make_shared<value>(std::move(v));
}
std::string id(std::optional<std::int64_t> n) {
  return n ? std::to_string(*n) : "none";
}
bool positive(std::optional<std::int64_t> n) {
  return !n || *n > 0;
}
bool valid(const history_event& e) {
  if (!positive(e.before) || !positive(e.after))
    return false;
  if (e.reason == event_reason::initialized)
    return !e.before && !e.after && !e.references;
  if (!e.references || !e.after)
    return false;
  const auto& r = *e.references;
  if (r.source <= 0 || r.registry <= 0 || r.stdlib <= 0 || r.interpretation <= 0 || r.versions <= 0)
    return false;
  if (e.reason == event_reason::declaration_accepted)
    return !e.before || *e.before < *e.after;
  return e.reason == event_reason::identical_declaration && e.before == e.after;
}
bool valid(const load_result& r) {
  if (r.ledger.size() != 32 ||
      !std::ranges::all_of(r.ledger, [](unsigned char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); }))
    return false;
  if (r.request_id.empty() || r.request_id.size() > 128 || !std::ranges::all_of(r.request_id, [](unsigned char c) {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '.' || c == ':' ||
               c == '-';
      }))
    return false;
  return (r.outcome == load_outcome::accepted || r.outcome == load_outcome::noop) && r.revision > 0 && r.event > 0 &&
         positive(r.parent) && (!r.parent || *r.parent < r.revision);
}
std::expected<std::string, error> encode(const value_ptr& v) {
  auto out = codec_encode(v, {maximum_bytes, 8, 64});
  if (!out)
    return std::unexpected(out.error().detail.code == "resource_limit" ? error::resource_limit : error::malformed);
  return *out;
}
bool keys(const value_ptr& v, std::initializer_list<std::string_view> names) {
  if (!v || v->kind != value_kind::record || v->fields.size() != names.size())
    return false;
  return std::ranges::all_of(names, [&](auto k) { return v->fields.contains(std::string(k)); });
}
value_ptr at(const value_ptr& v, std::string_view k) {
  auto p = v->fields.find(std::string(k));
  return p == v->fields.end() ? nullptr : p->second;
}
std::optional<std::string> string(const value_ptr& v) {
  if (!v || v->kind != value_kind::string)
    return {};
  return v->string;
}
std::expected<std::optional<std::int64_t>, error> parse_id(const value_ptr& v, bool optional = false) {
  auto s = string(v);
  if (!s)
    return std::unexpected(error::malformed);
  if (optional && *s == "none")
    return std::optional<std::int64_t>{};
  if (s->empty() || (*s)[0] < '1' || (*s)[0] > '9')
    return std::unexpected(error::malformed);
  std::int64_t n = 0;
  auto [p, ec]   = std::from_chars(s->data(), s->data() + s->size(), n);
  if (ec != std::errc{} || p != s->data() + s->size() || n <= 0)
    return std::unexpected(error::malformed);
  return std::optional{n};
}
std::expected<value_ptr, error> decode(std::string_view bytes) {
  if (bytes.size() > maximum_bytes)
    return std::unexpected(error::resource_limit);
  auto v = codec_decode(bytes, {maximum_bytes, 8, 64});
  // Structural depth/node exhaustion within this fixed schema is malformed, not a larger allowed payload.
  if (!v || (*v)->kind != value_kind::record)
    return std::unexpected(error::malformed);
  auto ver = at(*v, "version");
  if (!ver || ver->kind != value_kind::integer)
    return std::unexpected(error::malformed);
  if (ver->integer != 1)
    return std::unexpected(error::unsupported_version);
  return *v;
}
} // namespace
std::expected<std::string, error> encode_history_event(const history_event& e) {
  if (!valid(e))
    return std::unexpected(error::malformed);
  auto refs = object({});
  if (e.references) {
    const auto& r = *e.references;
    refs          = object({{"source", text(id(r.source))},
                            {"registry", text(id(r.registry))},
                            {"stdlib", text(id(r.stdlib))},
                            {"interpretation", text(id(r.interpretation))},
                            {"versions", text(id(r.versions))}});
  }
  auto reason = e.reason == event_reason::initialized            ? "initialized"
                : e.reason == event_reason::declaration_accepted ? "declaration_accepted"
                                                                 : "identical_declaration";
  return encode(object({{"version", version()},
                        {"before", object({{"active_revision", text(id(e.before))}})},
                        {"after", object({{"active_revision", text(id(e.after))}})},
                        {"reason", text(reason)},
                        {"references", refs}}));
}
std::expected<history_event, error> decode_history_event(std::string_view bytes) {
  auto parsed = decode(bytes);
  if (!parsed)
    return std::unexpected(parsed.error());
  auto v = *parsed;
  if (!keys(v, {"version", "before", "after", "reason", "references"}) || !keys(at(v, "before"), {"active_revision"}) ||
      !keys(at(v, "after"), {"active_revision"}))
    return std::unexpected(error::malformed);
  auto before = parse_id(at(at(v, "before"), "active_revision"), true),
       after  = parse_id(at(at(v, "after"), "active_revision"), true);
  auto reason = string(at(v, "reason"));
  if (!before || !after || !reason)
    return std::unexpected(error::malformed);
  event_reason kind;
  if (*reason == "initialized")
    kind = event_reason::initialized;
  else if (*reason == "declaration_accepted")
    kind = event_reason::declaration_accepted;
  else if (*reason == "identical_declaration")
    kind = event_reason::identical_declaration;
  else
    return std::unexpected(error::malformed);
  history_event result{*before, *after, kind, {}};
  auto          refs = at(v, "references");
  if (kind == event_reason::initialized) {
    if (!keys(refs, {}))
      return std::unexpected(error::malformed);
  } else {
    if (!keys(refs, {"source", "registry", "stdlib", "interpretation", "versions"}))
      return std::unexpected(error::malformed);
    auto s = parse_id(at(refs, "source")), r = parse_id(at(refs, "registry")), t = parse_id(at(refs, "stdlib")),
         i = parse_id(at(refs, "interpretation")), z = parse_id(at(refs, "versions"));
    if (!s || !r || !t || !i || !z)
      return std::unexpected(error::malformed);
    result.references = event_references{**s, **r, **t, **i, **z};
  }
  auto encoded = encode_history_event(result);
  if (!encoded || *encoded != bytes)
    return std::unexpected(error::malformed);
  return result;
}
std::expected<std::string, error> encode_load_result(const load_result& r) {
  if (!valid(r))
    return std::unexpected(error::malformed);
  return encode(object({{"version", version()},
                        {"kind", text("load")},
                        {"ledger", text(r.ledger)},
                        {"request_id", text(r.request_id)},
                        {"outcome", text(r.outcome == load_outcome::accepted ? "accepted" : "noop")},
                        {"revision", text(id(r.revision))},
                        {"parent", text(id(r.parent))},
                        {"event", text(id(r.event))}}));
}
std::expected<load_result, error> decode_load_result(std::string_view bytes) {
  auto parsed = decode(bytes);
  if (!parsed)
    return std::unexpected(parsed.error());
  auto v = *parsed;
  if (!keys(v, {"version", "kind", "ledger", "request_id", "outcome", "revision", "parent", "event"}))
    return std::unexpected(error::malformed);
  auto kind = string(at(v, "kind")), ledger = string(at(v, "ledger")), request = string(at(v, "request_id")),
       outcome  = string(at(v, "outcome"));
  auto revision = parse_id(at(v, "revision")), parent = parse_id(at(v, "parent"), true), event = parse_id(at(v, "event"));
  if (!kind || *kind != "load" || !ledger || !request || !outcome || (*outcome != "accepted" && *outcome != "noop") ||
      !revision || !parent || !event)
    return std::unexpected(error::malformed);
  load_result result{*ledger,    *request, *outcome == "accepted" ? load_outcome::accepted : load_outcome::noop,
                     **revision, *parent,  **event};
  auto        encoded = encode_load_result(result);
  if (!encoded || *encoded != bytes)
    return std::unexpected(error::malformed);
  return result;
}
} // namespace planar::engine::ledger
