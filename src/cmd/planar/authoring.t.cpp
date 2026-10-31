// RA01-B1 authoring tests: root help splice, response kind mapping, the six CLI11 families, the stage function and the
// authoring bundle entry (compile-time selected stack). Expected bytes are typed from the authoring contract.
#include <catch2/catch_test_macros.hpp>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
import std;
import cli11;
import planar.cmd.authoring;
import planar.cmd.authoring.spec_prompt;
import planar.cmd.authoring.critique;
import planar.cmd.authoring.ingest;
import planar.cmd.authoring.explain_rejection;
import planar.cmd.authoring.observe;
import planar.cmd.authoring.orchestrator;
import planar.cmd.authoring_stack.bootstrap;
import planar.cmd.bootstrap;

namespace {
namespace fs        = std::filesystem;
namespace authoring = planar::cmd::authoring;

/// Five-command HEAD root help, typed independently of the product.
std::string ra01_head_help() {
  return "Usage: planar render --workspace FILE --registry FILE --root ID=DIRECTORY --job ID --role ID\n"
         "Optional: --capture-outputs --base-root ID=DIRECTORY --observations FILE --budget-bytes N\n"
         "planar init --ledger FILE --workspace-id ID\n"
         "planar history --ledger FILE [--after-event ID] [--limit N]\n"
         "  or --revision ID | --request-id ID | --at-event ID\n"
         "planar export --ledger PATH --revision ID --target NEW_DIRECTORY\n"
         "planar load --ledger PATH --workspace FILE_OR_NAME --registry FILE --expect-revision ID_OR_NONE --request-id ID "
         "--actor TEXT\n"
         "planar --help\n";
}
/// The six RA-1 lines in contract order.
std::string ra01_six_lines() {
  return "planar spec-prompt --workspace FILE_OR_NAME --registry FILE\n"
         "planar critique --workspace FILE_OR_NAME --registry FILE\n"
         "planar ingest --workspace FILE_OR_NAME --registry FILE\n"
         "planar explain-rejection --diagnostic FILE\n"
         "planar observe --from DIRECTORY\n"
         "planar orchestrator --workspace FILE_OR_NAME --registry FILE --job ID\n";
}
std::string ra01_final_line() {
  return "planar --help\n";
}
std::string ra01_authoring_help() {
  auto head = ra01_head_help();
  head.resize(head.size() - ra01_final_line().size());
  return head + ra01_six_lines() + ra01_final_line();
}

/// Exit status plus exact stdout and stderr bytes.
struct ra01_bytes {
  int         status = -1;
  std::string out, err;
  bool        operator==(const ra01_bytes&) const = default;
};
ra01_bytes ra01_of(const planar::cmd::response& r) {
  return {r.status, r.output, r.error};
}
ra01_bytes ra01_usage() {
  return ra01_of(planar::cmd::detail::usage());
}
/// The wave-1 stub refusal, typed literally.
ra01_bytes ra01_stub() {
  return {4, "",
          "{\"error\":{\"code\":\"internal\",\"field\":\"\",\"message\":\"Invalid embedded command protocol\"},"
          "\"kind\":\"error\",\"version\":1}\n"};
}
planar::engine::diagnostic ra01_diagnostic(std::string_view code, std::string_view message) {
  planar::engine::diagnostic d;
  d.code.assign(code);
  d.message.assign(message);
  return d;
}
ra01_bytes ra01_native(std::string_view code, std::string_view message) {
  return ra01_of(planar::cmd::detail::failed(planar::cmd::detail::native_error(ra01_diagnostic(code, message))));
}

struct ra01_verb {
  std::string              name;
  std::vector<std::string> options;
  authoring::verb          which;
};
const std::vector<ra01_verb>& ra01_verbs() {
  static const std::vector<ra01_verb> table{
      {"spec-prompt", {"--workspace", "--registry"}, authoring::verb::spec_prompt},
      {"critique", {"--workspace", "--registry"}, authoring::verb::critique},
      {"ingest", {"--workspace", "--registry"}, authoring::verb::ingest},
      {"explain-rejection", {"--diagnostic"}, authoring::verb::explain_rejection},
      {"observe", {"--from"}, authoring::verb::observe},
      {"orchestrator", {"--workspace", "--registry", "--job"}, authoring::verb::orchestrator},
  };
  return table;
}

/// Scratch tree with one existing file or directory per explicit path option; the cwd is `work`.
struct ra01_scratch {
  fs::path root, work, workspace, registry, diagnostic, observations;
  ra01_scratch() {
    char pattern[] = "/tmp/planar-ra01-XXXXXX";
    REQUIRE(::mkdtemp(pattern));
    root         = fs::canonical(pattern);
    work         = root / "work";
    workspace    = root / "ws" / "workspace.lua";
    registry     = root / "registry.json";
    diagnostic   = root / "diag.json";
    observations = root / "obs";
    fs::create_directories(work);
    fs::create_directories(workspace.parent_path());
    fs::create_directories(observations);
    write(workspace, "return {}\n");
    write(registry, "{}\n");
    write(diagnostic, "{\"code\":\"schema\"}\n");
    write(observations / "o.json", "{}\n");
    write(work / "keep.txt", "keep\n");
  }
  ra01_scratch(const ra01_scratch&)            = delete;
  ra01_scratch& operator=(const ra01_scratch&) = delete;
  ~ra01_scratch() {
    std::error_code ignored;
    fs::remove_all(root, ignored);
  }
  static void write(const fs::path& path, std::string_view bytes) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << bytes;
    REQUIRE(out.good());
  }
  std::string value(std::string_view option) const {
    if (option == "--workspace")
      return workspace.string();
    if (option == "--registry")
      return registry.string();
    if (option == "--diagnostic")
      return diagnostic.string();
    if (option == "--from")
      return observations.string();
    return "job-a";
  }
  /// Verb name and its full valid signature.
  std::vector<std::string> full(const ra01_verb& verb) const {
    std::vector<std::string> args{verb.name};
    for (const auto& option : verb.options) {
      args.push_back(option);
      args.push_back(value(option));
    }
    return args;
  }
  /// Full signature with one option's value replaced.
  std::vector<std::string> with(const ra01_verb& verb, std::string_view option, std::string replacement) const {
    std::vector<std::string> args{verb.name};
    for (const auto& name : verb.options) {
      args.push_back(name);
      args.push_back(name == option ? replacement : value(name));
    }
    return args;
  }
  /// Full signature without one option.
  std::vector<std::string> without(const ra01_verb& verb, std::string_view option) const {
    std::vector<std::string> args{verb.name};
    for (const auto& name : verb.options) {
      if (name == option)
        continue;
      args.push_back(name);
      args.push_back(value(name));
    }
    return args;
  }
};

