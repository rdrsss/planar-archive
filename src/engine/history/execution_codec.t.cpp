// Independent execution codec checks: payload v2 JSON literals, transition rules, role structures, manifests,
// work fingerprints, driver provenance bounds, value-record framing, saved JSON results, the packet value and the
// encoder grammars. Literal oracles are written from the execution contract, not from the codec.
#include <catch2/catch_test_macros.hpp>
#include <xxhash.h>
import std;
import planar.engine.canonical;
import planar.engine.hash;
import planar.engine.json_codec;
import planar.engine.value;
import planar.engine.history.execution_codec;
import planar.engine.history.record;
using namespace planar::engine;
using namespace planar::engine::history;
namespace {
constexpr std::string_view ledger_id         = "0123456789abcdef0123456789abcdef";
constexpr std::string_view run_token         = "7.2.fedcba9876543210fedcba9876543210";
constexpr std::string_view fingerprint       = "xxh3_128:00112233445566778899aabbccddeeff";
constexpr std::string_view other_fingerprint = "xxh3_128:ffeeddccbbaa99887766554433221100";

std::shared_ptr<value> node(value_kind kind) {
  auto p  = std::make_shared<value>();
  p->kind = kind;
  return p;
}

value_ptr text_of(std::string s) {
  auto p    = node(value_kind::string);
  p->string = std::move(s);
  return p;
}

value_ptr int_of(std::int64_t n) {
  auto p     = node(value_kind::integer);
  p->integer = n;
  return p;
}

value_ptr bool_of(bool b) {
  auto p     = node(value_kind::boolean);
  p->boolean = b;
  return p;
}

value_ptr array_of(std::vector<value_ptr> items) {
  auto p   = node(value_kind::list);
  p->items = std::move(items);
  return p;
}

value_ptr object_of(std::map<std::string, value_ptr> fields) {
  auto p    = node(value_kind::record);
  p->fields = std::move(fields);
  return p;
}

value_ptr with_field(const value_ptr& base, const std::string& key, value_ptr field) {
  auto copy         = std::make_shared<value>(*base);
  copy->fields[key] = std::move(field);
  return copy;
}

value_ptr without_field(const value_ptr& base, const std::string& key) {
  auto copy = std::make_shared<value>(*base);
  copy->fields.erase(key);
  return copy;
}

std::string canonical(const value_ptr& v) {
  return canonical_encode(v).value_or(std::string());
}

std::string json(const value_ptr& v) {
  return codec_encode(v).value_or(std::string());
}

void wide(std::string& bytes, std::uint64_t n) {
  for (int shift = 56; shift >= 0; shift -= 8)
    bytes += static_cast<char>((n >> shift) & 255);
}

void field(std::string& bytes, std::string_view part) {
  wide(bytes, part.size());
  bytes.append(part);
}

// Hand-built value frame: magic, version 01, value tag 03, bytes(codec), bytes(encoded).
std::string value_frame(std::string_view codec, std::string_view encoded) {
  std::string bytes = std::string("PLANAR-RECORD") + '\x01' + '\x03';
  field(bytes, codec);
  field(bytes, encoded);
  return bytes;
}

// Seed-zero XXH3-128 of the exact bytes in canonical big-endian order.
record_digest xxh3(std::string_view bytes) {
  XXH128_canonical_t form;
  XXH128_canonicalFromHash(&form, XXH3_128bits(bytes.data(), bytes.size()));
  record_digest result{};
  std::ranges::copy(form.digest, result.begin());
  return result;
}

std::string replaced(std::string text, std::string_view from, std::string_view to) {
  const auto at = text.find(from);
  REQUIRE(at != std::string::npos);
  text.replace(at, from.size(), to);
  return text;
}

std::string without_part(std::string text, std::string_view part) {
  const auto at = text.find(part);
  REQUIRE(at != std::string::npos);
  text.erase(at, part.size());
  return text;
}

// ---- payload v2 fixtures and literal oracles -------------------------------------------------------------

event_payload_v2 accepted_payload() {
  return {"load",
          {"none", ""},
          {"1", ""},
          "structural_change",
          {{"source", "1"}, {"registry", "2"}, {"stdlib", "3"}, {"interpretation", "4"}, {"versions", "5"}}};
}

event_payload_v2 noop_payload() {
  return {"load",
          {"1", ""},
          {"1", ""},
          "",
          {{"source", "1"}, {"registry", "2"}, {"stdlib", "3"}, {"interpretation", "4"}, {"versions", "5"}}};
}

event_payload_v2 activation_payload() {
  return {"activate", {"none", ""}, {"1", "parse"}, "activated", {{"activation", "3"}, {"revision", "4"}}};
}

event_payload_v2 claimed_payload() {
  return {"claim",
          {"1", "parse"},
          {"1", "parse"},
          "claimed",
          {{"run", "7"}, {"dispatch", "9"}, {"packet", "11"}, {"manifest", "12"}, {"work", "13"}}};
}

event_payload_v2 claimed_with_provenance_payload() {
  auto payload                     = claimed_payload();
  payload.references["provenance"] = "14";
  return payload;
}

event_payload_v2 takeover_payload() {
  return {"takeover",
          {"1", "parse"},
          {"1", "parse"},
          "taken_over",
          {{"run", "8"}, {"previous_run", "7"}, {"dispatch", "10"}, {"packet", "11"}, {"manifest", "12"}, {"work", "13"}}};
}

constexpr std::string_view accepted_json =
    R"j({"after":{"active_job":"","active_revision":"1"},"before":{"active_job":"","active_revision":"none"},"operation":"load","reason":"structural_change","references":{"interpretation":"4","registry":"2","source":"1","stdlib":"3","versions":"5"},"version":2})j";

constexpr std::string_view noop_json =
    R"j({"after":{"active_job":"","active_revision":"1"},"before":{"active_job":"","active_revision":"1"},"operation":"load","reason":"","references":{"interpretation":"4","registry":"2","source":"1","stdlib":"3","versions":"5"},"version":2})j";

constexpr std::string_view activation_json =
    R"j({"after":{"active_job":"parse","active_revision":"1"},"before":{"active_job":"","active_revision":"none"},"operation":"activate","reason":"activated","references":{"activation":"3","revision":"4"},"version":2})j";

constexpr std::string_view claimed_json =
    R"j({"after":{"active_job":"parse","active_revision":"1"},"before":{"active_job":"parse","active_revision":"1"},"operation":"claim","reason":"claimed","references":{"dispatch":"9","manifest":"12","packet":"11","run":"7","work":"13"},"version":2})j";

constexpr std::string_view claimed_provenance_json =
    R"j({"after":{"active_job":"parse","active_revision":"1"},"before":{"active_job":"parse","active_revision":"1"},"operation":"claim","reason":"claimed","references":{"dispatch":"9","manifest":"12","packet":"11","provenance":"14","run":"7","work":"13"},"version":2})j";

constexpr std::string_view takeover_json =
    R"j({"after":{"active_job":"parse","active_revision":"1"},"before":{"active_job":"parse","active_revision":"1"},"operation":"takeover","reason":"taken_over","references":{"dispatch":"10","manifest":"12","packet":"11","previous_run":"7","run":"8","work":"13"},"version":2})j";

// ---- transition, role structure and manifest fixtures ----------------------------------------------------

transition_row row(std::uint64_t ordinal, std::string reason) {
  transition_row r;
  r.ordinal        = ordinal;
  r.job            = "parse";
  r.role           = "worker_a";
  r.generation_id  = 7;
  r.before_state   = "blocked";
  r.before_drifted = false;
  r.after_state    = "ready";
  r.after_drifted  = false;
  r.reason         = std::move(reason);
  return r;
}

role_structure_v1 structure() {
  return {"parse", "worker_a", "worker", {"u1", "u2"}, {}, {{"u1", {"src/a.lua"}}, {"u2", {"src/b.lua", "src/c.lua"}}}, "linear"};
}

value_ptr manifest(std::size_t registry_padding) {
  return object_of({
      {"version", int_of(1)},
      {"revision", text_of("7")},
      {"job", text_of("parse")},
      {"role", text_of("worker_a")},
      {"generation", text_of("2")},
      {"run", object_of({{"request", text_of("req-claim-1")}, {"kind", text_of("claim")}})},
      {"versions",
       array_of({text_of("declarations_v1"), text_of("selection_v1"), text_of("dependencies_v1"), text_of("exact_bytes_v1"),
                 text_of("canonical_v1"), text_of("xxh3_128_v1"), text_of("work_exec_v1"), text_of("dispatch_v1")})},
      {"work", object_of({{"digest", text_of(std::string(fingerprint))}})},
      {"render",
       object_of(
           {{"digest", text_of(std::string(other_fingerprint))}, {"bytes", int_of(1234)}, {"budget_bytes", int_of(1048576)}})},
      {"sections",
       array_of({object_of(
           {{"id", text_of("run")}, {"mandatory", bool_of(true)}, {"included", bool_of(true)}, {"bytes", int_of(40)}})})},
      {"omitted", array_of({})},
      {"prerequisites", array_of({})},
      {"registry", object_of({{"id", text_of("registry" + std::string(registry_padding, 'r'))},
                              {"revision", text_of("3")},
                              {"authority", text_of("driver_selected_trusted_configuration")}})},
      {"inputs", object_of({{"source", text_of("1")},
                            {"interpretation", text_of("2")},
                            {"publications", array_of({})},
                            {"repair", text_of("0")}})},
  });
}

value_ptr work_value() {
  return object_of({
      {"versions",
       array_of({text_of("declarations_v1"), text_of("selection_v1"), text_of("dependencies_v1"), text_of("exact_bytes_v1"),
                 text_of("canonical_v1"), text_of("xxh3_128_v1"), text_of("work_exec_v1")})},
      {"workspace", text_of("workspace")},
      {"spec", text_of("spec")},
      {"job", text_of("parse")},
      {"role", text_of("worker_a")},
      {"root", text_of("1")},
      {"base", object_of({{"status", text_of("declared")}, {"root", text_of("1")}, {"label", text_of("base")}})},
      {"goal", text_of("goal")},
      {"units", array_of({})},
      {"milestones", array_of({})},
      {"constraints", array_of({})},
      {"prescriptions", array_of({})},
      {"contracts", array_of({})},
      {"dependencies", array_of({})},
      {"facets", array_of({})},
      {"pipeline_predecessors", array_of({})},
      {"questions", array_of({})},
      {"ownership", array_of({})},
      {"effective_checks", array_of({})},
      {"checks", array_of({})},
      {"toolchains", array_of({})},
      {"shape", text_of("linear")},
      {"start_facets", array_of({})},
      {"repair", text_of("0")},
  });
}

// ---- provenance document fixtures -----------------------------------------------------------------------

value_ptr unknown_leaf() {
  return object_of({{"status", text_of("unknown")}});
}

value_ptr setting(value_ptr name, value_ptr value) {
  return object_of({{"name", std::move(name)}, {"value", std::move(value)}});
}

std::vector<std::string> base_leaves() {
  return {"driver", "1.0", "trial-1", "view-1", "model-a", "model-a", "session-a", "session-a"};
}

// Driver provenance document in the P2-4 shape; leaves are in the order driver.name, driver.version, trial.id,
// view.id, model.requested, model.actual, session.requested, session.actual.
value_ptr document_from(const std::vector<std::string>& leaves, std::vector<value_ptr> settings) {
  return object_of({
      {"version", int_of(1)},
      {"driver", object_of({{"name", text_of(leaves[0])}, {"version", text_of(leaves[1])}})},
      {"trial", object_of({{"id", text_of(leaves[2])}})},
      {"view", object_of({{"id", text_of(leaves[3])}})},
      {"model", object_of({{"requested", text_of(leaves[4])}, {"actual", text_of(leaves[5])}})},
      {"session", object_of({{"requested", text_of(leaves[6])}, {"actual", text_of(leaves[7])}})},
      {"settings", array_of(std::move(settings))},
  });
}

value_ptr document(std::vector<value_ptr> settings = {}) {
  return document_from(base_leaves(), std::move(settings));
}

value_ptr with_settings(const value_ptr& doc, std::vector<value_ptr> settings) {
  return with_field(doc, "settings", array_of(std::move(settings)));
}

std::string filled(std::string_view prefix, std::size_t length, char fill) {
  REQUIRE(length >= prefix.size());
  std::string s(prefix);
  s.resize(length, fill);
  return s;
}

// Slots are the eight leaves, then the name and value of each setting. Names start with a two-digit index,
// so they stay unique and sorted whatever their padding.
value_ptr sized_document(const std::vector<std::size_t>& lengths) {
  std::vector<std::string> leaves;
  for (std::size_t i = 0; i < 8; ++i)
    leaves.push_back(filled("", lengths[i], 'd'));
  std::vector<value_ptr> settings;
  for (std::size_t i = 8; i < lengths.size(); i += 2) {
    const auto index = (i - 8) / 2;
    settings.push_back(
        setting(text_of(filled(std::format("{:02}", index), lengths[i], 'x')), text_of(filled("", lengths[i + 1], 'v'))));
  }
  return document_from(leaves, std::move(settings));
}

std::vector<std::size_t> minimal_slots(std::size_t settings) {
  std::vector<std::size_t> lengths(8, 1);
  for (std::size_t i = 0; i < settings; ++i) {
    lengths.push_back(2);
    lengths.push_back(1);
  }
  return lengths;
}

// Grows slots in order, each up to 1024 bytes, until the compact JSON of the document is exactly target bytes.
std::vector<std::size_t> slots_for_json(std::size_t settings, std::size_t target) {
  auto       lengths = minimal_slots(settings);
  const auto size    = json(sized_document(lengths)).size();
  REQUIRE(size <= target);
  std::size_t delta = target - size;
  for (auto& length : lengths) {
    const auto grow = std::min<std::size_t>(delta, 1024 - length);
    length += grow;
    delta -= grow;
    if (delta == 0)
      break;
  }
  REQUIRE(delta == 0);
  return lengths;
}

// ---- saved result fixtures ------------------------------------------------------------------------------

claim_result_v1 claim_result(std::optional<std::string> previous = std::nullopt) {
  claim_result_v1 r;
  r.ledger        = std::string(ledger_id);
  r.request_id    = "req-claim-1";
  r.job           = "parse";
  r.role          = "worker_a";
  r.revision      = "5";
  r.run_id        = "7";
  r.generation    = "2";
  r.token         = std::string(run_token);
  r.dispatch_id   = "9";
  r.work_fp       = std::string(fingerprint);
  r.render_digest = std::string(other_fingerprint);
  r.bytes         = 1234;
  r.previous_run  = std::move(previous);
  return r;
}

// Saved claim JSON as the contract spells it, with sorted keys and no terminal LF.
constexpr std::string_view claim_json =
    R"j({"dispatch":{"bytes":1234,"id":"9","render_digest":"xxh3_128:ffeeddccbbaa99887766554433221100","work_fp":"xxh3_128:00112233445566778899aabbccddeeff"},"job":"parse","kind":"claim","ledger":"0123456789abcdef0123456789abcdef","request_id":"req-claim-1","revision":"5","role":"worker_a","run":{"generation":"2","id":"7","token":"7.2.fedcba9876543210fedcba9876543210"},"version":1})j";

constexpr std::string_view takeover_result_json =
    R"j({"dispatch":{"bytes":1234,"id":"9","render_digest":"xxh3_128:ffeeddccbbaa99887766554433221100","work_fp":"xxh3_128:00112233445566778899aabbccddeeff"},"job":"parse","kind":"claim","ledger":"0123456789abcdef0123456789abcdef","previous_run":"6","request_id":"req-claim-1","revision":"5","role":"worker_a","run":{"generation":"2","id":"7","token":"7.2.fedcba9876543210fedcba9876543210"},"version":1})j";

activate_result_v1 activate_result(std::string previous_job) {
  activate_result_v1 r;
  r.ledger       = std::string(ledger_id);
  r.request_id   = "req-activate-1";
  r.revision     = "5";
  r.job          = "parse";
  r.previous_job = std::move(previous_job);
  r.activation   = "3";
  r.event        = "4";
  r.roles        = {{"worker_a", "3", "ready"}, {"worker_b", "3", "blocked"}};
  return r;
}

// Saved activate JSON with roles sorted by role and no terminal LF.
constexpr std::string_view activate_json =
    R"j({"activation":"3","event":"4","job":"parse","kind":"activate","ledger":"0123456789abcdef0123456789abcdef","previous_job":"","request_id":"req-activate-1","revision":"5","roles":[{"generation":"3","role":"worker_a","state":"ready"},{"generation":"3","role":"worker_b","state":"blocked"}],"version":1})j";

// Object form of a claim result with arbitrary field values, used to build decode inputs the encoder refuses.
value_ptr claim_value(const claim_result_v1& r) {
  std::map<std::string, value_ptr> fields{
      {"version", int_of(1)},
      {"kind", text_of("claim")},
      {"ledger", text_of(r.ledger)},
      {"request_id", text_of(r.request_id)},
      {"job", text_of(r.job)},
      {"role", text_of(r.role)},
      {"revision", text_of(r.revision)},
      {"run", object_of({{"id", text_of(r.run_id)}, {"generation", text_of(r.generation)}, {"token", text_of(r.token)}})},
      {"dispatch", object_of({{"id", text_of(r.dispatch_id)},
                              {"work_fp", text_of(r.work_fp)},
                              {"render_digest", text_of(r.render_digest)},
                              {"bytes", int_of(static_cast<std::int64_t>(r.bytes))}})},
  };
  if (r.previous_run)
    fields["previous_run"] = text_of(*r.previous_run);
  return object_of(std::move(fields));
}

value_ptr activate_value(const activate_result_v1& r) {
  std::vector<value_ptr> roles;
  for (const auto& role : r.roles)
    roles.push_back(
        object_of({{"role", text_of(role.role)}, {"generation", text_of(role.generation)}, {"state", text_of(role.state)}}));
  return object_of({
      {"version", int_of(1)},
      {"kind", text_of("activate")},
      {"ledger", text_of(r.ledger)},
      {"request_id", text_of(r.request_id)},
      {"revision", text_of(r.revision)},
      {"job", text_of(r.job)},
      {"previous_job", text_of(r.previous_job)},
      {"activation", text_of(r.activation)},
      {"event", text_of(r.event)},
      {"roles", array_of(std::move(roles))},
  });
}
} // namespace

