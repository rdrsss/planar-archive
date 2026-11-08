module planar.engine.history.read;
import std;
import planar.engine.json_codec;
import planar.engine.history.recorded_interpretation;
import planar.engine.history.tree;
import planar.engine.history.execution_codec;
import planar.engine.canonical;
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
value_ptr array(std::vector<value_ptr> items = {}) {
  auto v   = std::make_shared<value>();
  v->kind  = value_kind::list;
  v->items = std::move(items);
  return v;
}
value_ptr field(const value_ptr& v, std::string_view key) {
  if (!v || v->kind != value_kind::record)
    return {};
  auto it = v->fields.find(std::string(key));
  return it == v->fields.end() ? value_ptr{} : it->second;
}
std::expected<value_ptr, history_read_error> execution_value(const ledger::format_file& file, std::int64_t id,
                                                             std::string_view codec) {
  auto rec = file.read_record(id);
  if (!rec)
    return storage(rec.error());
  auto* v = std::get_if<value_record>(&rec->value);
  if (!v || v->codec != codec)
    return corrupt();
  auto decoded = canonical_decode(v->encoded);
  if (!decoded)
    return corrupt();
  return *decoded;
}
std::expected<value_ptr, history_read_error> driver_value(const ledger::format_file& file, std::optional<std::int64_t> record) {
  auto      unknown  = object({{"status", text("unknown")}});
  value_ptr document = object({{"version", integer(1)},
                               {"driver", object({{"name", unknown}, {"version", unknown}})},
                               {"trial", object({{"id", unknown}})},
                               {"view", object({{"id", unknown}})},
                               {"model", object({{"requested", unknown}, {"actual", unknown}})},
                               {"session", object({{"requested", unknown}, {"actual", unknown}})},
                               {"settings", array()}});
  if (record) {
    auto value = execution_value(file, *record, "driver_provenance_v1");
    if (!value)
      return std::unexpected(value.error());
    document = field(*value, "document");
    if (!document)
      return corrupt();
  }
  return object(
      {{"id", text(id(record))}, {"authority", text(record ? "driver_assertion_unverified" : "none")}, {"document", document}});
}
std::expected<value_ptr, history_read_error> event_view(const ledger::format_file& file, const ledger::event_row& row) {
  auto summary = event_summary(row, file.metadata().format);
  if (!summary)
    return std::unexpected(summary.error());
  auto detail = file.read_execution_event(row.sequence);
  if (!detail)
    return storage(detail.error());
  std::vector<value_ptr> transitions;
  for (const auto& t : detail->transitions) {
    auto before =
        t.before ? object({{"state", text(*t.before)}, {"drifted", boolean(t.before_drifted.value_or(false))}}) : text("none");
    auto after = object({{"state", text(t.after)}, {"drifted", boolean(t.after_drifted)}});
    transitions.push_back(object({{"ordinal", integer(t.ordinal)},
                                  {"job", text(t.job)},
                                  {"role", text(t.role)},
                                  {"generation", text(id(t.generation))},
                                  {"before", before},
                                  {"after", after},
                                  {"reason", text(t.reason)}}));
  }
  std::map<std::string, value_ptr> result{
      {"selector", text("event")}, {"event", *summary}, {"transitions", array(std::move(transitions))}};
  if (file.metadata().format != ledger::ledger_format::v3 || row.type == "ledger_initialized")
    return object(std::move(result));
  auto payload = decode_event_payload_v2(row.type, row.payload);
  if (!payload)
    return corrupt();
  auto ref = [&](std::string_view key) -> std::optional<std::int64_t> {
    auto it = payload->references.find(std::string(key));
    return it == payload->references.end() ? std::nullopt : decimal(it->second);
  };
  if (row.type == "job_activated") {
    result["execution"] =
        object({{"activation",
                 object({{"id", text(id(ref("activation")))}, {"job", text(row.job)}, {"revision", text(id(row.revision))}})}});
    return object(std::move(result));
  }
  if (detail->supplemental && row.type != "verdict_recorded") {
    result["execution"] = detail->supplemental;
    return object(std::move(result));
  }
  if (!detail->run)
    return object(std::move(result));
  auto run = object(
      {{"id", text(id(detail->run))}, {"generation", text(id(detail->generation))}, {"request_id", text(detail->run_request)}});
  std::map<std::string, value_ptr> expansion{{"run", run}};
  value_ptr                        checkpoint = text("none");
  if (detail->checkpoint_body) {
    auto note = execution_value(file, *detail->checkpoint_body, "checkpoint_v1");
    if (!note)
      return std::unexpected(note.error());
    checkpoint = object({{"id", text(id(detail->checkpoint))}, {"value", *note}});
  }
  if (detail->dispatch) {
    auto packet = file.read_record(*detail->packet);
    if (!packet)
      return storage(packet.error());
    const auto* body = std::get_if<file_record>(&packet->value);
    if (!body)
      return corrupt();
    auto manifest = execution_value(file, *detail->manifest, "dispatch_manifest_v1"),
         work     = execution_value(file, *detail->work, "work_exec_v1");
    if (!manifest)
      return std::unexpected(manifest.error());
    if (!work)
      return std::unexpected(work.error());
    expansion["dispatch"] = object({{"id", text(id(detail->dispatch))},
                                    {"kind", text(detail->dispatch_kind)},
                                    {"work_fp", text("xxh3_128:" + detail->work_fp)},
                                    {"render_fp", text("xxh3_128:" + detail->render_fp)},
                                    {"revision", text(id(row.revision))}});
    expansion["packet"] =
        object({{"id", text(id(detail->packet))}, {"bytes", integer(body->payload.size())}, {"markdown", text(body->payload)}});
    expansion["manifest"] = object({{"id", text(id(detail->manifest))}, {"value", *manifest}});
    expansion["work"]     = object({{"id", text(id(detail->work))}, {"value", *work}});
    auto revision         = file.read_revision(*row.revision);
    if (!revision)
      return storage(revision.error());
    if (!*revision)
      return corrupt();
    auto source = file.read_record((**revision).references.source);
    if (!source)
      return storage(source.error());
    std::string source_digest = "xxh3_128:";
    for (auto b : source->reference.digest)
      source_digest += std::format("{:02x}", b);
    auto      unknown = object({{"status", text("unknown")}});
    value_ptr model = unknown, session = unknown;
    auto      interp = execution_value(file, (**revision).references.interpretation, "canonical_v1");
    if (!interp)
      return std::unexpected(interp.error());
    auto jobs = field(field(*interp, "bundle"), "jobs");
    if (jobs && jobs->kind == value_kind::list)
      for (const auto& job : jobs->items) {
        auto name = field(job, "id");
        if (name && name->kind == value_kind::string && name->string == row.job) {
          if (auto m = field(job, "model"))
            model = m;
          if (auto v = field(job, "session"))
            session = v;
        }
      }
    auto base  = field(*work, "base");
    auto label = base && base->kind == value_kind::string ? base : field(base, "label");
    if (!label)
      label = text("");
    auto native = object({{"ledger", text(file.metadata().identity)},
                          {"revision", text(id(row.revision))},
                          {"source", object({{"id", text(id((**revision).references.source))}, {"digest", text(source_digest)}})},
                          {"job", text(row.job)},
                          {"role", text(row.role)},
                          {"generation", text(id(detail->generation))},
                          {"base_label", label},
                          {"work_fp", text("xxh3_128:" + detail->work_fp)},
                          {"render_fp", text("xxh3_128:" + detail->render_fp)},
                          {"preferences", object({{"model", model}, {"session", session}})}});
    auto driver = driver_value(file, detail->provenance);
    if (!driver)
      return std::unexpected(driver.error());
    expansion["provenance"] = object({{"native", native}, {"driver", *driver}});
    if (row.type == "run_taken_over")
      expansion["previous_run"] = text(id(ref("previous_run")));
    if (row.type == "run_refreshed")
      expansion["checkpoint"] = checkpoint;
  } else if (row.type == "checkpoint_recorded")
    expansion["checkpoint"] = checkpoint;
  else if (row.type == "run_released" || row.type == "run_interrupted") {
    expansion["run"]        = object({{"id", text(id(detail->run))},
                                      {"generation", text(id(detail->generation))},
                                      {"request_id", text(detail->run_request)},
                                      {"state", text(row.type == "run_released" ? "released" : "interrupted")}});
    expansion["checkpoint"] = checkpoint;
    auto driver             = driver_value(file, ref("provenance"));
    if (!driver)
      return std::unexpected(driver.error());
    expansion["provenance"] = object({{"driver", *driver}});
  } else
    return object(std::move(result));
  if (row.type == "verdict_recorded" && detail->supplemental) {
    auto provenance = expansion.find("provenance");
    if (provenance == expansion.end())
      return corrupt();
    auto native = field(provenance->second, "native");
    if (!native)
      return corrupt();
    expansion               = detail->supplemental->fields;
    expansion["provenance"] = object({{"native", native}});
  }
  result["execution"] = object(std::move(expansion));
  return object(std::move(result));
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
  else if (r.operation == "refresh")
    valid = static_cast<bool>(decode_refresh_result_v1(r.result));
  else if (r.operation == "checkpoint")
    valid = static_cast<bool>(decode_checkpoint_result_v1(r.result));
  else if (r.operation == "release")
    valid = static_cast<bool>(decode_release_result_v1(r.result));
  else if (r.operation == "verdict")
    valid = static_cast<bool>(decode_verdict_result_v1(r.result));
  else if (r.operation == "check" || r.operation == "check-recover") {
    auto saved = saved_json(r.result);
    auto kind  = saved ? field(*saved, "kind") : value_ptr{};
    valid      = kind && kind->kind == value_kind::string && kind->string == "check";
  }
  if (!valid)
    return corrupt();
  auto result = saved_json(r.result);
  if (!result)
    return std::unexpected(result.error());
  if (r.operation == "claim" || r.operation == "takeover" || r.operation == "refresh") {
    auto event = file.read_event(r.event_sequence);
    if (!event)
      return storage(event.error());
    if (!*event)
      return corrupt();
    auto detail = file.read_execution_event(r.event_sequence);
    if (!detail)
      return storage(detail.error());
    if (!detail->packet || !detail->manifest || !detail->work || !detail->structure)
      return corrupt();
    auto record = file.read_record(*detail->packet);
    if (!record)
      return storage(record.error());
    auto* body = std::get_if<file_record>(&record->value);
    if (!body)
      return corrupt();
    auto manifest  = execution_value(file, *detail->manifest, "dispatch_manifest_v1"),
         work      = execution_value(file, *detail->work, "work_exec_v1"),
         structure = execution_value(file, *detail->structure, "role_structure_v1");
    if (!manifest)
      return std::unexpected(manifest.error());
    if (!work)
      return std::unexpected(work.error());
    if (!structure)
      return std::unexpected(structure.error());
    auto workspace = field(*work, "workspace"), spec = field(*work, "spec"), kind = field(*structure, "kind"),
         units = field(*structure, "work");
    if (kind && kind->kind == value_kind::string && kind->string == "reviewer")
      units = field(*structure, "targets");
    if (!workspace || !spec || !kind || !units)
      return corrupt();
    if (workspace->kind == value_kind::record)
      workspace = field(workspace, "id");
    if (spec->kind == value_kind::record)
      spec = field(spec, "id");
    if (!workspace || !spec)
      return corrupt();
    auto packet        = object({{"version", integer(1)},
                                 {"mode", text("dispatch")},
                                 {"workspace", workspace},
                                 {"spec", spec},
                                 {"job", field(*work, "job")},
                                 {"role", field(*work, "role")},
                                 {"kind", kind},
                                 {"work", units},
                                 {"markdown", text(body->payload)}});
    auto fields        = (*result)->fields;
    fields["packet"]   = packet;
    fields["manifest"] = *manifest;
    result             = object(std::move(fields));
  }
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
  case history_selector::event: {
    auto row = file.read_event(q.id);
    if (!row)
      return storage(row.error());
    if (!*row)
      return std::unexpected(history_read_error{code::event_not_found});
    auto view = event_view(file, **row);
    if (!view)
      return std::unexpected(view.error());
    data = *view;
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