/// Scoped cwd and workspace-environment guard.
struct ra01_guard {
  fs::path saved;
  ra01_guard(const fs::path& to, const std::optional<std::string>& workspace_env) : saved(fs::current_path()) {
    fs::current_path(to);
    ::unsetenv("PLANAR_WORKSPACE_DIR");
    if (workspace_env)
      ::setenv("PLANAR_WORKSPACE_DIR", workspace_env->c_str(), 1);
  }
  ra01_guard(const ra01_guard&)            = delete;
  ra01_guard& operator=(const ra01_guard&) = delete;
  ~ra01_guard() {
    std::error_code ignored;
    fs::current_path(saved, ignored);
    ::unsetenv("PLANAR_WORKSPACE_DIR");
  }
};
ra01_bytes ra01_stack(const ra01_scratch& s, const std::vector<std::string>& args, const std::optional<std::string>& env = {}) {
  ra01_guard                    guard(s.work, env);
  std::vector<std::string_view> views(args.begin(), args.end());
  return ra01_of(planar::cmd::authoring_stack::invoke(views));
}
ra01_bytes ra01_default(const ra01_scratch& s, const std::vector<std::string>& args, const std::optional<std::string>& env = {}) {
  ra01_guard                    guard(s.work, env);
  std::vector<std::string_view> views(args.begin(), args.end());
  return ra01_of(planar::cmd::invoke(views));
}
std::vector<std::string> ra01_cat(std::vector<std::string> head, const std::vector<std::string>& tail) {
  head.insert(head.end(), tail.begin(), tail.end());
  return head;
}

/// Counting readers over fixed values.
struct ra01_counts {
  int cwd = 0;
  int env = 0;
};
authoring::readers ra01_readers(std::string cwd, std::string env, ra01_counts& counts) {
  return authoring::readers{[&counts, cwd = std::move(cwd)]() -> std::expected<std::string, planar::engine::diagnostic> {
                              ++counts.cwd;
                              return cwd;
                            },
                            [&counts, env = std::move(env)]() -> std::expected<std::string, planar::engine::diagnostic> {
                              ++counts.env;
                              return env;
                            }};
}

/// Every entry under a root: relative path to bytes, directories as a marker.
std::map<std::string, std::string> ra01_snapshot(const fs::path& root) {
  std::map<std::string, std::string> out;
  for (const auto& entry : fs::recursive_directory_iterator(root)) {
    const auto key = fs::relative(entry.path(), root).generic_string();
    if (entry.is_symlink())
      out[key] = "<link>" + fs::read_symlink(entry.path()).string();
    else if (entry.is_directory())
      out[key] = "<dir>";
    else {
      std::ifstream in(entry.path(), std::ios::binary);
      out[key] = std::string{std::istreambuf_iterator<char>(in), {}};
    }
  }
  return out;
}

// ---- CLI11 family adapters -------------------------------------------------------------------------------------------

struct ra01_family {
  std::string                                                                 name;
  std::vector<std::string>                                                    options;
  std::function<std::map<std::string, std::string>(std::vector<std::string>)> parse; ///< Forward-order arguments after the verb.
  std::function<std::vector<std::string>()> bound; ///< Sorted long names of the attached options.
};
template <class options_type>
ra01_family ra01_make(std::string name, std::vector<std::string> options, CLI::App& (*attach)(CLI::App&, options_type&),
                      std::map<std::string, std::string> (*fields)(const options_type&)) {
  ra01_family family;
  family.name    = name;
  family.options = std::move(options);
  family.parse   = [name, attach, fields](std::vector<std::string> arguments) {
    CLI::App root{"", "planar"};
    root.set_help_flag();
    options_type values;
    auto&        command = attach(root, values);
    arguments.insert(arguments.begin(), name);
    std::ranges::reverse(arguments);
    root.parse(arguments);
    CHECK(command.count() == 1);
    return fields(values);
  };
  family.bound = [name, attach] {
    CLI::App root{"", "planar"};
    root.set_help_flag();
    options_type values;
    auto&        command = attach(root, values);
    CHECK(root.get_subcommands({}).size() == 1);
    CHECK(command.get_name() == name);
    CHECK(command.get_subcommands({}).empty());
    std::vector<std::string> out;
    for (const auto* option : command.get_options()) {
      CHECK(option->get_lnames().size() == 1);
      CHECK(option->get_snames().empty());
      CHECK_FALSE(option->get_positional());
      CHECK(option->get_expected_min() == 1);
      CHECK(option->get_expected_max() == 1);
      for (const auto& long_name : option->get_lnames())
        out.push_back("--" + long_name);
    }
    std::ranges::sort(out);
    return out;
  };
  return family;
}
const std::vector<ra01_family>& ra01_families() {
  static const std::vector<ra01_family> table{
      ra01_make<authoring::spec_prompt_options>(
          "spec-prompt", {"--workspace", "--registry"}, authoring::attach_spec_prompt,
          +[](const authoring::spec_prompt_options& o) {
            return std::map<std::string, std::string>{{"--workspace", o.workspace}, {"--registry", o.registry}};
          }),
      ra01_make<authoring::critique_options>(
          "critique", {"--workspace", "--registry"}, authoring::attach_critique,
          +[](const authoring::critique_options& o) {
            return std::map<std::string, std::string>{{"--workspace", o.workspace}, {"--registry", o.registry}};
          }),
      ra01_make<authoring::ingest_options>(
          "ingest", {"--workspace", "--registry"}, authoring::attach_ingest,
          +[](const authoring::ingest_options& o) {
            return std::map<std::string, std::string>{{"--workspace", o.workspace}, {"--registry", o.registry}};
          }),
      ra01_make<authoring::explain_rejection_options>(
          "explain-rejection", {"--diagnostic"}, authoring::attach_explain_rejection,
          +[](const authoring::explain_rejection_options& o) {
            return std::map<std::string, std::string>{{"--diagnostic", o.diagnostic}};
          }),
      ra01_make<authoring::observe_options>(
          "observe", {"--from"}, authoring::attach_observe,
          +[](const authoring::observe_options& o) { return std::map<std::string, std::string>{{"--from", o.from}}; }),
      ra01_make<authoring::orchestrator_options>(
          "orchestrator", {"--workspace", "--registry", "--job"}, authoring::attach_orchestrator,
          +[](const authoring::orchestrator_options& o) {
            return std::map<std::string, std::string>{{"--workspace", o.workspace}, {"--registry", o.registry}, {"--job", o.job}};
          }),
  };
  return table;
}
std::string ra01_unit_value(std::string_view option) {
  return "v-" + std::string(option.substr(2));
}
std::vector<std::string> ra01_unit_args(const ra01_family& family) {
  std::vector<std::string> args;
  for (const auto& option : family.options) {
    args.push_back(option);
    args.push_back(ra01_unit_value(option));
  }
  return args;
}
std::map<std::string, std::string> ra01_unit_expected(const ra01_family& family) {
  std::map<std::string, std::string> out;
  for (const auto& option : family.options)
    out[option] = ra01_unit_value(option);
  return out;
}
} // namespace

