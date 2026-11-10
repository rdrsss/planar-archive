module planar.cmd.execution.takeover;
import std;
import cli11;
import planar.cmd.invocation;
namespace planar::cmd::handlers {
namespace {
engine::command_run_options takeover_intent(const takeover_options& o) {
  return {o.ledger, o.job, o.role, o.request_id, o.actor, o.run_id, o.provenance};
}
} // namespace
CLI::App& attach_takeover(CLI::App& root, takeover_options& o) {
  auto& command = *root.add_subcommand("takeover");
  command.set_help_flag();
  command.allow_extras(false);
  command.fallthrough(false);
  const CLI::Validator value(
      [](std::string& token) { return token.empty() || token.starts_with("--") ? "Expected explicit value" : ""; }, "VALUE");
  command.add_option("--ledger", o.ledger)
      ->expected(1)
      ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
      ->check(value)
      ->required();
  command.add_option("--job", o.job)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(value)->required();
  command.add_option("--role", o.role)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(value)->required();
  command.add_option("--request-id", o.request_id)
      ->expected(1)
      ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
      ->check(value)
      ->required();
  command.add_option("--actor", o.actor)
      ->expected(1)
      ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
      ->check(value)
      ->required();
  command.add_option("--run-id", o.run_id)
      ->expected(1)
      ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
      ->check(value)
      ->required();
  command.add_option("--provenance", o.provenance)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(value);
  command.get_option("--job")->check(CLI::Validator(
      [](std::string& t) {
        return !t.empty() && t.size() <= 64 && t.front() >= 'a' && t.front() <= 'z' &&
                       std::ranges::all_of(t, [](char c) { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'; })
                   ? ""
                   : "Expected identifier";
      },
      "ID"));
  command.get_option("--role")->check(CLI::Validator(
      [](std::string& t) {
        return !t.empty() && t.size() <= 64 && t.front() >= 'a' && t.front() <= 'z' &&
                       std::ranges::all_of(t, [](char c) { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'; })
                   ? ""
                   : "Expected identifier";
      },
      "ID"));
  command.get_option("--request-id")
      ->check(CLI::Validator(
          [](std::string& t) {
            return !t.empty() && t.size() <= 128 &&
                           std::ranges::all_of(t,
                                               [](unsigned char c) {
                                                 return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                                                        (c >= '0' && c <= '9') || c == '_' || c == '.' || c == ':' || c == '-';
                                               })
                       ? ""
                       : "Expected request key";
          },
          "KEY"));
  command.get_option("--actor")->check(CLI::Validator(
      [](std::string& t) { return !t.empty() && t.size() <= 256 && engine::valid_text(t) ? "" : "Expected UTF-8 actor"; },
      "ACTOR"));
  command.get_option("--run-id")
      ->check(CLI::Validator(
          [](std::string& t) {
            std::int64_t n = 0;
            auto [end, e]  = std::from_chars(t.data(), t.data() + t.size(), n);
            return !t.empty() && t.front() >= '1' && t.front() <= '9' && e == std::errc{} && end == t.data() + t.size() && n > 0
                       ? ""
                       : "Expected canonical positive ID";
          },
          "ID"));
  return command;
}
response takeover_command(const takeover_options& o, engine::command_vm* vm) {
  return execute(takeover_intent(o), vm);
}
response takeover_with_profile(const takeover_options& o, std::string_view selector) {
  return execute_with_profile(takeover_intent(o), selector);
}
} // namespace planar::cmd::handlers
