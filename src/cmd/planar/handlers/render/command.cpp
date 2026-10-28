module planar.cmd.planar.handlers.render;
import std;
import cli11;
import planar.cmd.invocation;
namespace planar::cmd::handlers {
namespace {
using namespace planar::engine;
using namespace planar::cmd::detail;
bool mapping(std::string_view value) {
  const auto separator = value.find('=');
  return separator != std::string_view::npos && separator > 0 && separator + 1 < value.size();
}
bool budget(std::string_view value) {
  if (value.empty() || value.front() == '0' || value.size() > 7)
    return false;
  unsigned result = 0;
  for (const char byte : value) {
    if (byte < '0' || byte > '9')
      return false;
    result = result * 10 + static_cast<unsigned>(byte - '0');
  }
  return result <= 1048576;
}
} // namespace
CLI::App& attach_render(CLI::App& root, render_options& options) {
  auto& command = *root.add_subcommand("render");
  command.set_help_flag();
  command.allow_extras(false);
  command.fallthrough(false);
  const CLI::Validator explicit_value(
      [](std::string& value) { return value.empty() || value.starts_with("--") ? "Expected explicit nonempty value" : ""; },
      "VALUE");
  const std::array<std::string, 8> names = {"--workspace", "--registry",  "--root",         "--job",
                                            "--role",      "--base-root", "--observations", "--budget-bytes"};
  for (std::size_t index = 0; index < names.size(); ++index) {
    auto* option = index == 7
                       ? command.add_option(names[index], options.budget_bytes)->each([&options](const std::string& token) {
                           options.values[7] = token;
                         })
                       : command.add_option(names[index], options.values[index]);
    option->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(explicit_value);
    if (index < 5)
      option->required();
    if (index == 2 || index == 5)
      option->check(CLI::Validator([](std::string& value) { return mapping(value) ? "" : "Expected ID=DIRECTORY"; }, "ROOT"));
    if (index == 7)
      option->check(
          CLI::Validator([](std::string& value) { return budget(value) ? "" : "Expected bounded positive decimal"; }, "BUDGET"));
  }
  command.add_flag("--capture-outputs", options.capture_outputs)->multi_option_policy(CLI::MultiOptionPolicy::Throw);
  return command;
}
engine::command_options render_intent(const render_options& options) {
  engine::command_render_options intent{
      options.values[0], options.values[1], options.values[2], options.values[3], options.values[4], {}, {}, {}, false};
  if (!options.values[5].empty())
    intent.base_root = options.values[5];
  if (!options.values[6].empty())
    intent.observations = options.values[6];
  if (!options.values[7].empty())
    intent.budget_bytes = options.budget_bytes;
  intent.capture_outputs = options.capture_outputs;
  return intent;
}
response render_command(const render_options& options, engine::command_vm* vm) {
  return execute(render_intent(options), vm);
}
} // namespace planar::cmd::handlers
