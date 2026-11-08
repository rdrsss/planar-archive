module planar.cmd.planar.handlers.history;
import std;
import cli11;
import planar.cmd.invocation;
namespace planar::cmd::handlers {
namespace {
std::optional<std::uint64_t> positive(std::string_view s) {
  if (s.empty() || s.front() < '1' || s.front() > '9')
    return {};
  std::int64_t n = 0;
  auto [p, e]    = std::from_chars(s.data(), s.data() + s.size(), n);
  if (e != std::errc{} || p != s.data() + s.size() || n <= 0)
    return {};
  return static_cast<std::uint64_t>(n);
}
} // namespace
CLI::App& attach_history(CLI::App& root, history_options& o) {
  auto& c = *root.add_subcommand("history");
  c.set_help_flag();
  c.allow_extras(false);
  c.fallthrough(false);
  const CLI::Validator explicit_value(
      [](std::string& s) { return s.empty() || s.starts_with("--") ? "Expected explicit value" : ""; }, "VALUE");
  const CLI::Validator id_value([](std::string& s) { return positive(s) ? "" : "Expected positive decimal ID"; }, "ID");
  const CLI::Validator limit_value(
      [](std::string& s) {
        auto n = positive(s);
        return n && *n <= 100 ? "" : "Expected count 1..100";
      },
      "COUNT");
  const CLI::Validator key_value(
      [](std::string& s) {
        return !s.empty() && s.size() <= 128 &&
                       std::ranges::all_of(s,
                                           [](unsigned char c) {
                                             return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
                                                    c == '_' || c == '.' || c == ':' || c == '-';
                                           })
                   ? ""
                   : "Expected ASCII request key";
      },
      "KEY");
  auto add = [&](const char* name, std::string& value, const CLI::Validator& validator) {
    return c.add_option(name, value)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(validator);
  };
  add("--ledger", o.ledger, explicit_value)->required();
  auto decimal = [&](const char* name, std::int64_t& number, std::string& token, const CLI::Validator& validator) {
    return c.add_option(name, number)
        ->expected(1)
        ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
        ->check(validator)
        ->each([&token](const std::string& value) { token = value; });
  };
  auto r = decimal("--revision", o.revision_number, o.revision, id_value), k = add("--request-id", o.request_id, key_value),
       e      = decimal("--at-event", o.at_event_number, o.at_event, id_value),
       a      = decimal("--after-event", o.after_event_number, o.after_event, id_value),
       l      = decimal("--limit", o.limit_number, o.limit, limit_value);
  auto detail = decimal("--event", o.event_number, o.event, id_value);
  detail->excludes(r)->excludes(k)->excludes(e)->excludes(a)->excludes(l);
  r->excludes(k)->excludes(e)->excludes(a)->excludes(l);
  k->excludes(e)->excludes(a)->excludes(l);
  e->excludes(a)->excludes(l);
  return c;
}
engine::command_options history_intent(const history_options& o) {
  engine::command_history_options intent;
  intent.ledger = o.ledger;
  if (!o.event.empty())
    intent.event = o.event_number;
  if (!o.revision.empty())
    intent.revision = o.revision_number;
  if (!o.at_event.empty())
    intent.at_event = o.at_event_number;
  if (!o.after_event.empty())
    intent.after_event = o.after_event_number;
  if (!o.request_id.empty())
    intent.request_id = o.request_id;
  if (!o.limit.empty())
    intent.limit = static_cast<std::int32_t>(o.limit_number);
  return intent;
}
response history_command(const history_options& o, engine::command_vm* vm) {
  return execute(history_intent(o), vm);
}
} // namespace planar::cmd::handlers