TEST_CASE("execution payload v2 canonical shape", "[execution_codec]") {
  auto accepted = encode_event_payload_v2("declaration_accepted", accepted_payload());
  REQUIRE(accepted);
  CHECK(*accepted == accepted_json);
  CHECK(accepted->back() == '}');
  CHECK(accepted->find('\n') == std::string::npos);

  auto noop = encode_event_payload_v2("load_noop", noop_payload());
  REQUIRE(noop);
  CHECK(*noop == noop_json);

  auto activated = encode_event_payload_v2("job_activated", activation_payload());
  REQUIRE(activated);
  CHECK(*activated == activation_json);

  auto claimed = encode_event_payload_v2("run_claimed", claimed_payload());
  REQUIRE(claimed);
  CHECK(*claimed == claimed_json);

  auto provenance = encode_event_payload_v2("run_claimed", claimed_with_provenance_payload());
  REQUIRE(provenance);
  CHECK(*provenance == claimed_provenance_json);

  auto takeover = encode_event_payload_v2("run_taken_over", takeover_payload());
  REQUIRE(takeover);
  CHECK(*takeover == takeover_json);
}

TEST_CASE("execution payload v2 position rules", "[execution_codec]") {
  auto encoded = encode_event_payload_v2("declaration_accepted", accepted_payload());
  REQUIRE(encoded);
  // The empty job is the empty string and never the text none.
  CHECK(encoded->find(R"j("active_job":"")j") != std::string::npos);
  CHECK(encoded->find(R"j("active_job":"none")j") == std::string::npos);
  CHECK(encoded->find(R"j("active_revision":"none")j") != std::string::npos);

  // none is a legal job identifier and stays a job string.
  auto named         = accepted_payload();
  named.after        = {"1", "none"};
  auto with_none_job = encode_event_payload_v2("declaration_accepted", named);
  REQUIRE(with_none_job);
  CHECK(with_none_job->find(R"j("active_job":"none")j") != std::string::npos);

  for (std::string_view job : {"Parse", "1parse", "job-a", "job a"}) {
    auto bad  = accepted_payload();
    bad.after = {"1", std::string(job)};
    CHECK_FALSE(encode_event_payload_v2("declaration_accepted", bad));
  }
  for (std::string_view revision : {"0", "01", "x", "+1", "-1", "", "1 "}) {
    auto bad                   = accepted_payload();
    bad.before.active_revision = std::string(revision);
    CHECK_FALSE(encode_event_payload_v2("declaration_accepted", bad));
  }

  // Decoding maps the empty job back to the empty string and keeps none as a revision position.
  auto decoded = decode_event_payload_v2("declaration_accepted", *encoded);
  REQUIRE(decoded);
  CHECK(decoded->after.active_job.empty());
  CHECK(decoded->before.active_revision == "none");
  CHECK(decoded->after.active_revision == "1");
}