TEST_CASE("RA01-B1 help splice inserts six lines before the final line only", "[authoring][ra01]") {
  const auto spliced = authoring::splice_root_help(ra01_head_help());
  REQUIRE(spliced.has_value());
  CHECK(*spliced == ra01_authoring_help());
  // The final line absent, or the text empty, gives no result.
  auto head = ra01_head_help();
  head.resize(head.size() - ra01_final_line().size());
  CHECK_FALSE(authoring::splice_root_help(head).has_value());
  CHECK_FALSE(authoring::splice_root_help("").has_value());
  // Only the final occurrence receives the lines and every other byte is kept.
  const std::string twice = "planar --help\nmiddle\nplanar --help\n";
  const auto        both  = authoring::splice_root_help(twice);
  REQUIRE(both.has_value());
  CHECK(*both == "planar --help\nmiddle\n" + ra01_six_lines() + "planar --help\n");
}

TEST_CASE("RA01-B1 authoring root help inserts the six RA-1 lines before the final line", "[authoring][ra01]") {
  const std::array<std::string_view, 1> args   = {"--help"};
  const auto                            result = planar::cmd::authoring_stack::invoke(args);
  CHECK(result.status == 0);
  CHECK(result.error.empty());
  CHECK(result.output == ra01_authoring_help());
  CHECK(result.output != ra01_head_help());
}

TEST_CASE("RA01-B1 response kinds map to envelope kinds", "[authoring][ra01]") {
  using planar::cmd::detail::authoring_envelope_kind;
  for (const std::string_view kind : {"spec_prompt_v1", "critique_v1", "ingest_v1", "explain_rejection_v1", "orchestrator_v1"}) {
    CAPTURE(kind);
    const auto mapped = authoring_envelope_kind(kind);
    REQUIRE(mapped.has_value());
    CHECK(*mapped == "prompt");
  }
  const auto observe = authoring_envelope_kind("observe_v1");
  REQUIRE(observe.has_value());
  CHECK(observe->empty());
  const std::array<std::pair<std::string_view, std::string_view>, 5> shipped{
      {{"render_v1", "render"}, {"init_v1", "init"}, {"history_v1", "history"}, {"export_v1", "export"}, {"load_v1", "load"}}};
  for (const auto& [kind, envelope] : shipped) {
    CAPTURE(kind);
    const auto mapped = authoring_envelope_kind(kind);
    REQUIRE(mapped.has_value());
    CHECK(*mapped == envelope);
  }
  for (const std::string_view kind : {"extension_v1", "", "OBSERVE_V1", "prompt", "observe_v2"}) {
    CAPTURE(kind);
    CHECK_FALSE(authoring_envelope_kind(kind).has_value());
  }
}

TEST_CASE("RA01-B1 each authoring family binds exactly its RA-1 option names", "[authoring][ra01]") {
  CHECK(ra01_families().size() == 6);
  for (const auto& family : ra01_families()) {
    CAPTURE(family.name);
    auto expected = family.options;
    std::ranges::sort(expected);
    CHECK(family.bound() == expected);
    CHECK(family.parse(ra01_unit_args(family)) == ra01_unit_expected(family));
    // Option order on the command line does not matter.
    auto reversed = family.options;
    std::ranges::reverse(reversed);
    std::vector<std::string> args;
    for (const auto& option : reversed) {
      args.push_back(option);
      args.push_back(ra01_unit_value(option));
    }
    CHECK(family.parse(args) == ra01_unit_expected(family));
  }
}

TEST_CASE("RA01-B1 authoring families require every option", "[authoring][ra01]") {
  for (const auto& family : ra01_families()) {
    CAPTURE(family.name);
    CHECK(family.parse(ra01_unit_args(family)) == ra01_unit_expected(family));
    CHECK_THROWS_AS(family.parse({}), CLI::ParseError);
    for (const auto& omitted : family.options) {
      CAPTURE(omitted);
      std::vector<std::string> args;
      for (const auto& option : family.options) {
        if (option == omitted)
          continue;
        args.push_back(option);
        args.push_back(ra01_unit_value(option));
      }
      CHECK_THROWS_AS(family.parse(args), CLI::ParseError);
    }
  }
}

TEST_CASE("RA01-B1 authoring families refuse repeated options", "[authoring][ra01]") {
  for (const auto& family : ra01_families()) {
    CAPTURE(family.name);
    CHECK(family.parse(ra01_unit_args(family)) == ra01_unit_expected(family));
    for (const auto& repeated : family.options) {
      CAPTURE(repeated);
      CHECK_THROWS_AS(family.parse(ra01_cat(ra01_unit_args(family), {repeated, "again"})), CLI::ParseError);
      CHECK_THROWS_AS(family.parse(ra01_cat({repeated, "again"}, ra01_unit_args(family))), CLI::ParseError);
      CHECK_THROWS_AS(family.parse(ra01_cat(ra01_unit_args(family), {repeated, ra01_unit_value(repeated)})), CLI::ParseError);
    }
  }
}

