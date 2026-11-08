module planar.cmd.execution.verdict;
import std;
import cli11;
import planar.cmd.invocation;
namespace planar::cmd::handlers {
namespace {
[[maybe_unused]] bool positive(std::string_view s) {
  if (s.empty() || s.front() < '1' || s.front() > '9')
    return false;
  std::int64_t n    = 0;
  auto [end, error] = std::from_chars(s.data(), s.data() + s.size(), n);
  return error == std::errc{} && end == s.data() + s.size() && n > 0;
}
[[maybe_unused]] bool token(std::string_view s) {
  auto first  = s.find('.');
  auto second = first == s.npos ? s.npos : s.find('.', first + 1);
  if (first == s.npos || second == s.npos || !positive(s.substr(0, first)) || !positive(s.substr(first + 1, second - first - 1)))
    return false;
  auto secret = s.substr(second + 1);
  return secret.size() == 32 &&
         std::ranges::all_of(secret, [](char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); });
}
} // namespace
CLI::App& attach_verdict(CLI::App& root, verdict_options& options) {
  auto& command = *root.add_subcommand("verdict");
  command.set_help_flag();
  command.allow_extras(false);
  command.fallthrough(false);
  const CLI::Validator explicit_value(
      [](std::string& value) { return value.empty() || value.starts_with("--") ? "Expected explicit value" : ""; }, "VALUE");
  command.add_option("--ledger", options.ledger)
      ->expected(1)
      ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
      ->check(explicit_value)
      ->required();
  command.add_option("--run", options.run)
      ->expected(1)
      ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
      ->check(explicit_value)
      ->required()
      ->check(CLI::Validator([](std::string& s) { return token(s) ? "" : "Invalid run token"; }, "TOKEN"));
  command.add_option("--verdict", options.verdict)
      ->expected(1)
      ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
      ->check(explicit_value)
      ->required();
  command.add_option("--request-id", options.request_id)
      ->expected(1)
      ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
      ->check(explicit_value)
      ->required()
      ->check(CLI::Validator(
          [](std::string& s) {
            return s.size() <= 128 && std::ranges::all_of(s,
                                                          [](char c) {
                                                            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                                                                   (c >= '0' && c <= '9') || c == '_' || c == '.' || c == ':' ||
                                                                   c == '-';
                                                          })
                       ? ""
                       : "Invalid request ID";
          },
          "ID"));
  command.add_option("--actor", options.actor)
      ->expected(1)
      ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
      ->check(explicit_value)
      ->required()
      ->check(CLI::Validator([](std::string& s) { return s.size() <= 256 && engine::valid_text(s) ? "" : "Invalid actor"; },
                             "ACTOR"));
  return command;
}

namespace {
engine::command_check_options intent(const verdict_options& o) {
  engine::command_check_options v;
  v.operation  = "verdict";
  v.ledger     = o.ledger;
  v.run        = o.run;
  v.verdict    = o.verdict;
  v.request_id = o.request_id;
  v.actor      = o.actor;
  return v;
}
} // namespace
response verdict_command(const verdict_options& o, engine::command_vm* vm) {
  return execute(intent(o), vm);
}
response verdict_with_profile(const verdict_options& o, std::string_view selector) {
  return execute_with_profile(intent(o), selector);
}
} // namespace planar::cmd::handlers
