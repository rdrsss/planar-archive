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
constexpr std::array<std::string_view, 7>  payload_operations{"load",    "activate", "claim",        "takeover",
                                                              "verdict", "check",    "check-recover"};
// R18-2 and EX-8 row "Verdict file (R18)".
constexpr std::size_t verdict_targets_ceiling       = 256;
constexpr std::size_t verdict_findings_ceiling      = 256;
constexpr std::size_t verdict_finding_bytes_ceiling = 4096;
constexpr std::size_t verdict_summary_bytes_ceiling = 16384;
constexpr std::size_t verdict_path_bytes_ceiling    = 4096;

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
         codec == codec_role_structure_v1 || codec == codec_verdict_v1 || codec == codec_check_definition_v1 ||
         codec == codec_output_vector_v1 || codec == codec_check_observation_v1 || codec == codec_boundary_diff_v1;
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
  } else if (event_type == "check_reserved") {
    required           = {"reservation", "run", "dispatch"};
    provenance_allowed = true;
  } else if (event_type == "check_recorded") {
    required = {"reservation", "observation"};
    if (refs.contains("completion"))
      required.push_back("completion");
  } else if (event_type == "check_interrupted") {
    required = {"reservation"};
  } else if (event_type == "verdict_recorded") {
    // R18-4: value record id, runs.id and verdicts.id; a verdict attaches no provenance (P2-4).
    required = {"body", "run", "verdict"};
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
  const bool empty_reason_allowed =
      (event_type == "load_noop" || event_type == "declaration_accepted" || event_type == "check_reserved" ||
       event_type == "check_recorded" || event_type == "check_interrupted") &&
      p.reason.empty();
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

// ---- verdict documents (R18-2) -----------------------------------------------------------------------------------

// A field-table failure, thrown inside the verdict checks and returned as verdict_violation at the boundary.
struct verdict_failure {
  std::string field;
  std::string message;
};

[[noreturn]] void violate(std::string field, std::string_view message) {
  throw verdict_failure{std::move(field), std::string(message)};
}

std::string at(std::string_view base, std::string_view key) {
  std::string out(base);
  out += '.';
  out += key;
  return out;
}

std::string at(std::string_view base, std::size_t index) {
  std::string out(base);
  out += std::format(".{}", index);
  return out;
}

// Finding identifier grammar [A-Za-z0-9_.:-]{1,128}.
bool finding_identifier(std::string_view s) {
  return request_token(s);
}

// Portable source path: the declaration path segment rules with the R18-2 length of 1 to 4096 bytes.
bool verdict_source_path(std::string_view p) {
  if (p.empty() || p.size() > verdict_path_bytes_ceiling || p.back() == '/')
    return false;
  while (true) {
    const auto end  = p.find('/');
    const auto part = p.substr(0, end);
    if (part.empty() || part.size() > 128 || part == "." || part == "..")
      return false;
    if (part.size() == 4 && part[0] == '.' && (part[1] == 'g' || part[1] == 'G') && (part[2] == 'i' || part[2] == 'I') &&
        (part[3] == 't' || part[3] == 'T'))
      return false;
    for (char c : part)
      if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '.' || c == '-'))
        return false;
    if (end == std::string_view::npos)
      return true;
    p.remove_prefix(end + 1);
  }
}

// Unknown keys first (sorted, the first wins), then missing required keys in the listed order.
void verdict_keys(const value& v, std::string_view field, std::initializer_list<std::string_view> required,
                  std::initializer_list<std::string_view> optional) {
  for (const auto& entry : v.fields) {
    const bool known = std::ranges::find(required, std::string_view(entry.first)) != required.end() ||
                       std::ranges::find(optional, std::string_view(entry.first)) != optional.end();
    if (!known)
      violate(at(field, entry.first), "Unknown field");
  }
  for (std::string_view key : required)
    if (!v.fields.contains(std::string(key)))
      violate(at(field, key), "Required field");
}

const value& verdict_child(const value& v, std::string_view key) {
  const auto& p = v.fields.at(std::string(key));
  if (!p)
    invalid();
  return *p;
}

const value* verdict_optional(const value& v, std::string_view key) {
  auto it = v.fields.find(std::string(key));
  if (it == v.fields.end())
    return nullptr;
  if (!it->second)
    invalid();
  return it->second.get();
}