TEST_CASE("RA01-B1 authoring families refuse empty and dash prefixed values", "[authoring][ra01]") {
  for (const auto& family : ra01_families()) {
    CAPTURE(family.name);
    CHECK(family.parse(ra01_unit_args(family)) == ra01_unit_expected(family));
    for (const auto& target : family.options)
      for (const std::string bad : {"", "--x", "--value", "---"}) {
        CAPTURE(target, bad);
        std::vector<std::string> args;
        for (const auto& option : family.options) {
          args.push_back(option);
          args.push_back(option == target ? bad : ra01_unit_value(option));
        }
        CHECK_THROWS_AS(family.parse(args), CLI::ParseError);
      }
  }
}

TEST_CASE("RA01-B1 authoring families refuse positionals unknown options and help", "[authoring][ra01]") {
  for (const auto& family : ra01_families()) {
    CAPTURE(family.name);
    const auto valid = ra01_unit_args(family);
    CHECK(family.parse(valid) == ra01_unit_expected(family));
    CHECK_THROWS_AS(family.parse(ra01_cat(valid, {"extra"})), CLI::ParseError);
    CHECK_THROWS_AS(family.parse(ra01_cat({"extra"}, valid)), CLI::ParseError);
    CHECK_THROWS_AS(family.parse(ra01_cat(valid, {"--bogus", "v"})), CLI::ParseError);
    CHECK_THROWS_AS(family.parse(ra01_cat(valid, {"--bogus"})), CLI::ParseError);
    CHECK_THROWS_AS(family.parse(ra01_cat(valid, {"--help"})), CLI::ParseError);
    CHECK_THROWS_AS(family.parse(ra01_cat(valid, {"-h"})), CLI::ParseError);
    CHECK_THROWS_AS(family.parse(ra01_cat({"--help"}, valid)), CLI::ParseError);
  }
}

TEST_CASE("RA01-B1 orchestrator job value is text", "[authoring][ra01]") {
  const ra01_family* orchestrator = nullptr;
  for (const auto& family : ra01_families())
    if (family.name == "orchestrator")
      orchestrator = &family;
  REQUIRE(orchestrator != nullptr);
  for (const std::string job : {"12", "not an id!", "job-a", "007"}) {
    CAPTURE(job);
    const auto stored = orchestrator->parse({"--workspace", "w", "--registry", "r", "--job", job});
    CHECK(stored.at("--job") == job);
    CHECK(stored.at("--workspace") == "w");
    CHECK(stored.at("--registry") == "r");
  }
}

TEST_CASE("RA01-B1 each authoring verb parses its signature through the bundle to the stub refusal", "[authoring][ra01]") {
  const ra01_scratch s;
  for (const auto& verb : ra01_verbs()) {
    CAPTURE(verb.name);
    CHECK(ra01_stack(s, s.full(verb)) == ra01_stub());
    CHECK_FALSE(ra01_stack(s, s.full(verb)) == ra01_usage());
  }
}

TEST_CASE("RA01-B1 authoring bundle runs shipped verbs like the default entry", "[authoring][ra01]") {
  const ra01_scratch                          s;
  const auto                                  missing = (s.root / "missing.db").string();
  const std::vector<std::vector<std::string>> cases{{"history", "--ledger", missing},
                                                    {"status"},
                                                    {},
                                                    {"render"},
                                                    {"history"},
                                                    {"init", "--ledger", missing},
                                                    {"export", "--ledger", missing, "--revision", "1", "--target", "out"}};
  for (const auto& args : cases) {
    CAPTURE(args);
    CHECK(ra01_stack(s, args) == ra01_default(s, args));
  }
  // The history missing ledger is a real shipped failure, never the authoring stub.
  CHECK_FALSE(ra01_stack(s, cases.front()) == ra01_stub());
  CHECK(ra01_stack(s, cases.front()).status == ra01_default(s, cases.front()).status);
  // Positive control: the same stack does run an authoring verb that the default entry refuses.
  const auto& observe = ra01_verbs()[4];
  CHECK(ra01_stack(s, s.full(observe)) == ra01_stub());
  CHECK(ra01_default(s, s.full(observe)) == ra01_usage());
}

TEST_CASE("RA01-B1 authoring verbs refuse missing required options as usage", "[authoring][ra01]") {
  const ra01_scratch s;
  for (const auto& verb : ra01_verbs()) {
    CAPTURE(verb.name);
    CHECK(ra01_stack(s, s.full(verb)) == ra01_stub());
    CHECK(ra01_stack(s, {verb.name}) == ra01_usage());
    for (const auto& option : verb.options) {
      CAPTURE(option);
      CHECK(ra01_stack(s, s.without(verb, option)) == ra01_usage());
    }
  }
}

TEST_CASE("RA01-B1 authoring verbs refuse repeats empty and dash prefixed values as usage", "[authoring][ra01]") {
  const ra01_scratch s;
  for (const auto& verb : ra01_verbs()) {
    CAPTURE(verb.name);
    CHECK(ra01_stack(s, s.full(verb)) == ra01_stub());
    for (const auto& option : verb.options) {
      CAPTURE(option);
      CHECK(ra01_stack(s, ra01_cat(s.full(verb), {option, s.value(option)})) == ra01_usage());
      CHECK(ra01_stack(s, ra01_cat(s.full(verb), {option, "again"})) == ra01_usage());
      for (const std::string bad : {"", "--x", "--value"}) {
        CAPTURE(bad);
        CHECK(ra01_stack(s, s.with(verb, option, bad)) == ra01_usage());
      }
    }
  }
}