TEST_CASE("execution payload v2 reference key sets per event type", "[execution_codec]") {
  REQUIRE(encode_event_payload_v2("declaration_accepted", accepted_payload()));
  REQUIRE(encode_event_payload_v2("load_noop", noop_payload()));
  REQUIRE(encode_event_payload_v2("job_activated", activation_payload()));
  REQUIRE(encode_event_payload_v2("run_claimed", claimed_payload()));
  REQUIRE(encode_event_payload_v2("run_claimed", claimed_with_provenance_payload()));
  REQUIRE(encode_event_payload_v2("run_taken_over", takeover_payload()));

  auto missing_source = accepted_payload();
  missing_source.references.erase("versions");
  CHECK_FALSE(encode_event_payload_v2("declaration_accepted", missing_source));
  auto extra_provenance                     = accepted_payload();
  extra_provenance.references["provenance"] = "14";
  CHECK_FALSE(encode_event_payload_v2("declaration_accepted", extra_provenance));
  auto noop_extra              = noop_payload();
  noop_extra.references["run"] = "7";
  CHECK_FALSE(encode_event_payload_v2("load_noop", noop_extra));

  auto activation_extra                 = activation_payload();
  activation_extra.references["source"] = "1";
  CHECK_FALSE(encode_event_payload_v2("job_activated", activation_extra));
  auto activation_missing = activation_payload();
  activation_missing.references.erase("revision");
  CHECK_FALSE(encode_event_payload_v2("job_activated", activation_missing));

  auto claim_missing = claimed_payload();
  claim_missing.references.erase("packet");
  CHECK_FALSE(encode_event_payload_v2("run_claimed", claim_missing));
  auto claim_previous                       = claimed_payload();
  claim_previous.references["previous_run"] = "6";
  CHECK_FALSE(encode_event_payload_v2("run_claimed", claim_previous));

  auto takeover_missing = takeover_payload();
  takeover_missing.references.erase("previous_run");
  CHECK_FALSE(encode_event_payload_v2("run_taken_over", takeover_missing));

  // Ledger initialization keeps format2 payloads, and the reserved later-round events have no version 2 key set yet.
  for (std::string_view type : {"ledger_initialized", "run_refreshed", "checkpoint_recorded", "run_released", "run_interrupted",
                                "check_reserved", "check_recorded", "check_interrupted", "verdict_recorded", "bogus"})
    CHECK_FALSE(encode_event_payload_v2(type, accepted_payload()));

  for (std::string_view value : {"0", "01", "x", "+1", "-1", "", "18446744073709551616"}) {
    auto bad                 = accepted_payload();
    bad.references["source"] = std::string(value);
    CHECK_FALSE(encode_event_payload_v2("declaration_accepted", bad));
  }
  auto largest                 = accepted_payload();
  largest.references["source"] = "18446744073709551615";
  CHECK(encode_event_payload_v2("declaration_accepted", largest));
}

