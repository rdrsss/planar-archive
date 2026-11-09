// Format3 execution ledger: the shared cross-row verifier, the verified snapshot reader, exact request lookup, run-token
// authorization, the D24 state measure, and the locked commit protocol (stage_execution, commit_execution and the
// execution_write commit and abort operations).
module;
#include <sqlite3.h>
#include <sys/random.h>
module planar.engine.ledger.execution;

import std;
import planar.engine.canonical;
import planar.engine.hash;
import planar.engine.json_codec;
import planar.engine.value;
import planar.engine.json_codec;
import planar.engine.history.record;
import planar.engine.history.request_v2;
import planar.engine.history.request_v3;
import planar.engine.history.execution_codec;
import planar.engine.history.recorded_interpretation;
import planar.engine.history.tree_core;
import planar.engine.ledger.history_codec;
import planar.engine.ledger.sqlite;
import planar.engine.ledger.execution_schema;

namespace planar::engine::ledger {

namespace {

namespace hist = ::planar::engine::history;
using ::planar::engine::value;
using ::planar::engine::value_kind;
using ::planar::engine::value_ptr;

template <typename T> using value_result = std::expected<T, execution_error>;
using checked                            = std::expected<void, execution_error>;
using role_key                           = std::pair<std::string, std::string>;

constexpr std::uint64_t    snapshot_limit = 16777216;
constexpr std::string_view init_actor     = "planar:init";

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

// Accumulating reader of one result row: the first failed column access is kept and reported by finish().
class row_reader {
public:
  explicit row_reader(statement& row) : _row(row) {
  }

  std::int64_t number(int column) {
    auto value = _row.integer(column);
    if (!value) {
      fail(map_sqlite(value.error()));
      return 0;
    }
    if (!value->has_value()) {
      fail(corrupt());
      return 0;
    }
    return value->value();
  }

  std::optional<std::int64_t> optional_number(int column) {
    auto value = _row.integer(column);
    if (!value) {
      fail(map_sqlite(value.error()));
      return std::nullopt;
    }
    return *value;
  }

  std::uint64_t id(int column) {
    const auto value = number(column);
    if (value <= 0) {
      fail(corrupt());
      return 0;
    }
    return static_cast<std::uint64_t>(value);
  }

  std::optional<std::uint64_t> optional_id(int column) {
    const auto value = optional_number(column);
    if (!value)
      return std::nullopt;
    if (*value <= 0) {
      fail(corrupt());
      return std::nullopt;
    }
    return static_cast<std::uint64_t>(*value);
  }

  bool flag(int column) {
    const auto value = number(column);
    if (value != 0 && value != 1) {
      fail(corrupt());
      return false;
    }
    return value == 1;
  }

  std::optional<bool> optional_flag(int column) {
    const auto value = optional_number(column);
    if (!value)
      return std::nullopt;
    if (*value != 0 && *value != 1) {
      fail(corrupt());
      return std::nullopt;
    }
    return *value == 1;
  }

  std::string text(int column) {
    auto value = _row.text(column);
    if (!value) {
      fail(map_sqlite(value.error()));
      return std::string{};
    }
    if (!value->has_value()) {
      fail(corrupt());
      return std::string{};
    }
    return value->value();
  }

  std::optional<std::string> optional_text(int column) {
    auto value = _row.text(column);
    if (!value) {
      fail(map_sqlite(value.error()));
      return std::nullopt;
    }
    return *value;
  }

  std::string blob(int column) {
    auto value = _row.blob(column);
    if (!value) {
      fail(map_sqlite(value.error()));
      return std::string{};
    }
    if (!value->has_value()) {
      fail(corrupt());
      return std::string{};
    }
    return value->value();
  }

  std::optional<std::string> optional_blob(int column) {
    auto value = _row.blob(column);
    if (!value) {
      fail(map_sqlite(value.error()));
      return std::nullopt;
    }
    return *value;
  }

  checked finish() const {
    if (_error)
      return std::unexpected(*_error);
    return {};
  }

private:
  void fail(const execution_error& error) {
    if (!_error)
      _error = error;
  }

