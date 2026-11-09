// R18 ledger read side of planar.engine.ledger.execution (DB-R3): the derived review values (repair revision, verdict
// activation, review count, open blockers, structural lineage), the cross-row invariants 21 to 27 of R18-T, the
// R17 completion/event/vector consistency checks, and the snapshot review value of DR18-8. Every function reads through the
// caller's open connection inside its transaction; every violated invariant is ledger_corrupt.
module;
#include <sqlite3.h>
module planar.engine.ledger.execution;

import std;
import planar.engine.canonical;
import planar.engine.value;
import planar.engine.history.record;
import planar.engine.history.request_v3;
import planar.engine.history.execution_codec;
import planar.engine.ledger.sqlite;
import planar.engine.ledger.execution_schema;

namespace planar::engine::ledger {

namespace {

namespace hist = ::planar::engine::history;
using ::planar::engine::value;
using ::planar::engine::value_kind;
using ::planar::engine::value_ptr;

template <typename T> using result = std::expected<T, execution_error>;
using checked                      = std::expected<void, execution_error>;

constexpr std::uint64_t default_max_rounds = 2; // declarations.md:275

execution_error failure(execution_error_code code) {
  execution_error error{};
  error.code = code;
  return error;
}

execution_error corrupt() {
  return failure(execution_error_code::ledger_corrupt);
}

execution_error map_sqlite(const sqlite_error& error) {
  if (error.code == SQLITE_BUSY || error.code == SQLITE_LOCKED)
    return failure(execution_error_code::ledger_busy);
  if (error.code == SQLITE_FULL)
    return failure(execution_error_code::storage_full);
  if (error.code == SQLITE_CORRUPT || error.code == SQLITE_NOTADB)
    return failure(execution_error_code::ledger_corrupt);
  return failure(execution_error_code::io);
}

// ---------------------------------------------------------------------------------------------------------------------
// Query helpers: bound parameters, then one visitor call per row with typed column readers.

struct blob_value {
  std::string bytes;
};
using bind_value = std::variant<std::int64_t, std::string, blob_value>;

template <typename Visit>
checked each_row(connection& db, std::string_view sql, const std::vector<bind_value>& binds, Visit&& visit) {
  auto query = db.prepare(sql);
  if (!query)
    return std::unexpected(map_sqlite(query.error()));
  for (std::size_t i = 0; i < binds.size(); ++i) {
    const int                         index = static_cast<int>(i + 1);
    std::expected<void, sqlite_error> bound{};
    if (const auto* number = std::get_if<std::int64_t>(&binds[i]))
      bound = query->bind_integer(index, *number);
    else if (const auto* text = std::get_if<std::string>(&binds[i]))
      bound = query->bind_text(index, *text);
    else
      bound = query->bind_blob(index, std::get<blob_value>(binds[i]).bytes);
    if (!bound)
      return std::unexpected(map_sqlite(bound.error()));
  }
  while (true) {
    auto step = query->step();
    if (!step)
      return std::unexpected(map_sqlite(step.error()));
    if (*step == step_result::done)
      return {};
    if (auto visited = visit(*query); !visited)
      return std::unexpected(visited.error());
  }
}

result<std::optional<std::int64_t>> opt_int(statement& row, int column) {
  auto value = row.integer(column);
  if (!value)
    return std::unexpected(corrupt());
  return *value;
}

result<std::uint64_t> id_col(statement& row, int column) {
  auto value = opt_int(row, column);
  if (!value)
    return std::unexpected(value.error());
  if (!*value || **value <= 0)
    return std::unexpected(corrupt());
  return static_cast<std::uint64_t>(**value);
}

result<std::optional<std::uint64_t>> opt_id_col(statement& row, int column) {
  auto value = opt_int(row, column);
  if (!value)
    return std::unexpected(value.error());
  if (!*value)
    return std::optional<std::uint64_t>{};
  if (**value <= 0)
    return std::unexpected(corrupt());
  return std::optional<std::uint64_t>{static_cast<std::uint64_t>(**value)};
}

result<std::string> text_col(statement& row, int column) {
  auto value = row.text(column);
  if (!value || !*value)
    return std::unexpected(corrupt());
  return **value;
}

result<std::optional<std::string>> opt_text_col(statement& row, int column) {
  auto value = row.text(column);
  if (!value)
    return std::unexpected(corrupt());
  return *value;
}

result<std::string> blob_col(statement& row, int column) {
  auto value = row.blob(column);
  if (!value || !*value)
    return std::unexpected(corrupt());
  return **value;
}

std::int64_t as_bind(std::uint64_t number) {
  return static_cast<std::int64_t>(number);
}

std::string hex_of(std::string_view bytes) {
  constexpr std::string_view digits = "0123456789abcdef";
  std::string                out;
  out.reserve(bytes.size() * 2);
  for (unsigned char byte : bytes) {
    out += digits[byte >> 4];
    out += digits[byte & 15];
  }
  return out;
}

std::optional<std::uint64_t> parse_decimal(std::string_view text) {
  if (text.empty() || text.size() > 20 || (text.size() > 1 && text.front() == '0'))
    return std::nullopt;
  std::uint64_t number = 0;
  for (char c : text) {
    if (c < '0' || c > '9')
      return std::nullopt;
    const auto digit = static_cast<std::uint64_t>(c - '0');
    if (number > (std::numeric_limits<std::uint64_t>::max() - digit) / 10)
      return std::nullopt;
    number = number * 10 + digit;
  }
  return number;
}

// ---------------------------------------------------------------------------------------------------------------------
// Record access through SQL: a value record's codec identifier and canonical bytes.

struct value_bytes {
  std::string codec;
  std::string canonical;
};

result<value_bytes> value_record(connection& db, std::uint64_t id) {
  std::optional<value_bytes> found;
  auto scan = each_row(db, "SELECT kind,codec,encoded FROM records WHERE id=?1", {as_bind(id)}, [&](statement& row) -> checked {
    auto kind    = opt_int(row, 0);
    auto codec   = opt_int(row, 1);
    auto encoded = blob_col(row, 2);
    if (!kind || !codec || !encoded || *kind != 3 || *codec != 1)
      return std::unexpected(corrupt());
    if (auto exec = hist::decode_execution_value_record(*encoded); exec) {
      found = value_bytes{exec->codec, exec->encoded};
      return {};
    }
    auto decoded = hist::decode_record(*encoded);
    if (!decoded)
      return std::unexpected(corrupt());
    const auto* plain = std::get_if<hist::value_record>(&*decoded);
    if (!plain || plain->codec != "canonical_v1")
      return std::unexpected(corrupt());
    found = value_bytes{"canonical_v1", plain->encoded};
    return {};
  });
  if (!scan)
    return std::unexpected(scan.error());
  if (!found)
    return std::unexpected(corrupt());
  return *found;
}

// ---------------------------------------------------------------------------------------------------------------------
// Row images of the R18 tables and the rows they reference.

struct generation_image {
  std::uint64_t id = 0;
  std::string   job;
  std::string   role;
  std::uint64_t ordinal    = 0;
  std::uint64_t structure  = 0;
  std::uint64_t opened_seq = 0;
  std::string   reason;
};

struct verdict_image {
  std::uint64_t    id = 0;
  std::string      request_id;
  std::uint64_t    run     = 0;
  std::uint64_t    seq     = 0;
  std::uint64_t    body_id = 0;
  std::string      applicability;
  std::string      decision;
  hist::verdict_v1 body;
  std::string      job;  ///< events.job of the verdict event.
  std::string      role; ///< events.role of the verdict event (the reviewer).
  std::uint64_t    generation = 0;
  std::string      saved; ///< Saved verdict_response_v1 bytes.
};

struct repair_image {
  std::uint64_t verdict = 0;
  std::string   job;
  std::string   role;
  std::uint64_t generation = 0;
  std::uint64_t revision   = 0;
};

struct blocker_image {
  std::uint64_t                id  = 0;
  std::uint64_t                seq = 0;
  std::string                  job;
  std::string                  kind;
  std::uint64_t                verdict = 0;
  std::string                  state; ///< From blocker_state.
  std::optional<std::uint64_t> cleared_seq;
  std::uint64_t                last_seq = 0;
};

struct activation_image {
  std::uint64_t id = 0;
  std::string   job;
  std::uint64_t event_seq = 0;
};

result<std::map<std::uint64_t, generation_image>> load_generations(connection& db) {
  std::map<std::uint64_t, generation_image> out;
  auto scan = each_row(db, "SELECT id,job,role,ordinal,structure_id,opened_seq,reason FROM generations ORDER BY id", {},
                       [&](statement& row) -> checked {
                         generation_image g;
                         auto id = id_col(row, 0), ordinal = id_col(row, 3), structure = id_col(row, 4), opened = id_col(row, 5);
                         auto job = text_col(row, 1), role = text_col(row, 2), reason = text_col(row, 6);
                         if (!id || !ordinal || !structure || !opened || !job || !role || !reason)
                           return std::unexpected(corrupt());
                         g = generation_image{*id, *job, *role, *ordinal, *structure, *opened, *reason};
                         out.emplace(g.id, std::move(g));
                         return {};
                       });
  if (!scan)
    return std::unexpected(scan.error());
  return out;
}

// The lineage of one generation over a loaded generation map (EX-5 Lineage): walk back over takeover generations.
result<std::vector<std::uint64_t>> lineage_of(const std::map<std::uint64_t, generation_image>& generations, std::uint64_t id) {
  const auto found = generations.find(id);
  if (found == generations.end())
    return std::unexpected(corrupt());
  std::vector<const generation_image*> chain;
  for (const auto& [gid, g] : generations) {
    (void)gid;
    if (g.job == found->second.job && g.role == found->second.role)
      chain.push_back(&g);
  }
  std::ranges::sort(chain, [](const generation_image* a, const generation_image* b) { return a->ordinal < b->ordinal; });
  auto position = std::ranges::find_if(chain, [&](const generation_image* g) { return g->id == id; });
  if (position == chain.end())
    return std::unexpected(corrupt());
  auto start = position;
  while ((*start)->reason == "takeover") {
    if (start == chain.begin())
      return std::unexpected(corrupt());
    --start;
  }
  std::vector<std::uint64_t> out;
  for (auto it = start; it != position + 1; ++it)
    out.push_back((*it)->id);
  return out;
}

result<std::vector<activation_image>> load_activations(connection& db) {
  std::vector<activation_image> out;
  auto scan = each_row(db, "SELECT id,job,event_seq FROM job_activations ORDER BY id", {}, [&](statement& row) -> checked {
    auto id = id_col(row, 0), seq = id_col(row, 2);
    auto job = text_col(row, 1);
    if (!id || !seq || !job)
      return std::unexpected(corrupt());
    out.push_back(activation_image{*id, *job, *seq});
    return {};
  });
  if (!scan)
    return std::unexpected(scan.error());
  return out;
}

std::optional<std::uint64_t> activation_at(const std::vector<activation_image>& activations, std::string_view job,
                                           std::uint64_t seq) {
  std::optional<std::uint64_t> out;
  std::uint64_t                best = 0;
  for (const auto& a : activations)
    if (a.job == job && a.event_seq < seq && a.event_seq >= best) {
      best = a.event_seq;
      out  = a.id;
    }
  return out;
}

result<std::vector<verdict_image>> load_verdicts(connection& db) {
  std::vector<verdict_image> out;
  auto                       scan =
      each_row(db,
               "SELECT v.id,v.request_id,v.run_id,v.seq,v.body_id,v.applicability,v.decision,e.job,e.role,e.generation_id "
               "FROM verdicts v LEFT JOIN events e ON e.seq=v.seq ORDER BY v.id",
               {}, [&](statement& row) -> checked {
                 verdict_image v;
                 auto          id = id_col(row, 0), run = id_col(row, 2), seq = id_col(row, 3), body = id_col(row, 4);
                 auto          generation = opt_id_col(row, 9);
                 auto          request = text_col(row, 1), applicability = text_col(row, 5), decision = text_col(row, 6);
                 auto          job = opt_text_col(row, 7), role = opt_text_col(row, 8);
                 if (!id || !run || !seq || !body || !generation || !request || !applicability || !decision || !job || !role)
                   return std::unexpected(corrupt());
                 if (!*job || !*role || !*generation)
                   return std::unexpected(corrupt());
                 v.id            = *id;
                 v.request_id    = *request;
                 v.run           = *run;
                 v.seq           = *seq;
                 v.body_id       = *body;
                 v.applicability = *applicability;
                 v.decision      = *decision;
                 v.job           = **job;
                 v.role          = **role;
                 v.generation    = **generation;
                 out.push_back(std::move(v));
                 return {};
               });
  if (!scan)
    return std::unexpected(scan.error());
  for (auto& v : out) {
    auto record = value_record(db, v.body_id);
    if (!record)
      return std::unexpected(record.error());
    if (record->codec != hist::codec_verdict_v1)
      return std::unexpected(corrupt());
    auto body = hist::decode_verdict_v1(record->canonical);
    if (!body || body->decision != v.decision)
      return std::unexpected(corrupt());
    v.body = std::move(*body);
  }
  return out;
}

result<std::vector<repair_image>> load_repairs(connection& db) {
  std::vector<repair_image> out;
  auto scan = each_row(db, "SELECT verdict_id,job,role,generation_id,revision FROM repairs ORDER BY verdict_id,job,role", {},
                       [&](statement& row) -> checked {
                         auto verdict = id_col(row, 0), generation = id_col(row, 3), revision = id_col(row, 4);
                         auto job = text_col(row, 1), role = text_col(row, 2);
                         if (!verdict || !generation || !revision || !job || !role)
                           return std::unexpected(corrupt());
                         out.push_back(repair_image{*verdict, *job, *role, *generation, *revision});
                         return {};
                       });
  if (!scan)
    return std::unexpected(scan.error());
  return out;
}

// Blockers joined with their state rows; the bijection between the two tables is checked here (invariant 26).
result<std::vector<blocker_image>> load_blockers(connection& db) {
  std::vector<blocker_image> out;
  auto scan = each_row(db, "SELECT id,seq,job,kind,verdict_id FROM blockers ORDER BY id", {}, [&](statement& row) -> checked {
    auto id = id_col(row, 0), seq = id_col(row, 1), verdict = id_col(row, 4);
    auto job = text_col(row, 2), kind = text_col(row, 3);
    if (!id || !seq || !verdict || !job || !kind)
      return std::unexpected(corrupt());
    out.push_back(blocker_image{*id, *seq, *job, *kind, *verdict, {}, std::nullopt, 0});
    return {};
  });
  if (!scan)
    return std::unexpected(scan.error());
  std::size_t states = 0;
  scan               = each_row(db, "SELECT blocker_id,state,cleared_seq,last_seq FROM blocker_state ORDER BY blocker_id", {},
                                [&](statement& row) -> checked {
                    auto id = id_col(row, 0), last = id_col(row, 3);
                    auto state   = text_col(row, 1);
                    auto cleared = opt_id_col(row, 2);
                    if (!id || !last || !state || !cleared)
                      return std::unexpected(corrupt());
                    auto found = std::ranges::find_if(out, [&](const blocker_image& b) { return b.id == *id; });
                    if (found == out.end())
                      return std::unexpected(corrupt());
                    found->state       = *state;
                    found->cleared_seq = *cleared;
                    found->last_seq    = *last;
                    ++states;
                    return {};
                                });
  if (!scan)
    return std::unexpected(scan.error());
  if (states != out.size())
    return std::unexpected(corrupt());
  return out;
}

// shape.max_rounds of one W value (DB-R17): a W shape that is not a record, or has no max_rounds, has the default 2.
std::optional<std::uint64_t> max_rounds_of_work(const value_ptr& w) {
  if (!w || w->kind != value_kind::record)
    return std::nullopt;
  const auto shape = w->fields.find("shape");
  if (shape == w->fields.end() || !shape->second || shape->second->kind != value_kind::record)
    return default_max_rounds;
  const auto cap = shape->second->fields.find("max_rounds");
  if (cap == shape->second->fields.end() || !cap->second)
    return default_max_rounds;
  if (cap->second->kind == value_kind::integer && cap->second->integer >= 0)
    return static_cast<std::uint64_t>(cap->second->integer);
  if (cap->second->kind == value_kind::string)
    if (auto parsed = parse_decimal(cap->second->string))
      return *parsed;
  return std::nullopt;
}

// max_rounds of a verdict (DB-R6): shape.max_rounds of the reviewer run's acknowledged W at the verdict, default 2.
result<std::uint64_t> max_rounds_at(connection& db, std::uint64_t run, std::uint64_t seq) {
  std::optional<std::uint64_t> work;
  auto scan = each_row(db, "SELECT work_id FROM dispatches WHERE run_id=?1 AND seq<?2 ORDER BY seq DESC, id DESC LIMIT 1",
                       {as_bind(run), as_bind(seq)}, [&](statement& row) -> checked {
                         auto id = id_col(row, 0);
                         if (!id)
                           return std::unexpected(id.error());
                         work = *id;
                         return {};
                       });
  if (!scan)
    return std::unexpected(scan.error());
  if (!work)
    return std::unexpected(corrupt());
  auto record = value_record(db, *work);
  if (!record)
    return std::unexpected(record.error());
  if (record->codec != hist::codec_work_exec_v1)
    return std::unexpected(corrupt());
  auto w = hist::decode_work_exec_v1(record->canonical);
  if (!w)
    return std::unexpected(corrupt());
  auto cap = max_rounds_of_work(*w);
  if (!cap)
    return std::unexpected(corrupt());
  return *cap;
}

result<std::string> structure_kind(connection& db, std::uint64_t structure) {
  auto record = value_record(db, structure);
  if (!record)
    return std::unexpected(record.error());
  if (record->codec != hist::codec_role_structure_v1)
    return std::unexpected(corrupt());
  auto decoded = hist::decode_role_structure_v1(record->canonical);
  if (!decoded)
    return std::unexpected(corrupt());
  return decoded->kind;
}

// ---------------------------------------------------------------------------------------------------------------------
// Invariants 21 to 27.

std::string decimal(std::uint64_t number) {
  return std::to_string(number);
}

// 21 and 22: the verdict, its request, its event, its saved result and its completed reviewer run.
checked check_verdict_rows(connection& db, const std::vector<verdict_image>& verdicts,
                           const std::map<std::uint64_t, generation_image>& generations, std::vector<verdict_image>& out) {
  std::set<std::uint64_t> runs;
  std::uint64_t           previous_seq = 0;
  for (const auto& v : verdicts) {
    if (v.seq <= previous_seq || !runs.insert(v.run).second)
      return std::unexpected(corrupt());
    previous_seq = v.seq;
    if (v.applicability != "current" && v.applicability != "historical")
      return std::unexpected(corrupt());
    // Event at seq: type verdict_recorded, the verdict's request and run, payload references body, run and verdict.
    bool event_ok = false;
    auto scan     = each_row(
        db, "SELECT type,request_id,run_id,payload FROM events WHERE seq=?1", {as_bind(v.seq)}, [&](statement& row) -> checked {
          auto type = text_col(row, 0), payload = blob_col(row, 3);
          auto request = opt_text_col(row, 1);
          auto run     = opt_id_col(row, 2);
          if (!type || !payload || !request || !run)
            return std::unexpected(corrupt());
          if (*type != "verdict_recorded" || *request != std::optional<std::string>{v.request_id} ||
              *run != std::optional<std::uint64_t>{v.run})
            return std::unexpected(corrupt());
          auto decoded = hist::decode_event_payload_v2(*type, *payload);
          if (!decoded || decoded->references.at("verdict") != decimal(v.id) || decoded->references.at("run") != decimal(v.run) ||
              decoded->references.at("body") != decimal(v.body_id))
            return std::unexpected(corrupt());
          event_ok = true;
          return {};
        });
    if (!scan)
      return std::unexpected(scan.error());
    if (!event_ok)
      return std::unexpected(corrupt());
    // Request: operation verdict, outcome recorded, event_seq equals the verdict seq; saved result names the verdict.
    std::optional<std::string> saved;
    scan = each_row(db, "SELECT operation,outcome,event_seq,result FROM requests WHERE request_id=?1", {v.request_id},
                    [&](statement& row) -> checked {
                      auto operation = text_col(row, 0), outcome = text_col(row, 1);
                      auto seq    = id_col(row, 2);
                      auto result = blob_col(row, 3);
                      if (!operation || !outcome || !seq || !result || *operation != "verdict" || *outcome != "recorded" ||
                          *seq != v.seq)
                        return std::unexpected(corrupt());
                      saved = *result;
                      return {};
                    });
    if (!scan)
      return std::unexpected(scan.error());
    if (!saved)
      return std::unexpected(corrupt());
    auto response = hist::decode_verdict_result_v1(*saved);
    if (!response || response->verdict_id != decimal(v.id) || response->applicability != v.applicability ||
        response->decision != v.decision || response->run != decimal(v.run) || response->event != decimal(v.seq))
      return std::unexpected(corrupt());
    // 22: the run's state row is completed at this seq, of the event's role and generation, and the role is a reviewer.
    bool run_ok = false;
    scan        = each_row(db,
                           "SELECT s.job,s.role,s.generation_id,s.state,s.last_seq,r.generation_id FROM run_state s JOIN runs r "
                           "ON r.id=s.run_id WHERE s.run_id=?1",
                           {as_bind(v.run)}, [&](statement& row) -> checked {
                      auto job = text_col(row, 0), role = text_col(row, 1), state = text_col(row, 3);
                      auto generation = id_col(row, 2), last = id_col(row, 4), run_generation = id_col(row, 5);
                      if (!job || !role || !state || !generation || !last || !run_generation)
                        return std::unexpected(corrupt());
                      if (*job != v.job || *role != v.role || *generation != v.generation || *run_generation != v.generation ||
                          *state != "completed" || *last != v.seq)
                        return std::unexpected(corrupt());
                      run_ok = true;
                      return {};
                           });
    if (!scan)
      return std::unexpected(scan.error());
    if (!run_ok)
      return std::unexpected(corrupt());
    const auto generation = generations.find(v.generation);
    if (generation == generations.end())
      return std::unexpected(corrupt());
    auto kind = structure_kind(db, generation->second.structure);
    if (!kind)
      return std::unexpected(kind.error());
    if (*kind != "reviewer")
      return std::unexpected(corrupt());
    auto copy  = v;
    copy.saved = *saved;
    out.push_back(std::move(copy));
  }
  // 21: every verdict_recorded event has exactly one verdicts row.
  std::uint64_t events = 0;
  auto scan = each_row(db, "SELECT COUNT(*) FROM events WHERE type='verdict_recorded'", {}, [&](statement& row) -> checked {
    auto count = opt_int(row, 0);
    if (!count || !*count)
      return std::unexpected(corrupt());
    events = static_cast<std::uint64_t>(**count);
    return {};
  });
  if (!scan)
    return std::unexpected(scan.error());
  if (events != verdicts.size())
    return std::unexpected(corrupt());
  return {};
}

// 23, 24 and 25 for repairs; 25 and 26 for blockers.
checked check_repairs_and_blockers(const std::vector<verdict_image>& verdicts, const std::vector<repair_image>& repairs,
                                   const std::vector<blocker_image>&                blockers,
                                   const std::map<std::uint64_t, generation_image>& generations, std::uint64_t last_seq) {
  std::map<std::uint64_t, const verdict_image*> by_id;
  for (const auto& v : verdicts)
    by_id[v.id] = &v;
  std::map<std::uint64_t, std::set<std::string>> repaired;
  std::map<std::tuple<std::string, std::string, std::uint64_t>, std::vector<std::pair<std::uint64_t, std::uint64_t>>>
      by_lineage; // (job, role, start) -> (seq, revision)
  for (const auto& r : repairs) {
    const auto found = by_id.find(r.verdict);
    if (found == by_id.end())
      return std::unexpected(corrupt());
    const auto& v = *found->second;
    if (v.decision != "reject" || v.applicability != "current" || r.job != v.job)
      return std::unexpected(corrupt());
    if (!repaired[r.verdict].insert(r.role).second)
      return std::unexpected(corrupt());
    const auto generation = generations.find(r.generation);
    if (generation == generations.end() || generation->second.job != r.job || generation->second.role != r.role ||
        generation->second.opened_seq >= v.seq)
      return std::unexpected(corrupt());
    auto lineage = lineage_of(generations, r.generation);
    if (!lineage)
      return std::unexpected(lineage.error());
    by_lineage[{r.job, r.role, lineage->front()}].emplace_back(v.seq, r.revision);
  }
  // 23: the repaired (job, role) set equals the document's repair list exactly.
  for (const auto& [verdict, roles] : repaired) {
    const auto& list = by_id.at(verdict)->body.repair;
    if (std::set<std::string>(list.begin(), list.end()) != roles)
      return std::unexpected(corrupt());
  }
  // 24: revisions contiguous from 1 per (job, role) and structural lineage, in verdict order.
  for (auto& [key, entries] : by_lineage) {
    (void)key;
    std::ranges::sort(entries);
    for (std::size_t i = 0; i < entries.size(); ++i)
      if (entries[i].second != i + 1)
        return std::unexpected(corrupt());
  }
  // 25 and 26 for blockers.
  std::set<std::uint64_t>                                                          with_blocker;
  std::map<std::pair<std::string, std::string>, std::vector<const blocker_image*>> by_kind;
  for (const auto& b : blockers) {
    const auto found = by_id.find(b.verdict);
    if (found == by_id.end())
      return std::unexpected(corrupt());
    const auto& v = *found->second;
    if (v.decision != "reject" || v.applicability != "current" || b.seq != v.seq || b.job != v.job ||
        !with_blocker.insert(b.verdict).second)
      return std::unexpected(corrupt());
    if (b.kind != "max_rounds" && b.kind != "unowned_rejection")
      return std::unexpected(corrupt());
    if (b.state == "open") {
      if (b.cleared_seq || b.last_seq != b.seq)
        return std::unexpected(corrupt());
    } else if (b.state == "cleared") {
      if (!b.cleared_seq || *b.cleared_seq <= b.seq || *b.cleared_seq > last_seq || b.last_seq != *b.cleared_seq)
        return std::unexpected(corrupt());
    } else {
      return std::unexpected(corrupt());
    }
    by_kind[{b.job, b.kind}].push_back(&b);
  }
  for (auto& [key, list] : by_kind) {
    (void)key;
    std::ranges::sort(list, [](const blocker_image* a, const blocker_image* b) { return a->seq < b->seq; });
    for (std::size_t i = 0; i + 1 < list.size(); ++i)
      if (!list[i]->cleared_seq || *list[i]->cleared_seq > list[i + 1]->seq)
        return std::unexpected(corrupt()); // two blockers of one (job, kind) open at once
  }
  return {};
}

// A blocker of (job, kind) is open at a verdict when it opened earlier and was not cleared before or by that commit.
bool open_at(const std::vector<blocker_image>& blockers, std::string_view job, std::string_view kind, std::uint64_t seq) {
  return std::ranges::any_of(blockers, [&](const blocker_image& b) {
    return b.job == job && b.kind == kind && b.seq < seq && (!b.cleared_seq || *b.cleared_seq > seq);
  });
}

// 27: replaying verdicts in seq order reproduces admitted, capped and unowned exactly as R18-5.
checked check_admission_replay(connection& db, const std::vector<verdict_image>& verdicts,
                               const std::vector<repair_image>& repairs, const std::vector<blocker_image>& blockers,
                               const std::vector<activation_image>&             activations,
                               const std::map<std::uint64_t, generation_image>& generations, std::string_view identity) {
  std::map<std::pair<std::string, std::uint64_t>, std::uint64_t> counts; // (job, activation) -> admitted rejections
  for (const auto& v : verdicts) {
    const bool has_repairs = std::ranges::any_of(repairs, [&](const repair_image& r) { return r.verdict == v.id; });
    const auto blocker     = std::ranges::find_if(blockers, [&](const blocker_image& b) { return b.verdict == v.id; });
    const auto activation  = activation_at(activations, v.job, v.seq);
    if (!activation)
      return std::unexpected(corrupt());
    auto& count    = counts[{v.job, *activation}];
    auto  response = hist::decode_verdict_result_v1(v.saved);
    if (!response)
      return std::unexpected(corrupt());
    auto max_rounds = max_rounds_at(db, v.run, v.seq);
    if (!max_rounds)
      return std::unexpected(max_rounds.error());
    bool admitted = false;
    if (v.decision == "approve" || v.applicability == "historical") {
      if (has_repairs || blocker != blockers.end())
        return std::unexpected(corrupt());
    } else {
      std::string expected_kind;
      if (v.body.repair.empty())
        expected_kind = "unowned_rejection";
      else if (count < *max_rounds)
        admitted = true;
      else
        expected_kind = "max_rounds";
      if (admitted != has_repairs)
        return std::unexpected(corrupt());
      if (admitted) {
        if (blocker != blockers.end())
          return std::unexpected(corrupt());
        ++count;
      } else {
        const bool should_open = !open_at(blockers, v.job, expected_kind, v.seq);
        if (should_open != (blocker != blockers.end()))
          return std::unexpected(corrupt());
        if (should_open && blocker->kind != expected_kind)
          return std::unexpected(corrupt());
      }
    }
    hist::verdict_result_v1 expected;
    expected.ledger        = std::string(identity);
    expected.request_id    = v.request_id;
    expected.job           = v.job;
    expected.role          = v.role;
    expected.run           = decimal(v.run);
    expected.event         = decimal(v.seq);
    expected.verdict_id    = decimal(v.id);
    expected.applicability = v.applicability;
    expected.decision      = v.decision;
    expected.admitted      = admitted;
    expected.count         = decimal(count);
    expected.max_rounds    = decimal(*max_rounds);
    auto revision = each_row(db, "SELECT revision_id FROM events WHERE seq=?1", {as_bind(v.seq)}, [&](statement& row) -> checked {
      auto id = id_col(row, 0);
      if (!id)
        return std::unexpected(corrupt());
      expected.revision = decimal(*id);
      return {};
    });
    if (!revision)
      return revision;
    for (const auto& repair : repairs)
      if (repair.verdict == v.id) {
        auto generation = generations.find(repair.generation);
        if (generation == generations.end())
          return std::unexpected(corrupt());
        expected.repairs.push_back({repair.role, decimal(generation->second.ordinal), decimal(repair.revision)});
      }
    std::ranges::sort(expected.repairs, {}, &hist::verdict_result_repair::role);
    for (const auto& item : blockers) {
      if (item.verdict == v.id)
        expected.opened.push_back({decimal(item.id), item.kind});
      if (item.cleared_seq == v.seq)
        expected.cleared.push_back(decimal(item.id));
    }
    auto transitions = each_row(db, "SELECT role,after_state,after_drifted FROM transitions WHERE event_seq=?1 ORDER BY role",
                                {as_bind(v.seq)}, [&](statement& row) -> checked {
                                  auto role = text_col(row, 0), state = text_col(row, 1);
                                  auto drifted = opt_int(row, 2);
                                  if (!role || !state || !drifted || !*drifted || (**drifted != 0 && **drifted != 1))
                                    return std::unexpected(corrupt());
                                  expected.roles.push_back({*role, *state, **drifted == 1});
                                  return {};
                                });
    if (!transitions)
      return transitions;
    auto encoded = hist::encode_verdict_result_v1(expected);
    if (!encoded || *encoded != v.saved)
      return std::unexpected(corrupt());
    if (response->count != decimal(count) || response->max_rounds != decimal(*max_rounds))
      return std::unexpected(corrupt());
  }
  return {};
}

// ---------------------------------------------------------------------------------------------------------------------
// R17 completions: event/reservation/dispatch linkage and evidence vectors of the recorded observation.

value_ptr text_node(std::string text);
value_ptr list_node(std::vector<value_ptr> items);
value_ptr record_node(std::map<std::string, value_ptr> fields);

result<std::vector<execution_completion>> load_completions(connection& db, std::uint64_t last_seq) {
  std::vector<execution_completion> out;
  std::vector<std::uint64_t>        observations;
  auto                              scan = each_row(
      db,
      "SELECT c.id,c.run_id,c.seq,c.dispatch_id,c.work_fp,c.observation_id,s.job,s.role,s.generation_id,d.run_id,d.work_fp "
      "FROM completions c LEFT JOIN run_state s ON s.run_id=c.run_id LEFT JOIN dispatches d ON d.id=c.dispatch_id ORDER BY c.id",
      {}, [&](statement& row) -> checked {
        auto id = id_col(row, 0), run = id_col(row, 1), seq = id_col(row, 2), dispatch = id_col(row, 3),
             observation = id_col(row, 5), generation = id_col(row, 8), dispatch_run = id_col(row, 9);
        auto work_fp = blob_col(row, 4), dispatch_fp = blob_col(row, 10);
        auto job = text_col(row, 6), role = text_col(row, 7);
        if (!id || !run || !seq || !dispatch || !observation || !generation || !dispatch_run || !work_fp || !dispatch_fp ||
            !job || !role)
          return std::unexpected(corrupt());
        if (*dispatch_run != *run || *work_fp != *dispatch_fp || work_fp->size() != 16 || *seq > last_seq)
          return std::unexpected(corrupt());
        execution_completion c;
        c.id         = *id;
        c.job        = *job;
        c.role       = *role;
        c.generation = *generation;
        c.run        = *run;
        c.dispatch   = *dispatch;
        c.work_fp    = hex_of(*work_fp);
        c.seq        = *seq;
        out.push_back(std::move(c));
        observations.push_back(*observation);
        return {};
      });
  if (!scan)
    return std::unexpected(scan.error());
  for (std::size_t i = 0; i < out.size(); ++i) {
    const auto&                  completion = out[i];
    std::optional<std::uint64_t> reservation;
    auto                         linked = each_row(
        db,
        "SELECT e.type,e.payload,e.run_id,r.id,r.run_id,r.opened_seq,rs.state,rs.last_seq,s.state "
        "FROM events e JOIN reservations r ON r.request_id=e.request_id "
        "JOIN reservation_state rs ON rs.reservation_id=r.id JOIN run_state s ON s.run_id=r.run_id WHERE e.seq=?1",
        {as_bind(completion.seq)}, [&](statement& row) -> checked {
          auto type = text_col(row, 0), payload = blob_col(row, 1), state = text_col(row, 6), run_state = text_col(row, 8);
          auto event_run = id_col(row, 2), id = id_col(row, 3), run = id_col(row, 4), opened = id_col(row, 5),
               last = id_col(row, 7);
          if (!type || !payload || !state || !run_state || !event_run || !id || !run || !opened || !last ||
              *type != "check_recorded" || *state != "completed" || *last != completion.seq || *event_run != completion.run ||
              *run != completion.run || *run_state != "completed" || *opened >= completion.seq)
            return std::unexpected(corrupt());
          auto decoded = hist::decode_event_payload_v2(*type, *payload);
          if (!decoded || !decoded->references.contains("completion") ||
              decoded->references.at("completion") != decimal(completion.id) ||
              decoded->references.at("observation") != decimal(observations[i]) ||
              decoded->references.at("reservation") != decimal(*id))
            return std::unexpected(corrupt());
          bool dispatch_ok = false;
          auto dispatched = each_row(db, "SELECT id FROM dispatches WHERE run_id=?1 AND seq<?2 ORDER BY seq DESC,id DESC LIMIT 1",
                                     {as_bind(completion.run), as_bind(*opened)}, [&](statement& d) -> checked {
                                       auto dispatch = id_col(d, 0);
                                       if (!dispatch)
                                         return std::unexpected(corrupt());
                                       dispatch_ok = *dispatch == completion.dispatch;
                                       return {};
                                     });
          if (!dispatched)
            return dispatched;
          if (!dispatch_ok || reservation)
            return std::unexpected(corrupt());
          reservation = *id;
          return {};
        });
    if (!linked)
      return std::unexpected(linked.error());
    if (!reservation)
      return std::unexpected(corrupt());
    std::map<std::string, value_ptr> vectors;
    linked = each_row(
        db, "SELECT work,vector_id,outcome,seq FROM evidence WHERE reservation_id=?1 ORDER BY work,\"check\"",
        {as_bind(*reservation)}, [&](statement& row) -> checked {
          auto work = text_col(row, 0), outcome = text_col(row, 2);
          auto vector_id = id_col(row, 1), seq = id_col(row, 3);
          if (!work || !outcome || !vector_id || !seq || *outcome != "pass" || *seq != completion.seq)
            return std::unexpected(corrupt());
          auto vector = value_record(db, *vector_id);
          if (!vector)
            return std::unexpected(vector.error());
          auto decoded = canonical_decode(vector->canonical);
          if (!decoded || !*decoded || (*decoded)->kind != value_kind::record)
            return std::unexpected(corrupt());
          auto targets = (*decoded)->fields.find("targets");
          if (targets == (*decoded)->fields.end() || targets->second->kind != value_kind::list)
            return std::unexpected(corrupt());
          auto                             old = vectors.find(*work);
          std::map<std::string, value_ptr> union_targets;
          if (old != vectors.end())
            for (const auto& t : old->second->items)
              union_targets[t->fields.at("work")->string] = t;
          for (const auto& t : targets->second->items) {
            if (!t || t->kind != value_kind::record || !t->fields.contains("work") || !t->fields.contains("output") ||
                t->fields.at("work")->kind != value_kind::string || t->fields.at("output")->kind != value_kind::string)
              return std::unexpected(corrupt());
            auto key   = t->fields.at("work")->string;
            auto prior = union_targets.find(key);
            if (prior != union_targets.end() && prior->second->fields.at("output")->string != t->fields.at("output")->string)
              return std::unexpected(corrupt());
            union_targets[key] = t;
          }
          std::vector<value_ptr> merged;
          for (const auto& [_, t] : union_targets)
            merged.push_back(t);
          vectors[*work] = list_node(std::move(merged));
          return {};
        });
    if (!linked)
      return std::unexpected(linked.error());
    if (vectors.empty())
      return std::unexpected(corrupt());
    for (const auto& [work, targets] : vectors)
      out[i].vectors.push_back(record_node({{"work", text_node(work)}, {"targets", targets}}));
    auto record = value_record(db, observations[i]);
    if (!record)
      return std::unexpected(record.error());
    auto decoded = ::planar::engine::canonical_decode(record->canonical);
    if (!decoded || !*decoded || (*decoded)->kind != value_kind::record)
      return std::unexpected(corrupt());
    const auto units = (*decoded)->fields.find("units");
    if (units == (*decoded)->fields.end() || !units->second || units->second->kind != value_kind::list)
      return std::unexpected(corrupt());
    for (const auto& unit : units->second->items) {
      if (!unit || unit->kind != value_kind::record)
        return std::unexpected(corrupt());
      const auto work   = unit->fields.find("work");
      const auto output = unit->fields.find("output");
      if (work == unit->fields.end() || output == unit->fields.end() || !work->second || !output->second ||
          work->second->kind != value_kind::string || output->second->kind != value_kind::string)
        return std::unexpected(corrupt());
      if (!out[i].units.empty() && out[i].units.back().work >= work->second->string)
        return std::unexpected(corrupt());
      out[i].units.push_back(execution_completion_unit{work->second->string, output->second->string});
    }
  }
  return out;
}

// ---------------------------------------------------------------------------------------------------------------------
// Values of the D24 measure.

value_ptr make_node(value&& node) {
  return std::make_shared<const value>(std::move(node));
}

value_ptr text_node(std::string text) {
  value node{};
  node.kind   = value_kind::string;
  node.string = std::move(text);
  return make_node(std::move(node));
}

value_ptr list_node(std::vector<value_ptr> items) {
  value node{};
  node.kind  = value_kind::list;
  node.items = std::move(items);
  return make_node(std::move(node));
}

value_ptr record_node(std::map<std::string, value_ptr> fields) {
  value node{};
  node.kind   = value_kind::record;
  node.fields = std::move(fields);
  return make_node(std::move(node));
}

} // namespace

// ---------------------------------------------------------------------------------------------------------------------
// Module-linkage helpers.

std::expected<std::vector<std::uint64_t>, execution_error> lineage_generations(connection& db, std::uint64_t generation) {
  auto generations = load_generations(db);
  if (!generations)
    return std::unexpected(generations.error());
  return lineage_of(*generations, generation);
}

std::expected<std::uint64_t, execution_error> repair_revision(connection& db, std::string_view job, std::string_view role,
                                                              std::uint64_t generation) {
  auto lineage = lineage_generations(db, generation);
  if (!lineage)
    return std::unexpected(lineage.error());
  std::uint64_t revision = 0;
  std::uint64_t latest   = 0;
  auto          scan     = each_row(db,
                                    "SELECT r.generation_id,r.revision,v.seq FROM repairs r JOIN verdicts v ON v.id=r.verdict_id "
                                    "WHERE r.job=?1 AND r.role=?2",
                                    {std::string(job), std::string(role)}, [&](statement& row) -> checked {
                         auto generation_id = id_col(row, 0), number = id_col(row, 1), seq = id_col(row, 2);
                         if (!generation_id || !number || !seq)
                           return std::unexpected(corrupt());
                         if (std::ranges::find(*lineage, *generation_id) != lineage->end() && *seq >= latest) {
                           latest   = *seq;
                           revision = *number;
                         }
                         return {};
                                    });
  if (!scan)
    return std::unexpected(scan.error());
  return revision;
}

std::expected<std::optional<std::uint64_t>, execution_error> verdict_activation(connection& db, std::uint64_t verdict) {
  std::optional<std::pair<std::string, std::uint64_t>> located;
  auto scan = each_row(db, "SELECT e.job,v.seq FROM verdicts v JOIN events e ON e.seq=v.seq WHERE v.id=?1", {as_bind(verdict)},
                       [&](statement& row) -> checked {
                         auto job = text_col(row, 0);
                         auto seq = id_col(row, 1);
                         if (!job || !seq)
                           return std::unexpected(corrupt());
                         located = std::pair{*job, *seq};
                         return {};
                       });
  if (!scan)
    return std::unexpected(scan.error());
  if (!located)
    return std::unexpected(corrupt());
  auto activations = load_activations(db);
  if (!activations)
    return std::unexpected(activations.error());
  return activation_at(*activations, located->first, located->second);
}

std::expected<std::uint64_t, execution_error> review_count(connection& db, std::string_view job, std::uint64_t activation) {
  auto activations = load_activations(db);
  if (!activations)
    return std::unexpected(activations.error());
  std::uint64_t count = 0;
  auto scan = each_row(db,
                       "SELECT v.seq FROM verdicts v JOIN events e ON e.seq=v.seq WHERE e.job=?1 AND EXISTS(SELECT 1 FROM "
                       "repairs r WHERE r.verdict_id=v.id) ORDER BY v.seq",
                       {std::string(job)}, [&](statement& row) -> checked {
                         auto seq = id_col(row, 0);
                         if (!seq)
                           return std::unexpected(seq.error());
                         if (activation_at(*activations, job, *seq) == std::optional<std::uint64_t>{activation})
                           ++count;
                         return {};
                       });
  if (!scan)
    return std::unexpected(scan.error());
  return count;
}

std::expected<std::vector<execution_blocker>, execution_error> open_blockers(connection& db, std::string_view job) {
  std::vector<execution_blocker> out;
  auto scan = each_row(db,
                       "SELECT b.id,b.seq,b.job,b.kind,b.verdict_id FROM blockers b JOIN blocker_state s ON s.blocker_id=b.id "
                       "WHERE s.state='open' AND b.job=?1 ORDER BY b.id",
                       {std::string(job)}, [&](statement& row) -> checked {
                         auto id = id_col(row, 0), seq = id_col(row, 1), verdict = id_col(row, 4);
                         auto job_text = text_col(row, 2), kind = text_col(row, 3);
                         if (!id || !seq || !verdict || !job_text || !kind)
                           return std::unexpected(corrupt());
                         out.push_back(execution_blocker{*id, *seq, *job_text, *kind, *verdict});
                         return {};
                       });
  if (!scan)
    return std::unexpected(scan.error());
  return out;
}

std::expected<void, execution_error> read_review(connection& db, execution_snapshot& snapshot) {
  auto generations = load_generations(db);
  if (!generations)
    return std::unexpected(generations.error());
  auto activations = load_activations(db);
  if (!activations)
    return std::unexpected(activations.error());
  auto loaded = load_verdicts(db);
  if (!loaded)
    return std::unexpected(loaded.error());
  auto repairs = load_repairs(db);
  if (!repairs)
    return std::unexpected(repairs.error());
  auto blockers = load_blockers(db);
  if (!blockers)
    return std::unexpected(blockers.error());
  auto completions = load_completions(db, snapshot.last_seq);
  if (!completions)
    return std::unexpected(completions.error());

  std::vector<verdict_image> verdicts;
  if (auto ok = check_verdict_rows(db, *loaded, *generations, verdicts); !ok)
    return ok;
  if (auto ok = check_repairs_and_blockers(verdicts, *repairs, *blockers, *generations, snapshot.last_seq); !ok)
    return ok;
  if (auto ok = check_admission_replay(db, verdicts, *repairs, *blockers, *activations, *generations, snapshot.identity); !ok)
    return ok;

  // Snapshot values (DR18-8). Only the active job's current lineages contribute.
  snapshot.completions.clear();
  snapshot.review = execution_review{};
  for (const auto& b : *blockers)
    if (b.state == "open")
      snapshot.review.blockers.push_back(execution_blocker{b.id, b.seq, b.job, b.kind, b.verdict});
  if (snapshot.active_job.empty())
    return {};
  std::set<std::uint64_t> selected;
  for (const auto& role : snapshot.roles) {
    if (role.job != snapshot.active_job || role.state == "retired")
      continue;
    auto lineage = lineage_of(*generations, role.generation_id);
    if (!lineage)
      return std::unexpected(lineage.error());
    for (const auto& c : *completions)
      if (c.job == role.job && c.role == role.role && std::ranges::find(*lineage, c.generation) != lineage->end())
        snapshot.completions.push_back(c);
    if (role.structure.kind == "worker") {
      auto revision = repair_revision(db, role.job, role.role, role.generation_id);
      if (!revision)
        return std::unexpected(revision.error());
      if (*revision > 0)
        snapshot.review.repairs.push_back(execution_repair{role.job, role.role, *revision});
    } else if (role.structure.kind == "reviewer") {
      // The latest verdict of the reviewer lineage.
      const verdict_image* latest = nullptr;
      for (const auto& v : verdicts)
        if (v.job == role.job && v.role == role.role && std::ranges::find(*lineage, v.generation) != lineage->end())
          latest = &v;
      if (latest)
        selected.insert(latest->id);
    }
  }
  std::ranges::sort(snapshot.completions,
                    [](const execution_completion& a, const execution_completion& b) { return a.id < b.id; });
  if (snapshot.activation) {
    for (const auto& v : verdicts)
      if (v.job == snapshot.active_job && activation_at(*activations, v.job, v.seq) == snapshot.activation)
        selected.insert(v.id);
    auto count = review_count(db, snapshot.active_job, *snapshot.activation);
    if (!count)
      return std::unexpected(count.error());
    snapshot.review.count = *count;
  }
  for (const auto& v : verdicts)
    if (selected.contains(v.id))
      snapshot.review.verdicts.push_back(execution_verdict{v.id, v.request_id, v.run, v.seq, v.applicability, v.decision, v.body,
                                                           v.generation, activation_at(*activations, v.job, v.seq)});
  return {};
}

std::vector<std::pair<std::string, value_ptr>> review_state_entries(const execution_snapshot& snapshot) {
  std::vector<std::pair<std::string, value_ptr>> out;
  if (!snapshot.completions.empty()) {
    std::vector<value_ptr> items;
    for (const auto& c : snapshot.completions) {
      std::vector<value_ptr> units;
      for (const auto& unit : c.units)
        units.push_back(record_node({{"work", text_node(unit.work)}, {"output", text_node(unit.output)}}));
      items.push_back(record_node({{"id", text_node(decimal(c.id))},
                                   {"job", text_node(c.job)},
                                   {"role", text_node(c.role)},
                                   {"generation", text_node(decimal(c.generation))},
                                   {"run", text_node(decimal(c.run))},
                                   {"dispatch", text_node(decimal(c.dispatch))},
                                   {"work_fp", text_node(c.work_fp)},
                                   {"seq", text_node(decimal(c.seq))},
                                   {"units", list_node(std::move(units))},
                                   {"vectors", list_node(c.vectors)}}));
    }
    out.emplace_back("completions", list_node(std::move(items)));
  }
  const auto& review = snapshot.review;
  if (review.count != 0 || !review.verdicts.empty() || !review.blockers.empty() || !review.repairs.empty()) {
    std::vector<value_ptr> verdicts;
    for (const auto& v : review.verdicts) {
      auto body = hist::verdict_value(v.body);
      verdicts.push_back(record_node({{"id", text_node(decimal(v.id))},
                                      {"request_id", text_node(v.request_id)},
                                      {"run", text_node(decimal(v.run))},
                                      {"generation", text_node(decimal(v.generation))},
                                      {"activation", text_node(v.activation ? decimal(*v.activation) : "none")},
                                      {"seq", text_node(decimal(v.seq))},
                                      {"applicability", text_node(v.applicability)},
                                      {"decision", text_node(v.decision)},
                                      {"body", body ? *body : text_node("")}}));
    }
    std::vector<value_ptr> blockers;
    for (const auto& b : review.blockers)
      blockers.push_back(record_node({{"id", text_node(decimal(b.id))},
                                      {"seq", text_node(decimal(b.seq))},
                                      {"job", text_node(b.job)},
                                      {"kind", text_node(b.kind)},
                                      {"verdict", text_node(decimal(b.verdict))}}));
    std::vector<value_ptr> repairs;
    for (const auto& r : review.repairs)
      repairs.push_back(
          record_node({{"job", text_node(r.job)}, {"role", text_node(r.role)}, {"revision", text_node(decimal(r.revision))}}));
    out.emplace_back("review", record_node({{"count", text_node(decimal(review.count))},
                                            {"verdicts", list_node(std::move(verdicts))},
                                            {"blockers", list_node(std::move(blockers))},
                                            {"repairs", list_node(std::move(repairs))}}));
  }
  return out;
}

// ---------------------------------------------------------------------------------------------------------------------
// R18 P3: the verdict commit (R18-4), its native step-5 checks, blocker clearing (R18-5, DB-R7) and the history
// expansion of verdict_recorded (R18-8).

namespace {

constexpr std::string_view digest_prefix = "xxh3_128:";

execution_error invalid() {
  return failure(execution_error_code::invalid_change);
}

execution_error conflict() {
  return failure(execution_error_code::state_conflict);
}

bool is_identifier(std::string_view text) {
  if (text.empty() || text.size() > 64 || text.front() < 'a' || text.front() > 'z')
    return false;
  return std::ranges::all_of(text, [](char c) { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'; });
}

const execution_role* role_of(const execution_snapshot& snapshot, std::string_view job, std::string_view role) {
  for (const auto& row : snapshot.roles)
    if (row.job == job && row.role == role)
      return &row;
  return nullptr;
}

std::optional<std::string> canonical_text(const value_ptr& node) {
  if (!node)
    return std::nullopt;
  auto encoded = ::planar::engine::canonical_encode(node);
  if (!encoded)
    return std::nullopt;
  return *encoded;
}

// R18-2 steps 4 to 7 natively (the reference rule excepted, DB-R8): the same rules normalize_verdict applies.
std::optional<hist::verdict_v1> normalize_document(const value_ptr& document) {
  if (!document || !hist::check_verdict_bounds(document))
    return std::nullopt;
  auto typed = hist::verdict_from_value(document);
  if (!typed)
    return std::nullopt;
  if (typed->authority.empty())
    typed->authority = std::string(hist::verdict_authority_reviewer);
  else if (typed->authority != hist::verdict_authority_reviewer)
    return std::nullopt;
  if (!hist::check_verdict_decision(*typed))
    return std::nullopt;
  return *typed;
}

bool same_targets(const std::vector<hist::verdict_target>& left, const std::vector<hist::verdict_target>& right) {
  if (left.size() != right.size())
    return false;
  for (std::size_t i = 0; i < left.size(); ++i)
    if (left[i].work != right[i].work || left[i].output != right[i].output || left[i].work_fp != right[i].work_fp)
      return false;
  return true;
}

// The worker role of the job whose structure owns the work unit.
const execution_role* owner_of(const execution_snapshot& snapshot, std::string_view job, std::string_view work) {
  for (const auto& row : snapshot.roles)
    if (row.job == job && row.structure.kind == "worker" &&
        std::ranges::find(row.structure.work, work) != row.structure.work.end())
      return &row;
  return nullptr;
}

// R18-5 case of a verdict from its applicability, document and the review count before the commit.
std::string admission_case(bool current, const hist::verdict_v1& verdict, std::uint64_t count, std::uint64_t max_rounds) {
  if (!current)
    return "historical";
  if (verdict.decision == "approve")
    return "approve";
  if (verdict.repair.empty())
    return "unowned";
  return count < max_rounds ? "admitted" : "capped";
}

std::string blocker_kind_of(std::string_view admission) {
  if (admission == "unowned")
    return "unowned_rejection";
  if (admission == "capped")
    return "max_rounds";
  return {};
}

// Reviewer after-state (R18-4 rows M37, M42, M43; DR18-1): admitted leaves a start prerequisite absent (blocked); a
// historical verdict leaves the reviewer claimable again (stale) when every worker is done, else blocked.
std::string reviewer_after(std::string_view admission, const execution_snapshot& snapshot, std::string_view job) {
  if (admission == "admitted")
    return "blocked";
  if (admission == "historical") {
    const bool all_done = std::ranges::all_of(snapshot.roles, [&](const execution_role& row) {
      return row.job != job || row.structure.kind != "worker" || row.state == "done";
    });
    return all_done ? "stale" : "blocked";
  }
  return "done";
}

checked check_verdict_transitions(const execution_snapshot& snapshot, const execution_change& change,
                                  const std::string& admission) {
  const auto& facts = *change.verdict;
  const auto& job   = snapshot.active_job;
  const auto& first = change.transitions.front();
  if (first.job != job || first.role != facts.role || first.before_state != std::optional<std::string>("running") ||
      first.before_drifted != std::optional<bool>(false) || first.after_state != reviewer_after(admission, snapshot, job) ||
      first.after_drifted || first.reason != "completed")
    return std::unexpected(invalid());
  std::set<std::string> repaired;
  std::string           previous;
  for (std::size_t i = 1; i < change.transitions.size(); ++i) {
    const auto& t = change.transitions[i];
    if (t.job != job || t.role == facts.role || (i > 1 && t.role <= previous))
      return std::unexpected(invalid());
    previous        = t.role;
    const auto* row = role_of(snapshot, job, t.role);
    if (!row || t.before_state != std::optional<std::string>(row->state) || t.before_drifted != std::optional<bool>(row->drifted))
      return std::unexpected(invalid());
    // No run starts or ends besides the reviewer's: a running role never transitions in a verdict commit.
    if (row->state == "running" || t.after_state == "running" || t.after_drifted)
      return std::unexpected(invalid());
    if (t.reason == "repair_admitted") {
      if (admission != "admitted" || std::ranges::find(facts.repairs, t.role) == facts.repairs.end())
        return std::unexpected(invalid());
      // M39 done -> stale; M44 blocked -> blocked. Any other before state cannot be in a current vector (internal).
      if (!((row->state == "done" && t.after_state == "stale") || (row->state == "blocked" && t.after_state == "blocked")))
        return std::unexpected(invalid());
      repaired.insert(t.role);
    } else if (t.reason != "prerequisite_available" && t.reason != "prerequisite_unavailable") {
      return std::unexpected(invalid()); // only the supplied M17 to M22 rows
    }
  }
  if (repaired != std::set<std::string>(facts.repairs.begin(), facts.repairs.end()))
    return std::unexpected(invalid());
  std::vector<hist::transition_row> rows;
  for (std::size_t i = 0; i < change.transitions.size(); ++i) {
    const auto&          t = change.transitions[i];
    hist::transition_row row;
    row.ordinal        = i + 1;
    row.job            = t.job;
    row.role           = t.role;
    row.generation_id  = 1;
    row.before_state   = t.before_state;
    row.before_drifted = t.before_drifted;
    row.after_state    = t.after_state;
    row.after_drifted  = t.after_drifted;
    row.reason         = t.reason;
    rows.push_back(std::move(row));
  }
  if (!hist::validate_transition_sequence(rows))
    return std::unexpected(invalid());
  return {};
}

checked check_verdict_vector(const execution_snapshot& snapshot, const execution_change& change) {
  std::map<std::string, execution_vector_entry> wanted;
  for (const auto& row : snapshot.roles)
    if (row.job == snapshot.active_job)
      wanted[row.role] =
          execution_vector_entry{.job = row.job, .role = row.role, .state = row.state, .drifted = row.drifted, .run = row.run_id};
  for (const auto& t : change.transitions)
    wanted[t.role] =
        execution_vector_entry{.job = t.job, .role = t.role, .state = t.after_state, .drifted = false, .run = std::nullopt};
  if (change.vector.size() != wanted.size())
    return std::unexpected(invalid());
  std::set<std::string> seen;
  for (const auto& entry : change.vector) {
    const auto found = wanted.find(entry.role);
    if (entry.job != snapshot.active_job || found == wanted.end() || !seen.insert(entry.role).second ||
        found->second.state != entry.state || found->second.drifted != entry.drifted || found->second.run != entry.run)
      return std::unexpected(invalid());
  }
  return {};
}

// The exact R18-4 precondition set: active_revision, active_job, fact_version of every role of the active job,
// role_state(reviewer) == (running, false, run), generation_current(reviewer, G), run_active(run, G), request_unused.
checked check_verdict_expected(const execution_snapshot& snapshot, const execution_change& change,
                               const execution_role& reviewer) {
  using kind_t                             = execution_expectation::kind_t;
  const auto&                        facts = *change.verdict;
  std::vector<execution_expectation> wanted;
  auto                               add = [&](kind_t kind) -> execution_expectation& {
    wanted.push_back(execution_expectation{});
    wanted.back().kind = kind;
    return wanted.back();
  };
  add(kind_t::active_revision).revision = snapshot.revision->id;
  add(kind_t::active_job).text          = snapshot.active_job;
  for (const auto& row : snapshot.roles)
    if (row.job == snapshot.active_job) {
      auto& fact  = add(kind_t::fact_version);
      fact.job    = row.job;
      fact.role   = row.role;
      fact.number = row.fact_version;
    }
  auto& state    = add(kind_t::role_state);
  state.job      = reviewer.job;
  state.role     = reviewer.role;
  state.state    = "running";
  state.drifted  = false;
  state.run      = facts.run;
  auto& current  = add(kind_t::generation_current);
  current.job    = reviewer.job;
  current.role   = reviewer.role;
  current.number = reviewer.generation_id;
  auto& active   = add(kind_t::run_active);
  active.job     = reviewer.job;
  active.role    = reviewer.role;
  active.run     = facts.run;
  active.number  = reviewer.generation_id;
  add(kind_t::request_unused);
  if (change.expected.size() != wanted.size())
    return std::unexpected(invalid());
  std::vector<bool> used(wanted.size(), false);
  for (const auto& given : change.expected) {
    bool matched = false;
    for (std::size_t i = 0; i < wanted.size() && !matched; ++i) {
      const auto& w = wanted[i];
      if (used[i] || w.kind != given.kind || w.job != given.job || w.role != given.role || given.no_row)
        continue;
      bool same = false;
      switch (w.kind) {
      case kind_t::active_revision:
        same = w.revision == given.revision;
        break;
      case kind_t::active_job:
        same = w.text == given.text;
        break;
      case kind_t::fact_version:
      case kind_t::generation_current:
        same = w.number == given.number;
        break;
      case kind_t::role_state:
        same = w.state == given.state && w.drifted == given.drifted && w.run == given.run;
        break;
      case kind_t::run_active:
        same = w.run == given.run && w.number == given.number;
        break;
      default:
        same = true;
        break;
      }
      if (same) {
        used[i] = true;
        matched = true;
      }
    }
    if (!matched)
      return std::unexpected(invalid());
  }
  return {};
}

struct locked_role_row {
  std::uint64_t                generation = 0;
  std::string                  state;
  bool                         drifted = false;
  std::optional<std::uint64_t> run;
  std::uint64_t                fact_version = 0;
};

result<std::optional<locked_role_row>> read_role_row(connection& db, std::string_view job, std::string_view role) {
  std::optional<locked_role_row> out;
  auto scan = each_row(db, "SELECT generation_id,state,drifted,run_id,fact_version FROM role_state WHERE job=?1 AND role=?2",
                       {std::string(job), std::string(role)}, [&](statement& row) -> checked {
                         auto generation = id_col(row, 0), fact = id_col(row, 4);
                         auto state   = text_col(row, 1);
                         auto drifted = opt_int(row, 2);
                         auto run     = opt_id_col(row, 3);
                         if (!generation || !fact || !state || !drifted || !*drifted || !run)
                           return std::unexpected(corrupt());
                         out = locked_role_row{*generation, *state, **drifted != 0, *run, *fact};
                         return {};
                       });
  if (!scan)
    return std::unexpected(scan.error());
  return out;
}

struct locked_run_row {
  std::string   state;
  std::uint64_t generation = 0;
};

result<std::optional<locked_run_row>> read_run_row(connection& db, std::uint64_t run) {
  std::optional<locked_run_row> out;
  auto                          scan =
      each_row(db, "SELECT state,generation_id FROM run_state WHERE run_id=?1", {as_bind(run)}, [&](statement& row) -> checked {
        auto state      = text_col(row, 0);
        auto generation = id_col(row, 1);
        if (!state || !generation)
          return std::unexpected(corrupt());
        out = locked_run_row{*state, *generation};
        return {};
      });
  if (!scan)
    return std::unexpected(scan.error());
  return out;
}

result<std::uint64_t> next_of(connection& db, std::string_view sql) {
  std::optional<std::uint64_t> out;
  auto                         scan = each_row(db, sql, {}, [&](statement& row) -> checked {
    auto value = opt_int(row, 0);
    if (!value || !*value || **value < 0)
      return std::unexpected(corrupt());
    out = static_cast<std::uint64_t>(**value) + 1;
    return {};
  });
  if (!scan)
    return std::unexpected(scan.error());
  if (!out)
    return std::unexpected(corrupt());
  return *out;
}

result<std::uint64_t> generation_ordinal_of(connection& db, std::uint64_t generation) {
  std::optional<std::uint64_t> out;
  auto scan = each_row(db, "SELECT ordinal FROM generations WHERE id=?1", {as_bind(generation)}, [&](statement& row) -> checked {
    auto ordinal = id_col(row, 0);
    if (!ordinal)
      return std::unexpected(ordinal.error());
    out = *ordinal;
    return {};
  });
  if (!scan)
    return std::unexpected(scan.error());
  if (!out)
    return std::unexpected(corrupt());
  return *out;
}

checked write_sql(connection& db, std::string_view sql, const std::vector<bind_value>& binds) {
  return each_row(db, sql, binds, [](statement&) -> checked { return {}; });
}

value_ptr unknown_node() {
  return record_node({{"status", text_node("unknown")}});
}

} // namespace

std::expected<void, execution_error> check_verdict_shape(const execution_change& change) {
  using kind_t = execution_expectation::kind_t;
  if (change.operation != "verdict" || !change.verdict)
    return std::unexpected(invalid());
  const auto& facts = *change.verdict;
  if (!is_identifier(change.job) || !is_identifier(facts.role) || change.role != facts.role || facts.run == 0)
    return std::unexpected(invalid());
  if (change.previous_run || change.revision || change.packet || change.provenance || !change.generations.empty() ||
      !change.fenced_runs.empty() || change.transitions.empty())
    return std::unexpected(invalid());
  if (!facts.document || !facts.current.work)
    return std::unexpected(invalid());
  constexpr std::array<std::string_view, 5> cases{"approve", "admitted", "capped", "unowned", "historical"};
  if (std::ranges::find(cases, facts.admission) == cases.end())
    return std::unexpected(invalid());
  for (const auto& kind : facts.blockers_opened)
    if (kind != "max_rounds" && kind != "unowned_rejection")
      return std::unexpected(invalid());
  if (std::set<std::uint64_t>(change.blockers_cleared.begin(), change.blockers_cleared.end()).size() !=
      change.blockers_cleared.size())
    return std::unexpected(invalid());
  for (const auto& member : change.expected) {
    if (member.no_row || member.kind == kind_t::reservation_open)
      return std::unexpected(invalid());
  }
  for (const auto& entry : change.vector)
    if (!is_identifier(entry.job) || !is_identifier(entry.role) || (entry.drifted && entry.state != "running") ||
        (entry.run && entry.state != "running"))
      return std::unexpected(invalid());
  return {};
}

std::expected<void, execution_error> check_verdict_snapshot(const execution_snapshot& snapshot, const execution_change& change) {
  const auto& facts = *change.verdict;
  const auto& job   = snapshot.active_job;
  if (job.empty() || change.job != job || !snapshot.revision || !snapshot.activation)
    return std::unexpected(invalid());
  const auto* reviewer = role_of(snapshot, job, facts.role);
  if (!reviewer || reviewer->structure.kind != "reviewer" || reviewer->state != "running" || reviewer->drifted ||
      reviewer->run_id != std::optional<std::uint64_t>(facts.run) || !reviewer->dispatch || !reviewer->dispatch->work)
    return std::unexpected(invalid());

  // The change's verdict equals the native normalization of the document read by this invocation (bytes compared).
  auto normalized = normalize_document(facts.document);
  if (!normalized)
    return std::unexpected(invalid());
  auto mine  = hist::encode_verdict_v1(*normalized);
  auto given = hist::encode_verdict_v1(facts.verdict);
  if (!mine || !given || *mine != *given)
    return std::unexpected(invalid());
  const auto& verdict = facts.verdict;

  // facts.current.work is the reviewer's acknowledged W (DB-R5); the policy is its work_exec_v1 digest.
  const auto work_text  = canonical_text(facts.current.work);
  const auto dispatched = canonical_text(reviewer->dispatch->work);
  auto       digest     = hist::work_exec_digest(facts.current.work);
  if (!work_text || !dispatched || *work_text != *dispatched || !digest || *digest != facts.current.policy ||
      facts.current.policy != std::format("{}{}", digest_prefix, reviewer->dispatch->work_fp))
    return std::unexpected(invalid());
  const auto cap = max_rounds_of_work(facts.current.work);
  if (!cap || *cap != facts.max_rounds)
    return std::unexpected(invalid());

  // Each current target names the output and work fingerprint of a completion in its owner's current lineage.
  const auto& reviewed = reviewer->structure.targets;
  for (std::size_t i = 0; i < facts.current.targets.size(); ++i) {
    const auto& target = facts.current.targets[i];
    if ((i > 0 && target.work <= facts.current.targets[i - 1].work) || std::ranges::find(reviewed, target.work) == reviewed.end())
      return std::unexpected(invalid());
    const auto* owner = owner_of(snapshot, job, target.work);
    if (!owner)
      return std::unexpected(invalid());
    const bool found = std::ranges::any_of(snapshot.completions, [&](const execution_completion& completion) {
      if (completion.job != job || completion.role != owner->role ||
          std::format("{}{}", digest_prefix, completion.work_fp) != target.work_fp)
        return false;
      return std::ranges::any_of(completion.units, [&](const execution_completion_unit& unit) {
        return unit.work == target.work && unit.output == target.output;
      });
    });
    if (!found)
      return std::unexpected(invalid());
  }
  const bool complete = facts.current.targets.size() == reviewed.size();

  // A repair entry must be a worker of the active job owning a target of the document (DB-R8; Lua's refusal is reference).
  for (const auto& role : verdict.repair) {
    const auto* row = role_of(snapshot, job, role);
    if (!row || row->structure.kind != "worker")
      return std::unexpected(invalid());
    const bool owns = std::ranges::any_of(verdict.targets, [&](const hist::verdict_target& target) {
      return std::ranges::find(row->structure.work, target.work) != row->structure.work.end();
    });
    if (!owns)
      return std::unexpected(invalid());
  }

  // Applicability (R18-3) and the admission case (R18-5) from the native count.
  const bool current = complete && same_targets(verdict.targets, facts.current.targets) && verdict.policy == facts.current.policy;
  const auto admission = admission_case(current, verdict, snapshot.review.count, facts.max_rounds);
  if (facts.admission != admission)
    return std::unexpected(invalid());
  const std::vector<std::string> repairs = admission == "admitted" ? verdict.repair : std::vector<std::string>{};
  if (facts.repairs != repairs)
    return std::unexpected(invalid());
  std::vector<std::string> opened;
  if (const auto kind = blocker_kind_of(admission); !kind.empty()) {
    const bool open = std::ranges::any_of(snapshot.review.blockers, [&](const execution_blocker& blocker) {
      return blocker.job == job && blocker.kind == kind &&
             std::ranges::find(change.blockers_cleared, blocker.id) == change.blockers_cleared.end();
    });
    if (!open)
      opened.push_back(kind);
  }
  if (facts.blockers_opened != opened)
    return std::unexpected(invalid());
  for (auto id : change.blockers_cleared)
    if (std::ranges::none_of(snapshot.review.blockers, [&](const execution_blocker& blocker) { return blocker.id == id; }))
      return std::unexpected(invalid());

  if (auto ok = check_verdict_transitions(snapshot, change, admission); !ok)
    return ok;
  if (auto ok = check_verdict_vector(snapshot, change); !ok)
    return ok;
  return check_verdict_expected(snapshot, change, *reviewer);
}

std::expected<void, execution_error> check_blockers_cleared(const execution_snapshot& snapshot, const execution_change& change) {
  const auto& ids = change.blockers_cleared;
  if (std::set<std::uint64_t>(ids.begin(), ids.end()).size() != ids.size())
    return std::unexpected(invalid());
  if ((change.operation == "claim" || change.operation == "takeover") && !ids.empty())
    return std::unexpected(invalid());
  // activate and a load that clears the active job clear every open blocker (R18-5, D94).
  const bool all = change.operation == "activate" || (change.operation == "load" && change.job.empty());
  if (all)
    for (const auto& blocker : snapshot.review.blockers)
      if (std::ranges::find(ids, blocker.id) == ids.end())
        return std::unexpected(invalid());
  // Openness is checked under the lock (state_conflict); the full clearing rule of other loads is Lua's (DB-R7).
  return {};
}

std::expected<void, execution_error> check_verdict_locked(connection& db, const execution_change& change) {
  using kind_t                       = execution_expectation::kind_t;
  const auto&                  facts = *change.verdict;
  std::optional<std::uint64_t> revision;
  std::string                  job;
  auto head = each_row(db, "SELECT active_revision_id,active_job FROM current_state WHERE singleton=1", {},
                       [&](statement& row) -> checked {
                         auto rev    = opt_id_col(row, 0);
                         auto active = opt_text_col(row, 1);
                         if (!rev || !active)
                           return std::unexpected(corrupt());
                         revision = *rev;
                         job      = active->value_or(std::string{});
                         return {};
                       });
  if (!head)
    return std::unexpected(head.error());

  // Lock-time mapping (R18-4): when the reviewer's locked run differs from the token's run, or that run is no longer
  // active in the expected generation, the failure is fenced with the locked run_state; otherwise state_conflict.
  std::uint64_t expected_generation = 0;
  for (const auto& member : change.expected)
    if (member.kind == kind_t::run_active)
      expected_generation = member.number;
  auto refusal = [&]() -> execution_error {
    auto run = read_run_row(db, facts.run);
    auto row = read_role_row(db, change.job, facts.role);
    if (!run || !row)
      return !run ? run.error() : row.error();
    if (!*run)
      return conflict();
    const bool moved = !*row || (*row)->run != std::optional<std::uint64_t>(facts.run);
    if (moved || (*run)->state != "active" || (*run)->generation != expected_generation) {
      execution_error error = failure(execution_error_code::fenced);
      error.state           = (*run)->state;
      return error;
    }
    return conflict();
  };

  std::vector<execution_expectation> members = change.expected;
  std::stable_sort(members.begin(), members.end(), [](const execution_expectation& a, const execution_expectation& b) {
    if (a.kind != b.kind)
      return static_cast<int>(a.kind) < static_cast<int>(b.kind);
    return std::tie(a.job, a.role) < std::tie(b.job, b.role);
  });
  for (const auto& member : members) {
    bool ok = true;
    switch (member.kind) {
    case kind_t::active_revision:
      ok = revision == member.revision;
      break;
    case kind_t::active_job:
      ok = job == member.text;
      break;
    case kind_t::fact_version:
    case kind_t::role_state:
    case kind_t::generation_current: {
      auto row = read_role_row(db, member.job, member.role);
      if (!row)
        return std::unexpected(row.error());
      if (!*row)
        ok = false;
      else if (member.kind == kind_t::fact_version)
        ok = (*row)->fact_version == member.number;
      else if (member.kind == kind_t::generation_current)
        ok = (*row)->generation == member.number;
      else
        ok = (*row)->state == member.state && (*row)->drifted == member.drifted && (*row)->run == member.run;
      break;
    }
    case kind_t::run_active: {
      auto run = read_run_row(db, facts.run);
      if (!run)
        return std::unexpected(run.error());
      ok = *run && (*run)->state == "active" && (*run)->generation == member.number &&
           member.run == std::optional<std::uint64_t>(facts.run);
      break;
    }
    case kind_t::request_unused:
      break;
    default:
      return std::unexpected(invalid());
    }
    if (!ok)
      return std::unexpected(refusal());
  }
  // The locked before states of every transition (D40).
  for (const auto& t : change.transitions) {
    auto row = read_role_row(db, t.job, t.role);
    if (!row)
      return std::unexpected(row.error());
    if (!*row || !t.before_state || (*row)->state != *t.before_state || std::optional<bool>((*row)->drifted) != t.before_drifted)
      return std::unexpected(conflict());
  }
  return {};
}

std::expected<void, execution_error> clear_blockers(connection& db, const std::vector<std::uint64_t>& ids, std::uint64_t seq,
                                                    std::string_view job) {
  for (auto id : ids) {
    bool open = false;
    auto scan = each_row(db, "SELECT b.job,s.state FROM blockers b JOIN blocker_state s ON s.blocker_id=b.id WHERE b.id=?1",
                         {as_bind(id)}, [&](statement& row) -> checked {
                           auto owner = text_col(row, 0), state = text_col(row, 1);
                           if (!owner || !state)
                             return std::unexpected(corrupt());
                           open = *state == "open" && (job.empty() || *owner == job);
                           return {};
                         });
    if (!scan)
      return std::unexpected(scan.error());
    if (!open)
      return std::unexpected(conflict());
    if (auto updated = write_sql(db, "UPDATE blocker_state SET state='cleared', cleared_seq=?1, last_seq=?1 WHERE blocker_id=?2",
                                 {as_bind(seq), as_bind(id)});
        !updated)
      return std::unexpected(updated.error());
  }
  return {};
}

std::expected<execution_commit_result, execution_error> write_verdict(connection& db, std::string_view identity,
                                                                      const execution_change& change, std::uint64_t request_body,
                                                                      std::uint64_t verdict_body, std::string_view now) {
  const auto&                  facts = *change.verdict;
  std::optional<std::uint64_t> revision;
  std::optional<std::uint64_t> activation;
  std::string                  job;
  auto head = each_row(db, "SELECT active_revision_id,active_job,active_activation_id FROM current_state WHERE singleton=1", {},
                       [&](statement& row) -> checked {
                         auto rev = opt_id_col(row, 0), act = opt_id_col(row, 2);
                         auto active = opt_text_col(row, 1);
                         if (!rev || !act || !active)
                           return std::unexpected(corrupt());
                         revision   = *rev;
                         activation = *act;
                         job        = active->value_or(std::string{});
                         return {};
                       });
  if (!head)
    return std::unexpected(head.error());
  if (!revision || !activation || job != change.job)
    return std::unexpected(conflict());
  auto seq = next_of(db, "SELECT COALESCE(MAX(seq),0) FROM events");
  if (!seq)
    return std::unexpected(seq.error());
  auto verdict_id = next_of(db, "SELECT COALESCE(MAX(id),0) FROM verdicts");
  if (!verdict_id)
    return std::unexpected(verdict_id.error());
  auto blocker_base = next_of(db, "SELECT COALESCE(MAX(id),0) FROM blockers");
  if (!blocker_base)
    return std::unexpected(blocker_base.error());
  auto reviewer = read_role_row(db, job, facts.role);
  if (!reviewer)
    return std::unexpected(reviewer.error());
  if (!*reviewer)
    return std::unexpected(conflict());

  // Native admission under the lock: the count of this activation and the case it gives (R18-5, E17 A).
  auto count = review_count(db, job, *activation);
  if (!count)
    return std::unexpected(count.error());
  const bool current = facts.admission != "historical";
  if (admission_case(current, facts.verdict, *count, facts.max_rounds) != facts.admission)
    return std::unexpected(invalid());
  const bool admitted = facts.admission == "admitted";

  // Repair revisions are native: the previous lineage revision plus one, at the worker's current generation.
  struct planned_repair {
    std::string   role;
    std::uint64_t generation = 0;
    std::uint64_t ordinal    = 0;
    std::uint64_t revision   = 0;
  };
  std::vector<planned_repair> repairs;
  for (const auto& role : facts.repairs) {
    auto row = read_role_row(db, job, role);
    if (!row)
      return std::unexpected(row.error());
    if (!*row)
      return std::unexpected(conflict());
    auto previous = repair_revision(db, job, role, (*row)->generation);
    if (!previous)
      return std::unexpected(previous.error());
    auto ordinal = generation_ordinal_of(db, (*row)->generation);
    if (!ordinal)
      return std::unexpected(ordinal.error());
    repairs.push_back(planned_repair{role, (*row)->generation, *ordinal, *previous + 1});
  }

  // Blockers opened: none of the same kind may be open for the job unless this commit clears it (DB-R19).
  auto open = open_blockers(db, job);
  if (!open)
    return std::unexpected(open.error());
  std::vector<hist::verdict_result_blocker> opened;
  for (std::size_t i = 0; i < facts.blockers_opened.size(); ++i) {
    const auto& kind = facts.blockers_opened[i];
    const bool  held = std::ranges::any_of(*open, [&](const execution_blocker& blocker) {
      return blocker.kind == kind && std::ranges::find(change.blockers_cleared, blocker.id) == change.blockers_cleared.end();
    });
    if (held)
      return std::unexpected(conflict());
    opened.push_back(hist::verdict_result_blocker{decimal(*blocker_base + i), kind});
  }

  // Event payload v2 and the saved verdict_response_v1.
  hist::event_payload_v2 payload;
  payload.operation  = "verdict";
  payload.before     = hist::payload_position{decimal(*revision), job};
  payload.after      = hist::payload_position{decimal(*revision), job};
  payload.reason     = change.transitions.front().reason;
  payload.references = {{"body", decimal(verdict_body)}, {"run", decimal(facts.run)}, {"verdict", decimal(*verdict_id)}};
  auto payload_bytes = hist::encode_event_payload_v2("verdict_recorded", payload);
  if (!payload_bytes)
    return std::unexpected(invalid());

  hist::verdict_result_v1 result;
  result.ledger        = std::string(identity);
  result.request_id    = change.request_id;
  result.job           = job;
  result.role          = facts.role;
  result.revision      = decimal(*revision);
  result.run           = decimal(facts.run);
  result.event         = decimal(*seq);
  result.verdict_id    = decimal(*verdict_id);
  result.applicability = current ? "current" : "historical";
  result.decision      = facts.verdict.decision;
  result.admitted      = admitted;
  for (const auto& repair : repairs)
    result.repairs.push_back(hist::verdict_result_repair{repair.role, decimal(repair.ordinal), decimal(repair.revision)});
  result.count      = decimal(*count + (admitted ? 1 : 0));
  result.max_rounds = decimal(facts.max_rounds);
  result.opened     = opened;
  auto cleared      = change.blockers_cleared;
  std::ranges::sort(cleared);
  for (auto id : cleared)
    result.cleared.push_back(decimal(id));
  for (const auto& t : change.transitions)
    result.roles.push_back(hist::verdict_result_role{t.role, t.after_state, t.after_drifted});
  std::ranges::sort(result.roles, [](const auto& a, const auto& b) { return a.role < b.role; });
  auto saved = hist::encode_verdict_result_v1(result);
  if (!saved)
    return std::unexpected(invalid());

  const auto s = as_bind(*seq);
  if (auto ok = write_sql(db,
                          "INSERT INTO events(seq,type,version,revision_id,request_id,actor,occurred_at,payload,job,role,"
                          "generation_id,run_id) VALUES(?1,'verdict_recorded',2,?2,?3,?4,?5,?6,?7,?8,?9,?10)",
                          {s, as_bind(*revision), change.request_id, change.actor, std::string(now), blob_value{*payload_bytes},
                           job, facts.role, as_bind(reviewer->value().generation), as_bind(facts.run)});
      !ok)
    return std::unexpected(ok.error());
  if (auto ok = write_sql(db,
                          "INSERT INTO requests(request_id,operation,body_id,event_seq,revision_id,outcome,result,attachment_id) "
                          "VALUES(?1,'verdict',?2,?3,?4,'recorded',?5,NULL)",
                          {change.request_id, as_bind(request_body), s, as_bind(*revision), blob_value{*saved}});
      !ok)
    return std::unexpected(ok.error());
  if (auto ok = write_sql(
          db, "INSERT INTO verdicts(id,request_id,run_id,seq,body_id,applicability,decision) VALUES(?1,?2,?3,?4,?5,?6,?7)",
          {as_bind(*verdict_id), change.request_id, as_bind(facts.run), s, as_bind(verdict_body), result.applicability,
           facts.verdict.decision});
      !ok)
    return std::unexpected(ok.error());

  // Transitions: ordinal 1 the reviewer, then by role; each at the role's locked generation.
  std::vector<locked_role_row> befores;
  for (std::size_t i = 0; i < change.transitions.size(); ++i) {
    const auto& t   = change.transitions[i];
    auto        row = read_role_row(db, t.job, t.role);
    if (!row)
      return std::unexpected(row.error());
    if (!*row)
      return std::unexpected(conflict());
    befores.push_back(**row);
    if (auto ok =
            write_sql(db,
                      "INSERT INTO transitions(event_seq,ordinal,job,role,generation_id,before_state,after_state,"
                      "before_drifted,after_drifted,reason) VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9,?10)",
                      {s, static_cast<std::int64_t>(i + 1), t.job, t.role, as_bind((*row)->generation), (*row)->state,
                       t.after_state, std::int64_t{(*row)->drifted ? 1 : 0}, std::int64_t{t.after_drifted ? 1 : 0}, t.reason});
        !ok)
      return std::unexpected(ok.error());
  }
  for (const auto& repair : repairs)
    if (auto ok = write_sql(db, "INSERT INTO repairs(verdict_id,job,role,generation_id,revision) VALUES(?1,?2,?3,?4,?5)",
                            {as_bind(*verdict_id), job, repair.role, as_bind(repair.generation), as_bind(repair.revision)});
        !ok)
      return std::unexpected(ok.error());
  // Clear first, so a blocker cleared and one opened of the same kind never overlap (DB-R19).
  if (auto ok = clear_blockers(db, change.blockers_cleared, *seq, job); !ok)
    return std::unexpected(ok.error());
  for (std::size_t i = 0; i < facts.blockers_opened.size(); ++i) {
    const auto id = as_bind(*blocker_base + i);
    if (auto ok = write_sql(db, "INSERT INTO blockers(id,seq,job,kind,verdict_id) VALUES(?1,?2,?3,?4,?5)",
                            {id, s, job, facts.blockers_opened[i], as_bind(*verdict_id)});
        !ok)
      return std::unexpected(ok.error());
    if (auto ok =
            write_sql(db, "INSERT INTO blocker_state(blocker_id,state,cleared_seq,last_seq) VALUES(?1,'open',NULL,?2)", {id, s});
        !ok)
      return std::unexpected(ok.error());
  }
  // N11: the reviewer run completes. Then the role_state delta: transitioned roles bump fact_version (EX-3, D16).
  if (auto ok = write_sql(db, "UPDATE run_state SET state='completed', last_seq=?1 WHERE run_id=?2 AND state='active'",
                          {s, as_bind(facts.run)});
      !ok)
    return std::unexpected(ok.error());
  for (std::size_t i = 0; i < change.transitions.size(); ++i) {
    const auto& t = change.transitions[i];
    if (auto ok = write_sql(db,
                            "UPDATE role_state SET state=?1, drifted=0, run_id=NULL, fact_version=?2, last_seq=?3 "
                            "WHERE job=?4 AND role=?5",
                            {t.after_state, as_bind(befores[i].fact_version + 1), s, t.job, t.role});
        !ok)
      return std::unexpected(ok.error());
  }
  if (auto ok = write_sql(db, "UPDATE current_state SET last_seq=?1 WHERE singleton=1", {s}); !ok)
    return std::unexpected(ok.error());

  execution_commit_result out;
  out.replayed     = false;
  out.saved_result = *saved;
  out.event_seq    = *seq;
  return out;
}

std::expected<std::optional<value_ptr>, execution_error> read_verdict_expansion(const std::filesystem::path& path,
                                                                                std::uint64_t                event) {
  if (auto verified = verify_execution_ledger(path); !verified)
    return std::unexpected(verified.error());
  auto database = open_format3(path, execution_access::read_only);
  if (!database)
    return std::unexpected(database.error());
  auto& db = database->sql();

  struct event_image {
    std::string   type;
    std::uint64_t revision = 0;
    std::string   job;
    std::string   role;
    std::uint64_t generation = 0;
    std::uint64_t run        = 0;
  };
  std::optional<event_image> found;
  auto scan = each_row(db, "SELECT type,revision_id,job,role,generation_id,run_id FROM events WHERE seq=?1", {as_bind(event)},
                       [&](statement& row) -> checked {
                         auto type = text_col(row, 0);
                         if (!type)
                           return std::unexpected(type.error());
                         event_image image;
                         image.type = *type;
                         if (image.type != "verdict_recorded") {
                           found = image;
                           return {};
                         }
                         auto revision = id_col(row, 1), generation = id_col(row, 4), run = id_col(row, 5);
                         auto job = text_col(row, 2), role = text_col(row, 3);
                         if (!revision || !generation || !run || !job || !role)
                           return std::unexpected(corrupt());
                         image.revision   = *revision;
                         image.job        = *job;
                         image.role       = *role;
                         image.generation = *generation;
                         image.run        = *run;
                         found            = image;
                         return {};
                       });
  if (!scan)
    return std::unexpected(scan.error());
  if (!found || found->type != "verdict_recorded")
    return std::optional<value_ptr>{};

  // The verdict and its decoded normalized body.
  value_ptr     verdict;
  std::uint64_t verdict_id = 0;
  std::uint64_t body_id    = 0;
  std::string   request_id, applicability, decision;
  scan = each_row(db, "SELECT id,request_id,body_id,applicability,decision FROM verdicts WHERE seq=?1", {as_bind(event)},
                  [&](statement& row) -> checked {
                    auto id = id_col(row, 0), body = id_col(row, 2);
                    auto request = text_col(row, 1), app = text_col(row, 3), dec = text_col(row, 4);
                    if (!id || !body || !request || !app || !dec)
                      return std::unexpected(corrupt());
                    verdict_id    = *id;
                    body_id       = *body;
                    request_id    = *request;
                    applicability = *app;
                    decision      = *dec;
                    return {};
                  });
  if (!scan)
    return std::unexpected(scan.error());
  if (verdict_id == 0)
    return std::unexpected(corrupt());
  auto record = value_record(db, body_id);
  if (!record)
    return std::unexpected(record.error());
  auto body = hist::decode_verdict_v1(record->canonical);
  if (!body)
    return std::unexpected(corrupt());
  auto body_value = hist::verdict_value(*body);
  if (!body_value)
    return std::unexpected(corrupt());
  verdict = record_node({{"id", text_node(decimal(verdict_id))},
                         {"request_id", text_node(request_id)},
                         {"run", text_node(decimal(found->run))},
                         {"applicability", text_node(applicability)},
                         {"decision", text_node(decision)},
                         {"body", *body_value}});

  std::vector<value_ptr> repairs;
  scan = each_row(db,
                  "SELECT r.role,g.ordinal,r.revision FROM repairs r JOIN generations g ON g.id=r.generation_id "
                  "WHERE r.verdict_id=?1 ORDER BY r.role",
                  {as_bind(verdict_id)}, [&](statement& row) -> checked {
                    auto role    = text_col(row, 0);
                    auto ordinal = id_col(row, 1), revision = id_col(row, 2);
                    if (!role || !ordinal || !revision)
                      return std::unexpected(corrupt());
                    repairs.push_back(record_node({{"role", text_node(*role)},
                                                   {"generation", text_node(decimal(*ordinal))},
                                                   {"revision", text_node(decimal(*revision))}}));
                    return {};
                  });
  if (!scan)
    return std::unexpected(scan.error());
  std::vector<value_ptr> opened;
  scan = each_row(db, "SELECT id,kind FROM blockers WHERE verdict_id=?1 ORDER BY id", {as_bind(verdict_id)},
                  [&](statement& row) -> checked {
                    auto id   = id_col(row, 0);
                    auto kind = text_col(row, 1);
                    if (!id || !kind)
                      return std::unexpected(corrupt());
                    opened.push_back(record_node({{"id", text_node(decimal(*id))}, {"kind", text_node(*kind)}}));
                    return {};
                  });
  if (!scan)
    return std::unexpected(scan.error());
  std::vector<value_ptr> cleared;
  scan = each_row(db, "SELECT blocker_id FROM blocker_state WHERE state='cleared' AND cleared_seq=?1 ORDER BY blocker_id",
                  {as_bind(event)}, [&](statement& row) -> checked {
                    auto id = id_col(row, 0);
                    if (!id)
                      return std::unexpected(id.error());
                    cleared.push_back(text_node(decimal(*id)));
                    return {};
                  });
  if (!scan)
    return std::unexpected(scan.error());

  // Native provenance of the reviewer run (P2-5 native), derived from rows only. DRAFT (DB-R25): the recorded
  // interpretation is not read here, so preferences are unknown; base_label is the W base text.
  std::uint64_t source_id = 0;
  std::string   source_digest;
  scan = each_row(db, "SELECT r.source_id,c.digest FROM revisions r JOIN records c ON c.id=r.source_id WHERE r.id=?1",
                  {as_bind(found->revision)}, [&](statement& row) -> checked {
                    auto id     = id_col(row, 0);
                    auto digest = blob_col(row, 1);
                    if (!id || !digest)
                      return std::unexpected(corrupt());
                    source_id     = *id;
                    source_digest = std::format("{}{}", digest_prefix, hex_of(*digest));
                    return {};
                  });
  if (!scan)
    return std::unexpected(scan.error());
  if (source_id == 0)
    return std::unexpected(corrupt());
  std::uint64_t work_id = 0;
  std::string   work_fp, render_fp;
  scan = each_row(db, "SELECT work_id,work_fp,render_fp FROM dispatches WHERE run_id=?1 ORDER BY seq DESC, id DESC LIMIT 1",
                  {as_bind(found->run)}, [&](statement& row) -> checked {
                    auto id     = id_col(row, 0);
                    auto work   = blob_col(row, 1);
                    auto render = blob_col(row, 2);
                    if (!id || !work || !render)
                      return std::unexpected(corrupt());
                    work_id   = *id;
                    work_fp   = std::format("{}{}", digest_prefix, hex_of(*work));
                    render_fp = std::format("{}{}", digest_prefix, hex_of(*render));
                    return {};
                  });
  if (!scan)
    return std::unexpected(scan.error());
  if (work_id == 0)
    return std::unexpected(corrupt());
  auto work_record = value_record(db, work_id);
  if (!work_record)
    return std::unexpected(work_record.error());
  auto w = hist::decode_work_exec_v1(work_record->canonical);
  if (!w || !*w)
    return std::unexpected(corrupt());
  value_ptr  base_label = unknown_node();
  const auto base       = (*w)->fields.find("base");
  if (base != (*w)->fields.end() && base->second && base->second->kind == value_kind::string)
    base_label = text_node(base->second->string);
  auto ordinal = generation_ordinal_of(db, found->generation);
  if (!ordinal)
    return std::unexpected(ordinal.error());
  auto native =
      record_node({{"ledger", text_node(database->metadata().identity)},
                   {"revision", text_node(decimal(found->revision))},
                   {"source", record_node({{"id", text_node(decimal(source_id))}, {"digest", text_node(source_digest)}})},
                   {"job", text_node(found->job)},
                   {"role", text_node(found->role)},
                   {"generation", text_node(decimal(*ordinal))},
                   {"base_label", base_label},
                   {"work_fp", text_node(work_fp)},
                   {"render_fp", text_node(render_fp)},
                   {"preferences", record_node({{"model", unknown_node()}, {"session", unknown_node()}})}});
  return std::optional<value_ptr>(record_node(
      {{"verdict", verdict},
       {"repairs", list_node(std::move(repairs))},
       {"blockers", record_node({{"opened", list_node(std::move(opened))}, {"cleared", list_node(std::move(cleared))}})},
       {"provenance", record_node({{"native", native}})}}));
}

std::expected<std::optional<std::string>, execution_error> read_saved_result(const std::filesystem::path& path,
                                                                             std::string_view             request_id) {
  if (auto verified = verify_execution_ledger(path); !verified)
    return std::unexpected(verified.error());
  auto database = open_format3(path, execution_access::read_only);
  if (!database)
    return std::unexpected(database.error());
  std::optional<std::string> saved;
  auto scan = each_row(database->sql(), "SELECT result FROM requests WHERE request_id=?1", {std::string(request_id)},
                       [&](statement& row) -> checked {
                         auto bytes = blob_col(row, 0);
                         if (!bytes)
                           return std::unexpected(bytes.error());
                         saved = *bytes;
                         return {};
                       });
  if (!scan)
    return std::unexpected(scan.error());
  return saved;
}

} // namespace planar::engine::ledger