TEST_CASE("execution payload v2 strict decode", "[execution_codec]") {
  auto decoded = decode_event_payload_v2("declaration_accepted", accepted_json);
  REQUIRE(decoded);
  CHECK(decoded->operation == "load");
  CHECK(decoded->before.active_revision == "none");
  CHECK(decoded->after.active_revision == "1");
  CHECK(decoded->reason == "structural_change");
  CHECK(decoded->references.size() == 5);
  REQUIRE(decode_event_payload_v2("load_noop", noop_json));
  REQUIRE(decode_event_payload_v2("job_activated", activation_json));
  REQUIRE(decode_event_payload_v2("run_claimed", claimed_json));
  REQUIRE(decode_event_payload_v2("run_taken_over", takeover_json));

  const std::string      base(accepted_json);
  const std::string_view type = "declaration_accepted";
  CHECK_FALSE(decode_event_payload_v2(type, replaced(base, R"j("version":2)j", R"j("version":1)j")));
  CHECK_FALSE(decode_event_payload_v2(type, replaced(base, R"j("version":2)j", R"j("version":3)j")));
  CHECK_FALSE(decode_event_payload_v2(type, replaced(base, R"j("version":2)j", R"j("version":"2")j")));
  CHECK_FALSE(decode_event_payload_v2(type, replaced(base, R"j("version":2)j", R"j("extra":"x","version":2)j")));
  CHECK_FALSE(decode_event_payload_v2(type, without_part(base, R"j("reason":"structural_change",)j")));
  CHECK_FALSE(decode_event_payload_v2(type, replaced(base, R"j("version":2)j", R"j("version":2,"version":2)j")));
  CHECK_FALSE(
      decode_event_payload_v2(type, replaced(base, R"j("operation":"load",)j", R"j("operation":"load","operation":"load",)j")));
  CHECK_FALSE(decode_event_payload_v2(type, base + "x"));
  CHECK_FALSE(decode_event_payload_v2(type, base + "\n"));
  CHECK_FALSE(decode_event_payload_v2(type, base + " "));
  CHECK_FALSE(decode_event_payload_v2(type, replaced(base, R"j("source":"1")j", R"j("source":"01")j")));

  // Reference key sets and event types are checked on decode as well.
  CHECK_FALSE(decode_event_payload_v2("declaration_accepted", claimed_json));
  CHECK_FALSE(decode_event_payload_v2("run_claimed", accepted_json));
  CHECK_FALSE(decode_event_payload_v2("ledger_initialized", accepted_json));
}

TEST_CASE("execution event reason is the ordinal one transition reason", "[execution_codec]") {
  const std::vector<transition_row> two{row(1, "activated"), row(2, "fenced")};
  CHECK(event_level_reason(two) == "activated");
  const std::vector<transition_row> three{row(1, "claimed"), row(2, "fenced"), row(3, "released")};
  CHECK(event_level_reason(three) == "claimed");
  const std::vector<transition_row> one{row(1, "completed")};
  CHECK(event_level_reason(one) == "completed");
  const std::vector<transition_row> none{};
  CHECK(event_level_reason(none).empty());
}

TEST_CASE("execution transition row closed rules", "[execution_codec]") {
  const transition_row base = row(1, "activated");
  REQUIRE(validate_transition_row(base));

  for (std::string_view state : {"blocked", "ready", "running", "done", "stale", "retired"}) {
    auto after        = base;
    after.after_state = std::string(state);
    CHECK(validate_transition_row(after));
    auto before         = base;
    before.before_state = std::string(state);
    CHECK(validate_transition_row(before));
  }
  for (std::string_view state : {"active", "fenced", "", "Ready", "claimed"}) {
    auto after        = base;
    after.after_state = std::string(state);
    CHECK_FALSE(validate_transition_row(after));
    auto before         = base;
    before.before_state = std::string(state);
    CHECK_FALSE(validate_transition_row(before));
  }

  for (std::string_view reason : {"activated", "prerequisite_available", "prerequisite_unavailable", "claimed", "taken_over",
                                  "fenced", "released", "interrupted", "completed", "work_changed", "evidence_inapplicable",
                                  "repair_admitted", "structural_change", "retired", "recreated", "drift_cleared"}) {
    auto r   = base;
    r.reason = std::string(reason);
    CHECK(validate_transition_row(r));
  }
  for (std::string_view reason : {"bogus", "", "Claimed", "retire", "fence"}) {
    auto r   = base;
    r.reason = std::string(reason);
    CHECK_FALSE(validate_transition_row(r));
  }

  // After drift is true only for a running after state.
  auto drifted          = base;
  drifted.after_drifted = true;
  CHECK_FALSE(validate_transition_row(drifted));
  for (std::string_view state : {"blocked", "done", "stale", "retired"}) {
    auto r          = base;
    r.after_state   = std::string(state);
    r.after_drifted = true;
    CHECK_FALSE(validate_transition_row(r));
  }
  auto running          = base;
  running.after_state   = "running";
  running.after_drifted = true;
  CHECK(validate_transition_row(running));

  // Before drift is NULL exactly when the before state is NULL, and false unless the before state is running.
  auto no_before = base;
  no_before.before_state.reset();
  CHECK_FALSE(validate_transition_row(no_before));
  auto no_drift = base;
  no_drift.before_drifted.reset();
  CHECK_FALSE(validate_transition_row(no_drift));
  auto ready_drift           = base;
  ready_drift.before_state   = "ready";
  ready_drift.before_drifted = true;
  CHECK_FALSE(validate_transition_row(ready_drift));
  auto running_drift           = base;
  running_drift.before_state   = "running";
  running_drift.before_drifted = true;
  CHECK(validate_transition_row(running_drift));

  auto zero_ordinal    = base;
  zero_ordinal.ordinal = 0;
  CHECK_FALSE(validate_transition_row(zero_ordinal));
  auto zero_generation          = base;
  zero_generation.generation_id = 0;
  CHECK_FALSE(validate_transition_row(zero_generation));
}

