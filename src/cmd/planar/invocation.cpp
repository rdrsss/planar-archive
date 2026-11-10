module planar.cmd.invocation;
import std;
import planar.engine.capture;
import planar.engine.acceptance.load;
import planar.engine.capture.host_path;
import planar.engine.ledger.format;
import planar.engine.ledger.acceptance;
import planar.engine.history.read;
import planar.engine.source_export;
import planar.engine.runtime; // X07-REAL-RUNTIME
import planar.engine.canonical;
import planar.engine.ledger.execution;
import planar.engine.execution.snapshot_value;
import planar.engine.execution.change_value;
import planar.engine.history.provenance;
import planar.engine.history.tree;
import planar.engine.acceptance.source_capture;
import planar.engine.execution.drafts;
import planar.engine.execution.recovery_capture;
import planar.engine.execution.check;
import planar.engine.execution.verdict;
import planar.engine.execution.provenance_input;
import planar.engine.ledger.sqlite;
#ifdef PLANAR_AUTHORING_BUNDLE
import planar.engine.capture.directory;
import planar.engine.hash;
#endif
namespace planar::cmd {
namespace {
using namespace detail;
using namespace engine;
std::expected<value_ptr, ledger::execution_error> assemble_run_result(const std::string& path, const value_ptr& saved) {
  auto dispatch = ledger::read_retained_dispatch(path, std::stoull(saved->fields.at("dispatch")->fields.at("id")->string));
  if (!dispatch)
    return std::unexpected(dispatch.error());
  const auto&            w = dispatch->work;
  std::vector<value_ptr> units;
  for (const auto& unit : w->fields.at("units")->items)
    units.push_back(unit->fields.at("id"));
  auto packet        = object({{"version", integer(1)},
                               {"mode", text("dispatch")},
                               {"workspace", w->fields.at("workspace")},
                               {"spec", w->fields.at("spec")},
                               {"job", w->fields.at("job")},
                               {"role", w->fields.at("role")},
                               {"kind", text(dispatch->role_kind)},
                               {"work", array(std::move(units))},
                               {"markdown", text(dispatch->markdown)}});
  auto fields        = saved->fields;
  fields["packet"]   = packet;
  fields["manifest"] = dispatch->manifest;
  return object(std::move(fields));
}
struct refusal {
  value_ptr error;
};
template <class T> T required(std::expected<T, diagnostic> result) {
  if (!result)
    throw result.error();
  return std::move(*result);
}
void required(std::expected<void, diagnostic> result) {
  if (!result)
    throw result.error();
}
value_ptr uncertainty_error(std::string operation) {
  using namespace detail;
  return object({{"code", text("io")},
                 {"message", text("Export may be complete; inspect the explicit target before retrying")},
                 {"field", text("")},
                 {"details", object({{"operation", text(std::move(operation))}})}});
}
value_ptr execution_uncertainty_error(std::string verb, std::string operation) {
  auto title = verb;
  if (!title.empty() && title[0] >= 'a' && title[0] <= 'z')
    title[0] = static_cast<char>(title[0] - 'a' + 'A');
  return object({{"code", text("io")},
                 {"message", text(title + " may be complete; retry the same request ID or inspect request history")},
                 {"field", text("")},
                 {"details", object({{"operation", text(std::move(operation))}})}});
}
std::string execution_mutation_name(const engine::command_options& options) {
  if (const auto* run = std::get_if<engine::command_run_options>(&options))
    return run->run_id ? "takeover" : "claim";
  if (const auto* recovery = std::get_if<engine::command_recovery_options>(&options))
    return recovery->operation;
  if (const auto* check = std::get_if<engine::command_check_options>(&options))
    return check->operation;
  return {};
}
value_ptr load_uncertainty_error(std::string operation) {
  return object({{"code", text("io")},
                 {"message", text("Load may be complete; retry the same request ID or inspect request history")},
                 {"field", text("")},
                 {"details", object({{"operation", text(std::move(operation))}})}});
}
value_ptr load_error(const acceptance::load_error& error) {
  if (!error.detail.code.empty())
    return native_error(error.detail);
  if (error.code == acceptance::load_error_code::invalid_path)
    return native_error({"path_denied", "Cannot resolve explicit input path"});
  if (error.code == acceptance::load_error_code::resource_limit)
    return native_error({"resource_limit", "Load exceeds native resource limits"});
  return native_error({"internal", "Invalid native load preparation"});
}
value_ptr activate_uncertainty_error(std::string operation) {
  return object({{"code", text("io")},
                 {"message", text("Activate may be complete; retry the same request ID or inspect request history")},
                 {"field", text("")},
                 {"details", object({{"operation", text(std::move(operation))}})}});
}
/// Closed public mapping of the format3 execution failure categories (execution-contract EX-7 and Messages): reused codes
/// keep their existing messages byte for byte; the E16 codes use their fixed messages.
value_ptr execution_error_value(const ledger::execution_error& error) {
  using enum ledger::execution_error_code;
  auto plain = [](std::string code, std::string message) {
    return object({{"code", text(std::move(code))}, {"message", text(std::move(message))}, {"field", text("")}});
  };
  auto with = [](std::string code, std::string message, value_ptr details) {
    return object({{"code", text(std::move(code))},
                   {"message", text(std::move(message))},
                   {"field", text("")},
                   {"details", std::move(details)}});
  };
  auto ledger_io = [&](std::string message) { return with("io", std::move(message), object({{"operation", text("ledger")}})); };
  switch (error.code) {
  case unsupported_ledger_version:
    return plain("unsupported_ledger_version", "Ledger version or state is unsupported");
  case foreign_ledger:
    return plain("foreign_ledger", "File is not a Planar ledger");
  case ledger_not_found:
    return plain("ledger_not_found", "Ledger was not found");
  case ledger_exists:
    return plain("ledger_exists", "Ledger target already exists");
  case ledger_corrupt:
    return plain("ledger_corrupt", "Ledger integrity verification failed");
  case ledger_busy:
    return plain("ledger_busy", "Ledger is busy");
  case storage_full:
    return plain("storage_full", "Ledger storage is full");
  case io:
    return ledger_io("Ledger operation failed");
  case path_denied:
    return plain("path_denied", "Ledger path requires existing real parent directories and a valid target");
  case recovery_required:
    return ledger_io("Explicit ledger recovery is required");
  case request_conflict:
    return plain("request_conflict", "Ledger request conflicts with its saved intent");
  case revision_conflict:
    return with("revision_conflict", "Ledger revision does not match the expected revision",
                object({{"expected", text(error.expected)}, {"actual", text(error.actual)}}));
  case state_conflict:
    return plain("state_conflict", "Execution state changed since the request was prepared");
  case claim_conflict:
    return with("claim_conflict", "Role state does not allow this operation", object({{"state", text(error.state)}}));
  case check_in_progress:
    return with("check_in_progress", "A check is already in progress for this role", object({}));
  case fenced:
    return with("fenced", "Run is not current", object({{"run_state", text(error.state)}}));
  case resource_limit:
    return with("resource_limit", "Execution resource limit exceeded",
                object({{"limit", text(error.limit)}, {"maximum", integer(static_cast<std::int64_t>(error.maximum))}}));
  case hash_collision: {
    std::map<std::string, value_ptr> details{{"kind", text(error.record_kind)}};
    if (!error.record_digest.empty())
      details["digest"] = text(error.record_digest);
    return with("hash_collision", "Ledger record identity collision", object(std::move(details)));
  }
  case drifted:
    return plain("drifted", "Role work changed since it was acknowledged");
  case unknown_reservation:
    return object({{"code", text("reference")}, {"message", text("Unknown check reservation")}, {"field", text("reservation")}});
  case unknown_run:
    return plain("state_conflict", "Execution state changed since the request was prepared");
  case invalid_change:
    break;
  }
  return plain("internal", "Invalid embedded command protocol");
}
/// Map a format3 initialization or open failure to the existing ledger refusal, so init and the format3 read variant keep
/// the closed `ledger_error` bytes of format1 and format2 (EX-1, Q13 A: the init response and diagnostics are unchanged).
ledger::types::format_error init_format_error(const ledger::execution_error& error) {
  using enum ledger::execution_error_code;
  using code = ledger::types::format_error_code;
  switch (error.code) {
  case unsupported_ledger_version:
    return {code::unsupported_version, ""};
  case foreign_ledger:
    return {code::foreign_ledger, ""};
  case ledger_not_found:
    return {code::not_found, ""};
  case ledger_exists:
    return {code::exists, ""};
  case ledger_corrupt:
    return {code::corrupt, ""};
  case ledger_busy:
    return {code::busy, ""};
  case storage_full:
    return {code::storage_full, ""};
  case path_denied:
    return {code::unsafe_path, ""};
  case recovery_required:
    return {code::recovery_required, ""};
  case invalid_change:
    return {code::invalid_argument, ""};
  default:
    return {code::io, ""};
  }
}
/// Complete format3 verification before a format3 read through the storage read variant (R15-C DD35): the storage
/// reader checks only the format3 shape, the execution module verifies every format3 invariant.
/// @return Null when the ledger is not format3 or verifies, otherwise the public refusal.
value_ptr verify_format3_read(const std::filesystem::path& path, const ledger::ledger_metadata& metadata) {
  if (metadata.format != ledger::ledger_format::v3)
    return nullptr;
  auto verified = ledger::verify_execution_ledger(path);
  if (!verified)
    return ledger_error(init_format_error(verified.error()));
  return nullptr;
}
ledger::execution_error execution_failure(ledger::execution_error_code code) {
  ledger::execution_error error{};
  error.code = code;
  return error;
}
/// The fixed EX-7 code messages that native execution sites of activate raise before the commit.
value_ptr execution_code(std::string_view code) {
  std::string message = code == "history_unavailable"       ? "History needed for this request is unavailable"
                        : code == "check_in_progress"       ? "A check is already in progress for this role"
                        : code == "drifted"                 ? "Role work changed since it was acknowledged"
                        : code == "no_active_job"           ? "No active job matches the request"
                        : code == "interpretation_mismatch" ? "Interpretation does not match the active revision"
                                                            : "Execution state changed since the request was prepared";
  return object({{"code", text(std::string(code))}, {"message", text(std::move(message))}, {"field", text("")}});
}
/// Preserve execution-owned diagnostic fields without changing the generic error boundary.
value_ptr check_error_value(const execution::check_error& error, std::uint64_t revision) {
  auto fields     = native_error(error)->fields;
  fields["field"] = text(error.field);
  if (error.contract_delivery)
    fields["details"] = object({{"expected", text(std::to_string(revision))}, {"actual", text("working_bytes")}});
  else if (!error.work.empty())
    fields["details"] = object({{"work", text(error.work)}});
  return object(std::move(fields));
}
/// Preserve the native parser/normalizer's verdict field path.
value_ptr verdict_error_value(const execution::verdict_error& error) {
  auto fields     = native_error({error.code, error.message, error.limit, error.maximum})->fields;
  fields["field"] = text(error.field);
  return object(std::move(fields));
}
std::string digest_hex(const history::record_digest& digest) {
  std::string out;
  for (auto byte : digest)
    out += std::format("{:02x}", byte);
  return out;
}
/// Portable root digest of a complete tree, as the format3 ledger records it, without storage.
std::optional<std::string> tree_digest(const std::vector<history::tree_file>& files, history::tree_profile profile) {
  std::vector<history::tree_file_view> views;
  views.reserve(files.size());
  for (const auto& file : files)
    views.push_back(history::tree_file_view{file.path, file.payload, file.mode});
  auto digest = history::digest_tree(views, profile);
  if (!digest)
    return std::nullopt;
  return digest_hex(*digest);
}
std::optional<std::string> canonical_text(const value_ptr& node) {
  if (!node)
    return std::nullopt;
  auto bytes = canonical_encode(node);
  if (!bytes)
    return std::nullopt;
  return std::move(*bytes);
}
/// Map a declaration source-capture refusal onto the public error, as the load family maps its capture refusals.
value_ptr source_failure(const acceptance::source_error& error) {
  if (!error.detail.code.empty())
    return native_error(error.detail);
  if (error.code == acceptance::source_error_code::invalid_path)
    return native_error({"path_denied", "Cannot resolve explicit input path"});
  if (error.code == acceptance::source_error_code::resource_limit)
    return native_error({"resource_limit", "Load exceeds native resource limits"});
  return native_error({"internal", "Invalid native load preparation"});
}
/// The declaration-content classes that make a draft capture `invalid` rather than failing status (P2-1 results list).
bool draft_content_class(std::string_view code) {
  return code == "lua_syntax" || code == "capability_denied" || code == "schema" || code == "reference" || code == "ownership" ||
         code == "dependency_cycle" || code == "path_denied" || code == "unsupported_version";
}
/// `{code,message,field}` of a public error value, without details, as `drafts.error` carries it.
value_ptr typed_error(const value_ptr& error) {
  auto field = [&](const char* key) { return error->fields.contains(key) ? error->fields.at(key) : text(""); };
  return object({{"code", field("code")}, {"message", field("message")}, {"field", field("field")}});
}
std::optional<std::uint32_t> captured_mode(std::string_view spelling) {
  if (spelling == "0644")
    return 420;
  if (spelling == "0755")
    return 493;
  return std::nullopt;
}
/// Copy a record value with one member replaced.
value_ptr with_member(const value_ptr& node, const std::string& key, value_ptr member) {
  auto copy         = std::make_shared<value>(*node);
  copy->fields[key] = std::move(member);
  return copy;
}
/// Read one TEXT or BLOB column as bytes.
std::optional<std::string> column_bytes(const ledger::statement& row, int column) {
  if (auto blob = row.blob(column); blob && *blob)
    return **blob;
  if (auto plain = row.text(column); plain && *plain)
    return **plain;
  return std::nullopt;
}
/// Last 16 transitions of one role of a job, oldest first, as `{event,ordinal,reason}` (draft decision DD-D4).
/// @note A separate read transaction from the snapshot's (recorded draft limit).
std::expected<value_ptr, ledger::execution_error> role_reasons(const std::filesystem::path& path, const std::string& job,
                                                               const std::string& role) {
  auto opened = ledger::open_format3(path, ledger::execution_access::read_only);
  if (!opened)
    return std::unexpected(opened.error());
  ledger::execution_error io_error{};
  io_error.code = ledger::execution_error_code::io;
  auto query    = opened->sql().prepare("SELECT event_seq,ordinal,reason FROM transitions WHERE job=?1 AND role=?2 "
                                        "ORDER BY event_seq DESC, ordinal DESC LIMIT 16");
  if (!query || !query->bind_text(1, job) || !query->bind_text(2, role))
    return std::unexpected(io_error);
  std::vector<value_ptr> items;
  while (true) {
    auto step = query->step();
    if (!step)
      return std::unexpected(io_error);
    if (*step == ledger::step_result::done)
      break;
    auto event = query->integer(0), ordinal = query->integer(1);
    auto reason = query->text(2);
    if (!event || !*event || !ordinal || !*ordinal || !reason || !*reason)
      return std::unexpected(io_error);
    items.push_back(object(
        {{"event", text(std::to_string(**event))}, {"ordinal", text(std::to_string(**ordinal))}, {"reason", text(**reason)}}));
  }
  std::ranges::reverse(items);
  return array(std::move(items));
}
bool stage_token(std::string_view s) {
  constexpr std::string_view prefix = ".planar-export-";
  return s.starts_with(prefix) && s.size() == prefix.size() + 32 &&
         std::ranges::all_of(s.substr(prefix.size()), [](char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); });
}
value_ptr export_error(const engine::source_export::export_error& e) {
  using namespace detail;
  using enum engine::source_export::error_code;
  if (e.code == storage)
    return ledger_error(
        e.storage.value_or(engine::ledger::types::format_error{engine::ledger::types::format_error_code::io, ""}));
  if (e.code == publication_uncertain)
    return uncertainty_error("export");
  std::string code, message;
  switch (e.code) {
  case revision_not_found:
    code    = "revision_not_found";
    message = "Recorded revision was not found";
    break;
  case invalid_argument:
    code    = "schema";
    message = "Invalid export arguments";
    break;
  case path_denied:
    code    = "path_denied";
    message = "Export path requires existing real parent directories and a valid target";
    break;
  case target_exists:
    code    = "target_exists";
    message = "Export target already exists";
    break;
  case cleanup_incomplete:
    code = "export_incomplete";
    message =
        "Export staging cleanup is incomplete; inspect the sibling location in the explicit target parent before taking action";
    break;
  default:
    code    = "io";
    message = "Export operation failed";
    break;
  }
  auto fields = std::map<std::string, engine::value_ptr>{{"code", text(code)}, {"message", text(message)}, {"field", text("")}};
  if (e.code == cleanup_incomplete) {
    if (!stage_token(e.location))
      return native_error({"internal", "Invalid export cleanup location"});
    fields["details"] = object({{"operation", text("export")}, {"cleanup", text("incomplete")}, {"location", text(e.location)}});
  } else if (code == "io")
    fields["details"] = object({{"operation", text("export")}});
  return object(std::move(fields));
}

std::expected<std::string, diagnostic> process_cwd() {
  try {
    return std::filesystem::current_path().lexically_normal().string();
  } catch (const std::filesystem::filesystem_error&) {
    return std::unexpected(diagnostic{"io", "Cannot obtain invocation cwd"});
  }
}
std::optional<std::string_view> process_workspace_environment() {
  const auto* value = std::getenv("PLANAR_WORKSPACE_DIR");
  return value ? std::optional<std::string_view>(value) : std::nullopt;
}
std::expected<std::string, diagnostic> process_workspace_directory() {
  return snapshot_workspace_directory(process_workspace_environment());
}
using snapshot_reader = std::function<std::expected<std::string, diagnostic>()>;
#ifdef PLANAR_AUTHORING_BUNDLE
/// Maps both digest allocation failures (Hash and Canonical allocation failed) of an observation entry to the observation
/// total bound (D-C01-32); other errors pass.
diagnostic observation_digest_failure(diagnostic error) {
  if (error.code == "resource_limit" &&
      (error.message == "Hash allocation failed" || error.message == "Canonical allocation failed"))
    return capture_limit("observation_total_bytes", 4194304);
  return error;
}
#endif

class native_services final : public command_services {
  capture_session                                  _session;
  std::string                                      _workspace_path;
  value_ptr                                        _workspace;
  template <class Function> command_service_result capture_boundary(Function&& function) {
    try {
      return function();
    } catch (const refusal& e) {
      return {{}, e.error};
    } catch (const diagnostic& e) {
      return {{}, native_error(e)};
    } catch (const std::filesystem::filesystem_error&) {
      return {{}, native_error({"io", "Cannot resolve explicit input path"})};
    }
  }

public:
  bool                                      export_published = false;
  std::optional<acceptance::request_intent> load_intent;
  std::string                               load_ledger_identity;
  std::optional<acceptance::load_capture>   load_capture;
  // Borrow the stable VM receipt before entering completion. It remains authoritative
  // even if constructing the first service value throws after COMMIT.
  ledger::completion_receipt* load_receipt = nullptr;
  command_service_result      open_workspace(const command_load_options& options) override {
    auto opened = acceptance::load_capture::open_workspace(options.workspace, options.registry);
    if (!opened)
      return {{}, load_error(opened.error())};
    load_capture.emplace(std::move(*opened));
    auto value = load_capture->workspace();
    if (!value)
      return {{}, load_error(value.error())};
    return {*value, {}};
  }
  command_service_result capture_load(const command_load_options&, const value_ptr& route) override {
    if (!load_capture)
      return {{}, native_error({"internal", "Missing native load capture"})};
    auto captured = load_capture->capture(route);
    if (!captured)
      return {{}, load_error(captured.error())};
    return {object({{"bundle", *captured}}), {}};
  }
  command_service_result accept_load(const command_load_options& options, const value_ptr& bundle, const value_ptr& semantic,
                                     std::span<const history::tree_file> provenance,
                                     ledger::completion_receipt&         completion) override {
    load_receipt = &completion;
    if (!load_capture || !load_intent)
      return {{}, native_error({"internal", "Missing native load intent"})};
    auto prepared = load_capture->prepare(bundle, semantic, provenance);
    if (!prepared)
      return {{}, load_error(prepared.error())};
    auto accepted = ledger::complete_acceptance(options.ledger, *load_intent, *prepared, load_ledger_identity, completion);
    if (!accepted)
      return {{}, ledger_error(accepted.error())};
    auto decoded = codec_decode(completion.saved_result);
    if (!decoded)
      return {{}, native_error({"internal", "Invalid saved load response"})};
    return {*decoded, {}};
  }
  // Format3 load (R15-C P3, DD1): set by the load prelude when the ledger is format3; the snapshot is the one
  // read_execution returned, and every later step of the invocation uses exactly that snapshot.
  bool        load_format3 = false;
  std::string execution_workspace;
  // R17/R18 invocation-owned native facts. Lua selects policy; these objects own the observable execution.
  std::optional<history::request_v3>            check_request;
  std::string                                   check_identity, check_role, check_saved;
  std::optional<execution::prepared_check>      check_prepared;
  std::optional<ledger::execution_reservation>  check_reservation;
  std::optional<ledger::execution_check_result> check_result;
  value_ptr                                     check_provenance, check_document, check_result_value;
  std::optional<std::string>                    execution_profile;
  std::optional<std::string>                    execution_stdlib;
  void                                          bind_execution_stdlib(std::string_view digest) override {
    execution_stdlib = std::string(digest);
  }
  std::optional<std::string> execution_binding() const {
    return execution_stdlib;
  }
  bool                   check_committed = false;
  command_service_result check_step(const engine::command_check_options& options, std::string_view service,
                                    const value_ptr& arguments) override {
    return capture_boundary([&]() -> command_service_result {
      auto field = [](const value_ptr& value, std::string_view name) -> value_ptr {
        if (!value || value->kind != value_kind::record)
          return {};
        auto i = value->fields.find(std::string(name));
        return i == value->fields.end() ? value_ptr{} : i->second;
      };
      auto scalar = [&](const value_ptr& value, std::string_view name) -> std::string {
        auto f = field(value, name);
        return f && f->kind == value_kind::string ? f->string : std::string{};
      };
      auto same = [](const value_ptr& a, const value_ptr& b) {
        auto x = canonical_encode(a), y = canonical_encode(b);
        return x && y && *x == *y;
      };
      auto internal = [&]() -> command_service_result {
        return {{}, native_error({"internal", "Invalid embedded command protocol"})};
      };
      auto saved = [&](const ledger::execution_commit_result& committed) -> command_service_result {
        check_committed = true;
        check_saved     = committed.saved_result;
        auto parsed     = codec_decode(check_saved);
        return parsed ? command_service_result{*parsed, {}} : internal();
      };
      auto token = ledger::parse_run_token(options.run);
      if (service == "read_execution") {
        auto snapshot = ledger::read_execution_snapshot(options.ledger);
        if (!snapshot)
          return {{}, execution_error_value(snapshot.error())};
        if (snapshot->identity != check_identity)
          return {{}, execution_error_value(execution_failure(ledger::execution_error_code::path_denied))};
        if (options.operation != "check-recover") {
          if (!token)
            return {{}, native_error({"usage", "Invalid command syntax"})};
          if (snapshot->revision) {
            auto digest = execution_binding();
            if (!digest)
              return internal();
            if (*digest != snapshot->revision->stdlib_digest)
              return {{}, execution_code("interpretation_mismatch")};
          }
          if (snapshot->active_job.empty())
            return {{}, execution_code("no_active_job")};
          auto authorized = ledger::authorize_run_token(options.ledger, options.run);
          if (!authorized)
            return {{}, execution_error_value(authorized.error())};
          const ledger::execution_role* target = nullptr;
          for (const auto& r : snapshot->roles)
            if (r.job == snapshot->active_job && r.run_id == token->run_id)
              target = &r;
          if (!target)
            return {{}, execution_error_value(execution_failure(ledger::execution_error_code::state_conflict))};
          if ((options.operation == "verdict") != (target->structure.kind == "reviewer"))
            return {{},
                    object({{"code", text("reference")},
                            {"message", text("Unknown selected role")},
                            {"field", text("selection.role")}})};
          if (target->drifted && (!check_result || options.operation == "verdict"))
            return {{}, execution_code("drifted")};
          check_role = target->role;
          if (!check_result && options.operation == "check")
            for (const auto& r : snapshot->reservations)
              if (r.run == token->run_id)
                return {{}, execution_code("check_in_progress")};
        }
        auto value = execution::snapshot_value(*snapshot);
        if (!value)
          return internal();
        auto fields           = (*value)->fields;
        fields["target_role"] = text(check_role);
        fields["run_id"]      = text(token ? std::to_string(token->run_id) : options.run_id);
        execution_state       = std::move(*snapshot);
        return {object(std::move(fields)), {}};
      }
      if (!execution_state || !check_request)
        return internal();
      auto expectations = [&](bool completion, bool reserving) -> value_ptr {
        if (!token || !execution_state->revision)
          return {};
        const ledger::execution_role* role = nullptr;
        for (const auto& r : execution_state->roles)
          if (r.job == execution_state->active_job && r.role == check_role)
            role = &r;
        if (!role)
          return {};
        auto no                    = std::make_shared<value>();
        no->kind                   = value_kind::boolean;
        no->boolean                = false;
        const auto             job = text(role->job), who = text(role->role), run = text(std::to_string(token->run_id)),
                               generation = text(std::to_string(role->generation_id));
        std::vector<value_ptr> values{object({{"kind", text("request_unused")}, {"request_id", text(options.request_id)}})};
        if (completion || reserving) {
          values.push_back(
              object({{"kind", text("active_revision")}, {"revision", text(std::to_string(execution_state->revision->id))}}));
          values.push_back(object({{"kind", text("active_job")}, {"job", job}}));
          values.push_back(object({{"kind", text("role_state")},
                                   {"job", job},
                                   {"role", who},
                                   {"state", text("running")},
                                   {"drifted", no},
                                   {"run", run}}));
          values.push_back(
              object({{"kind", text("generation_current")}, {"job", job}, {"role", who}, {"generation", generation}}));
        }
        values.push_back(
            object({{"kind", text("run_active")}, {"job", job}, {"role", who}, {"run", run}, {"generation", generation}}));
        if (!reserving) {
          if (!check_reservation)
            return {};
          values.push_back(
              object({{"kind", text("reservation_open")}, {"reservation", text(std::to_string(check_reservation->id))}}));
          if (completion)
            for (const auto& r : execution_state->roles)
              if (r.job == role->job)
                values.push_back(object({{"kind", text("fact_version")},
                                         {"job", text(r.job)},
                                         {"role", text(r.role)},
                                         {"fact_version", text(std::to_string(r.fact_version))}}));
        }
        return array(std::move(values));
      };

      if (service == "read_provenance") {
        if (!options.provenance)
          return internal();
        auto value = execution::read_provenance(*options.provenance);
        if (!value)
          return {{}, native_error({value.error().code, value.error().message, value.error().limit, value.error().maximum})};
        check_provenance = *value;
        return {*value, {}};
      }
      if (service == "prepare_check") {
        auto plan = field(arguments, "plan");
        if (!plan || !token || !execution_state->revision)
          return internal();
        const ledger::execution_role* target = nullptr;
        for (const auto& r : execution_state->roles)
          if (r.job == execution_state->active_job && r.role == check_role)
            target = &r;
        if (!target || scalar(plan, "job") != target->job || scalar(plan, "role") != target->role ||
            scalar(plan, "run") != std::to_string(token->run_id) ||
            scalar(plan, "generation") != std::to_string(target->generation_id))
          return internal();
        auto interpretation = execution_state->revision->interpretation_value;
        auto bundle = field(interpretation, "bundle"), registry = field(bundle, "registry"),
             workspace = field(bundle, "workspace"), spec = field(bundle, "spec");
        if (!bundle || !registry || !workspace || !spec)
          return internal();
        auto root = field(workspace, "root");
        if (scalar(plan, "root") != scalar(root, "id") || scalar(plan, "base") != scalar(root, "base") ||
            !same(field(plan, "excluded"), field(root, "excluded")) ||
            !same(field(plan, "frozen"), (field(spec, "frozen") ? field(spec, "frozen") : array())))
          return internal();
        std::vector<value_ptr> units, owned;
        for (const auto& u : target->structure.ownership) {
          std::vector<value_ptr> paths;
          for (const auto& p : u.paths) {
            paths.push_back(text(p));
            owned.push_back(text(p));
          }
          units.push_back(object({{"work", text(u.work)}, {"paths", array(std::move(paths))}}));
        }
        std::ranges::sort(owned, [](const auto& a, const auto& b) { return a->string < b->string; });
        if (!same(field(plan, "units"), array(std::move(units))) || !same(field(plan, "owned"), array(std::move(owned))))
          return internal();
        std::set<std::pair<std::string, std::string>> required_checks, actual_checks;
        auto semantic = field(interpretation, "semantic"), models = field(semantic, "jobs");
        if (!models)
          return internal();
        for (const auto& model : models->items)
          if (scalar(model, "id") == target->job) {
            auto effective = field(model, "effective_checks");
            if (!effective)
              return internal();
            for (const auto& e : effective->items)
              if (std::ranges::find(target->structure.work, scalar(e, "work")) != target->structure.work.end())
                for (const auto& id : field(e, "checks")->items)
                  required_checks.emplace(scalar(e, "work"), id->string);
          }
        auto planned = field(plan, "checks"), definitions = field(registry, "checks"), tools = field(registry, "toolchains");
        if (!planned || !definitions || !tools)
          return internal();
        for (const auto& c : planned->items) {
          if (!actual_checks.emplace(scalar(c, "work"), scalar(c, "check")).second)
            return internal();
          value_ptr definition, tool;
          for (const auto& d : definitions->items)
            if (scalar(d, "id") == scalar(c, "check"))
              definition = d;
          if (!definition)
            return internal();
          for (const auto& t : tools->items)
            if (scalar(t, "id") == scalar(definition, "toolchain"))
              tool = t;
          if (!tool)
            return internal();
          for (const auto* key : {"toolchain", "argv", "coverage", "inputs", "timeout_ms", "max_output_bytes", "selection"})
            if (!same(field(c, key), field(definition, key)))
              return internal();
          if (!same(field(c, "command"), field(tool, "command")))
            return internal();
          std::map<std::string, value_ptr> env;
          for (const auto& e : field(tool, "environment")->items)
            env.emplace(scalar(e, "name"), field(e, "value"));
          if (!same(field(c, "environment"), object(std::move(env))))
            return internal();
        }
        if (actual_checks != required_checks)
          return internal();
        auto prepared = execution::prepare_check(plan, options.root, options.base_root, options.scratch);
        if (!prepared)
          return {{}, check_error_value(prepared.error(), execution_state->revision->id)};
        check_prepared = std::move(*prepared);
        return {execution::check_observation(*check_prepared), {}};
      }
      if (service == "reserve_check") {
        if (!check_prepared)
          return internal();
        auto expected = field(arguments, "expected");
        if (!same(expected, expectations(true, true)))
          return internal();
        auto reserved = ledger::reserve_check(options.ledger, *execution_state, options.request_id, options.actor, *check_request,
                                              options.run, check_provenance);
        if (!reserved)
          return {{}, execution_error_value(reserved.error())};
        check_reservation = *reserved;
        return {
            object({{"reservation", text(std::to_string(reserved->id))}, {"event", text(std::to_string(reserved->opened_seq))}}),
            {}};
      }
      if (service == "run_checks") {
        if (!check_prepared || !check_reservation)
          return internal();
        auto run = execution::run_checks(*check_prepared, check_reservation->id);
        if (!run)
          return {{}, check_error_value(run.error(), execution_state->revision->id)};
        ledger::execution_check_result result;
        result.observation       = run->observation;
        result.recapture_matches = run->recapture_matches;
        std::vector<value_ptr> evidence, facets;
        for (const auto& e : run->evidence) {
          result.evidence.push_back({e.work, e.check, e.outcome, e.reason, e.matched, e.exit_status, e.duration_ms,
                                     e.output_bytes, e.output_truncated, e.output, e.definition, e.vector});
          evidence.push_back(object({{"work", text(e.work)},
                                     {"check", text(e.check)},
                                     {"outcome", text(e.outcome)},
                                     {"reason", text(e.reason)},
                                     {"matched", text(e.matched ? std::to_string(*e.matched) : "none")},
                                     {"vector", e.vector}}));
        }
        for (const auto& f : run->publications) {
          ledger::execution_check_publication publication;
          publication.work     = f.work;
          publication.facet    = f.facet;
          publication.semantic = f.semantic;
          for (const auto& file : f.files)
            publication.files.push_back({file.path, file.bytes, file.mode == "0755" ? 493u : 420u});
          result.publications.push_back(std::move(publication));
          facets.push_back(object({{"work", text(f.work)}, {"facet", text(f.facet)}, {"semantic", text(f.semantic)}}));
        }
        check_result       = std::move(result);
        check_result_value = object({{"evidence", array(std::move(evidence))},
                                     {"recapture", text(run->recapture_matches ? "matched" : "differs")},
                                     {"observation", run->observation},
                                     {"facets", array(std::move(facets))},
                                     {"reservation", text(std::to_string(check_reservation->id))}});
        return {check_result_value, {}};
      }
      if (service == "commit_check" && options.operation == "check-recover") {
        if (scalar(arguments, "run_id") != options.run_id || scalar(arguments, "reservation") != options.reservation)
          return internal();
        auto result = ledger::recover_check(options.ledger, options.request_id, options.actor, *check_request,
                                            std::stoull(options.run_id), std::stoull(options.reservation));
        if (!result)
          return {{}, execution_error_value(result.error())};
        return saved(*result);
      }
      if (service == "commit_check") {
        if (!check_result || !check_reservation)
          return internal();
        auto change = field(arguments, "change"), facts = field(change, "facts");
        if (!same(field(facts, "evidence"), field(check_result_value, "evidence")) ||
            !same(field(facts, "observation"), check_result->observation) ||
            scalar(facts, "reservation") != std::to_string(check_reservation->id))
          return internal();
        std::vector<std::string> withheld;
        auto                     reasons = field(change, "withheld");
        if (!reasons || reasons->kind != value_kind::list)
          return internal();
        for (const auto& r : reasons->items)
          withheld.push_back(r->string);
        std::vector<std::uint64_t> cleared;
        auto                       blockers = field(facts, "blockers_cleared");
        if (!blockers || blockers->kind != value_kind::list)
          return internal();
        for (const auto& id : blockers->items)
          cleared.push_back(std::stoull(id->string));
        std::vector<ledger::execution_transition_input> transitions;
        auto                                            rows = field(change, "transitions");
        if (!rows || rows->kind != value_kind::list)
          return internal();
        for (const auto& r : rows->items) {
          ledger::execution_transition_input t;
          t.job            = scalar(r, "job");
          t.role           = scalar(r, "role");
          t.before_state   = scalar(r, "before_state");
          t.before_drifted = field(r, "before_drifted")->boolean;
          t.after_state    = scalar(r, "after_state");
          t.after_drifted  = field(r, "after_drifted")->boolean;
          t.reason         = scalar(r, "reason");
          transitions.push_back(std::move(t));
        }
        const bool completing = bool(field(facts, "completion"));
        if (completing != withheld.empty() || !same(field(change, "expected"), expectations(completing, false)))
          return internal();
        auto result = ledger::complete_check(options.ledger, *execution_state, check_reservation->id, options.run, *check_result,
                                             completing, transitions, cleared, withheld);
        if (!result)
          return {{}, execution_error_value(result.error())};
        return saved(*result);
      }
      if (service == "read_verdict") {
        auto input = execution::read_verdict_file(options.verdict);
        if (!input)
          return {{}, verdict_error_value(input.error())};
        auto normalized = execution::normalize_verdict(*input);
        if (!normalized)
          return {{}, verdict_error_value(normalized.error())};
        auto value = history::verdict_value(*normalized);
        if (!value)
          return internal();
        check_document = *value;
        return {check_document, {}};
      }
      if (service == "commit_verdict") {
        if (!token || !check_document)
          return internal();
        auto change = execution::decode_verdict_change(field(arguments, "change"), *execution_state, options.request_id,
                                                       *check_request, token->run_id, check_document);
        if (!change)
          return internal();
        auto authorized = ledger::authorize_run_token(options.ledger, options.run);
        if (!authorized)
          return {{}, execution_error_value(authorized.error())};
        change->run_token_text = options.run;
        auto result            = ledger::commit_execution(options.ledger, *execution_state, *change);
        if (!result)
          return {{}, execution_error_value(result.error())};
        return saved(*result);
      }
      return internal();
    });
  }

