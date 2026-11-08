// execution_change_v1 closed-shape decoding and native request framing (execution-contract P2-1, EX-2, EX-3, D151).
module planar.engine.execution.change_value;

import std;
import planar.engine.canonical;

namespace planar::engine::execution {

namespace {

using kind_t = ledger::execution_expectation::kind_t;

/// Thrown inside the decoder and caught at the entry point; carries the failing member path.
struct mismatch {
  std::string field;
};

[[noreturn]] void refuse(std::string field) {
  throw mismatch{std::move(field)};
}

value_ptr text_node(std::string_view content) {
  value item{};
  item.kind   = value_kind::string;
  item.string = std::string(content);
  return std::make_shared<const value>(std::move(item));
}

value_ptr record_node(std::map<std::string, value_ptr> fields) {
  value item{};
  item.kind   = value_kind::record;
  item.fields = std::move(fields);
  return std::make_shared<const value>(std::move(item));
}

const value& as_record(const value_ptr& node, const std::string& path) {
  if (!node || node->kind != value_kind::record)
    refuse(path);
  return *node;
}

/// Require exactly the keys of `required` plus any subset of `optional`.
void exact_keys(const value& node, std::initializer_list<std::string_view> required,
                std::initializer_list<std::string_view> optional, const std::string& path) {
  for (const auto& [key, unused] : node.fields) {
    const bool known = std::ranges::find(required, std::string_view(key)) != required.end() ||
                       std::ranges::find(optional, std::string_view(key)) != optional.end();
    if (!known)
      refuse(path + "." + key);
  }
  for (auto key : required)
    if (!node.fields.contains(std::string(key)))
      refuse(path + "." + std::string(key));
}

const value_ptr& member(const value& node, std::string_view key, const std::string& path) {
  const auto found = node.fields.find(std::string(key));
  if (found == node.fields.end() || !found->second)
    refuse(path + "." + std::string(key));
  return found->second;
}

bool has(const value& node, std::string_view key) {
  return node.fields.contains(std::string(key));
}

std::string text(const value& node, std::string_view key, const std::string& path) {
  const auto& item = member(node, key, path);
  if (item->kind != value_kind::string)
    refuse(path + "." + std::string(key));
  return item->string;
}

bool boolean(const value& node, std::string_view key, const std::string& path) {
  const auto& item = member(node, key, path);
  if (item->kind != value_kind::boolean)
    refuse(path + "." + std::string(key));
  return item->boolean;
}

std::optional<std::uint64_t> parse_decimal(std::string_view digits) {
  if (digits.empty() || digits.size() > 19 || (digits.size() > 1 && digits.front() == '0'))
    return std::nullopt;
  std::uint64_t out = 0;
  for (char c : digits) {
    if (c < '0' || c > '9')
      return std::nullopt;
    out = out * 10 + static_cast<std::uint64_t>(c - '0');
  }
  if (out > 9223372036854775807ULL)
    return std::nullopt;
  return out;
}

std::uint64_t decimal(const value& node, std::string_view key, const std::string& path) {
  auto parsed = parse_decimal(text(node, key, path));
  if (!parsed)
    refuse(path + "." + std::string(key));
  return *parsed;
}

std::uint64_t positive(const value& node, std::string_view key, const std::string& path) {
  const auto number = decimal(node, key, path);
  if (number == 0)
    refuse(path + "." + std::string(key));
  return number;
}

/// `"none"` or a positive canonical decimal.
std::optional<std::uint64_t> optional_id(const value& node, std::string_view key, const std::string& path) {
  const auto spelled = text(node, key, path);
  if (spelled == "none")
    return std::nullopt;
  auto parsed = parse_decimal(spelled);
  if (!parsed || *parsed == 0)
    refuse(path + "." + std::string(key));
  return *parsed;
}

/// A list member. Draft decision DD18: an empty Lua table crosses as an empty record, which reads as an empty list.
const std::vector<value_ptr>& items(const value& node, std::string_view key, const std::string& path) {
  static const std::vector<value_ptr> none;
  const auto&                         item = member(node, key, path);
  if (item->kind == value_kind::list)
    return item->items;
  if (item->kind == value_kind::record && item->fields.empty())
    return none;
  refuse(path + "." + std::string(key));
}

std::string at_index(const std::string& path, std::string_view key, std::size_t index) {
  return std::format("{}.{}.{}", path, key, index);
}

history::role_structure_v1 structure_of(const value_ptr& node, const std::string& path) {
  auto bytes = canonical_encode(node);
  if (!bytes)
    refuse(path);
  auto decoded = history::decode_role_structure_v1(*bytes);
  if (!decoded)
    refuse(path);
  return std::move(*decoded);
}

ledger::execution_generation_input generation_of(const value_ptr& node, const std::string& path) {
  const auto& entry = as_record(node, path);
  exact_keys(entry, {"job", "role", "reason", "structure"}, {}, path);
  ledger::execution_generation_input out;
  out.job       = text(entry, "job", path);
  out.role      = text(entry, "role", path);
  out.reason    = text(entry, "reason", path);
  out.structure = structure_of(member(entry, "structure", path), path + ".structure");
  return out;
}

ledger::execution_transition_input transition_of(const value_ptr& node, const std::string& path) {
  const auto& entry = as_record(node, path);
  exact_keys(entry, {"job", "role", "after_state", "after_drifted", "reason"}, {"before_state", "before_drifted"}, path);
  ledger::execution_transition_input out;
  out.job           = text(entry, "job", path);
  out.role          = text(entry, "role", path);
  out.after_state   = text(entry, "after_state", path);
  out.after_drifted = boolean(entry, "after_drifted", path);
  out.reason        = text(entry, "reason", path);
  // NULL-first: the two before keys are both present or both absent.
  if (has(entry, "before_state") != has(entry, "before_drifted"))
    refuse(path + ".before_state");
  if (has(entry, "before_state")) {
    out.before_state   = text(entry, "before_state", path);
    out.before_drifted = boolean(entry, "before_drifted", path);
  }
  return out;
}

ledger::execution_vector_entry vector_of(const value_ptr& node, const std::string& path) {
  const auto& entry = as_record(node, path);
  exact_keys(entry, {"job", "role", "state", "drifted", "run"}, {}, path);
  ledger::execution_vector_entry out;
  out.job     = text(entry, "job", path);
  out.role    = text(entry, "role", path);
  out.state   = text(entry, "state", path);
  out.drifted = boolean(entry, "drifted", path);
  out.run     = optional_id(entry, "run", path);
  return out;
}

ledger::execution_expectation expectation_of(const value_ptr& node, const std::string& path, std::string_view request_id) {
  const auto&                   entry = as_record(node, path);
  const auto                    kind  = text(entry, "kind", path);
  ledger::execution_expectation out;
  if (kind == "request_unused") {
    exact_keys(entry, {"kind"}, {"request_id"}, path);
    out.kind = kind_t::request_unused;
    if (has(entry, "request_id") && text(entry, "request_id", path) != request_id)
      refuse(path + ".request_id");
  } else if (kind == "active_revision") {
    exact_keys(entry, {"kind", "revision"}, {}, path);
    out.kind     = kind_t::active_revision;
    out.revision = optional_id(entry, "revision", path);
  } else if (kind == "active_job") {
    exact_keys(entry, {"kind", "job"}, {}, path);
    out.kind = kind_t::active_job;
    out.text = text(entry, "job", path);
  } else if (kind == "fact_version") {
    exact_keys(entry, {"kind", "job", "role", "fact_version"}, {}, path);
    out.kind   = kind_t::fact_version;
    out.job    = text(entry, "job", path);
    out.role   = text(entry, "role", path);
    out.number = decimal(entry, "fact_version", path);
  } else if (kind == "role_state") {
    out.kind = kind_t::role_state;
    if (has(entry, "no_row")) {
      exact_keys(entry, {"kind", "job", "role", "no_row"}, {}, path);
      if (!boolean(entry, "no_row", path))
        refuse(path + ".no_row");
      out.no_row = true;
    } else {
      exact_keys(entry, {"kind", "job", "role", "state", "drifted", "run"}, {}, path);
      out.state   = text(entry, "state", path);
      out.drifted = boolean(entry, "drifted", path);
      out.run     = optional_id(entry, "run", path);
    }
    out.job  = text(entry, "job", path);
    out.role = text(entry, "role", path);
  } else if (kind == "generation_current") {
    // Draft decision DD19: `generation` is the generation identifier (decimal), as execution_expectation stores it.
    exact_keys(entry, {"kind", "job", "role", "generation"}, {}, path);
    out.kind   = kind_t::generation_current;
    out.job    = text(entry, "job", path);
    out.role   = text(entry, "role", path);
    out.number = positive(entry, "generation", path);
  } else if (kind == "run_active") {
    exact_keys(entry, {"kind", "run"}, {"generation"}, path);
    out.kind = kind_t::run_active;
    out.run  = positive(entry, "run", path);
    if (has(entry, "generation"))
      out.number = positive(entry, "generation", path);
  } else if (kind == "reservation_open") {
    exact_keys(entry, {"kind", "reservation"}, {}, path);
    out.kind   = kind_t::reservation_open;
    out.number = positive(entry, "reservation", path);
  } else {
    // Unknown members remain closed.
    refuse(path + ".kind");
  }
  return out;
}

std::string canonical_bytes(const value_ptr& node, const std::string& path) {
  auto bytes = canonical_encode(node);
  if (!bytes)
    refuse(path);
  return *bytes;
}

const ledger::execution_role* snapshot_row(const ledger::execution_snapshot& snapshot, std::string_view job,
                                           std::string_view role) {
  for (const auto& row : snapshot.roles)
    if (row.job == job && row.role == role)
      return &row;
  return nullptr;
}

/// L20: every role whose first row this change writes asserts its no-row member, and no no-row member names a role
/// that has a row in the snapshot.
void check_no_row_members(const ledger::execution_change& change, const ledger::execution_snapshot& snapshot) {
  std::set<std::pair<std::string, std::string>> asserted;
  for (std::size_t i = 0; i < change.expected.size(); ++i) {
    const auto& member = change.expected[i];
    if (member.kind != kind_t::role_state || !member.no_row)
      continue;
    if (snapshot_row(snapshot, member.job, member.role))
      refuse(std::format("change.expected.{}.no_row", i));
    asserted.emplace(member.job, member.role);
  }
  // EX-3 (draft decision DD29): only roles of the snapshot's active job assert a no-row member; the first rows that an
  // activation writes for another job carry no member, and the ledger refuses a no-row member outside the active job.
  for (std::size_t i = 0; i < change.transitions.size(); ++i) {
    const auto& transition = change.transitions[i];
    if (!transition.before_state && transition.job == snapshot.active_job &&
        !asserted.contains({transition.job, transition.role}))
      refuse(std::format("change.transitions.{}", i));
  }
  for (std::size_t i = 0; i < change.generations.size(); ++i) {
    const auto& generation = change.generations[i];
    if (!snapshot_row(snapshot, generation.job, generation.role) && generation.job == snapshot.active_job &&
        !asserted.contains({generation.job, generation.role}))
      refuse(std::format("change.facts.generations.{}", i));
  }
}

/// Provenance in the facts must be exactly the native document: present together and canonically equal.
void check_provenance(const value& facts, const value_ptr& provenance, const std::string& path) {
  if (has(facts, "provenance") != static_cast<bool>(provenance))
    refuse(path + ".provenance");
  if (provenance && canonical_bytes(member(facts, "provenance", path), path + ".provenance") !=
                        canonical_bytes(provenance, path + ".provenance"))
    refuse(path + ".provenance");
}

ledger::execution_change decode(const value_ptr& root, const ledger::execution_snapshot& snapshot,
                                const change_request& request) {
  const std::string path      = "change";
  const auto&       change    = as_record(root, path);
  const auto        operation = std::string(operation_name(request.options));
  const bool        run_op    = operation == "claim" || operation == "takeover";
  if (run_op || operation == "refresh")
    exact_keys(change, {"version", "operation", "snapshot_seq", "facts", "vector", "transitions", "packet", "expected"}, {},
               path);
  else
    exact_keys(change, {"version", "operation", "snapshot_seq", "facts", "vector", "transitions", "expected"}, {}, path);
  const auto& version = member(change, "version", path);
  if (version->kind != value_kind::integer || version->integer != 1)
    refuse(path + ".version");
  if (text(change, "operation", path) != operation)
    refuse(path + ".operation");
  // L16: the change was built from exactly this snapshot.
  if (decimal(change, "snapshot_seq", path) != snapshot.last_seq)
    refuse(path + ".snapshot_seq");

  ledger::execution_change out;
  out.operation  = operation;
  out.request_id = request.request_id;
  out.actor      = request.actor;
  auto framing   = build_framing(request);
  if (!framing)
    refuse(framing.error().field);
  out.request = std::move(*framing);

  const std::string facts_path = path + ".facts";
  const auto&       facts      = as_record(member(change, "facts", path), facts_path);
  std::visit(
      [&](const auto& options) {
        using T = std::decay_t<decltype(options)>;
        if constexpr (std::is_same_v<T, load_options> || std::is_same_v<T, activate_options>) {
          // Draft decision DD13: load and activate facts are {job,generations,fenced_runs}.
          exact_keys(facts, {"job", "generations", "fenced_runs"}, {}, facts_path);
          out.job                 = text(facts, "job", facts_path);
          const auto& generations = items(facts, "generations", facts_path);
          for (std::size_t i = 0; i < generations.size(); ++i)
            out.generations.push_back(generation_of(generations[i], at_index(facts_path, "generations", i)));
          const auto& fenced = items(facts, "fenced_runs", facts_path);
          for (std::size_t i = 0; i < fenced.size(); ++i) {
            const auto item_path = at_index(facts_path, "fenced_runs", i);
            if (!fenced[i] || fenced[i]->kind != value_kind::string)
              refuse(item_path);
            auto run = parse_decimal(fenced[i]->string);
            if (!run || *run == 0)
              refuse(item_path);
            out.fenced_runs.push_back(*run);
          }
          if constexpr (std::is_same_v<T, load_options>) {
            out.revision = options.revision;
            out.prepared = options.prepared;
          } else {
            // The activated job is the active job after the commit.
            if (out.job != options.job)
              refuse(facts_path + ".job");
          }
        } else if constexpr (std::is_same_v<T, claim_options>) {
          exact_keys(facts, {"role"}, {"provenance"}, facts_path);
          if (text(facts, "role", facts_path) != options.role)
            refuse(facts_path + ".role");
          check_provenance(facts, request.provenance, facts_path);
          out.job  = options.job;
          out.role = options.role;
        } else if constexpr (std::is_same_v<T, takeover_options>) {
          exact_keys(facts, {"role", "previous_run"}, {"provenance"}, facts_path);
          if (text(facts, "role", facts_path) != options.role)
            refuse(facts_path + ".role");
          if (positive(facts, "previous_run", facts_path) != options.run_id)
            refuse(facts_path + ".previous_run");
          check_provenance(facts, request.provenance, facts_path);
          out.job          = options.job;
          out.role         = options.role;
          out.previous_run = options.run_id;
          // Takeover lineage is native: the new generation carries the current structure, and the named run is fenced.
          const auto* row = snapshot_row(snapshot, options.job, options.role);
          if (!row)
            refuse(facts_path + ".role");
          out.generations.push_back(ledger::execution_generation_input{
              .job = options.job, .role = options.role, .reason = "takeover", .structure = row->structure});
          out.fenced_runs.push_back(options.run_id);
        } else {
          auto token = ledger::parse_run_token(options.run_token);
          if (!token)
            refuse("request.run_token");
          if (options.operation == "release")
            exact_keys(facts, {"role", "run", "reason"}, {"checkpoint", "provenance"}, facts_path);
          else if (options.operation == "checkpoint")
            exact_keys(facts, {"role", "run", "checkpoint"}, {}, facts_path);
          else if (options.operation == "refresh")
            exact_keys(facts, {"role", "run"}, {"checkpoint"}, facts_path);
          else
            refuse("request.operation");
          if (text(facts, "role", facts_path) != options.role || positive(facts, "run", facts_path) != token->run_id)
            refuse(facts_path + ".run");
          out.job            = options.job;
          out.role           = options.role;
          out.run            = token->run_id;
          out.run_token_text = options.run_token;
          out.release_reason = options.reason;
          if (options.operation == "release") {
            if (text(facts, "reason", facts_path) != options.reason)
              refuse(facts_path + ".reason");
            check_provenance(facts, request.provenance, facts_path);
            out.provenance = request.provenance;
          }
          if (has(facts, "checkpoint")) {
            out.checkpoint = member(facts, "checkpoint", facts_path);
            if (!history::encode_checkpoint_v1(out.checkpoint))
              refuse(facts_path + ".checkpoint");
          }
        }
      },
      request.options);
  if (run_op)
    out.provenance = request.provenance;

  const auto& transitions = items(change, "transitions", path);
  for (std::size_t i = 0; i < transitions.size(); ++i)
    out.transitions.push_back(transition_of(transitions[i], at_index(path, "transitions", i)));
  const auto& vector = items(change, "vector", path);
  for (std::size_t i = 0; i < vector.size(); ++i)
    out.vector.push_back(vector_of(vector[i], at_index(path, "vector", i)));
  const auto& expected = items(change, "expected", path);
  for (std::size_t i = 0; i < expected.size(); ++i)
    out.expected.push_back(expectation_of(expected[i], at_index(path, "expected", i), request.request_id));

  if (run_op || operation == "refresh") {
    const std::string packet_path = path + ".packet";
    const auto&       packet      = as_record(member(change, "packet", path), packet_path);
    exact_keys(packet, {"markdown", "manifest", "work"}, {}, packet_path);
    ledger::execution_packet decoded;
    decoded.markdown = text(packet, "markdown", packet_path);
    decoded.manifest = member(packet, "manifest", packet_path);
    decoded.work     = member(packet, "work", packet_path);
    as_record(decoded.manifest, packet_path + ".manifest");
    as_record(decoded.work, packet_path + ".work");
    out.packet = std::move(decoded);
  } else {
    check_no_row_members(out, snapshot);
  }
  return out;
}

} // namespace

std::string_view operation_name(const operation_options& options) {
  return std::visit(
      [](const auto& typed) -> std::string_view {
        using T = std::decay_t<decltype(typed)>;
        if constexpr (std::is_same_v<T, load_options>)
          return "load";
        else if constexpr (std::is_same_v<T, activate_options>)
          return "activate";
        else if constexpr (std::is_same_v<T, claim_options>)
          return "claim";
        else if constexpr (std::is_same_v<T, takeover_options>)
          return "takeover";
        else
          return typed.operation;
      },
      options);
}

std::expected<history::request_v3, change_error> build_request_v3(std::string_view actor, const operation_options& options,
                                                                  const std::optional<std::string>& profile_argument) {
  std::map<std::string, value_ptr> parameters;
  bool                             valid = true;
  std::visit(
      [&](const auto& typed) {
        using T = std::decay_t<decltype(typed)>;
        if constexpr (std::is_same_v<T, load_options>) {
          valid = false;
        } else if constexpr (std::is_same_v<T, activate_options>) {
          parameters["expect_job"]      = text_node(typed.expect_job);
          parameters["expect_revision"] = text_node(typed.expect_revision);
          parameters["job"]             = text_node(typed.job);
        } else if constexpr (std::is_same_v<T, recovery_options>) {
          auto token = ledger::parse_run_token(typed.run_token);
          if (!token) {
            valid = false;
            return;
          }
          if (typed.operation != "refresh" && typed.operation != "checkpoint" && typed.operation != "release") {
            valid = false;
            return;
          }
          parameters["run_id"] = text_node(std::to_string(token->run_id));
          if (typed.operation == "refresh")
            parameters["expect_fp"] = text_node(typed.expect_fp);
          if (typed.operation == "release")
            parameters["reason"] = text_node(typed.reason);
          if (typed.observations_path)
            parameters["observations_path"] = text_node(*typed.observations_path);
          if (typed.provenance_path)
            parameters["provenance_path"] = text_node(*typed.provenance_path);
        } else {
          parameters["job"]  = text_node(typed.job);
          parameters["role"] = text_node(typed.role);
          if constexpr (std::is_same_v<T, takeover_options>)
            parameters["run_id"] = text_node(std::to_string(typed.run_id));
          if (typed.provenance_path)
            parameters["provenance_path"] = text_node(*typed.provenance_path);
        }
      },
      options);
  if (!valid)
    return std::unexpected(change_error{"request.operation"});
  auto bytes = canonical_encode(record_node(std::move(parameters)));
  if (!bytes)
    return std::unexpected(change_error{"request.parameters"});
  return history::request_v3{.operation        = std::string(operation_name(options)),
                             .actor            = std::string(actor),
                             .parameters       = std::move(*bytes),
                             .profile_argument = profile_argument};
}

std::expected<ledger::execution_framing, change_error> build_framing(const change_request& request) {
  if (const auto* load = std::get_if<load_options>(&request.options)) {
    if (load->request.operation != "load" || load->request.actor != request.actor)
      return std::unexpected(change_error{"request.framing"});
    return ledger::execution_framing{load->request};
  }
  auto framed = build_request_v3(request.actor, request.options, request.profile_argument);
  if (!framed)
    return std::unexpected(framed.error());
  return ledger::execution_framing{std::move(*framed)};
}

std::expected<ledger::execution_change, change_error>
decode_execution_change(const value_ptr& change, const ledger::execution_snapshot& snapshot, const change_request& request) {
  try {
    return decode(change, snapshot, request);
  } catch (const mismatch& failure) {
    return std::unexpected(change_error{failure.field});
  }
}

} // namespace planar::engine::execution