TEST_CASE("execution transition sequence contiguity", "[execution_codec]") {
  const std::vector<transition_row> none{};
  CHECK(validate_transition_sequence(none));
  const std::vector<transition_row> one{row(1, "activated")};
  REQUIRE(validate_transition_sequence(one));
  const std::vector<transition_row> three{row(1, "activated"), row(2, "fenced"), row(3, "released")};
  REQUIRE(validate_transition_sequence(three));

  const std::vector<transition_row> gap{row(1, "activated"), row(3, "released")};
  CHECK_FALSE(validate_transition_sequence(gap));
  const std::vector<transition_row> from_two{row(2, "fenced"), row(3, "released")};
  CHECK_FALSE(validate_transition_sequence(from_two));
  const std::vector<transition_row> duplicate{row(1, "activated"), row(1, "fenced")};
  CHECK_FALSE(validate_transition_sequence(duplicate));
  const std::vector<transition_row> zero{row(0, "activated")};
  CHECK_FALSE(validate_transition_sequence(zero));
  const std::vector<transition_row> bad_row{row(1, "bogus")};
  CHECK_FALSE(validate_transition_sequence(bad_row));
}

TEST_CASE("role structure v1 digest covers every field", "[execution_codec]") {
  const auto base   = structure();
  auto       digest = role_structure_digest(base);
  REQUIRE(digest);
  CHECK(digest->starts_with("xxh3_128:"));
  CHECK(digest->size() == 9 + 32);

  auto encoded = encode_role_structure_v1(base);
  REQUIRE(encoded);
  auto decoded = decode_role_structure_v1(*encoded);
  REQUIRE(decoded);
  CHECK(decoded->job == base.job);
  CHECK(decoded->role == base.role);
  CHECK(decoded->kind == base.kind);
  CHECK(decoded->work == base.work);
  CHECK(decoded->targets == base.targets);
  CHECK(decoded->ownership.size() == base.ownership.size());
  CHECK(decoded->shape == base.shape);
  CHECK(encode_role_structure_v1(*decoded).value_or(std::string()) == *encoded);
  CHECK(role_structure_digest(*decoded).value_or(std::string()) == *digest);

  auto changed = [&](auto mutate) {
    auto variant = base;
    mutate(variant);
    auto changed_digest = role_structure_digest(variant);
    REQUIRE(changed_digest);
    CHECK(*changed_digest != *digest);
  };
  changed([](role_structure_v1& s) { s.job = "parse_two"; });
  changed([](role_structure_v1& s) { s.role = "worker_b"; });
  changed([](role_structure_v1& s) { s.kind = "reviewer"; });
  changed([](role_structure_v1& s) { s.work.push_back("u3"); });
  changed([](role_structure_v1& s) { s.targets = {"t1"}; });
  changed([](role_structure_v1& s) { s.ownership[0].paths = {"src/other.lua"}; });
  changed([](role_structure_v1& s) { s.ownership.push_back({"u3", {"src/d.lua"}}); });
  changed([](role_structure_v1& s) { s.shape = "fanout"; });
}

TEST_CASE("role structure v1 strict validation", "[execution_codec]") {
  REQUIRE(encode_role_structure_v1(structure()));

  auto unsorted_work = structure();
  unsorted_work.work = {"u2", "u1"};
  CHECK_FALSE(encode_role_structure_v1(unsorted_work));
  auto duplicate_work = structure();
  duplicate_work.work = {"u1", "u1"};
  CHECK_FALSE(encode_role_structure_v1(duplicate_work));
  auto unsorted_targets    = structure();
  unsorted_targets.targets = {"t2", "t1"};
  CHECK_FALSE(encode_role_structure_v1(unsorted_targets));
  auto duplicate_targets    = structure();
  duplicate_targets.targets = {"t1", "t1"};
  CHECK_FALSE(encode_role_structure_v1(duplicate_targets));
  auto unsorted_ownership      = structure();
  unsorted_ownership.ownership = {{"u2", {"a"}}, {"u1", {"b"}}};
  CHECK_FALSE(encode_role_structure_v1(unsorted_ownership));
  auto duplicate_ownership      = structure();
  duplicate_ownership.ownership = {{"u1", {"a"}}, {"u1", {"b"}}};
  CHECK_FALSE(encode_role_structure_v1(duplicate_ownership));

  for (std::string_view kind : {"lead", "", "Worker", "worker "}) {
    auto bad = structure();
    bad.kind = std::string(kind);
    CHECK_FALSE(encode_role_structure_v1(bad));
  }
  auto reviewer    = structure();
  reviewer.kind    = "reviewer";
  reviewer.work    = {};
  reviewer.targets = {"t1"};
  CHECK(encode_role_structure_v1(reviewer));

  // Decoding applies the same validation to canonical bytes.
  auto raw_unsorted = object_of({{"job", text_of("parse")},
                                 {"role", text_of("worker_a")},
                                 {"kind", text_of("worker")},
                                 {"work", array_of({text_of("u2"), text_of("u1")})},
                                 {"targets", array_of({})},
                                 {"ownership", array_of({})},
                                 {"shape", text_of("linear")}});
  CHECK_FALSE(decode_role_structure_v1(canonical(raw_unsorted)));
  auto raw_kind = with_field(raw_unsorted, "work", array_of({text_of("u1")}));
  raw_kind      = with_field(raw_kind, "kind", text_of("lead"));
  CHECK_FALSE(decode_role_structure_v1(canonical(raw_kind)));
  auto valid_bytes = encode_role_structure_v1(structure());
  REQUIRE(valid_bytes);
  CHECK(decode_role_structure_v1(*valid_bytes));
}

TEST_CASE("dispatch manifest v1 exact key set", "[execution_codec]") {
  const auto good    = manifest(0);
  auto       encoded = encode_dispatch_manifest_v1(good);
  REQUIRE(encoded);
  CHECK(*encoded == canonical(good));
  auto decoded = decode_dispatch_manifest_v1(*encoded);
  REQUIRE(decoded);
  CHECK((*decoded)->fields.size() == 14);
  CHECK(canonical(*decoded) == *encoded);

  for (std::string_view key : {"token", "secret", "files", "extra"}) {
    auto refused = encode_dispatch_manifest_v1(with_field(good, std::string(key), text_of("x")));
    REQUIRE_FALSE(refused);
    CHECK(refused.error().code == execution_codec_code::invalid);
  }
  auto missing = encode_dispatch_manifest_v1(without_field(good, "inputs"));
  REQUIRE_FALSE(missing);
  CHECK(missing.error().code == execution_codec_code::invalid);

  // Stored bytes with a forbidden key are refused on decode.
  CHECK_FALSE(decode_dispatch_manifest_v1(canonical(with_field(good, "token", text_of("x")))));
}

TEST_CASE("dispatch manifest bound at limit and limit plus one", "[execution_codec]") {
  constexpr std::size_t limit     = 16777216;
  const std::size_t     base_size = canonical(manifest(0)).size();
  REQUIRE(base_size < limit);

  const auto at = manifest(limit - base_size);
  REQUIRE(canonical(at).size() == limit);
  auto encoded = encode_dispatch_manifest_v1(at);
  REQUIRE(encoded);
  CHECK(encoded->size() == limit);
  CHECK(*encoded == canonical(at));

  const auto over = manifest(limit - base_size + 1);
  REQUIRE(canonical(over).size() == limit + 1);
  auto refused = encode_dispatch_manifest_v1(over);
  REQUIRE_FALSE(refused);
  CHECK(refused.error().code == execution_codec_code::resource_limit);
  CHECK(refused.error().limit == "manifest_bytes");
  CHECK(refused.error().maximum == limit);
}

TEST_CASE("work exec v1 field set and digest domain", "[execution_codec]") {
  const auto work = work_value();
  REQUIRE(work->fields.size() == 24);
  auto encoded = encode_work_exec_v1(work);
  REQUIRE(encoded);
  CHECK(*encoded == canonical(work));
  auto decoded = decode_work_exec_v1(*encoded);
  REQUIRE(decoded);
  CHECK(canonical(*decoded) == *encoded);

  CHECK_FALSE(encode_work_exec_v1(with_field(work, "generation", text_of("2"))));
  CHECK_FALSE(encode_work_exec_v1(without_field(work, "repair")));

  auto digest = work_exec_digest(work);
  REQUIRE(digest);
  auto expected = hash_value("work_exec_v1", work);
  REQUIRE(expected);
  CHECK(*digest == *expected);
  auto wrong_domain = hash_value("work_v1", work);
  REQUIRE(wrong_domain);
  CHECK(*digest != *wrong_domain);
}