TEST_CASE("RA01-B1 authoring verbs refuse positionals unknown options and equals syntax as usage", "[authoring][ra01]") {
  const ra01_scratch s;
  for (const auto& verb : ra01_verbs()) {
    CAPTURE(verb.name);
    const auto full = s.full(verb);
    CHECK(ra01_stack(s, full) == ra01_stub());
    CHECK(ra01_stack(s, ra01_cat(full, {"extra"})) == ra01_usage());
    CHECK(ra01_stack(s, ra01_cat({verb.name, "extra"}, std::vector<std::string>(full.begin() + 1, full.end()))) == ra01_usage());
    CHECK(ra01_stack(s, ra01_cat(full, {"--bogus", "v"})) == ra01_usage());
    CHECK(ra01_stack(s, ra01_cat(full, {"--bogus"})) == ra01_usage());
    CHECK(ra01_stack(s, ra01_cat(full, {"--help"})) == ra01_usage());
    CHECK(ra01_stack(s, ra01_cat({"--help"}, full)) == ra01_usage());
    for (const auto& option : verb.options) {
      CAPTURE(option);
      auto joined = std::vector<std::string>{verb.name};
      for (const auto& name : verb.options)
        if (name == option)
          joined.push_back(name + "=" + s.value(name));
        else {
          joined.push_back(name);
          joined.push_back(s.value(name));
        }
      CHECK(ra01_stack(s, joined) == ra01_usage());
    }
  }
}

TEST_CASE("RA01-B1 orchestrator requires all three options", "[authoring][ra01]") {
  const ra01_scratch s;
  const auto&        orchestrator = ra01_verbs()[5];
  REQUIRE(orchestrator.name == "orchestrator");
  CHECK(ra01_stack(s, s.full(orchestrator)) == ra01_stub());
  CHECK(ra01_stack(s, s.with(orchestrator, "--job", "12")) == ra01_stub());
  for (const auto& option : orchestrator.options)
    CHECK(ra01_stack(s, s.without(orchestrator, option)) == ra01_usage());
  CHECK(ra01_stack(s, {"orchestrator", "--job", "job-a"}) == ra01_usage());
  CHECK(ra01_stack(s, {"orchestrator", "--workspace", s.workspace.string(), "--registry", s.registry.string()}) == ra01_usage());
}

TEST_CASE("RA01-B1 authoring verbs do not accept each others options", "[authoring][ra01]") {
  const ra01_scratch s;
  const auto         ws = s.workspace.string();
  const auto         rg = s.registry.string();
  const auto         dg = s.diagnostic.string();
  const auto         ob = s.observations.string();
  // Each row is a valid signature of its verb plus one option that belongs to another verb.
  const std::vector<std::vector<std::string>> rows{
      {"observe", "--from", ob, "--workspace", ws},
      {"observe", "--from", ob, "--registry", rg},
      {"spec-prompt", "--workspace", ws, "--registry", rg, "--job", "job-a"},
      {"critique", "--workspace", ws, "--registry", rg, "--diagnostic", dg},
      {"ingest", "--workspace", ws, "--registry", rg, "--from", ob},
      {"explain-rejection", "--diagnostic", dg, "--from", ob},
      {"explain-rejection", "--diagnostic", dg, "--workspace", ws},
      {"orchestrator", "--workspace", ws, "--registry", rg, "--job", "job-a", "--diagnostic", dg},
      {"orchestrator", "--workspace", ws, "--registry", rg, "--job", "job-a", "--from", ob},
  };
  for (const auto& row : rows) {
    CAPTURE(row);
    CHECK(ra01_stack(s, row) == ra01_usage());
  }
  CHECK(ra01_stack(s, {"observe", "--from", ob}) == ra01_stub());
}

TEST_CASE("RA01-B1 stub invocations write nothing and print no success document", "[authoring][ra01]") {
  const ra01_scratch s;
  for (const auto& verb : ra01_verbs()) {
    CAPTURE(verb.name);
    const auto before = ra01_snapshot(s.root);
    const auto got    = ra01_stack(s, s.full(verb));
    CHECK(got.status == 4);
    CHECK(got.out.empty());
    CHECK(got == ra01_stub());
    CHECK(ra01_snapshot(s.root) == before);
  }
  CHECK(ra01_snapshot(s.root).contains("work/keep.txt"));
}

TEST_CASE("RA01-B1 stub refusal is the closed internal envelope on stderr only", "[authoring][ra01]") {
  const auto direct = authoring::refuse_stub();
  CHECK(direct.status == 4);
  CHECK(direct.output.empty());
  CHECK(direct.error == ra01_stub().err);
  const ra01_scratch s;
  for (const auto& verb : ra01_verbs()) {
    CAPTURE(verb.name);
    const auto got = ra01_stack(s, s.full(verb));
    CHECK(got.status == 4);
    CHECK(got.out.empty());
    CHECK(got.err == ra01_stub().err);
    CHECK_FALSE(got.err == ra01_usage().err);
  }
  // The stage function followed by the stub is the same refusal.
  ra01_counts       counts;
  const auto        io = ra01_readers(s.root.string(), (s.root / "wsdir").string(), counts);
  authoring::inputs values;
  values.from = s.observations.string();
  CHECK(ra01_of(authoring::run_stub(authoring::verb::observe, values, io)) == ra01_stub());
}

TEST_CASE("RA01-B1 usage precedes the workspace selector stage", "[authoring][ra01]") {
  const ra01_scratch s;
  const std::string  bad_selector = "bad/\xff";
  const auto         registry     = s.registry.string();
  const ra01_bytes   selector     = ra01_native("path_denied", "Invalid explicit input path");
  REQUIRE(selector.status == 3);
  // Positive control: without the root help token the selector stage runs and refuses.
  CHECK(ra01_stack(s, {"spec-prompt", "--workspace", bad_selector, "--registry", registry}) == selector);
  CHECK(ra01_stack(s, {"--help", "spec-prompt", "--workspace", bad_selector, "--registry", registry}) == ra01_usage());
  CHECK(ra01_stack(s, {"--help", "spec-prompt", "--workspace", bad_selector, "--registry", registry}).status == 2);
}

TEST_CASE("RA01-B1 selector stage precedes the explicit path stage", "[authoring][ra01]") {
  const ra01_scratch s;
  const std::string  bad_selector = "bad/\xff";
  const auto         missing      = (s.root / "missing.json").string();
  const ra01_bytes   selector     = ra01_native("path_denied", "Invalid explicit input path");
  const ra01_bytes   unresolved   = ra01_native("io", "Explicit input must be a regular file");
  REQUIRE(selector.status == 3);
  REQUIRE(unresolved.status == 4);
  for (const std::string verb : {"spec-prompt", "critique", "ingest"}) {
    CAPTURE(verb);
    CHECK(ra01_stack(s, {verb, "--workspace", bad_selector, "--registry", missing}) == selector);
    // Control: a valid selector lets the missing registry report its own io error.
    CHECK(ra01_stack(s, {verb, "--workspace", s.workspace.string(), "--registry", missing}) == unresolved);
  }
  CHECK(ra01_stack(s, {"orchestrator", "--workspace", bad_selector, "--registry", missing, "--job", "j"}) == selector);
  CHECK(ra01_stack(s, {"orchestrator", "--workspace", s.workspace.string(), "--registry", missing, "--job", "j"}) == unresolved);
}

