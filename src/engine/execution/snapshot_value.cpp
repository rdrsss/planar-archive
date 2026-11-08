// read_execution service value: execution_snapshot to {version:1,ledger,last_seq,snapshot} (execution-contract P2-1).
module planar.engine.execution.snapshot_value;

import std;
import planar.engine.canonical;

namespace planar::engine::execution {

namespace {

value_ptr node(value&& item) {
  return std::make_shared<const value>(std::move(item));
}

value_ptr text(std::string_view content) {
  value item{};
  item.kind   = value_kind::string;
  item.string = std::string(content);
  return node(std::move(item));
}

value_ptr decimal(std::uint64_t number) {
  return text(std::to_string(number));
}

value_ptr flag(bool on) {
  value item{};
  item.kind    = value_kind::boolean;
  item.boolean = on;
  return node(std::move(item));
}

value_ptr integer(std::int64_t number) {
  value item{};
  item.kind    = value_kind::integer;
  item.integer = number;
  return node(std::move(item));
}

value_ptr list(std::vector<value_ptr> items) {
  value item{};
  item.kind  = value_kind::list;
  item.items = std::move(items);
  return node(std::move(item));
}

value_ptr record(std::map<std::string, value_ptr> fields) {
  value item{};
  item.kind   = value_kind::record;
  item.fields = std::move(fields);
  return node(std::move(item));
}

std::unexpected<snapshot_value_error> fail(std::string field) {
  return std::unexpected(snapshot_value_error{std::move(field)});
}

std::expected<value_ptr, snapshot_value_error> revision_value(const ledger::execution_revision& revision) {
  if (!revision.interpretation_value || !revision.versions_value || !revision.registry_value)
    return fail("snapshot.revision");
  // Draft decision DD17: source is {id,digest} like the registry identity, so a handler can name the source record.
  return record({{"id", decimal(revision.id)},
                 {"interpretation", revision.interpretation_value},
                 {"versions", revision.versions_value},
                 {"stdlib_digest", text(revision.stdlib_digest)},
                 {"registry", revision.registry_value},
                 {"source", record({{"id", decimal(revision.source)}, {"digest", text(revision.source_digest)}})}});
}

std::expected<value_ptr, snapshot_value_error> role_value(const ledger::execution_role& row, std::size_t index,
                                                          std::optional<std::uint64_t> active_revision) {
  const std::string prefix    = std::format("snapshot.roles.{}", index);
  auto              structure = role_structure_value(row.structure);
  if (!structure)
    return fail(prefix + ".generation.structure");
  value_ptr run = text("none");
  if (row.run_id) {
    if (!row.dispatch || !row.dispatch->work)
      return fail(prefix + ".run.dispatch");
    // L18 / A1: the dispatch names its own recorded revision; a zero (unknown) revision falls back to the active one.
    const std::uint64_t revision = row.dispatch->revision != 0 ? row.dispatch->revision : active_revision.value_or(0);
    run                          = record({{"id", decimal(*row.run_id)},
                                           {"generation", decimal(row.generation_ordinal)},
                                           {"dispatch", record({{"id", decimal(row.dispatch->id)},
                                                                {"work_fp", text(row.dispatch->work_fp)},
                                                                {"render_fp", text(row.dispatch->render_fp)},
                                                                {"revision", decimal(revision)},
                                                                {"work", row.dispatch->work}})}});
  }
  return record(
      {{"job", text(row.job)},
       {"role", text(row.role)},
       {"kind", text(row.structure.kind)},
       {"generation",
        record({{"id", decimal(row.generation_id)}, {"ordinal", decimal(row.generation_ordinal)}, {"structure", *structure}})},
       {"state", text(row.state)},
       {"drifted", flag(row.drifted)},
       {"fact_version", decimal(row.fact_version)},
       {"run", run}});
}

} // namespace

std::expected<value_ptr, snapshot_value_error> role_structure_value(const history::role_structure_v1& structure) {
  auto bytes = history::encode_role_structure_v1(structure);
  if (!bytes)
    return fail("structure");
  auto decoded = canonical_decode(*bytes);
  if (!decoded)
    return fail("structure");
  return *decoded;
}

std::expected<value_ptr, snapshot_value_error> snapshot_value(const ledger::execution_snapshot& snapshot) {
  value_ptr                    revision = text("none");
  std::optional<std::uint64_t> active;
  if (snapshot.revision) {
    auto built = revision_value(*snapshot.revision);
    if (!built)
      return std::unexpected(built.error());
    revision = *built;
    active   = snapshot.revision->id;
  }
  std::vector<value_ptr> roles;
  roles.reserve(snapshot.roles.size());
  for (std::size_t i = 0; i < snapshot.roles.size(); ++i) {
    auto row = role_value(snapshot.roles[i], i, active);
    if (!row)
      return std::unexpected(row.error());
    roles.push_back(*row);
  }
  std::vector<value_ptr> completions, evidence, observed, publications, reservations, verdicts, blockers, repairs;
  for (const auto& c : snapshot.completions) {
    std::vector<value_ptr> units;
    for (const auto& u : c.units)
      units.push_back(record({{"work", text(u.work)}, {"output", text(u.output)}}));
    completions.push_back(record({{"id", decimal(c.id)},
                                  {"job", text(c.job)},
                                  {"role", text(c.role)},
                                  {"generation", decimal(c.generation)},
                                  {"run", decimal(c.run)},
                                  {"dispatch", decimal(c.dispatch)},
                                  {"work_fp", text("xxh3_128:" + c.work_fp)},
                                  {"seq", decimal(c.seq)},
                                  {"units", list(std::move(units))},
                                  {"vectors", list(c.vectors)}}));
  }
  for (const auto& e : snapshot.evidence)
    evidence.push_back(record({{"id", decimal(e.id)},
                               {"reservation", decimal(e.reservation)},
                               {"work", text(e.work)},
                               {"check", text(e.check)},
                               {"outcome", text(e.outcome)},
                               {"reason", text(e.reason)},
                               {"matched", e.matched ? decimal(*e.matched) : text("none")},
                               {"seq", decimal(e.seq)},
                               {"vector", e.vector}}));
  for (const auto& o : snapshot.observed)
    observed.push_back(record({{"work", text(o.work)}, {"output", text(o.output)}, {"seq", decimal(o.seq)}}));
  for (const auto& p : snapshot.publications)
    publications.push_back(record({{"publication", decimal(p.publication)},
                                   {"completion", decimal(p.completion)},
                                   {"job", text(p.job)},
                                   {"role", text(p.role)},
                                   {"work", text(p.work)},
                                   {"facet", text(p.facet)},
                                   {"semantic", text(p.semantic)}}));
  for (const auto& r : snapshot.reservations)
    reservations.push_back(record({{"id", decimal(r.id)},
                                   {"run", decimal(r.run)},
                                   {"request_id", text(r.request_id)},
                                   {"opened_seq", decimal(r.opened_seq)}}));
  for (const auto& v : snapshot.review.verdicts) {
    auto body = history::verdict_value(v.body);
    if (!body)
      return fail("snapshot.review.verdicts");
    verdicts.push_back(record({{"id", decimal(v.id)},
                               {"request_id", text(v.request_id)},
                               {"run", decimal(v.run)},
                               {"seq", decimal(v.seq)},
                               {"applicability", text(v.applicability)},
                               {"decision", text(v.decision)},
                               {"body", *body}}));
  }
  for (const auto& b : snapshot.review.blockers)
    blockers.push_back(record({{"id", decimal(b.id)},
                               {"seq", decimal(b.seq)},
                               {"job", text(b.job)},
                               {"kind", text(b.kind)},
                               {"verdict", decimal(b.verdict)}}));
  for (const auto& r : snapshot.review.repairs)
    repairs.push_back(record({{"job", text(r.job)}, {"role", text(r.role)}, {"revision", decimal(r.revision)}}));
  auto inner = record({{"active_revision", active ? decimal(*active) : text("none")},
                       {"active_job", text(snapshot.active_job)},
                       {"activation", snapshot.activation ? decimal(*snapshot.activation) : text("none")},
                       {"revision", revision},
                       {"roles", list(std::move(roles))},
                       {"publications", list(std::move(publications))},
                       {"completions", list(std::move(completions))},
                       {"evidence", list(std::move(evidence))},
                       {"observed", list(std::move(observed))},
                       {"reservations", list(std::move(reservations))},
                       {"review", record({{"count", decimal(snapshot.review.count)},
                                          {"verdicts", list(std::move(verdicts))},
                                          {"blockers", list(std::move(blockers))},
                                          {"repairs", list(std::move(repairs))}})}});
  return record({{"version", integer(1)},
                 {"ledger", text(snapshot.identity)},
                 {"last_seq", decimal(snapshot.last_seq)},
                 {"snapshot", inner}});
}

} // namespace planar::engine::execution