std::string verdict_identifier(const value& v, const std::string& field) {
  if (v.kind != value_kind::string || !identifier(v.string))
    violate(field, "Invalid ID");
  return v.string;
}

std::string verdict_digest(const value& v, const std::string& field) {
  if (v.kind != value_kind::string || !fingerprint_text(v.string))
    violate(field, "Malformed output digest");
  return v.string;
}

std::string verdict_string(const value& v, const std::string& field, std::size_t minimum, std::size_t maximum) {
  if (v.kind != value_kind::string || v.string.size() < minimum || v.string.size() > maximum || !valid_text(v.string))
    violate(field, "Invalid string");
  return v.string;
}

const std::vector<value_ptr>& verdict_list(const value& v, const std::string& field) {
  if (v.kind != value_kind::list)
    violate(field, "Expected list");
  return v.items;
}

const value& verdict_record(const value_ptr& p, const std::string& field) {
  if (!p)
    invalid();
  if (p->kind != value_kind::record)
    violate(field, "Expected object");
  return *p;
}

verdict_target verdict_target_from(const value_ptr& item, const std::string& field) {
  const auto& v = verdict_record(item, field);
  verdict_keys(v, field, {"work", "output", "work_fp"}, {});
  verdict_target t;
  t.work    = verdict_identifier(verdict_child(v, "work"), at(field, "work"));
  t.output  = verdict_digest(verdict_child(v, "output"), at(field, "output"));
  t.work_fp = verdict_digest(verdict_child(v, "work_fp"), at(field, "work_fp"));
  return t;
}

verdict_finding verdict_finding_from(const value_ptr& item, const std::string& field, const std::set<std::string>& works) {
  const auto& v = verdict_record(item, field);
  verdict_keys(v, field, {"id", "severity", "work", "text"}, {"path", "line", "prescription"});
  verdict_finding f;
  const auto&     id = verdict_child(v, "id");
  if (id.kind != value_kind::string || !finding_identifier(id.string))
    violate(at(field, "id"), "Invalid ID");
  f.id                 = id.string;
  const auto& severity = verdict_child(v, "severity");
  if (severity.kind != value_kind::string ||
      (severity.string != "must" && severity.string != "should" && severity.string != "note"))
    violate(at(field, "severity"), "Invalid enum value");
  f.severity = severity.string;
  f.work     = verdict_identifier(verdict_child(v, "work"), at(field, "work"));
  if (!works.contains(f.work))
    violate(at(field, "work"), "Invalid enum value");
  if (const auto* path = verdict_optional(v, "path")) {
    if (path->kind != value_kind::string || !verdict_source_path(path->string))
      violate(at(field, "path"), "Invalid portable path");
    f.path = path->string;
  }
  if (const auto* line = verdict_optional(v, "line")) {
    if (line->kind != value_kind::integer || line->integer < 1 || static_cast<std::uint64_t>(line->integer) > int32_maximum)
      violate(at(field, "line"), "Integer out of range");
    if (!f.path)
      violate(at(field, "path"), "Required field");
    f.line = line->integer;
  }
  if (const auto* prescription = verdict_optional(v, "prescription"))
    f.prescription = verdict_identifier(*prescription, at(field, "prescription"));
  f.text = verdict_string(verdict_child(v, "text"), at(field, "text"), 1, verdict_finding_bytes_ceiling);
  return f;
}