TEST_CASE("RA01-B1 explicit path stage precedes the stub refusal", "[authoring][ra01]") {
  const ra01_scratch s;
  const auto         missing     = (s.root / "missing").string();
  const ra01_bytes   no_file     = ra01_native("io", "Cannot resolve explicit input path");
  const ra01_bytes   no_registry = ra01_native("io", "Explicit input must be a regular file");
  const ra01_bytes   no_root     = ra01_native("io", "Cannot resolve explicit root directory");
  for (const auto& verb : ra01_verbs()) {
    CAPTURE(verb.name);
    CHECK(ra01_stack(s, s.full(verb)) == ra01_stub());
    for (const auto& option : verb.options) {
      if (option == "--workspace" || option == "--job")
        continue;
      CAPTURE(option);
      CHECK(ra01_stack(s, s.with(verb, option, missing)) == (option == "--from"       ? no_root
                                                             : option == "--registry" ? no_registry
                                                                                      : no_file));
    }
  }
}

TEST_CASE("RA01-B1 only workspace verbs read the workspace environment", "[authoring][ra01]") {
  const ra01_scratch s;
  const auto         directory = (s.root / "wsdir").string();
  for (const auto which :
       {authoring::verb::spec_prompt, authoring::verb::critique, authoring::verb::ingest, authoring::verb::orchestrator}) {
    ra01_counts       counts;
    auto              io = ra01_readers(s.root.string(), directory, counts);
    authoring::inputs values;
    values.workspace  = "ws";
    values.registry   = s.registry.string();
    const auto result = authoring::prepare_inputs(which, values, io);
    CHECK(result.has_value());
    CHECK(counts.env == 1);
    CHECK(values.workspace == directory + "/ws/workspace.lua");
    // An explicit workspace file never reads the environment.
    ra01_counts       file_counts;
    auto              file_io = ra01_readers(s.root.string(), directory, file_counts);
    authoring::inputs file_values;
    file_values.workspace = s.workspace.string();
    file_values.registry  = s.registry.string();
    CHECK(authoring::prepare_inputs(which, file_values, file_io).has_value());
    CHECK(file_counts.env == 0);
  }
  {
    ra01_counts       counts;
    auto              io = ra01_readers(s.root.string(), directory, counts);
    authoring::inputs values;
    values.workspace  = "ws";
    values.diagnostic = s.diagnostic.string();
    CHECK(authoring::prepare_inputs(authoring::verb::explain_rejection, values, io).has_value());
    CHECK(counts.env == 0);
    CHECK(counts.cwd == 0);
    CHECK(values.diagnostic == s.diagnostic.string());
  }
  {
    ra01_counts       counts;
    auto              io = ra01_readers(s.root.string(), directory, counts);
    authoring::inputs values;
    values.workspace = "ws";
    values.from      = s.observations.string();
    CHECK(authoring::prepare_inputs(authoring::verb::observe, values, io).has_value());
    CHECK(counts.env == 0);
    CHECK(counts.cwd == 0);
    CHECK(values.from == s.observations.string());
  }
}

TEST_CASE("RA01-B1 stages read the cwd at most once and only for relative paths", "[authoring][ra01]") {
  const ra01_scratch s;
  {
    ra01_counts       counts;
    auto              io = ra01_readers(s.root.string(), (s.root / "wsdir").string(), counts);
    authoring::inputs values;
    values.workspace = s.workspace.string();
    values.registry  = s.registry.string();
    CHECK(authoring::prepare_inputs(authoring::verb::spec_prompt, values, io).has_value());
    CHECK(counts.cwd == 0);
    CHECK(counts.env == 0);
  }
  for (const auto which : {authoring::verb::spec_prompt, authoring::verb::orchestrator}) {
    ra01_counts       counts;
    auto              io = ra01_readers(s.root.string(), (s.root / "wsdir").string(), counts);
    authoring::inputs values;
    values.workspace  = "ws/workspace.lua";
    values.registry   = "registry.json";
    const auto result = authoring::prepare_inputs(which, values, io);
    CHECK(result.has_value());
    CHECK(counts.cwd == 1);
    CHECK(values.workspace == s.workspace.string());
    CHECK(values.registry == s.registry.string());
  }
  {
    ra01_counts       counts;
    auto              io = ra01_readers(s.root.string(), (s.root / "wsdir").string(), counts);
    authoring::inputs values;
    values.workspace = s.workspace.string();
    values.registry  = "registry.json";
    CHECK(authoring::prepare_inputs(authoring::verb::critique, values, io).has_value());
    CHECK(counts.cwd == 1);
    CHECK(values.registry == s.registry.string());
  }
  {
    ra01_counts       counts;
    auto              io = ra01_readers(s.root.string(), (s.root / "wsdir").string(), counts);
    authoring::inputs values;
    values.from = "obs";
    CHECK(authoring::prepare_inputs(authoring::verb::observe, values, io).has_value());
    CHECK(counts.cwd == 1);
    CHECK(values.from == s.observations.string());
  }
}

TEST_CASE("RA01-B1 explain-rejection and observe ignore an invalid workspace environment", "[authoring][ra01]") {
  const ra01_scratch               s;
  const std::optional<std::string> relative{"relative/dir"};
  const auto                       diagnostic = *std::ranges::find(ra01_verbs(), "explain-rejection", &ra01_verb::name);
  const auto                       observe    = *std::ranges::find(ra01_verbs(), "observe", &ra01_verb::name);
  CHECK(ra01_stack(s, s.full(diagnostic), relative) == ra01_stub());
  CHECK(ra01_stack(s, s.full(observe), relative) == ra01_stub());
  // A bare workspace name reads the environment and refuses its relative value as usage.
  const auto spec = *std::ranges::find(ra01_verbs(), "spec-prompt", &ra01_verb::name);
  const auto got  = ra01_stack(s, s.with(spec, "--workspace", "ws"), relative);
  CHECK(got.status == 2);
  CHECK(got.out.empty());
  CHECK(got.err.contains("\"code\":\"usage\""));
  // Controls: a valid directory or an explicit workspace file does not refuse.
  CHECK(ra01_stack(s, s.with(spec, "--workspace", "ws"), (s.root / "wsdir").string()) == ra01_stub());
  CHECK(ra01_stack(s, s.full(spec), relative) == ra01_stub());
}