TEST_CASE("driver provenance leaf rule", "[execution_codec]") {
  REQUIRE(encode_driver_provenance_v1(document({setting(text_of("mode"), text_of("fast"))})));

  auto empty_name = base_leaves();
  empty_name[0]   = "";
  auto refused    = encode_driver_provenance_v1(document_from(empty_name, {}));
  REQUIRE_FALSE(refused);
  CHECK(refused.error().code == execution_codec_code::invalid);

  auto nul_name = base_leaves();
  nul_name[0]   = std::string("a\0b", 3);
  CHECK_FALSE(encode_driver_provenance_v1(document_from(nul_name, {})));

  // Exactly {status:"unknown"} is the only object leaf.
  REQUIRE(encode_driver_provenance_v1(document()));
  auto unknown_document = with_field(document(), "driver", object_of({{"name", unknown_leaf()}, {"version", text_of("1.0")}}));
  REQUIRE(encode_driver_provenance_v1(unknown_document));
  for (auto leaf :
       {object_of({{"status", text_of("known")}}), object_of({{"status", text_of("unknown")}, {"extra", text_of("x")}}),
        object_of(std::map<std::string, value_ptr>{}), object_of({{"status", int_of(1)}})}) {
    CHECK_FALSE(
        encode_driver_provenance_v1(with_field(document(), "driver", object_of({{"name", leaf}, {"version", text_of("1.0")}}))));
  }

  // Setting names are string leaves, unique, and unsorted input normalizes to the sorted bytes.
  CHECK_FALSE(encode_driver_provenance_v1(with_settings(document(), {setting(unknown_leaf(), text_of("fast"))})));
  CHECK_FALSE(encode_driver_provenance_v1(with_settings(document(), {setting(text_of(""), text_of("fast"))})));
  CHECK_FALSE(encode_driver_provenance_v1(
      with_settings(document(), {setting(text_of("mode"), text_of("a")), setting(text_of("mode"), text_of("b"))})));
  auto unsorted =
      encode_driver_provenance_v1(document({setting(text_of("b"), text_of("2")), setting(text_of("a"), text_of("1"))}));
  auto sorted = encode_driver_provenance_v1(document({setting(text_of("a"), text_of("1")), setting(text_of("b"), text_of("2"))}));
  REQUIRE(unsorted);
  REQUIRE(sorted);
  CHECK(*unsorted == *sorted);

  // Shape violations are invalid, never resource limits.
  auto missing_trial = encode_driver_provenance_v1(without_field(document(), "trial"));
  REQUIRE_FALSE(missing_trial);
  CHECK(missing_trial.error().code == execution_codec_code::invalid);
  auto unknown_top = encode_driver_provenance_v1(with_field(document(), "extra", text_of("x")));
  REQUIRE_FALSE(unknown_top);
  CHECK(unknown_top.error().code == execution_codec_code::invalid);
}

TEST_CASE("driver provenance bounds at limit and limit plus one", "[execution_codec]") {
  // Provenance bytes are the compact JSON of the document: exactly 65536 is accepted, one more is refused.
  const auto at = sized_document(slots_for_json(32, 65536));
  REQUIRE(json(at).size() == 65536);
  auto encoded = encode_driver_provenance_v1(at);
  REQUIRE(encoded);
  // The canonical_v1 size of the same document exceeds 65536 and is still accepted.
  REQUIRE(canonical(at).size() > 65536);
  auto decoded = decode_driver_provenance_v1(*encoded);
  REQUIRE(decoded);
  CHECK(json(*decoded) == json(at));

  const auto over = sized_document(slots_for_json(32, 65537));
  REQUIRE(json(over).size() == 65537);
  auto refused = encode_driver_provenance_v1(over);
  REQUIRE_FALSE(refused);
  CHECK(refused.error().code == execution_codec_code::resource_limit);
  CHECK(refused.error().limit == "provenance_bytes");
  CHECK(refused.error().maximum == 65536);

  // Escape guard: these characters are written raw, so the measure never exceeds a captured file.
  const std::string escapes = "/\x7f\xe2\x80\xa8\xc3\xa9\xf0\x9f\x98\x80";
  const auto        escaped = document({setting(text_of("mode"), text_of(escapes))});
  REQUIRE(encode_driver_provenance_v1(escaped));
  CHECK(json(escaped).find(escapes) != std::string::npos);

  // Depth counts the document root as one: the settings value object at depth four is the deepest legal shape.
  REQUIRE(encode_driver_provenance_v1(document({setting(text_of("mode"), unknown_leaf())})));
  const auto too_deep     = document({setting(text_of("mode"), object_of({{"status", object_of({{"unknown", text_of("x")}})}}))});
  auto       deep_refused = encode_driver_provenance_v1(too_deep);
  REQUIRE_FALSE(deep_refused);
  CHECK(deep_refused.error().code == execution_codec_code::resource_limit);
  CHECK(deep_refused.error().limit == "provenance_depth");
  CHECK(deep_refused.error().maximum == 4);

  // Settings: 32 are accepted, 33 are refused.
  const auto thirty_three = sized_document(minimal_slots(33));
  auto       too_many     = encode_driver_provenance_v1(thirty_three);
  REQUIRE_FALSE(too_many);
  CHECK(too_many.error().code == execution_codec_code::resource_limit);
  CHECK(too_many.error().limit == "provenance_settings");
  CHECK(too_many.error().maximum == 32);

  // Strings: 1024 bytes are accepted, 1025 are refused as a resource limit, not a shape error.
  auto leaves = base_leaves();
  leaves[0]   = std::string(1024, 'd');
  REQUIRE(encode_driver_provenance_v1(document_from(leaves, {})));
  leaves[0]        = std::string(1025, 'd');
  auto long_string = encode_driver_provenance_v1(document_from(leaves, {}));
  REQUIRE_FALSE(long_string);
  CHECK(long_string.error().code == execution_codec_code::resource_limit);
  CHECK(long_string.error().limit == "provenance_string_bytes");
  CHECK(long_string.error().maximum == 1024);

  // Bounds win over shape errors.
  auto shaped = encode_driver_provenance_v1(with_field(thirty_three, "extra", text_of("x")));
  REQUIRE_FALSE(shaped);
  CHECK(shaped.error().code == execution_codec_code::resource_limit);
  CHECK(shaped.error().limit == "provenance_settings");
}

TEST_CASE("driver provenance stored record shape", "[execution_codec]") {
  const auto doc    = document({setting(text_of("mode"), text_of("fast"))});
  auto       stored = encode_driver_provenance_v1(doc);
  REQUIRE(stored);

  auto record = canonical_decode(*stored);
  REQUIRE(record);
  REQUIRE((*record)->kind == value_kind::record);
  REQUIRE((*record)->fields.size() == 3);
  REQUIRE(((*record)->fields.contains("version")));
  REQUIRE(((*record)->fields.contains("authority")));
  REQUIRE(((*record)->fields.contains("document")));
  CHECK((*record)->fields.at("version")->integer == 1);
  CHECK((*record)->fields.at("authority")->string == "driver_assertion_unverified");
  CHECK(canonical((*record)->fields.at("document")) == canonical(doc));

  auto decoded = decode_driver_provenance_v1(*stored);
  REQUIRE(decoded);
  CHECK(json(*decoded) == json(doc));

  auto other_authority = object_of({{"version", int_of(1)}, {"authority", text_of("driver_verified")}, {"document", doc}});
  CHECK_FALSE(decode_driver_provenance_v1(canonical(other_authority)));
  auto other_version =
      object_of({{"version", int_of(2)}, {"authority", text_of("driver_assertion_unverified")}, {"document", doc}});
  CHECK_FALSE(decode_driver_provenance_v1(canonical(other_version)));

  // The same bounds apply to a stored document.
  auto stored_over = object_of({{"version", int_of(1)},
                                {"authority", text_of("driver_assertion_unverified")},
                                {"document", sized_document(minimal_slots(33))}});
  CHECK_FALSE(decode_driver_provenance_v1(canonical(stored_over)));
}

