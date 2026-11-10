module planar.cmd.execution.claim;
import std;
import cli11;
import planar.cmd.invocation;
namespace planar::cmd::handlers {
namespace {
engine::command_run_options claim_intent(const claim_options& o) {
  return {o.ledger, o.job, o.role, o.request_id, o.actor, o.run_id, o.provenance};
}
} // namespace
CLI::App& attach_claim(CLI::App& root, claim_options& o) {
  auto& command = *root.add_subcommand("claim");
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
  return command;
}
response claim_command(const claim_options& o, engine::command_vm* vm) {
  return execute(claim_intent(o), vm);
}
response claim_with_profile(const claim_options& o, std::string_view selector) {
  return execute_with_profile(claim_intent(o), selector);
}
} // namespace planar::cmd::handlers