TEST_CASE("RA01-B1 explicit path stage keeps the existing messages", "[authoring][ra01]") {
  const ra01_scratch s;
  ra01_counts        counts;
  const auto         io = ra01_readers(s.root.string(), (s.root / "wsdir").string(), counts);
  fs::create_symlink(s.diagnostic, s.root / "link.json");
  fs::create_directories(s.root / "dir.json");
  fs::create_directory_symlink(s.observations, s.root / "obs-link");
  const auto missing = (s.root / "missing").string();
  struct row {
    authoring::verb  which;
    std::string      field;
    std::string      value;
    std::string_view code;
    std::string_view message;
  };
  const std::vector<row> rows{
      {authoring::verb::explain_rejection, "diagnostic", missing, "io", "Cannot resolve explicit input path"},
      {authoring::verb::explain_rejection, "diagnostic", (s.root / "link.json").string(), "path_denied",
       "Explicit input file is a symbolic link"},
      {authoring::verb::explain_rejection, "diagnostic", (s.root / "dir.json").string(), "io",
       "Explicit input must be a regular file"},
      {authoring::verb::spec_prompt, "registry", missing, "io", "Explicit input must be a regular file"},
      {authoring::verb::critique, "registry", (s.root / "link.json").string(), "path_denied",
       "Explicit input file is a symbolic link"},
      {authoring::verb::ingest, "registry", (s.root / "dir.json").string(), "io", "Explicit input must be a regular file"},
      {authoring::verb::orchestrator, "registry", missing, "io", "Explicit input must be a regular file"},
      {authoring::verb::observe, "from", missing, "io", "Cannot resolve explicit root directory"},
      {authoring::verb::observe, "from", s.registry.string(), "io", "Explicit root must be a directory"},
      {authoring::verb::observe, "from", (s.root / "obs-link").string(), "path_denied",
       "Explicit root is a symbolic link target"},
  };
  for (const auto& r : rows) {
    CAPTURE(r.field, r.value);
    authoring::inputs values;
    values.workspace = s.workspace.string();
    values.registry  = s.registry.string();
    if (r.field == "diagnostic")
      values.diagnostic = r.value;
    else if (r.field == "registry")
      values.registry = r.value;
    else
      values.from = r.value;
    if (r.which == authoring::verb::explain_rejection)
      values.workspace.clear();
    const auto result = authoring::prepare_inputs(r.which, values, io);
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().code == r.code);
    CHECK(result.error().message == r.message);
  }
  // Controls: the valid spellings pass the same stage.
  authoring::inputs valid;
  valid.diagnostic = s.diagnostic.string();
  CHECK(authoring::prepare_inputs(authoring::verb::explain_rejection, valid, io).has_value());
  authoring::inputs valid_from;
  valid_from.from = s.observations.string();
  CHECK(authoring::prepare_inputs(authoring::verb::observe, valid_from, io).has_value());
}

// Kills: registry call site passing true for remap_missing_leaf (C-R3-REG); diagnostic call site passing false (C-R3-DIAG);
// deleting the directory-branch mapping to the explicit root message (C-R3-FROM). One missing path, differential results.
TEST_CASE("RA01-B1 missing leaf remap applies only to diagnostic and from", "[authoring][ra01]") {
  const ra01_scratch s;
  const auto         missing  = (s.root / "missing").string();
  const ra01_bytes   registry = ra01_native("io", "Explicit input must be a regular file");
  const ra01_bytes   diag     = ra01_native("io", "Cannot resolve explicit input path");
  const ra01_bytes   root     = ra01_native("io", "Cannot resolve explicit root directory");
  CHECK_FALSE(registry == diag);
  CHECK_FALSE(registry == root);
  CHECK_FALSE(diag == root);
  ra01_counts counts;
  const auto  io = ra01_readers(s.root.string(), (s.root / "wsdir").string(), counts);
  for (const auto& verb : ra01_verbs()) {
    CAPTURE(verb.name);
    for (const auto& option : verb.options) {
      if (option == "--workspace" || option == "--job")
        continue;
      CAPTURE(option);
      const auto got = ra01_stack(s, s.with(verb, option, missing));
      CHECK(got == (option == "--registry" ? registry : option == "--diagnostic" ? diag : root));
    }
  }
  struct row {
    authoring::verb  which;
    std::string_view field;
    std::string_view message;
  };
  const std::vector<row> rows{
      {authoring::verb::spec_prompt, "registry", "Explicit input must be a regular file"},
      {authoring::verb::critique, "registry", "Explicit input must be a regular file"},
      {authoring::verb::ingest, "registry", "Explicit input must be a regular file"},
      {authoring::verb::orchestrator, "registry", "Explicit input must be a regular file"},
      {authoring::verb::explain_rejection, "diagnostic", "Cannot resolve explicit input path"},
      {authoring::verb::observe, "from", "Cannot resolve explicit root directory"},
  };
  std::set<std::string> distinct;
  for (const auto& r : rows) {
    CAPTURE(r.field);
    authoring::inputs values;
    values.workspace = s.workspace.string();
    values.registry  = s.registry.string();
    if (r.field == "diagnostic")
      values.diagnostic = missing;
    else if (r.field == "registry")
      values.registry = missing;
    else
      values.from = missing;
    if (r.which == authoring::verb::explain_rejection || r.which == authoring::verb::observe)
      values.workspace.clear();
    const auto result = authoring::prepare_inputs(r.which, values, io);
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().code == "io");
    CHECK(result.error().message == r.message);
    distinct.insert(result.error().message);
  }
  CHECK(distinct.size() == 3);
  // A leaf below a regular file fails symlink_status with an error code; it is still an absent leaf.
  {
    authoring::inputs values;
    values.diagnostic = (s.registry / "leaf").string();
    const auto result = authoring::prepare_inputs(authoring::verb::explain_rejection, values, io);
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().message == "Cannot resolve explicit input path");
  }
  // Narrowing: a present directory or regular file keeps the unmapped messages.
  fs::create_directories(s.root / "dir.json");
  {
    authoring::inputs values;
    values.diagnostic = (s.root / "dir.json").string();
    const auto result = authoring::prepare_inputs(authoring::verb::explain_rejection, values, io);
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().message == "Explicit input must be a regular file");
  }
  {
    authoring::inputs values;
    values.from       = s.registry.string();
    const auto result = authoring::prepare_inputs(authoring::verb::observe, values, io);
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().message == "Explicit root must be a directory");
  }
}

