module planar.cmd.planar.handlers.load;
import std;
import cli11;
import planar.cmd.invocation;
namespace planar::cmd::handlers {
namespace {
std::optional<std::int64_t> positive(std::string_view token) {
  if (token.empty() || token.front() < '1' || token.front() > '9')
    return {};
  std::int64_t number = 0;
  auto [end, error]   = std::from_chars(token.data(), token.data() + token.size(), number);
  if (error != std::errc{} || end != token.data() + token.size() || number <= 0)
    return {};
  return number;
}
} // namespace
CLI::App& attach_load(CLI::App& root, load_options& options) {
  auto& command = *root.add_subcommand("load");
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
  add("--workspace", options.workspace);
  add("--registry", options.registry);
  add("--expect-revision", options.expect_revision)
      ->check(CLI::Validator(
          [](std::string& token) { return token == "none" || positive(token) ? "" : "Expected canonical revision or none"; },
          "HEAD"));
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
engine::command_options load_intent(const load_options& options) {
  return engine::command_load_options{options.ledger,
                                      options.workspace,
                                      options.registry,
                                      options.expect_revision,
                                      options.request_id,
                                      options.actor,
                                      positive(options.expect_revision)};
}
response load_command(const load_options& options, engine::command_vm* vm) {
  return execute(load_intent(options), vm);
}
} // namespace planar::cmd::handlers