TEST_CASE("execution value record framing and codec ids", "[execution_codec]") {
  const std::string payload = canonical(object_of({{"version", int_of(1)}}));
  for (std::string_view codec :
       {codec_dispatch_manifest_v1, codec_work_exec_v1, codec_driver_provenance_v1, codec_role_structure_v1}) {
    const auto expected_frame = value_frame(codec, payload);
    auto       frame          = encode_execution_value_record(codec, payload);
    REQUIRE(frame);
    CHECK(*frame == expected_frame);

    auto decoded = decode_execution_value_record(*frame);
    REQUIRE(decoded);
    CHECK(decoded->codec == codec);
    CHECK(decoded->encoded == payload);

    auto digest = digest_execution_value_record(*frame);
    REQUIRE(digest);
    CHECK(*digest == xxh3(expected_frame));

    for (std::size_t i = 0; i < expected_frame.size(); ++i)
      CHECK_FALSE(decode_execution_value_record(expected_frame.substr(0, i)));
    CHECK_FALSE(decode_execution_value_record(expected_frame + '\0'));

    auto file_kind = expected_frame;
    file_kind[14]  = '\x01';
    CHECK_FALSE(decode_execution_value_record(file_kind));
    auto version_two = expected_frame;
    version_two[13]  = '\x02';
    CHECK_FALSE(decode_execution_value_record(version_two));
  }

  auto manifest_frame = encode_execution_value_record(codec_dispatch_manifest_v1, payload);
  auto work_frame     = encode_execution_value_record(codec_work_exec_v1, payload);
  REQUIRE(manifest_frame);
  REQUIRE(work_frame);
  auto manifest_digest = digest_execution_value_record(*manifest_frame);
  auto work_digest     = digest_execution_value_record(*work_frame);
  REQUIRE(manifest_digest);
  REQUIRE(work_digest);
  CHECK(*manifest_digest != *work_digest);

  for (std::string_view codec :
       {"canonical_v1", "work_v1", "", "dispatch_manifest", "Dispatch_manifest_v1", "driver_provenance_v2"}) {
    CHECK_FALSE(encode_execution_value_record(codec, payload));
    CHECK_FALSE(decode_execution_value_record(value_frame(codec, payload)));
  }
}

TEST_CASE("claim result v1 saved JSON shape", "[execution_codec]") {
  auto claim = encode_claim_result_v1(claim_result());
  REQUIRE(claim);
  CHECK(*claim == claim_json);
  CHECK(claim->find("packet") == std::string::npos);
  CHECK(claim->find("manifest") == std::string::npos);
  CHECK(claim->find("previous_run") == std::string::npos);
  CHECK(claim->back() == '}');

  auto takeover = encode_claim_result_v1(claim_result(std::string("6")));
  REQUIRE(takeover);
  CHECK(*takeover == takeover_result_json);

  auto decoded = decode_claim_result_v1(claim_json);
  REQUIRE(decoded);
  CHECK(decoded->token == std::string(run_token));
  CHECK(decoded->bytes == 1234);
  CHECK_FALSE(decoded->previous_run);
  auto decoded_takeover = decode_claim_result_v1(takeover_result_json);
  REQUIRE(decoded_takeover);
  CHECK(decoded_takeover->previous_run == std::optional<std::string>("6"));

  // Extra keys are refused on decode.
  const std::string with_packet = std::string(R"j({"packet":{},)j") + std::string(claim_json.substr(1));
  CHECK_FALSE(decode_claim_result_v1(with_packet));
}

TEST_CASE("activate result v1 saved JSON shape", "[execution_codec]") {
  auto encoded = encode_activate_result_v1(activate_result(""));
  REQUIRE(encoded);
  CHECK(*encoded == activate_json);
  CHECK(encoded->back() == '}');

  auto with_previous = encode_activate_result_v1(activate_result("old_job"));
  REQUIRE(with_previous);
  CHECK(with_previous->find(R"j("previous_job":"old_job")j") != std::string::npos);

  auto decoded = decode_activate_result_v1(activate_json);
  REQUIRE(decoded);
  CHECK(decoded->roles.size() == 2);
  CHECK(decoded->roles[0].role == "worker_a");
  CHECK(decoded->previous_job.empty());
  CHECK(decode_activate_result_v1(*encoded).has_value());

  // An empty previous job is still present in the saved form.
  CHECK_FALSE(decode_activate_result_v1(without_part(std::string(activate_json), R"j("previous_job":"",)j")));
}

