module planar.engine.history.read;
import std;
import planar.engine.json_codec;
import planar.engine.history.recorded_interpretation;
import planar.engine.history.tree;
import planar.engine.history.execution_codec;
namespace planar::engine::history {
namespace {
using code = history_read_code;
value_ptr text(std::string s) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = std::move(s);
  return v;
}
value_ptr integer(std::int64_t n) {
  auto v     = std::make_shared<value>();
  v->kind    = value_kind::integer;
  v->integer = n;
  return v;
}
value_ptr boolean(bool b) {
  auto v     = std::make_shared<value>();
  v->kind    = value_kind::boolean;
  v->boolean = b;
  return v;
}
value_ptr object(std::map<std::string, value_ptr> f) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::record;
  v->fields = std::move(f);
  return v;
}
std::string id(std::optional<std::int64_t> n) {
  return n ? std::to_string(*n) : "none";
}
auto corrupt() {
  return std::unexpected(history_read_error{code::corrupt});
}
auto storage(const ledger::format_error& e) {
  return std::unexpected(history_read_error{code::storage, e});
}
std::expected<value_ptr, history_read_error> saved_json(std::string_view s) {
  auto decoded = codec_decode(s, {1048576, 8, 64});
  if (!decoded)
    return corrupt();
  return *decoded;
}
std::expected<value_ptr, history_read_error> revision_view(const ledger::format_file& file, std::uint64_t n) {
  auto row = file.read_revision(n);
  if (!row)
    return storage(row.error());
  if (!*row)
    return std::unexpected(history_read_error{code::revision_not_found});
  const auto& r = **row;
  auto        i = file.read_record(r.references.interpretation);
  if (!i)
    return storage(i.error());
  auto v = file.read_record(r.references.versions);
  if (!v)
    return storage(v.error());
  const auto* ib = std::get_if<value_record>(&i->value);
  const auto* vb = std::get_if<value_record>(&v->value);
  if (!ib || !vb)
    return corrupt();
  auto family = classify_recorded_interpretation(ib->encoded, vb->encoded);
  if (!family)
    return corrupt();
  auto root = file.read_record(r.references.stdlib);
  if (!root)
    return storage(root.error());
  auto files = read_tree(file, root->reference,
                         *family == recorded_family::effective ? tree_profile::effective_provenance : tree_profile::provenance);
  if (!files) {
    if (files.error().storage)
      return storage(*files.error().storage);
    return corrupt();
  }
  auto verified = verify_recorded_interpretation(ib->encoded, vb->encoded, *files, file.metadata().workspace_id);
  if (!verified)
    return corrupt();
  if (verified->status == recorded_status::unsupported_interpretation)
    return std::unexpected(history_read_error{code::unsupported_interpretation});
  if (!verified->decoded)
    return corrupt();
  const auto& decoded = *verified->decoded;
  return object({{"selector", text("revision")},
                 {"revision", revision_metadata(r)},
                 {"interpretation", decoded.interpretation},
                 {"versions", decoded.versions}});
}
std::optional<std::int64_t> decimal(std::string_view s) {
  std::int64_t n = 0;
  if (s.empty() || s.front() == '0')
    return std::nullopt;
  auto parsed = std::from_chars(s.data(), s.data() + s.size(), n);
  if (parsed.ec != std::errc{} || parsed.ptr != s.data() + s.size() || n <= 0)
    return std::nullopt;
  return n;
}
/// Format3 saved request result (P2-5): load keeps the existing result, activate is `activate_v1`; claim and takeover
/// assembly with the packet and manifest belongs to R15-D, so their saved value is returned as stored.
std::expected<value_ptr, history_read_error> format3_request_view(const ledger::format_file& file, std::string_view key) {
  auto row = file.read_execution_request(key);
  if (!row)
    return storage(row.error());
  if (!*row)
    return std::unexpected(history_read_error{code::request_not_found});
  const auto& r     = **row;
  bool        valid = false;
  if (r.operation == "load")
    valid = static_cast<bool>(ledger::decode_load_result(r.result));
  else if (r.operation == "activate")
    valid = static_cast<bool>(decode_activate_result_v1(r.result));
  else if (r.operation == "claim" || r.operation == "takeover")
    valid = static_cast<bool>(decode_claim_result_v1(r.result));
  if (!valid)
    return corrupt();
  auto result = saved_json(r.result);
  if (!result)
    return std::unexpected(result.error());
  return object({{"selector", text("request")}, {"result", *result}});
}
/// Format3 `--at-event` state (P2-5), replayed from rows with sequence at most the event, without Lua.
std::expected<value_ptr, history_read_error> format3_state_view(const ledger::format_file& file, const ledger::event_row& row) {
  std::optional<std::int64_t> active_revision;
  std::string                 active_job;
  if (row.type != "ledger_initialized") {
    auto decoded = decode_event_payload_v2(row.type, row.payload);
    if (!decoded)
      return corrupt();
    if (decoded->after.active_revision != "none") {
      active_revision = decimal(decoded->after.active_revision);
      if (!active_revision)
        return corrupt();
    }
    active_job = decoded->after.active_job;
  } else if (!ledger::decode_history_event(row.payload))
    return corrupt();
  auto event = event_summary(row, ledger::ledger_format::v3);
  if (!event)
    return std::unexpected(event.error());
  auto replayed = file.read_execution_state_at(static_cast<std::uint64_t>(row.sequence), active_job);
  if (!replayed)
    return storage(replayed.error());
  auto roles  = std::make_shared<value>();
  roles->kind = value_kind::list;
  for (const auto& role : replayed->roles) {
    value_ptr run = text("none");
    if (role.run)
      run = object({{"id", text(id(role.run->id))}, {"generation", text(id(role.run->generation))}});
    roles->items.push_back(object({{"role", text(role.role)},
                                   {"generation", text(id(role.generation))},
                                   {"state", text(role.state)},
                                   {"drifted", boolean(role.drifted)},
                                   {"run", run}}));
  }
  auto runs  = std::make_shared<value>();
  runs->kind = value_kind::list;
  for (const auto& run : replayed->runs)
    runs->items.push_back(object({{"id", text(id(run.id))}, {"role", text(run.role)}, {"generation", text(id(run.generation))}}));
  auto revision = text("none");
  if (active_revision) {
    auto r = file.read_revision(static_cast<std::uint64_t>(*active_revision));
    if (!r)
      return storage(r.error());
    if (!*r)
      return corrupt();
    revision = revision_metadata(**r);
  }
  return object({{"selector", text("state")},
                 {"event", *event},
                 {"state", object({{"active_revision", text(id(active_revision))},
                                   {"active_job", text(active_job)},
                                   {"activation", text(id(replayed->activation))},
                                   {"roles", roles},
                                   {"runs", runs}})},
                 {"revision", revision}});
}
} // namespace
value_ptr revision_metadata(const ledger::revision_row& r) {
  return object({{"id", text(id(r.id))},
                 {"parent", text(id(r.parent))},
                 {"source", text(id(r.references.source))},
                 {"registry", text(id(r.references.registry))},
                 {"stdlib", text(id(r.references.stdlib))},
                 {"interpretation", text(id(r.references.interpretation))},
                 {"versions", text(id(r.references.versions))},
                 {"actor", text(r.actor)},
                 {"accepted_at", text(r.accepted_at)}});
}
std::expected<value_ptr, history_read_error> event_summary(const ledger::event_row& r, ledger::ledger_format format) {
  const bool format3 = format == ledger::ledger_format::v3;
  // Format3 keeps payload version 1 only for ledger_initialized; every other format3 event carries payload version 2.
  if (!format3 || r.type == "ledger_initialized") {
    if (!ledger::decode_history_event(r.payload))
      return corrupt();
  } else if (!decode_event_payload_v2(r.type, r.payload))
    return corrupt();
  auto payload = saved_json(r.payload);
  if (!payload)
    return std::unexpected(payload.error());
  std::map<std::string, value_ptr> fields{{"sequence", text(id(r.sequence))},
                                          {"type", text(r.type)},
                                          {"version", integer(r.version)},
                                          {"revision", text(id(r.revision))},
                                          {"request_id", text(r.request_id.value_or("none"))},
                                          {"actor", text(r.actor)},
                                          {"occurred_at", text(r.occurred_at)},
                                          {"payload", *payload}};
  if (format3) {
    // P2-5: absent job and role are the empty string (`none` is a legal ID); absent database IDs are `none`.
    fields.emplace("job", text(r.job));
    fields.emplace("role", text(r.role));
    fields.emplace("generation_id", text(id(r.generation_id)));
    fields.emplace("run_id", text(id(r.run_id)));
  }
  return object(std::move(fields));
}
std::expected<value_ptr, history_read_error> read_history(const ledger::format_file& file, const history_query& q) {
  auto head = file.read_head();
  if (!head)
    return storage(head.error());
  const auto format  = file.metadata().format;
  const bool format3 = format == ledger::ledger_format::v3;
  value_ptr  data;
  switch (q.selector) {
  case history_selector::page: {
    auto page = file.read_events_after(q.id, q.limit);
    if (!page)
      return storage(page.error());
    auto events  = std::make_shared<value>();
    events->kind = value_kind::list;
    for (const auto& row : page->events) {
      auto event = event_summary(row, format);
      if (!event)
        return std::unexpected(event.error());
      events->items.push_back(*event);
    }
    std::map<std::string, value_ptr> fields{{"selector", text("page")}, {"events", events}};
    if (page->next_after)
      fields["next_after"] = text(id(page->next_after));
    data = object(std::move(fields));
    break;
  }
  case history_selector::revision: {
    auto view = revision_view(file, q.id);
    if (!view)
      return std::unexpected(view.error());
    data = *view;
    break;
  }
  case history_selector::request: {
    if (format3) {
      auto view = format3_request_view(file, q.request_id);
      if (!view)
        return std::unexpected(view.error());
      data = *view;
      break;
    }
    auto row = file.read_request(q.request_id);
    if (!row)
      return storage(row.error());
    if (!*row)
      return std::unexpected(history_read_error{code::request_not_found});
    if (!ledger::decode_load_result((**row).result))
      return corrupt();
    auto result = saved_json((**row).result);
    if (!result)
      return std::unexpected(result.error());
    data = object({{"selector", text("request")}, {"result", *result}});
    break;
  }
  case history_selector::state: {
    auto row = file.read_event(q.id);
    if (!row)
      return storage(row.error());
    if (!*row)
      return std::unexpected(history_read_error{code::event_not_found});
    if (format3) {
      auto view = format3_state_view(file, **row);
      if (!view)
        return std::unexpected(view.error());
      data = *view;
      break;
    }
    auto decoded = ledger::decode_history_event((**row).payload);
    if (!decoded)
      return corrupt();
    auto event = event_summary(**row);
    if (!event)
      return std::unexpected(event.error());
    auto revision = text("none");
    if (decoded->after) {
      auto r = file.read_revision(*decoded->after);
      if (!r)
        return storage(r.error());
      if (!*r)
        return corrupt();
      revision = revision_metadata(**r);
    }
    data = object({{"selector", text("state")},
                   {"event", *event},
                   {"state", object({{"active_revision", text(id(decoded->after))}})},
                   {"revision", revision}});
    break;
  }
  }
  return object({{"version", integer(1)},
                 {"kind", text("history")},
                 {"ledger", text(file.metadata().identity)},
                 {"active_revision", text(id(head->revision))},
                 {"last_event", text(id(head->last_sequence))},
                 {"data", data}});
}
} // namespace planar::engine::history
