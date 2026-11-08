module planar.cmd.execution.preview;
import std;
import cli11;
import planar.cmd.invocation;
namespace planar::cmd::handlers {
CLI::App& attach_preview(CLI::App& root, preview_options& options) {
  auto& command = *root.add_subcommand("preview");
  command.set_help_flag();
  command.allow_extras(false);
  command.fallthrough(false);
  const CLI::Validator explicit_value(
      [](std::string& token) { return token.empty() || token.starts_with("--") ? "Expected explicit value" : ""; }, "VALUE");
  auto* ledger = command.add_option("--ledger", options.ledger)
                     ->expected(1)
                     ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
                     ->check(explicit_value)
                     ->required();
  auto* role   = command.add_option("--role", options.role)
                     ->expected(1)
                     ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
                     ->check(explicit_value)
                     ->required();
  command.add_option("--since", options.since)
      ->expected(1)
      ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
      ->check(explicit_value);
  (void)ledger;
  (void)role;
  command.get_option("--role")->check(CLI::Validator(
      [](std::string& t) {
        return !t.empty() && t.size() <= 64 && t.front() >= 'a' && t.front() <= 'z' &&
                       std::ranges::all_of(t, [](char c) { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'; })
                   ? ""
                   : "Expected identifier";
      },
      "ID"));
  command.get_option("--since")->check(CLI::Validator(
      [](std::string& t) {
        return t.size() == 41 && t.starts_with("xxh3_128:") &&
                       std::ranges::all_of(std::string_view(t).substr(9),
                                           [](char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); })
                   ? ""
                   : "Expected digest";
      },
      "DIGEST"));
  return command;
}
response preview_command(const preview_options& options, engine::command_vm* vm) {
  return execute(engine::command_preview_options{options.ledger, options.role, options.since}, vm);
}
response preview_with_profile(const preview_options& options, std::string_view selector) {
  return execute_with_profile(engine::command_preview_options{options.ledger, options.role, options.since}, selector);
}
} // namespace planar::cmd::handlers
