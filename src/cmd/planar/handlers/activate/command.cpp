module planar.cmd.execution.activate;
import std;
import cli11;
import planar.cmd.invocation;
namespace planar::cmd::handlers {
namespace {
bool positive(std::string_view token) {
  if (token.empty() || token.size() > 19 || token.front() < '1' || token.front() > '9')
    return false;
  std::int64_t number = 0;
  auto [end, error]   = std::from_chars(token.data(), token.data() + token.size(), number);
  return error == std::errc{} && end == token.data() + token.size() && number > 0;
}
/// Declaration identifier grammar `[a-z][a-z0-9_]{0,63}` (declarations.md).
bool identifier(std::string_view token) {
  if (token.empty() || token.size() > 64 || token.front() < 'a' || token.front() > 'z')
    return false;
  return std::ranges::all_of(token, [](char c) { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'; });
}
} // namespace
CLI::App& attach_activate(CLI::App& root, activate_options& options) {
  auto& command = *root.add_subcommand("activate");
  command.set_help_flag();
  command.allow_extras(false);
  command.fallthrough(false);
  const CLI::Validator explicit_value(
      [](std::string& token) { return token.empty() || token.starts_with("--") ? "Expected explicit value" : ""; }, "VALUE");
  auto add = [&](const char* name, std::string& value) {
    return command.add_option(name, value)
        ->expected(1)
        ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
        ->check(explicit_value)
        ->required();
  };
  add("--ledger", options.ledger);
  add("--job", options.job)
      ->check(CLI::Validator([](std::string& token) { return identifier(token) ? "" : "Expected job ID"; }, "ID"));
  add("--expect-revision", options.expect_revision)
      ->check(CLI::Validator(
          [](std::string& token) { return token == "none" || positive(token) ? "" : "Expected canonical revision or none"; },
          "ID_OR_NONE"));
  add("--expect-job", options.expect_job)
      ->check(
          CLI::Validator([](std::string& token) { return token == "NONE" || identifier(token) ? "" : "Expected job ID or NONE"; },
                         "JOB_OR_NONE"));
  add("--request-id", options.request_id)
      ->check(CLI::Validator(
          [](std::string& token) {
            return token.size() <= 128 && std::ranges::all_of(token,
                                                              [](unsigned char c) {
                                                                return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                                                                       (c >= '0' && c <= '9') || c == '_' || c == '.' ||
                                                                       c == ':' || c == '-';
                                                              })
                       ? ""
                       : "Expected ASCII request key";
          },
          "KEY"));
  add("--actor", options.actor)
      ->check(CLI::Validator(
          [](std::string& token) { return token.size() <= 256 && engine::valid_text(token) ? "" : "Expected UTF-8 actor"; },
          "ACTOR"));
  return command;
}
engine::command_options activate_intent(const activate_options& options) {
  return engine::command_activate_options{options.ledger,     options.job,        options.expect_revision,
                                          options.expect_job, options.request_id, options.actor};
}
response activate_command(const activate_options& options, engine::command_vm* vm) {
  return execute(activate_intent(options), vm);
}
response activate_with_profile(const activate_options& options, std::string_view profile_selector) {
  return execute_with_profile(activate_intent(options), profile_selector);
}
} // namespace planar::cmd::handlers
