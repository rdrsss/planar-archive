module planar.engine.history.read;
import std;
import planar.engine.json_codec;
import planar.engine.history.recorded_interpretation;
import planar.engine.history.tree;
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
std::expected<value_ptr, history_read_error> event_summary(const ledger::event_row& r) {
  if (!ledger::decode_history_event(r.payload))
    return corrupt();
  auto payload = saved_json(r.payload);
  if (!payload)
    return std::unexpected(payload.error());
  return object({{"sequence", text(id(r.sequence))},
                 {"type", text(r.type)},
                 {"version", integer(r.version)},
                 {"revision", text(id(r.revision))},
                 {"request_id", text(r.request_id.value_or("none"))},
                 {"actor", text(r.actor)},
                 {"occurred_at", text(r.occurred_at)},
                 {"payload", *payload}});
}
std::expected<value_ptr, history_read_error> read_history(const ledger::format_file& file, const history_query& q) {
  auto head = file.read_head();
  if (!head)
    return storage(head.error());
  value_ptr data;
  switch (q.selector) {
  case history_selector::page: {
    auto page = file.read_events_after(q.id, q.limit);
    if (!page)
      return storage(page.error());
    auto events  = std::make_shared<value>();
    events->kind = value_kind::list;
    for (const auto& row : page->events) {
      auto event = event_summary(row);
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