// R18-2 step 5. Key order: version, targets, policy, decision, findings, repair, summary, authority.
verdict_v1 verdict_document(const value_ptr& document) {
  const std::string root = "verdict";
  const auto&       v    = verdict_record(document, root);
  verdict_keys(v, root, {"version", "targets", "policy", "decision", "findings", "repair", "summary"}, {"authority"});
  const auto& version = verdict_child(v, "version");
  if (version.kind != value_kind::integer)
    violate(at(root, "version"), "Expected version integer");
  if (version.integer != 1)
    violate(at(root, "version"), "Unsupported schema version");
  verdict_v1  out;
  const auto  targets_field = at(root, "targets");
  const auto& targets       = verdict_list(verdict_child(v, "targets"), targets_field);
  if (targets.empty())
    violate(targets_field, "List is too short");
  if (targets.size() > verdict_targets_ceiling)
    violate(targets_field, "Expected list");
  std::set<std::string> works;
  for (std::size_t i = 0; i < targets.size(); ++i) {
    const auto field = at(targets_field, i);
    auto       t     = verdict_target_from(targets[i], field);
    if (i > 0 && !(out.targets.back().work < t.work))
      violate(at(field, "work"), "Duplicate value");
    works.insert(t.work);
    out.targets.push_back(std::move(t));
  }
  out.policy           = verdict_digest(verdict_child(v, "policy"), at(root, "policy"));
  const auto& decision = verdict_child(v, "decision");
  if (decision.kind != value_kind::string || (decision.string != "approve" && decision.string != "reject"))
    violate(at(root, "decision"), "Invalid enum value");
  out.decision                         = decision.string;
  const auto            findings_field = at(root, "findings");
  const auto&           findings       = verdict_list(verdict_child(v, "findings"), findings_field);
  std::set<std::string> ids;
  for (std::size_t i = 0; i < findings.size(); ++i) {
    const auto field = at(findings_field, i);
    auto       f     = verdict_finding_from(findings[i], field, works);
    if (!ids.insert(f.id).second)
      violate(at(field, "id"), "Duplicate value");
    out.findings.push_back(std::move(f));
  }
  const auto  repair_field = at(root, "repair");
  const auto& repair       = verdict_list(verdict_child(v, "repair"), repair_field);
  for (std::size_t i = 0; i < repair.size(); ++i) {
    const auto field = at(repair_field, i);
    if (!repair[i])
      invalid();
    auto role = verdict_identifier(*repair[i], field);
    if (i > 0 && !(out.repair.back() < role))
      violate(field, "Duplicate value");
    out.repair.push_back(std::move(role));
  }
  out.summary = verdict_string(verdict_child(v, "summary"), at(root, "summary"), 0, verdict_summary_bytes_ceiling);
  if (const auto* authority = verdict_optional(v, "authority")) {
    if (authority->kind != value_kind::string || !valid_text(authority->string))
      violate(at(root, "authority"), "Invalid string");
    out.authority = authority->string;
  }
  return out;
}

// R18-2 step 4, in the order findings count, finding text, summary. Only applies where the shape is as expected.
void verdict_bounds(const value_ptr& document) {
  if (!document || document->kind != value_kind::record)
    return;
  const auto findings = document->fields.find("findings");
  if (findings != document->fields.end() && findings->second && findings->second->kind == value_kind::list) {
    if (findings->second->items.size() > verdict_findings_ceiling)
      exceeded("verdict_findings", verdict_findings_ceiling);
    for (const auto& item : findings->second->items) {
      if (!item || item->kind != value_kind::record)
        continue;
      const auto text = item->fields.find("text");
      if (text != item->fields.end() && text->second && text->second->kind == value_kind::string &&
          text->second->string.size() > verdict_finding_bytes_ceiling)
        exceeded("verdict_finding_bytes", verdict_finding_bytes_ceiling);
    }
  }
  const auto summary = document->fields.find("summary");
  if (summary != document->fields.end() && summary->second && summary->second->kind == value_kind::string &&
      summary->second->string.size() > verdict_summary_bytes_ceiling)
    exceeded("verdict_summary_bytes", verdict_summary_bytes_ceiling);
}

void verdict_decision(const verdict_v1& verdict) {
  if (verdict.decision != "approve")
    return;
  if (!verdict.repair.empty() ||
      std::ranges::any_of(verdict.findings, [](const verdict_finding& f) { return f.severity == "must"; }))
    violate("verdict.decision", "Invalid enum value");
}

value_ptr verdict_structure(const verdict_v1& verdict) {
  std::vector<value_ptr> targets;
  targets.reserve(verdict.targets.size());
  for (const auto& t : verdict.targets)
    targets.push_back(
        make_record({{"work", make_text(t.work)}, {"output", make_text(t.output)}, {"work_fp", make_text(t.work_fp)}}));
  std::vector<value_ptr> findings;
  findings.reserve(verdict.findings.size());
  for (const auto& f : verdict.findings) {
    std::map<std::string, value_ptr> fields{
        {"id", make_text(f.id)}, {"severity", make_text(f.severity)}, {"work", make_text(f.work)}, {"text", make_text(f.text)}};
    if (f.path)
      fields["path"] = make_text(*f.path);
    if (f.line)
      fields["line"] = make_integer(*f.line);
    if (f.prescription)
      fields["prescription"] = make_text(*f.prescription);
    findings.push_back(make_record(std::move(fields)));
  }
  return make_record({{"version", make_integer(1)},
                      {"targets", make_list(std::move(targets))},
                      {"policy", make_text(verdict.policy)},
                      {"decision", make_text(verdict.decision)},
                      {"findings", make_list(std::move(findings))},
                      {"repair", make_texts(verdict.repair)},
                      {"summary", make_text(verdict.summary)},
                      {"authority", make_text(verdict.authority)}});
}

