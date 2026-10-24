module planar.cmd.planar.handlers.source_export;
import std;
import cli11;
import planar.cmd.invocation;
namespace planar::cmd::handlers {
namespace {
std::optional<std::int64_t> positive(std::string_view s) {
  if (s.empty() || s.front() < '1' || s.front() > '9')
    return {};
  std::int64_t n = 0;
  auto [p, e]    = std::from_chars(s.data(), s.data() + s.size(), n);
  if (e != std::errc{} || p != s.data() + s.size() || n <= 0)
    return {};
  return n;
}
} // namespace
CLI::App& attach_export(CLI::App& root, export_options& o) {
  auto& c = *root.add_subcommand("export");
  c.set_help_flag();
  c.allow_extras(false);
  c.fallthrough(false);
  const CLI::Validator explicit_value(
      [](std::string& s) { return s.empty() || s.starts_with("--") ? "Expected explicit value" : ""; }, "VALUE");
  const CLI::Validator id_value([](std::string& s) { return positive(s) ? "" : "Expected positive decimal ID"; }, "ID");
  auto                 add = [&](const char* name, std::string& value, const CLI::Validator& validator) {
    c.add_option(name, value)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(validator)->required();
  };
  add("--ledger", o.ledger, explicit_value);
  c.add_option("--revision", o.revision_number)
      ->expected(1)
      ->required()
      ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
      ->check(id_value)
      ->each([&o](const std::string& token) { o.revision = token; });
  add("--target", o.target, explicit_value);
  return c;
}
response export_failure(const engine::source_export::export_error& error) {
  return native_export_failure(error);
}
response export_command(const export_options& o, engine::command_vm* vm) {
  return execute(engine::command_export_options{o.ledger, o.target, o.revision_number}, vm);
}
} // namespace planar::cmd::handlers