  std::optional<ledger::execution_snapshot> execution_state;
  // Activate: the prelude identity, and the saved result once the commit is known to have happened.
  std::string activate_identity;
  std::string activate_saved;
  bool        activate_committed = false;
  bool        load_execution() const override {
    return load_format3;
  }
  command_service_result read_execution(const engine::command_load_options& options) override {
    auto snapshot = ledger::read_execution_snapshot(options.ledger);
    if (!snapshot)
      return {{}, execution_error_value(snapshot.error())};
    if (snapshot->identity != load_ledger_identity)
      return {{}, execution_error_value(execution_failure(ledger::execution_error_code::path_denied))};
    auto encoded = execution::snapshot_value(*snapshot);
    if (!encoded)
      return {{}, native_error({"internal", "Invalid embedded command protocol"})};
    execution_state = std::move(*snapshot);
    return {*encoded, {}};
  }
  command_service_result accept_load(const engine::command_load_options& options, const value_ptr& bundle,
                                     const value_ptr& semantic, const value_ptr& reconciled,
                                     std::span<const history::tree_file> provenance,
                                     ledger::completion_receipt&         completion) override {
    load_receipt = &completion;
    if (!load_capture || !load_intent || !execution_state)
      return {{}, native_error({"internal", "Missing native load intent"})};
    auto intent_data = load_intent->data();
    if (!intent_data || !(*intent_data)->version2)
      return {{}, native_error({"internal", "Missing native load intent"})};
    auto candidate = load_capture->prepare(bundle, semantic, provenance);
    if (!candidate)
      return {{}, load_error(candidate.error())};
    auto data = candidate->data();
    if (!data)
      return {{}, native_error({"internal", "Invalid native load preparation"})};
    const auto& input = **data;
    if (input.workspace_id != execution_workspace)
      return {{}, ledger_error({ledger::format_error_code::invalid_argument, ""})};
    // The candidate records (G2): interned by the commit inside its own write transaction.
    ledger::execution_prepared_revision records{{input.source, history::tree_profile::source},
                                                {input.registry.files, history::tree_profile::provenance},
                                                {input.stdlib.files, history::tree_profile::provenance},
                                                input.interpretation.encoded,
                                                input.interpretation.versions_encoded};
    // A load whose five records equal the active revision exactly is a noop: it names the active identifiers and carries
    // no prepared records (EX-4, DD25).
    ledger::execution_revision_input revision{};
    bool                             noop = false;
    if (const auto& active = execution_state->revision) {
      const auto registry_digest = active->registry_value && active->registry_value->fields.contains("digest") &&
                                           active->registry_value->fields.at("digest")->kind == value_kind::string
                                       ? std::optional<std::string>(active->registry_value->fields.at("digest")->string)
                                       : std::nullopt;
      noop = tree_digest(records.source.files, records.source.profile) == std::optional<std::string>(active->source_digest) &&
             tree_digest(records.stdlib.files, records.stdlib.profile) == std::optional<std::string>(active->stdlib_digest) &&
             registry_digest && tree_digest(records.registry.files, records.registry.profile) == registry_digest &&
             canonical_text(active->interpretation_value) == std::optional<std::string>(records.interpretation) &&
             canonical_text(active->versions_value) == std::optional<std::string>(records.versions);
      if (noop)
        revision = ledger::execution_revision_input{active->source,         active->registry, active->stdlib,
                                                    active->interpretation, active->versions, true};
    }
    execution::load_options typed{*(*intent_data)->version2, revision, std::nullopt};
    if (!noop)
      typed.prepared = std::move(records);
    const execution::change_request request{(*intent_data)->request_id, (*intent_data)->fields.actor, std::nullopt,
                                            std::move(typed), nullptr};
    auto                            change = execution::decode_execution_change(reconciled, *execution_state, request);
    if (!change)
      return {{}, native_error({"internal", "Invalid embedded command protocol"})};
    auto staged = ledger::stage_execution(options.ledger, *execution_state, *change);
    if (!staged)
      return {{}, execution_error_value(staged.error())};
    if (staged->result().replayed) {
      completion.saved_result = staged->result().saved_result;
      completion.phase        = ledger::completion_phase::replayed;
    } else {
      // Final receipt verification immediately before COMMIT, exactly as the format1 and format2 path.
      if (!candidate->verify()) {
        staged->abort();
        return {{}, ledger_error({ledger::format_error_code::capture_changed, ""})};
      }
      completion.saved_result = staged->result().saved_result;
      completion.phase        = ledger::completion_phase::commit_uncertain;
      if (auto committed = staged->commit(); !committed) {
        // A refused COMMIT leaves nothing written (execution_write::commit).
        completion.phase = ledger::completion_phase::not_completed;
        completion.saved_result.clear();
        return {{}, execution_error_value(committed.error())};
      }
      completion.phase = ledger::completion_phase::committed;
    }
    auto saved_value = engine::codec_decode(completion.saved_result);
    if (!saved_value)
      return {{}, native_error({"internal", "Invalid saved load response"})};
    return {*saved_value, {}};
  }
  command_service_result read_execution(const engine::command_activate_options& options) override {
    auto snapshot = ledger::read_execution_snapshot(options.ledger);
    if (!snapshot)
      return {{}, execution_error_value(snapshot.error())};
    if (snapshot->identity != activate_identity)
      return {{}, execution_error_value(execution_failure(ledger::execution_error_code::path_denied))};
    // Refusal step (4): EX-6 interpretation binding, only when an active revision exists.
    if (snapshot->revision) {
      const auto embedded = execution_binding();
      if (!embedded)
        return {{}, native_error({"internal", "Invalid embedded command protocol"})};
      if (*embedded != snapshot->revision->stdlib_digest)
        return {{}, execution_code("interpretation_mismatch")};
    }
    // Refusal step (5): expectations (D111). --expect-job NONE while a job is active is the D149 state_conflict site.
    const std::string actual = snapshot->revision ? std::to_string(snapshot->revision->id) : std::string("none");
    if (options.expect_revision != actual) {
      auto conflict     = execution_failure(ledger::execution_error_code::revision_conflict);
      conflict.expected = options.expect_revision;
      conflict.actual   = actual;
      return {{}, execution_error_value(conflict)};
    }
    if (options.expect_job == "NONE") {
      if (!snapshot->active_job.empty())
        return {{}, execution_code("state_conflict")};
    } else if (snapshot->active_job != options.expect_job) {
      return {{}, execution_code("no_active_job")};
    }
    auto encoded = execution::snapshot_value(*snapshot);
    if (!encoded)
      return {{}, native_error({"internal", "Invalid embedded command protocol"})};
    execution_state = std::move(*snapshot);
    return {*encoded, {}};
  }
  // status and preview (R15-D P1): the prelude identity and the draft capture owner of status --workspace --registry.
  std::string                               read_identity;
  std::optional<acceptance::source_capture> draft_capture;
  /// Shared snapshot read of status and preview. `bound` runs refusal steps (4) and (6) of P2-1 (EX-6 binding and
  /// no_active_job) inside the service (DD-D2); `reasons_role` adds the DD-D4 reasons to that role's snapshot row.
  command_service_result execution_read(const std::string& ledger_path, bool bound,
                                        const std::optional<std::string>& reasons_role) {
    auto snapshot = ledger::read_execution_snapshot(ledger_path);
    if (!snapshot)
      return {{}, execution_error_value(snapshot.error())};
    if (snapshot->identity != read_identity)
      return {{}, execution_error_value(execution_failure(ledger::execution_error_code::path_denied))};
    if (bound) {
      if (snapshot->revision) {
        const auto embedded = execution_binding();
        if (!embedded)
          return {{}, native_error({"internal", "Invalid embedded command protocol"})};
        if (*embedded != snapshot->revision->stdlib_digest)
          return {{}, execution_code("interpretation_mismatch")};
      }
      if (snapshot->active_job.empty())
        return {{}, execution_code("no_active_job")};
    }
    auto encoded = execution::snapshot_value(*snapshot);
    if (!encoded)
      return {{}, native_error({"internal", "Invalid embedded command protocol"})};
    value_ptr out = *encoded;
    if (reasons_role && !snapshot->active_job.empty()) {
      auto reasons = role_reasons(ledger_path, snapshot->active_job, *reasons_role);
      if (!reasons)
        return {{}, execution_error_value(reasons.error())};
      const auto&            inner = out->fields.at("snapshot");
      std::vector<value_ptr> roles;
      for (const auto& row : inner->fields.at("roles")->items) {
        const bool target =
            row->fields.at("job")->string == snapshot->active_job && row->fields.at("role")->string == *reasons_role;
        roles.push_back(target ? with_member(row, "reasons", *reasons) : row);
      }
      out = with_member(out, "snapshot", with_member(inner, "roles", array(std::move(roles))));
    }
    execution_state = std::move(*snapshot);
    return {out, {}};
  }
  std::string                 recovery_job, recovery_role;
  execution::recovery_options recovery_intent(const engine::command_recovery_options& o) {
    return {o.operation, recovery_job, recovery_role, o.run, o.expect_fp, o.reason, o.observations, o.provenance};
  }
  command_service_result read_execution(const engine::command_recovery_options& o) override {
    auto read = execution_read(o.ledger, true, std::nullopt);
    if (read.error)
      return read;
    auto authorized = ledger::authorize_run_token(o.ledger, o.run);
    if (!authorized)
      return {{}, execution_error_value(authorized.error())};
    const auto token = ledger::parse_run_token(o.run);
    for (const auto& row : execution_state->roles)
      if (row.run_id == token->run_id) {
        recovery_job  = row.job;
        recovery_role = row.role;
        if (o.operation == "refresh" && (!row.dispatch || ("xxh3_128:" + row.dispatch->work_fp) != o.expect_fp))
          return {{}, execution_code("drifted")};
        return {with_member(with_member(read.value, "target_job", text(row.job)), "target_role", text(row.role)), {}};
      }
    return {{}, execution_error_value(execution_failure(ledger::execution_error_code::unknown_run))};
  }
  command_service_result read_execution(const engine::command_recover_options& o) override {
    return execution_read(o.ledger, true, std::nullopt);
  }
  command_service_result read_observations(const engine::command_recovery_options& o) override {
    return capture_boundary([&]() -> command_service_result {
      required(_session.add_input("observations", required(resolve_explicit_file(*o.observations))));
      auto file = required(_session.read("observations", "input.json"));
      return {object({{"bytes", text(file.bytes)}}), {}};
    });
  }
  command_service_result read_provenance(const engine::command_recovery_options& o) override {
    engine::command_run_options run;
    run.provenance = o.provenance;
    return read_provenance(run);
  }
  command_service_result commit_execution(const engine::command_recovery_options& o, const value_ptr& change) override {
    execution::change_request request{o.request_id, o.actor, execution_profile, recovery_intent(o), run_provenance};
    auto                      decoded = execution::decode_execution_change(change, *execution_state, request);
    if (!decoded)
      return {{}, native_error({"internal", "Invalid embedded command protocol"})};
    auto committed = ledger::commit_execution(o.ledger, *execution_state, *decoded);
    if (!committed)
      return {{}, execution_error_value(committed.error())};
    activate_committed = true;
    activate_saved     = committed->saved_result;
    auto saved         = engine::codec_decode(activate_saved);
    if (!saved)
      return {{}, native_error({"internal", "Invalid embedded command protocol"})};
    if (o.operation == "refresh") {
      auto assembled = assemble_run_result(o.ledger, *saved);
      if (!assembled)
        return {{}, execution_error_value(assembled.error())};
      return {*assembled, {}};
    }
    return {*saved, {}};
  }
  value_ptr retained_value(const ledger::retained_dispatch& d) {
    return object({{"dispatch", object({{"id", text(std::to_string(d.id))},
                                        {"run", text(std::to_string(d.run_id))},
                                        {"kind", text(d.kind)},
                                        {"request_id", text(d.request_id)},
                                        {"seq", text(std::to_string(d.seq))},
                                        {"revision", text(std::to_string(d.revision_id))},
                                        {"work_fp", text("xxh3_128:" + d.work_fp)},
                                        {"render_digest", text("xxh3_128:" + d.render_fp)}})},
                   {"run_state", text(d.run_state)},
                   {"markdown", text(d.markdown)},
                   {"manifest", d.manifest},
                   {"work", d.work}});
  }
  command_service_result read_dispatch(const engine::command_preview_options& o) override {
    auto dispatch =
        ledger::find_dispatch_by_render(o.ledger, execution_state->active_job, o.role, std::string_view(*o.since).substr(9));
    if (!dispatch)
      return {{}, execution_error_value(dispatch.error())};
    if (!*dispatch)
      return {{}, execution_code("history_unavailable")};
    return {retained_value(**dispatch), {}};
  }
  command_service_result capture_outputs(const engine::command_recover_options& o) override {
    auto captured = execution::capture_recovery_outputs(*execution_state, o.role, *o.root, *o.base_root);
    if (!captured)
      return {{}, native_error(captured.error())};
    return {*captured, {}};
  }
  command_service_result read_recovery(const engine::command_recover_options& o) override {
    auto records = ledger::read_recovery_records(o.ledger, execution_state->active_job, o.role);
    if (!records)
      return {{}, execution_error_value(records.error())};
    value_ptr run = text("none"), checkpoint = text("none"), ack = text("none");
    if (records->run) {
      const auto& r = *records->run;
      run           = object({{"id", text(std::to_string(r.id))},
                              {"generation", text(std::to_string(r.generation_ordinal))},
                              {"request_id", text(r.request_id)},
                              {"state", text(r.state)}});
    }
    if (records->acknowledged)
      ack = retained_value(*records->acknowledged);
    if (records->checkpoint) {
      const auto& c = *records->checkpoint;
      checkpoint    = object({{"id", text(std::to_string(c.id))},
                              {"run", text(std::to_string(c.run_id))},
                              {"event", text(std::to_string(c.seq))},
                              {"value", c.value}});
    }
    return {object({{"run", run},
                    {"acknowledged", ack},
                    {"checkpoint", checkpoint},
                    {"evidence", array(std::move(records->evidence))},
                    {"findings", array(std::move(records->findings))}}),
            {}};
  }
  value_ptr              run_provenance;
  command_service_result read_execution(const engine::command_run_options& options) override {
    auto read = execution_read(options.ledger, true, std::nullopt);
    if (read.error)
      return read;
    if (execution_state->active_job != options.job)
      return {{}, execution_code("no_active_job")};
    const ledger::execution_role* target = nullptr;
    for (const auto& row : execution_state->roles)
      if (row.job == options.job && row.role == options.role)
        target = &row;
    if (!target || target->state == "retired")
      return {{},
              object({{"code", text("reference")},
                      {"message", text("Unknown selected role")},
                      {"field", text("selection.role")},
                      {"details", object({})}})};
    if (options.run_id) {
      auto id    = std::stoull(*options.run_id);
      auto named = ledger::lookup_named_run(options.ledger, id);
      if (!named)
        return {{}, execution_error_value(named.error())};
      if (!*named || (*named)->job != options.job || (*named)->role != options.role)
        return {{}, execution_error_value(execution_failure(ledger::execution_error_code::state_conflict))};
      if (target->state != "running" || target->run_id != id) {
        auto error  = execution_failure(ledger::execution_error_code::fenced);
        error.state = (*named)->state;
        return {{}, execution_error_value(error)};
      }
    } else if (target->state != "ready" && target->state != "stale") {
      auto error  = execution_failure(ledger::execution_error_code::claim_conflict);
      error.state = target->state;
      return {{}, execution_error_value(error)};
    }
    return read;
  }
  command_service_result read_provenance(const engine::command_run_options& options) override {
    auto value = execution::read_provenance(*options.provenance);
    if (!value)
      return {{},
              object({{"code", text(value.error().code)},
                      {"message", text(value.error().message)},
                      {"field", text(value.error().field)},
                      {"details", object({})}})};
    run_provenance = *value;
    return {*value, {}};
  }
  command_service_result commit_execution(const engine::command_run_options& options, const value_ptr& change) override {
    execution::operation_options op = execution::claim_options{options.job, options.role, options.provenance};
    if (options.run_id)
      op = execution::takeover_options{options.job, options.role, std::stoull(*options.run_id), options.provenance};
    execution::change_request request{options.request_id, options.actor, execution_profile, std::move(op), run_provenance};
    auto                      decoded = execution::decode_execution_change(change, *execution_state, request);
    if (!decoded)
      return {{}, native_error({"internal", "Invalid embedded command protocol"})};
    auto committed = ledger::commit_execution(options.ledger, *execution_state, *decoded);
    if (!committed)
      return {{}, execution_error_value(committed.error())};
    activate_committed = true;
    activate_saved     = committed->saved_result;
    auto saved         = engine::codec_decode(activate_saved);
    if (!saved)
      return {{}, native_error({"internal", "Invalid embedded command protocol"})};
    auto assembled = assemble_run_result(options.ledger, *saved);
    if (!assembled)
      return {{}, execution_error_value(assembled.error())};
    return {*assembled, {}};
  }
  command_service_result read_execution(const engine::command_status_options& options) override {
    return execution_read(options.ledger, options.role.has_value(), options.role);
  }
  command_service_result read_execution(const engine::command_preview_options& options) override {
    return execution_read(options.ledger, true, std::nullopt);
  }
  command_service_result read_request(const engine::command_status_options& options) override {
    auto opened = ledger::open_format3(options.ledger, ledger::execution_access::read_only);
    if (!opened)
      return {{}, execution_error_value(opened.error())};
    const auto failure = execution_error_value(execution_failure(ledger::execution_error_code::io));
    // Keep the completed-request and open-reservation branches in one read snapshot.
    auto transaction = opened->sql().begin(false);
    if (!transaction)
      return {{}, failure};
    auto query = opened->sql().prepare("SELECT operation,outcome,result FROM requests WHERE request_id=?1");
    if (!query || !query->bind_text(1, *options.request_id))
      return {{}, failure};
    auto step = query->step();
    if (!step)
      return {{}, failure};
    if (*step == ledger::step_result::done) {
      auto reservation = opened->sql().prepare("SELECT r.id,r.run_id,g.ordinal FROM reservations r "
                                               "JOIN reservation_state s ON s.reservation_id=r.id "
                                               "JOIN runs u ON u.id=r.run_id JOIN generations g ON g.id=u.generation_id "
                                               "WHERE r.request_id=?1 AND s.state='open'");
      if (!reservation || !reservation->bind_text(1, *options.request_id))
        return {{}, failure};
      auto found = reservation->step();
      if (!found)
        return {{}, failure};
      if (*found == ledger::step_result::done)
        return {{}, native_error({"request_not_found", "Recorded history view is unavailable"})};
      auto id = reservation->integer(0), run = reservation->integer(1), generation = reservation->integer(2);
      if (!id || !*id || **id <= 0 || !run || !*run || **run <= 0 || !generation || !*generation || **generation <= 0)
        return {{}, execution_error_value(execution_failure(ledger::execution_error_code::ledger_corrupt))};
      return {
          object({{"request_id", text(*options.request_id)},
                  {"operation", text("check")},
                  {"outcome", text("in_progress")},
                  {"reservation", text(std::to_string(**id))},
                  {"run", object({{"id", text(std::to_string(**run))}, {"generation", text(std::to_string(**generation))}})}}),
          {}};
    }
    auto operation = query->text(0), outcome = query->text(1);
    auto saved = column_bytes(*query, 2);
    if (!operation || !*operation || !outcome || !*outcome || !saved)
      return {{}, failure};
    auto decoded = engine::codec_decode(*saved);
    if (!decoded)
      return {{}, execution_error_value(execution_failure(ledger::execution_error_code::ledger_corrupt))};
    std::map<std::string, value_ptr> fields{{"request_id", text(*options.request_id)},
                                            {"operation", text(**operation)},
                                            {"outcome", text(**outcome)},
                                            {"result", *decoded}};
    if (**operation == "claim" || **operation == "takeover") {
      auto assembled = assemble_run_result(options.ledger, *decoded);
      if (!assembled)
        return {{}, execution_error_value(assembled.error())};
      fields["result"] = *assembled;
      // EX-2 disposition: the state of the request's run, `current` for an active run.
      auto run = opened->sql().prepare(
          "SELECT rs.state FROM runs r JOIN run_state rs ON rs.run_id=r.id WHERE r.request_id=?1 ORDER BY r.id DESC LIMIT 1");
      if (!run || !run->bind_text(1, *options.request_id))
        return {{}, failure};
      auto found = run->step();
      if (!found || *found == ledger::step_result::done)
        return {{}, execution_error_value(execution_failure(ledger::execution_error_code::ledger_corrupt))};
      auto state = run->text(0);
      if (!state || !*state)
        return {{}, failure};
      fields["disposition"] = text(**state == "active" ? std::string("current") : **state);
    }
    return {object(std::move(fields)), {}};
  }
  command_service_result open_workspace(const engine::command_status_options& options) override {
    auto opened = acceptance::source_capture::open_embedded(*options.workspace, *options.registry);
    if (!opened)
      return {{}, source_failure(opened.error())};
    draft_capture.emplace(std::move(*opened));
    auto value = draft_capture->workspace();
    if (!value)
      return {{}, source_failure(value.error())};
    return {*value, {}};
  }
  command_service_result capture_drafts(const engine::command_status_options& options, const value_ptr& route) override {
    if (!draft_capture || !execution_state)
      return {{}, native_error({"internal", "Missing native load capture"})};
    auto bundle = draft_capture->capture(route);
    if (!bundle) {
      auto error = source_failure(bundle.error());
      if (draft_content_class(error->fields.at("code")->string))
        return {object({{"outcome", text("rejected")}, {"error", typed_error(error)}, {"changed", array()}}), {}};
      return {{}, error};
    }
    auto& sealed = draft_capture->sealed();
    if (!sealed)
      return {{}, native_error({"internal", "Missing native load capture"})};
    auto files = sealed->snapshot();
    if (!files)
      return {{}, native_error(files.error())};
    std::vector<history::tree_file> captured;
    for (const auto& file : **files) {
      if (file.root != "workspace")
        continue;
      auto mode = captured_mode(file.mode);
      if (!mode)
        return {{}, native_error({"internal", "Invalid native load preparation"})};
      captured.push_back({file.path, file.bytes, *mode});
    }
    std::vector<history::tree_file> recorded;
    if (execution_state->revision) {
      auto reader = ledger::open_existing(options.ledger);
      if (!reader)
        return {{}, ledger_error(reader.error())};
      auto source = reader->read_record(execution_state->revision->source);
      if (!source)
        return {{}, ledger_error(source.error())};
      auto tree = history::read_tree(*reader, source->reference, history::tree_profile::source);
      if (!tree)
        return {{}, ledger_error({ledger::format_error_code::corrupt, ""})};
      recorded = std::move(*tree);
    }
    std::vector<value_ptr> changed;
    for (auto& path : execution::changed_paths(captured, recorded))
      changed.push_back(text(std::move(path)));
    return {object({{"outcome", text("captured")}, {"bundle", *bundle}, {"changed", array(std::move(changed))}}), {}};
  }
  command_service_result commit_execution(const engine::command_activate_options& options, const value_ptr& change) override {
    if (!execution_state)
      return {{}, native_error({"internal", "Invalid embedded command protocol"})};
    const execution::change_request request{options.request_id, options.actor, execution_profile,
                                            execution::activate_options{options.job, options.expect_revision, options.expect_job},
                                            nullptr};
    auto                            decoded = execution::decode_execution_change(change, *execution_state, request);
    if (!decoded)
      return {{}, native_error({"internal", "Invalid embedded command protocol"})};
    auto committed = ledger::commit_execution(options.ledger, *execution_state, *decoded);
    if (!committed)
      return {{}, execution_error_value(committed.error())};
    activate_saved     = committed->saved_result;
    activate_committed = true;
    auto saved_value   = engine::codec_decode(activate_saved);
    if (!saved_value)
      return {{}, native_error({"internal", "Invalid embedded command protocol"})};
    return {*saved_value, {}};
  }
  command_service_result open_workspace(const command_render_options& options) override {
    return capture_boundary([&]() -> command_service_result {
      const std::filesystem::path file(required(resolve_explicit_file(options.workspace)));
      required(_session.add_root("workspace", file.parent_path().string()));
      _workspace_path = file.filename().string();
      _workspace      = required(_session.load(_workspace_path)).root;
      return {object({{"file", text(_workspace_path)}, {"workspace", _workspace}}), {}};
    });
  }
  command_service_result capture_render(const command_render_options& options, const value_ptr& route) override {
    return capture_boundary([&]() -> command_service_result {
      const auto source_id  = route->fields.at("root")->fields.at("id")->string;
      auto       mapping_id = options.root.substr(0, options.root.find('='));
      if (mapping_id != source_id)
        throw refusal{object({{"code", text("reference")},
                              {"field", text("root")},
                              {"message", text("Root mapping must match workspace source ID")}})};
      auto                   spec_path = route->fields.at("spec");
      auto                   spec      = required(_session.load(spec_path->string));
      std::vector<value_ptr> jobs;
      for (const auto& job : route->fields.at("jobs")->items) {
        auto path   = job->fields.at("file");
        auto loaded = required(_session.load(path->string));
        jobs.push_back(object({{"file", path}, {"value", loaded.root}}));
      }
      required(_session.add_input("registry", required(resolve_explicit_file(options.registry))));
      auto registry = required(_session.read("registry", "input.json"));
      if (!options.observations.value_or("").empty()) {
        required(_session.add_input("observations", required(resolve_explicit_file(options.observations.value_or("")))));
        required(_session.read("observations", "input.json"));
      }
      required(_session.add_root("current:" + source_id,
                                 required(resolve_explicit_directory(options.root.substr(options.root.find('=') + 1)))));
      if (options.capture_outputs) {
        required(_session.add_root("base:" + source_id, required(resolve_explicit_directory(options.base_root.value_or("").substr(
                                                            options.base_root.value_or("").find('=') + 1)))));
        std::vector<std::string> exclusions;
        for (const auto& entry : route->fields.at("root")->fields.at("excluded")->items)
          exclusions.push_back(entry->string);
        required(_session.tree("current:" + source_id, exclusions));
        required(_session.tree("base:" + source_id, exclusions));
      }
      auto                   snapshot    = required(_session.finish());
      auto                   directories = required(_session.directories());
      std::vector<value_ptr> retained, workspace_files, source_files, source_directories;
      for (const auto& file : *snapshot) {
        retained.push_back(object(
            {{"root", text(file.root)}, {"path", text(file.path)}, {"mode", text(file.mode)}, {"bytes", text(file.bytes)}}));
        if (file.root == "workspace")
          workspace_files.push_back(object({{"path", text(file.path)}, {"bytes", text(file.bytes)}}));
        if (file.root.starts_with("current:") || file.root.starts_with("base:"))
          source_files.push_back(object({{"root", text(file.root)}, {"path", text(file.path)}}));
      }
      for (const auto& directory : *directories)
        source_directories.push_back(object({{"root", text(directory.root)}, {"path", text(directory.path)}}));
      auto bundle = object({{"workspace", object({{"file", text(_workspace_path)}, {"value", _workspace}})},
                            {"spec", object({{"file", spec_path}, {"value", spec.root}})},
                            {"jobs", array(jobs)},
                            {"registry", object({{"file", text("input.json")}, {"bytes", text(registry.bytes)}})},
                            {"files", array(workspace_files)}});
      auto capture =
          options.capture_outputs
              ? object({{"status", text("captured")}, {"files", array(source_files)}, {"directories", array(source_directories)}})
              : object({{"status", text("not_captured")}});
      std::map<std::string, value_ptr> request{{"bundle", bundle},
                                               {"capture", capture},
                                               {"files", array(retained)},
                                               {"job", text(std::string(options.job))},
                                               {"role", text(std::string(options.role))}};
      if (options.budget_bytes)
        request["budget_bytes"] = integer(*options.budget_bytes);
      return {object(std::move(request)), {}};
    });
  }
  command_service_result initialize_ledger(const command_init_options& options) override {
    try {
      // Preserve the existing syntax/schema boundary before any filesystem traversal.
      const auto filename = std::filesystem::path(options.ledger).filename().string();
      if (filename.empty() || filename == "." || filename == "..")
        return {{}, ledger_error({engine::ledger::types::format_error_code::invalid_argument, ""})};
      // Init cutover (EX-1, Q13 A, XD5): a new ledger is format3; the CLI signature and response bytes are unchanged.
      auto metadata = engine::ledger::initialize_format3(options.ledger, options.workspace_id);
      if (!metadata)
        return {{}, ledger_error(init_format_error(metadata.error()))};
      return {object({{"ledger", text(metadata->identity)},
                      {"workspace", text(metadata->workspace_id)},
                      {"active_revision", text("none")},
                      {"event", text("1")}}),
              {}};
    } catch (const std::bad_alloc&) {
      throw;
    } catch (...) {
      return {{}, ledger_error({engine::ledger::types::format_error_code::io, ""})};
    }
  }
  command_service_result read_history(const command_history_options& options, const command_history_selector& selector) override {
    using namespace engine::history;
    try {
      history_query q;
      if (selector.selector == "revision")
        q.selector = history_selector::revision;
      else if (selector.selector == "request")
        q.selector = history_selector::request;
      else if (selector.selector == "state")
        q.selector = history_selector::state;
      else if (selector.selector == "event")
        q.selector = history_selector::event;
      // The retained VM has already checked exact agreement with this typed intent.
      q.id = static_cast<std::uint64_t>(
          options.event.value_or(options.revision.value_or(options.at_event.value_or(options.after_event.value_or(0)))));
      q.request_id = selector.request_id;
      q.limit      = selector.limit;
      auto file    = engine::ledger::open_existing(options.ledger);
      if (!file)
        return {{}, ledger_error(file.error())};
      if (auto refused = verify_format3_read(options.ledger, file->metadata()))
        return {{}, refused};
      auto result = engine::history::read_history(*file, q);
      if (result)
        return {*result, {}};
      const auto& e = result.error();
      if (e.code == history_read_code::storage)
        return {{}, ledger_error(e.storage)};
      if (e.code == history_read_code::corrupt)
        return {{}, ledger_error({engine::ledger::format_error_code::corrupt, ""})};
      const auto code = e.code == history_read_code::revision_not_found  ? "revision_not_found"
                        : e.code == history_read_code::request_not_found ? "request_not_found"
                        : e.code == history_read_code::event_not_found   ? "event_not_found"
                                                                         : "unsupported_interpretation";
      return {{}, native_error({code, "Recorded history view is unavailable"})};
    } catch (const std::bad_alloc&) {
      throw;
    } catch (...) {
      return {{}, ledger_error({engine::ledger::types::format_error_code::io, ""})};
    }
  }
  command_service_result export_source(const command_export_options& options) override {
    {
      // Format3 sources are read by the storage read variant after the complete format3 verification (DD35, DD7).
      auto file = engine::ledger::open_existing(options.ledger);
      if (file) {
        if (auto refused = verify_format3_read(options.ledger, file->metadata()))
          return {{}, refused};
      }
    }
    auto result = engine::source_export::export_source(options.ledger, options.revision, options.target);
    if (!result)
      return {{}, export_error(result.error())};
    export_published = true;
    return {object({{"ledger", text(result->ledger)},
                    {"revision", text(std::to_string(result->revision))},
                    {"source", text(std::to_string(result->source))}}),
            {}};
  }
#ifdef PLANAR_AUTHORING_BUNDLE
  /// Open the authoring workspace exactly like render: resolve the explicit file, root its directory, load the declaration.
  command_service_result open_workspace(const command_authoring_options& options) override {
    return capture_boundary([&]() -> command_service_result {
      const std::filesystem::path file(required(resolve_explicit_file(options.workspace)));
      required(_session.add_root("workspace", file.parent_path().string()));
      _workspace_path = file.filename().string();
      _workspace      = required(_session.load(_workspace_path)).root;
      return {object({{"file", text(_workspace_path)}, {"workspace", _workspace}}), {}};
    });
  }
  /// Capture the workspace, optionally the spec, and the registry. Job files are never read, listed or required.
  command_service_result capture_authoring(const command_authoring_options& options, const value_ptr& route,
                                           std::string_view include) override {
    return capture_boundary([&]() -> command_service_result {
      std::map<std::string, value_ptr> bundle_fields;
      if (include == "spec") {
        auto spec_path = route->fields.at("spec");
        auto spec      = required(_session.load(spec_path->string));
        bundle_fields.emplace("spec", object({{"file", spec_path}, {"value", spec.root}}));
      }
      auto captured = finish_authoring(options.registry, std::move(bundle_fields), {});
      return {object({{"bundle", captured.bundle}, {"capture", captured.capture}, {"files", captured.files}}), {}};
    });
  }
  /// Capture the spec, the registry and exactly the selected job; other listed jobs are neither read nor required.
  command_service_result capture_orchestrator(const command_authoring_options& options, const value_ptr& route,
                                              std::string_view job) override {
    return capture_boundary([&]() -> command_service_result {
      auto      spec_path = route->fields.at("spec");
      auto      spec      = required(_session.load(spec_path->string));
      value_ptr job_path;
      for (const auto& entry : route->fields.at("jobs")->items)
        if (entry->fields.at("id")->string == job)
          job_path = entry->fields.at("file");
      if (!job_path)
        throw refusal{unknown_selected_job()};
      auto                             loaded = required(_session.load(job_path->string));
      std::map<std::string, value_ptr> bundle_fields;
      bundle_fields.emplace("spec", object({{"file", spec_path}, {"value", spec.root}}));
      auto captured =
          finish_authoring(options.registry, std::move(bundle_fields), {object({{"file", job_path}, {"value", loaded.root}})});
      return {object({{"bundle", captured.bundle},
                      {"capture", captured.capture},
                      {"files", captured.files},
                      {"job", text(std::string(job))}}),
              {}};
    });
  }
  command_service_result read_orchestrator(const command_authoring_options& options) override {
    auto opened = ledger::open_format3(options.ledger, ledger::execution_access::read_only);
    if (!opened)
      return {{}, execution_error_value(opened.error())};
    read_identity = opened->metadata().identity;
    return execution_read(options.ledger, false, std::nullopt);
  }
  /// Read the diagnostic file through the bounded no-follow capture of the engine.
  command_service_result read_diagnostic(const command_authoring_options& options) override {
    return capture_boundary([&]() -> command_service_result {
      capture_session session;
      auto            facts = required(capture_diagnostic(session, options.diagnostic, 65536));
      return {object({{"bytes", text(facts.bytes)},
                      {"size", integer(static_cast<std::int64_t>(facts.size))},
                      {"digest", text(facts.digest)}}),
              {}};
    });
  }
  /// Read the observation directory through the directory module.
  command_service_result read_observation_directory(const command_authoring_options& options) override {
    return capture_boundary([&]() -> command_service_result {
      auto                   directory = required(engine::read_observation_directory(options.from));
      std::vector<value_ptr> entries;
      for (const auto& entry : directory.entries) {
        auto digest = engine::file_digest(entry.bytes);
        if (!digest)
          throw observation_digest_failure(std::move(digest.error()));
        entries.push_back(object({{"category", text(entry.category)},
                                  {"name", text(entry.name)},
                                  {"size", integer(static_cast<std::int64_t>(entry.size))},
                                  {"digest", text(*digest)},
                                  {"bytes", text(entry.bytes)}}));
      }
      return {object({{"entries", array(entries)}}), {}};
    });
  }

private:
  /// The value render's selection stage gives for a job the workspace does not list, byte for byte.
  static value_ptr unknown_selected_job() {
    // Render's selection stage owns this existing message; kept out of a literal pair so no source adds a message to the table.
    static const std::string unknown_job = "Unknown selected job";
    return object({{"code", text("reference")},
                   {"message", text(unknown_job)},
                   {"file", text("")},
                   {"field", text("selection.job")},
                   {"details", object({})}});
  }
  struct authoring_capture {
    value_ptr bundle, capture, files;
  };
  /// Register the registry, seal the session and build the render-shaped bundle, capture and files values.
  authoring_capture finish_authoring(const std::string& registry_path, std::map<std::string, value_ptr> bundle_fields,
                                     std::vector<value_ptr> jobs) {
    required(_session.add_input("registry", required(resolve_explicit_file(registry_path))));
    auto                   registry = required(_session.read("registry", "input.json"));
    auto                   snapshot = required(_session.finish());
    std::vector<value_ptr> retained, workspace_files;
    for (const auto& file : *snapshot) {
      retained.push_back(
          object({{"root", text(file.root)}, {"path", text(file.path)}, {"mode", text(file.mode)}, {"bytes", text(file.bytes)}}));
      if (file.root == "workspace")
        workspace_files.push_back(object({{"path", text(file.path)}, {"bytes", text(file.bytes)}}));
    }
    bundle_fields.emplace("workspace", object({{"file", text(_workspace_path)}, {"value", _workspace}}));
    bundle_fields.emplace("jobs", array(jobs));
    bundle_fields.emplace("registry", object({{"file", text("input.json")}, {"bytes", text(registry.bytes)}}));
    bundle_fields.emplace("files", array(workspace_files));
    return {object(std::move(bundle_fields)), object({{"status", text("not_captured")}}), array(retained)};
  }
#endif
};
} // namespace
std::expected<void, engine::diagnostic>
prepare_invocation_paths(engine::command_options&                                               options,
                         const std::function<std::expected<std::string, engine::diagnostic>()>& cwd_reader,
                         const std::function<std::expected<std::string, engine::diagnostic>()>& workspace_directory_reader) {
  struct selected_path {
    std::string* target;
    std::size_t  prefix;
    bool         lexical = false;
  };
  std::vector<selected_path> selected;
  if (auto* render = std::get_if<engine::command_render_options>(&options)) {
    const auto named = engine::workspace_is_name(render->workspace);
    if (!named)
      return std::unexpected(named.error());
    std::optional<std::string> directory;
    if (*named) {
      auto captured = workspace_directory_reader();
      if (!captured)
        return std::unexpected(captured.error());
      directory = std::move(*captured);
    }
    auto workspace =
        engine::expand_workspace(render->workspace, directory ? std::optional<std::string_view>(*directory) : std::nullopt);
    if (!workspace)
      return std::unexpected(workspace.error());
    render->workspace = std::move(*workspace);
    selected.push_back({&render->workspace, 0});
    selected.push_back({&render->registry, 0});
    selected.push_back({&render->root, render->root.find('=') + 1});
    if (render->base_root)
      selected.push_back({&*render->base_root, render->base_root->find('=') + 1});
    if (render->observations)
      selected.push_back({&*render->observations, 0});
  } else if (auto* load = std::get_if<engine::command_load_options>(&options)) {
    const auto named = engine::workspace_is_name(load->workspace);
    if (!named)
      return std::unexpected(named.error());
    std::optional<std::string> directory;
    if (*named) {
      auto captured = workspace_directory_reader();
      if (!captured)
        return std::unexpected(captured.error());
      directory = std::move(*captured);
    }
    auto workspace =
        engine::expand_workspace(load->workspace, directory ? std::optional<std::string_view>(*directory) : std::nullopt);
    if (!workspace)
      return std::unexpected(workspace.error());
    load->workspace = std::move(*workspace);
    selected.push_back({&load->workspace, 0, true});
    selected.push_back({&load->registry, 0, true});
    selected.push_back({&load->ledger, 0});
  } else if (auto* exporting = std::get_if<engine::command_export_options>(&options)) {
    selected.push_back({&exporting->ledger, 0});
    selected.push_back({&exporting->target, 0});
  } else if (auto* initializing = std::get_if<engine::command_init_options>(&options)) {
    selected.push_back({&initializing->ledger, 0});
  } else if (auto* activating = std::get_if<engine::command_activate_options>(&options)) {
    selected.push_back({&activating->ledger, 0});
  } else if (auto* recovery = std::get_if<engine::command_recovery_options>(&options)) {
    selected.push_back({&recovery->ledger, 0});
    if (recovery->observations)
      selected.push_back({&*recovery->observations, 0});
    if (recovery->provenance)
      selected.push_back({&*recovery->provenance, 0});
  } else if (auto* recover = std::get_if<engine::command_recover_options>(&options)) {
    selected.push_back({&recover->ledger, 0});
    if (recover->root)
      selected.push_back({&*recover->root, recover->root->find('=') + 1});
    if (recover->base_root)
      selected.push_back({&*recover->base_root, recover->base_root->find('=') + 1});
  } else if (auto* checking = std::get_if<engine::command_check_options>(&options)) {
    selected.push_back({&checking->ledger, 0});
    if (checking->operation == "check") {
      selected.push_back({&checking->root, 0});
      selected.push_back({&checking->base_root, 0});
      selected.push_back({&checking->scratch, 0});
    }
    if (checking->operation == "verdict")
      selected.push_back({&checking->verdict, 0});
    if (checking->provenance)
      selected.push_back({&*checking->provenance, 0});
  } else if (auto* running = std::get_if<engine::command_run_options>(&options)) {
    selected.push_back({&running->ledger, 0});
    if (running->provenance)
      selected.push_back({&*running->provenance, 0});
  } else if (auto* reading = std::get_if<engine::command_status_options>(&options)) {
    selected.push_back({&reading->ledger, 0});
    if (reading->workspace && reading->registry) {
      // Draft workspace and registry follow the load rules: a bare name expands, both become lexical intents.
      const auto named = engine::workspace_is_name(*reading->workspace);
      if (!named)
        return std::unexpected(named.error());
      std::optional<std::string> directory;
      if (*named) {
        auto captured = workspace_directory_reader();
        if (!captured)
          return std::unexpected(captured.error());
        directory = std::move(*captured);
      }
      auto workspace =
          engine::expand_workspace(*reading->workspace, directory ? std::optional<std::string_view>(*directory) : std::nullopt);
      if (!workspace)
        return std::unexpected(workspace.error());
      *reading->workspace = std::move(*workspace);
      selected.push_back({&*reading->workspace, 0, true});
      selected.push_back({&*reading->registry, 0, true});
    }
  } else if (auto* previewing = std::get_if<engine::command_preview_options>(&options)) {
    selected.push_back({&previewing->ledger, 0});
  } else {
    selected.push_back({&std::get<engine::command_history_options>(options).ledger, 0});
  }
  bool needs_cwd = false;
  for (const auto& path : selected) {
    const auto argument = std::string_view(*path.target).substr(path.prefix);
    if (auto checked = engine::validate_host_path(argument); !checked)
      return std::unexpected(checked.error());
    needs_cwd = needs_cwd || !argument.starts_with('/');
  }
  std::optional<std::string> cwd;
  if (needs_cwd) {
    auto captured = cwd_reader();
    if (!captured)
      return std::unexpected(engine::diagnostic{"io", "Cannot obtain invocation cwd"});
    cwd = std::move(*captured);
  }
  for (const auto& path : selected) {
    auto prepared = engine::prepare_host_path(std::string_view(*path.target).substr(path.prefix),
                                              cwd ? std::optional<std::string_view>(*cwd) : std::nullopt);
    if (!prepared)
      return std::unexpected(prepared.error());
    // Only load workspace/registry use lexical targets for intent and capture.
    // Ledger traversal retains its original parent spelling like all ledger commands.
    path.target->replace(path.prefix, std::string::npos, path.lexical ? prepared->intent : prepared->spelling);
  }
  return {};
}
namespace {
// Normalize the profile selector lexically against the invocation's one cwd snapshot, without any existence check.
std::expected<std::string, engine::diagnostic>
normalize_profile_selector(std::string_view selector, invocation_snapshot& snapshot, const snapshot_reader& cwd_reader) {
  if (auto checked = engine::validate_host_path(selector); !checked)
    return std::unexpected(checked.error());
  std::optional<std::string_view> cwd;
  if (!selector.starts_with('/')) {
    if (!snapshot.cwd) {
      auto captured = cwd_reader();
      if (!captured)
        return std::unexpected(engine::diagnostic{"io", "Cannot obtain invocation cwd"});
      snapshot.cwd = std::move(*captured);
    }
    cwd = *snapshot.cwd;
  }
  auto prepared = engine::prepare_host_path(selector, cwd);
  if (!prepared)
    return std::unexpected(prepared.error());
  return std::move(prepared->intent);
}
} // namespace
std::expected<std::string, engine::diagnostic>
prepare_profile_invocation(std::string_view selector, engine::command_options& options, invocation_snapshot& snapshot,
                           const std::function<std::expected<std::string, engine::diagnostic>()>& cwd_reader,
                           const std::function<std::expected<std::string, engine::diagnostic>()>& workspace_directory_reader) {
  if (auto checked = engine::validate_host_path(selector); !checked)
    return std::unexpected(checked.error());
  // Each reader runs at most once; later slots, including the selector, reuse the snapshot.
  const snapshot_reader cwd_once = [&]() -> std::expected<std::string, engine::diagnostic> {
    if (snapshot.cwd)
      return *snapshot.cwd;
    auto captured = cwd_reader();
    if (!captured)
      return std::unexpected(captured.error());
    snapshot.cwd = *captured;
    return std::move(*captured);
  };
  const snapshot_reader workspace_once = [&]() -> std::expected<std::string, engine::diagnostic> {
    if (snapshot.workspace_directory)
      return *snapshot.workspace_directory;
    auto captured = workspace_directory_reader();
    if (!captured)
      return std::unexpected(captured.error());
    snapshot.workspace_directory = *captured;
    return std::move(*captured);
  };
  if (auto prepared = prepare_invocation_paths(options, cwd_once, workspace_once); !prepared)
    return std::unexpected(prepared.error());
  return normalize_profile_selector(selector, snapshot, cwd_once);
}
std::expected<std::string, engine::diagnostic> prepare_process_profile_selector(std::string_view     selector,
                                                                                invocation_snapshot& snapshot) {
  return normalize_profile_selector(selector, snapshot, process_cwd);
}
std::expected<void, engine::diagnostic> complete_process_snapshot(invocation_snapshot& snapshot) {
  if (!snapshot.cwd) {
    auto captured = process_cwd();
    if (!captured)
      return std::unexpected(captured.error());
    snapshot.cwd = std::move(*captured);
  }
  if (!snapshot.workspace_directory && process_workspace_environment()) {
    // An unusable environment value is left absent: a later bare workspace name then refuses as usage.
    if (auto captured = process_workspace_directory())
      snapshot.workspace_directory = std::move(*captured);
  }
  return {};
}
namespace {
// Profile load always runs on the one real runtime that owns the ledger's prepared_profile type, even in generated variants.
namespace rt_api = ::planar::engine;
// X07-ANCHOR:response-ceiling
constexpr std::size_t response_ceiling = 16777215;
} // namespace
namespace {
// Commit only immutable profile preparation; the retained VM is dormant and no Lua runs under the write lock.
value_ptr complete_profile_format3_load(std::string_view path, const rt_api::prepared_profile& candidate,
                                        const engine::ledger::execution_snapshot& snapshot, std::string_view workspace,
                                        engine::ledger::completion_receipt& completion) {
  auto data = candidate.data();
  if (!data || !(*data)->bound_intent || !(*data)->bound_intent->version2 || !(*data)->execution_change)
    return native_error({"internal", "Invalid native load preparation"});
  const auto& input          = **data;
  auto        bound_snapshot = execution::snapshot_value(snapshot);
  if (!bound_snapshot || canonical_text(*bound_snapshot) != canonical_text(input.execution_snapshot))
    return native_error({"internal", "Invalid native load preparation"});
  if (input.workspace_id != workspace)
    return ledger_error({ledger::format_error_code::invalid_argument, ""});
  ledger::execution_prepared_revision records{{input.source, history::tree_profile::source},
                                              {input.registry.files, history::tree_profile::provenance},
                                              {input.effective.files, history::tree_profile::effective_provenance},
                                              input.interpretation.encoded,
                                              input.interpretation.versions_encoded};
  ledger::execution_revision_input    revision{};
  bool                                noop = false;
  if (const auto& active = snapshot.revision) {
    const auto registry_digest = active->registry_value && active->registry_value->fields.contains("digest") &&
                                         active->registry_value->fields.at("digest")->kind == value_kind::string
                                     ? std::optional<std::string>(active->registry_value->fields.at("digest")->string)
                                     : std::nullopt;
    noop = tree_digest(records.source.files, records.source.profile) == std::optional<std::string>(active->source_digest) &&
           tree_digest(records.stdlib.files, records.stdlib.profile) == std::optional<std::string>(active->stdlib_digest) &&
           registry_digest && tree_digest(records.registry.files, records.registry.profile) == registry_digest &&
           canonical_text(active->interpretation_value) == std::optional<std::string>(records.interpretation) &&
           canonical_text(active->versions_value) == std::optional<std::string>(records.versions);
    if (noop)
      revision = {active->source, active->registry, active->stdlib, active->interpretation, active->versions, true};
  }
  execution::load_options typed{*input.bound_intent->version2, revision, std::nullopt};
  if (!noop)
    typed.prepared = std::move(records);
  execution::change_request request{input.bound_intent->request_id, input.bound_intent->fields.actor, std::nullopt,
                                    std::move(typed), nullptr};
  auto                      change = execution::decode_execution_change(input.execution_change, snapshot, request);
  if (!change)
    return native_error({"internal", "Invalid embedded command protocol"});
  auto staged = ledger::stage_execution(std::filesystem::path(path), snapshot, *change);
  if (!staged)
    return execution_error_value(staged.error());
  if (staged->result().replayed) {
    completion.saved_result = staged->result().saved_result;
    completion.phase        = ledger::completion_phase::replayed;
    return {};
  }
  if (auto verified = candidate.verify_native_receipts(); !verified) {
    staged->abort();
    return native_error(verified.error());
  }
  completion.saved_result = staged->result().saved_result;
  completion.phase        = ledger::completion_phase::commit_uncertain;
  if (auto committed = staged->commit(); !committed) {
    completion.phase = ledger::completion_phase::not_completed;
    completion.saved_result.clear();
    return execution_error_value(committed.error());
  }
  completion.phase = ledger::completion_phase::committed;
  return {};
}
} // namespace
response execute_profile_load(const engine::acceptance::request_intent& intent, std::string_view ledger_path) {
  using namespace detail;
  response                           fallback{4, "", ""};
  std::string                        stdout_error;
  engine::ledger::completion_receipt completion;
  const auto committed = [&]() { return completion.phase != engine::ledger::completion_phase::not_completed; };
  try {
    fallback     = failed(load_uncertainty_error("load"));
    stdout_error = failed(load_uncertainty_error("stdout")).error;
    if (fallback.error.empty() || stdout_error.empty())
      return {4, "", ""};
    auto data = intent.data();
    if (!data)
      return failed(native_error({"internal", "Missing native load intent"}));
    const auto&                                       request = **data;
    std::string                                       ledger_identity, workspace_identity;
    bool                                              format3 = false;
    std::optional<engine::ledger::execution_snapshot> execution_snapshot;
    value_ptr                                         encoded_snapshot;
    {
      auto ledger = engine::ledger::open_existing(std::filesystem::path(std::string(ledger_path)),
                                                  engine::ledger::ledger_access::read_only);
      if (!ledger)
        return failed(ledger_error(ledger.error()));
      ledger_identity    = ledger->metadata().identity;
      workspace_identity = ledger->metadata().workspace_id;
      format3            = ledger->metadata().format == engine::ledger::ledger_format::v3;
      if (!format3) {
        auto saved = ledger->read_request(request.request_id);
        if (!saved)
          return failed(ledger_error(saved.error()));
        if (*saved) {
          if ((*saved)->body_encoded != request.encoded)
            return failed(ledger_error({engine::ledger::format_error_code::request_conflict, ""}));
          return {0, (*saved)->result + "\n", "", std::move(stdout_error)};
        }
        if (ledger->metadata().format != engine::ledger::ledger_format::v2)
          return failed(ledger_error({engine::ledger::format_error_code::unsupported_version, ""}));
      }
    } // Release the read transaction before capture or writable acceptance.
    if (format3) {
      if (!request.version2)
        return failed(native_error({"internal", "Missing native load intent"}));
      auto saved = engine::ledger::lookup_execution_request(std::filesystem::path(ledger_path), request.request_id,
                                                            engine::ledger::execution_framing{*request.version2});
      if (!saved)
        return failed(execution_error_value(saved.error()));
      if (saved->state == engine::ledger::request_state::conflict)
        return failed(ledger_error({engine::ledger::format_error_code::request_conflict, ""}));
      if (saved->state == engine::ledger::request_state::replay)
        return {0, saved->result + "\n", "", std::move(stdout_error)};
      auto snapshot = engine::ledger::read_execution_snapshot(std::filesystem::path(ledger_path));
      if (!snapshot)
        return failed(execution_error_value(snapshot.error()));
      if (snapshot->identity != ledger_identity)
        return failed(execution_error_value(execution_failure(engine::ledger::execution_error_code::path_denied)));
      auto encoded = engine::execution::snapshot_value(*snapshot);
      if (!encoded)
        return failed(native_error({"internal", "Invalid embedded command protocol"}));
      encoded_snapshot   = *encoded;
      execution_snapshot = std::move(*snapshot);
    }
    // X07-ANCHOR:after-early-miss
    auto opened = rt_api::profile_invocation::open_with_profile(intent, ledger_path);
    if (!opened)
      return registration_failed(opened.error().detail);
    // X07-ANCHOR:after-factory-open
    // The step-4 intent object is passed unchanged: preparation refuses any other intent.
    auto prepared =
        format3 ? std::move(*opened).prepare_execution_load(intent, encoded_snapshot) : std::move(*opened).prepare_load(intent);
    if (!prepared) {
      if (prepared.error().semantic)
        return semantic_failed(prepared.error().semantic);
      return registration_failed(prepared.error().failure.detail);
    }
    // X07-ANCHOR:after-prepare-load
    // Exact relookup under the lock, the stale-head check and the final receipt verification run inside the commit.
    if (format3) {
      if (auto refused =
              complete_profile_format3_load(ledger_path, *prepared, *execution_snapshot, workspace_identity, completion))
        return committed() ? std::move(fallback) : failed(refused);
    } else {
      auto sealed = engine::ledger::complete_acceptance(std::filesystem::path(std::string(ledger_path)), intent, *prepared,
                                                        ledger_identity, completion);
      if (!sealed) {
        if (committed())
          return fallback;
        return failed(ledger_error(sealed.error()));
      }
    }
    // X07-ANCHOR:after-commit
    if (completion.phase != engine::ledger::completion_phase::committed &&
        completion.phase != engine::ledger::completion_phase::replayed)
      return committed() ? std::move(fallback) : failed(native_error({"internal", "Missing load completion"}));
    // Encoding the saved response is a post-commit step: an oversize or malformed record keeps the completion receipt.
    if (completion.saved_result.size() > response_ceiling)
      return fallback;
    auto saved_value = engine::codec_decode(completion.saved_result);
    if (!saved_value)
      return fallback;
    return {0, completion.saved_result + "\n", "", std::move(stdout_error)};
  } catch (...) {
    if (committed())
      return fallback;
    return {4, "", ""};
  }
}
#ifdef PLANAR_AUTHORING_BUNDLE
response execute_authoring(engine::command_authoring_options options, engine::command_vm* registered) {
  using namespace detail;
  // Both ceilings of RA-3 and RA-6: checked before the generic 16 MiB output ceiling, each with its own limit name.
  constexpr std::size_t prompt_ceiling = 1048576;
  try {
    std::optional<engine::command_vm> local;
    engine::command_vm*               vm = registered;
    if (!vm) {
      auto created = engine::command_vm::create();
      if (!created)
        return failed(native_error(created.error()));
      local.emplace(std::move(*created));
      vm = &*local;
    }
    const bool observing = options.verb == engine::command_authoring_verb::observe;
    auto       services  = std::make_unique<native_services>();
    auto       result    = vm->invoke_authoring(std::move(options), *services);
    if (!result)
      return failed(native_error(result.error()));
    if (!(*result)->fields.at("ok")->boolean)
      return failed((*result)->fields.at("error"));
    auto envelope = (*result)->fields.at("value");
    if (!observing && envelope->fields.at("prompt")->fields.at("markdown")->string.size() > prompt_ceiling)
      return failed(native_error(capture_limit("prompt_bytes", prompt_ceiling)));
    auto encoded = engine::codec_encode(envelope, {response_ceiling, 256, 4000000});
    if (!encoded) {
      auto error = encoded.error().detail;
      if (error.code == "resource_limit" && error.limit == "output_bytes")
        error.maximum = response_ceiling + 1;
      return failed(native_error(error));
    }
    if (observing && encoded->size() > prompt_ceiling)
      return failed(native_error(capture_limit("observation_bytes", prompt_ceiling)));
    return {0, *encoded + "\n", ""};
  } catch (...) {
    return {4, "", ""};
  }
}
#endif
response native_export_failure(const engine::source_export::export_error& error) {
  auto result = detail::failed(export_error(error));
  if (error.code == engine::source_export::error_code::cleanup_incomplete)
    result.status = 4;
  return result;
}
namespace {
/// Outcome of the format3 load prelude: whether the ledger is format3, and the typed refusal when one applies.
struct format3_selection {
  bool      selected = false; ///< True when the ledger is format3 and the execution path owns the load.
  value_ptr refusal;          ///< Public error value; null when the load continues.
};
/// Format3 load prelude (EX-3 step 1, DD1): verify the format3 ledger, frame request_v2 and answer exact replay through
/// the execution lookup. Format1 and format2 ledgers, and every refusal the existing open reports, keep the existing path.
format3_selection format3_load_prelude(const engine::command_load_options& load, native_services& services,
                                       engine::ledger::completion_receipt& receipt) {
  format3_selection out;
  {
    // The storage reader opens format3 read-only since the cutover (DD35); format1 and format2 keep the existing path.
    auto legacy = engine::ledger::open_existing(load.ledger, engine::ledger::ledger_access::read_only);
    if (legacy ? legacy->metadata().format != engine::ledger::ledger_format::v3
               : legacy.error().code != engine::ledger::format_error_code::unsupported_version)
      return out;
  }
  std::string identity, workspace;
  {
    auto opened = engine::ledger::open_format3(load.ledger, engine::ledger::execution_access::read_only);
    if (!opened) {
      if (opened.error().code == engine::ledger::execution_error_code::unsupported_ledger_version)
        return out;
      out.selected = true;
      out.refusal  = execution_error_value(opened.error());
      return out;
    }
    identity  = opened->metadata().identity;
    workspace = opened->metadata().workspace_id;
  } // The verified read connection is released before the lookup opens its own.
  out.selected = true;
  auto framed  = engine::acceptance::request_intent::create_v2(load.request_id, load.actor, load.expected_head, load.workspace,
                                                               load.registry);
  if (!framed) {
    out.refusal = framed.error().code == engine::acceptance::preparation_error_code::resource_limit
                      ? detail::native_error({"resource_limit", "Load request exceeds native resource limits"})
                      : detail::native_error({"schema", "Invalid load arguments"});
    return out;
  }
  services.load_intent.emplace(std::move(*framed));
  auto data = services.load_intent->data();
  if (!data || !(*data)->version2) {
    out.refusal = detail::native_error({"internal", "Missing native load intent"});
    return out;
  }
  auto found = engine::ledger::lookup_execution_request(load.ledger, load.request_id,
                                                        engine::ledger::execution_framing{*(*data)->version2});
  if (!found) {
    out.refusal = execution_error_value(found.error());
    return out;
  }
  if (found->state == engine::ledger::request_state::conflict) {
    out.refusal = execution_error_value(execution_failure(engine::ledger::execution_error_code::request_conflict));
    return out;
  }
  if (found->state == engine::ledger::request_state::replay) {
    receipt.saved_result = std::move(found->result);
    receipt.phase        = engine::ledger::completion_phase::replayed;
  }
  services.load_ledger_identity = std::move(identity);
  services.execution_workspace  = std::move(workspace);
  services.load_format3         = true;
  return out;
}
/// Activate prelude, refusal steps (2) and (3) of P2-1: format3 only, then exact replay of the request_v3 framing.
/// @return Public refusal, or null when the invocation continues (the receipt is filled for a replay).
value_ptr activate_prelude(const engine::command_activate_options& options, native_services& services,
                           engine::ledger::completion_receipt& receipt) {
  auto framing = engine::execution::build_request_v3(
      options.actor, engine::execution::activate_options{options.job, options.expect_revision, options.expect_job},
      services.execution_profile);
  if (!framing)
    return detail::native_error({"internal", "Invalid embedded command protocol"});
  auto found = engine::ledger::lookup_execution_request(options.ledger, options.request_id,
                                                        engine::ledger::execution_framing{std::move(*framing)});
  if (!found)
    return execution_error_value(found.error());
  if (found->state == engine::ledger::request_state::conflict)
    return execution_error_value(execution_failure(engine::ledger::execution_error_code::request_conflict));
  if (found->state == engine::ledger::request_state::replay) {
    receipt.saved_result = std::move(found->result);
    receipt.phase        = engine::ledger::completion_phase::replayed;
    return nullptr;
  }
  auto opened = engine::ledger::open_format3(options.ledger, engine::ledger::execution_access::read_only);
  if (!opened)
    return execution_error_value(opened.error());
  services.activate_identity = opened->metadata().identity;
  return nullptr;
}
value_ptr check_prelude(const engine::command_check_options& o, native_services& services, ledger::completion_receipt& receipt,
                        std::optional<std::string> profile = {}) {
  std::map<std::string, value_ptr> parameters;
  if (o.operation == "check-recover")
    parameters = {{"run_id", text(o.run_id)}, {"reservation", text(o.reservation)}, {"confirm_stopped", text(o.confirm_stopped)}};
  else {
    auto token = ledger::parse_run_token(o.run);
    if (!token)
      return native_error({"usage", "Invalid command syntax"});
    parameters["run_id"] = text(std::to_string(token->run_id));
    if (o.operation == "check") {
      parameters["root"]      = text(o.root);
      parameters["base_root"] = text(o.base_root);
      parameters["scratch"]   = text(o.scratch);
      if (o.provenance)
        parameters["provenance_path"] = text(*o.provenance);
    } else
      parameters["verdict_path"] = text(o.verdict);
  }
  auto encoded = canonical_encode(object(std::move(parameters)));
  if (!encoded)
    return native_error({"internal", "Invalid embedded command protocol"});
  services.check_request = history::request_v3{o.operation, o.actor, *encoded, std::move(profile)};
  auto found = ledger::lookup_execution_request(o.ledger, o.request_id, ledger::execution_framing{*services.check_request});
  if (!found)
    return execution_error_value(found.error());
  if (found->state == ledger::request_state::conflict)
    return execution_error_value(execution_failure(ledger::execution_error_code::request_conflict));
  if (found->state == ledger::request_state::replay) {
    receipt.saved_result = found->result;
    receipt.phase        = ledger::completion_phase::replayed;
    return {};
  }
  auto opened = ledger::open_format3(o.ledger, ledger::execution_access::read_only);
  if (!opened)
    return execution_error_value(opened.error());
  services.check_identity = opened->metadata().identity;
  return {};
}
value_ptr run_prelude(const engine::command_run_options& options, engine::ledger::completion_receipt& receipt,
                      const std::optional<std::string>& profile = {}) {
  execution::operation_options op = execution::claim_options{options.job, options.role, options.provenance};
  if (options.run_id)
    op = execution::takeover_options{options.job, options.role, std::stoull(*options.run_id), options.provenance};
  auto framing = execution::build_request_v3(options.actor, op, profile);
  if (!framing)
    return native_error({"internal", "Invalid embedded command protocol"});
  auto found = ledger::lookup_execution_request(options.ledger, options.request_id, ledger::execution_framing{*framing});
  if (!found)
    return execution_error_value(found.error());
  if (found->state == ledger::request_state::conflict)
    return execution_error_value(execution_failure(ledger::execution_error_code::request_conflict));
  if (found->state == ledger::request_state::replay) {
    auto saved = engine::codec_decode(found->result);
    if (!saved)
      return native_error({"internal", "Invalid embedded command protocol"});
    auto assembled = assemble_run_result(options.ledger, *saved);
    if (!assembled)
      return execution_error_value(assembled.error());
    auto bytes = engine::codec_encode(*assembled);
    if (!bytes)
      return native_error({"internal", "Invalid embedded command protocol"});
    receipt.saved_result = *bytes;
    receipt.phase        = ledger::completion_phase::replayed;
  }
  return nullptr;
}
value_ptr recovery_prelude(const engine::command_recovery_options& options, ledger::completion_receipt& receipt,
                           const std::optional<std::string>& profile = {}) {
  const execution::recovery_options op{
      options.operation, "", "", options.run, options.expect_fp, options.reason, options.observations, options.provenance};
  auto framing = execution::build_request_v3(options.actor, op, profile);
  if (!framing)
    return native_error({"internal", "Invalid embedded command protocol"});
  auto found = ledger::lookup_execution_request(options.ledger, options.request_id, ledger::execution_framing{*framing});
  if (!found)
    return execution_error_value(found.error());
  if (found->state == ledger::request_state::conflict)
    return execution_error_value(execution_failure(ledger::execution_error_code::request_conflict));
  if (found->state == ledger::request_state::replay) {
    receipt.saved_result = found->result;
    if (options.operation == "refresh") {
      auto saved = engine::codec_decode(found->result);
      if (!saved)
        return native_error({"internal", "Invalid embedded command protocol"});
      auto assembled = assemble_run_result(options.ledger, *saved);
      if (!assembled)
        return execution_error_value(assembled.error());
      auto bytes = engine::codec_encode(*assembled);
      if (!bytes)
        return native_error({"internal", "Invalid embedded command protocol"});
      receipt.saved_result = *bytes;
    }
    receipt.phase = ledger::completion_phase::replayed;
  }
  return nullptr;
}
/// Read prelude of status and preview, refusal step (2) of P2-1: a format1 or format2 ledger is
/// `unsupported_ledger_version` for every form before any service, capture or Lua. Exempt reads never replay.
/// @return Public refusal, or null when the invocation continues; the verified identity is retained for the read.
value_ptr execution_read_prelude(const engine::command_options& options, native_services& services) {
  const std::string* ledger = nullptr;
  if (const auto* status = std::get_if<engine::command_status_options>(&options))
    ledger = &status->ledger;
  else if (const auto* preview = std::get_if<engine::command_preview_options>(&options))
    ledger = &preview->ledger;
  else if (const auto* run = std::get_if<engine::command_run_options>(&options))
    ledger = &run->ledger;
  if (const auto* o = std::get_if<engine::command_recovery_options>(&options))
    ledger = &o->ledger;
  if (const auto* o = std::get_if<engine::command_recover_options>(&options))
    ledger = &o->ledger;
  if (!ledger)
    return nullptr;
  auto opened = engine::ledger::open_format3(*ledger, engine::ledger::execution_access::read_only);
  if (!opened)
    return execution_error_value(opened.error());
  services.read_identity = opened->metadata().identity;
  return nullptr;
}
} // namespace
response execute(engine::command_options options, engine::command_vm* registered) {
  using namespace detail;
  response                           fallback{4, "", ""};
  std::string                        stdout_error;
  std::optional<engine::command_vm>  local;
  engine::command_vm*                vm = registered;
  std::unique_ptr<native_services>   services;
  engine::ledger::completion_receipt early_completion;
  auto                               published = [&]() {
    return (services &&
            (services->export_published ||
             (services->load_receipt && services->load_receipt->phase != engine::ledger::completion_phase::not_completed) ||
             services->activate_committed || services->check_committed)) ||
           (vm && (vm->export_may_be_complete() || vm->load_may_be_complete())) ||
           early_completion.phase != engine::ledger::completion_phase::not_completed;
  };
  try {
    if (!vm) {
      auto created = engine::command_vm::create();
      if (!created)
        return failed(native_error(created.error()));
      local.emplace(std::move(*created));
      vm = &*local;
    }
    auto prepared = prepare_invocation_paths(options, process_cwd, process_workspace_directory);
    if (!prepared)
      return failed(native_error(prepared.error()));
    const bool exporting = std::holds_alternative<engine::command_export_options>(options);
    if (exporting) {
      fallback     = failed(uncertainty_error("export"));
      stdout_error = failed(uncertainty_error("stdout")).error;
      if (fallback.error.empty() || stdout_error.empty())
        return {4, "", ""};
    }
    const bool loading = std::holds_alternative<engine::command_load_options>(options);
    if (loading) {
      fallback     = failed(load_uncertainty_error("load"));
      stdout_error = failed(load_uncertainty_error("stdout")).error;
      if (fallback.error.empty() || stdout_error.empty())
        return {4, "", ""};
    }
    if (auto mutation = execution_mutation_name(options); !mutation.empty()) {
      fallback     = failed(execution_uncertainty_error(mutation, mutation));
      stdout_error = failed(execution_uncertainty_error(mutation, "stdout")).error;
      if (fallback.error.empty() || stdout_error.empty())
        return {4, "", ""};
    }
    services = std::make_unique<native_services>();
    if (auto refusal = execution_read_prelude(options, *services))
      return failed(refusal);
    if (const auto* check = std::get_if<engine::command_check_options>(&options)) {
      fallback     = failed(activate_uncertainty_error(check->operation));
      stdout_error = failed(activate_uncertainty_error("stdout")).error;
      if (auto refusal = check_prelude(*check, *services, early_completion))
        return failed(refusal);
      if (early_completion.phase == ledger::completion_phase::replayed)
        return {0, early_completion.saved_result + "\n", "", stdout_error};
    }
    if (const auto* run = std::get_if<engine::command_run_options>(&options)) {
      if (auto refusal = run_prelude(*run, early_completion))
        return failed(refusal);
      if (early_completion.phase == ledger::completion_phase::replayed)
        return {0, early_completion.saved_result + "\n", "", stdout_error};
    }
    if (const auto* recovery = std::get_if<engine::command_recovery_options>(&options)) {
      if (auto refusal = recovery_prelude(*recovery, early_completion))
        return failed(refusal);
      if (early_completion.phase == ledger::completion_phase::replayed)
        return {0, early_completion.saved_result + "\n", "", stdout_error};
    }
    const bool activating = std::holds_alternative<engine::command_activate_options>(options);
    if (activating) {
      fallback     = failed(activate_uncertainty_error("activate"));
      stdout_error = failed(activate_uncertainty_error("stdout")).error;
      if (fallback.error.empty() || stdout_error.empty())
        return {4, "", ""};
      if (auto refusal = activate_prelude(std::get<engine::command_activate_options>(options), *services, early_completion))
        return failed(refusal);
      if (early_completion.phase == engine::ledger::completion_phase::replayed)
        return {0, early_completion.saved_result + "\n", "", std::move(stdout_error)};
    }
    if (loading) {
      const auto& load = std::get<engine::command_load_options>(options);
      auto intent = engine::acceptance::request_intent::create(load.request_id, load.actor, load.expected_head, load.workspace,
                                                               load.registry);
      if (!intent) {
        if (intent.error().code == engine::acceptance::preparation_error_code::resource_limit)
          return failed(native_error({"resource_limit", "Load request exceeds native resource limits"}));
        return failed(native_error({"schema", "Invalid load arguments"}));
      }
      services->load_intent.emplace(std::move(*intent));
      // Format3 (DD1): the prelude selects the request_v2 framing and answers early replay through the execution lookup.
      const auto format3 = format3_load_prelude(load, *services, early_completion);
      if (format3.refusal)
        return failed(format3.refusal);
      if (!format3.selected) {
        auto ledger = engine::ledger::open_existing(load.ledger, engine::ledger::ledger_access::read_only);
        if (!ledger)
          return failed(ledger_error(ledger.error()));
        services->load_ledger_identity = ledger->metadata().identity;
        if (ledger->metadata().format == engine::ledger::ledger_format::v2) {
          // The ledger format, not the command, selects the request codec; the embedded profile stays absent.
          auto framed = engine::acceptance::request_intent::create_v2(load.request_id, load.actor, load.expected_head,
                                                                      load.workspace, load.registry);
          if (!framed) {
            if (framed.error().code == engine::acceptance::preparation_error_code::resource_limit)
              return failed(native_error({"resource_limit", "Load request exceeds native resource limits"}));
            return failed(native_error({"schema", "Invalid load arguments"}));
          }
          services->load_intent.emplace(std::move(*framed));
        }
        auto saved = ledger->read_request(load.request_id);
        if (!saved)
          return failed(ledger_error(saved.error()));
        if (*saved) {
          auto data = services->load_intent->data();
          if ((*saved)->body_encoded != (*data)->encoded)
            return failed(ledger_error({engine::ledger::format_error_code::request_conflict, ""}));
          early_completion.saved_result = std::move((*saved)->result);
          early_completion.phase        = engine::ledger::completion_phase::replayed;
        }
      } // Release the verified read transaction before capture or writable acceptance.
      if (early_completion.phase == engine::ledger::completion_phase::replayed)
        return {0, early_completion.saved_result + "\n", "", std::move(stdout_error)};
    }
    auto result = vm->invoke(std::move(options), *services);
    if (!result) {
      if (published())
        return fallback;
      return failed(native_error(result.error()));
    }
    if (!(*result)->fields.at("ok")->boolean) {
      if (published())
        return fallback;
      auto error    = (*result)->fields.at("error");
      auto response = failed(error);
      if (error->fields.at("code")->string == "export_incomplete")
        response.status = 4;
      return response;
    }
    if (activating) {
      if (!services->activate_committed)
        return published() ? std::move(fallback) : failed(native_error({"internal", "Missing load completion"}));
      return {0, services->activate_saved + "\n", "", std::move(stdout_error)};
    }
    if (loading) {
      const auto* completion = vm->load_completion();
      if (!completion || (completion->phase != engine::ledger::completion_phase::committed &&
                          completion->phase != engine::ledger::completion_phase::replayed))
        return published() ? std::move(fallback) : failed(native_error({"internal", "Missing load completion"}));
      return {0, completion->saved_result + "\n", "", std::move(stdout_error)};
    }
    auto envelope = (*result)->fields.at("value");
    auto encoded  = engine::codec_encode(envelope, {16777215, 256, 4000000});
    if (!encoded) {
      if (published())
        return fallback;
      auto error = encoded.error().detail;
      if (error.code == "resource_limit" && error.limit == "output_bytes")
        error.maximum = 16777216;
      return failed(native_error(error));
    }
    return {0, *encoded + "\n", "", std::move(stdout_error)};
  } catch (...) {
    if (published())
      return fallback;
    return {4, "", ""};
  }
}
response execute_with_profile(engine::command_options options, std::string_view profile_selector) {
  using namespace detail;
  response                         fallback{4, "", ""};
  std::string                      stdout_error;
  std::unique_ptr<native_services> services;
  const auto                       published = [&]() {
    return services && (services->export_published || services->activate_committed || services->check_committed);
  };
  try {
    invocation_snapshot snapshot;
    auto selector = prepare_profile_invocation(profile_selector, options, snapshot, process_cwd, process_workspace_directory);
    if (!selector)
      return failed(native_error(selector.error()));
    if (const auto* load = std::get_if<engine::command_load_options>(&options)) {
      // The one request_v2 intent framed here is the intent every later load step uses.
      auto intent = engine::acceptance::request_intent::create_v2(load->request_id, load->actor, load->expected_head,
                                                                  load->workspace, load->registry, *selector);
      if (!intent) {
        if (intent.error().code == engine::acceptance::preparation_error_code::resource_limit)
          return failed(native_error({"resource_limit", "Load request exceeds native resource limits"}));
        return failed(native_error({"schema", "Invalid load arguments"}));
      }
      return execute_profile_load(*intent, load->ledger);
    }
    if (std::holds_alternative<engine::command_export_options>(options)) {
      fallback     = failed(uncertainty_error("export"));
      stdout_error = failed(uncertainty_error("stdout")).error;
      if (fallback.error.empty() || stdout_error.empty())
        return {4, "", ""};
    }
    if (auto mutation = execution_mutation_name(options); !mutation.empty()) {
      fallback     = failed(execution_uncertainty_error(mutation, mutation));
      stdout_error = failed(execution_uncertainty_error(mutation, "stdout")).error;
      if (fallback.error.empty() || stdout_error.empty())
        return {4, "", ""};
    }
    services                    = std::make_unique<native_services>();
    services->execution_profile = *selector;
    // status and preview: the format refusal precedes the profile, as for activate.
    if (auto refusal = execution_read_prelude(options, *services))
      return failed(refusal);
    if (std::holds_alternative<engine::command_activate_options>(options)) {
      // Steps (2) and (3) run before the profile is read, as for a profile load.
      fallback     = failed(activate_uncertainty_error("activate"));
      stdout_error = failed(activate_uncertainty_error("stdout")).error;
      if (fallback.error.empty() || stdout_error.empty())
        return {4, "", ""};
      engine::ledger::completion_receipt replay;
      if (auto refusal = activate_prelude(std::get<engine::command_activate_options>(options), *services, replay))
        return failed(refusal);
      if (replay.phase == engine::ledger::completion_phase::replayed)
        return {0, replay.saved_result + "\n", "", std::move(stdout_error)};
    }
    if (const auto* check = std::get_if<engine::command_check_options>(&options)) {
      ledger::completion_receipt early_completion;
      fallback     = failed(activate_uncertainty_error(check->operation));
      stdout_error = failed(activate_uncertainty_error("stdout")).error;
      if (auto refusal = check_prelude(*check, *services, early_completion, services->execution_profile))
        return failed(refusal);
      if (early_completion.phase == ledger::completion_phase::replayed)
        return {0, early_completion.saved_result + "\n", "", stdout_error};
    }
    if (const auto* run = std::get_if<engine::command_run_options>(&options)) {
      ledger::completion_receipt replay;
      if (auto refusal = run_prelude(*run, replay, services->execution_profile))
        return failed(refusal);
      if (replay.phase == ledger::completion_phase::replayed)
        return {0, replay.saved_result + "\n", "", stdout_error};
    }
    if (const auto* recovery = std::get_if<engine::command_recovery_options>(&options)) {
      ledger::completion_receipt replay;
      if (auto refusal = recovery_prelude(*recovery, replay, services->execution_profile))
        return failed(refusal);
      if (replay.phase == ledger::completion_phase::replayed)
        return {0, replay.saved_result + "\n", "", stdout_error};
    }
    auto opened = engine::profile_invocation::open_with_profile(*selector, options);
    if (!opened)
      return registration_failed(opened.error().detail);
    // The owner dispatches exactly the options it admitted and is consumed by the call.
    auto result = std::move(*opened).run_known(options, *services);
    if (!result) {
      if (published())
        return fallback;
      return failed(native_error(result.error()));
    }
    if (!(*result)->fields.at("ok")->boolean) {
      if (published())
        return fallback;
      auto error    = (*result)->fields.at("error");
      auto response = failed(error);
      if (error->fields.at("code")->string == "export_incomplete")
        response.status = 4;
      return response;
    }
    auto encoded = engine::codec_encode((*result)->fields.at("value"), {response_ceiling, 256, 4000000});
    if (!encoded) {
      if (published())
        return fallback;
      auto error = encoded.error().detail;
      if (error.code == "resource_limit" && error.limit == "output_bytes")
        error.maximum = response_ceiling + 1;
      return failed(native_error(error));
    }
    return {0, *encoded + "\n", "", std::move(stdout_error)};
  } catch (...) {
    if (published())
      return fallback;
    return {4, "", ""};
  }
}
} // namespace planar::cmd