// Every stored or encoded verdict is normalized: all R18-2 rules hold and authority is reviewer_role_v1.
verdict_v1 normalized_verdict(const value_ptr& structure) {
  verdict_bounds(structure);
  try {
    auto typed = verdict_document(structure);
    if (typed.authority != verdict_authority_reviewer)
      invalid();
    verdict_decision(typed);
    return typed;
  } catch (const verdict_failure&) {
    invalid();
  }
}

// ---- verdict saved result ------------------------------------------------------------------------------------------

bool nonnegative_decimal(std::string_view s) {
  return s == "0" || positive_decimal(s);
}

// Canonical decimals order numerically by length, then bytes.
bool decimal_less(std::string_view a, std::string_view b) {
  return a.size() != b.size() ? a.size() < b.size() : a < b;
}

void validate_verdict_result(const verdict_result_v1& r) {
  if (!ledger_text(r.ledger) || !request_token(r.request_id))
    invalid();
  if (!identifier(r.job) || !identifier(r.role))
    invalid();
  if (!positive_decimal(r.revision) || !positive_decimal(r.run) || !positive_decimal(r.event) || !positive_decimal(r.verdict_id))
    invalid();
  if (r.applicability != "current" && r.applicability != "historical")
    invalid();
  if (r.decision != "approve" && r.decision != "reject")
    invalid();
  // An admitted rejection is current, has repairs, and only it has repairs.
  if (r.admitted != !r.repairs.empty())
    invalid();
  if (r.admitted && (r.decision != "reject" || r.applicability != "current"))
    invalid();
  std::vector<std::string> names;
  for (const auto& repair : r.repairs) {
    if (!identifier(repair.role) || !positive_decimal(repair.generation) || !positive_decimal(repair.revision))
      invalid();
    names.push_back(repair.role);
  }
  if (!strictly_increasing(names))
    invalid();
  if (!nonnegative_decimal(r.count) || !nonnegative_decimal(r.max_rounds))
    invalid();
  const std::string* previous = nullptr;
  for (const auto& blocker : r.opened) {
    if (!positive_decimal(blocker.id) || (blocker.kind != "max_rounds" && blocker.kind != "unowned_rejection"))
      invalid();
    if (previous && !decimal_less(*previous, blocker.id))
      invalid();
    previous = &blocker.id;
  }
  previous = nullptr;
  for (const auto& id : r.cleared) {
    if (!positive_decimal(id) || (previous && !decimal_less(*previous, id)))
      invalid();
    previous = &id;
  }
  names.clear();
  for (const auto& role : r.roles) {
    if (!identifier(role.role) || !one_of(role.state, role_states) || (role.drifted && role.state != "running"))
      invalid();
    names.push_back(role.role);
  }
  if (!strictly_increasing(names))
    invalid();
}

value_ptr make_boolean(bool b) {
  auto p     = make_node(value_kind::boolean);
  p->boolean = b;
  return p;
}

value_ptr verdict_result_value(const verdict_result_v1& r) {
  std::vector<value_ptr> repairs;
  for (const auto& repair : r.repairs)
    repairs.push_back(make_record({{"role", make_text(repair.role)},
                                   {"generation", make_text(repair.generation)},
                                   {"revision", make_text(repair.revision)}}));
  std::vector<value_ptr> opened;
  for (const auto& blocker : r.opened)
    opened.push_back(make_record({{"id", make_text(blocker.id)}, {"kind", make_text(blocker.kind)}}));
  std::vector<value_ptr> roles;
  for (const auto& role : r.roles)
    roles.push_back(
        make_record({{"role", make_text(role.role)}, {"state", make_text(role.state)}, {"drifted", make_boolean(role.drifted)}}));
  return make_record({{"version", make_integer(1)},
                      {"kind", make_text("verdict")},
                      {"ledger", make_text(r.ledger)},
                      {"request_id", make_text(r.request_id)},
                      {"job", make_text(r.job)},
                      {"role", make_text(r.role)},
                      {"revision", make_text(r.revision)},
                      {"run", make_text(r.run)},
                      {"event", make_text(r.event)},
                      {"verdict", make_record({{"id", make_text(r.verdict_id)},
                                               {"applicability", make_text(r.applicability)},
                                               {"decision", make_text(r.decision)},
                                               {"admitted", make_boolean(r.admitted)}})},
                      {"repairs", make_list(std::move(repairs))},
                      {"review", make_record({{"count", make_text(r.count)}, {"max_rounds", make_text(r.max_rounds)}})},
                      {"blockers", make_record({{"opened", make_list(std::move(opened))}, {"cleared", make_texts(r.cleared)}})},
                      {"roles", make_list(std::move(roles))}});
}