// Kills E08 only: deleting the validate_host_path loop in prepare_explicit. The workspace rows are a behaviour pin: the E07
// mutant (validate_host_path deleted in prepare_selector) is EQUIVALENT because engine::expand_workspace validates explicit
// selectors itself (host_path.cpp:201-204) and named selectors come from a directory already checked at :88-93.
TEST_CASE("RA01-B1 path text validation precedes the cwd read", "[authoring][ra01]") {
  const ra01_scratch s;
  const std::string  bad     = "bad/\xff.lua";
  const auto         invalid = ra01_native("path_denied", "Invalid explicit input path");
  for (const auto which : {authoring::verb::spec_prompt, authoring::verb::orchestrator}) {
    {
      ra01_counts       counts;
      const auto        io = ra01_readers(s.root.string(), (s.root / "wsdir").string(), counts);
      authoring::inputs values;
      values.workspace  = s.workspace.string();
      values.registry   = bad;
      const auto result = authoring::prepare_inputs(which, values, io);
      REQUIRE_FALSE(result.has_value());
      CHECK(result.error().code == "path_denied");
      CHECK(result.error().message == "Invalid explicit input path");
      CHECK(counts.cwd == 0);
    }
    {
      // Behaviour pin only (E07 equivalent).
      ra01_counts       counts;
      const auto        io = ra01_readers(s.root.string(), (s.root / "wsdir").string(), counts);
      authoring::inputs values;
      values.workspace  = bad;
      values.registry   = s.registry.string();
      const auto result = authoring::prepare_inputs(which, values, io);
      REQUIRE_FALSE(result.has_value());
      CHECK(result.error().code == "path_denied");
      CHECK(result.error().message == "Invalid explicit input path");
      CHECK(counts.cwd == 0);
    }
    {
      // Control: valid relative spellings read the cwd exactly once.
      ra01_counts       counts;
      const auto        io = ra01_readers(s.root.string(), (s.root / "wsdir").string(), counts);
      authoring::inputs values;
      values.workspace  = "ws/workspace.lua";
      values.registry   = "registry.json";
      const auto result = authoring::prepare_inputs(which, values, io);
      CHECK(result.has_value());
      CHECK(counts.cwd == 1);
    }
  }
  {
    ra01_counts       counts;
    const auto        io = ra01_readers(s.root.string(), (s.root / "wsdir").string(), counts);
    authoring::inputs values;
    values.diagnostic = bad;
    const auto result = authoring::prepare_inputs(authoring::verb::explain_rejection, values, io);
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().code == "path_denied");
    CHECK(result.error().message == "Invalid explicit input path");
    CHECK(counts.cwd == 0);
  }
  {
    // --from is validated before the cwd read too.
    ra01_counts       counts;
    const auto        io = ra01_readers(s.root.string(), (s.root / "wsdir").string(), counts);
    authoring::inputs values;
    values.from       = bad;
    const auto result = authoring::prepare_inputs(authoring::verb::observe, values, io);
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().code == "path_denied");
    CHECK(result.error().message == "Invalid explicit input path");
    CHECK(counts.cwd == 0);
  }
  {
    ra01_counts       counts;
    const auto        io = ra01_readers(s.root.string(), (s.root / "wsdir").string(), counts);
    authoring::inputs values;
    values.from = "from-dir";
    (void)authoring::prepare_inputs(authoring::verb::observe, values, io);
    CHECK(counts.cwd == 1);
  }
  {
    ra01_counts       counts;
    const auto        io = ra01_readers(s.root.string(), (s.root / "wsdir").string(), counts);
    authoring::inputs values;
    values.diagnostic = "diag.json";
    CHECK(authoring::prepare_inputs(authoring::verb::explain_rejection, values, io).has_value());
    CHECK(counts.cwd == 1);
  }
  CHECK(invalid.status == 3);
}

TEST_CASE("RA01-B1 workspace selector stage expands names and keeps files lexical", "[authoring][ra01]") {
  const ra01_scratch s;
  const auto         directory = s.root / "wsdir";
  {
    ra01_counts       counts;
    const auto        io = ra01_readers(s.root.string(), directory.string(), counts);
    authoring::inputs values;
    values.workspace = "ws";
    values.registry  = s.registry.string();
    REQUIRE(authoring::prepare_inputs(authoring::verb::spec_prompt, values, io).has_value());
    CHECK(values.workspace == (directory / "ws" / "workspace.lua").string());
    CHECK(values.registry == s.registry.string());
    CHECK_FALSE(fs::exists(values.workspace));
  }
  {
    // An explicit relative file is made absolute with the cwd snapshot and is not required to exist.
    ra01_counts       counts;
    const auto        io = ra01_readers(s.root.string(), directory.string(), counts);
    authoring::inputs values;
    values.workspace = "sub/ws.lua";
    values.registry  = s.registry.string();
    REQUIRE(authoring::prepare_inputs(authoring::verb::ingest, values, io).has_value());
    CHECK(values.workspace == (s.root / "sub" / "ws.lua").string());
    CHECK_FALSE(fs::exists(values.workspace));
    CHECK(counts.env == 0);
    CHECK(counts.cwd == 1);
  }
  {
    // An absolute nonexistent file is kept as spelled.
    ra01_counts       counts;
    const auto        io = ra01_readers(s.root.string(), directory.string(), counts);
    authoring::inputs values;
    values.workspace = (s.root / "nope" / "workspace.lua").string();
    values.registry  = s.registry.string();
    REQUIRE(authoring::prepare_inputs(authoring::verb::orchestrator, values, io).has_value());
    CHECK(values.workspace == (s.root / "nope" / "workspace.lua").string());
  }
}