  statement&                     _row;
  std::optional<execution_error> _error;
};

// Run every row of one query through the visitor, which validates the row and reports the first failure.
template <typename Visit> checked for_each(connection& db, std::string_view sql, Visit&& visit) {
  auto query = db.prepare(sql);
  if (!query)
    return std::unexpected(map_sqlite(query.error()));
  while (true) {
    auto step = query->step();
    if (!step)
      return std::unexpected(map_sqlite(step.error()));
    if (*step == step_result::done)
      return {};
    row_reader reader(*query);
    if (auto visited = visit(reader); !visited)
      return std::unexpected(visited.error());
    if (auto finished = reader.finish(); !finished)
      return std::unexpected(finished.error());
  }
}

std::optional<std::uint64_t> parse_decimal(std::string_view text) {
  if (text.empty() || text.size() > 20 || text.front() == '0')
    return std::nullopt;
  std::uint64_t value = 0;
  for (char c : text) {
    if (c < '0' || c > '9')
      return std::nullopt;
    const auto digit = static_cast<std::uint64_t>(c - '0');
    if (value > (std::numeric_limits<std::uint64_t>::max() - digit) / 10)
      return std::nullopt;
    value = value * 10 + digit;
  }
  return value;
}

// Position text of a payload: none or a canonical positive decimal revision identifier.
value_result<std::optional<std::uint64_t>> parse_position(std::string_view text) {
  if (text == "none")
    return std::optional<std::uint64_t>{};
  auto parsed = parse_decimal(text);
  if (!parsed)
    return std::unexpected(corrupt());
  return parsed;
}

std::optional<unsigned> hex_nibble(char c) {
  if (c >= '0' && c <= '9')
    return static_cast<unsigned>(c - '0');
  if (c >= 'a' && c <= 'f')
    return static_cast<unsigned>(c - 'a' + 10);
  return std::nullopt;
}

// Portable digest text, xxh3_128:<32 lowercase hex>, to the sixteen stored digest bytes.
std::optional<std::string> digest_bytes_of_text(std::string_view text) {
  const auto colon = text.rfind(':');
  if (colon == std::string_view::npos)
    return std::nullopt;
  const auto hex = text.substr(colon + 1);
  if (hex.size() != 32)
    return std::nullopt;
  std::string out;
  out.reserve(16);
  for (std::size_t i = 0; i < hex.size(); i += 2) {
    const auto high = hex_nibble(hex[i]);
    const auto low  = hex_nibble(hex[i + 1]);
    if (!high || !low)
      return std::nullopt;
    out += static_cast<char>((*high << 4) | *low);
  }
  return out;
}

std::string digest_bytes(const hist::record_digest& digest) {
  return {reinterpret_cast<const char*>(digest.data()), digest.size()};
}

std::string hex_of(std::string_view bytes) {
  constexpr std::string_view digits = "0123456789abcdef";
  std::string                out;
  out.reserve(bytes.size() * 2);
  for (char byte : bytes) {
    const auto value = static_cast<unsigned char>(byte);
    out += digits[value >> 4];
    out += digits[value & 15];
  }
  return out;
}

value_ptr make_node(value&& node) {
  return std::make_shared<const value>(std::move(node));
}

value_ptr text_node(std::string_view text) {
  value node{};
  node.kind   = value_kind::string;
  node.string = std::string(text);
  return make_node(std::move(node));
}

value_ptr integer_node(std::int64_t n) {
  value node{};
  node.kind    = value_kind::integer;
  node.integer = n;
  return make_node(std::move(node));
}

value_ptr bool_node(bool flag) {
  value node{};
  node.kind    = value_kind::boolean;
  node.boolean = flag;
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

value_ptr texts_node(const std::vector<std::string>& items) {
  std::vector<value_ptr> nodes;
  nodes.reserve(items.size());
  for (const auto& item : items)
    nodes.push_back(text_node(item));
  return list_node(std::move(nodes));
}

// Stored row images. Each table keeps the columns the verifier and the snapshot read.

struct record_entry {
  std::int64_t kind  = 0;   ///< 1 file, 2 directory, 3 value, 4 request.
  std::int64_t codec = 0;   ///< Stored codec column.
  std::string  digest;      ///< Sixteen canonical digest bytes.
  std::string  encoded;     ///< Stored frame bytes.
  std::string  value_codec; ///< Kind 3 only: canonical_v1 or one execution codec identifier.
  std::string  canonical;   ///< Kind 3 only: canonical_v1 bytes of the typed value.
};

struct event_entry {
  std::uint64_t                seq = 0;
  std::string                  type;
  std::int64_t                 version = 0;
  std::optional<std::uint64_t> revision;
  std::optional<std::string>   request_id;
  std::string                  actor;
  std::string                  occurred_at;
  std::string                  payload;
  std::optional<std::string>   job;
  std::optional<std::string>   role;
  std::optional<std::uint64_t> generation;
  std::optional<std::uint64_t> run;
};

struct revision_entry {
  std::uint64_t                id = 0;
  std::optional<std::uint64_t> parent;
  std::uint64_t                source         = 0;
  std::uint64_t                registry       = 0;
  std::uint64_t                stdlib         = 0;
  std::uint64_t                interpretation = 0;
  std::uint64_t                versions       = 0;
  std::string                  actor;
  std::string                  accepted_at;
};

struct generation_entry {
  std::uint64_t id = 0;
  std::string   job;
  std::string   role;
  std::uint64_t ordinal    = 0;
  std::uint64_t structure  = 0;
  std::uint64_t opened_seq = 0;
  std::string   reason;
};

struct activation_entry {
  std::uint64_t id = 0;
  std::string   job;
  std::uint64_t revision  = 0;
  std::uint64_t event_seq = 0;
};

struct run_entry {
  std::uint64_t                id         = 0;
  std::uint64_t                generation = 0;
  std::string                  request_id;
  std::uint64_t                claimed_seq = 0;
  std::string                  secret;
  std::optional<std::uint64_t> provenance;
};

struct dispatch_entry {
  std::uint64_t                id       = 0;
  std::uint64_t                run      = 0;
  std::uint64_t                seq      = 0;
  std::uint64_t                revision = 0;
  std::string                  kind;
  std::uint64_t                packet   = 0;
  std::uint64_t                manifest = 0;
  std::uint64_t                work     = 0;
  std::string                  work_fp;
  std::string                  render_fp;
  std::optional<std::uint64_t> checkpoint;
};

struct request_entry {
  std::string                  request_id;
  std::string                  operation;
  std::uint64_t                body      = 0;
  std::uint64_t                event_seq = 0;
  std::uint64_t                revision  = 0;
  std::string                  outcome;
  std::string                  result;
  std::optional<std::uint64_t> attachment;
};

struct role_row {
  std::string                  job;
  std::string                  role;
  std::uint64_t                generation = 0;
  std::string                  state;
  bool                         drifted = false;
  std::optional<std::uint64_t> run;
  std::uint64_t                fact_version = 0;
  std::uint64_t                last_seq     = 0;
};

struct run_state_row {
  std::uint64_t run = 0;
  std::string   job;
  std::string   role;
  std::uint64_t generation = 0;
  std::string   state;
  std::uint64_t last_seq = 0;
};

struct edge_row {
  std::uint64_t ordinal = 0;
  std::string   name;
  std::uint64_t child = 0;
};

struct current_row {
  std::optional<std::uint64_t> revision;
  std::uint64_t                last_seq = 0;
  std::optional<std::string>   job;
  std::optional<std::uint64_t> activation;
};

// Every table of one ledger read through one connection, plus the lookups the verifier builds from them.
struct ledger_rows {
  std::map<std::uint64_t, record_entry>          records;
  std::vector<event_entry>                       events;
  std::vector<std::vector<hist::transition_row>> transitions;      ///< Indexed by sequence minus one.
  std::vector<std::optional<std::string>>        request_of_event; ///< Indexed by sequence minus one.
  std::map<std::uint64_t, revision_entry>        revisions;
  std::map<std::uint64_t, generation_entry>      generations;
  std::vector<activation_entry>                  activations;
  std::map<std::uint64_t, run_entry>             runs;
  std::map<std::uint64_t, dispatch_entry>        dispatches;
  std::map<std::uint64_t, retained_checkpoint>   checkpoints;
  std::map<std::uint64_t, std::uint64_t>         checkpoint_bodies;
  std::map<std::uint64_t, std::uint64_t>         dispatch_by_run;
  std::map<std::string, request_entry>           requests;
  std::vector<role_row>                          role_states;
  std::vector<run_state_row>                     run_states;
  std::optional<current_row>                     current;
  std::map<std::uint64_t, std::vector<edge_row>> edges; ///< tree_edges by parent, in ordinal order.
};

// ---------------------------------------------------------------------------------------------------------------------
// Table loading. Every loader reads one complete table in its stored order and rejects impossible column values.

checked load_reserved_empty(connection& db) {
  // R16-R18 tables have dedicated readers below.
  constexpr std::array<std::string_view, 0> reserved{};
  for (auto table : reserved) {
    const auto sql  = std::format("SELECT COUNT(*) FROM {}", table);
    bool       any  = false;
    auto       scan = for_each(db, sql, [&](row_reader& row) -> checked {
      any = row.number(0) != 0;
      return {};
    });
    if (!scan)
      return scan;
    if (any)
      return std::unexpected(corrupt());
  }
  return {};
}

// Validate one stored record against its kind and codec, and keep the canonical value bytes of kind 3 records.
value_result<record_entry> validate_record(std::int64_t kind, std::int64_t codec, std::string digest, std::string encoded) {
  if (digest.size() != 16)
    return std::unexpected(corrupt());
  record_entry entry;
  entry.kind    = kind;
  entry.codec   = codec;
  entry.digest  = std::move(digest);
  entry.encoded = std::move(encoded);
  if (kind == 1 || kind == 2) {
    if (codec != 1)
      return std::unexpected(corrupt());
    auto decoded = hist::decode_record(entry.encoded);
    if (!decoded)
      return std::unexpected(corrupt());
    const bool shaped = kind == 1 ? std::holds_alternative<hist::file_record>(*decoded)
                                  : std::holds_alternative<hist::directory_record>(*decoded);
    if (!shaped)
      return std::unexpected(corrupt());
    auto portable = hist::digest_record(*decoded);
    if (!portable || digest_bytes(*portable) != entry.digest)
      return std::unexpected(corrupt());
    return entry;
  }
  if (kind == 3) {
    if (codec != 1)
      return std::unexpected(corrupt());
    if (auto exec = hist::decode_execution_value_record(entry.encoded); exec) {
      auto portable = hist::digest_execution_value_record(entry.encoded);
      if (!portable || digest_bytes(*portable) != entry.digest)
        return std::unexpected(corrupt());
      entry.value_codec = exec->codec;
      entry.canonical   = exec->encoded;
      return entry;
    }
    auto decoded = hist::decode_record(entry.encoded);
    if (!decoded)
      return std::unexpected(corrupt());
    const auto* value = std::get_if<hist::value_record>(&*decoded);
    if (!value || value->codec != "canonical_v1")
      return std::unexpected(corrupt());
    auto portable = hist::digest_record(*decoded);
    if (!portable || digest_bytes(*portable) != entry.digest)
      return std::unexpected(corrupt());
    entry.value_codec = "canonical_v1";
    entry.canonical   = value->encoded;
    return entry;
  }
  if (kind == 4) {
    if (codec == 2) {
      auto request = hist::decode_request_v2(entry.encoded);
      if (!request)
        return std::unexpected(corrupt());
      auto portable = hist::digest_request_v2(*request);
      if (!portable || digest_bytes(*portable) != entry.digest)
        return std::unexpected(corrupt());
      return entry;
    }
    if (codec == 3) {
      auto request = hist::decode_request_v3(entry.encoded);
      if (!request)
        return std::unexpected(corrupt());
      auto portable = hist::digest_request_v3(*request);
      if (!portable || digest_bytes(*portable) != entry.digest)
        return std::unexpected(corrupt());
      return entry;
    }
  }
  return std::unexpected(corrupt());
}

checked load_records(connection& db, ledger_rows& rows) {
  return for_each(db, "SELECT id,kind,codec,digest,encoded FROM records ORDER BY id", [&](row_reader& row) -> checked {
    const auto id      = row.id(0);
    const auto kind    = row.number(1);
    const auto codec   = row.number(2);
    auto       digest  = row.blob(3);
    auto       encoded = row.blob(4);
    if (auto finished = row.finish(); !finished)
      return finished;
    auto entry = validate_record(kind, codec, std::move(digest), std::move(encoded));
    if (!entry)
      return std::unexpected(entry.error());
    rows.records.emplace(id, std::move(*entry));
    return {};
  });
}

checked load_tree_edges(connection& db, ledger_rows& rows) {
  return for_each(db, "SELECT parent_id,ordinal,name,child_id FROM tree_edges ORDER BY parent_id,ordinal",
                  [&](row_reader& row) -> checked {
                    const auto parent  = row.id(0);
                    const auto ordinal = row.number(1);
                    auto       name    = row.text(2);
                    const auto child   = row.id(3);
                    if (auto finished = row.finish(); !finished)
                      return finished;
                    if (ordinal < 0)
                      return std::unexpected(corrupt());
                    rows.edges[parent].push_back(edge_row{static_cast<std::uint64_t>(ordinal), std::move(name), child});
                    return {};
                  });
}

checked load_events(connection& db, ledger_rows& rows) {
  return for_each(db,
                  "SELECT seq,type,version,revision_id,request_id,actor,occurred_at,payload,job,role,generation_id,run_id "
                  "FROM events ORDER BY seq",
                  [&](row_reader& row) -> checked {
                    event_entry entry;
                    entry.seq         = row.id(0);
                    entry.type        = row.text(1);
                    entry.version     = row.number(2);
                    entry.revision    = row.optional_id(3);
                    entry.request_id  = row.optional_text(4);
                    entry.actor       = row.text(5);
                    entry.occurred_at = row.text(6);
                    entry.payload     = row.blob(7);
                    entry.job         = row.optional_text(8);
                    entry.role        = row.optional_text(9);
                    entry.generation  = row.optional_id(10);
                    entry.run         = row.optional_id(11);
                    if (auto finished = row.finish(); !finished)
                      return finished;
                    if (entry.seq != rows.events.size() + 1)
                      return std::unexpected(corrupt());
                    rows.events.push_back(std::move(entry));
                    return {};
                  });
}

checked load_transitions(connection& db, ledger_rows& rows) {
  rows.transitions.assign(rows.events.size(), {});
  return for_each(db,
                  "SELECT event_seq,ordinal,job,role,generation_id,before_state,after_state,before_drifted,after_drifted,reason "
                  "FROM transitions ORDER BY event_seq,ordinal",
                  [&](row_reader& row) -> checked {
                    const auto           seq = row.id(0);
                    hist::transition_row entry;
                    entry.ordinal        = static_cast<std::uint64_t>(row.number(1));
                    entry.job            = row.text(2);
                    entry.role           = row.text(3);
                    entry.generation_id  = row.id(4);
                    entry.before_state   = row.optional_text(5);
                    entry.before_drifted = row.optional_flag(7);
                    entry.after_state    = row.text(6);
                    entry.after_drifted  = row.flag(8);
                    entry.reason         = row.text(9);
                    if (auto finished = row.finish(); !finished)
                      return finished;
                    if (seq > rows.events.size())
                      return std::unexpected(corrupt());
                    rows.transitions[seq - 1].push_back(std::move(entry));
                    return {};
                  });
}

checked load_revisions(connection& db, ledger_rows& rows) {
  return for_each(db,
                  "SELECT id,parent_id,source_id,registry_id,stdlib_id,interpretation_id,versions_id,actor,accepted_at "
                  "FROM revisions ORDER BY id",
                  [&](row_reader& row) -> checked {
                    revision_entry entry;
                    entry.id             = row.id(0);
                    entry.parent         = row.optional_id(1);
                    entry.source         = row.id(2);
                    entry.registry       = row.id(3);
                    entry.stdlib         = row.id(4);
                    entry.interpretation = row.id(5);
                    entry.versions       = row.id(6);
                    entry.actor          = row.text(7);
                    entry.accepted_at    = row.text(8);
                    if (auto finished = row.finish(); !finished)
                      return finished;
                    if (entry.id != rows.revisions.size() + 1)
                      return std::unexpected(corrupt());
                    rows.revisions.emplace(entry.id, std::move(entry));
                    return {};
                  });
}

checked load_generations(connection& db, ledger_rows& rows) {
  return for_each(db, "SELECT id,job,role,ordinal,structure_id,opened_seq,reason FROM generations ORDER BY id",
                  [&](row_reader& row) -> checked {
                    generation_entry entry;
                    entry.id         = row.id(0);
                    entry.job        = row.text(1);
                    entry.role       = row.text(2);
                    entry.ordinal    = row.id(3);
                    entry.structure  = row.id(4);
                    entry.opened_seq = row.id(5);
                    entry.reason     = row.text(6);
                    if (auto finished = row.finish(); !finished)
                      return finished;
                    rows.generations.emplace(entry.id, std::move(entry));
                    return {};
                  });
}

checked load_activations(connection& db, ledger_rows& rows) {
  return for_each(db, "SELECT id,job,revision_id,event_seq FROM job_activations ORDER BY id", [&](row_reader& row) -> checked {
    activation_entry entry;
    entry.id        = row.id(0);
    entry.job       = row.text(1);
    entry.revision  = row.id(2);
    entry.event_seq = row.id(3);
    if (auto finished = row.finish(); !finished)
      return finished;
    if (entry.id != rows.activations.size() + 1)
      return std::unexpected(corrupt());
    rows.activations.push_back(std::move(entry));
    return {};
  });
}

checked load_runs(connection& db, ledger_rows& rows) {
  return for_each(db, "SELECT id,generation_id,request_id,claimed_seq,secret,provenance_id FROM runs ORDER BY id",
                  [&](row_reader& row) -> checked {
                    run_entry entry;
                    entry.id          = row.id(0);
                    entry.generation  = row.id(1);
                    entry.request_id  = row.text(2);
                    entry.claimed_seq = row.id(3);
                    entry.secret      = row.blob(4);
                    entry.provenance  = row.optional_id(5);
                    if (auto finished = row.finish(); !finished)
                      return finished;
                    if (entry.secret.size() != 16 || entry.id != rows.runs.size() + 1)
                      return std::unexpected(corrupt());
                    rows.runs.emplace(entry.id, std::move(entry));
                    return {};
                  });
}

checked load_dispatches(connection& db, ledger_rows& rows) {
  return for_each(db,
                  "SELECT id,run_id,seq,revision_id,kind,packet_id,manifest_id,work_id,work_fp,render_fp,checkpoint_id "
                  "FROM dispatches ORDER BY id",
                  [&](row_reader& row) -> checked {
                    dispatch_entry entry;
                    entry.id         = row.id(0);
                    entry.run        = row.id(1);
                    entry.seq        = row.id(2);
                    entry.revision   = row.id(3);
                    entry.kind       = row.text(4);
                    entry.packet     = row.id(5);
                    entry.manifest   = row.id(6);
                    entry.work       = row.id(7);
                    entry.work_fp    = row.blob(8);
                    entry.render_fp  = row.blob(9);
                    entry.checkpoint = row.optional_id(10);
                    if (auto finished = row.finish(); !finished)
                      return finished;
                    if (entry.work_fp.size() != 16 || entry.render_fp.size() != 16)
                      return std::unexpected(corrupt());
                    if (entry.id != rows.dispatches.size() + 1)
                      return std::unexpected(corrupt());
                    rows.dispatch_by_run[entry.run] = entry.id;
                    rows.dispatches.emplace(entry.id, std::move(entry));
                    return {};
                  });
}

checked load_requests(connection& db, ledger_rows& rows) {
  rows.request_of_event.assign(rows.events.size(), std::nullopt);
  return for_each(db,
                  "SELECT request_id,operation,body_id,event_seq,revision_id,outcome,result,attachment_id FROM requests "
                  "ORDER BY event_seq",
                  [&](row_reader& row) -> checked {
                    request_entry entry;
                    entry.request_id = row.text(0);
                    entry.operation  = row.text(1);
                    entry.body       = row.id(2);
                    entry.event_seq  = row.id(3);
                    entry.revision   = row.id(4);
                    entry.outcome    = row.text(5);
                    entry.result     = row.blob(6);
                    entry.attachment = row.optional_id(7);
                    if (auto finished = row.finish(); !finished)
                      return finished;
                    if (entry.event_seq > rows.events.size())
                      return std::unexpected(corrupt());
                    if (rows.request_of_event[entry.event_seq - 1])
                      return std::unexpected(corrupt());
                    rows.request_of_event[entry.event_seq - 1] = entry.request_id;
                    rows.requests.emplace(entry.request_id, std::move(entry));
                    return {};
                  });
}

checked load_role_states(connection& db, ledger_rows& rows) {
  return for_each(db,
                  "SELECT job,role,generation_id,state,drifted,run_id,fact_version,last_seq FROM role_state ORDER BY job,role",
                  [&](row_reader& row) -> checked {
                    role_row entry;
                    entry.job          = row.text(0);
                    entry.role         = row.text(1);
                    entry.generation   = row.id(2);
                    entry.state        = row.text(3);
                    entry.drifted      = row.flag(4);
                    entry.run          = row.optional_id(5);
                    entry.fact_version = static_cast<std::uint64_t>(row.number(6));
                    entry.last_seq     = row.id(7);
                    if (auto finished = row.finish(); !finished)
                      return finished;
                    rows.role_states.push_back(std::move(entry));
                    return {};
                  });
}

checked load_run_states(connection& db, ledger_rows& rows) {
  return for_each(db, "SELECT run_id,job,role,generation_id,state,last_seq FROM run_state ORDER BY run_id",
                  [&](row_reader& row) -> checked {
                    run_state_row entry;
                    entry.run        = row.id(0);
                    entry.job        = row.text(1);
                    entry.role       = row.text(2);
                    entry.generation = row.id(3);
                    entry.state      = row.text(4);
                    entry.last_seq   = row.id(5);
                    if (auto finished = row.finish(); !finished)
                      return finished;
                    rows.run_states.push_back(std::move(entry));
                    return {};
                  });
}

checked load_current(connection& db, ledger_rows& rows) {
  return for_each(db, "SELECT singleton,active_revision_id,last_seq,active_job,active_activation_id FROM current_state",
                  [&](row_reader& row) -> checked {
                    const auto  singleton = row.number(0);
                    current_row entry;
                    entry.revision   = row.optional_id(1);
                    entry.last_seq   = row.id(2);
                    entry.job        = row.optional_text(3);
                    entry.activation = row.optional_id(4);
                    if (auto finished = row.finish(); !finished)
                      return finished;
                    if (singleton != 1 || rows.current)
                      return std::unexpected(corrupt());
                    rows.current = std::move(entry);
                    return {};
                  });
}

value_result<const record_entry*> record_of(const ledger_rows& rows, std::uint64_t id, std::int64_t kind) {
  const auto found = rows.records.find(id);
  if (found == rows.records.end() || found->second.kind != kind)
    return std::unexpected(corrupt());
  return &found->second;
}

// A kind 3 record of one execution codec or canonical_v1, selected by its identifier.
value_result<const record_entry*> value_of(const ledger_rows& rows, std::uint64_t id, std::string_view codec) {
  auto found = record_of(rows, id, 3);
  if (!found)
    return found;
  if ((*found)->value_codec != codec)
    return std::unexpected(corrupt());
  return found;
}

// ---------------------------------------------------------------------------------------------------------------------
// Replay of the event log. The replay derives every role row, run row and active-run index from the immutable facts.

struct role_track {
  std::uint64_t                generation = 0;
  std::string                  state;
  bool                         drifted = false;
  std::optional<std::uint64_t> run;
  std::uint64_t                fact_version = 0;
  std::uint64_t                last_seq     = 0;
};

struct run_track {
  std::string   job;
  std::string   role;
  std::uint64_t generation = 0;
  std::string   state; ///< active or fenced in this round.
  std::uint64_t last_seq = 0;
};

struct replay_state {
  std::optional<std::uint64_t>           head;
  std::string                            active_job;
  std::optional<std::uint64_t>           activation;
  std::size_t                            activations_seen = 0;
  std::map<role_key, role_track>         roles;
  std::map<role_key, std::uint64_t>      active_runs;
  std::map<std::uint64_t, run_track>     runs;
  std::map<std::uint64_t, std::uint64_t> accepted;
};

std::optional<std::string> operation_of(std::string_view type) {
  if (type == "declaration_accepted" || type == "load_noop")
    return "load";
  if (type == "job_activated")
    return "activate";
  if (type == "run_claimed")
    return "claim";
  if (type == "run_taken_over")
    return "takeover";
  if (type == "check_reserved" || type == "check_recorded")
    return "check";
  if (type == "check_interrupted")
    return "check-recover";
  if (type == "verdict_recorded") // R18:
    return "verdict";
  if (type == "run_refreshed")
    return "refresh";
  if (type == "checkpoint_recorded")
    return "checkpoint";
  if (type == "run_released" || type == "run_interrupted")
    return "release";
  return std::nullopt;
}

std::optional<std::string> outcome_of(std::string_view type) {
  if (type == "declaration_accepted")
    return "accepted";
  if (type == "load_noop")
    return "noop";
  if (type == "job_activated")
    return "activated";
  if (type == "run_claimed")
    return "claimed";
  if (type == "run_taken_over")
    return "taken_over";
  if (type == "check_reserved" || type == "check_recorded")
    return "recorded";
  if (type == "check_interrupted")
    return "interrupted";
  if (type == "verdict_recorded") // R18:
    return "recorded";
  if (type == "run_refreshed")
    return "refreshed";
  if (type == "checkpoint_recorded")
    return "recorded";
  if (type == "run_released")
    return "released";
  if (type == "run_interrupted")
    return "interrupted";
  return std::nullopt;
}

// Event types whose committed change increments fact_version of every role it transitions (EX-3 fact versions).
bool bumps_fact(std::string_view type) {
  return type == "declaration_accepted" || type == "job_activated" || type == "run_taken_over" || type == "check_recorded" ||
         type == "verdict_recorded"; // R18: verdict_recorded is a bumping event (R18-4).
}

// Typed event columns per EX-4: job, role, generation and run are present exactly as the event type requires.
bool typed_columns_ok(const event_entry& event) {
  const bool job        = event.job.has_value();
  const bool role       = event.role.has_value();
  const bool generation = event.generation.has_value();
  const bool run        = event.run.has_value();
  if (event.type == "declaration_accepted" || event.type == "load_noop")
    return !job && !role && !generation && !run;
  if (event.type == "job_activated")
    return job && !role && !generation && !run;
  if (event.type == "run_claimed" || event.type == "run_taken_over" || event.type == "verdict_recorded" ||
      event.type == "check_reserved" || event.type == "check_recorded" || event.type == "check_interrupted" ||
      event.type == "run_refreshed" || event.type == "checkpoint_recorded" || event.type == "run_released" ||
      event.type == "run_interrupted")
    return job && role && generation && run;
  return false;
}

std::string ref_of(const std::map<std::string, std::string>& references, std::string_view key) {
  const auto found = references.find(std::string(key));
  return found == references.end() ? std::string{} : found->second;
}

bool has_ref(const std::map<std::string, std::string>& references, std::string_view key) {
  return references.contains(std::string(key));
}

// Two records are the same stored record when their kinds, codecs and encoded bytes agree (identical content, D22 c).
bool same_stored_record(const ledger_rows& rows, std::uint64_t left, std::uint64_t right) {
  if (left == right)
    return true;
  const auto a = rows.records.find(left);
  const auto b = rows.records.find(right);
  if (a == rows.records.end() || b == rows.records.end())
    return false;
  return a->second.kind == b->second.kind && a->second.codec == b->second.codec && a->second.encoded == b->second.encoded;
}

bool identical_revisions(const ledger_rows& rows, const revision_entry& a, const revision_entry& b) {
  return same_stored_record(rows, a.source, b.source) && same_stored_record(rows, a.registry, b.registry) &&
         same_stored_record(rows, a.stdlib, b.stdlib) && same_stored_record(rows, a.interpretation, b.interpretation) &&
         same_stored_record(rows, a.versions, b.versions);
}

// The five references of a load event, compared with the ids of one revision row.
bool references_match(const std::map<std::string, std::string>& references, const revision_entry& revision) {
  return ref_of(references, "source") == std::to_string(revision.source) &&
         ref_of(references, "registry") == std::to_string(revision.registry) &&
         ref_of(references, "stdlib") == std::to_string(revision.stdlib) &&
         ref_of(references, "interpretation") == std::to_string(revision.interpretation) &&
         ref_of(references, "versions") == std::to_string(revision.versions);
}

const revision_entry* revision_of(const ledger_rows& rows, std::uint64_t id) {
  const auto found = rows.revisions.find(id);
  return found == rows.revisions.end() ? nullptr : &found->second;
}

const generation_entry* generation_of(const ledger_rows& rows, std::uint64_t id) {
  const auto found = rows.generations.find(id);
  return found == rows.generations.end() ? nullptr : &found->second;
}

checked check_initialization(const ledger_rows& rows, const event_entry& event, const execution_metadata& metadata) {
  if (event.type != "ledger_initialized" || event.version != 1 || event.revision || event.request_id ||
      event.actor != init_actor || event.occurred_at != metadata.created_at || event.job || event.role || event.generation ||
      event.run || rows.request_of_event[0])
    return std::unexpected(corrupt());
  auto payload = decode_history_event(event.payload);
  if (!payload || *payload != history_event{std::nullopt, std::nullopt, event_reason::initialized, std::nullopt})
    return std::unexpected(corrupt());
  return {};
}

// Claim and takeover: the run row, its dispatch and references, and for a takeover the fencing of the previous run.
checked apply_run_event(const ledger_rows& rows, const event_entry& event, const hist::event_payload_v2& payload,
                        const std::vector<hist::transition_row>& transitions, replay_state& state) {
  const auto run_id = *event.run;
  const auto key    = role_key{*event.job, *event.role};
  const auto run    = rows.runs.find(run_id);
  if (run == rows.runs.end() || run->second.claimed_seq != event.seq || run->second.request_id != *event.request_id ||
      run->second.generation != *event.generation)
    return std::unexpected(corrupt());
  const auto* generation = generation_of(rows, *event.generation);
  if (!generation || generation->job != *event.job || generation->role != *event.role)
    return std::unexpected(corrupt());
  if (transitions.empty() || transitions[0].job != *event.job || transitions[0].role != *event.role ||
      transitions[0].generation_id != *event.generation || transitions[0].after_state != "running")
    return std::unexpected(corrupt());
  const auto dispatch_id = parse_decimal(ref_of(payload.references, "dispatch"));
  if (!dispatch_id || !rows.dispatches.contains(*dispatch_id))
    return std::unexpected(corrupt());
  const auto& dispatch = rows.dispatches.at(*dispatch_id);
  const bool  claim    = event.type == "run_claimed";
  if (dispatch.seq != event.seq || dispatch.revision != *event.revision || dispatch.kind != (claim ? "claim" : "takeover"))
    return std::unexpected(corrupt());
  const auto& references = payload.references;
  if (ref_of(references, "run") != std::to_string(run_id) || ref_of(references, "dispatch") != std::to_string(dispatch.id) ||
      ref_of(references, "packet") != std::to_string(dispatch.packet) ||
      ref_of(references, "manifest") != std::to_string(dispatch.manifest) ||
      ref_of(references, "work") != std::to_string(dispatch.work))
    return std::unexpected(corrupt());
  if (run->second.provenance.has_value() != has_ref(references, "provenance"))
    return std::unexpected(corrupt());
  if (run->second.provenance && ref_of(references, "provenance") != std::to_string(*run->second.provenance))
    return std::unexpected(corrupt());
  if (claim) {
    if (has_ref(references, "previous_run") || state.active_runs.contains(key))
      return std::unexpected(corrupt());
  } else {
    const auto previous = parse_decimal(ref_of(references, "previous_run"));
    const auto active   = state.active_runs.find(key);
    if (!previous || active == state.active_runs.end() || active->second != *previous)
      return std::unexpected(corrupt());
    auto fenced = state.runs.find(*previous);
    if (fenced == state.runs.end() || fenced->second.state != "active")
      return std::unexpected(corrupt());
    fenced->second.state    = "fenced";
    fenced->second.last_seq = event.seq;
    state.active_runs.erase(active);
  }
  if (state.runs.contains(run_id))
    return std::unexpected(corrupt());
  state.runs[run_id]     = run_track{*event.job, *event.role, *event.generation, "active", event.seq};
  state.active_runs[key] = run_id;
  return {};
}

// R18: a verdict_recorded event completes the reviewer's active run (N11): the run is the role's active run of the
// event's generation, ordinal 1 is that role leaving running, and the payload names the run. The row-level checks of
// invariants 21 to 27 are in execution_review.cpp.
checked apply_verdict_event(const event_entry& event, const hist::event_payload_v2& payload,
                            const std::vector<hist::transition_row>& transitions, const replay_state& state) {
  const auto key    = role_key{*event.job, *event.role};
  const auto active = state.active_runs.find(key);
  if (active == state.active_runs.end() || active->second != *event.run)
    return std::unexpected(corrupt());
  const auto run = state.runs.find(*event.run);
  if (run == state.runs.end() || run->second.state != "active" || run->second.generation != *event.generation)
    return std::unexpected(corrupt());
  if (transitions.empty() || transitions[0].job != *event.job || transitions[0].role != *event.role ||
      transitions[0].generation_id != *event.generation || !transitions[0].before_state ||
      *transitions[0].before_state != "running" || transitions[0].after_state == "running")
    return std::unexpected(corrupt());
  if (ref_of(payload.references, "run") != std::to_string(*event.run))
    return std::unexpected(corrupt());
  return {};
}

// Apply the transition rows of one event to the role replay, deriving each running role's run and its fencing.
checked apply_transitions(const ledger_rows& rows, const event_entry& event, const std::vector<hist::transition_row>& transitions,
                          replay_state& state) {
  const bool         carries_target = event.type == "run_claimed" || event.type == "run_taken_over";
  const bool         bumping        = bumps_fact(event.type);
  std::set<role_key> touched;
  for (const auto& transition : transitions) {
    const auto key = role_key{transition.job, transition.role};
    if (!touched.insert(key).second)
      return std::unexpected(corrupt());
    const auto* generation = generation_of(rows, transition.generation_id);
    if (!generation || generation->job != transition.job || generation->role != transition.role)
      return std::unexpected(corrupt());
    const auto found   = state.roles.find(key);
    const bool created = found == state.roles.end();
    if (created) {
      if (transition.before_state || transition.before_drifted)
        return std::unexpected(corrupt());
    } else {
      if (!transition.before_state || *transition.before_state != found->second.state)
        return std::unexpected(corrupt());
      if (!transition.before_drifted || *transition.before_drifted != found->second.drifted)
        return std::unexpected(corrupt());
    }
    const bool                   target = carries_target && transition.ordinal == 1;
    std::optional<std::uint64_t> derived;
    if (target) {
      derived = event.run;
    } else if (transition.after_state == "running") {
      if (created || found->second.state != "running" || !found->second.run)
        return std::unexpected(corrupt());
      derived = found->second.run;
    }
    if (!created && found->second.state == "running" && transition.after_state != "running") {
      const auto active = state.active_runs.find(key);
      if (active == state.active_runs.end() || found->second.run != active->second)
        return std::unexpected(corrupt());
      auto fenced = state.runs.find(active->second);
      if (fenced == state.runs.end() || fenced->second.state != "active")
        return std::unexpected(corrupt());
      // R18: the reviewer run of a verdict completes (N11) instead of being fenced.
      const bool completes    = transition.ordinal == 1 && (event.type == "verdict_recorded" ||
                                                            (event.type == "check_recorded" && transition.after_state == "done"));
      fenced->second.state    = completes                         ? "completed"
                                : event.type == "run_released"    ? "released"
                                : event.type == "run_interrupted" ? "interrupted"
                                                                  : "fenced";
      fenced->second.last_seq = event.seq;
      state.active_runs.erase(active);
    }
    if (transition.after_state == "running") {
      const auto active = state.active_runs.find(key);
      if (!derived || active == state.active_runs.end() || active->second != *derived)
        return std::unexpected(corrupt());
    } else if (state.active_runs.contains(key)) {
      return std::unexpected(corrupt());
    }
    role_track next;
    next.generation = transition.generation_id;
    next.state      = transition.after_state;
    next.drifted    = transition.after_drifted;
    next.run        = transition.after_state == "running" ? derived : std::nullopt;
    next.last_seq   = event.seq;
    if (created)
      next.fact_version = 1;
    else
      next.fact_version = found->second.fact_version + (bumping ? 1 : 0);
    state.roles[key] = next;
  }
  return {};
}

// Requests of one event: the bijection, operation, outcome, revision, body, result and attachment.
checked check_request(const ledger_rows& rows, const execution_metadata& metadata, const event_entry& event,
                      const hist::event_payload_v2& payload, const request_entry& request, const replay_state& state) {
  if (request.operation != operation_of(event.type).value_or("") ||
      (request.outcome != outcome_of(event.type).value_or("") &&
       !(event.type == "check_reserved" && request.outcome == "interrupted")) ||
      request.revision != *event.revision)
    return std::unexpected(corrupt());
  const bool load = request.operation == "load";
  const auto body = rows.records.find(request.body);
  if (body == rows.records.end() || body->second.kind != 4 || body->second.codec != (load ? 2 : 3))
    return std::unexpected(corrupt());
  if (load) {
    auto framed = hist::decode_request_v2(body->second.encoded);
    if (!framed || framed->operation != "load" || framed->actor != event.actor ||
        framed->expected_head != payload.before.active_revision)
      return std::unexpected(corrupt());
  } else {
    auto framed = hist::decode_request_v3(body->second.encoded);
    if (!framed || framed->operation != request.operation || framed->actor != event.actor)
      return std::unexpected(corrupt());
  }
  const auto* revision = revision_of(rows, *event.revision);
  if (!revision)
    return std::unexpected(corrupt());
  if (event.type == "declaration_accepted" || event.type == "load_noop") {
    auto saved = decode_load_result(request.result);
    if (!saved)
      return std::unexpected(corrupt());
    load_result expected{};
    expected.ledger     = metadata.identity;
    expected.request_id = request.request_id;
    expected.outcome    = event.type == "load_noop" ? load_outcome::noop : load_outcome::accepted;
    expected.revision   = static_cast<std::int64_t>(revision->id);
    if (revision->parent)
      expected.parent = static_cast<std::int64_t>(*revision->parent);
    expected.event = static_cast<std::int64_t>(event.seq);
    if (!(*saved == expected))
      return std::unexpected(corrupt());
    if (request.attachment)
      return std::unexpected(corrupt());
    return {};
  }
  if (event.type == "job_activated") {
    if (!state.activation || request.attachment)
      return std::unexpected(corrupt());
    const auto&              activation = rows.activations[*state.activation - 1];
    hist::activate_result_v1 expected;
    expected.ledger       = metadata.identity;
    expected.request_id   = request.request_id;
    expected.job          = *event.job;
    expected.revision     = std::to_string(*event.revision);
    expected.activation   = std::to_string(activation.id);
    expected.event        = std::to_string(event.seq);
    expected.previous_job = payload.before.active_job;
    for (const auto& transition : rows.transitions[event.seq - 1]) {
      if (transition.job != *event.job)
        continue;
      const auto* generation = generation_of(rows, transition.generation_id);
      if (!generation)
        return std::unexpected(corrupt());
      expected.roles.push_back({transition.role, std::to_string(generation->ordinal), transition.after_state});
    }
    std::ranges::sort(expected.roles, {}, &hist::activate_role_v1::role);
    auto encoded = hist::encode_activate_result_v1(expected);
    if (!encoded || *encoded != request.result)
      return std::unexpected(corrupt());
    return {};
  }

  if (event.type == "check_reserved" || event.type == "check_interrupted") {
    auto result = codec_decode(request.result);
    if (!result || !*result || (*result)->kind != value_kind::record || request.attachment)
      return std::unexpected(corrupt());
    const auto kind = (*result)->fields.find("kind");
    if (kind == (*result)->fields.end() || !kind->second || kind->second->string != "check")
      return std::unexpected(corrupt());
    return {};
  }
  if (event.type == "verdict_recorded") { // R18: the saved verdict_response_v1 names this request, run and event.
    auto saved = hist::decode_verdict_result_v1(request.result);
    if (!saved || saved->ledger != metadata.identity || saved->request_id != request.request_id || saved->job != *event.job ||
        saved->role != *event.role || saved->revision != std::to_string(*event.revision) ||
        saved->run != std::to_string(*event.run) || saved->event != std::to_string(event.seq) || request.attachment)
      return std::unexpected(corrupt());
    return {};
  }
  // Each remaining result is reconstructed solely from the original event's retained facts.
  // The replay state has already applied this event's transitions, so it must not supply before-drift.
  if (!event.run || !rows.runs.contains(*event.run))
    return std::unexpected(corrupt());
  const auto& run        = rows.runs.at(*event.run);
  const auto* generation = generation_of(rows, run.generation);
  if (!generation)
    return std::unexpected(corrupt());
  auto identity = [&](hist::recovery_result_identity& result) {
    result.ledger     = metadata.identity;
    result.request_id = request.request_id;
    result.job        = *event.job;
    result.role       = *event.role;
    result.run_id     = std::to_string(run.id);
    result.generation = std::to_string(generation->ordinal);
  };
  auto dispatch_metadata = [&](auto& result) -> checked {
    const auto id = parse_decimal(ref_of(payload.references, "dispatch"));
    if (!id || !rows.dispatches.contains(*id))
      return std::unexpected(corrupt());
    const auto& dispatch = rows.dispatches.at(*id);
    if (request.attachment != dispatch.packet)
      return std::unexpected(corrupt());
    const auto packet = record_of(rows, dispatch.packet, 1);
    if (!packet || (*packet)->codec != 1)
      return std::unexpected(corrupt());
    const auto  decoded = hist::decode_record((*packet)->encoded);
    const auto* file    = decoded ? std::get_if<hist::file_record>(&*decoded) : nullptr;
    if (!file)
      return std::unexpected(corrupt());
    result.dispatch_id   = std::to_string(dispatch.id);
    result.work_fp       = "xxh3_128:" + hex_of(dispatch.work_fp);
    result.render_digest = "xxh3_128:" + hex_of(dispatch.render_fp);
    result.bytes         = file->payload.size();
    return {};
  };
  std::optional<std::string> checkpoint;
  if (has_ref(payload.references, "checkpoint"))
    checkpoint = ref_of(payload.references, "checkpoint");
  if (request.operation == "refresh") {
    hist::refresh_result_v1 expected;
    identity(expected);
    expected.revision      = std::to_string(*event.revision);
    expected.checkpoint    = checkpoint;
    expected.drift_cleared = !rows.transitions[event.seq - 1].empty();
    if (auto ok = dispatch_metadata(expected); !ok)
      return ok;
    auto encoded = hist::encode_refresh_result_v1(expected);
    if (!encoded || *encoded != request.result)
      return std::unexpected(corrupt());
    return {};
  }
  if (request.operation == "checkpoint") {
    if (!checkpoint || request.attachment)
      return std::unexpected(corrupt());
    hist::checkpoint_result_v1 expected;
    identity(expected);
    expected.checkpoint = *checkpoint;
    expected.event      = std::to_string(event.seq);
    auto encoded        = hist::encode_checkpoint_result_v1(expected);
    if (!encoded || *encoded != request.result)
      return std::unexpected(corrupt());
    return {};
  }
  if (request.operation == "release") {
    const auto& transitions = rows.transitions[event.seq - 1];
    if (request.attachment || transitions.empty())
      return std::unexpected(corrupt());
    hist::release_result_v1 expected;
    identity(expected);
    expected.state      = event.type == "run_released" ? "released" : "interrupted";
    expected.role_state = transitions.front().after_state;
    expected.event      = std::to_string(event.seq);
    expected.checkpoint = checkpoint;
    auto encoded        = hist::encode_release_result_v1(expected);
    if (!encoded || *encoded != request.result)
      return std::unexpected(corrupt());
    return {};
  }
  hist::claim_result_v1 expected;
  expected.ledger     = metadata.identity;
  expected.request_id = request.request_id;
  expected.job        = *event.job;
  expected.role       = *event.role;
  expected.revision   = std::to_string(*event.revision);
  expected.run_id     = std::to_string(run.id);
  expected.generation = std::to_string(generation->ordinal);
  std::array<std::uint8_t, 16> secret{};
  for (std::size_t i = 0; i < secret.size(); ++i)
    secret[i] = static_cast<std::uint8_t>(run.secret[i]);
  expected.token = format_run_token(run_token{run.id, generation->ordinal, secret});
  if (event.type == "run_taken_over")
    expected.previous_run = ref_of(payload.references, "previous_run");
  if (auto ok = dispatch_metadata(expected); !ok)
    return ok;
  auto encoded = hist::encode_claim_result_v1(expected);
  if (!encoded || *encoded != request.result)
    return std::unexpected(corrupt());
  return {};
}

checked replay_events(const ledger_rows& rows, const execution_metadata& metadata, replay_state& state) {
  if (rows.events.empty())
    return std::unexpected(corrupt());
  for (std::size_t index = 0; index < rows.events.size(); ++index) {
    const auto& event       = rows.events[index];
    const auto& transitions = rows.transitions[index];
    const auto  seq         = event.seq;
    if (seq == 1) {
      if (auto ok = check_initialization(rows, event, metadata); !ok)
        return ok;
      if (!transitions.empty())
        return std::unexpected(corrupt());
      continue;
    }
    const auto op = operation_of(event.type);
    if (!op || event.version != 2 || !outcome_of(event.type))
      return std::unexpected(corrupt());
    auto payload = hist::decode_event_payload_v2(event.type, event.payload);
    if (!payload || payload->operation != *op)
      return std::unexpected(corrupt());
    auto before = parse_position(payload->before.active_revision);
    auto after  = parse_position(payload->after.active_revision);
    if (!before || !after || *before != state.head || payload->before.active_job != state.active_job || !event.revision ||
        *after != event.revision)
      return std::unexpected(corrupt());
    if (!typed_columns_ok(event))
      return std::unexpected(corrupt());
    if (!hist::validate_transition_sequence(transitions) || payload->reason != hist::event_level_reason(transitions))
      return std::unexpected(corrupt());
    const auto& request_id = rows.request_of_event[index];
    if (event.type != "check_reserved" && event.type != "check_recorded" &&
        (event.request_id.has_value() != request_id.has_value() || (request_id && *request_id != *event.request_id)))
      return std::unexpected(corrupt());
    const request_entry* request = request_id ? &rows.requests.at(*request_id) : nullptr;

    if (event.type == "declaration_accepted") {
      const auto* revision = revision_of(rows, *event.revision);
      if (!revision || state.accepted[revision->id]++ != 0)
        return std::unexpected(corrupt());
      if (revision->parent != before.value() || revision->actor != event.actor || revision->accepted_at != event.occurred_at ||
          !references_match(payload->references, *revision))
        return std::unexpected(corrupt());
      if (*before) {
        const auto* previous = revision_of(rows, **before);
        if (!previous || identical_revisions(rows, *revision, *previous))
          return std::unexpected(corrupt());
      }
      if (!payload->after.active_job.empty() && payload->after.active_job != state.active_job)
        return std::unexpected(corrupt());
    } else if (event.type == "load_noop") {
      const auto* revision = revision_of(rows, *event.revision);
      if (!*before || !revision || payload->after.active_job != state.active_job || !transitions.empty() ||
          !references_match(payload->references, *revision))
        return std::unexpected(corrupt());
    } else if (event.type == "job_activated") {
      if (!*before || payload->after.active_job != *event.job || state.activations_seen >= rows.activations.size())
        return std::unexpected(corrupt());
      const auto& activation = rows.activations[state.activations_seen];
      if (activation.id != state.activations_seen + 1 || activation.job != *event.job || activation.revision != *event.revision ||
          activation.event_seq != seq || ref_of(payload->references, "activation") != std::to_string(activation.id) ||
          ref_of(payload->references, "revision") != std::to_string(*event.revision))
        return std::unexpected(corrupt());
      ++state.activations_seen;
      state.activation = activation.id;
    } else {
      if (payload->before.active_revision != payload->after.active_revision ||
          payload->before.active_job != payload->after.active_job)
        return std::unexpected(corrupt());
      if (event.type == "check_reserved" || event.type == "check_recorded" || event.type == "check_interrupted") {
        if (!event.request_id || (event.type != "check_recorded" && !transitions.empty()))
          return std::unexpected(corrupt());
        if (event.type != "check_interrupted") {
          const auto active = state.active_runs.find({*event.job, *event.role});
          if (active == state.active_runs.end() || active->second != *event.run)
            return std::unexpected(corrupt());
        }
      } else if (event.type == "verdict_recorded") { // R18:
        if (auto ok = apply_verdict_event(event, *payload, transitions, state); !ok)
          return ok;
      } else if (event.type == "run_claimed" || event.type == "run_taken_over") {
        if (auto ok = apply_run_event(rows, event, *payload, transitions, state); !ok)
          return ok;
      } else {
        auto run  = state.runs.find(*event.run);
        auto role = state.roles.find({*event.job, *event.role});
        if (run == state.runs.end() || role == state.roles.end() || run->second.state != "active" ||
            run->second.generation != *event.generation || role->second.run != event.run ||
            ref_of(payload->references, "run") != std::to_string(*event.run))
          return std::unexpected(corrupt());
        if (event.type == "checkpoint_recorded" && !transitions.empty())
          return std::unexpected(corrupt());
        if (event.type == "run_released" || event.type == "run_interrupted") {
          auto reason = event.type == "run_released" ? "released" : "interrupted";
          if (transitions.empty() || transitions[0].job != *event.job || transitions[0].role != *event.role ||
              transitions[0].reason != reason || transitions[0].after_state == "running" ||
              transitions[0].after_state == "retired")
            return std::unexpected(corrupt());
          for (std::size_t i = 1; i < transitions.size(); ++i) {
            const auto& t = transitions[i];
            if (t.job != *event.job || t.role == *event.role || t.before_state != "blocked" ||
                t.reason != "prerequisite_available" || t.after_drifted ||
                (t.after_state != "ready" && t.after_state != "done" && t.after_state != "stale"))
              return std::unexpected(corrupt());
          }
        }
        if (event.type == "run_refreshed") {
          auto id = parse_decimal(ref_of(payload->references, "dispatch"));
          if (!id || !rows.dispatches.contains(*id))
            return std::unexpected(corrupt());
          const auto& d = rows.dispatches.at(*id);
          if (d.run != *event.run || d.seq != seq || d.kind != "refresh" || d.revision != *event.revision ||
              ref_of(payload->references, "packet") != std::to_string(d.packet) ||
              ref_of(payload->references, "manifest") != std::to_string(d.manifest) ||
              ref_of(payload->references, "work") != std::to_string(d.work))
            return std::unexpected(corrupt());
          if (role->second.drifted ? (transitions.size() != 1 || transitions[0].reason != "drift_cleared") : !transitions.empty())
            return std::unexpected(corrupt());
        }
        if (has_ref(payload->references, "checkpoint")) {
          auto id = parse_decimal(ref_of(payload->references, "checkpoint"));
          if (!id || !rows.checkpoints.contains(*id))
            return std::unexpected(corrupt());
          const auto& c = rows.checkpoints.at(*id);
          if (c.run_id != *event.run || c.seq != seq)
            return std::unexpected(corrupt());
        }
      }
    }

    if (auto ok = apply_transitions(rows, event, transitions, state); !ok)
      return ok;
    if (request) {
      if (auto ok = check_request(rows, metadata, event, *payload, *request, state); !ok)
        return ok;
    }
    state.head       = *after;
    state.active_job = payload->after.active_job;
  }
  return {};
}

// Five-role revision checks that need no event: the record shapes of D23 and the stdlib digest of the versions value.
checked check_revision_shapes(const ledger_rows& rows) {
  for (const auto& [id, revision] : rows.revisions) {
    (void)id;
    const auto source   = record_of(rows, revision.source, 2);
    const auto registry = record_of(rows, revision.registry, 2);
    const auto stdlib   = record_of(rows, revision.stdlib, 2);
    if (!source || !registry || !stdlib)
      return std::unexpected(corrupt());
    auto tree = hist::decode_record((*registry)->encoded);
    if (!tree)
      return std::unexpected(corrupt());
    const auto* directory = std::get_if<hist::directory_record>(&*tree);
    if (!directory || directory->children.size() != 1 || directory->children[0].name != "input.json" ||
        directory->children[0].kind != hist::record_kind::file)
      return std::unexpected(corrupt());
    const auto interpretation = value_of(rows, revision.interpretation, "canonical_v1");
    const auto versions       = value_of(rows, revision.versions, "canonical_v1");
    if (!interpretation || !versions)
      return std::unexpected(corrupt());
    auto decoded = ::planar::engine::canonical_decode((*versions)->canonical);
    if (!decoded || decoded.value()->kind != value_kind::record)
      return std::unexpected(corrupt());
    const auto field = decoded.value()->fields.find("stdlib_digest");
    if (field == decoded.value()->fields.end() || !field->second || field->second->kind != value_kind::string ||
        field->second->string != hex_of((*stdlib)->digest))
      return std::unexpected(corrupt());
  }
  return {};
}

// ---------------------------------------------------------------------------------------------------------------------
// L2 residuals: the five format2-era cross-row checks of EX-1 invariant 20, on format3 rows.

hist::record_digest digest_array(std::string_view bytes) {
  hist::record_digest out{};
  for (std::size_t i = 0; i < out.size() && i < bytes.size(); ++i)
    out[i] = static_cast<std::uint8_t>(bytes[i]);
  return out;
}

// Directory edges versus encoded children: every edge belongs to a directory record, ordinals are contiguous from zero,
// and each edge names exactly the encoded child at its ordinal, whose stored record has the encoded kind and digest.
checked check_tree_edges(const ledger_rows& rows) {
  for (const auto& [parent, list] : rows.edges) {
    const auto found = rows.records.find(parent);
    if (found == rows.records.end() || found->second.kind != 2)
      return std::unexpected(corrupt());
  }
  for (const auto& [id, entry] : rows.records) {
    if (entry.kind != 2)
      continue;
    auto decoded = hist::decode_record(entry.encoded);
    if (!decoded)
      return std::unexpected(corrupt());
    const auto* directory = std::get_if<hist::directory_record>(&*decoded);
    if (!directory)
      return std::unexpected(corrupt());
    const auto                         edges = rows.edges.find(id);
    static const std::vector<edge_row> none;
    const auto&                        list = edges == rows.edges.end() ? none : edges->second;
    if (list.size() != directory->children.size())
      return std::unexpected(corrupt());
    for (std::size_t i = 0; i < list.size(); ++i) {
      const auto& edge  = list[i];
      const auto& child = directory->children[i];
      if (edge.ordinal != i || edge.name != child.name || edge.child != child.id || child.id >= id)
        return std::unexpected(corrupt());
      const auto stored = rows.records.find(child.id);
      if (stored == rows.records.end() || stored->second.kind != static_cast<std::int64_t>(child.kind) ||
          stored->second.digest != digest_bytes(child.digest))
        return std::unexpected(corrupt());
    }
  }
  return {};
}

// Reader over the loaded records for tree_core: codec1 file, directory and value records only.
std::function<std::expected<types::stored_record, types::format_error>(std::uint64_t)> rows_reader(const ledger_rows& rows) {
  return [&rows](std::uint64_t id) -> std::expected<types::stored_record, types::format_error> {
    const auto found = rows.records.find(id);
    if (found == rows.records.end() || found->second.kind < 1 || found->second.kind > 3 || found->second.codec != 1)
      return std::unexpected(types::format_error{types::format_error_code::corrupt, "Invalid tree record"});
    auto decoded = hist::decode_record(found->second.encoded);
    if (!decoded)
      return std::unexpected(types::format_error{types::format_error_code::corrupt, "Invalid tree record"});
    const types::record_reference reference{id, hist::kind_of(*decoded), digest_array(found->second.digest)};
    return types::stored_record(reference, std::move(*decoded));
  };
}

value_result<std::vector<hist::tree_file>> read_tree(const ledger_rows& rows, std::uint64_t root, hist::tree_profile profile) {
  const auto found = rows.records.find(root);
  if (found == rows.records.end() || found->second.kind != 2)
    return std::unexpected(corrupt());
  const types::record_reference reference{root, hist::record_kind::directory, digest_array(found->second.digest)};
  auto                          files = hist::read_tree_core(reference, profile, rows_reader(rows));
  if (!files)
    return std::unexpected(corrupt());
  return std::move(*files);
}

// Tree reachability, embedded versions, workspace identity and recorded family versus the profile selector of every
// load request that names the revision.
checked check_revision_content(const ledger_rows& rows, const execution_metadata& metadata) {
  std::map<std::uint64_t, hist::recorded_family> families;
  for (const auto& [id, revision] : rows.revisions) {
    const auto interpretation = rows.records.find(revision.interpretation);
    const auto versions       = rows.records.find(revision.versions);
    if (interpretation == rows.records.end() || versions == rows.records.end() ||
        interpretation->second.value_codec != "canonical_v1" || versions->second.value_codec != "canonical_v1")
      return std::unexpected(corrupt());
    auto family = hist::classify_recorded_interpretation(interpretation->second.canonical, versions->second.canonical);
    if (!family)
      return std::unexpected(corrupt());
    families[id] = *family;
    // Tree reachability: every file and directory of the three trees resolves from its root under its profile.
    if (auto tree = read_tree(rows, revision.source, hist::tree_profile::source); !tree)
      return std::unexpected(tree.error());
    if (auto tree = read_tree(rows, revision.registry, hist::tree_profile::provenance); !tree)
      return std::unexpected(tree.error());
    const auto stdlib_profile =
        *family == hist::recorded_family::effective ? hist::tree_profile::effective_provenance : hist::tree_profile::provenance;
    if (auto tree = read_tree(rows, revision.stdlib, stdlib_profile); !tree)
      return std::unexpected(tree.error());
    // Embedded versions equal the versions record byte for byte.
    auto root = ::planar::engine::canonical_decode(interpretation->second.canonical);
    if (!root || !*root || (*root)->kind != value_kind::record)
      return std::unexpected(corrupt());
    const auto embedded = (*root)->fields.find("versions");
    if (embedded == (*root)->fields.end())
      return std::unexpected(corrupt());
    auto embedded_bytes = ::planar::engine::canonical_encode(embedded->second);
    if (!embedded_bytes || *embedded_bytes != versions->second.canonical)
      return std::unexpected(corrupt());
    // Workspace identifier of the recorded bundle equals ledger_meta.workspace_id.
    const auto bundle = (*root)->fields.find("bundle");
    if (bundle == (*root)->fields.end() || !bundle->second || bundle->second->kind != value_kind::record)
      return std::unexpected(corrupt());
    const auto workspace = bundle->second->fields.find("workspace");
    if (workspace == bundle->second->fields.end() || !workspace->second || workspace->second->kind != value_kind::record)
      return std::unexpected(corrupt());
    const auto workspace_id = workspace->second->fields.find("id");
    if (workspace_id == workspace->second->fields.end() || !workspace_id->second ||
        workspace_id->second->kind != value_kind::string || workspace_id->second->string != metadata.workspace_id)
      return std::unexpected(corrupt());
  }
  // Recorded family versus the profile selector of each load request.
  for (const auto& [key, request] : rows.requests) {
    (void)key;
    if (request.operation != "load")
      continue;
    const auto body = rows.records.find(request.body);
    if (body == rows.records.end() || body->second.kind != 4 || body->second.codec != 2)
      return std::unexpected(corrupt());
    auto framed = hist::decode_request_v2(body->second.encoded);
    if (!framed)
      return std::unexpected(corrupt());
    const auto family = families.find(request.revision);
    if (family == families.end())
      return std::unexpected(corrupt());
    const auto requested = framed->profile_argument ? hist::recorded_family::effective : hist::recorded_family::embedded;
    if (family->second != requested)
      return std::unexpected(corrupt());
  }
  return {};
}

// Role lineage as stored facts (EX-5, D44): reasons, opening events, takeover lineage and the structure digests.
checked check_generations(const ledger_rows& rows) {
  std::map<role_key, std::vector<const generation_entry*>> groups;
  for (const auto& [id, generation] : rows.generations) {
    (void)id;
    groups[role_key{generation.job, generation.role}].push_back(&generation);
  }
  for (auto& [key, list] : groups) {
    (void)key;
    std::ranges::sort(list, [](const generation_entry* a, const generation_entry* b) { return a->ordinal < b->ordinal; });
    for (std::size_t i = 0; i < list.size(); ++i)
      if (list[i]->ordinal != i + 1)
        return std::unexpected(corrupt());
    for (std::size_t i = 0; i < list.size(); ++i) {
      const auto& generation       = *list[i];
      const auto  predecessor      = i == 0 ? nullptr : list[i - 1];
      auto        structure_record = value_of(rows, generation.structure, "role_structure_v1");
      if (!structure_record)
        return std::unexpected(corrupt());
      auto structure = hist::decode_role_structure_v1((*structure_record)->canonical);
      if (!structure || structure->job != generation.job || structure->role != generation.role)
        return std::unexpected(corrupt());
      if (generation.opened_seq == 0 || generation.opened_seq > rows.events.size())
        return std::unexpected(corrupt());
      const auto& opening = rows.events[generation.opened_seq - 1];
      const auto& carried = rows.transitions[generation.opened_seq - 1];
      const auto  row     = std::ranges::find_if(carried, [&](const hist::transition_row& t) {
        return t.job == generation.job && t.role == generation.role && t.generation_id == generation.id;
      });
      if (row == carried.end())
        return std::unexpected(corrupt());
      const auto reason = generation.reason;
      if (reason == "activated") {
        if (opening.type != "job_activated")
          return std::unexpected(corrupt());
      } else if (reason == "structural_change" || reason == "recreated") {
        if (opening.type != "declaration_accepted")
          return std::unexpected(corrupt());
      } else if (reason == "takeover") {
        if (opening.type != "run_taken_over")
          return std::unexpected(corrupt());
      } else {
        return std::unexpected(corrupt());
      }
      if (reason == "recreated" && (!row->before_state || *row->before_state != "retired"))
        return std::unexpected(corrupt());
      if (reason == "takeover") {
        if (!predecessor)
          return std::unexpected(corrupt());
        if (predecessor->structure != generation.structure)
          return std::unexpected(corrupt());
      }
      if (reason == "structural_change" && predecessor) {
        auto previous_record = value_of(rows, predecessor->structure, "role_structure_v1");
        if (!previous_record)
          return std::unexpected(corrupt());
        auto previous = hist::decode_role_structure_v1((*previous_record)->canonical);
        if (!previous)
          return std::unexpected(corrupt());
        auto before = hist::role_structure_digest(*previous);
        auto after  = hist::role_structure_digest(*structure);
        if (!before || !after || *before == *after)
          return std::unexpected(corrupt());
      }
    }
  }
  return {};
}

// Dispatch content (EX-1 invariant rows of V27): the W value, the packet Markdown and the manifest digests.
checked check_dispatch_content(const ledger_rows& rows, const dispatch_entry& dispatch) {
  auto work_record = value_of(rows, dispatch.work, "work_exec_v1");
  if (!work_record)
    return std::unexpected(corrupt());
  auto work = hist::decode_work_exec_v1((*work_record)->canonical);
  if (!work)
    return std::unexpected(corrupt());
  auto work_text = ::planar::engine::hash_value("work_exec_v1", *work);
  if (!work_text || digest_bytes_of_text(*work_text) != dispatch.work_fp)
    return std::unexpected(corrupt());

  const auto packet = record_of(rows, dispatch.packet, 1);
  if (!packet || (*packet)->codec != 1)
    return std::unexpected(corrupt());
  auto        packet_value = hist::decode_record((*packet)->encoded);
  const auto* file         = packet_value ? std::get_if<hist::file_record>(&*packet_value) : nullptr;
  if (!file || file->mode != 420)
    return std::unexpected(corrupt());
  auto render_text = ::planar::engine::fragment_digest(file->payload);
  if (!render_text || digest_bytes_of_text(*render_text) != dispatch.render_fp)
    return std::unexpected(corrupt());

  auto manifest_record = value_of(rows, dispatch.manifest, "dispatch_manifest_v1");
  if (!manifest_record)
    return std::unexpected(corrupt());
  auto manifest = hist::decode_dispatch_manifest_v1((*manifest_record)->canonical);
  if (!manifest || manifest.value()->kind != value_kind::record)
    return std::unexpected(corrupt());
  const auto& fields       = manifest.value()->fields;
  const auto  work_field   = fields.find("work");
  const auto  render_field = fields.find("render");
  if (work_field == fields.end() || render_field == fields.end() || !work_field->second || !render_field->second)
    return std::unexpected(corrupt());
  const auto work_digest   = work_field->second->fields.find("digest");
  const auto render_digest = render_field->second->fields.find("digest");
  if (work_digest == work_field->second->fields.end() || render_digest == render_field->second->fields.end() ||
      !work_digest->second || !render_digest->second || work_digest->second->string != *work_text ||
      render_digest->second->string != *render_text)
    return std::unexpected(corrupt());
  return {};
}

// Every run is one claim or takeover event with exactly one dispatch; the dispatch rows agree with their runs and events.
checked check_dispatches(const ledger_rows& rows) {
  if (rows.dispatches.size() < rows.runs.size() || rows.dispatch_by_run.size() != rows.runs.size())
    return std::unexpected(corrupt());
  for (const auto& [id, dispatch] : rows.dispatches) {
    (void)id;
    const auto run = rows.runs.find(dispatch.run);
    if (run == rows.runs.end() || (dispatch.kind != "refresh" && run->second.claimed_seq != dispatch.seq) ||
        (dispatch.kind == "refresh" && (dispatch.seq <= run->second.claimed_seq || dispatch.seq > rows.events.size() ||
                                        rows.events[dispatch.seq - 1].type != "run_refreshed")))
      return std::unexpected(corrupt());
    if (dispatch.checkpoint) {
      bool found = false;
      for (const auto& [id, c] : rows.checkpoints)
        if (c.run_id == dispatch.run && c.seq <= dispatch.seq && rows.checkpoint_bodies.at(id) == *dispatch.checkpoint)
          found = true;
      if (!found)
        return std::unexpected(corrupt());
    }
    if (auto ok = check_dispatch_content(rows, dispatch); !ok)
      return ok;
  }
  return {};
}

// Run rows and run_state rows equal the replay; role rows equal the replayed roles and the coupling holds (EX-1 12, 17).
checked check_final_state(const ledger_rows& rows, const replay_state& state) {
  if (rows.role_states.size() != state.roles.size() || rows.run_states.size() != state.runs.size() ||
      rows.run_states.size() != rows.runs.size())
    return std::unexpected(corrupt());
  for (const auto& row : rows.role_states) {
    const auto found = state.roles.find(role_key{row.job, row.role});
    if (found == state.roles.end())
      return std::unexpected(corrupt());
    const auto& track = found->second;
    if (row.generation != track.generation || row.state != track.state || row.drifted != track.drifted || row.run != track.run ||
        row.fact_version != track.fact_version || row.last_seq != track.last_seq)
      return std::unexpected(corrupt());
  }
  for (const auto& row : rows.run_states) {
    const auto found = state.runs.find(row.run);
    if (found == state.runs.end() || !rows.runs.contains(row.run))
      return std::unexpected(corrupt());
    const auto& track = found->second;
    if (row.job != track.job || row.role != track.role || row.generation != track.generation || row.state != track.state ||
        row.last_seq != track.last_seq || rows.runs.at(row.run).generation != track.generation)
      return std::unexpected(corrupt());
  }
  for (const auto& [key, track] : state.roles) {
    const auto active = state.active_runs.find(key);
    if (track.state == "running") {
      if (!track.run || active == state.active_runs.end() || active->second != *track.run)
        return std::unexpected(corrupt());
      const auto run = state.runs.find(*track.run);
      if (run == state.runs.end() || run->second.state != "active" || run->second.generation != track.generation)
        return std::unexpected(corrupt());
    } else if (active != state.active_runs.end() || track.run) {
      return std::unexpected(corrupt());
    }
    const bool active_job = !state.active_job.empty() && key.first == state.active_job;
    if (!active_job && track.state != "retired")
      return std::unexpected(corrupt());
  }
  for (const auto& [key, run] : state.active_runs) {
    const auto track = state.roles.find(key);
    if (track == state.roles.end() || track->second.state != "running" || track->second.run != run)
      return std::unexpected(corrupt());
  }
  for (const auto& [id, track] : state.runs) {
    (void)id;
    if (track.state == "active" && state.active_runs.find(role_key{track.job, track.role}) == state.active_runs.end())
      return std::unexpected(corrupt());
  }
  return {};
}

checked check_current(const ledger_rows& rows, const replay_state& state) {
  if (!rows.current || rows.events.empty())
    return std::unexpected(corrupt());
  const auto& current = *rows.current;
  if (current.last_seq != rows.events.size() || current.revision != state.head || current.job.value_or("") != state.active_job ||
      current.activation != state.activation)
    return std::unexpected(corrupt());
  if (state.activation && state.activations_seen != rows.activations.size())
    return std::unexpected(corrupt());
  return {};
}

// Consistent identifiers and the closed-set row counts that the verifier needs before the replay.
checked check_sequences(const ledger_rows& rows) {
  for (std::size_t i = 0; i < rows.activations.size(); ++i)
    if (rows.activations[i].id != i + 1)
      return std::unexpected(corrupt());
  if (rows.activations.size() > rows.events.size())
    return std::unexpected(corrupt());
  return {};
}

// Stored provenance records decode under the same bounds as the writer (EX-8, D48).
checked check_provenance(const ledger_rows& rows) {
  for (const auto& [id, run] : rows.runs) {
    (void)id;
    if (!run.provenance)
      continue;
    auto record = value_of(rows, *run.provenance, "driver_provenance_v1");
    if (!record || !hist::decode_driver_provenance_v1((*record)->canonical))
      return std::unexpected(corrupt());
  }
  return {};
}

// The execution state value S of D24. The dispatch revision comes from the map when the caller knows it.
value_ptr state_value(const execution_snapshot& snapshot, const std::map<std::uint64_t, std::uint64_t>& revisions) {
  std::vector<value_ptr> roles;
  for (const auto& row : snapshot.roles) {
    std::vector<value_ptr> work_units;
    for (const auto& unit : row.structure.work)
      work_units.push_back(text_node(unit));
    std::vector<value_ptr> ownership;
    for (const auto& entry : row.structure.ownership)
      ownership.push_back(record_node({{"work", text_node(entry.work)}, {"paths", texts_node(entry.paths)}}));
    auto      structure = record_node({{"job", text_node(row.structure.job)},
                                       {"role", text_node(row.structure.role)},
                                       {"kind", text_node(row.structure.kind)},
                                       {"work", list_node(work_units)},
                                       {"targets", texts_node(row.structure.targets)},
                                       {"ownership", list_node(ownership)},
                                       {"shape", text_node(row.structure.shape)}});
    value_ptr run       = text_node("none");
    if (row.run_id && row.dispatch) {
      std::uint64_t revision = snapshot.revision ? snapshot.revision->id : 0;
      const auto    found    = revisions.find(row.dispatch->id);
      if (found != revisions.end())
        revision = found->second;
      run = record_node({{"id", text_node(std::to_string(*row.run_id))},
                         {"generation", text_node(std::to_string(row.generation_ordinal))},
                         {"dispatch", record_node({{"id", text_node(std::to_string(row.dispatch->id))},
                                                   {"work_fp", text_node(row.dispatch->work_fp)},
                                                   {"render_fp", text_node(row.dispatch->render_fp)},
                                                   {"revision", text_node(std::to_string(revision))},
                                                   {"work", row.dispatch->work}})}});
    }
    roles.push_back(record_node({{"job", text_node(row.job)},
                                 {"role", text_node(row.role)},
                                 {"kind", text_node(row.structure.kind)},
                                 {"generation", record_node({{"id", text_node(std::to_string(row.generation_id))},
                                                             {"ordinal", text_node(std::to_string(row.generation_ordinal))},
                                                             {"structure", structure}})},
                                 {"state", text_node(row.state)},
                                 {"drifted", bool_node(row.drifted)},
                                 {"fact_version", text_node(std::to_string(row.fact_version))},
                                 {"run", run}}));
  }
  value_ptr   revision = text_node("none");
  std::string head     = "none";
  if (snapshot.revision) {
    head     = std::to_string(snapshot.revision->id);
    revision = record_node({{"id", text_node(head)}});
  }
  const std::string                activation = snapshot.activation ? std::to_string(*snapshot.activation) : "none";
  std::map<std::string, value_ptr> fields{{"active_revision", text_node(head)},  {"active_job", text_node(snapshot.active_job)},
                                          {"activation", text_node(activation)}, {"revision", revision},
                                          {"roles", list_node(roles)},           {"publications", list_node({})}};
  std::vector<value_ptr>           reservations, evidence, observed, publications;
  for (const auto& r : snapshot.reservations)
    reservations.push_back(record_node({{"id", text_node(std::to_string(r.id))},
                                        {"run", text_node(std::to_string(r.run))},
                                        {"request_id", text_node(r.request_id)},
                                        {"opened_seq", text_node(std::to_string(r.opened_seq))}}));
  for (const auto& e : snapshot.evidence)
    evidence.push_back(record_node({{"id", text_node(std::to_string(e.id))},
                                    {"reservation", text_node(std::to_string(e.reservation))},
                                    {"seq", text_node(std::to_string(e.seq))},
                                    {"work", text_node(e.work)},
                                    {"check", text_node(e.check)},
                                    {"outcome", text_node(e.outcome)},
                                    {"reason", text_node(e.reason)},
                                    {"matched", text_node(e.matched ? std::to_string(*e.matched) : "none")},
                                    {"vector", e.vector}}));
  for (const auto& o : snapshot.observed)
    observed.push_back(
        record_node({{"work", text_node(o.work)}, {"output", text_node(o.output)}, {"seq", text_node(std::to_string(o.seq))}}));
  for (const auto& p : snapshot.publications)
    publications.push_back(record_node({{"publication", text_node(std::to_string(p.publication))},
                                        {"completion", text_node(std::to_string(p.completion))},
                                        {"job", text_node(p.job)},
                                        {"role", text_node(p.role)},
                                        {"work", text_node(p.work)},
                                        {"facet", text_node(p.facet)},
                                        {"semantic", text_node(p.semantic)}}));
  fields["publications"] = list_node(std::move(publications));
  if (!reservations.empty())
    fields["reservations"] = list_node(std::move(reservations));
  if (!evidence.empty())
    fields["evidence"] = list_node(std::move(evidence));
  if (!observed.empty())
    fields["observed"] = list_node(std::move(observed));
  for (auto& [key, member] : review_state_entries(snapshot)) // R18: completions and review (DB-R16).
    fields[key] = std::move(member);
  return record_node(std::move(fields));
}

std::uint64_t measure_state(const execution_snapshot& snapshot, const std::map<std::uint64_t, std::uint64_t>& revisions) {
  auto encoded = ::planar::engine::canonical_encode(state_value(snapshot, revisions));
  if (!encoded)
    return std::numeric_limits<std::uint64_t>::max();
  return encoded->size();
}

// Snapshot of the verified rows: the revision values, the active job, the roles, and the acknowledged dispatches.
value_result<execution_snapshot> build_snapshot(const ledger_rows& rows, const replay_state& state,
                                                const execution_metadata&               metadata,
                                                std::map<std::uint64_t, std::uint64_t>& dispatch_revisions) {
  execution_snapshot snapshot;
  snapshot.identity   = metadata.identity;
  snapshot.last_seq   = rows.events.size();
  snapshot.active_job = state.active_job;
  snapshot.activation = state.activation;
  if (state.head) {
    const auto&        revision = rows.revisions.at(*state.head);
    execution_revision out;
    out.id              = revision.id;
    out.source          = revision.source;
    out.registry        = revision.registry;
    out.stdlib          = revision.stdlib;
    out.interpretation  = revision.interpretation;
    out.versions        = revision.versions;
    out.source_digest   = hex_of(rows.records.at(revision.source).digest);
    out.stdlib_digest   = hex_of(rows.records.at(revision.stdlib).digest);
    auto interpretation = ::planar::engine::canonical_decode(rows.records.at(revision.interpretation).canonical);
    auto versions       = ::planar::engine::canonical_decode(rows.records.at(revision.versions).canonical);
    if (!interpretation || !versions)
      return std::unexpected(corrupt());
    out.interpretation_value = *interpretation;
    out.versions_value       = *versions;
    out.registry_value       = record_node({{"id", text_node(std::to_string(revision.registry))},
                                            {"digest", text_node(hex_of(rows.records.at(revision.registry).digest))}});
    snapshot.revision        = std::move(out);
  }
  for (const auto& row : rows.role_states) {
    execution_role role;
    role.job                = row.job;
    role.role               = row.role;
    role.generation_id      = row.generation;
    role.generation_ordinal = generation_of(rows, row.generation)->ordinal;
    auto structure_record   = value_of(rows, generation_of(rows, row.generation)->structure, "role_structure_v1");
    if (!structure_record)
      return std::unexpected(corrupt());
    auto structure = hist::decode_role_structure_v1((*structure_record)->canonical);
    if (!structure)
      return std::unexpected(corrupt());
    role.structure    = std::move(*structure);
    role.state        = row.state;
    role.drifted      = row.drifted;
    role.run_id       = row.run;
    role.fact_version = row.fact_version;
    role.last_seq     = row.last_seq;
    if (row.run) {
      role.run_request_id  = rows.runs.at(*row.run).request_id;
      const auto& dispatch = rows.dispatches.at(rows.dispatch_by_run.at(*row.run));
      auto        work     = hist::decode_work_exec_v1(rows.records.at(dispatch.work).canonical);
      if (!work)
        return std::unexpected(corrupt());
      execution_dispatch out;
      out.id          = dispatch.id;
      out.kind        = dispatch.kind;
      out.work_fp     = hex_of(dispatch.work_fp);
      out.render_fp   = hex_of(dispatch.render_fp);
      out.packet_id   = dispatch.packet;
      out.manifest_id = dispatch.manifest;
      out.work_id     = dispatch.work;
      out.revision    = dispatch.revision;
      out.work        = *work;
      role.dispatch   = std::move(out);
      for (const auto& [id, note] : rows.checkpoints)
        if (note.run_id == *row.run && (!role.checkpoint || note.seq > role.checkpoint->seq))
          role.checkpoint = note;
      dispatch_revisions[dispatch.id] = dispatch.revision;
    }
    snapshot.roles.push_back(std::move(role));
  }
  return snapshot;
}

// The shared cross-row verifier (D47). It runs on an open connection inside its current transaction, reads every table
// into rows, verifies every format3 invariant, and returns the snapshot of the rows it verified. Every violation is
// ledger_corrupt; the commit layer maps the same check on uncommitted state to invalid_change.
checked read_check_rows(connection& db, execution_snapshot& snapshot, const ledger_rows& rows);

value_result<execution_snapshot> verify_rows(connection& db, const execution_metadata& metadata, ledger_rows& rows) {
  if (auto reserved = load_reserved_empty(db); !reserved)
    return std::unexpected(reserved.error());
  if (auto loaded = load_records(db, rows); !loaded)
    return std::unexpected(loaded.error());
  if (auto loaded = load_events(db, rows); !loaded)
    return std::unexpected(loaded.error());
  if (auto loaded = load_transitions(db, rows); !loaded)
    return std::unexpected(loaded.error());
  if (auto loaded = load_revisions(db, rows); !loaded)
    return std::unexpected(loaded.error());
  if (auto loaded = load_generations(db, rows); !loaded)
    return std::unexpected(loaded.error());
  if (auto loaded = load_activations(db, rows); !loaded)
    return std::unexpected(loaded.error());
  if (auto loaded = load_runs(db, rows); !loaded)
    return std::unexpected(loaded.error());
  if (auto loaded = load_dispatches(db, rows); !loaded)
    return std::unexpected(loaded.error());
  if (auto loaded =
          for_each(db, "SELECT id,run_id,seq,body_id FROM checkpoints ORDER BY id",
                   [&](row_reader& r) -> checked {
                     auto id = r.id(0), run = r.id(1), seq = r.id(2), body = r.id(3);
                     auto rec = value_of(rows, body, "checkpoint_v1");
                     if (!rec || !rows.runs.contains(run) || id != rows.checkpoints.size() + 1 || seq > rows.events.size())
                       return std::unexpected(corrupt());
                     auto note = hist::decode_checkpoint_v1((*rec)->canonical);
                     if (!note)
                       return std::unexpected(corrupt());
                     const auto& event = rows.events[seq - 1];
                     if (event.run != std::optional<std::uint64_t>(run) ||
                         (event.type != "checkpoint_recorded" && event.type != "run_refreshed" && event.type != "run_released" &&
                          event.type != "run_interrupted"))
                       return std::unexpected(corrupt());
                     auto payload = hist::decode_event_payload_v2(event.type, event.payload);
                     if (!payload || ref_of(payload->references, "checkpoint") != std::to_string(id))
                       return std::unexpected(corrupt());
                     rows.checkpoints[id]       = retained_checkpoint{id, run, seq, *note};
                     rows.checkpoint_bodies[id] = body;
                     return {};
                   });
      !loaded)
    return std::unexpected(loaded.error());
  if (auto loaded = load_requests(db, rows); !loaded)
    return std::unexpected(loaded.error());
  if (auto loaded = load_role_states(db, rows); !loaded)
    return std::unexpected(loaded.error());
  if (auto loaded = load_run_states(db, rows); !loaded)
    return std::unexpected(loaded.error());
  if (auto loaded = load_current(db, rows); !loaded)
    return std::unexpected(loaded.error());
  if (auto loaded = load_tree_edges(db, rows); !loaded)
    return std::unexpected(loaded.error());

  if (auto checked_sequences = check_sequences(rows); !checked_sequences)
    return std::unexpected(checked_sequences.error());
  if (auto shapes = check_revision_shapes(rows); !shapes)
    return std::unexpected(shapes.error());
  if (auto edges = check_tree_edges(rows); !edges)
    return std::unexpected(edges.error());
  if (auto content = check_revision_content(rows, metadata); !content)
    return std::unexpected(content.error());

  replay_state state;
  if (auto replayed = replay_events(rows, metadata, state); !replayed)
    return std::unexpected(replayed.error());
  if (state.accepted.size() != rows.revisions.size())
    return std::unexpected(corrupt());
  for (const auto& [id, count] : state.accepted)
    if (count != 1 || !rows.revisions.contains(id))
      return std::unexpected(corrupt());
  if (state.activations_seen != rows.activations.size())
    return std::unexpected(corrupt());

  if (auto generations = check_generations(rows); !generations)
    return std::unexpected(generations.error());
  if (auto dispatches = check_dispatches(rows); !dispatches)
    return std::unexpected(dispatches.error());
  if (auto provenance = check_provenance(rows); !provenance)
    return std::unexpected(provenance.error());
  if (auto final_state = check_final_state(rows, state); !final_state)
    return std::unexpected(final_state.error());
  if (auto current = check_current(rows, state); !current)
    return std::unexpected(current.error());

  std::map<std::uint64_t, std::uint64_t> dispatch_revisions;
  auto                                   snapshot = build_snapshot(rows, state, metadata, dispatch_revisions);
  if (!snapshot)
    return std::unexpected(snapshot.error());
  if (auto review = read_review(db, *snapshot); !review) // R18: invariants 21 to 27, completions and review.
    return std::unexpected(review.error());
  if (auto checks = read_check_rows(db, *snapshot, rows); !checks)
    return std::unexpected(checks.error());
  for (auto& role : snapshot->roles) {
    auto lineage = lineage_generations(db, role.generation_id);
    if (!lineage)
      return std::unexpected(lineage.error());
    role.lineage = std::move(*lineage);
    if (role.lineage.empty())
      return std::unexpected(corrupt());
    role.lineage_opened = rows.generations.at(role.lineage.front()).opened_seq;
  }
  snapshot->next_checkpoint_id = rows.checkpoints.size() + 1;
  snapshot->state_bytes        = measure_state(*snapshot, dispatch_revisions);
  return snapshot;
}

bool is_identifier(std::string_view text);

// Reconstruct every retained check response from its rows, including historical reservations.
checked verify_check_results(connection& db, const ledger_rows& rows, std::string_view identity) {
  auto same = [](const value_ptr& a, const value_ptr& b) {
    if (!a || !b)
      return false;
    auto x = canonical_encode(a), y = canonical_encode(b);
    return x && y && *x == *y;
  };
  auto field = [](const value_ptr& v, std::string_view name) -> value_ptr {
    if (!v || v->kind != value_kind::record)
      return {};
    auto i = v->fields.find(std::string(name));
    return i == v->fields.end() ? value_ptr{} : i->second;
  };
  return for_each(
      db,
      "SELECT r.id,r.run_id,r.opened_seq,r.request_id,s.state,s.last_seq FROM reservations r JOIN reservation_state s ON "
      "s.reservation_id=r.id",
      [&](row_reader& q) -> checked {
        auto id = q.id(0), run = q.id(1), opened = q.id(2), last = q.id(5);
        auto request = q.text(3), state = q.text(4);
        if (state == "open")
          return {};
        if (!rows.runs.contains(run) || last == 0 || last > rows.events.size())
          return std::unexpected(corrupt());
        const auto& g     = rows.generations.at(rows.runs.at(run).generation);
        const auto& event = rows.events[last - 1];
        if (event.run != run || event.generation != g.id || !event.request_id)
          return std::unexpected(corrupt());
        auto payload = hist::decode_event_payload_v2(event.type, event.payload);
        if (!payload || ref_of(payload->references, "reservation") != std::to_string(id))
          return std::unexpected(corrupt());
        const dispatch_entry* dispatch = nullptr;
        for (const auto& [_, d] : rows.dispatches)
          if (d.run == run && d.seq < opened && (!dispatch || d.seq > dispatch->seq))
            dispatch = &d;
        if (!dispatch)
          return std::unexpected(corrupt());
        auto decoded = hist::decode_work_exec_v1(rows.records.at(dispatch->work).canonical);
        if (!decoded)
          return std::unexpected(corrupt());
        auto                                          work = *decoded;
        std::set<std::pair<std::string, std::string>> expected, actual;
        for (const auto& u : field(work, "units")->items) {
          auto name = u->kind == value_kind::string ? u : field(u, "id");
          if (!name || name->kind != value_kind::string)
            return std::unexpected(corrupt());
          expected.emplace(name->string, "planar.boundary_v1");
        }
        for (const auto& e : field(work, "effective_checks")->items) {
          auto w = field(e, "work"), checks = field(e, "checks");
          if (!w || !checks || w->kind != value_kind::string || checks->kind != value_kind::list)
            return std::unexpected(corrupt());
          for (const auto& c : checks->items) {
            if (c->kind != value_kind::string || !expected.emplace(w->string, c->string).second)
              return std::unexpected(corrupt());
          }
        }
        // Check vectors are per consuming work, including grouped internal producers.
        std::set<std::string>                                                own, observed_members;
        std::map<std::pair<std::string, std::string>, std::set<std::string>> vector_members;
        for (const auto& pair : expected) {
          if (pair.second == "planar.boundary_v1")
            own.insert(pair.first);
        }
        observed_members  = own;
        auto declarations = field(work, "checks"), dependencies = field(work, "dependencies");
        if (!declarations || declarations->kind != value_kind::list || !dependencies || dependencies->kind != value_kind::list)
          return std::unexpected(corrupt());
        for (const auto& pair : expected) {
          auto& targets = vector_members[pair];
          targets.insert(pair.first);
          if (pair.second == "planar.boundary_v1")
            continue;
          value_ptr declaration;
          for (const auto& d : declarations->items)
            if (same(field(d, "id"), text_node(pair.second)))
              declaration = d;
          auto inputs = field(declaration, "inputs");
          if (!inputs || inputs->kind != value_kind::list)
            return std::unexpected(corrupt());
          bool wants_vector = false;
          for (const auto& input : inputs->items)
            if (same(input, text_node("output_vector")))
              wants_vector = true;
          if (wants_vector)
            for (const auto& d : dependencies->items) {
              if (same(field(d, "consumer"), text_node(pair.first)) && same(field(d, "phase"), text_node("complete"))) {
                auto producer = field(d, "producer");
                if (!producer || producer->kind != value_kind::string || !is_identifier(producer->string))
                  return std::unexpected(corrupt());
                targets.insert(producer->string);
              }
            }
          observed_members.insert(targets.begin(), targets.end());
        }
        std::map<std::string, std::string> observed_outputs;
        if (state == "completed") {
          auto oid = parse_decimal(ref_of(payload->references, "observation"));
          if (!oid || !rows.records.contains(*oid))
            return std::unexpected(corrupt());
          auto observation = hist::decode_check_value(hist::codec_check_observation_v1, rows.records.at(*oid).canonical);
          auto units       = observation ? field(*observation, "units") : value_ptr{};
          if (!units || units->kind != value_kind::list)
            return std::unexpected(corrupt());
          std::string previous;
          for (const auto& unit : units->items) {
            auto name = field(unit, "work"), output = field(unit, "output");
            if (!unit || unit->kind != value_kind::record || unit->fields.size() != 2 || !name || !output ||
                name->kind != value_kind::string || output->kind != value_kind::string || !is_identifier(name->string) ||
                name->string <= previous || !digest_bytes_of_text(output->string))
              return std::unexpected(corrupt());
            previous = name->string;
            observed_outputs.emplace(name->string, output->string);
          }
          std::set<std::string> actual_observed;
          for (const auto& [name, _] : observed_outputs)
            actual_observed.insert(name);
          if (actual_observed != observed_members)
            return std::unexpected(corrupt());
        }
        std::vector<value_ptr> evidence;
        auto                   scanned = for_each(
            db,
            std::format("SELECT "
                        "id,work,\"check\",outcome,reason,matched,exit_status,duration_ms,output_bytes,output_truncated,output_"
                        "id,seq,definition_id,vector_id FROM evidence WHERE reservation_id={} ORDER BY work,\"check\"",
                        id),
            [&](row_reader& e) -> checked {
              auto eid = e.id(0);
              auto w = e.text(1), c = e.text(2), outcome = e.text(3), reason = e.text(4);
              auto matched = e.optional_number(5), exit = e.optional_number(6);
              auto duration = e.number(7), bytes = e.number(8), truncated = e.number(9);
              auto output = e.optional_id(10);
              auto seq = e.id(11), definition = e.id(12);
              auto vector_id = e.id(13);
              if (!rows.records.contains(vector_id))
                return std::unexpected(corrupt());
              auto vector = hist::decode_check_value(hist::codec_output_vector_v1, rows.records.at(vector_id).canonical);
              if (!vector)
                return std::unexpected(corrupt());
              std::set<std::string> actual_targets;
              for (const auto& target : field(*vector, "targets")->items) {
                auto name = field(target, "work"), output = field(target, "output");
                if (!name || !output || !observed_outputs.contains(name->string) ||
                    observed_outputs.at(name->string) != output->string)
                  return std::unexpected(corrupt());
                actual_targets.insert(name->string);
              }
              auto expected_targets = vector_members.find({w, c});
              if (expected_targets == vector_members.end() || actual_targets != expected_targets->second)
                return std::unexpected(corrupt());
              bool reason_ok =
                  (outcome == "pass" && reason == "passed") || (outcome == "inconclusive" && reason == "input_changed") ||
                  (outcome == "fail" &&
                   (reason == "exit_status" || reason == "zero_selected" || reason == "missing_executable" ||
                    reason == "timeout" || reason == "output_limit" || reason == "malformed_result" || reason == "boundary"));
              if (!reason_ok || seq != last || duration < 0 || bytes < 0 || (truncated != 0 && truncated != 1) ||
                  !actual.emplace(w, c).second || !rows.records.contains(definition))
                return std::unexpected(corrupt());
              auto d = hist::decode_check_value(hist::codec_check_definition_v1, rows.records.at(definition).canonical);
              if (!d || !same(field(*d, "work"), text_node(w)) || !same(field(*d, "check"), text_node(c)) ||
                  !same(field(*d, "gate"), text_node(c == "planar.boundary_v1" ? "boundary_v1" : "registry")))
                return std::unexpected(corrupt());
              std::set<std::string> definition_keys{"version", "work", "check", "gate", "inputs"};
              if (c == "planar.boundary_v1") {
                definition_keys.insert({"cwd", "owned", "frozen", "excluded", "producers"});
              } else {
                definition_keys.insert({"registry", "toolchain", "command", "environment", "reserved", "cwd", "timeout_ms",
                                        "max_output_bytes", "selection", "coverage"});
              }
              if ((*d)->fields.size() != definition_keys.size())
                return std::unexpected(corrupt());
              for (const auto& [name, _] : (*d)->fields)
                if (!definition_keys.contains(name))
                  return std::unexpected(corrupt());
              if (c != "planar.boundary_v1") {
                value_ptr declaration;
                for (const auto& x : field(work, "checks")->items)
                  if (same(field(x, "id"), text_node(c)))
                    declaration = x;
                if (!declaration)
                  return std::unexpected(corrupt());
                for (auto name : {"toolchain", "timeout_ms", "max_output_bytes", "selection", "inputs", "coverage"})
                  if (!same(field(*d, name), field(declaration, name)))
                    return std::unexpected(corrupt());
              }
              if (c == "planar.boundary_v1" && (exit || matched))
                return std::unexpected(corrupt());
              if (reason == "missing_executable" && exit)
                return std::unexpected(corrupt());
              evidence.push_back(record_node({{"evidence", text_node(std::to_string(eid))},
                                              {"work", text_node(w)},
                                              {"check", text_node(c)},
                                              {"outcome", text_node(outcome)},
                                              {"reason", text_node(reason)},
                                              {"matched", text_node(matched ? std::to_string(*matched) : "none")},
                                              {"exit_status", text_node(exit ? std::to_string(*exit) : "none")},
                                              {"duration_ms", integer_node(duration)},
                                              {"output_bytes", integer_node(bytes)},
                                              {"output_truncated", bool_node(truncated != 0)},
                                              {"output", text_node(output ? std::to_string(*output) : "none")}}));
              return {};
            });
        if (!scanned)
          return scanned;
        if ((state == "completed" && actual != expected) || (state == "interrupted" && !actual.empty()))
          return std::unexpected(corrupt());
        value_ptr observation = text_node("none"), completion = text_node("none");
        if (state == "completed") {
          auto oid = parse_decimal(ref_of(payload->references, "observation"));
          if (!oid || !rows.records.contains(*oid))
            return std::unexpected(corrupt());
          auto o = hist::decode_check_value(hist::codec_check_observation_v1, rows.records.at(*oid).canonical);
          if (!o)
            return std::unexpected(corrupt());
          auto digest = hash_value("check_observation_v1", *o);
          if (!digest)
            return std::unexpected(corrupt());
          observation = record_node(
              {{"id", text_node(std::to_string(*oid))}, {"digest", text_node(*digest)}, {"units", field(*o, "units")}});
          auto cid = payload->references.find("completion");
          if (cid != payload->references.end()) {
            auto n = parse_decimal(cid->second);
            if (!n)
              return std::unexpected(corrupt());
            bool found = false;
            auto check =
                for_each(db, std::format("SELECT run_id,seq,dispatch_id,work_fp,observation_id FROM completions WHERE id={}", *n),
                         [&](row_reader& c) -> checked {
                           if (c.id(0) != run || c.id(1) != last || c.id(2) != dispatch->id || c.blob(3) != dispatch->work_fp ||
                               c.id(4) != *oid)
                             return std::unexpected(corrupt());
                           found = true;
                           return {};
                         });
            if (!check || !found)
              return std::unexpected(corrupt());
            std::vector<value_ptr> publications;
            check = for_each(
                db, std::format("SELECT id,work,facet,semantic FROM publications WHERE completion_id={} ORDER BY work,facet", *n),
                [&](row_reader& p) -> checked {
                  publications.push_back(record_node({{"publication", text_node(std::to_string(p.id(0)))},
                                                      {"work", text_node(p.text(1))},
                                                      {"facet", text_node(p.text(2))},
                                                      {"semantic", text_node("xxh3_128:" + hex_of(p.blob(3)))}}));
                  return {};
                });
            if (!check)
              return check;
            completion = record_node({{"id", text_node(cid->second)},
                                      {"work_fp", text_node("xxh3_128:" + hex_of(dispatch->work_fp))},
                                      {"publications", list_node(std::move(publications))}});
          }
        }
        for (const auto& name : std::set<std::string>{request, *event.request_id}) {
          auto r = rows.requests.find(name);
          if (r == rows.requests.end())
            return std::unexpected(corrupt());
          auto saved = codec_decode(r->second.result);
          if (!saved)
            return std::unexpected(corrupt());
          auto withheld = field(*saved, "withheld");
          if (!withheld || withheld->kind != value_kind::list)
            return std::unexpected(corrupt());
          std::string prior;
          for (const auto& reason : withheld->items) {
            if (reason->kind != value_kind::string || reason->string <= prior)
              return std::unexpected(corrupt());
            prior = reason->string;
            if (prior != "input_changed" && prior != "boundary" && prior != "check_failed" && prior != "drifted" &&
                prior != "start_prerequisite" && prior != "complete_prerequisite" && prior != "retry_exhausted")
              return std::unexpected(corrupt());
          }
          bool completed = completion->kind == value_kind::record;
          if ((completed || state == "interrupted") && !withheld->items.empty())
            return std::unexpected(corrupt());
          auto expected_response = record_node({{"version", integer_node(1)},
                                                {"kind", text_node("check")},
                                                {"ledger", text_node(identity)},
                                                {"request_id", text_node(*event.request_id)},
                                                {"job", text_node(g.job)},
                                                {"role", text_node(g.role)},
                                                {"run", record_node({{"id", text_node(std::to_string(run))},
                                                                     {"generation", text_node(std::to_string(g.ordinal))}})},
                                                {"reservation", text_node(std::to_string(id))},
                                                {"outcome", text_node(state == "interrupted" ? "interrupted"
                                                                      : completed            ? "completed"
                                                                                             : "recorded")},
                                                {"withheld", withheld},
                                                {"evidence", list_node(evidence)},
                                                {"observation", observation},
                                                {"completion", completion}});
          if (!same(*saved, expected_response))
            return std::unexpected(corrupt());
        }
        return {};
      });
}

// Publication integrity uses the dispatch revision, never the active declaration or the live checkout.
// W lists consumed facets only, so an own facet's declaration comes from its retained interpretation.
checked verify_publication(const ledger_rows& rows, std::uint64_t dispatch_id, std::string_view work_id,
                           std::string_view facet_id, std::uint64_t payload_id, std::string_view semantic) {
  auto field = [](const value_ptr& v, std::string_view key) -> value_ptr {
    if (!v || v->kind != value_kind::record)
      return {};
    auto i = v->fields.find(std::string(key));
    return i == v->fields.end() ? value_ptr{} : i->second;
  };
  auto is_text = [](const value_ptr& v, std::string_view text) {
    return v && v->kind == value_kind::string && v->string == text;
  };
  auto d = rows.dispatches.find(dispatch_id);
  if (d == rows.dispatches.end())
    return std::unexpected(corrupt());
  auto r = rows.revisions.find(d->second.revision);
  if (r == rows.revisions.end() || !rows.records.contains(d->second.work) || !rows.records.contains(r->second.interpretation))
    return std::unexpected(corrupt());
  auto decoded_work   = hist::decode_work_exec_v1(rows.records.at(d->second.work).canonical);
  auto interpretation = canonical_decode(rows.records.at(r->second.interpretation).canonical);
  if (!decoded_work || !interpretation)
    return std::unexpected(corrupt());
  auto work  = *decoded_work;
  auto units = field(work, "units"), root = field(work, "root");
  auto spec   = field(field(*interpretation, "bundle"), "spec");
  auto facets = field(spec, "facets");
  if (!units || units->kind != value_kind::list || !root || root->kind != value_kind::string || !facets ||
      facets->kind != value_kind::list || !field(spec, "id") || !is_text(field(work, "spec"), field(spec, "id")->string))
    return std::unexpected(corrupt());
  bool owns = false;
  for (const auto& u : units->items)
    owns = owns || is_text(u->kind == value_kind::string ? u : field(u, "id"), work_id);
  if (!owns)
    return std::unexpected(corrupt());
  value_ptr declaration;
  for (const auto& f : facets->items)
    if (is_text(field(f, "id"), facet_id)) {
      if (declaration)
        return std::unexpected(corrupt());
      declaration = f;
    }
  auto paths = field(declaration, "paths");
  if (!is_text(field(declaration, "producer"), work_id) || !paths || paths->kind != value_kind::list || paths->items.empty())
    return std::unexpected(corrupt());
  auto files = read_tree(rows, payload_id, hist::tree_profile::source);
  if (!files || files->size() != paths->items.size())
    return std::unexpected(corrupt());
  std::vector<value_ptr> entries;
  std::string            previous;
  std::uint64_t          total = 0;
  for (std::size_t i = 0; i < files->size(); ++i) {
    const auto& file = (*files)[i];
    const auto& path = paths->items[i];
    if (!is_text(path, file.path) || file.path <= previous || (file.mode != 420 && file.mode != 493))
      return std::unexpected(corrupt());
    previous = file.path;
    total += file.payload.size();
    if (total > 4194304)
      return std::unexpected(corrupt());
    auto content = file_digest(file.payload);
    if (!content)
      return std::unexpected(corrupt());
    entries.push_back(record_node({{"root", root},
                                   {"path", text_node(file.path)},
                                   {"kind", text_node("file")},
                                   {"mode", text_node(file.mode == 493 ? "0755" : "0644")},
                                   {"size", integer_node(file.payload.size())},
                                   {"content", text_node(*content)}}));
  }
  auto computed = hash_value("facet_v1", record_node({{"kind", text_node("files")},
                                                      {"normalizer", text_node("exact_bytes_v1")},
                                                      {"root", root},
                                                      {"paths", paths},
                                                      {"entries", list_node(std::move(entries))}}));
  if (!computed || *computed != "xxh3_128:" + hex_of(semantic))
    return std::unexpected(corrupt());
  return {};
}

checked read_check_rows(connection& db, execution_snapshot& snapshot, const ledger_rows& rows) {
  if (auto verified = verify_check_results(db, rows, snapshot.identity); !verified)
    return verified;
  auto value_record = [&](std::uint64_t id, std::string_view codec) -> value_result<value_ptr> {
    auto record = rows.records.find(id);
    if (record == rows.records.end() || record->second.value_codec != codec)
      return std::unexpected(corrupt());
    auto decoded = hist::decode_check_value(codec, record->second.canonical);
    if (!decoded)
      return std::unexpected(corrupt());
    return *decoded;
  };
  auto reservations =
      for_each(db,
               "SELECT r.id,r.request_id,r.run_id,r.opened_seq,r.body_id,s.state,s.last_seq FROM reservations r LEFT JOIN "
               "reservation_state s ON s.reservation_id=r.id ORDER BY r.id",
               [&](row_reader& q) -> checked {
                 execution_reservation r;
                 r.id         = q.id(0);
                 r.request_id = q.text(1);
                 r.run        = q.id(2);
                 r.opened_seq = q.id(3);
                 auto body    = q.id(4);
                 auto state   = q.text(5);
                 auto last    = q.id(6);
                 if (r.opened_seq > rows.events.size() || last > rows.events.size() || !rows.runs.contains(r.run) ||
                     !rows.records.contains(body))
                   return std::unexpected(corrupt());
                 const auto& event = rows.events[r.opened_seq - 1];
                 if (event.type != "check_reserved" || event.run != r.run || event.request_id != r.request_id)
                   return std::unexpected(corrupt());
                 const auto request = rows.requests.find(r.request_id);
                 if (state == "open") {
                   if (last != r.opened_seq || request != rows.requests.end())
                     return std::unexpected(corrupt());
                   snapshot.reservations.push_back(r);
                 } else if (state == "completed" || state == "interrupted") {
                   if (last <= r.opened_seq || request == rows.requests.end() || request->second.body != body ||
                       request->second.event_seq != r.opened_seq ||
                       rows.events[last - 1].type != (state == "completed" ? "check_recorded" : "check_interrupted"))
                     return std::unexpected(corrupt());
                 } else
                   return std::unexpected(corrupt());
                 return {};
               });
  if (!reservations)
    return reservations;
  auto count = for_each(db, "SELECT (SELECT COUNT(*) FROM reservations),(SELECT COUNT(*) FROM reservation_state)",
                        [](row_reader& q) -> checked {
                          if (q.number(0) != q.number(1))
                            return std::unexpected(corrupt());
                          return {};
                        });
  if (!count)
    return count;
  auto owns = [&](const execution_role& role, std::string_view work) {
    return role.job == snapshot.active_job && role.state != "retired" && role.structure.kind == "worker" &&
           std::ranges::find(role.structure.work, work) != role.structure.work.end();
  };
  std::map<std::pair<std::string, std::string>, execution_evidence> latest;
  auto                                                              evidence =
      for_each(db,
               "SELECT "
               "e.id,e.reservation_id,e.seq,e.work,e.\"check\",e.outcome,e.reason,e.matched,e.vector_id,e.definition_id,r.run_id,"
               "s.job,s.role,s.generation_id,e.output_id,e.output_bytes FROM evidence e LEFT JOIN reservations r ON "
               "r.id=e.reservation_id LEFT JOIN run_state s ON s.run_id=r.run_id ORDER BY e.seq,e.id",
               [&](row_reader& q) -> checked {
                 execution_evidence e;
                 e.id          = q.id(0);
                 e.reservation = q.id(1);
                 e.seq         = q.id(2);
                 e.work        = q.text(3);
                 e.check       = q.text(4);
                 e.outcome     = q.text(5);
                 e.reason      = q.text(6);
                 auto matched  = q.optional_number(7);
                 if (matched) {
                   if (*matched < 0)
                     return std::unexpected(corrupt());
                   e.matched = static_cast<std::uint64_t>(*matched);
                 }
                 auto vector = q.id(8), definition = q.id(9), run = q.id(10);
                 e.job           = q.text(11);
                 e.role          = q.text(12);
                 auto generation = q.id(13);
                 auto output     = q.optional_id(14);
                 auto bytes      = q.number(15);
                 if (e.seq > rows.events.size() || rows.events[e.seq - 1].type != "check_recorded" ||
                     rows.events[e.seq - 1].run != run || bytes < 0)
                   return std::unexpected(corrupt());
                 auto v = value_record(vector, hist::codec_output_vector_v1),
                      d = value_record(definition, hist::codec_check_definition_v1);
                 if (!v || !d)
                   return std::unexpected(corrupt());
                 e.vector = *v;
                 if (output && (!rows.records.contains(*output) || rows.records.at(*output).kind != 1))
                   return std::unexpected(corrupt());
                 const auto role =
                     std::ranges::find_if(snapshot.roles, [&](const auto& r) { return r.job == e.job && r.role == e.role; });
                 if (role != snapshot.roles.end() && owns(*role, e.work)) {
                   auto lineage = lineage_generations(db, role->generation_id);
                   if (!lineage)
                     return std::unexpected(lineage.error());
                   if (std::ranges::find(*lineage, generation) != lineage->end())
                     latest[{e.work, e.check}] = e;
                 }
                 return {};
               });
  if (!evidence)
    return evidence;
  for (auto& [key, e] : latest)
    snapshot.evidence.push_back(std::move(e));
  std::map<std::string, execution_observed> observed;
  // Every completion is verified against its check_recorded event. Scan those events,
  // including failed/inconclusive checks, once in order and select only original own
  // units of the current owning lineage. Completion observations also contain consumed
  // producer units and cannot independently seed authoritative observed outputs.
  for (const auto& event : rows.events)
    if (event.type == "check_recorded") {
      auto payload = hist::decode_event_payload_v2(event.type, event.payload);
      if (!payload)
        return std::unexpected(corrupt());
      auto id = parse_decimal(ref_of(payload->references, "observation"));
      if (!id)
        return std::unexpected(corrupt());
      auto obs = value_record(*id, hist::codec_check_observation_v1);
      if (!obs)
        return std::unexpected(obs.error());
      auto run = rows.runs.find(event.run.value_or(0));
      if (run == rows.runs.end())
        return std::unexpected(corrupt());
      auto gen = generation_of(rows, run->second.generation);
      if (!gen)
        return std::unexpected(corrupt());
      if (gen->job != snapshot.active_job)
        continue;
      const auto selected_role =
          std::ranges::find_if(snapshot.roles, [&](const auto& r) { return r.job == gen->job && r.role == gen->role; });
      if (selected_role == snapshot.roles.end() || selected_role->state == "retired")
        continue;
      auto lineage = lineage_generations(db, selected_role->generation_id);
      if (!lineage)
        return std::unexpected(lineage.error());
      if (std::ranges::find(*lineage, gen->id) == lineage->end())
        continue;
      auto structure = value_of(rows, gen->structure, "role_structure_v1");
      if (!structure)
        return std::unexpected(corrupt());
      auto shape = hist::decode_role_structure_v1((*structure)->canonical);
      if (!shape)
        return std::unexpected(corrupt());
      for (const auto& u : (*obs)->fields.at("units")->items) {
        if (!u || u->kind != value_kind::record || !u->fields.contains("work") || !u->fields.contains("output"))
          return std::unexpected(corrupt());
        auto work = u->fields.at("work")->string;
        if (owns(*selected_role, work) && std::ranges::find(shape->work, work) != shape->work.end())
          observed[work] = {work, u->fields.at("output")->string, event.seq};
      }
    }
  for (auto& [key, o] : observed)
    snapshot.observed.push_back(std::move(o));
  auto publications =
      for_each(db,
               "SELECT p.id,p.completion_id,p.work,p.facet,p.semantic,p.payload_id,c.dispatch_id FROM publications p "
               "LEFT JOIN completions c ON c.id=p.completion_id ORDER BY p.id",
               [&](row_reader& q) -> checked {
                 execution_publication p;
                 p.publication = q.id(0);
                 p.completion  = q.id(1);
                 p.work        = q.text(2);
                 p.facet       = q.text(3);
                 auto semantic = q.blob(4);
                 auto payload  = q.id(5);
                 auto dispatch = q.id(6);
                 if (auto valid = verify_publication(rows, dispatch, p.work, p.facet, payload, semantic); !valid)
                   return valid;
                 if (semantic.size() != 16 || !rows.records.contains(payload) || rows.records.at(payload).kind != 2)
                   return std::unexpected(corrupt());
                 p.semantic = "xxh3_128:" + hex_of(semantic);
                 auto c     = std::ranges::find(snapshot.completions, p.completion, &execution_completion::id);
                 if (c == snapshot.completions.end())
                   return {};
                 const auto role = std::ranges::find_if(
                     snapshot.roles, [&](const auto& r) { return r.job == c->job && r.role == c->role && owns(r, p.work); });
                 if (role == snapshot.roles.end())
                   return {};
                 p.job      = c->job;
                 p.role     = c->role;
                 auto newer = std::ranges::any_of(snapshot.completions, [&](const auto& other) {
                   return other.job == c->job && other.role == c->role && other.seq > c->seq;
                 });
                 if (!newer)
                   snapshot.publications.push_back(p);
                 return {};
               });
  if (!publications)
    return publications;
  std::ranges::sort(snapshot.publications,
                    [](const auto& a, const auto& b) { return std::tie(a.work, a.facet) < std::tie(b.work, b.facet); });
  return {};
}

// Open the ledger, then verify it inside one read transaction so the rows form one consistent snapshot.
value_result<execution_snapshot> read_verified(execution_database& db, ledger_rows& rows) {
  auto tx = db.sql().begin(false);
  if (!tx)
    return std::unexpected(map_sqlite(tx.error()));
  return verify_rows(db.sql(), db.metadata(), rows);
}

std::optional<std::array<std::uint8_t, 16>> parse_secret(std::string_view hex) {
  if (hex.size() != 32)
    return std::nullopt;
  std::array<std::uint8_t, 16> out{};
  for (std::size_t i = 0; i < out.size(); ++i) {
    const auto high = hex_nibble(hex[2 * i]);
    const auto low  = hex_nibble(hex[2 * i + 1]);
    if (!high || !low)
      return std::nullopt;
    out[i] = static_cast<std::uint8_t>((*high << 4) | *low);
  }
  return out;
}

} // namespace

// ---------------------------------------------------------------------------------------------------------------------
// Public reader, lookup and token entry points.

std::expected<execution_snapshot, execution_error> read_execution_snapshot(const std::filesystem::path& path) {
  auto db = open_format3(path, execution_access::read_only);
  if (!db)
    return std::unexpected(db.error());
  ledger_rows rows;
  auto        snapshot = read_verified(*db, rows);
  if (!snapshot)
    return std::unexpected(snapshot.error());
  if (snapshot->state_bytes > snapshot_limit) {
    execution_error error = failure(execution_error_code::resource_limit);
    error.limit           = "execution_snapshot_bytes";
    error.maximum         = snapshot_limit;
    return std::unexpected(error);
  }
  return snapshot;
}

std::expected<void, execution_error> verify_execution_ledger(const std::filesystem::path& path) {
  auto db = open_format3(path, execution_access::read_only);
  if (!db)
    return std::unexpected(db.error());
  ledger_rows rows;
  auto        snapshot = read_verified(*db, rows);
  if (!snapshot)
    return std::unexpected(snapshot.error());
  return {};
}

std::uint64_t execution_state_bytes(const execution_snapshot& snapshot) {
  // A1: each dispatch is measured at its own recorded revision, exactly as the reader measures it.
  std::map<std::uint64_t, std::uint64_t> revisions;
  for (const auto& row : snapshot.roles)
    if (row.dispatch && row.dispatch->revision != 0)
      revisions[row.dispatch->id] = row.dispatch->revision;
  return measure_state(snapshot, revisions);
}

std::expected<request_lookup, execution_error>
lookup_execution_request(const std::filesystem::path& path, std::string_view request_id, const execution_framing& framing) {
  auto db = open_format3(path, execution_access::read_only);
  if (!db)
    return std::unexpected(db.error());
  ledger_rows rows;
  if (auto verified = read_verified(*db, rows); !verified)
    return std::unexpected(verified.error());
  const auto  found = rows.requests.find(std::string(request_id));
  std::string frame;
  if (const auto* version_two = std::get_if<hist::request_v2>(&framing)) {
    auto encoded = hist::encode_request_v2(*version_two);
    if (!encoded)
      return std::unexpected(failure(execution_error_code::invalid_change));
    frame = *encoded;
  } else {
    auto encoded = hist::encode_request_v3(std::get<hist::request_v3>(framing));
    if (!encoded)
      return std::unexpected(failure(execution_error_code::invalid_change));
    frame = *encoded;
  }
  if (found == rows.requests.end()) {
    auto query = db->sql().prepare(
        "SELECT r.body_id,s.state FROM reservations r JOIN reservation_state s ON s.reservation_id=r.id WHERE r.request_id=?1");
    if (!query)
      return std::unexpected(map_sqlite(query.error()));
    if (auto bound = query->bind_text(1, request_id); !bound)
      return std::unexpected(map_sqlite(bound.error()));
    auto step = query->step();
    if (!step)
      return std::unexpected(map_sqlite(step.error()));
    if (*step == step_result::row) {
      row_reader q(*query);
      auto       body  = q.id(0);
      auto       state = q.text(1);
      if (auto ok = q.finish(); !ok)
        return std::unexpected(ok.error());
      if (!rows.records.contains(body))
        return std::unexpected(corrupt());
      if (rows.records.at(body).encoded != frame)
        return request_lookup{request_state::conflict, {}};
      if (state == "open")
        return std::unexpected(failure(execution_error_code::check_in_progress));
      return std::unexpected(corrupt());
    }
    return request_lookup{request_state::unused, {}};
  }
  const auto& stored = rows.records.at(found->second.body);
  if (stored.encoded != frame)
    return request_lookup{request_state::conflict, {}};
  return request_lookup{request_state::replay, found->second.result};
}

std::string format_run_token(const run_token& token) {
  std::string out = std::format("{}.{}.", token.run_id, token.generation_ordinal);
  for (auto byte : token.secret)
    out += std::format("{:02x}", byte);
  return out;
}

std::optional<run_token> parse_run_token(std::string_view text) {
  const auto first = text.find('.');
  if (first == std::string_view::npos)
    return std::nullopt;
  const auto second = text.find('.', first + 1);
  if (second == std::string_view::npos)
    return std::nullopt;
  const auto run        = parse_decimal(text.substr(0, first));
  const auto generation = parse_decimal(text.substr(first + 1, second - first - 1));
  const auto secret     = parse_secret(text.substr(second + 1));
  if (!run || !generation || !secret)
    return std::nullopt;
  return run_token{*run, *generation, *secret};
}

std::expected<void, execution_error> authorize_run_token(const std::filesystem::path& path, std::string_view token_text) {
  auto db = open_format3(path, execution_access::read_only);
  if (!db)
    return std::unexpected(db.error());
  ledger_rows rows;
  if (auto verified = read_verified(*db, rows); !verified)
    return std::unexpected(verified.error());
  const auto token = parse_run_token(token_text);
  if (!token)
    return std::unexpected(failure(execution_error_code::unknown_run));
  const auto run = rows.runs.find(token->run_id);
  if (run == rows.runs.end())
    return std::unexpected(failure(execution_error_code::unknown_run));
  const auto* generation = generation_of(rows, run->second.generation);
  if (!generation || generation->ordinal != token->generation_ordinal)
    return std::unexpected(failure(execution_error_code::unknown_run));
  std::string state;
  for (const auto& row : rows.run_states)
    if (row.run == token->run_id)
      state = row.state;
  if (state.empty())
    return std::unexpected(corrupt());
  bool same_secret = true;
  for (std::size_t i = 0; i < token->secret.size(); ++i)
    same_secret = same_secret && static_cast<std::uint8_t>(run->second.secret[i]) == token->secret[i];
  if (state == "active" && same_secret)
    return {};
  execution_error error = failure(execution_error_code::fenced);
  error.state           = state;
  return std::unexpected(error);
}

// ---------------------------------------------------------------------------------------------------------------------
// Commit protocol (D12, D13). Shape, request, snapshot and native checks run before the lock. The write transaction then
// verifies the format, relooks up the request, checks the load compare-and-swap, evaluates the preconditions in the EX-3
// vocabulary order, rechecks the locked before states, writes every row, and runs the shared verifier before COMMIT.

namespace {

enum class bind_kind { integer, text, blob, absent };

/// One value bound to a statement parameter by its one-based position.
struct bind_arg {
  bind_kind    kind   = bind_kind::absent;
  std::int64_t number = 0;
  std::string  bytes;
};

bind_arg int_arg(std::uint64_t value) {
  return bind_arg{bind_kind::integer, static_cast<std::int64_t>(value), {}};
}

bind_arg text_arg(std::string_view text) {
  return bind_arg{bind_kind::text, 0, std::string(text)};
}

bind_arg blob_arg(std::string_view bytes) {
  return bind_arg{bind_kind::blob, 0, std::string(bytes)};
}

bind_arg null_arg() {
  return bind_arg{};
}

bind_arg optional_int_arg(const std::optional<std::uint64_t>& value) {
  return value ? int_arg(*value) : null_arg();
}

checked bind_args(statement& stmt, const std::vector<bind_arg>& args) {
  int index = 1;
  for (const auto& arg : args) {
    std::expected<void, sqlite_error> result{};
    switch (arg.kind) {
    case bind_kind::integer:
      result = stmt.bind_integer(index, arg.number);
      break;
    case bind_kind::text:
      result = stmt.bind_text(index, arg.bytes);
      break;
    case bind_kind::blob:
      result = stmt.bind_blob(index, arg.bytes);
      break;
    case bind_kind::absent:
      result = stmt.bind_null(index);
      break;
    }
    if (!result)
      return std::unexpected(map_sqlite(result.error()));
    ++index;
  }
  return {};
}

value_result<statement> prepare_bound(connection& db, std::string_view sql, const std::vector<bind_arg>& args) {
  auto stmt = db.prepare(sql);
  if (!stmt)
    return std::unexpected(map_sqlite(stmt.error()));
  if (auto bound = bind_args(*stmt, args); !bound)
    return std::unexpected(bound.error());
  return std::move(*stmt);
}

/// Run one statement that returns no row and require it to finish.
checked run_bound(connection& db, std::string_view sql, const std::vector<bind_arg>& args) {
  auto stmt = prepare_bound(db, sql, args);
  if (!stmt)
    return std::unexpected(stmt.error());
  auto step = stmt->step();
  if (!step)
    return std::unexpected(map_sqlite(step.error()));
  if (*step != step_result::done)
    return std::unexpected(failure(execution_error_code::io));
  return {};
}

/// The first row of a bound query, or nothing when the query returns no row.
value_result<std::optional<statement>> first_row(connection& db, std::string_view sql, const std::vector<bind_arg>& args = {}) {
  auto stmt = prepare_bound(db, sql, args);
  if (!stmt)
    return std::unexpected(stmt.error());
  auto step = stmt->step();
  if (!step)
    return std::unexpected(map_sqlite(step.error()));
  if (*step == step_result::done)
    return std::optional<statement>{};
  return std::optional<statement>(std::move(*stmt));
}

/// One more than the single integer of a query that returns the current maximum; the first identifier is one.
value_result<std::uint64_t> next_value(connection& db, std::string_view sql, const std::vector<bind_arg>& args = {}) {
  auto found = first_row(db, sql, args);
  if (!found)
    return std::unexpected(found.error());
  if (!*found)
    return std::unexpected(corrupt());
  row_reader reader(**found);
  const auto current = reader.number(0);
  if (auto finished = reader.finish(); !finished)
    return std::unexpected(finished.error());
  if (current < 0)
    return std::unexpected(corrupt());
  return static_cast<std::uint64_t>(current) + 1;
}

bool is_identifier(std::string_view text) {
  if (text.empty() || text.size() > 64)
    return false;
  if (text.front() < 'a' || text.front() > 'z')
    return false;
  return std::ranges::all_of(text, [](char c) { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'; });
}

bool is_role_state(std::string_view text) {
  constexpr std::array<std::string_view, 6> states{"blocked", "ready", "running", "done", "stale", "retired"};
  return std::ranges::find(states, text) != states.end();
}

bool is_generation_reason(std::string_view text) {
  constexpr std::array<std::string_view, 4> reasons{"activated", "structural_change", "takeover", "recreated"};
  return std::ranges::find(reasons, text) != reasons.end();
}

execution_error invalid_error() {
  return failure(execution_error_code::invalid_change);
}

/// Encoder refusals: EX-8 bounds keep their names when the caller allows them; every other refusal is invalid_change.
execution_error codec_error(const hist::execution_codec_error& error, bool limits) {
  if (limits && error.code == hist::execution_codec_code::resource_limit) {
    execution_error out = failure(execution_error_code::resource_limit);
    out.limit           = error.limit;
    out.maximum         = error.maximum;
    return out;
  }
  return invalid_error();
}

/// Fixed-width UTC timestamp of this commit, shared by the event and the revision it accepts.
std::string commit_timestamp() {
  using namespace std::chrono;
  const auto           now = floor<milliseconds>(system_clock::now());
  const auto           day = floor<days>(now);
  const year_month_day ymd(day);
  const hh_mm_ss       tod(now - day);
  return std::format("{:04}-{:02}-{:02}T{:02}:{:02}:{:02}.{:03}Z", int(ymd.year()), unsigned(ymd.month()), unsigned(ymd.day()),
                     tod.hours().count(), tod.minutes().count(), tod.seconds().count(), tod.subseconds().count());
}

/// Sixteen bytes from the operating system entropy source, used only for run secrets (D8).
value_result<std::array<std::uint8_t, 16>> run_secret() {
  std::array<std::uint8_t, 16> secret{};
  if (::getentropy(secret.data(), secret.size()) != 0)
    return std::unexpected(failure(execution_error_code::io));
  return secret;
}

/// A record to store: its kind and codec, the complete frame, the sixteen raw digest bytes and the collision label.
struct artifact {
  std::int64_t kind  = 0;
  std::int64_t codec = 0;
  std::string  frame;
  std::string  digest;
  std::string  label;
};

/// Every byte the commit stores, framed and digested before the lock (D12 step 1).
struct prepared_change {
  artifact                request;
  std::optional<artifact> packet;
  std::optional<artifact> manifest;
  std::optional<artifact> work;
  std::optional<artifact> provenance;
  std::optional<artifact> verdict; // R18: verdict_v1 body of a verdict change.
  std::optional<artifact> checkpoint;
  std::vector<artifact>   structures;
  std::string             markdown;
  std::string             render_text;
  std::string             work_text;
};

value_result<artifact> value_artifact(std::string_view codec, const std::string& canonical) {
  auto frame = hist::encode_execution_value_record(codec, canonical);
  if (!frame)
    return std::unexpected(invalid_error());
  auto digest = hist::digest_execution_value_record(*frame);
  if (!digest)
    return std::unexpected(invalid_error());
  return artifact{3, 1, *frame, digest_bytes(*digest), "value"};
}

value_result<prepared_change> prepare_change(const execution_change& change) {
  prepared_change out;
  if (change.operation == "load") {
    const auto* framed = std::get_if<hist::request_v2>(&change.request);
    if (!framed)
      return std::unexpected(invalid_error());
    auto frame  = hist::encode_request_v2(*framed);
    auto digest = hist::digest_request_v2(*framed);
    if (!frame || !digest)
      return std::unexpected(invalid_error());
    out.request = artifact{4, 2, *frame, digest_bytes(*digest), "request"};
  } else {
    const auto* framed = std::get_if<hist::request_v3>(&change.request);
    if (!framed)
      return std::unexpected(invalid_error());
    auto frame  = hist::encode_request_v3(*framed);
    auto digest = hist::digest_request_v3(*framed);
    if (!frame || !digest)
      return std::unexpected(invalid_error());
    out.request = artifact{4, 3, *frame, digest_bytes(*digest), "request"};
  }

  if (change.packet) {
    const auto& packet = *change.packet;
    if (!packet.manifest || !packet.work)
      return std::unexpected(invalid_error());
    const hist::record file   = hist::file_record{.mode = 420, .payload = packet.markdown};
    auto               frame  = hist::encode_record(file);
    auto               digest = hist::digest_record(file);
    if (!frame || !digest)
      return std::unexpected(invalid_error());
    out.packet   = artifact{1, 1, *frame, digest_bytes(*digest), "file"};
    out.markdown = packet.markdown;
    auto render  = ::planar::engine::fragment_digest(packet.markdown);
    if (!render)
      return std::unexpected(invalid_error());
    out.render_text = *render;
    auto work_text  = ::planar::engine::hash_value("work_exec_v1", packet.work);
    if (!work_text)
      return std::unexpected(invalid_error());
    out.work_text = *work_text;

    auto manifest_bytes = hist::encode_dispatch_manifest_v1(packet.manifest);
    if (!manifest_bytes)
      return std::unexpected(codec_error(manifest_bytes.error(), true));
    auto manifest = value_artifact(hist::codec_dispatch_manifest_v1, *manifest_bytes);
    if (!manifest)
      return std::unexpected(manifest.error());
    out.manifest = std::move(*manifest);

    auto work_bytes = hist::encode_work_exec_v1(packet.work);
    if (!work_bytes)
      return std::unexpected(codec_error(work_bytes.error(), false));
    auto work = value_artifact(hist::codec_work_exec_v1, *work_bytes);
    if (!work)
      return std::unexpected(work.error());
    out.work = std::move(*work);
  }

  if (change.provenance) {
    auto canonical = hist::encode_driver_provenance_v1(change.provenance);
    if (!canonical)
      return std::unexpected(codec_error(canonical.error(), true));
    auto stored = value_artifact(hist::codec_driver_provenance_v1, *canonical);
    if (!stored)
      return std::unexpected(stored.error());
    out.provenance = std::move(*stored);
  }

  if (change.verdict) { // R18: the normalized verdict document as a verdict_v1 value record.
    auto canonical = hist::encode_verdict_v1(change.verdict->verdict);
    if (!canonical)
      return std::unexpected(invalid_error());
    auto stored = value_artifact(hist::codec_verdict_v1, *canonical);
    if (!stored)
      return std::unexpected(stored.error());
    out.verdict = std::move(*stored);
  }

  if (change.checkpoint) {
    auto bytes = hist::encode_checkpoint_v1(change.checkpoint);
    if (!bytes)
      return std::unexpected(codec_error(bytes.error(), true));
    auto stored = value_artifact("checkpoint_v1", *bytes);
    if (!stored)
      return std::unexpected(stored.error());
    out.checkpoint = std::move(*stored);
  }
  for (const auto& generation : change.generations) {
    auto canonical = hist::encode_role_structure_v1(generation.structure);
    if (!canonical)
      return std::unexpected(invalid_error());
    auto structure = value_artifact(hist::codec_role_structure_v1, *canonical);
    if (!structure)
      return std::unexpected(structure.error());
    out.structures.push_back(std::move(*structure));
  }
  return out;
}

const execution_role* snapshot_role(const execution_snapshot& snapshot, std::string_view job, std::string_view role) {
  for (const auto& row : snapshot.roles)
    if (row.job == job && row.role == role)
      return &row;
  return nullptr;
}

execution_expectation state_member_of(const execution_role& row) {
  execution_expectation out;
  out.kind    = execution_expectation::kind_t::role_state;
  out.job     = row.job;
  out.role    = row.role;
  out.state   = row.state;
  out.drifted = row.drifted;
  out.run     = row.run_id;
  return out;
}

/// The comparison key of one member: job and role only for the kinds that use them.
std::string member_key(const execution_expectation& member) {
  using kind_t    = execution_expectation::kind_t;
  const auto kind = static_cast<int>(member.kind);
  switch (member.kind) {
  case kind_t::fact_version:
  case kind_t::role_state:
  case kind_t::generation_current:
    return std::format("{}|{}|{}", kind, member.job, member.role);
  default:
    return std::format("{}||", kind);
  }
}

/// Whether a member carries the snapshot value its kind compares.
bool same_value(const execution_expectation& expected, const execution_expectation& given) {
  using kind_t = execution_expectation::kind_t;
  switch (expected.kind) {
  case kind_t::active_revision:
    return expected.revision == given.revision;
  case kind_t::active_job:
    return expected.text == given.text;
  case kind_t::fact_version:
  case kind_t::generation_current:
    return expected.number == given.number;
  case kind_t::run_active:
    return expected.run == given.run;
  case kind_t::role_state:
    return expected.state == given.state && expected.drifted == given.drifted && expected.run == given.run;
  default:
    return true;
  }
}

/// The fixed expected set of one operation, computed from the snapshot rows (D37 a, EX-3 581-591).
value_result<std::vector<execution_expectation>> fixed_members(const execution_snapshot& snapshot,
                                                               const execution_change&   change) {
  using kind_t = execution_expectation::kind_t;
  std::vector<execution_expectation> out;
  execution_expectation              revision;
  revision.kind = kind_t::active_revision;
  if (snapshot.revision)
    revision.revision = snapshot.revision->id;
  out.push_back(revision);
  execution_expectation active;
  active.kind = kind_t::active_job;
  active.text = snapshot.active_job;
  out.push_back(active);

  const bool token_op = change.operation == "refresh" || change.operation == "checkpoint" || change.operation == "release";
  const bool targeted = change.operation == "claim" || change.operation == "takeover" || token_op;
  std::vector<std::string> jobs;
  if (!snapshot.active_job.empty())
    jobs.push_back(snapshot.active_job);
  if (change.operation == "activate" && std::ranges::find(jobs, change.job) == jobs.end())
    jobs.push_back(change.job);
  for (const auto& row : snapshot.roles) {
    if (std::ranges::find(jobs, row.job) == jobs.end())
      continue;
    if (change.operation == "checkpoint")
      continue;
    execution_expectation fact;
    fact.kind   = kind_t::fact_version;
    fact.job    = row.job;
    fact.role   = row.role;
    fact.number = row.fact_version;
    out.push_back(fact);
    if (!targeted)
      out.push_back(state_member_of(row));
  }
  if (targeted) {
    const auto* target = snapshot_role(snapshot, change.job, change.role);
    if (!target)
      return std::unexpected(invalid_error());
    execution_expectation state = state_member_of(*target);
    if (change.operation == "takeover") {
      state.state   = "running";
      state.drifted = target->drifted;
      state.run     = change.previous_run;
    }
    if (change.operation != "checkpoint")
      out.push_back(state);
    execution_expectation generation;
    generation.kind   = kind_t::generation_current;
    generation.job    = target->job;
    generation.role   = target->role;
    generation.number = target->generation_id;
    out.push_back(generation);
    if (token_op) {
      execution_expectation auth;
      auth.kind = kind_t::run_active;
      auth.run  = change.run;
      out.push_back(auth);
    }
  }
  execution_expectation unused;
  unused.kind = kind_t::request_unused;
  out.push_back(unused);
  return out;
}

checked check_shape(const execution_change& change) {
  if (change.operation == "verdict") // R18: execution_review.cpp
    return check_verdict_shape(change);
  if (change.verdict) // R18: verdict facts belong to operation verdict only.
    return std::unexpected(invalid_error());
  const auto& op       = change.operation;
  const bool  load     = op == "load";
  const bool  activate = op == "activate";
  const bool  claim    = op == "claim";
  const bool  takeover = op == "takeover";
  const bool  token_op = op == "refresh" || op == "checkpoint" || op == "release";
  if (!load && !activate && !claim && !takeover && !token_op)
    return std::unexpected(invalid_error());
  if (load) {
    if (!change.revision || (!change.job.empty() && !is_identifier(change.job)) || !change.role.empty() || change.previous_run ||
        change.packet || change.provenance)
      return std::unexpected(invalid_error());
    if (change.revision->noop && (!change.transitions.empty() || !change.generations.empty() || !change.fenced_runs.empty()))
      return std::unexpected(invalid_error());
    if (change.revision->noop && change.prepared)
      return std::unexpected(invalid_error());
  }
  if (!load && change.prepared)
    return std::unexpected(invalid_error());
  if (activate) {
    if (change.revision || !is_identifier(change.job) || !change.role.empty() || change.previous_run || change.packet ||
        change.provenance)
      return std::unexpected(invalid_error());
  }
  if (claim || takeover) {
    if (change.revision || !is_identifier(change.job) || !is_identifier(change.role) || !change.packet)
      return std::unexpected(invalid_error());
    if (claim && change.previous_run)
      return std::unexpected(invalid_error());
    if (takeover && !change.previous_run)
      return std::unexpected(invalid_error());
  }

  if (token_op) {
    auto token = parse_run_token(change.run_token_text);
    if (!change.run || !token || token->run_id != *change.run || !is_identifier(change.job) || !is_identifier(change.role) ||
        change.revision || change.previous_run || !change.generations.empty() || !change.fenced_runs.empty())
      return std::unexpected(invalid_error());
    if ((op == "refresh") != change.packet.has_value())
      return std::unexpected(invalid_error());
    if (op == "checkpoint" && (!change.checkpoint || !change.transitions.empty()))
      return std::unexpected(invalid_error());
    if (op != "release" && change.provenance)
      return std::unexpected(invalid_error());
    if (op == "release" && (change.release_reason != "released" && change.release_reason != "interrupted"))
      return std::unexpected(invalid_error());
    if (op == "release" &&
        (change.transitions.empty() || change.transitions[0].job != change.job || change.transitions[0].role != change.role ||
         change.transitions[0].after_state == "running" || change.transitions[0].after_state == "retired" ||
         change.transitions[0].reason != change.release_reason))
      return std::unexpected(invalid_error());
    if (op == "release")
      for (std::size_t i = 1; i < change.transitions.size(); ++i) {
        const auto& t = change.transitions[i];
        if (t.job != change.job || t.role == change.role || t.before_state != "blocked" || t.reason != "prerequisite_available" ||
            t.after_drifted || (t.after_state != "ready" && t.after_state != "done" && t.after_state != "stale"))
          return std::unexpected(invalid_error());
      }
  } else if (change.run || change.checkpoint || !change.run_token_text.empty() || !change.release_reason.empty())
    return std::unexpected(invalid_error());

  std::set<role_key>                touched;
  std::vector<hist::transition_row> rows;
  for (std::size_t index = 0; index < change.transitions.size(); ++index) {
    const auto& transition = change.transitions[index];
    if (!is_identifier(transition.job) || !is_identifier(transition.role))
      return std::unexpected(invalid_error());
    if (!touched.insert(role_key{transition.job, transition.role}).second)
      return std::unexpected(invalid_error());
    hist::transition_row row;
    row.ordinal        = index + 1;
    row.job            = transition.job;
    row.role           = transition.role;
    row.generation_id  = 1;
    row.before_state   = transition.before_state;
    row.before_drifted = transition.before_drifted;
    row.after_state    = transition.after_state;
    row.after_drifted  = transition.after_drifted;
    row.reason         = transition.reason;
    rows.push_back(std::move(row));
  }
  if (!hist::validate_transition_sequence(rows))
    return std::unexpected(invalid_error());

  std::set<role_key> generated;
  for (const auto& generation : change.generations) {
    if (!is_identifier(generation.job) || !is_identifier(generation.role) || !is_generation_reason(generation.reason))
      return std::unexpected(invalid_error());
    if (!generated.insert(role_key{generation.job, generation.role}).second)
      return std::unexpected(invalid_error());
    if (generation.structure.job != generation.job || generation.structure.role != generation.role)
      return std::unexpected(invalid_error());
  }
  std::set<std::uint64_t> fenced(change.fenced_runs.begin(), change.fenced_runs.end());
  if (fenced.size() != change.fenced_runs.size())
    return std::unexpected(invalid_error());
  for (const auto& entry : change.vector) {
    if (!is_identifier(entry.job) || !is_identifier(entry.role) || !is_role_state(entry.state))
      return std::unexpected(invalid_error());
    if (entry.drifted && entry.state != "running")
      return std::unexpected(invalid_error());
    if (entry.run && entry.state != "running")
      return std::unexpected(invalid_error());
  }
  using kind_t = execution_expectation::kind_t;
  for (const auto& member : change.expected) {
    switch (member.kind) {
    case kind_t::active_revision:
    case kind_t::active_job:
    case kind_t::fact_version:
    case kind_t::role_state:
    case kind_t::generation_current:
    case kind_t::request_unused:
    case kind_t::run_active:
      break;
    default:
      return std::unexpected(invalid_error());
    }
  }
  return {};
}

checked check_parameters(const execution_change& change, const hist::request_v3& framed) {
  auto decoded = ::planar::engine::canonical_decode(framed.parameters);
  if (!decoded || (*decoded)->kind != value_kind::record)
    return std::unexpected(invalid_error());
  const auto& fields = (*decoded)->fields;
  if (change.operation == "refresh" || change.operation == "checkpoint" || change.operation == "release") {
    std::set<std::string> allowed{"run_id"};
    if (change.operation == "refresh")
      allowed.insert("expect_fp");
    if (change.operation == "release")
      allowed.insert("reason");
    if (fields.contains("observations_path"))
      allowed.insert("observations_path");
    if (fields.contains("provenance_path") && change.operation == "release")
      allowed.insert("provenance_path");
    if (fields.size() != allowed.size())
      return std::unexpected(invalid_error());
    for (const auto& key : allowed)
      if (!fields.contains(key) || !fields.at(key) || fields.at(key)->kind != value_kind::string)
        return std::unexpected(invalid_error());
    if (!change.run || fields.at("run_id")->string != std::to_string(*change.run))
      return std::unexpected(invalid_error());
    if (change.operation == "release" && fields.at("reason")->string != change.release_reason)
      return std::unexpected(invalid_error());
    if (change.operation == "checkpoint" && !fields.contains("observations_path"))
      return std::unexpected(invalid_error());
    return {};
  }

  std::set<std::string> keys;
  if (change.operation == "verdict") { // R18: request_v3 parameters {run_id, verdict_path} (R18-4 Replay, DR18-10).
    if (!change.verdict || fields.size() != 2)
      return std::unexpected(invalid_error());
    for (const auto* key : {"run_id", "verdict_path"}) {
      const auto found = fields.find(key);
      if (found == fields.end() || !found->second || found->second->kind != value_kind::string)
        return std::unexpected(invalid_error());
    }
    if (fields.at("run_id")->string != std::to_string(change.verdict->run) || fields.at("verdict_path")->string.empty() ||
        fields.at("verdict_path")->string.front() != '/')
      return std::unexpected(invalid_error());
    return {};
  }
  if (change.operation == "claim" || change.operation == "takeover") {
    keys = {"job", "role"};
    if (change.operation == "takeover")
      keys.insert("run_id");
  } else {
    keys = {"expect_job", "expect_revision", "job"};
  }
  if (change.provenance)
    keys.insert("provenance_path");
  if (fields.size() != keys.size())
    return std::unexpected(invalid_error());
  for (const auto& key : keys) {
    const auto found = fields.find(key);
    if (found == fields.end() || !found->second || found->second->kind != value_kind::string)
      return std::unexpected(invalid_error());
  }
  auto text = [&](const char* key) { return fields.at(key)->string; };
  if (text("job") != change.job)
    return std::unexpected(invalid_error());
  if (change.operation == "claim" || change.operation == "takeover") {
    if (text("role") != change.role)
      return std::unexpected(invalid_error());
    if (change.operation == "takeover") {
      if (!change.previous_run || text("run_id") != std::to_string(*change.previous_run))
        return std::unexpected(invalid_error());
    }
  } else {
    const auto expect_job = text("expect_job");
    if (expect_job != "NONE" && !is_identifier(expect_job))
      return std::unexpected(invalid_error());
    const auto expect_revision = text("expect_revision");
    if (expect_revision != "none" && !parse_decimal(expect_revision))
      return std::unexpected(invalid_error());
  }
  return {};
}

checked check_request(const execution_change& change) {
  if (change.operation == "load") {
    const auto* framed = std::get_if<hist::request_v2>(&change.request);
    if (!framed || framed->operation != "load" || framed->actor != change.actor)
      return std::unexpected(invalid_error());
    return {};
  }
  const auto* framed = std::get_if<hist::request_v3>(&change.request);
  if (!framed || framed->operation != change.operation || framed->actor != change.actor)
    return std::unexpected(invalid_error());
  return check_parameters(change, *framed);
}

/// Snapshot equality (D37 a to f): the expected set, the before states, the complete vector, the target rules and the
/// fenced run set are all checked against the snapshot the change was built from.
checked check_snapshot(const execution_snapshot& snapshot, const execution_change& change) {
  using kind_t        = execution_expectation::kind_t;
  const bool load     = change.operation == "load";
  const bool claim    = change.operation == "claim";
  const bool takeover = change.operation == "takeover";
  const bool targeted = claim || takeover;

  if (load) {
    if (!change.job.empty() && change.job != snapshot.active_job)
      return std::unexpected(invalid_error());
    if (change.revision && change.revision->noop) {
      if (!snapshot.revision)
        return std::unexpected(invalid_error());
      const auto& active = *snapshot.revision;
      const auto& given  = *change.revision;
      if (active.source != given.source || active.registry != given.registry || active.stdlib != given.stdlib ||
          active.interpretation != given.interpretation || active.versions != given.versions)
        return std::unexpected(invalid_error());
    }
  }
  if (targeted && (change.job != snapshot.active_job || !snapshot_role(snapshot, change.job, change.role)))
    return std::unexpected(invalid_error());

  const bool token_op = change.operation == "refresh" || change.operation == "checkpoint" || change.operation == "release";
  if (token_op) {
    const auto* target = snapshot_role(snapshot, change.job, change.role);
    if (change.job != snapshot.active_job || !target || target->state != "running" || target->run_id != change.run)
      return std::unexpected(invalid_error());
    if (change.operation == "refresh" &&
        ((target->drifted && (change.transitions.size() != 1 || change.transitions[0].reason != "drift_cleared" ||
                              change.transitions[0].after_state != "running" || change.transitions[0].after_drifted)) ||
         (!target->drifted && !change.transitions.empty())))
      return std::unexpected(invalid_error());
  }
  // (a) The expected set equals the fixed set, plus optional no-row members of the active job.
  auto fixed = fixed_members(snapshot, change);
  if (!fixed)
    return std::unexpected(fixed.error());
  std::map<std::string, execution_expectation> wanted;
  for (const auto& member : *fixed)
    wanted.emplace(member_key(member), member);
  std::set<std::string> seen;
  std::set<role_key>    no_row_seen;
  for (const auto& member : change.expected) {
    if (member.kind == kind_t::role_state && member.no_row) {
      if (snapshot.active_job.empty() || member.job != snapshot.active_job || snapshot_role(snapshot, member.job, member.role) ||
          !no_row_seen.insert(role_key{member.job, member.role}).second)
        return std::unexpected(invalid_error());
      continue;
    }
    const auto key   = member_key(member);
    const auto found = wanted.find(key);
    if (found == wanted.end() || !seen.insert(key).second || !same_value(found->second, member))
      return std::unexpected(invalid_error());
  }
  if (seen.size() != wanted.size())
    return std::unexpected(invalid_error());

  // (b) Every before state equals the snapshot role state, and a row is absent exactly when there is no before state.
  for (const auto& transition : change.transitions) {
    const auto* row = snapshot_role(snapshot, transition.job, transition.role);
    if (!transition.before_state) {
      if (row)
        return std::unexpected(invalid_error());
      continue;
    }
    if (!row || row->state != *transition.before_state || transition.before_drifted != row->drifted)
      return std::unexpected(invalid_error());
  }

  // (c) The vector is the complete next role-state vector of the active job after the commit.
  std::map<role_key, execution_vector_entry> vector_wanted;
  if (!change.job.empty()) {
    for (const auto& row : snapshot.roles)
      if (row.job == change.job)
        vector_wanted[role_key{row.job, row.role}] = execution_vector_entry{
            .job = row.job, .role = row.role, .state = row.state, .drifted = row.drifted, .run = row.run_id};
    for (std::size_t index = 0; index < change.transitions.size(); ++index) {
      const auto& transition = change.transitions[index];
      if (transition.job != change.job)
        continue;
      execution_vector_entry entry{.job     = transition.job,
                                   .role    = transition.role,
                                   .state   = transition.after_state,
                                   .drifted = transition.after_drifted,
                                   .run     = std::nullopt};
      const bool             creates_run = targeted && index == 0;
      if (transition.after_state == "running" && !creates_run) {
        const auto* row = snapshot_role(snapshot, transition.job, transition.role);
        entry.run       = row ? row->run_id : std::nullopt;
      }
      vector_wanted[role_key{transition.job, transition.role}] = entry;
    }
  }
  std::set<role_key> vector_seen;
  for (const auto& entry : change.vector) {
    const role_key key{entry.job, entry.role};
    const auto     found = vector_wanted.find(key);
    if (found == vector_wanted.end() || !vector_seen.insert(key).second)
      return std::unexpected(invalid_error());
    if (found->second.state != entry.state || found->second.drifted != entry.drifted || found->second.run != entry.run)
      return std::unexpected(invalid_error());
  }
  if (vector_seen.size() != vector_wanted.size())
    return std::unexpected(invalid_error());

  // (d) A claim targets a ready or stale role, and its first transition is the target itself.
  if (claim) {
    const auto* target = snapshot_role(snapshot, change.job, change.role);
    if (!target || change.transitions.empty() || (target->state != "ready" && target->state != "stale"))
      return std::unexpected(invalid_error());
    const auto& first = change.transitions.front();
    if (first.job != change.job || first.role != change.role || first.after_state != "running")
      return std::unexpected(invalid_error());
  }
  // (e) A takeover names the snapshot active run of its target, which runs on through its first transition.
  if (takeover) {
    const auto* target = snapshot_role(snapshot, change.job, change.role);
    if (!target || target->state != "running" || target->run_id != change.previous_run || change.transitions.empty())
      return std::unexpected(invalid_error());
    const auto& first = change.transitions.front();
    if (first.job != change.job || first.role != change.role || first.before_state != std::optional<std::string>("running") ||
        first.after_state != "running")
      return std::unexpected(invalid_error());
  }

  // (f) The fenced runs are the snapshot active runs of roles leaving running, and of the takeover target.
  std::set<std::uint64_t> expected_fences;
  for (std::size_t index = 0; index < change.transitions.size(); ++index) {
    const auto& transition = change.transitions[index];
    if (transition.before_state != std::optional<std::string>("running"))
      continue;
    const bool takeover_target = takeover && index == 0;
    if (transition.after_state == "running" && !takeover_target)
      continue;
    const auto* row = snapshot_role(snapshot, transition.job, transition.role);
    if (!row || !row->run_id)
      return std::unexpected(invalid_error());
    if (change.operation != "release")
      expected_fences.insert(*row->run_id);
  }
  std::set<std::uint64_t> given_fences(change.fenced_runs.begin(), change.fenced_runs.end());
  if (given_fences != expected_fences)
    return std::unexpected(invalid_error());
  return {};
}

/// Native recomputation (D12 step 4, D52): the manifest and W name the change's job and role and carry the digests of
/// the packet Markdown and of W.
checked check_native(const execution_change& change, const prepared_change& prepared) {
  if (!change.packet)
    return {};
  const auto& manifest = change.packet->manifest;
  const auto& work     = change.packet->work;
  auto        text     = [](const value_ptr& node, std::string_view key) -> std::optional<std::string> {
    if (!node || node->kind != value_kind::record)
      return std::nullopt;
    const auto found = node->fields.find(std::string(key));
    if (found == node->fields.end() || !found->second || found->second->kind != value_kind::string)
      return std::nullopt;
    return found->second->string;
  };
  auto child = [](const value_ptr& node, std::string_view key) -> value_ptr {
    if (!node || node->kind != value_kind::record)
      return nullptr;
    const auto found = node->fields.find(std::string(key));
    return found == node->fields.end() ? nullptr : found->second;
  };
  if (text(manifest, "job") != change.job || text(manifest, "role") != change.role)
    return std::unexpected(invalid_error());
  if (text(work, "job") != change.job || text(work, "role") != change.role)
    return std::unexpected(invalid_error());
  if (text(child(manifest, "work"), "digest") != prepared.work_text ||
      text(child(manifest, "render"), "digest") != prepared.render_text)
    return std::unexpected(invalid_error());
  return {};
}

struct locked_head {
  std::optional<std::uint64_t> revision;
  std::string                  job;
  std::optional<std::uint64_t> activation;
};

value_result<locked_head> read_head(connection& db) {
  auto found = first_row(db, "SELECT active_revision_id,active_job,active_activation_id FROM current_state WHERE singleton = 1");
  if (!found)
    return std::unexpected(found.error());
  if (!*found)
    return std::unexpected(corrupt());
  row_reader  reader(**found);
  locked_head out;
  out.revision   = reader.optional_id(0);
  out.job        = reader.optional_text(1).value_or(std::string{});
  out.activation = reader.optional_id(2);
  if (auto finished = reader.finish(); !finished)
    return std::unexpected(finished.error());
  return out;
}

value_result<std::optional<role_row>> locked_role(connection& db, std::string_view job, std::string_view role) {
  auto found = first_row(
      db, "SELECT job,role,generation_id,state,drifted,run_id,fact_version,last_seq FROM role_state WHERE job = ?1 AND role = ?2",
      {text_arg(job), text_arg(role)});
  if (!found)
    return std::unexpected(found.error());
  if (!*found)
    return std::optional<role_row>{};
  row_reader reader(**found);
  role_row   out;
  out.job          = reader.text(0);
  out.role         = reader.text(1);
  out.generation   = reader.id(2);
  out.state        = reader.text(3);
  out.drifted      = reader.flag(4);
  out.run          = reader.optional_id(5);
  out.fact_version = static_cast<std::uint64_t>(reader.number(6));
  out.last_seq     = reader.id(7);
  if (auto finished = reader.finish(); !finished)
    return std::unexpected(finished.error());
  return std::optional<role_row>(std::move(out));
}

value_result<std::optional<std::string>> locked_run_state(connection& db, std::uint64_t run) {
  auto found = first_row(db, "SELECT state FROM run_state WHERE run_id = ?1", {int_arg(run)});
  if (!found)
    return std::unexpected(found.error());
  if (!*found)
    return std::optional<std::string>{};
  row_reader reader(**found);
  auto       state = reader.text(0);
  if (auto finished = reader.finish(); !finished)
    return std::unexpected(finished.error());
  return std::optional<std::string>(std::move(state));
}

value_result<std::optional<std::uint64_t>> revision_parent(connection& db, std::uint64_t revision) {
  auto found = first_row(db, "SELECT parent_id FROM revisions WHERE id = ?1", {int_arg(revision)});
  if (!found)
    return std::unexpected(found.error());
  if (!*found)
    return std::unexpected(corrupt());
  row_reader reader(**found);
  const auto parent = reader.optional_id(0);
  if (auto finished = reader.finish(); !finished)
    return std::unexpected(finished.error());
  return parent;
}

value_result<std::uint64_t> generation_ordinal(connection& db, const std::map<std::uint64_t, std::uint64_t>& known,
                                               std::uint64_t id) {
  if (const auto found = known.find(id); found != known.end())
    return found->second;
  auto stored = first_row(db, "SELECT ordinal FROM generations WHERE id = ?1", {int_arg(id)});
  if (!stored)
    return std::unexpected(stored.error());
  if (!*stored)
    return std::unexpected(corrupt());
  row_reader reader(**stored);
  const auto ordinal = reader.id(0);
  if (auto finished = reader.finish(); !finished)
    return std::unexpected(finished.error());
  return ordinal;
}

/// Map a failed target member to claim conflict or loss of the named run (Q-PREC, R16 token refusal step 10).
value_result<execution_error> target_conflict(connection& db, const execution_change& change,
                                              const std::optional<role_row>& row) {
  if (change.operation == "claim") {
    if (row && (row->state == "running" || row->state == "blocked" || row->state == "done" || row->state == "retired")) {
      execution_error error = failure(execution_error_code::claim_conflict);
      error.state           = row->state;
      return error;
    }
    return failure(execution_error_code::state_conflict);
  }
  const auto named_run = change.operation == "takeover" ? change.previous_run : change.run;
  if (named_run && (!row || row->run != named_run)) {
    auto state = locked_run_state(db, *named_run);
    if (!state)
      return std::unexpected(state.error());
    if (*state) {
      execution_error error = failure(execution_error_code::fenced);
      error.state           = **state;
      return error;
    }
  }
  return failure(execution_error_code::state_conflict);
}

/// Evaluate the expected members in the EX-3 vocabulary order under the lock; the first failing member decides.
checked check_preconditions(connection& db, const execution_change& change, const locked_head& head) {
  using kind_t                               = execution_expectation::kind_t;
  std::vector<execution_expectation> members = change.expected;
  std::stable_sort(members.begin(), members.end(), [](const execution_expectation& a, const execution_expectation& b) {
    if (a.kind != b.kind)
      return static_cast<int>(a.kind) < static_cast<int>(b.kind);
    return std::tie(a.job, a.role) < std::tie(b.job, b.role);
  });
  for (const auto& member : members) {
    switch (member.kind) {
    case kind_t::active_revision:
      if (head.revision != member.revision)
        return std::unexpected(failure(execution_error_code::state_conflict));
      break;
    case kind_t::active_job:
      if (head.job != member.text)
        return std::unexpected(failure(execution_error_code::state_conflict));
      break;
    case kind_t::fact_version: {
      auto row = locked_role(db, member.job, member.role);
      if (!row)
        return std::unexpected(row.error());
      if (!*row || (*row)->fact_version != member.number)
        return std::unexpected(failure(execution_error_code::state_conflict));
      break;
    }
    case kind_t::role_state: {
      auto row = locked_role(db, member.job, member.role);
      if (!row)
        return std::unexpected(row.error());
      if (member.no_row) {
        if (*row)
          return std::unexpected(failure(execution_error_code::state_conflict));
        break;
      }
      const bool target = (change.operation == "claim" || change.operation == "takeover" || change.operation == "checkpoint" ||
                           change.operation == "refresh" || change.operation == "release") &&
                          member.job == change.job && member.role == change.role;
      if (!*row || (*row)->state != member.state || (*row)->drifted != member.drifted || (*row)->run != member.run) {
        if (!target)
          return std::unexpected(failure(execution_error_code::state_conflict));
        auto conflict = target_conflict(db, change, *row);
        if (!conflict)
          return std::unexpected(conflict.error());
        return std::unexpected(*conflict);
      }
      break;
    }
    case kind_t::generation_current: {
      auto row = locked_role(db, member.job, member.role);
      if (!row)
        return std::unexpected(row.error());
      if (!*row || (*row)->generation != member.number)
        return std::unexpected(failure(execution_error_code::state_conflict));
      break;
    }
    case kind_t::run_active: {
      auto token = parse_run_token(change.run_token_text);
      if (!token || !change.run || token->run_id != *change.run)
        return std::unexpected(invalid_error());
      auto row = first_row(
          db,
          "SELECT rs.state,rs.generation_id,g.ordinal,r.secret,cr.generation_id FROM runs r JOIN run_state rs ON rs.run_id=r.id "
          "JOIN generations g ON g.id=r.generation_id JOIN role_state cr ON cr.job=g.job AND cr.role=g.role WHERE r.id=?1",
          {int_arg(*change.run)});
      if (!row)
        return std::unexpected(row.error());
      if (!*row)
        return std::unexpected(failure(execution_error_code::unknown_run));
      row_reader r(**row);
      auto       state      = r.text(0);
      auto       generation = r.id(1);
      auto       ordinal    = r.id(2);
      auto       secret     = r.blob(3);
      auto       current    = r.id(4);
      if (auto ok = r.finish(); !ok)
        return std::unexpected(ok.error());
      const std::string expected(reinterpret_cast<const char*>(token->secret.data()), token->secret.size());
      if (state != "active" || generation != current || ordinal != token->generation_ordinal || secret != expected) {
        auto error  = failure(execution_error_code::fenced);
        error.state = state;
        return std::unexpected(error);
      }
      break;
    }
    case kind_t::request_unused:
      break;
    default:
      return std::unexpected(invalid_error());
    }
  }
  return {};
}

/// The locked before-state recheck (D40): a first transition needs no row, any other must match the locked row, and every
/// fenced run must still be active.
checked check_locked_before(connection& db, const execution_change& change) {
  for (const auto& transition : change.transitions) {
    auto row = locked_role(db, transition.job, transition.role);
    if (!row)
      return std::unexpected(row.error());
    if (!transition.before_state) {
      if (*row)
        return std::unexpected(failure(execution_error_code::state_conflict));
      continue;
    }
    if (!*row || (*row)->state != *transition.before_state || (*row)->drifted != transition.before_drifted)
      return std::unexpected(failure(execution_error_code::state_conflict));
  }
  for (auto run : change.fenced_runs) {
    auto state = locked_run_state(db, run);
    if (!state)
      return std::unexpected(state.error());
    if (!*state || **state != "active")
      return std::unexpected(failure(execution_error_code::state_conflict));
  }
  return {};
}

/// Revision record identifiers of a load must name existing records of the right kind (directories, then values).
checked check_revision_records(connection& db, const execution_revision_input& input) {
  const std::array<std::pair<std::uint64_t, std::int64_t>, 5> records{
      {{input.source, 2}, {input.registry, 2}, {input.stdlib, 2}, {input.interpretation, 3}, {input.versions, 3}}};
  for (const auto& [id, kind] : records) {
    auto found = first_row(db, "SELECT kind FROM records WHERE id = ?1", {int_arg(id)});
    if (!found)
      return std::unexpected(found.error());
    if (!*found)
      return std::unexpected(invalid_error());
    row_reader reader(**found);
    const auto stored = reader.number(0);
    if (auto finished = reader.finish(); !finished)
      return std::unexpected(finished.error());
    if (stored != kind)
      return std::unexpected(invalid_error());
  }
  return {};
}

/// Collision-safe interning (D38): reuse only an exact row, refuse any other row with the same digest, else insert.
value_result<std::uint64_t> intern(connection& db, const artifact& item, bool* inserted = nullptr) {
  auto query = prepare_bound(db, "SELECT id,kind,codec,encoded FROM records WHERE digest = ?1", {blob_arg(item.digest)});
  if (!query)
    return std::unexpected(query.error());
  statement&                   stmt     = *query;
  bool                         collided = false;
  std::optional<std::uint64_t> reused;
  while (true) {
    auto step = stmt.step();
    if (!step)
      return std::unexpected(map_sqlite(step.error()));
    if (*step == step_result::done)
      break;
    row_reader reader(stmt);
    const auto id      = reader.id(0);
    const auto kind    = reader.number(1);
    const auto codec   = reader.number(2);
    auto       encoded = reader.blob(3);
    if (auto finished = reader.finish(); !finished)
      return std::unexpected(finished.error());
    if (kind == item.kind && codec == item.codec && encoded == item.frame)
      reused = id;
    else
      collided = true;
  }
  if (collided) {
    execution_error error = failure(execution_error_code::hash_collision);
    error.record_kind     = item.label;
    error.record_digest   = item.label == "request" ? std::string{} : hex_of(item.digest);
    return std::unexpected(error);
  }
  if (inserted)
    *inserted = !reused;
  if (reused)
    return *reused;
  auto id = next_value(db, "SELECT COALESCE(MAX(id),0) FROM records");
  if (!id)
    return std::unexpected(id.error());
  if (auto inserted = run_bound(db, "INSERT INTO records(id,kind,codec,digest,encoded) VALUES(?1,?2,?3,?4,?5)",
                                {int_arg(*id), int_arg(static_cast<std::uint64_t>(item.kind)),
                                 int_arg(static_cast<std::uint64_t>(item.codec)), blob_arg(item.digest), blob_arg(item.frame)});
      !inserted)
    return std::unexpected(inserted.error());
  return *id;
}

/// G2: intern one codec1 record of a load candidate, writing the directory edges of a newly inserted directory.
value_result<std::uint64_t> intern_codec1(connection& db, const hist::record& item) {
  auto frame  = hist::encode_record(item);
  auto digest = hist::digest_record(item);
  if (!frame || !digest)
    return std::unexpected(invalid_error());
  const auto  kind  = hist::kind_of(item);
  std::string label = kind == hist::record_kind::file ? "file" : kind == hist::record_kind::directory ? "directory" : "value";
  bool        fresh = false;
  auto        id    = intern(db, artifact{static_cast<std::int64_t>(kind), 1, *frame, digest_bytes(*digest), label}, &fresh);
  if (!id)
    return std::unexpected(id.error());
  if (fresh) {
    if (const auto* directory = std::get_if<hist::directory_record>(&item)) {
      for (std::size_t i = 0; i < directory->children.size(); ++i) {
        const auto& child = directory->children[i];
        if (auto edge = run_bound(db, "INSERT INTO tree_edges(parent_id,ordinal,name,child_id) VALUES(?1,?2,?3,?4)",
                                  {int_arg(*id), int_arg(i), text_arg(child.name), int_arg(child.id)});
            !edge)
          return std::unexpected(edge.error());
      }
    }
  }
  return id;
}

value_result<std::uint64_t> intern_prepared_tree(connection& db, const execution_prepared_tree& tree) {
  std::vector<hist::tree_file_view> views;
  views.reserve(tree.files.size());
  for (const auto& file : tree.files)
    views.push_back(hist::tree_file_view{file.path, file.payload, file.mode});
  std::optional<execution_error> stashed;
  const hist::tree_record_intern callback =
      [&](const hist::record& item) -> std::expected<types::record_reference, types::format_error> {
    auto id = intern_codec1(db, item);
    if (!id) {
      stashed = id.error();
      return std::unexpected(types::format_error{types::format_error_code::io, "Interning failed"});
    }
    auto digest = hist::digest_record(item);
    if (!digest)
      return std::unexpected(types::format_error{types::format_error_code::invalid_argument, "Invalid record"});
    return types::record_reference{*id, hist::kind_of(item), *digest};
  };
  auto root = hist::build_tree_core(views, tree.profile, callback);
  if (!root) {
    if (stashed)
      return std::unexpected(*stashed);
    if (root.error().code == hist::tree_error_code::resource_limit) {
      execution_error error = failure(execution_error_code::resource_limit);
      error.limit           = "tree";
      return std::unexpected(error);
    }
    return std::unexpected(invalid_error());
  }
  if (root->kind != hist::record_kind::directory)
    return std::unexpected(invalid_error());
  return root->id;
}

/// G2: intern the prepared records of an accepted load inside the staged transaction (collision-safe, D38) and return
/// the revision input with the allocated identifiers.
value_result<execution_revision_input> intern_prepared_revision(connection& db, const execution_prepared_revision& prepared,
                                                                execution_revision_input input) {
  auto source = intern_prepared_tree(db, prepared.source);
  if (!source)
    return std::unexpected(source.error());
  auto registry = intern_prepared_tree(db, prepared.registry);
  if (!registry)
    return std::unexpected(registry.error());
  auto stdlib = intern_prepared_tree(db, prepared.stdlib);
  if (!stdlib)
    return std::unexpected(stdlib.error());
  auto interpretation =
      intern_codec1(db, hist::record{hist::value_record{.codec = "canonical_v1", .encoded = prepared.interpretation}});
  if (!interpretation)
    return std::unexpected(interpretation.error());
  auto versions = intern_codec1(db, hist::record{hist::value_record{.codec = "canonical_v1", .encoded = prepared.versions}});
  if (!versions)
    return std::unexpected(versions.error());
  input.source         = *source;
  input.registry       = *registry;
  input.stdlib         = *stdlib;
  input.interpretation = *interpretation;
  input.versions       = *versions;
  return input;
}

struct locked_outcome {
  bool                    replayed = false;
  execution_commit_result result;
};

/// Steps (5) to (9b) of D12: the locked format check, the locked identity recheck, the request relookup, the
/// compare-and-swap, the preconditions, the writes and the shared verifier. Any failure returns before COMMIT, and the
/// caller's transaction rolls back.
value_result<locked_outcome> apply_locked(execution_database& database, const execution_change& change,
                                          const execution_snapshot& snapshot, const prepared_change& prepared) {
  connection&               db       = database.sql();
  const execution_metadata& metadata = database.metadata();
  if (auto verified = verify_format3_schema(db); !verified)
    return std::unexpected(verified.error());

  // EX-3 step 6 and D12 step 5: the snapshot identity must name the ledger now at the path. The identity is read from
  // the locked connection, not the cached metadata, and the check runs before any replay answer or write.
  auto locked = first_row(db, "SELECT identity FROM ledger_meta");
  if (!locked)
    return std::unexpected(locked.error());
  if (!*locked)
    return std::unexpected(corrupt());
  row_reader        locked_reader(**locked);
  const std::string locked_identity = locked_reader.text(0);
  if (auto finished = locked_reader.finish(); !finished)
    return std::unexpected(finished.error());
  if (locked_identity != snapshot.identity)
    return std::unexpected(failure(execution_error_code::path_denied));

  // (6) A replayed request returns its saved bytes before any compare-and-swap or precondition (D7).
  auto earlier = first_row(db,
                           "SELECT records.encoded,requests.result FROM requests JOIN records ON records.id = requests.body_id "
                           "WHERE requests.request_id = ?1",
                           {text_arg(change.request_id)});
  if (!earlier)
    return std::unexpected(earlier.error());
  if (*earlier) {
    row_reader reader(**earlier);
    auto       stored = reader.blob(0);
    auto       saved  = reader.blob(1);
    if (auto finished = reader.finish(); !finished)
      return std::unexpected(finished.error());
    if (stored != prepared.request.frame)
      return std::unexpected(failure(execution_error_code::request_conflict));
    locked_outcome replay;
    replay.replayed            = true;
    replay.result.replayed     = true;
    replay.result.saved_result = std::move(saved);
    return replay;
  }

  if (change.operation == "verdict") { // R18: the verdict commit of execution_review.cpp, then the shared verifier.
    if (auto ok = check_verdict_locked(db, change); !ok)
      return std::unexpected(ok.error());
    if (!prepared.verdict)
      return std::unexpected(invalid_error());
    auto request_record = intern(db, prepared.request);
    if (!request_record)
      return std::unexpected(request_record.error());
    auto body = intern(db, *prepared.verdict);
    if (!body)
      return std::unexpected(body.error());
    auto written = write_verdict(db, metadata.identity, change, *request_record, *body, commit_timestamp());
    if (!written)
      return std::unexpected(written.error());
    ledger_rows verdict_rows;
    if (auto verified_rows = verify_rows(db, metadata, verdict_rows); !verified_rows)
      return std::unexpected(invalid_error());
    locked_outcome verdict_outcome;
    verdict_outcome.result = *written;
    return verdict_outcome;
  }

  auto head = read_head(db);
  if (!head)
    return std::unexpected(head.error());
  const bool load        = change.operation == "load";
  const bool activate    = change.operation == "activate";
  const bool claim       = change.operation == "claim";
  const bool takeover    = change.operation == "takeover";
  const bool run_op      = claim || takeover;
  const bool refresh     = change.operation == "refresh";
  const bool checkpoint  = change.operation == "checkpoint";
  const bool release     = change.operation == "release";
  const bool token_op    = refresh || checkpoint || release;
  const bool dispatch_op = run_op || refresh;
  const bool accepted    = load && change.revision && !change.revision->noop;
  const bool noop        = load && change.revision && change.revision->noop;

  // (7) A load compares its intent expected head with the locked head (D39, EX-4 671).
  if (load) {
    const auto* framed = std::get_if<hist::request_v2>(&change.request);
    if (!framed)
      return std::unexpected(invalid_error());
    const std::string actual = head->revision ? std::to_string(*head->revision) : std::string("none");
    if (framed->expected_head != actual) {
      execution_error error = failure(execution_error_code::revision_conflict);
      error.expected        = framed->expected_head;
      error.actual          = actual;
      return std::unexpected(error);
    }
  }
  // (8) Preconditions in the vocabulary order, then the locked before states and fenced runs (D40).
  if (auto ok = check_preconditions(db, change, *head); !ok)
    return std::unexpected(ok.error());
  if (auto ok = check_locked_before(db, change); !ok)
    return std::unexpected(ok.error());
  if (refresh || release) {
    auto open =
        first_row(db,
                  "SELECT run_id FROM reservation_state JOIN reservations ON reservations.id=reservation_state.reservation_id "
                  "WHERE reservations.run_id=?1 AND reservation_state.state='open' LIMIT 1",
                  {int_arg(*change.run)});
    if (!open)
      return std::unexpected(open.error());
    if (*open)
      return std::unexpected(failure(execution_error_code::check_in_progress));
  }
  if (!accepted && !head->revision)
    return std::unexpected(invalid_error());
  // G2: the candidate's records are interned in this write transaction, before the revision record check.
  std::optional<execution_revision_input> revision_refs = change.revision;
  if (accepted && change.prepared) {
    auto interned = intern_prepared_revision(db, *change.prepared, *change.revision);
    if (!interned)
      return std::unexpected(interned.error());
    revision_refs = *interned;
  }
  if (accepted) {
    if (auto ok = check_revision_records(db, *revision_refs); !ok)
      return std::unexpected(ok.error());
  }

  // (9) Records first (collision-safe, D38), then identifiers, events, transitions, generations, runs, dispatches, requests
  // and the delta write of the projections, with the cross-row verifier last (D47).
  auto seq = next_value(db, "SELECT COALESCE(MAX(seq),0) FROM events");
  if (!seq)
    return std::unexpected(seq.error());
  const std::string now        = commit_timestamp();
  const std::string event_type = accepted                              ? "declaration_accepted"
                                 : noop                                ? "load_noop"
                                 : activate                            ? "job_activated"
                                 : claim                               ? "run_claimed"
                                 : takeover                            ? "run_taken_over"
                                 : refresh                             ? "run_refreshed"
                                 : checkpoint                          ? "checkpoint_recorded"
                                 : change.release_reason == "released" ? "run_released"
                                                                       : "run_interrupted";

  auto request_record = intern(db, prepared.request);
  if (!request_record)
    return std::unexpected(request_record.error());
  std::optional<std::uint64_t> packet_id;
  std::optional<std::uint64_t> manifest_id;
  std::optional<std::uint64_t> work_id;
  std::optional<std::uint64_t> provenance_id;
  std::optional<std::uint64_t> checkpoint_id, checkpoint_body;
  if (prepared.checkpoint) {
    auto id = intern(db, *prepared.checkpoint);
    if (!id)
      return std::unexpected(id.error());
    checkpoint_body = *id;
    auto next       = next_value(db, "SELECT COALESCE(MAX(id),0) FROM checkpoints");
    if (!next)
      return std::unexpected(next.error());
    checkpoint_id = *next;
    if (refresh && (*checkpoint_id != snapshot.next_checkpoint_id || *seq != snapshot.last_seq + 1))
      return std::unexpected(failure(execution_error_code::state_conflict));
  }
  if (prepared.packet) {
    auto id = intern(db, *prepared.packet);
    if (!id)
      return std::unexpected(id.error());
    packet_id = *id;
  }
  if (prepared.manifest) {
    auto id = intern(db, *prepared.manifest);
    if (!id)
      return std::unexpected(id.error());
    manifest_id = *id;
  }
  if (prepared.work) {
    auto id = intern(db, *prepared.work);
    if (!id)
      return std::unexpected(id.error());
    work_id = *id;
  }
  if (prepared.provenance) {
    auto id = intern(db, *prepared.provenance);
    if (!id)
      return std::unexpected(id.error());
    provenance_id = *id;
  }
  std::vector<std::uint64_t> structure_ids;
  for (const auto& structure : prepared.structures) {
    auto id = intern(db, structure);
    if (!id)
      return std::unexpected(id.error());
    structure_ids.push_back(*id);
  }

  // The revision this commit accepts, or the active revision every other event refers to.
  std::optional<std::uint64_t> revision_id = head->revision;
  if (accepted) {
    auto next = next_value(db, "SELECT COALESCE(MAX(id),0) FROM revisions");
    if (!next)
      return std::unexpected(next.error());
    const auto& refs = *revision_refs;
    if (auto inserted = run_bound(
            db,
            "INSERT INTO revisions(id,parent_id,source_id,registry_id,stdlib_id,interpretation_id,versions_id,"
            "actor,accepted_at) VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9)",
            {int_arg(*next), optional_int_arg(head->revision), int_arg(refs.source), int_arg(refs.registry), int_arg(refs.stdlib),
             int_arg(refs.interpretation), int_arg(refs.versions), text_arg(change.actor), text_arg(now)});
        !inserted)
      return std::unexpected(inserted.error());
    revision_id = *next;
  }

  // Generations: identifiers and ordinals allocated under the lock.
  auto generation_base = next_value(db, "SELECT COALESCE(MAX(id),0) FROM generations");
  if (!generation_base)
    return std::unexpected(generation_base.error());
  std::map<role_key, std::uint64_t>      generation_of_role;
  std::map<std::uint64_t, std::uint64_t> ordinal_of_generation;
  std::vector<std::uint64_t>             generation_ids;
  for (std::size_t index = 0; index < change.generations.size(); ++index) {
    const auto& input   = change.generations[index];
    const auto  id      = *generation_base + index;
    auto        ordinal = next_value(db, "SELECT COALESCE(MAX(ordinal),0) FROM generations WHERE job = ?1 AND role = ?2",
                                     {text_arg(input.job), text_arg(input.role)});
    if (!ordinal)
      return std::unexpected(ordinal.error());
    generation_of_role[role_key{input.job, input.role}] = id;
    ordinal_of_generation[id]                           = *ordinal;
    generation_ids.push_back(id);
  }

  // Every transition's generation: the new one of its role, else the locked generation of an existing row.
  std::vector<std::uint64_t>           transition_generation;
  std::vector<std::optional<role_row>> before_rows;
  for (const auto& transition : change.transitions) {
    auto row = locked_role(db, transition.job, transition.role);
    if (!row)
      return std::unexpected(row.error());
    const auto found = generation_of_role.find(role_key{transition.job, transition.role});
    if (found != generation_of_role.end())
      transition_generation.push_back(found->second);
    else if (*row)
      transition_generation.push_back((*row)->generation);
    else
      return std::unexpected(invalid_error());
    before_rows.push_back(*row);
  }

  // Runs, dispatches, activations and the run secret (D8).
  std::optional<std::uint64_t> run_id;
  std::optional<std::uint64_t> dispatch_id;
  std::optional<std::uint64_t> activation_id;
  std::optional<std::uint64_t> target_generation;
  std::uint64_t                ordinal = 0;
  std::array<std::uint8_t, 16> secret{};
  std::string                  token_text;
  if (run_op) {
    if (transition_generation.empty())
      return std::unexpected(invalid_error());
    target_generation = transition_generation.front();
    auto run          = next_value(db, "SELECT COALESCE(MAX(id),0) FROM runs");
    if (!run)
      return std::unexpected(run.error());
    auto dispatch = next_value(db, "SELECT COALESCE(MAX(id),0) FROM dispatches");
    if (!dispatch)
      return std::unexpected(dispatch.error());
    auto bytes = run_secret();
    if (!bytes)
      return std::unexpected(bytes.error());
    secret              = *bytes;
    auto target_ordinal = generation_ordinal(db, ordinal_of_generation, *target_generation);
    if (!target_ordinal)
      return std::unexpected(target_ordinal.error());
    ordinal     = *target_ordinal;
    run_id      = *run;
    dispatch_id = *dispatch;
    token_text  = format_run_token(run_token{*run_id, ordinal, secret});
  }
  if (token_op) {
    run_id            = change.run;
    const auto* role  = snapshot_role(snapshot, change.job, change.role);
    target_generation = role->generation_id;
    ordinal           = role->generation_ordinal;
    if (refresh) {
      auto next = next_value(db, "SELECT COALESCE(MAX(id),0) FROM dispatches");
      if (!next)
        return std::unexpected(next.error());
      dispatch_id = *next;
    }
  }
  if (activate) {
    auto activation = next_value(db, "SELECT COALESCE(MAX(id),0) FROM job_activations");
    if (!activation)
      return std::unexpected(activation.error());
    activation_id = *activation;
  }

  // Fingerprints of the dispatch rows (16 raw bytes each).
  std::string work_fp;
  std::string render_fp;
  if (dispatch_op) {
    auto work_bytes   = digest_bytes_of_text(prepared.work_text);
    auto render_bytes = digest_bytes_of_text(prepared.render_text);
    if (!work_bytes || !render_bytes)
      return std::unexpected(invalid_error());
    work_fp   = *work_bytes;
    render_fp = *render_bytes;
  }

  // Event payload v2 and the saved result of the request, both encoded from the allocated identifiers (D19, D25).
  hist::event_payload_v2 payload;
  payload.operation              = change.operation;
  payload.before.active_revision = head->revision ? std::to_string(*head->revision) : std::string("none");
  payload.before.active_job      = head->job;
  payload.after.active_revision  = revision_id ? std::to_string(*revision_id) : std::string("none");
  payload.after.active_job       = change.job;
  payload.reason                 = change.transitions.empty() ? std::string{} : change.transitions.front().reason;
  if (accepted || noop) {
    const auto& refs                     = *revision_refs;
    payload.references["source"]         = std::to_string(refs.source);
    payload.references["registry"]       = std::to_string(refs.registry);
    payload.references["stdlib"]         = std::to_string(refs.stdlib);
    payload.references["interpretation"] = std::to_string(refs.interpretation);
    payload.references["versions"]       = std::to_string(refs.versions);
  }
  if (activate) {
    payload.references["activation"] = std::to_string(*activation_id);
    payload.references["revision"]   = std::to_string(*revision_id);
  }
  if (token_op) {
    payload.references["run"] = std::to_string(*run_id);
    if (checkpoint_id)
      payload.references["checkpoint"] = std::to_string(*checkpoint_id);
    if (provenance_id)
      payload.references["provenance"] = std::to_string(*provenance_id);
  }
  if (dispatch_op) {
    payload.references["run"]      = std::to_string(*run_id);
    payload.references["dispatch"] = std::to_string(*dispatch_id);
    payload.references["packet"]   = std::to_string(*packet_id);
    payload.references["manifest"] = std::to_string(*manifest_id);
    payload.references["work"]     = std::to_string(*work_id);
    if (provenance_id)
      payload.references["provenance"] = std::to_string(*provenance_id);
    if (takeover)
      payload.references["previous_run"] = std::to_string(*change.previous_run);
  }
  auto payload_bytes = hist::encode_event_payload_v2(event_type, payload);
  if (!payload_bytes)
    return std::unexpected(invalid_error());

  std::string saved;
  if (accepted || noop) {
    load_result result{};
    result.ledger     = metadata.identity;
    result.request_id = change.request_id;
    result.outcome    = noop ? load_outcome::noop : load_outcome::accepted;
    result.revision   = static_cast<std::int64_t>(*revision_id);
    if (accepted) {
      if (head->revision)
        result.parent = static_cast<std::int64_t>(*head->revision);
    } else {
      auto parent = revision_parent(db, *head->revision);
      if (!parent)
        return std::unexpected(parent.error());
      if (*parent)
        result.parent = static_cast<std::int64_t>(**parent);
    }
    result.event = static_cast<std::int64_t>(*seq);
    auto encoded = encode_load_result(result);
    if (!encoded)
      return std::unexpected(invalid_error());
    saved = *encoded;
  } else if (activate) {
    hist::activate_result_v1 result;
    result.ledger       = metadata.identity;
    result.request_id   = change.request_id;
    result.revision     = std::to_string(*revision_id);
    result.job          = change.job;
    result.previous_job = head->job;
    result.activation   = std::to_string(*activation_id);
    result.event        = std::to_string(*seq);
    for (std::size_t index = 0; index < change.transitions.size(); ++index) {
      const auto& transition = change.transitions[index];
      if (transition.job != change.job)
        continue;
      auto generation = generation_ordinal(db, ordinal_of_generation, transition_generation[index]);
      if (!generation)
        return std::unexpected(generation.error());
      result.roles.push_back(hist::activate_role_v1{transition.role, std::to_string(*generation), transition.after_state});
    }
    std::ranges::sort(result.roles,
                      [](const hist::activate_role_v1& a, const hist::activate_role_v1& b) { return a.role < b.role; });
    auto encoded = hist::encode_activate_result_v1(result);
    if (!encoded)
      return std::unexpected(invalid_error());
    saved = *encoded;
  } else if (token_op) {
    auto run = record_node({{"id", text_node(std::to_string(*run_id))}, {"generation", text_node(std::to_string(ordinal))}});
    std::map<std::string, value_ptr> fields{{"version", integer_node(1)},
                                            {"kind", text_node(change.operation)},
                                            {"ledger", text_node(metadata.identity)},
                                            {"request_id", text_node(change.request_id)},
                                            {"job", text_node(change.job)},
                                            {"role", text_node(change.role)},
                                            {"run", run}};
    value_ptr note = checkpoint_id ? record_node({{"id", text_node(std::to_string(*checkpoint_id))}}) : text_node("none");
    if (checkpoint) {
      note = record_node({{"id", text_node(std::to_string(*checkpoint_id))}, {"event", text_node(std::to_string(*seq))}});
    }
    fields["checkpoint"] = note;
    if (refresh) {
      fields["revision"]      = text_node(std::to_string(*revision_id));
      fields["drift_cleared"] = bool_node(snapshot_role(snapshot, change.job, change.role)->drifted);
      fields["dispatch"]      = record_node({{"id", text_node(std::to_string(*dispatch_id))},
                                             {"work_fp", text_node(prepared.work_text)},
                                             {"render_digest", text_node(prepared.render_text)},
                                             {"bytes", integer_node(prepared.markdown.size())}});
    }
    if (release) {
      fields["run"]        = record_node({{"id", text_node(std::to_string(*run_id))},
                                          {"generation", text_node(std::to_string(ordinal))},
                                          {"state", text_node(change.release_reason)}});
      fields["role_state"] = record_node({{"state", text_node(change.transitions.front().after_state)}});
      fields["event"]      = text_node(std::to_string(*seq));
    }
    auto encoded = ::planar::engine::codec_encode(record_node(std::move(fields)));
    if (!encoded)
      return std::unexpected(invalid_error());
    saved = *encoded;
  } else {
    hist::claim_result_v1 result;
    result.ledger        = metadata.identity;
    result.request_id    = change.request_id;
    result.job           = change.job;
    result.role          = change.role;
    result.revision      = std::to_string(*revision_id);
    result.run_id        = std::to_string(*run_id);
    result.generation    = std::to_string(ordinal);
    result.token         = token_text;
    result.dispatch_id   = std::to_string(*dispatch_id);
    result.work_fp       = prepared.work_text;
    result.render_digest = prepared.render_text;
    result.bytes         = prepared.markdown.size();
    if (takeover)
      result.previous_run = std::to_string(*change.previous_run);
    auto encoded = hist::encode_claim_result_v1(result);
    if (!encoded)
      return std::unexpected(invalid_error());
    saved = *encoded;
  }

  // Event row, then the request row that names it.
  std::vector<bind_arg> event_args{int_arg(*seq),
                                   text_arg(event_type),
                                   optional_int_arg(revision_id),
                                   text_arg(change.request_id),
                                   text_arg(change.actor),
                                   text_arg(now),
                                   blob_arg(*payload_bytes),
                                   (activate || run_op || token_op) ? text_arg(change.job) : null_arg(),
                                   (run_op || token_op) ? text_arg(change.role) : null_arg(),
                                   (run_op || token_op) ? int_arg(*target_generation) : null_arg(),
                                   (run_op || token_op) ? int_arg(*run_id) : null_arg()};
  if (auto inserted = run_bound(db,
                                "INSERT INTO events(seq,type,version,revision_id,request_id,actor,occurred_at,payload,job,role,"
                                "generation_id,run_id) VALUES(?1,?2,2,?3,?4,?5,?6,?7,?8,?9,?10,?11)",
                                event_args);
      !inserted)
    return std::unexpected(inserted.error());

  for (std::size_t index = 0; index < change.generations.size(); ++index) {
    const auto& input = change.generations[index];
    const auto  id    = generation_ids[index];
    if (auto inserted = run_bound(
            db, "INSERT INTO generations(id,job,role,ordinal,structure_id,opened_seq,reason) VALUES(?1,?2,?3,?4,?5,?6,?7)",
            {int_arg(id), text_arg(input.job), text_arg(input.role), int_arg(ordinal_of_generation[id]),
             int_arg(structure_ids[index]), int_arg(*seq), text_arg(input.reason)});
        !inserted)
      return std::unexpected(inserted.error());
  }

  const std::string outcome = accepted     ? "accepted"
                              : noop       ? "noop"
                              : activate   ? "activated"
                              : claim      ? "claimed"
                              : takeover   ? "taken_over"
                              : refresh    ? "refreshed"
                              : checkpoint ? "recorded"
                                           : change.release_reason;
  if (auto inserted =
          run_bound(db,
                    "INSERT INTO requests(request_id,operation,body_id,event_seq,revision_id,outcome,result,attachment_id) "
                    "VALUES(?1,?2,?3,?4,?5,?6,?7,?8)",
                    {text_arg(change.request_id), text_arg(change.operation), int_arg(*request_record), int_arg(*seq),
                     int_arg(*revision_id), text_arg(outcome), blob_arg(saved),
                     optional_int_arg(dispatch_op ? packet_id : std::optional<std::uint64_t>{})});
      !inserted)
    return std::unexpected(inserted.error());

  for (std::size_t index = 0; index < change.transitions.size(); ++index) {
    const auto& transition = change.transitions[index];
    if (auto inserted = run_bound(db,
                                  "INSERT INTO transitions(event_seq,ordinal,job,role,generation_id,before_state,after_state,"
                                  "before_drifted,after_drifted,reason) VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9,?10)",
                                  {int_arg(*seq), int_arg(index + 1), text_arg(transition.job), text_arg(transition.role),
                                   int_arg(transition_generation[index]),
                                   transition.before_state ? text_arg(*transition.before_state) : null_arg(),
                                   text_arg(transition.after_state),
                                   transition.before_drifted ? int_arg(*transition.before_drifted ? 1 : 0) : null_arg(),
                                   int_arg(transition.after_drifted ? 1 : 0), text_arg(transition.reason)});
        !inserted)
      return std::unexpected(inserted.error());
  }

  if (activate) {
    if (auto inserted = run_bound(db, "INSERT INTO job_activations(id,job,revision_id,event_seq) VALUES(?1,?2,?3,?4)",
                                  {int_arg(*activation_id), text_arg(change.job), int_arg(*revision_id), int_arg(*seq)});
        !inserted)
      return std::unexpected(inserted.error());
  }

  // Fenced runs were active at the lock; their state and sequence move to this event.
  // This precedes the new run's insert so the one-active-run-per-role index never sees two active rows.
  for (auto fenced : change.fenced_runs) {
    if (auto updated = run_bound(db, "UPDATE run_state SET state = 'fenced', last_seq = ?1 WHERE run_id = ?2",
                                 {int_arg(*seq), int_arg(fenced)});
        !updated)
      return std::unexpected(updated.error());
  }

  if (run_op) {
    const std::string secret_bytes(reinterpret_cast<const char*>(secret.data()), secret.size());
    if (auto inserted = run_bound(
            db, "INSERT INTO runs(id,generation_id,request_id,claimed_seq,secret,provenance_id) VALUES(?1,?2,?3,?4,?5,?6)",
            {int_arg(*run_id), int_arg(*target_generation), text_arg(change.request_id), int_arg(*seq), blob_arg(secret_bytes),
             optional_int_arg(provenance_id)});
        !inserted)
      return std::unexpected(inserted.error());
    if (auto inserted = run_bound(
            db, "INSERT INTO run_state(run_id,job,role,generation_id,state,last_seq) VALUES(?1,?2,?3,?4,'active',?5)",
            {int_arg(*run_id), text_arg(change.job), text_arg(change.role), int_arg(*target_generation), int_arg(*seq)});
        !inserted)
      return std::unexpected(inserted.error());
  }
  if (checkpoint_id) {
    auto ok = run_bound(db, "INSERT INTO checkpoints(id,run_id,seq,body_id) VALUES(?1,?2,?3,?4)",
                        {int_arg(*checkpoint_id), int_arg(*run_id), int_arg(*seq), int_arg(*checkpoint_body)});
    if (!ok)
      return std::unexpected(ok.error());
  }
  if (release) {
    auto ok = run_bound(db, "UPDATE run_state SET state=?1,last_seq=?2 WHERE run_id=?3",
                        {text_arg(change.release_reason), int_arg(*seq), int_arg(*run_id)});
    if (!ok)
      return std::unexpected(ok.error());
  }
  if (dispatch_op) {
    std::optional<std::uint64_t> dispatch_checkpoint = checkpoint_body;
    if (refresh && !dispatch_checkpoint) {
      auto row = first_row(db, "SELECT body_id FROM checkpoints WHERE run_id=?1 ORDER BY seq DESC LIMIT 1", {int_arg(*run_id)});
      if (!row)
        return std::unexpected(row.error());
      if (*row) {
        row_reader r(**row);
        dispatch_checkpoint = r.id(0);
        if (auto ok = r.finish(); !ok)
          return std::unexpected(ok.error());
      }
    }
    if (auto inserted =
            run_bound(db,
                      "INSERT INTO dispatches(id,run_id,seq,revision_id,kind,packet_id,manifest_id,work_id,work_fp,render_fp,"
                      "checkpoint_id) VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11)",
                      {int_arg(*dispatch_id), int_arg(*run_id), int_arg(*seq), int_arg(*revision_id),
                       text_arg(claim      ? "claim"
                                : takeover ? "takeover"
                                           : "refresh"),
                       int_arg(*packet_id), int_arg(*manifest_id), int_arg(*work_id), blob_arg(work_fp), blob_arg(render_fp),
                       optional_int_arg(dispatch_checkpoint)});
        !inserted)
      return std::unexpected(inserted.error());
  }

  // The delta write of role_state: only transitioned roles, facts bumped per D16, the locked before state rechecked above.
  const bool bumping = accepted || activate || takeover;
  for (std::size_t index = 0; index < change.transitions.size(); ++index) {
    const auto&                  transition = change.transitions[index];
    const auto&                  before     = before_rows[index];
    std::optional<std::uint64_t> run;
    if (transition.after_state == "running") {
      run = (run_op && index == 0) ? run_id : (before ? before->run : std::nullopt);
      if (!run)
        return std::unexpected(invalid_error());
    }
    const std::uint64_t fact = before ? before->fact_version + (bumping ? 1 : 0) : 1;
    const auto          args = std::vector<bind_arg>{text_arg(transition.job),
                                                     text_arg(transition.role),
                                                     int_arg(transition_generation[index]),
                                                     text_arg(transition.after_state),
                                                     int_arg(transition.after_drifted ? 1 : 0),
                                                     optional_int_arg(run),
                                                     int_arg(fact),
                                                     int_arg(*seq)};
    if (before) {
      if (auto updated =
              run_bound(db,
                        "UPDATE role_state SET generation_id = ?3, state = ?4, drifted = ?5, run_id = ?6, fact_version = ?7, "
                        "last_seq = ?8 WHERE job = ?1 AND role = ?2",
                        args);
          !updated)
        return std::unexpected(updated.error());
    } else {
      if (auto inserted = run_bound(db,
                                    "INSERT INTO role_state(job,role,generation_id,state,drifted,run_id,fact_version,last_seq) "
                                    "VALUES(?1,?2,?3,?4,?5,?6,?7,?8)",
                                    args);
          !inserted)
        return std::unexpected(inserted.error());
    }
  }

  // R18: blockers this load or activate clears (DB-R7); a load that keeps its job clears only that job's blockers.
  if (!change.blockers_cleared.empty()) {
    const bool        any_job = activate || change.job.empty();
    const std::string job     = any_job ? std::string{} : head->job;
    if (auto cleared = clear_blockers(db, change.blockers_cleared, *seq, job); !cleared)
      return std::unexpected(cleared.error());
  }

  // The head, the active job, the activation and the sequence of the current state.
  const auto new_activation = activate ? activation_id : head->activation;
  if (auto updated = run_bound(db,
                               "UPDATE current_state SET active_revision_id = ?1, last_seq = ?2, active_job = ?3, "
                               "active_activation_id = ?4 WHERE singleton = 1",
                               {optional_int_arg(revision_id), int_arg(*seq),
                                change.job.empty() ? null_arg() : text_arg(change.job), optional_int_arg(new_activation)});
      !updated)
    return std::unexpected(updated.error());

  // (9b) The shared cross-row verifier on the uncommitted state (D47): any violation is an invalid change.
  ledger_rows rows;
  if (auto verified_rows = verify_rows(db, metadata, rows); !verified_rows)
    return std::unexpected(invalid_error());

  locked_outcome outcome_value;
  outcome_value.result.replayed     = false;
  outcome_value.result.saved_result = saved;
  outcome_value.result.event_seq    = *seq;
  if (run_op)
    outcome_value.result.token = token_text;
  return outcome_value;
}

} // namespace

/// Private state of an open execution write: the verified database and its single immediate transaction.
struct execution_write::impl {
  /// Takes ownership of the verified database.
  /// @param value The verified execution database.
  explicit impl(execution_database value) : database(std::move(value)) {
  }

  execution_database         database; ///< Verified ledger owner; declared first so the transaction ends before it.
  std::optional<transaction> tx;       ///< The open BEGIN IMMEDIATE transaction, absent after commit or abort.
  execution_commit_result    result;   ///< Result of the staged change.
};

execution_write::execution_write(execution_write&&) noexcept            = default;
execution_write& execution_write::operator=(execution_write&&) noexcept = default;
execution_write::~execution_write()                                     = default;

execution_write::execution_write(std::unique_ptr<impl> value) noexcept : _impl(std::move(value)) {
}

const execution_commit_result& execution_write::result() const {
  return _impl->result;
}

std::expected<void, execution_error> execution_write::commit() {
  if (_impl->result.replayed)
    return {};
  if (!_impl->tx)
    return std::unexpected(failure(execution_error_code::io));
  auto committed = _impl->tx->commit();
  _impl->tx.reset();
  if (!committed)
    return std::unexpected(map_sqlite(committed.error()));
  return {};
}

void execution_write::abort() noexcept {
  if (_impl && _impl->tx) {
    (void)_impl->tx->rollback();
    _impl->tx.reset();
  }
}

std::expected<execution_write, execution_error>
stage_execution(const std::filesystem::path& path, const execution_snapshot& snapshot, const execution_change& change) {
  // The format check runs first, so a format1 or format2 ledger is refused before any other work (V5).
  auto database = open_format3(path, execution_access::read_write);
  if (!database)
    return std::unexpected(database.error());
  if (auto shaped = check_shape(change); !shaped)
    return std::unexpected(shaped.error());
  auto prepared = prepare_change(change);
  if (!prepared)
    return std::unexpected(prepared.error());
  if (auto requested = check_request(change); !requested)
    return std::unexpected(requested.error());
  // D7: a used request id is answered under the lock (saved bytes or request_conflict) before any precondition, so a
  // change built from a snapshot taken after the original commit must not be refused by the snapshot checks first.
  auto used = first_row(database->sql(), "SELECT 1 FROM requests WHERE request_id = ?1", {text_arg(change.request_id)});
  if (!used)
    return std::unexpected(used.error());
  if (!*used) {
    if (change.operation == "verdict") { // R18: native step-5 checks (execution_review.cpp).
      if (auto checked = check_verdict_snapshot(snapshot, change); !checked)
        return std::unexpected(checked.error());
    } else {
      if (auto snapshot_checked = check_snapshot(snapshot, change); !snapshot_checked)
        return std::unexpected(snapshot_checked.error());
      if (auto native = check_native(change, *prepared); !native)
        return std::unexpected(native.error());
      if (auto cleared = check_blockers_cleared(snapshot, change); !cleared) // R18: DB-R7
        return std::unexpected(cleared.error());
    }
  }

  auto staged = std::make_unique<execution_write::impl>(std::move(*database));
  auto begun  = staged->database.sql().begin(true);
  if (!begun)
    return std::unexpected(map_sqlite(begun.error()));
  staged->tx.emplace(std::move(*begun));
  auto outcome = apply_locked(staged->database, change, snapshot, *prepared);
  if (!outcome)
    return std::unexpected(outcome.error());
  if (outcome->replayed) {
    (void)staged->tx->rollback();
    staged->tx.reset();
  }
  staged->result = outcome->result;
  return execution_write(std::move(staged));
}

std::expected<execution_commit_result, execution_error>
commit_execution(const std::filesystem::path& path, const execution_snapshot& snapshot, const execution_change& change) {
  auto staged = stage_execution(path, snapshot, change);
  if (!staged)
    return std::unexpected(staged.error());
  if (auto committed = staged->commit(); !committed)
    return std::unexpected(committed.error());
  return staged->result();
}

namespace {
value_ptr check_integer(std::uint64_t n) {
  value v{};
  v.kind    = value_kind::integer;
  v.integer = static_cast<std::int64_t>(n);
  return make_node(std::move(v));
}
struct check_run_row {
  std::uint64_t id = 0, generation = 0, ordinal = 0, dispatch = 0, revision = 0;
  std::string   job, role, state, work_fp;
};
value_result<check_run_row> check_run_locked(connection& db, std::uint64_t run, std::optional<run_token> token = {}) {
  auto row = first_row(
      db,
      "SELECT s.job,s.role,s.generation_id,s.state,g.ordinal,r.secret,rs.generation_id,rs.run_id,"
      "d.id,d.revision_id,d.work_fp FROM runs r JOIN run_state s ON s.run_id=r.id JOIN generations g ON g.id=r.generation_id "
      "JOIN role_state rs ON rs.job=s.job AND rs.role=s.role JOIN dispatches d ON d.run_id=r.id "
      "WHERE r.id=?1 ORDER BY d.seq DESC LIMIT 1",
      {int_arg(run)});
  if (!row)
    return std::unexpected(row.error());
  if (!*row)
    return std::unexpected(failure(execution_error_code::unknown_run));
  row_reader    q(**row);
  check_run_row out;
  out.id                  = run;
  out.job                 = q.text(0);
  out.role                = q.text(1);
  out.generation          = q.id(2);
  out.state               = q.text(3);
  out.ordinal             = q.id(4);
  auto secret             = q.blob(5);
  auto current_generation = q.id(6);
  auto active             = q.optional_id(7);
  out.dispatch            = q.id(8);
  out.revision            = q.id(9);
  out.work_fp             = q.blob(10);
  if (auto ok = q.finish(); !ok)
    return std::unexpected(ok.error());
  if (token) {
    bool matches = secret.size() == token->secret.size();
    for (std::size_t i = 0; i < secret.size() && i < token->secret.size(); ++i)
      matches = matches && static_cast<unsigned char>(secret[i]) == token->secret[i];
    if (!matches || out.ordinal != token->generation_ordinal || out.state != "active" || current_generation != out.generation ||
        active != run) {
      auto error  = failure(execution_error_code::fenced);
      error.state = out.state;
      return std::unexpected(error);
    }
  }
  return out;
}
checked check_identity_locked(connection& db, const execution_snapshot& snapshot) {
  auto id = first_row(db, "SELECT identity FROM ledger_meta");
  if (!id)
    return std::unexpected(id.error());
  if (!*id)
    return std::unexpected(corrupt());
  row_reader r(**id);
  auto       identity = r.text(0);
  if (auto ok = r.finish(); !ok)
    return ok;
  if (identity != snapshot.identity)
    return std::unexpected(failure(execution_error_code::path_denied));
  return {};
}
checked check_snapshot_locked(connection& db, const execution_snapshot& snapshot, bool facts) {
  if (auto ok = check_identity_locked(db, snapshot); !ok)
    return ok;
  auto head = read_head(db);
  if (!head)
    return std::unexpected(head.error());
  if (head->job != snapshot.active_job ||
      head->revision != (snapshot.revision ? std::optional(snapshot.revision->id) : std::nullopt))
    return std::unexpected(failure(execution_error_code::state_conflict));
  if (facts)
    for (const auto& role : snapshot.roles)
      if (role.job == snapshot.active_job) {
        auto now = locked_role(db, role.job, role.role);
        if (!now)
          return std::unexpected(now.error());
        if (!*now || (*now)->fact_version != role.fact_version)
          return std::unexpected(failure(execution_error_code::state_conflict));
      }
  return {};
}
value_result<std::uint64_t> check_value_record(connection& db, std::string_view codec, const value_ptr& v) {
  auto bytes = hist::encode_check_value(codec, v);
  if (!bytes)
    return std::unexpected(invalid_error());
  auto art = value_artifact(codec, *bytes);
  if (!art)
    return std::unexpected(art.error());
  return intern(db, *art);
}
value_result<std::uint64_t> check_file_record(connection& db, std::string bytes, std::uint32_t mode = 420) {
  hist::record file   = hist::file_record{mode, std::move(bytes)};
  auto         frame  = hist::encode_record(file);
  auto         digest = hist::digest_record(file);
  if (!frame || !digest)
    return std::unexpected(invalid_error());
  return intern(db, artifact{1, 1, *frame, digest_bytes(*digest), "file"});
}
value_result<std::uint64_t> check_request_record(connection& db, const hist::request_v3& request) {
  auto bytes  = hist::encode_request_v3(request);
  auto digest = hist::digest_request_v3(request);
  if (!bytes || !digest)
    return std::unexpected(invalid_error());
  return intern(db, artifact{4, 3, *bytes, digest_bytes(*digest), "request"});
}
checked check_event(connection& db, const check_run_row& run, std::uint64_t revision, std::string_view job, std::uint64_t seq,
                    std::string_view type, std::string_view request, std::string_view actor,
                    const std::map<std::string, std::string>& refs, std::string reason = {}) {
  hist::event_payload_v2 p;
  p.operation  = type == "check_interrupted" ? "check-recover" : "check";
  p.before     = {std::to_string(revision), std::string(job)};
  p.after      = p.before;
  p.references = refs;
  p.reason     = reason;
  auto payload = hist::encode_event_payload_v2(type, p);
  if (!payload)
    return std::unexpected(invalid_error());
  return run_bound(
      db,
      "INSERT INTO events(seq,type,version,revision_id,request_id,actor,occurred_at,payload,job,role,generation_id,run_id) "
      "VALUES(?1,?2,2,?3,?4,?5,?6,?7,?8,?9,?10,?11)",
      {int_arg(seq), text_arg(type), int_arg(revision), text_arg(request), text_arg(actor), text_arg(commit_timestamp()),
       blob_arg(*payload), text_arg(run.job), text_arg(run.role), int_arg(run.generation), int_arg(run.id)});
}
value_ptr check_response(std::string_view identity, std::string_view request, const check_run_row& run, std::uint64_t reservation,
                         std::string_view outcome, value_ptr evidence, value_ptr observation, value_ptr completion,
                         const std::vector<std::string>& withheld) {
  return record_node(
      {{"version", check_integer(1)},
       {"kind", text_node("check")},
       {"ledger", text_node(identity)},
       {"request_id", text_node(request)},
       {"job", text_node(run.job)},
       {"role", text_node(run.role)},
       {"run", record_node({{"id", text_node(std::to_string(run.id))}, {"generation", text_node(std::to_string(run.ordinal))}})},
       {"reservation", text_node(std::to_string(reservation))},
       {"outcome", text_node(outcome)},
       {"withheld", texts_node(withheld)},
       {"evidence", evidence},
       {"observation", observation},
       {"completion", completion}});
}
} // namespace

std::expected<execution_reservation, execution_error>
reserve_check(const std::filesystem::path& path, const execution_snapshot& snapshot, std::string_view request_id,
              std::string_view actor, const hist::request_v3& request, std::string_view token_text, const value_ptr& provenance) {
  auto token = parse_run_token(token_text);
  if (!token)
    return std::unexpected(failure(execution_error_code::unknown_run));
  if (request.operation != "check" || request.actor != actor)
    return std::unexpected(invalid_error());
  auto database = open_format3(path, execution_access::read_write);
  if (!database)
    return std::unexpected(database.error());
  auto& db = database->sql();
  auto  tx = db.begin(true);
  if (!tx)
    return std::unexpected(map_sqlite(tx.error()));
  if (auto ok = check_snapshot_locked(db, snapshot, false); !ok)
    return std::unexpected(ok.error());
  auto run = check_run_locked(db, token->run_id, token);
  if (!run)
    return std::unexpected(run.error());
  auto role = locked_role(db, run->job, run->role);
  if (!role)
    return std::unexpected(role.error());
  if (!*role || run->job != snapshot.active_job)
    return std::unexpected(failure(execution_error_code::state_conflict));
  if ((*role)->drifted)
    return std::unexpected(failure(execution_error_code::drifted));
  auto prior = first_row(db, "SELECT 1 FROM requests WHERE request_id=?1", {text_arg(request_id)});
  if (!prior)
    return std::unexpected(prior.error());
  if (*prior)
    return std::unexpected(failure(execution_error_code::request_conflict));
  auto open = first_row(db,
                        "SELECT 1 FROM reservations r JOIN reservation_state s ON s.reservation_id=r.id WHERE (r.run_id=?1 AND "
                        "s.state='open') OR r.request_id=?2",
                        {int_arg(run->id), text_arg(request_id)});
  if (!open)
    return std::unexpected(open.error());
  if (*open)
    return std::unexpected(failure(execution_error_code::check_in_progress));
  auto seq  = next_value(db, "SELECT last_seq FROM current_state WHERE singleton=1"),
       id   = next_value(db, "SELECT COALESCE(MAX(id),0) FROM reservations");
  auto body = check_request_record(db, request);
  if (!seq)
    return std::unexpected(seq.error());
  if (!id)
    return std::unexpected(id.error());
  if (!body)
    return std::unexpected(body.error());
  std::map<std::string, std::string> refs{
      {"reservation", std::to_string(*id)}, {"run", std::to_string(run->id)}, {"dispatch", std::to_string(run->dispatch)}};
  if (provenance) {
    auto bytes = hist::encode_driver_provenance_v1(provenance);
    if (!bytes)
      return std::unexpected(invalid_error());
    auto art = value_artifact(hist::codec_driver_provenance_v1, *bytes);
    if (!art)
      return std::unexpected(art.error());
    auto pid = intern(db, *art);
    if (!pid)
      return std::unexpected(pid.error());
    refs["provenance"] = std::to_string(*pid);
  }
  if (auto ok =
          check_event(db, *run, snapshot.revision->id, snapshot.active_job, *seq, "check_reserved", request_id, actor, refs);
      !ok)
    return std::unexpected(ok.error());
  if (auto ok = run_bound(db, "INSERT INTO reservations(id,request_id,run_id,opened_seq,body_id) VALUES(?1,?2,?3,?4,?5)",
                          {int_arg(*id), text_arg(request_id), int_arg(run->id), int_arg(*seq), int_arg(*body)});
      !ok)
    return std::unexpected(ok.error());
  if (auto ok = run_bound(db, "INSERT INTO reservation_state(reservation_id,state,last_seq) VALUES(?1,'open',?2)",
                          {int_arg(*id), int_arg(*seq)});
      !ok)
    return std::unexpected(ok.error());
  if (auto ok = run_bound(db, "UPDATE current_state SET last_seq=?1 WHERE singleton=1", {int_arg(*seq)}); !ok)
    return std::unexpected(ok.error());
  ledger_rows verified_rows;
  auto        verified = verify_rows(db, database->metadata(), verified_rows);
  if (!verified)
    return std::unexpected(invalid_error());
  if (auto ok = tx->commit(); !ok)
    return std::unexpected(map_sqlite(ok.error()));
  return execution_reservation{*id, run->id, *seq, std::string(request_id)};
}

std::expected<execution_commit_result, execution_error>
complete_check(const std::filesystem::path& path, const execution_snapshot& snapshot, std::uint64_t reservation,
               std::string_view token_text, const execution_check_result& result, bool completion,
               const std::vector<execution_transition_input>& transitions, const std::vector<std::uint64_t>& blockers,
               const std::vector<std::string>& withheld) {
  auto token = parse_run_token(token_text);
  if (!token)
    return std::unexpected(failure(execution_error_code::unknown_run));
  if (!result.observation || result.evidence.empty() || result.evidence.size() > 1024)
    return std::unexpected(invalid_error());
  if (completion && (!result.recapture_matches || std::ranges::any_of(result.evidence, [](const auto& e) {
        return e.outcome != "pass";
      }) || transitions.empty()))
    return std::unexpected(invalid_error());
  auto database = open_format3(path, execution_access::read_write);
  if (!database)
    return std::unexpected(database.error());
  auto& db = database->sql();
  auto  tx = db.begin(true);
  if (!tx)
    return std::unexpected(map_sqlite(tx.error()));
  if (auto ok = check_identity_locked(db, snapshot); !ok)
    return std::unexpected(ok.error());
  auto run = check_run_locked(db, token->run_id, token);
  if (!run)
    return std::unexpected(run.error());
  if (completion) {
    if (auto ok = check_snapshot_locked(db, snapshot, true); !ok)
      return std::unexpected(ok.error());
  }
  auto row = first_row(db,
                       "SELECT r.request_id,r.run_id,r.opened_seq,r.body_id,s.state,e.actor,e.revision_id FROM reservations r "
                       "JOIN reservation_state s ON s.reservation_id=r.id JOIN events e ON e.seq=r.opened_seq WHERE r.id=?1",
                       {int_arg(reservation)});
  if (!row)
    return std::unexpected(row.error());
  if (!*row)
    return std::unexpected(failure(execution_error_code::unknown_reservation));
  row_reader q(**row);
  auto       request      = q.text(0);
  auto       reserved_run = q.id(1), opened = q.id(2), body = q.id(3);
  auto       state = q.text(4), actor = q.text(5);
  auto       reserved_revision = q.id(6);
  if (auto ok = q.finish(); !ok)
    return std::unexpected(ok.error());
  if (reserved_run != run->id || state != "open")
    return std::unexpected(failure(execution_error_code::state_conflict));
  auto seq = next_value(db, "SELECT last_seq FROM current_state WHERE singleton=1");
  if (!seq)
    return std::unexpected(seq.error());
  auto head = read_head(db);
  if (!head)
    return std::unexpected(head.error());
  if (!head->revision)
    return std::unexpected(corrupt());
  auto observation = check_value_record(db, hist::codec_check_observation_v1, result.observation);
  if (!observation)
    return std::unexpected(observation.error());
  std::vector<value_ptr>              evidence_values;
  std::pair<std::string, std::string> previous;
  for (const auto& e : result.evidence) {
    auto key = std::pair(e.work, e.check);
    if (!previous.first.empty() && key <= previous)
      return std::unexpected(invalid_error());
    previous            = key;
    std::string outcome = result.recapture_matches ? e.outcome : "inconclusive",
                reason  = result.recapture_matches ? e.reason : "input_changed";
    bool reason_ok = (outcome == "pass" && reason == "passed") || (outcome == "inconclusive" && reason == "input_changed") ||
                     (outcome == "fail" &&
                      (reason == "exit_status" || reason == "zero_selected" || reason == "missing_executable" ||
                       reason == "timeout" || reason == "output_limit" || reason == "malformed_result" || reason == "boundary"));
    if (!reason_ok || e.output.size() > e.output_bytes)
      return std::unexpected(invalid_error());
    auto definition = check_value_record(db, hist::codec_check_definition_v1, e.definition),
         vector     = check_value_record(db, hist::codec_output_vector_v1, e.vector);
    if (!definition)
      return std::unexpected(definition.error());
    if (!vector)
      return std::unexpected(vector.error());
    std::optional<std::uint64_t> output;
    if (!e.output.empty()) {
      auto file = check_file_record(db, e.output);
      if (!file)
        return std::unexpected(file.error());
      output = *file;
    }
    auto id = next_value(db, "SELECT COALESCE(MAX(id),0) FROM evidence");
    if (!id)
      return std::unexpected(id.error());
    if (auto ok = run_bound(
            db,
            "INSERT INTO "
            "evidence(id,reservation_id,seq,work,\"check\",definition_id,vector_id,outcome,reason,matched,exit_status,duration_"
            "ms,output_id,output_bytes,output_truncated) VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14,?15)",
            {int_arg(*id), int_arg(reservation), int_arg(*seq), text_arg(e.work), text_arg(e.check), int_arg(*definition),
             int_arg(*vector), text_arg(outcome), text_arg(reason), optional_int_arg(e.matched),
             e.exit_status ? bind_arg{bind_kind::integer, static_cast<std::int64_t>(*e.exit_status), {}} : null_arg(),
             int_arg(e.duration_ms), optional_int_arg(output), int_arg(e.output_bytes), int_arg(e.output_truncated ? 1 : 0)});
        !ok)
      return std::unexpected(ok.error());
    evidence_values.push_back(record_node({{"evidence", text_node(std::to_string(*id))},
                                           {"work", text_node(e.work)},
                                           {"check", text_node(e.check)},
                                           {"outcome", text_node(outcome)},
                                           {"reason", text_node(reason)},
                                           {"matched", text_node(e.matched ? std::to_string(*e.matched) : "none")},
                                           {"exit_status", text_node(e.exit_status ? std::to_string(*e.exit_status) : "none")},
                                           {"duration_ms", check_integer(e.duration_ms)},
                                           {"output_bytes", check_integer(e.output_bytes)},
                                           {"output_truncated", bool_node(e.output_truncated)},
                                           {"output", text_node(output ? std::to_string(*output) : "none")}}));
  }
  std::optional<std::uint64_t> completion_id;
  value_ptr                    completion_value = text_node("none");
  if (completion) {
    if (transitions.front().role != run->role || transitions.front().job != run->job ||
        transitions.front().after_state != "done" || transitions.front().reason != "completed")
      return std::unexpected(invalid_error());
    auto id = next_value(db, "SELECT COALESCE(MAX(id),0) FROM completions");
    if (!id)
      return std::unexpected(id.error());
    completion_id = *id;
    if (auto ok =
            run_bound(db, "INSERT INTO completions(id,run_id,seq,dispatch_id,work_fp,observation_id) VALUES(?1,?2,?3,?4,?5,?6)",
                      {int_arg(*id), int_arg(run->id), int_arg(*seq), int_arg(run->dispatch), blob_arg(run->work_fp),
                       int_arg(*observation)});
        !ok)
      return std::unexpected(ok.error());
    std::vector<value_ptr> published;
    for (const auto& pub : result.publications) {
      auto semantic = digest_bytes_of_text(pub.semantic);
      if (!semantic)
        return std::unexpected(invalid_error());
      // Build the retained facet hierarchy from portable paths; file records retain exact bytes and normalized modes.
      struct directory {
        std::map<std::string, directory>                         dirs;
        std::map<std::string, const execution_publication_file*> files;
      };
      directory root;
      for (const auto& file : pub.files) {
        if (!valid_path(file.path))
          return std::unexpected(invalid_error());
        auto*            d    = &root;
        std::string_view rest = file.path;
        while (rest.find('/') != std::string_view::npos) {
          auto slash = rest.find('/');
          d          = &d->dirs[std::string(rest.substr(0, slash))];
          rest.remove_prefix(slash + 1);
        }
        if (!d->files.emplace(std::string(rest), &file).second)
          return std::unexpected(invalid_error());
      }
      std::function<value_result<std::pair<std::uint64_t, hist::record_digest>>(const directory&)> store_dir;
      store_dir = [&](const directory& dir) -> value_result<std::pair<std::uint64_t, hist::record_digest>> {
        hist::directory_record tree;
        for (const auto& [name, sub] : dir.dirs) {
          auto stored = store_dir(sub);
          if (!stored)
            return std::unexpected(stored.error());
          tree.children.push_back({name, hist::record_kind::directory, stored->first, stored->second});
        }
        for (const auto& [name, file] : dir.files) {
          auto id     = check_file_record(db, file->bytes, file->mode);
          auto digest = hist::digest_record(hist::file_record{file->mode, file->bytes});
          if (!id)
            return std::unexpected(id.error());
          if (!digest)
            return std::unexpected(invalid_error());
          tree.children.push_back({name, hist::record_kind::file, *id, *digest});
        }
        std::ranges::sort(tree.children, {}, &hist::record_child::name);
        hist::record record = tree;
        auto         frame  = hist::encode_record(record);
        auto         digest = hist::digest_record(record);
        if (!frame || !digest)
          return std::unexpected(invalid_error());
        auto id = intern_codec1(db, record);
        if (!id)
          return std::unexpected(id.error());
        return std::pair(*id, *digest);
      };
      auto payload = store_dir(root);
      if (!payload)
        return std::unexpected(payload.error());
      auto pid = next_value(db, "SELECT COALESCE(MAX(id),0) FROM publications");
      if (!pid)
        return std::unexpected(pid.error());
      if (auto ok =
              run_bound(db, "INSERT INTO publications(id,completion_id,work,facet,semantic,payload_id) VALUES(?1,?2,?3,?4,?5,?6)",
                        {int_arg(*pid), int_arg(*completion_id), text_arg(pub.work), text_arg(pub.facet), blob_arg(*semantic),
                         int_arg(payload->first)});
          !ok)
        return std::unexpected(ok.error());
      published.push_back(record_node({{"work", text_node(pub.work)},
                                       {"facet", text_node(pub.facet)},
                                       {"semantic", text_node(pub.semantic)},
                                       {"publication", text_node(std::to_string(*pid))}}));
    }
    completion_value = record_node({{"id", text_node(std::to_string(*completion_id))},
                                    {"work_fp", text_node("xxh3_128:" + hex_of(run->work_fp))},
                                    {"publications", list_node(std::move(published))}});
  }
  for (std::size_t i = 0; i < transitions.size(); ++i) {
    const auto& t      = transitions[i];
    auto        before = locked_role(db, t.job, t.role);
    if (!before)
      return std::unexpected(before.error());
    if (!*before || t.before_state != (*before)->state || t.before_drifted != (*before)->drifted)
      return std::unexpected(failure(execution_error_code::state_conflict));
    if (auto ok = run_bound(db,
                            "INSERT INTO "
                            "transitions(event_seq,ordinal,job,role,generation_id,before_state,after_state,before_drifted,after_"
                            "drifted,reason) VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9,?10)",
                            {int_arg(*seq), int_arg(i + 1), text_arg(t.job), text_arg(t.role), int_arg((*before)->generation),
                             text_arg((*before)->state), text_arg(t.after_state), int_arg((*before)->drifted ? 1 : 0),
                             int_arg(t.after_drifted ? 1 : 0), text_arg(t.reason)});
        !ok)
      return std::unexpected(ok.error());
    if (auto ok = run_bound(db,
                            "UPDATE role_state SET state=?1,drifted=?2,run_id=?3,fact_version=fact_version+1,last_seq=?4 WHERE "
                            "job=?5 AND role=?6",
                            {text_arg(t.after_state), int_arg(t.after_drifted ? 1 : 0),
                             t.after_state == "running" ? optional_int_arg((*before)->run) : null_arg(), int_arg(*seq),
                             text_arg(t.job), text_arg(t.role)});
        !ok)
      return std::unexpected(ok.error());
  }
  if (completion) {
    if (auto ok = run_bound(db, "UPDATE run_state SET state='completed',last_seq=?1 WHERE run_id=?2",
                            {int_arg(*seq), int_arg(run->id)});
        !ok)
      return std::unexpected(ok.error());
  }
  if (auto ok = clear_blockers(db, blockers, *seq, run->job); !ok)
    return std::unexpected(ok.error());
  std::map<std::string, std::string> refs{{"reservation", std::to_string(reservation)},
                                          {"observation", std::to_string(*observation)}};
  if (completion_id)
    refs["completion"] = std::to_string(*completion_id);
  if (auto ok = check_event(db, *run, *head->revision, head->job, *seq, "check_recorded", request, actor, refs,
                            transitions.empty() ? "" : transitions.front().reason);
      !ok)
    return std::unexpected(ok.error());
  auto obs_digest = hash_value("check_observation_v1", result.observation);
  if (!obs_digest)
    return std::unexpected(invalid_error());
  auto observation_value = record_node({{"id", text_node(std::to_string(*observation))},
                                        {"digest", text_node(*obs_digest)},
                                        {"units", result.observation->fields.at("units")}});
  auto response = check_response(database->metadata().identity, request, *run, reservation, completion ? "completed" : "recorded",
                                 list_node(std::move(evidence_values)), observation_value, completion_value, withheld);
  auto saved    = codec_encode(response);
  if (!saved || saved->size() > 1048576)
    return std::unexpected(invalid_error());
  if (auto ok = run_bound(db,
                          "INSERT INTO requests(request_id,operation,body_id,event_seq,revision_id,outcome,result,attachment_id) "
                          "VALUES(?1,'check',?2,?3,?4,'recorded',?5,NULL)",
                          {text_arg(request), int_arg(body), int_arg(opened), int_arg(reserved_revision), blob_arg(*saved)});
      !ok)
    return std::unexpected(ok.error());
  if (auto ok = run_bound(db, "UPDATE reservation_state SET state='completed',last_seq=?1 WHERE reservation_id=?2",
                          {int_arg(*seq), int_arg(reservation)});
      !ok)
    return std::unexpected(ok.error());
  if (auto ok = run_bound(db, "UPDATE current_state SET last_seq=?1 WHERE singleton=1", {int_arg(*seq)}); !ok)
    return std::unexpected(ok.error());
  ledger_rows verified_rows;
  auto        verified = verify_rows(db, database->metadata(), verified_rows);
  if (!verified)
    return std::unexpected(invalid_error());
  if (auto ok = tx->commit(); !ok)
    return std::unexpected(map_sqlite(ok.error()));
  return execution_commit_result{false, *saved, *seq, {}};
}
std::expected<execution_commit_result, execution_error> recover_check(const std::filesystem::path& path,
                                                                      std::string_view request_id, std::string_view actor,
                                                                      const hist::request_v3& request, std::uint64_t run_id,
                                                                      std::uint64_t reservation) {
  auto replay = lookup_execution_request(path, request_id, request);
  if (!replay)
    return std::unexpected(replay.error());
  if (replay->state == request_state::replay)
    return execution_commit_result{true, replay->result, 0, {}};
  if (replay->state == request_state::conflict)
    return std::unexpected(failure(execution_error_code::request_conflict));
  if (request.operation != "check-recover" || request.actor != actor)
    return std::unexpected(invalid_error());
  auto database = open_format3(path, execution_access::read_write);
  if (!database)
    return std::unexpected(database.error());
  auto& db = database->sql();
  auto  tx = db.begin(true);
  if (!tx)
    return std::unexpected(map_sqlite(tx.error()));
  auto run = check_run_locked(db, run_id);
  if (!run)
    return std::unexpected(run.error());
  auto row = first_row(db,
                       "SELECT r.request_id,r.run_id,r.opened_seq,r.body_id,s.state,e.revision_id FROM reservations r JOIN "
                       "reservation_state s ON s.reservation_id=r.id JOIN events e ON e.seq=r.opened_seq WHERE r.id=?1",
                       {int_arg(reservation)});
  if (!row)
    return std::unexpected(row.error());
  if (!*row)
    return std::unexpected(failure(execution_error_code::unknown_reservation));
  row_reader q(**row);
  auto       check_request = q.text(0);
  auto       reserved_run = q.id(1), opened = q.id(2), reserved_body = q.id(3);
  auto       state             = q.text(4);
  auto       reserved_revision = q.id(5);
  if (auto ok = q.finish(); !ok)
    return std::unexpected(ok.error());
  if (reserved_run != run_id)
    return std::unexpected(failure(execution_error_code::unknown_reservation));
  if (state != "open")
    return std::unexpected(failure(execution_error_code::state_conflict));
  auto used = first_row(db, "SELECT 1 FROM requests WHERE request_id=?1 OR request_id=?2",
                        {text_arg(request_id), text_arg(check_request)});
  if (!used)
    return std::unexpected(used.error());
  if (*used || request_id == check_request)
    return std::unexpected(failure(execution_error_code::state_conflict));
  auto head = read_head(db);
  if (!head)
    return std::unexpected(head.error());
  if (!head->revision)
    return std::unexpected(corrupt());
  auto seq = next_value(db, "SELECT last_seq FROM current_state WHERE singleton=1");
  if (!seq)
    return std::unexpected(seq.error());
  auto body = check_request_record(db, request);
  if (!body)
    return std::unexpected(body.error());
  auto response = check_response(database->metadata().identity, request_id, *run, reservation, "interrupted", list_node({}),
                                 text_node("none"), text_node("none"), {});
  auto saved    = codec_encode(response);
  if (!saved)
    return std::unexpected(invalid_error());
  if (auto ok = check_event(db, *run, *head->revision, head->job, *seq, "check_interrupted", request_id, actor,
                            {{"reservation", std::to_string(reservation)}});
      !ok)
    return std::unexpected(ok.error());
  if (auto ok = run_bound(db,
                          "INSERT INTO requests(request_id,operation,body_id,event_seq,revision_id,outcome,result,attachment_id) "
                          "VALUES(?1,'check-recover',?2,?3,?4,'interrupted',?5,NULL)",
                          {text_arg(request_id), int_arg(*body), int_arg(*seq), int_arg(*head->revision), blob_arg(*saved)});
      !ok)
    return std::unexpected(ok.error());
  if (auto ok = run_bound(
          db,
          "INSERT INTO requests(request_id,operation,body_id,event_seq,revision_id,outcome,result,attachment_id) "
          "VALUES(?1,'check',?2,?3,?4,'interrupted',?5,NULL)",
          {text_arg(check_request), int_arg(reserved_body), int_arg(opened), int_arg(reserved_revision), blob_arg(*saved)});
      !ok)
    return std::unexpected(ok.error());
  if (auto ok = run_bound(db, "UPDATE reservation_state SET state='interrupted',last_seq=?1 WHERE reservation_id=?2",
                          {int_arg(*seq), int_arg(reservation)});
      !ok)
    return std::unexpected(ok.error());
  if (auto ok = run_bound(db, "UPDATE current_state SET last_seq=?1 WHERE singleton=1", {int_arg(*seq)}); !ok)
    return std::unexpected(ok.error());
  ledger_rows verified_rows;
  auto        verified = verify_rows(db, database->metadata(), verified_rows);
  if (!verified)
    return std::unexpected(invalid_error());
  if (auto ok = tx->commit(); !ok)
    return std::unexpected(map_sqlite(ok.error()));
  return execution_commit_result{false, *saved, *seq, {}};
}

std::expected<std::optional<named_run>, execution_error> lookup_named_run(const std::filesystem::path& path, std::uint64_t run) {
  auto db = open_format3(path, execution_access::read_only);
  if (!db)
    return std::unexpected(db.error());
  auto tx = db->sql().begin(false);
  if (!tx)
    return std::unexpected(map_sqlite(tx.error()));
  auto row = first_row(db->sql(), "SELECT job,role,state FROM run_state WHERE run_id=?1", {int_arg(run)});
  if (!row)
    return std::unexpected(row.error());
  if (!*row)
    return std::optional<named_run>{};
  row_reader r(**row);
  named_run  out{r.text(0), r.text(1), r.text(2)};
  if (auto ok = r.finish(); !ok)
    return std::unexpected(ok.error());
  return std::optional<named_run>{out};
}

namespace {
// Recovery reads deliberately never select runs.secret. Validate the retained closure in their one read transaction.
value_result<execution_snapshot> recovery_snapshot(connection& db, ledger_rows& rows) {
  if (auto ok = load_records(db, rows); !ok)
    return std::unexpected(ok.error());
  if (auto ok = load_events(db, rows); !ok)
    return std::unexpected(ok.error());
  if (auto ok = load_generations(db, rows); !ok)
    return std::unexpected(ok.error());
  if (auto ok = for_each(db, "SELECT id,generation_id,request_id,claimed_seq,provenance_id FROM runs ORDER BY id",
                         [&](row_reader& r) -> checked {
                           run_entry run;
                           run.id          = r.id(0);
                           run.generation  = r.id(1);
                           run.request_id  = r.text(2);
                           run.claimed_seq = r.id(3);
                           run.provenance  = r.optional_id(4);
                           if (!rows.generations.contains(run.generation) || run.claimed_seq > rows.events.size())
                             return std::unexpected(corrupt());
                           rows.runs.emplace(run.id, std::move(run));
                           return {};
                         });
      !ok)
    return std::unexpected(ok.error());
  if (auto ok = load_dispatches(db, rows); !ok)
    return std::unexpected(ok.error());
  if (auto ok = load_role_states(db, rows); !ok)
    return std::unexpected(ok.error());
  if (auto ok = load_run_states(db, rows); !ok)
    return std::unexpected(ok.error());
  for (const auto& [id, d] : rows.dispatches) {
    if (!rows.runs.contains(d.run) || d.seq > rows.events.size())
      return std::unexpected(corrupt());
    if (auto ok = check_dispatch_content(rows, d); !ok)
      return std::unexpected(ok.error());
  }
  execution_snapshot snapshot;
  for (const auto& row : rows.role_states) {
    if (!rows.generations.contains(row.generation))
      return std::unexpected(corrupt());
    execution_role role;
    role.job                = row.job;
    role.role               = row.role;
    role.generation_id      = row.generation;
    role.generation_ordinal = rows.generations.at(row.generation).ordinal;
    role.state              = row.state;
    role.run_id             = row.run;
    if (role.run_id && !rows.runs.contains(*role.run_id))
      return std::unexpected(corrupt());
    snapshot.roles.push_back(std::move(role));
  }
  return snapshot;
}
value_result<retained_dispatch> retained_from(connection& db, const ledger_rows& rows, const dispatch_entry& d) {
  retained_dispatch out;
  out.id                = d.id;
  out.run_id            = d.run;
  out.seq               = d.seq;
  out.revision_id       = d.revision;
  out.kind              = d.kind;
  const auto& gen       = rows.generations.at(rows.runs.at(d.run).generation);
  auto        structure = hist::decode_role_structure_v1(rows.records.at(gen.structure).canonical);
  if (!structure)
    return std::unexpected(corrupt());
  out.role_kind = structure->kind;
  out.work_fp   = hex_of(d.work_fp);
  out.render_fp = hex_of(d.render_fp);
  auto event    = first_row(db, "SELECT request_id FROM events WHERE seq=?1", {int_arg(d.seq)});
  if (!event || !*event)
    return std::unexpected(event ? corrupt() : event.error());
  row_reader er(**event);
  out.request_id = er.text(0);
  if (auto ok = er.finish(); !ok)
    return std::unexpected(ok.error());
  auto state = first_row(db, "SELECT state FROM run_state WHERE run_id=?1", {int_arg(d.run)});
  if (!state || !*state)
    return std::unexpected(state ? corrupt() : state.error());
  row_reader sr(**state);
  out.run_state = sr.text(0);
  if (auto ok = sr.finish(); !ok)
    return std::unexpected(ok.error());
  const auto packet   = record_of(rows, d.packet, 1);
  auto       manifest = value_of(rows, d.manifest, "dispatch_manifest_v1");
  auto       work     = value_of(rows, d.work, "work_exec_v1");
  if (!packet || !manifest || !work)
    return std::unexpected(corrupt());
  auto        record = hist::decode_record((*packet)->encoded);
  const auto* file   = record ? std::get_if<hist::file_record>(&*record) : nullptr;
  auto        m      = hist::decode_dispatch_manifest_v1((*manifest)->canonical);
  auto        w      = hist::decode_work_exec_v1((*work)->canonical);
  if (!file || !m || !w)
    return std::unexpected(corrupt());
  out.markdown = file->payload;
  out.manifest = *m;
  out.work     = *w;
  return out;
}
} // namespace
std::expected<retained_dispatch, execution_error> read_retained_dispatch(const std::filesystem::path& path, std::uint64_t id) {
  auto db = open_format3(path, execution_access::read_only);
  if (!db)
    return std::unexpected(db.error());
  auto tx = db->sql().begin(false);
  if (!tx)
    return std::unexpected(map_sqlite(tx.error()));
  ledger_rows rows;
  auto        snapshot = recovery_snapshot(db->sql(), rows);
  if (!snapshot)
    return std::unexpected(snapshot.error());
  auto found = rows.dispatches.find(id);
  if (found == rows.dispatches.end())
    return std::unexpected(corrupt());
  return retained_from(db->sql(), rows, found->second);
}
std::expected<std::optional<retained_dispatch>, execution_error>
find_dispatch_by_render(const std::filesystem::path& path, std::string_view job, std::string_view role, std::string_view fp) {
  auto db = open_format3(path, execution_access::read_only);
  if (!db)
    return std::unexpected(db.error());
  auto tx = db->sql().begin(false);
  if (!tx)
    return std::unexpected(map_sqlite(tx.error()));
  ledger_rows rows;
  auto        snap = recovery_snapshot(db->sql(), rows);
  if (!snap)
    return std::unexpected(snap.error());
  const dispatch_entry* best = nullptr;
  for (const auto& [id, d] : rows.dispatches) {
    const auto& g = rows.generations.at(rows.runs.at(d.run).generation);
    if (g.job == job && g.role == role && hex_of(d.render_fp) == fp && (!best || d.seq > best->seq))
      best = &d;
  }
  if (!best)
    return std::optional<retained_dispatch>{};
  auto r = retained_from(db->sql(), rows, *best);
  if (!r)
    return std::unexpected(r.error());
  return std::optional<retained_dispatch>{std::move(*r)};
}
std::expected<recovery_records, execution_error> read_recovery_records(const std::filesystem::path& path, std::string_view job,
                                                                       std::string_view role) {
  auto db = open_format3(path, execution_access::read_only);
  if (!db)
    return std::unexpected(db.error());
  auto tx = db->sql().begin(false);
  if (!tx)
    return std::unexpected(map_sqlite(tx.error()));
  ledger_rows rows;
  auto        snap = recovery_snapshot(db->sql(), rows);
  if (!snap)
    return std::unexpected(snap.error());
  recovery_records      out;
  const execution_role* target = nullptr;
  for (const auto& r : snap->roles)
    if (r.job == job && r.role == role)
      target = &r;
  if (!target)
    return out;
  std::uint64_t first = target->generation_ordinal;
  while (first > 1) {
    const generation_entry* g = nullptr;
    for (const auto& [id, x] : rows.generations)
      if (x.job == job && x.role == role && x.ordinal == first)
        g = &x;
    if (!g || g->reason != "takeover")
      break;
    --first;
  }
  std::uint64_t lineage_opened = 0;
  for (const auto& [id, g] : rows.generations)
    if (g.job == job && g.role == role && g.ordinal == first)
      lineage_opened = g.opened_seq;
  std::set<std::uint64_t> runs;
  for (const auto& [id, r] : rows.runs) {
    const auto& g = rows.generations.at(r.generation);
    if (g.job == job && g.role == role && g.ordinal >= first && g.ordinal <= target->generation_ordinal)
      runs.insert(id);
  }
  if (target->run_id) {
    const auto& r = rows.runs.at(*target->run_id);
    out.run       = recovery_run{r.id, target->generation_ordinal, r.request_id, "active"};
  }
  const dispatch_entry* latest = nullptr;
  for (const auto& [id, d] : rows.dispatches)
    if (runs.contains(d.run) && (!latest || d.seq > latest->seq))
      latest = &d;
  if (latest) {
    auto r = retained_from(db->sql(), rows, *latest);
    if (!r)
      return std::unexpected(r.error());
    out.acknowledged = std::move(*r);
  }
  auto ok = for_each(db->sql(), "SELECT id,run_id,seq,body_id FROM checkpoints ORDER BY seq", [&](row_reader& r) -> checked {
    auto id = r.id(0), run = r.id(1), seq = r.id(2), body = r.id(3);
    if (!runs.contains(run))
      return {};
    auto v = value_of(rows, body, "checkpoint_v1");
    if (!v)
      return std::unexpected(corrupt());
    auto note = hist::decode_checkpoint_v1((*v)->canonical);
    if (!note)
      return std::unexpected(corrupt());
    out.checkpoint = retained_checkpoint{id, run, seq, *note};
    return {};
  });
  if (!ok)
    return std::unexpected(ok.error());
  std::map<std::pair<std::string, std::string>, value_ptr> evidence;
  ok = for_each(db->sql(),
                "SELECT e.id,e.reservation_id,e.work,e.\"check\",e.outcome,e.reason,e.matched,e.seq,e.vector_id,r.run_id FROM "
                "evidence e JOIN reservations r ON r.id=e.reservation_id ORDER BY e.seq,e.id",
                [&](row_reader& q) -> checked {
                  auto id = q.id(0), reservation = q.id(1);
                  auto work = q.text(2), check = q.text(3), outcome = q.text(4), reason = q.text(5);
                  auto matched = q.optional_number(6);
                  auto seq = q.id(7), vector = q.id(8), run = q.id(9);
                  if (!runs.contains(run))
                    return {};
                  auto record = value_of(rows, vector, hist::codec_output_vector_v1);
                  if (!record)
                    return std::unexpected(corrupt());
                  auto v = hist::decode_check_value(hist::codec_output_vector_v1, (*record)->canonical);
                  if (!v)
                    return std::unexpected(corrupt());
                  evidence[{work, check}] = record_node({{"id", text_node(std::to_string(id))},
                                                         {"reservation", text_node(std::to_string(reservation))},
                                                         {"work", text_node(work)},
                                                         {"check", text_node(check)},
                                                         {"outcome", text_node(outcome)},
                                                         {"reason", text_node(reason)},
                                                         {"matched", text_node(matched ? std::to_string(*matched) : "none")},
                                                         {"seq", text_node(std::to_string(seq))},
                                                         {"vector", *v}});
                  return {};
                });
  if (!ok)
    return std::unexpected(ok.error());
  for (auto& [key, v] : evidence)
    out.evidence.push_back(std::move(v));
  const auto& gen    = rows.generations.at(target->generation_id);
  auto        stored = value_of(rows, gen.structure, "role_structure_v1");
  if (!stored)
    return std::unexpected(corrupt());
  auto structure = hist::decode_role_structure_v1((*stored)->canonical);
  if (!structure)
    return std::unexpected(corrupt());
  auto reject =
      first_row(db->sql(),
                "SELECT v.id,v.body_id,v.applicability,v.seq FROM verdicts v JOIN runs r ON r.id=v.run_id JOIN generations g "
                "ON g.id=r.generation_id WHERE g.job=?1 AND v.decision='reject' AND v.seq>=(SELECT "
                "COALESCE(MAX(event_seq),0) FROM job_activations WHERE job=?1) ORDER BY v.seq DESC LIMIT 1",
                {text_arg(job)});
  if (!reject)
    return std::unexpected(reject.error());
  if (*reject) {
    row_reader q(**reject);
    auto       id = q.id(0), body = q.id(1);
    auto       applicability = q.text(2);
    auto       verdict_seq   = q.id(3);
    if (auto valid = q.finish(); !valid)
      return std::unexpected(valid.error());
    auto record = value_of(rows, body, hist::codec_verdict_v1);
    if (!record)
      return std::unexpected(corrupt());
    auto verdict = hist::decode_verdict_v1((*record)->canonical);
    if (!verdict)
      return std::unexpected(corrupt());
    const bool selected =
        structure->kind == "reviewer" || (applicability == "current" && verdict_seq >= lineage_opened &&
                                          std::ranges::find(verdict->repair, std::string(role)) != verdict->repair.end());
    if (selected)
      for (const auto& f : verdict->findings) {
        if (structure->kind == "worker" && std::ranges::find(structure->work, f.work) == structure->work.end())
          continue;
        std::map<std::string, value_ptr> fields{{"verdict", text_node(std::to_string(id))},
                                                {"id", text_node(f.id)},
                                                {"severity", text_node(f.severity)},
                                                {"work", text_node(f.work)},
                                                {"text", text_node(f.text)}};
        if (f.path)
          fields["path"] = text_node(*f.path);
        if (f.line)
          fields["line"] = integer_node(*f.line);
        if (f.prescription)
          fields["prescription"] = text_node(*f.prescription);
        out.findings.push_back(record_node(std::move(fields)));
      }
  }
  std::ranges::sort(out.findings, [](const auto& a, const auto& b) {
    return std::pair{a->fields.at("work")->string, a->fields.at("id")->string} <
           std::pair{b->fields.at("work")->string, b->fields.at("id")->string};
  });
  return out;
}

} // namespace planar::engine::ledger