bool boolean_field(const value& v, std::string_view key) {
  const auto& f = child(v, key);
  if (f.kind != value_kind::boolean)
    invalid();
  return f.boolean;
}

verdict_result_v1 verdict_result_from(const value& v) {
  if (!exact_keys(v, {"version", "kind", "ledger", "request_id", "job", "role", "revision", "run", "event", "verdict", "repairs",
                      "review", "blockers", "roles"}))
    invalid();
  if (integer_field(v, "version") != 1 || text_field(v, "kind") != "verdict")
    invalid();
  verdict_result_v1 r;
  r.ledger            = text_field(v, "ledger");
  r.request_id        = text_field(v, "request_id");
  r.job               = text_field(v, "job");
  r.role              = text_field(v, "role");
  r.revision          = text_field(v, "revision");
  r.run               = text_field(v, "run");
  r.event             = text_field(v, "event");
  const auto& verdict = object_field(v, "verdict");
  if (!exact_keys(verdict, {"id", "applicability", "decision", "admitted"}))
    invalid();
  r.verdict_id    = text_field(verdict, "id");
  r.applicability = text_field(verdict, "applicability");
  r.decision      = text_field(verdict, "decision");
  r.admitted      = boolean_field(verdict, "admitted");
  for (const auto& item : list_field(v, "repairs")) {
    if (!item || !exact_keys(*item, {"role", "generation", "revision"}))
      invalid();
    r.repairs.push_back(
        verdict_result_repair{text_field(*item, "role"), text_field(*item, "generation"), text_field(*item, "revision")});
  }
  const auto& review = object_field(v, "review");
  if (!exact_keys(review, {"count", "max_rounds"}))
    invalid();
  r.count              = text_field(review, "count");
  r.max_rounds         = text_field(review, "max_rounds");
  const auto& blockers = object_field(v, "blockers");
  if (!exact_keys(blockers, {"opened", "cleared"}))
    invalid();
  for (const auto& item : list_field(blockers, "opened")) {
    if (!item || !exact_keys(*item, {"id", "kind"}))
      invalid();
    r.opened.push_back(verdict_result_blocker{text_field(*item, "id"), text_field(*item, "kind")});
  }
  r.cleared = texts_field(blockers, "cleared");
  for (const auto& item : list_field(v, "roles")) {
    if (!item || !exact_keys(*item, {"role", "state", "drifted"}))
      invalid();
    r.roles.push_back(
        verdict_result_role{text_field(*item, "role"), text_field(*item, "state"), boolean_field(*item, "drifted")});
  }
  validate_verdict_result(r);
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

std::expected<void, execution_codec_error> check_verdict_bounds(const value_ptr& document) {
  return guarded<void>([&] { verdict_bounds(document); });
}

std::expected<verdict_v1, verdict_violation> verdict_from_value(const value_ptr& document) {
  try {
    return verdict_document(document);
  } catch (const verdict_failure& failure) {
    return std::unexpected(verdict_violation{failure.field, failure.message});
  } catch (const execution_codec_error&) {
    // A null child is not a JSON shape; report it at the root.
    return std::unexpected(verdict_violation{"verdict", "Expected object"});
  }
}

std::expected<void, verdict_violation> check_verdict_decision(const verdict_v1& verdict) {
  try {
    verdict_decision(verdict);
    return {};
  } catch (const verdict_failure& failure) {
    return std::unexpected(verdict_violation{failure.field, failure.message});
  }
}

std::expected<value_ptr, execution_codec_error> verdict_value(const verdict_v1& verdict) {
  return guarded<value_ptr>([&]() -> value_ptr {
    auto structure = verdict_structure(verdict);
    static_cast<void>(normalized_verdict(structure));
    return structure;
  });
}

std::expected<std::string, execution_codec_error> encode_verdict_v1(const verdict_v1& verdict) {
  return guarded<std::string>([&]() -> std::string {
    auto structure = verdict_structure(verdict);
    static_cast<void>(normalized_verdict(structure));
    return canonical_text(structure);
  });
}

std::expected<verdict_v1, execution_codec_error> decode_verdict_v1(std::string_view bytes) {
  return guarded<verdict_v1>([&]() -> verdict_v1 {
    auto structure = canonical_value(bytes);
    if (!structure->fields.contains("authority"))
      invalid();
    return normalized_verdict(structure);
  });
}

std::expected<std::string, execution_codec_error> encode_verdict_result_v1(const verdict_result_v1& result) {
  return guarded<std::string>([&]() -> std::string {
    validate_verdict_result(result);
    return json_text(verdict_result_value(result));
  });
}

std::expected<verdict_result_v1, execution_codec_error> decode_verdict_result_v1(std::string_view bytes) {
  return guarded<verdict_result_v1>([&]() -> verdict_result_v1 { return verdict_result_from(*canonical_json(bytes)); });
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

namespace {
void validate_check_value(std::string_view codec, const value_ptr& doc) {
  if (!doc || doc->kind != value_kind::record)
    invalid();
  const auto version = doc->fields.find("version");
  if (version == doc->fields.end() || !version->second || version->second->kind != value_kind::integer ||
      version->second->integer != 1)
    invalid();
  auto list = [&](std::string_view key) {
    auto it = doc->fields.find(std::string(key));
    if (it == doc->fields.end() || !it->second || it->second->kind != value_kind::list)
      invalid();
    return it->second;
  };
  if (codec == codec_output_vector_v1) {
    if (!exact_keys(*doc, {"version", "targets"}))
      invalid();
    std::string previous;
    for (const auto& item : list("targets")->items) {
      if (!item || !exact_keys(*item, {"work", "output"}))
        invalid();
      auto work = text_field(*item, "work"), output = text_field(*item, "output");
      if (!identifier(work) || (!previous.empty() && previous >= work) || output.size() != 41 || !output.starts_with("xxh3_128:"))
        invalid();
      previous = work;
    }
  } else if (codec == codec_check_observation_v1) {
    if (!exact_keys(*doc, {"version", "root", "base", "observation", "units", "contracts"}))
      invalid();
    (void)text_field(*doc, "root");
    (void)text_field(*doc, "base");
    (void)text_field(*doc, "observation");
    (void)list("units");
    (void)list("contracts");
  } else if (codec == codec_boundary_diff_v1) {
    if (!exact_keys(*doc, {"version", "changes"}))
      invalid();
    for (const auto& item : list("changes")->items) {
      if (!item || !exact_keys(*item, {"path", "change", "class"}))
        invalid();
      (void)text_field(*item, "path");
      auto change = text_field(*item, "change"), classification = text_field(*item, "class");
      if (change != "added" && change != "deleted" && change != "modified" && change != "mode")
        invalid();
      if (classification != "own" && classification != "producer" && classification != "frozen" && classification != "outside")
        invalid();
    }
  } else if (codec == codec_check_definition_v1) {
    (void)text_field(*doc, "work");
    (void)text_field(*doc, "check");
    auto gate = text_field(*doc, "gate");
    if (gate != "registry" && gate != "boundary_v1")
      invalid();
    (void)list("inputs");
    if (gate == "registry") {
      (void)list("command");
      (void)list("environment");
      (void)list("reserved");
    } else {
      (void)list("owned");
      (void)list("frozen");
      (void)list("excluded");
      (void)list("producers");
    }
  } else
    invalid();
}
} // namespace
std::expected<std::string, execution_codec_error> encode_check_value(std::string_view codec, const value_ptr& doc) {
  return guarded<std::string>([&] {
    validate_check_value(codec, doc);
    return canonical_text(doc);
  });
}
std::expected<value_ptr, execution_codec_error> decode_check_value(std::string_view codec, std::string_view bytes) {
  return guarded<value_ptr>([&] {
    auto doc = canonical_value(bytes);
    validate_check_value(codec, doc);
    return doc;
  });
}

} // namespace planar::engine::history
