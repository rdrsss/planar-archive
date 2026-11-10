module planar.cmd.execution.refresh;
import std;
import cli11;
import planar.cmd.invocation;
import planar.engine.ledger.execution;
namespace planar::cmd::handlers {
namespace {
engine::command_recovery_options refresh_intent(const refresh_options& o) {
  return {o.operation, o.ledger, o.run, o.request_id, o.actor, o.expect_fp, o.reason, o.observations, o.provenance};
}
} // namespace
CLI::App& attach_refresh(CLI::App& root, refresh_options& o) {
  o.operation   = "refresh";
  auto& command = *root.add_subcommand("refresh");
  command.set_help_flag();
  command.allow_extras(false);
  command.fallthrough(false);
  const CLI::Validator value([](std::string& t) { return t.empty() || t.starts_with("--") ? "Expected explicit value" : ""; },
                             "VALUE");
  auto*                ledger       = command.add_option("--ledger", o.ledger)
                                          ->expected(1)
                                          ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
                                          ->check(value)
                                          ->required();
  auto*                run          = command.add_option("--run", o.run)
                                          ->expected(1)
                                          ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
                                          ->check(value)
                                          ->required();
  auto*                request_id   = command.add_option("--request-id", o.request_id)
                                          ->expected(1)
                                          ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
                                          ->check(value)
                                          ->required();
  auto*                actor        = command.add_option("--actor", o.actor)
                                          ->expected(1)
                                          ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
                                          ->check(value)
                                          ->required();
  auto*                observations = command.add_option("--observations", o.observations)
                                          ->expected(1)
                                          ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
                                          ->check(value);
  auto*                expect_fp    = command.add_option("--expect-fp", o.expect_fp)
                                          ->expected(1)
                                          ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
                                          ->check(value)
                                          ->required();
  (void)ledger;
  (void)request_id;
  (void)actor;
  (void)observations;
  run->check(
      CLI::Validator([](std::string& t) { return engine::ledger::parse_run_token(t) ? "" : "Expected run token"; }, "TOKEN"));
  expect_fp->check(CLI::Validator(
      [](std::string& t) {
        return t.size() == 41 && t.starts_with("xxh3_128:") &&
                       std::ranges::all_of(std::string_view(t).substr(9),
                                           [](char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); })
                   ? ""
                   : "Expected digest";
      },
      "DIGEST"));
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
response refresh_command(const refresh_options& o, engine::command_vm* vm) {
  return execute(refresh_intent(o), vm);
}
response refresh_with_profile(const refresh_options& o, std::string_view selector) {
  return execute_with_profile(refresh_intent(o), selector);
}
} // namespace planar::cmd::handlers
