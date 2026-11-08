module;
#include <xxhash.h>
module planar.engine.history.execution_codec;
import std;
import planar.engine.canonical;
import planar.engine.hash;
import planar.engine.json_codec;
import planar.engine.value;

namespace planar::engine::history {
namespace {

// EX-8 and D49 ceilings. The JSON ceiling is defense in depth: valid fields cannot reach it.
constexpr std::size_t                      json_ceiling                = 1048576;
constexpr std::size_t                      manifest_ceiling            = 16777216;
constexpr std::size_t                      provenance_bytes_ceiling    = 65536;
constexpr std::size_t                      provenance_depth_ceiling    = 4;
constexpr std::size_t                      provenance_settings_ceiling = 32;
constexpr std::size_t                      provenance_string_ceiling   = 1024;
constexpr std::size_t                      value_record_ceiling        = 16777216;
constexpr std::uint64_t                    int32_maximum               = 2147483647ULL;
constexpr std::string_view                 record_magic                = "PLANAR-RECORD\x01";
constexpr unsigned char                    value_record_tag            = 3;
constexpr std::string_view                 provenance_authority        = "driver_assertion_unverified";
constexpr std::array<std::string_view, 6>  role_states{"blocked", "ready", "running", "done", "stale", "retired"};
constexpr std::array<std::string_view, 16> transition_reasons{"activated",
                                                              "prerequisite_available",
                                                              "prerequisite_unavailable",
                                                              "claimed",
                                                              "taken_over",
                                                              "fenced",
                                                              "released",
                                                              "interrupted",
                                                              "completed",
                                                              "work_changed",
                                                              "evidence_inapplicable",
                                                              "repair_admitted",
                                                              "structural_change",
                                                              "retired",
                                                              "recreated",
                                                              "drift_cleared"};
constexpr std::array<std::string_view, 4>  payload_operations{"load", "activate", "claim", "takeover"};

[[noreturn]] void invalid() {
  throw execution_codec_error{execution_codec_code::invalid, {}, 0};
}

[[noreturn]] void exceeded(std::string_view name, std::uint64_t maximum) {
  throw execution_codec_error{execution_codec_code::resource_limit, std::string(name), maximum};
}

// Allocation failure is not an EX-8 bound, so it is reported as an invalid encoding with no limit name.
template <class T, class F> std::expected<T, execution_codec_error> guarded(F&& f) {
  try {
    if constexpr (std::is_void_v<T>) {
      f();
      return {};
    } else {
      return f();
    }
  } catch (const execution_codec_error& e) {
    return std::unexpected(e);
  } catch (const std::bad_alloc&) {
    return std::unexpected(execution_codec_error{execution_codec_code::invalid, {}, 0});
  } catch (const std::length_error&) {
    return std::unexpected(execution_codec_error{execution_codec_code::invalid, {}, 0});
  }
}

// ---- grammars ----------------------------------------------------------------------------------------------------

template <std::size_t N> bool one_of(std::string_view text, const std::array<std::string_view, N>& options) {
  return std::ranges::find(options, text) != options.end();
}

bool lower_hex(std::string_view s) {
  return std::ranges::all_of(s, [](char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); });
}

// Canonical positive unsigned 64-bit decimal: no sign, no leading zero, and zero is refused.
bool positive_decimal(std::string_view s) {
  if (s.empty() || s.front() < '1' || s.front() > '9')
    return false;
  constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();
  std::uint64_t  n       = 0;
  for (char c : s) {
    if (c < '0' || c > '9')
      return false;
    const auto digit = static_cast<std::uint64_t>(c - '0');
    if (n > (maximum - digit) / 10)
      return false;
    n = n * 10 + digit;
  }
  return true;
}

// Identifier: a lowercase letter followed by lowercase letters, digits or underscores, at most 64 bytes.
bool identifier(std::string_view s) {
  if (s.empty() || s.size() > 64 || s.front() < 'a' || s.front() > 'z')
    return false;
  return std::ranges::all_of(s, [](char c) { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'; });
}

bool ledger_text(std::string_view s) {
  return s.size() == 32 && lower_hex(s);
}

// Request token grammar shared with the load result of ledger/history_codec.
bool request_token(std::string_view s) {
  return !s.empty() && s.size() <= 128 && std::ranges::all_of(s, [](char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '.' || c == ':' ||
           c == '-';
  });
}

// Digest text: the xxh3_128 prefix and exactly thirty-two lowercase hexadecimal digits.
bool fingerprint_text(std::string_view s) {
  constexpr std::string_view prefix = "xxh3_128:";
  return s.starts_with(prefix) && s.size() == prefix.size() + 32 && lower_hex(s.substr(prefix.size()));
}

// Run token: the run ID, the generation ordinal and thirty-two lowercase hexadecimal digits (D8).
bool run_token(std::string_view s, std::string_view run, std::string_view generation) {
  const std::string prefix = std::string(run) + "." + std::string(generation) + ".";
  return s.starts_with(prefix) && s.size() == prefix.size() + 32 && lower_hex(s.substr(prefix.size()));
}

bool strictly_increasing(const std::vector<std::string>& items) {
  return std::ranges::adjacent_find(items, [](const std::string& a, const std::string& b) { return !(a < b); }) == items.end();
}

bool known_codec(std::string_view codec) {
  return codec == codec_dispatch_manifest_v1 || codec == codec_work_exec_v1 || codec == codec_driver_provenance_v1 ||
         codec == codec_role_structure_v1;
}

// ---- value construction --------------------------------------------------------------------------------------------

std::shared_ptr<value> make_node(value_kind kind) {
  auto p  = std::make_shared<value>();
  p->kind = kind;
  return p;
}

value_ptr make_text(std::string s) {
  auto p    = make_node(value_kind::string);
  p->string = std::move(s);
  return p;
}

value_ptr make_integer(std::int64_t n) {
  auto p     = make_node(value_kind::integer);
  p->integer = n;
  return p;
}

value_ptr make_list(std::vector<value_ptr> items) {
  auto p   = make_node(value_kind::list);
  p->items = std::move(items);
  return p;
}

value_ptr make_record(std::map<std::string, value_ptr> fields) {
  auto p    = make_node(value_kind::record);
  p->fields = std::move(fields);
  return p;
}

value_ptr make_texts(const std::vector<std::string>& items) {
  std::vector<value_ptr> values;
  values.reserve(items.size());
  for (const auto& s : items)
    values.push_back(make_text(s));
  return make_list(std::move(values));
}

// ---- value access (decoders only; every accessor refuses a shape it does not expect) -------------------------

bool exact_keys(const value& v, std::initializer_list<std::string_view> names) {
  if (v.kind != value_kind::record || v.fields.size() != names.size())
    return false;
  return std::ranges::all_of(names, [&](std::string_view key) { return v.fields.contains(std::string(key)); });
}

const value& child(const value& v, std::string_view key) {
  if (v.kind != value_kind::record)
    invalid();
  auto it = v.fields.find(std::string(key));
  if (it == v.fields.end() || !it->second)
    invalid();
  return *it->second;
}

std::string text_field(const value& v, std::string_view key) {
  const auto& f = child(v, key);
  if (f.kind != value_kind::string)
    invalid();
  return f.string;
}

std::int64_t integer_field(const value& v, std::string_view key) {
  const auto& f = child(v, key);
  if (f.kind != value_kind::integer)
    invalid();
  return f.integer;
}

const value& object_field(const value& v, std::string_view key) {
  const auto& f = child(v, key);
  if (f.kind != value_kind::record)
    invalid();
  return f;
}

const std::vector<value_ptr>& list_field(const value& v, std::string_view key) {
  const auto& f = child(v, key);
  if (f.kind != value_kind::list)
    invalid();
  return f.items;
}

std::string text_of(const value_ptr& p) {
  if (!p || p->kind != value_kind::string)
    invalid();
  return p->string;
}

std::vector<std::string> texts_field(const value& v, std::string_view key) {
  std::vector<std::string> result;
  for (const auto& item : list_field(v, key))
    result.push_back(text_of(item));
  return result;
}

// ---- canonical and JSON bridges ----------------------------------------------------------------------------------

std::string canonical_text(const value_ptr& v) {
  if (!v)
    invalid();
  auto out = canonical_encode(v);
  if (!out)
    invalid();
  return std::move(*out);
}

value_ptr canonical_value(std::string_view bytes) {
  auto decoded = canonical_decode(bytes);
  if (!decoded)
    invalid();
  // Re-encoding must reproduce the exact bytes, which refuses noncanonical key order and integer spellings.
  auto again = canonical_encode(*decoded);
  if (!again || *again != bytes)
    invalid();
  return *decoded;
}

// Glaze JSON in canonical spelling: sorted keys, compact, and no terminal LF.
std::string json_text(const value_ptr& v) {
  auto out = codec_encode(v);
  if (!out || out->size() > json_ceiling)
    invalid();
  return std::move(*out);
}

// Decode saved JSON. Codec decoding keeps the first of duplicate keys, so the exact-bytes re-encode is what refuses them,
// along with whitespace, reordered keys, escapes and any other spelling the encoder would not produce.
value_ptr canonical_json(std::string_view bytes) {
  if (bytes.size() > json_ceiling)
    invalid();
  auto parsed = codec_decode(bytes);
  if (!parsed)
    invalid();
  auto again = codec_encode(*parsed);
  if (!again || *again != bytes)
    invalid();
  return *parsed;
}

// ---- payload v2 -------------------------------------------------------------------------------------------------

void validate_position(const payload_position& p) {
  if (p.active_revision != "none" && !positive_decimal(p.active_revision))
    invalid();
  if (!p.active_job.empty() && !identifier(p.active_job))
    invalid();
}

void validate_references(std::string_view event_type, const std::map<std::string, std::string>& refs) {
  std::vector<std::string_view> required;
  bool                          provenance_allowed = false;
  if (event_type == "declaration_accepted" || event_type == "load_noop") {
    required = {"source", "registry", "stdlib", "interpretation", "versions"};
  } else if (event_type == "job_activated") {
    required = {"activation", "revision"};
  } else if (event_type == "run_claimed") {
    required           = {"run", "dispatch", "packet", "manifest", "work"};
    provenance_allowed = true;
  } else if (event_type == "run_taken_over") {
    required           = {"run", "previous_run", "dispatch", "packet", "manifest", "work"};
    provenance_allowed = true;
  } else {
    invalid();
  }
  for (std::string_view key : required)
    if (!refs.contains(std::string(key)))
      invalid();
  const std::size_t extra = provenance_allowed && refs.contains("provenance") ? 1 : 0;
  if (refs.size() != required.size() + extra)
    invalid();
  for (const auto& entry : refs)
    if (!positive_decimal(entry.second))
      invalid();
}

void validate_payload(std::string_view event_type, const event_payload_v2& p) {
  if (!one_of(p.operation, payload_operations))
    invalid();
  validate_position(p.before);
  validate_position(p.after);
  // EX-4 event payload and D101: the event reason is the reason of transition ordinal 1, or empty when the event has none.
  // Only a load event can carry no transition (a noop, or a first acceptance with no active job); every other event
  // carries a transition reason.
  const bool empty_reason_allowed = (event_type == "load_noop" || event_type == "declaration_accepted") && p.reason.empty();
  if (!empty_reason_allowed && !one_of(p.reason, transition_reasons))
    invalid();
  validate_references(event_type, p.references);
}

value_ptr position_value(const payload_position& p) {
  return make_record({{"active_revision", make_text(p.active_revision)}, {"active_job", make_text(p.active_job)}});
}

payload_position position_from(const value& v) {
  if (!exact_keys(v, {"active_revision", "active_job"}))
    invalid();
  return payload_position{text_field(v, "active_revision"), text_field(v, "active_job")};
}

// ---- transition rows --------------------------------------------------------------------------------------------

void validate_row(const transition_row& row) {
  if (row.ordinal == 0 || row.generation_id == 0)
    invalid();
  if (row.before_state) {
    if (!one_of(*row.before_state, role_states) || !row.before_drifted)
      invalid();
    // Before drift is false unless the before state is running.
    if (*row.before_state != "running" && *row.before_drifted)
      invalid();
  } else if (row.before_drifted) {
    invalid();
  }
  if (!one_of(row.after_state, role_states))
    invalid();
  if (row.after_drifted && row.after_state != "running")
    invalid();
  if (!one_of(row.reason, transition_reasons))
    invalid();
}

// ---- role structures and manifests -----------------------------------------------------------------------------

void validate_structure(const role_structure_v1& s) {
  if (s.kind != "worker" && s.kind != "reviewer")
    invalid();
  if (!strictly_increasing(s.work) || !strictly_increasing(s.targets))
    invalid();
  std::vector<std::string> owners;
  owners.reserve(s.ownership.size());
  for (const auto& entry : s.ownership)
    owners.push_back(entry.work);
  if (!strictly_increasing(owners))
    invalid();
}

value_ptr structure_value(const role_structure_v1& s) {
  std::vector<value_ptr> ownership;
  ownership.reserve(s.ownership.size());
  for (const auto& entry : s.ownership)
    ownership.push_back(make_record({{"work", make_text(entry.work)}, {"paths", make_texts(entry.paths)}}));
  return make_record({{"job", make_text(s.job)},
                      {"role", make_text(s.role)},
                      {"kind", make_text(s.kind)},
                      {"work", make_texts(s.work)},
                      {"targets", make_texts(s.targets)},
                      {"ownership", make_list(std::move(ownership))},
                      {"shape", make_text(s.shape)}});
}

role_structure_v1 structure_from(const value& v) {
  if (!exact_keys(v, {"job", "role", "kind", "work", "targets", "ownership", "shape"}))
    invalid();
  role_structure_v1 s;
  s.job     = text_field(v, "job");
  s.role    = text_field(v, "role");
  s.kind    = text_field(v, "kind");
  s.work    = texts_field(v, "work");
  s.targets = texts_field(v, "targets");
  for (const auto& item : list_field(v, "ownership")) {
    if (!item || !exact_keys(*item, {"work", "paths"}))
      invalid();
    s.ownership.push_back(ownership_entry{text_field(*item, "work"), texts_field(*item, "paths")});
  }
  s.shape = text_field(v, "shape");
  validate_structure(s);
  return s;
}

void validate_manifest(const value& v) {
  if (!exact_keys(v, {"version", "revision", "job", "role", "generation", "run", "versions", "work", "render", "sections",
                      "omitted", "prerequisites", "registry", "inputs"}))
    invalid();
}

void validate_work(const value& v) {
  if (!exact_keys(v, {"versions",
                      "workspace",
                      "spec",
                      "job",
                      "role",
                      "root",
                      "base",
                      "goal",
                      "units",
                      "milestones",
                      "constraints",
                      "prescriptions",
                      "contracts",
                      "dependencies",
                      "facets",
                      "pipeline_predecessors",
                      "questions",
                      "ownership",
                      "effective_checks",
                      "checks",
                      "toolchains",
                      "shape",
                      "start_facets",
                      "repair"}))
    invalid();
}

// ---- driver provenance -----------------------------------------------------------------------------------------

void walk_child(const value_ptr& child_value, std::size_t depth);

// Bounds that precede shape: string bytes and container depth, with depth counted from the document root.
void walk_bounds(const value& v, std::size_t depth) {
  if (v.kind == value_kind::string) {
    if (v.string.size() > provenance_string_ceiling)
      exceeded("provenance_string_bytes", provenance_string_ceiling);
    return;
  }
  if (v.kind != value_kind::record && v.kind != value_kind::list)
    return;
  if (depth > provenance_depth_ceiling)
    exceeded("provenance_depth", provenance_depth_ceiling);
  for (const auto& entry : v.fields)
    walk_child(entry.second, depth + 1);
  for (const auto& item : v.items)
    walk_child(item, depth + 1);
}

void walk_child(const value_ptr& child_value, std::size_t depth) {
  if (!child_value)
    invalid();
  walk_bounds(*child_value, depth);
}

// A leaf is a string of 1..1024 bytes without NUL, or exactly the object {status:"unknown"}.
void check_leaf(const value& v) {
  if (v.kind == value_kind::string) {
    if (v.string.empty() || !valid_text(v.string))
      invalid();
    return;
  }
  if (v.kind == value_kind::record && exact_keys(v, {"status"}) && text_field(v, "status") == "unknown")
    return;
  invalid();
}

void check_group(const value& document, std::string_view key, std::initializer_list<std::string_view> names) {
  const auto& group = object_field(document, key);
  if (!exact_keys(group, names))
    invalid();
  for (std::string_view name : names)
    check_leaf(child(group, name));
}

void validate_document(const value& document) {
  if (!exact_keys(document, {"version", "driver", "trial", "view", "model", "session", "settings"}))
    invalid();
  if (integer_field(document, "version") != 1)
    invalid();
  check_group(document, "driver", {"name", "version"});
  check_group(document, "trial", {"id"});
  check_group(document, "view", {"id"});
  check_group(document, "model", {"requested", "actual"});
  check_group(document, "session", {"requested", "actual"});
  std::vector<std::string> names;
  for (const auto& item : list_field(document, "settings")) {
    if (!item || !exact_keys(*item, {"name", "value"}))
      invalid();
    const auto& name = child(*item, "name");
    // A setting name is a string leaf; the unknown-status object is not a name.
    if (name.kind != value_kind::string)
      invalid();
    check_leaf(name);
    check_leaf(child(*item, "value"));
    names.push_back(name.string);
  }
  std::ranges::sort(names);
  if (std::ranges::adjacent_find(names) != names.end())
    invalid();
}

// Settings are sorted by name. Shape was validated first, so every name is a string.
value_ptr normalized_document(const value& document) {
  auto                   copy     = std::make_shared<value>(document);
  std::vector<value_ptr> settings = document.fields.at("settings")->items;
  std::ranges::sort(settings, [](const value_ptr& a, const value_ptr& b) {
    return a->fields.at("name")->string < b->fields.at("name")->string;
  });
  copy->fields["settings"] = make_list(std::move(settings));
  return copy;
}

// Bounds first, then shape. provenance_bytes is the compact JSON length of the document (D48).
value_ptr provenance_document(const value_ptr& document) {
  if (!document)
    invalid();
  walk_bounds(*document, 1);
  if (document->kind == value_kind::record) {
    auto settings = document->fields.find("settings");
    if (settings != document->fields.end() && settings->second && settings->second->kind == value_kind::list &&
        settings->second->items.size() > provenance_settings_ceiling)
      exceeded("provenance_settings", provenance_settings_ceiling);
  }
  auto bytes = codec_encode(document);
  if (!bytes) {
    if (bytes.error().detail.code == "resource_limit")
      exceeded("provenance_bytes", provenance_bytes_ceiling);
    invalid();
  }
  if (bytes->size() > provenance_bytes_ceiling)
    exceeded("provenance_bytes", provenance_bytes_ceiling);
  validate_document(*document);
  return normalized_document(*document);
}

// ---- value records -------------------------------------------------------------------------------------------------

void append_field(std::string& out, std::string_view part) {
  for (int shift = 56; shift >= 0; shift -= 8)
    out += static_cast<char>((static_cast<std::uint64_t>(part.size()) >> shift) & 255);
  out.append(part);
}

std::string_view read_field(std::string_view input, std::size_t& position, std::size_t maximum) {
  if (input.size() - position < 8)
    invalid();
  std::uint64_t size = 0;
  for (int i = 0; i < 8; ++i)
    size = (size << 8) | static_cast<unsigned char>(input[position++]);
  if (size > maximum || size > input.size() - position)
    invalid();
  auto result = input.substr(position, static_cast<std::size_t>(size));
  position += static_cast<std::size_t>(size);
  return result;
}

void validate_encoded(std::string_view encoded) {
  if (encoded.size() > value_record_ceiling || !canonical_decode(encoded))
    invalid();
}

execution_value_record decode_frame(std::string_view frame) {
  if (frame.size() < record_magic.size() + 1 || frame.substr(0, record_magic.size()) != record_magic)
    invalid();
  std::size_t position = record_magic.size();
  if (static_cast<unsigned char>(frame[position++]) != value_record_tag)
    invalid();
  const auto codec   = read_field(frame, position, 64);
  const auto encoded = read_field(frame, position, value_record_ceiling);
  if (position != frame.size() || !known_codec(codec))
    invalid();
  validate_encoded(encoded);
  return execution_value_record{std::string(codec), std::string(encoded)};
}

// ---- saved results -------------------------------------------------------------------------------------------------

void validate_claim(const claim_result_v1& r) {
  if (!ledger_text(r.ledger) || !request_token(r.request_id))
    invalid();
  if (!identifier(r.job) || !identifier(r.role))
    invalid();
  if (!positive_decimal(r.revision) || !positive_decimal(r.run_id) || !positive_decimal(r.generation) ||
      !positive_decimal(r.dispatch_id))
    invalid();
  if (!run_token(r.token, r.run_id, r.generation))
    invalid();
  if (!fingerprint_text(r.work_fp) || !fingerprint_text(r.render_digest))
    invalid();
  if (r.bytes > int32_maximum)
    invalid();
  if (r.previous_run && !positive_decimal(*r.previous_run))
    invalid();
}

value_ptr claim_value(const claim_result_v1& r) {
  std::map<std::string, value_ptr> fields{
      {"version", make_integer(1)},
      {"kind", make_text("claim")},
      {"ledger", make_text(r.ledger)},
      {"request_id", make_text(r.request_id)},
      {"job", make_text(r.job)},
      {"role", make_text(r.role)},
      {"revision", make_text(r.revision)},
      {"run", make_record({{"id", make_text(r.run_id)}, {"generation", make_text(r.generation)}, {"token", make_text(r.token)}})},
      {"dispatch", make_record({{"id", make_text(r.dispatch_id)},
                                {"work_fp", make_text(r.work_fp)},
                                {"render_digest", make_text(r.render_digest)},
                                {"bytes", make_integer(static_cast<std::int64_t>(r.bytes))}})},
  };
  if (r.previous_run)
    fields["previous_run"] = make_text(*r.previous_run);
  return make_record(std::move(fields));
}

claim_result_v1 claim_from(const value& v) {
  const bool takeover = v.kind == value_kind::record && v.fields.contains("previous_run");
  if (takeover) {
    if (!exact_keys(v, {"version", "kind", "ledger", "request_id", "job", "role", "revision", "run", "dispatch", "previous_run"}))
      invalid();
  } else if (!exact_keys(v, {"version", "kind", "ledger", "request_id", "job", "role", "revision", "run", "dispatch"})) {
    invalid();
  }
  if (integer_field(v, "version") != 1 || text_field(v, "kind") != "claim")
    invalid();
  claim_result_v1 r;
  r.ledger        = text_field(v, "ledger");
  r.request_id    = text_field(v, "request_id");
  r.job           = text_field(v, "job");
  r.role          = text_field(v, "role");
  r.revision      = text_field(v, "revision");
  const auto& run = object_field(v, "run");
  if (!exact_keys(run, {"id", "generation", "token"}))
    invalid();
  r.run_id             = text_field(run, "id");
  r.generation         = text_field(run, "generation");
  r.token              = text_field(run, "token");
  const auto& dispatch = object_field(v, "dispatch");
  if (!exact_keys(dispatch, {"id", "work_fp", "render_digest", "bytes"}))
    invalid();
  r.dispatch_id    = text_field(dispatch, "id");
  r.work_fp        = text_field(dispatch, "work_fp");
  r.render_digest  = text_field(dispatch, "render_digest");
  const auto bytes = integer_field(dispatch, "bytes");
  if (bytes < 0)
    invalid();
  r.bytes = static_cast<std::uint64_t>(bytes);
  if (takeover)
    r.previous_run = text_field(v, "previous_run");
  validate_claim(r);
  return r;
}

void validate_activate(const activate_result_v1& r) {
  if (!ledger_text(r.ledger) || !request_token(r.request_id))
    invalid();
  if (!positive_decimal(r.revision) || !identifier(r.job) || (!r.previous_job.empty() && !identifier(r.previous_job)))
    invalid();
  if (!positive_decimal(r.activation) || !positive_decimal(r.event))
    invalid();
  std::vector<std::string> names;
  names.reserve(r.roles.size());
  for (const auto& role : r.roles) {
    if (!identifier(role.role) || !positive_decimal(role.generation))
      invalid();
    names.push_back(role.role);
  }
  if (!strictly_increasing(names))
    invalid();
}

value_ptr activate_value(const activate_result_v1& r) {
  std::vector<value_ptr> roles;
  roles.reserve(r.roles.size());
  for (const auto& role : r.roles)
    roles.push_back(make_record(
        {{"role", make_text(role.role)}, {"generation", make_text(role.generation)}, {"state", make_text(role.state)}}));
  return make_record({{"version", make_integer(1)},
                      {"kind", make_text("activate")},
                      {"ledger", make_text(r.ledger)},
                      {"request_id", make_text(r.request_id)},
                      {"revision", make_text(r.revision)},
                      {"job", make_text(r.job)},
                      {"previous_job", make_text(r.previous_job)},
                      {"activation", make_text(r.activation)},
                      {"event", make_text(r.event)},
                      {"roles", make_list(std::move(roles))}});
}

activate_result_v1 activate_from(const value& v) {
  if (!exact_keys(v,
                  {"version", "kind", "ledger", "request_id", "revision", "job", "previous_job", "activation", "event", "roles"}))
    invalid();
  if (integer_field(v, "version") != 1 || text_field(v, "kind") != "activate")
    invalid();
  activate_result_v1 r;
  r.ledger       = text_field(v, "ledger");
  r.request_id   = text_field(v, "request_id");
  r.revision     = text_field(v, "revision");
  r.job          = text_field(v, "job");
  r.previous_job = text_field(v, "previous_job");
  r.activation   = text_field(v, "activation");
  r.event        = text_field(v, "event");
  for (const auto& item : list_field(v, "roles")) {
    if (!item || !exact_keys(*item, {"role", "generation", "state"}))
      invalid();
    r.roles.push_back(activate_role_v1{text_field(*item, "role"), text_field(*item, "generation"), text_field(*item, "state")});
  }
  validate_activate(r);
  return r;
}

} // namespace

std::expected<std::string, execution_codec_error> encode_execution_value_record(std::string_view codec,
                                                                                std::string_view canonical_bytes) {
  return guarded<std::string>([&]() -> std::string {
    if (!known_codec(codec))
      invalid();
    validate_encoded(canonical_bytes);
    std::string out(record_magic);
    out += static_cast<char>(value_record_tag);
    append_field(out, codec);
    append_field(out, canonical_bytes);
    return out;
  });
}

std::expected<execution_value_record, execution_codec_error> decode_execution_value_record(std::string_view frame) {
  return guarded<execution_value_record>([&] { return decode_frame(frame); });
}

std::expected<record_digest, execution_codec_error> digest_execution_value_record(std::string_view frame) {
  return guarded<record_digest>([&] {
    static_cast<void>(decode_frame(frame));
    XXH128_canonical_t canonical;
    XXH128_canonicalFromHash(&canonical, XXH3_128bits(frame.data(), frame.size()));
    record_digest result{};
    std::ranges::copy(canonical.digest, result.begin());
    return result;
  });
}

std::expected<std::string, execution_codec_error> encode_event_payload_v2(std::string_view        event_type,
                                                                          const event_payload_v2& payload) {
  return guarded<std::string>([&]() -> std::string {
    validate_payload(event_type, payload);
    std::map<std::string, value_ptr> references;
    for (const auto& entry : payload.references)
      references[entry.first] = make_text(entry.second);
    return json_text(make_record({{"version", make_integer(2)},
                                  {"operation", make_text(payload.operation)},
                                  {"before", position_value(payload.before)},
                                  {"after", position_value(payload.after)},
                                  {"reason", make_text(payload.reason)},
                                  {"references", make_record(std::move(references))}}));
  });
}

std::expected<event_payload_v2, execution_codec_error> decode_event_payload_v2(std::string_view event_type,
                                                                               std::string_view bytes) {
  return guarded<event_payload_v2>([&]() -> event_payload_v2 {
    const auto  root = canonical_json(bytes);
    const auto& v    = *root;
    if (!exact_keys(v, {"version", "operation", "before", "after", "reason", "references"}))
      invalid();
    if (integer_field(v, "version") != 2)
      invalid();
    event_payload_v2 payload;
    payload.operation = text_field(v, "operation");
    payload.before    = position_from(object_field(v, "before"));
    payload.after     = position_from(object_field(v, "after"));
    payload.reason    = text_field(v, "reason");
    for (const auto& entry : object_field(v, "references").fields)
      payload.references[entry.first] = text_of(entry.second);
    validate_payload(event_type, payload);
    return payload;
  });
}

std::expected<void, execution_codec_error> validate_transition_row(const transition_row& row) {
  return guarded<void>([&] { validate_row(row); });
}

std::expected<void, execution_codec_error> validate_transition_sequence(std::span<const transition_row> rows) {
  return guarded<void>([&] {
    std::uint64_t expected = 1;
    for (const auto& row : rows) {
      validate_row(row);
      if (row.ordinal != expected)
        invalid();
      ++expected;
    }
  });
}

std::string event_level_reason(std::span<const transition_row> rows) {
  for (const auto& row : rows)
    if (row.ordinal == 1)
      return row.reason;
  return {};
}

std::expected<std::string, execution_codec_error> encode_role_structure_v1(const role_structure_v1& structure) {
  return guarded<std::string>([&]() -> std::string {
    validate_structure(structure);
    return canonical_text(structure_value(structure));
  });
}

std::expected<role_structure_v1, execution_codec_error> decode_role_structure_v1(std::string_view canonical_bytes) {
  return guarded<role_structure_v1>([&]() -> role_structure_v1 { return structure_from(*canonical_value(canonical_bytes)); });
}

std::expected<std::string, execution_codec_error> role_structure_digest(const role_structure_v1& structure) {
  return guarded<std::string>([&]() -> std::string {
    validate_structure(structure);
    auto digest = hash_value("role_structure_v1", structure_value(structure));
    if (!digest)
      invalid();
    return *digest;
  });
}

std::expected<std::string, execution_codec_error> encode_dispatch_manifest_v1(const value_ptr& manifest) {
  return guarded<std::string>([&]() -> std::string {
    if (!manifest)
      invalid();
    validate_manifest(*manifest);
    auto bytes = canonical_text(manifest);
    if (bytes.size() > manifest_ceiling)
      exceeded("manifest_bytes", manifest_ceiling);
    return bytes;
  });
}

std::expected<value_ptr, execution_codec_error> decode_dispatch_manifest_v1(std::string_view bytes) {
  return guarded<value_ptr>([&]() -> value_ptr {
    if (bytes.size() > manifest_ceiling)
      exceeded("manifest_bytes", manifest_ceiling);
    auto manifest = canonical_value(bytes);
    validate_manifest(*manifest);
    return manifest;
  });
}

std::expected<std::string, execution_codec_error> encode_work_exec_v1(const value_ptr& work) {
  return guarded<std::string>([&]() -> std::string {
    if (!work)
      invalid();
    validate_work(*work);
    return canonical_text(work);
  });
}

std::expected<value_ptr, execution_codec_error> decode_work_exec_v1(std::string_view bytes) {
  return guarded<value_ptr>([&]() -> value_ptr {
    auto work = canonical_value(bytes);
    validate_work(*work);
    return work;
  });
}

std::expected<std::string, execution_codec_error> work_exec_digest(const value_ptr& work) {
  return guarded<std::string>([&]() -> std::string {
    if (!work)
      invalid();
    validate_work(*work);
    auto digest = hash_value("work_exec_v1", work);
    if (!digest)
      invalid();
    return *digest;
  });
}

std::expected<std::string, execution_codec_error> encode_driver_provenance_v1(const value_ptr& document) {
  return guarded<std::string>([&]() -> std::string {
    auto normalized = provenance_document(document);
    return canonical_text(make_record(
        {{"version", make_integer(1)}, {"authority", make_text(std::string(provenance_authority))}, {"document", normalized}}));
  });
}

std::expected<value_ptr, execution_codec_error> decode_driver_provenance_v1(std::string_view bytes) {
  return guarded<value_ptr>([&]() -> value_ptr {
    auto wrapper = canonical_value(bytes);
    if (!exact_keys(*wrapper, {"version", "authority", "document"}) || integer_field(*wrapper, "version") != 1 ||
        text_field(*wrapper, "authority") != provenance_authority)
      invalid();
    return provenance_document(wrapper->fields.at("document"));
  });
}

std::expected<std::string, execution_codec_error> encode_claim_result_v1(const claim_result_v1& result) {
  return guarded<std::string>([&]() -> std::string {
    validate_claim(result);
    return json_text(claim_value(result));
  });
}

std::expected<claim_result_v1, execution_codec_error> decode_claim_result_v1(std::string_view bytes) {
  return guarded<claim_result_v1>([&]() -> claim_result_v1 { return claim_from(*canonical_json(bytes)); });
}

std::expected<std::string, execution_codec_error> encode_activate_result_v1(const activate_result_v1& result) {
  return guarded<std::string>([&]() -> std::string {
    validate_activate(result);
    return json_text(activate_value(result));
  });
}

std::expected<activate_result_v1, execution_codec_error> decode_activate_result_v1(std::string_view bytes) {
  return guarded<activate_result_v1>([&]() -> activate_result_v1 { return activate_from(*canonical_json(bytes)); });
}

std::expected<value_ptr, execution_codec_error> assemble_packet_value(const packet_fields& fields) {
  return guarded<value_ptr>([&]() -> value_ptr {
    if (!strictly_increasing(fields.work))
      invalid();
    return make_record({{"version", make_integer(1)},
                        {"mode", make_text("dispatch")},
                        {"workspace", make_text(fields.workspace)},
                        {"spec", make_text(fields.spec)},
                        {"job", make_text(fields.job)},
                        {"role", make_text(fields.role)},
                        {"kind", make_text(fields.kind)},
                        {"work", make_texts(fields.work)},
                        {"markdown", make_text(fields.markdown)}});
  });
}

} // namespace planar::engine::history