TEST_CASE("dispatch packet value assembly", "[execution_codec]") {
  auto packet =
      assemble_packet_value(packet_fields{"workspace", "spec", "parse", "worker_a", "worker", {"u1", "u2"}, "# Packet\n"});
  REQUIRE(packet);
  REQUIRE((*packet)->kind == value_kind::record);
  const auto& fields = (*packet)->fields;
  REQUIRE(fields.size() == 9);
  for (std::string_view key : {"version", "mode", "workspace", "spec", "job", "role", "kind", "work", "markdown"})
    CHECK(fields.contains(std::string(key)));
  CHECK(fields.at("version")->integer == 1);
  CHECK(fields.at("mode")->string == "dispatch");
  REQUIRE(fields.at("work")->kind == value_kind::list);
  REQUIRE(fields.at("work")->items.size() == 2);
  CHECK(fields.at("work")->items[0]->string == "u1");
  CHECK(fields.at("work")->items[1]->string == "u2");
  CHECK(
      json(*packet) ==
      std::string(
          R"j({"job":"parse","kind":"worker","markdown":"# Packet\n","mode":"dispatch","role":"worker_a","spec":"spec","version":1,"work":["u1","u2"],"workspace":"workspace"})j"));

  auto unsorted = assemble_packet_value(packet_fields{"workspace", "spec", "parse", "worker_a", "worker", {"u2", "u1"}, ""});
  REQUIRE_FALSE(unsorted);
  CHECK(unsorted.error().code == execution_codec_code::invalid);
  auto duplicate = assemble_packet_value(packet_fields{"workspace", "spec", "parse", "worker_a", "worker", {"u1", "u1"}, ""});
  REQUIRE_FALSE(duplicate);
  CHECK(duplicate.error().code == execution_codec_code::invalid);
  auto none = assemble_packet_value(packet_fields{"workspace", "spec", "parse", "worker_a", "worker", {}, ""});
  REQUIRE(none);
  CHECK(json(*none).find(R"j("work":[])j") != std::string::npos);
}

TEST_CASE("execution encoders validate field grammars and size", "[execution_codec]") {
  const std::string job64 = "a" + std::string(63, 'b');
  const std::string job65 = "a" + std::string(64, 'b');

  // Payload: active_job of 64 bytes is accepted, 65 bytes is invalid.
  auto payload             = accepted_payload();
  payload.after.active_job = job64;
  REQUIRE(encode_event_payload_v2("declaration_accepted", payload));
  payload.after.active_job = job65;
  auto long_job            = encode_event_payload_v2("declaration_accepted", payload);
  REQUIRE_FALSE(long_job);
  CHECK(long_job.error().code == execution_codec_code::invalid);
  CHECK(long_job.error().limit.empty());

  auto unknown_reason   = accepted_payload();
  unknown_reason.reason = "bogus";
  auto bad_reason       = encode_event_payload_v2("declaration_accepted", unknown_reason);
  REQUIRE_FALSE(bad_reason);
  CHECK(bad_reason.error().code == execution_codec_code::invalid);
  // EX-4 event payload and D101: a load with no transitions has an empty reason; an activation always carries one.
  auto empty_reason   = accepted_payload();
  empty_reason.reason = "";
  CHECK(encode_event_payload_v2("declaration_accepted", empty_reason));
  auto empty_activation   = activation_payload();
  empty_activation.reason = "";
  CHECK_FALSE(encode_event_payload_v2("job_activated", empty_activation));
  auto unknown_operation      = accepted_payload();
  unknown_operation.operation = "history";
  CHECK_FALSE(encode_event_payload_v2("declaration_accepted", unknown_operation));

  // Claim result: job, role, identifiers, grammars and the run token.
  REQUIRE(encode_claim_result_v1(claim_result()));
  auto claim_job = claim_result();
  claim_job.job  = job64;
  REQUIRE(encode_claim_result_v1(claim_job));
  claim_job.job   = job65;
  auto claim_long = encode_claim_result_v1(claim_job);
  REQUIRE_FALSE(claim_long);
  CHECK(claim_long.error().code == execution_codec_code::invalid);
  auto claim_role = claim_result();
  claim_role.role = job64;
  REQUIRE(encode_claim_result_v1(claim_role));
  claim_role.role = job65;
  CHECK_FALSE(encode_claim_result_v1(claim_role));

  for (std::string_view ledger : {"0123456789ABCDEF0123456789ABCDEF", "0123456789abcdef0123456789abcde",
                                  "0123456789abcdef0123456789abcdefa", "0123456789abcdef0123456789abcdeg"}) {
    auto bad   = claim_result();
    bad.ledger = std::string(ledger);
    CHECK_FALSE(encode_claim_result_v1(bad));
  }

  auto request_id       = claim_result();
  request_id.request_id = std::string(128, 'a');
  REQUIRE(encode_claim_result_v1(request_id));
  request_id.request_id = std::string(129, 'a');
  CHECK_FALSE(encode_claim_result_v1(request_id));
  request_id.request_id = "";
  CHECK_FALSE(encode_claim_result_v1(request_id));
  request_id.request_id = "req 1";
  CHECK_FALSE(encode_claim_result_v1(request_id));

  for (std::string_view token :
       {"7.2.FEDCBA9876543210FEDCBA9876543210", "7.2.fedcba9876543210fedcba987654321", "7.2.fedcba9876543210fedcba98765432100",
        "8.2.fedcba9876543210fedcba9876543210", "7.2", "7-2-fedcba9876543210fedcba9876543210"}) {
    auto bad  = claim_result();
    bad.token = std::string(token);
    CHECK_FALSE(encode_claim_result_v1(bad));
  }

  for (std::string_view fp : {"xxh3_128:00112233445566778899aabbccddeeF", "xxh3_128:0011223344556677",
                              "00112233445566778899aabbccddeeff", "xxh3_128:00112233445566778899aabbccddeeff0"}) {
    auto bad    = claim_result();
    bad.work_fp = std::string(fp);
    CHECK_FALSE(encode_claim_result_v1(bad));
  }

  for (std::string_view id : {"0", "01", "x", "+1"}) {
    auto a   = claim_result();
    a.run_id = std::string(id);
    CHECK_FALSE(encode_claim_result_v1(a));
    auto b        = claim_result();
    b.dispatch_id = std::string(id);
    CHECK_FALSE(encode_claim_result_v1(b));
    auto c       = claim_result();
    c.generation = std::string(id);
    CHECK_FALSE(encode_claim_result_v1(c));
    auto d     = claim_result();
    d.revision = std::string(id);
    CHECK_FALSE(encode_claim_result_v1(d));
  }

  // Activate result: job, previous job, role identifiers and sorted unique roles.
  REQUIRE(encode_activate_result_v1(activate_result("")));
  auto activate_job = activate_result("");
  activate_job.job  = job64;
  REQUIRE(encode_activate_result_v1(activate_job));
  activate_job.job = job65;
  CHECK_FALSE(encode_activate_result_v1(activate_job));
  auto previous_long = activate_result(job65);
  CHECK_FALSE(encode_activate_result_v1(previous_long));
  auto previous_ok = activate_result(job64);
  REQUIRE(encode_activate_result_v1(previous_ok));
  auto role_long          = activate_result("");
  role_long.roles[0].role = job65;
  CHECK_FALSE(encode_activate_result_v1(role_long));
  auto unsorted_roles = activate_result("");
  std::swap(unsorted_roles.roles[0], unsorted_roles.roles[1]);
  auto unsorted_refused = encode_activate_result_v1(unsorted_roles);
  REQUIRE_FALSE(unsorted_refused);
  CHECK(unsorted_refused.error().code == execution_codec_code::invalid);
  auto duplicate_roles          = activate_result("");
  duplicate_roles.roles[1].role = duplicate_roles.roles[0].role;
  CHECK_FALSE(encode_activate_result_v1(duplicate_roles));
  auto zero_revision     = activate_result("");
  zero_revision.revision = "0";
  CHECK_FALSE(encode_activate_result_v1(zero_revision));

  // Decoding refuses the same shapes, so stored bytes cannot carry them either.
  CHECK_FALSE(decode_claim_result_v1(json(claim_value([&] {
    auto r = claim_result();
    r.job  = job65;
    return r;
  }()))));
  CHECK_FALSE(decode_claim_result_v1(json(claim_value([&] {
    auto r   = claim_result();
    r.ledger = "0123456789ABCDEF0123456789ABCDEF";
    return r;
  }()))));
  CHECK_FALSE(decode_activate_result_v1(json(activate_value(unsorted_roles))));
  CHECK_FALSE(decode_activate_result_v1(json(activate_value(duplicate_roles))));
  CHECK_FALSE(decode_activate_result_v1(json(activate_value(previous_long))));
  CHECK(decode_claim_result_v1(json(claim_value(claim_result()))));
}

TEST_CASE("claim result v1 previous run and byte count rules", "[execution_codec]") {
  constexpr std::uint64_t int32_maximum = 2147483647;
  auto                    at            = claim_result();
  at.bytes                              = int32_maximum;
  CHECK(encode_claim_result_v1(at));
  auto over  = claim_result();
  over.bytes = int32_maximum + 1;
  CHECK_FALSE(encode_claim_result_v1(over));
  for (std::string previous : {"", "0", "01", "x", "-1", "1.5", "+1"}) {
    INFO(previous);
    CHECK_FALSE(encode_claim_result_v1(claim_result(previous)));
  }
  CHECK(encode_claim_result_v1(claim_result(std::string("9223372036854775807"))));
}

TEST_CASE("dispatch manifest decode bound at limit and limit plus one", "[execution_codec]") {
  constexpr std::size_t limit     = 16777216;
  const std::size_t     base_size = canonical(manifest(0)).size();
  const auto            at        = canonical(manifest(limit - base_size));
  REQUIRE(at.size() == limit);
  CHECK(decode_dispatch_manifest_v1(at));
  const auto over = canonical(manifest(limit - base_size + 1));
  REQUIRE(over.size() == limit + 1);
  auto refused = decode_dispatch_manifest_v1(over);
  REQUIRE_FALSE(refused);
  CHECK(refused.error().code == execution_codec_code::resource_limit);
  CHECK(refused.error().limit == "manifest_bytes");
  CHECK(refused.error().maximum == limit);
}
