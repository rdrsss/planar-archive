// RA01-B2 prompt tests (package T-PR, round V5-02-RA01-B-W2): spec-prompt, critique, ingest and explain-rejection through the
// authoring stack. Every test drives the real verbs over per-test temporary trees and states, in a comment, the mutant of the
// invariant table (tests/native/v5-02-ra01-b-w2-evidence/plan/INVARIANTS.md) that it kills. The tests never apply mutants.
// Expected values are typed from the authoring contract and the pinned decisions D-15 to D-21; prose bodies are not pinned.
#include <catch2/catch_test_macros.hpp>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
import std;
import planar.cmd.authoring_stack.bootstrap;
import planar.engine.hash;

#ifndef PLANAR_B2_WORKSPACE_FIXTURE
#error "PLANAR_B2_WORKSPACE_FIXTURE is defined for the command tests"
#endif
#ifndef PLANAR_PRODUCTION_BINARY
#error "PLANAR_PRODUCTION_BINARY is defined for the command tests"
#endif

namespace {
namespace fs = std::filesystem;

// ---- Minimal JSON reader (test only; never throws, a malformed text parses to the `none` kind) -------------------------

struct w2p_json {
  enum class kind { none, null, boolean, number, string, array, object };
  kind                     k       = kind::none;
  bool                     b       = false;
  bool                     integer = false;
  long long                n       = 0;
  std::string              s;
  std::vector<w2p_json>    items;
  std::vector<std::string> names; ///< Object member names, parallel to `items`.
  bool                     ok() const {
    return k != kind::none;
  }
  const w2p_json&          at(std::string_view key) const;
  const w2p_json&          at(std::size_t index) const;
  bool                     has(std::string_view key) const;
  std::vector<std::string> keys() const {
    return names;
  }
};
const w2p_json& w2p_none() {
  static const w2p_json value;
  return value;
}
const w2p_json& w2p_json::at(std::string_view key) const {
  if (k == kind::object)
    for (std::size_t i = 0; i < names.size(); ++i)
      if (names[i] == key)
        return items[i];
  return w2p_none();
}
const w2p_json& w2p_json::at(std::size_t index) const {
  if (k == kind::array && index < items.size())
    return items[index];
  return w2p_none();
}
bool w2p_json::has(std::string_view key) const {
  return k == kind::object && std::ranges::find(names, key) != names.end();
}

class w2p_parser {
public:
  explicit w2p_parser(std::string_view text) : _t(text) {
  }
  std::optional<w2p_json> run() {
    skip();
    auto v = value(0);
    if (!v)
      return std::nullopt;
    skip();
    if (_i != _t.size())
      return std::nullopt;
    return v;
  }

private:
  std::string_view _t;
  std::size_t      _i = 0;
  void             skip() {
    while (_i < _t.size() && (_t[_i] == ' ' || _t[_i] == '\n' || _t[_i] == '\t' || _t[_i] == '\r'))
      ++_i;
  }
  static void utf8(std::string& out, unsigned cp) {
    if (cp < 0x80)
      out += static_cast<char>(cp);
    else if (cp < 0x800) {
      out += static_cast<char>(0xC0 | (cp >> 6));
      out += static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
      out += static_cast<char>(0xE0 | (cp >> 12));
      out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
      out += static_cast<char>(0x80 | (cp & 0x3F));
    } else {
      out += static_cast<char>(0xF0 | (cp >> 18));
      out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
      out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
      out += static_cast<char>(0x80 | (cp & 0x3F));
    }
  }
  std::optional<unsigned> hex4() {
    if (_i + 4 > _t.size())
      return std::nullopt;
    unsigned v = 0;
    for (int d = 0; d < 4; ++d) {
      const char c = _t[_i++];
      v <<= 4;
      if (c >= '0' && c <= '9')
        v |= static_cast<unsigned>(c - '0');
      else if (c >= 'a' && c <= 'f')
        v |= static_cast<unsigned>(c - 'a' + 10);
      else if (c >= 'A' && c <= 'F')
        v |= static_cast<unsigned>(c - 'A' + 10);
      else
        return std::nullopt;
    }
    return v;
  }
  std::optional<std::string> string() {
    if (_i >= _t.size() || _t[_i] != '"')
      return std::nullopt;
    ++_i;
    std::string out;
    while (_i < _t.size()) {
      const char c = _t[_i++];
      if (c == '"')
        return out;
      if (c != '\\') {
        out += c;
        continue;
      }
      if (_i >= _t.size())
        return std::nullopt;
      const char e = _t[_i++];
      switch (e) {
      case '"':
        out += '"';
        break;
      case '\\':
        out += '\\';
        break;
      case '/':
        out += '/';
        break;
      case 'b':
        out += '\b';
        break;
      case 'f':
        out += '\f';
        break;
      case 'n':
        out += '\n';
        break;
      case 'r':
        out += '\r';
        break;
      case 't':
        out += '\t';
        break;
      case 'u': {
        auto hi = hex4();
        if (!hi)
          return std::nullopt;
        unsigned cp = *hi;
        if (cp >= 0xD800 && cp < 0xDC00 && _i + 1 < _t.size() && _t[_i] == '\\' && _t[_i + 1] == 'u') {
          _i += 2;
          auto lo = hex4();
          if (!lo)
            return std::nullopt;
          cp = 0x10000 + ((cp - 0xD800) << 10) + (*lo - 0xDC00);
        }
        utf8(out, cp);
        break;
      }
      default:
        return std::nullopt;
      }
    }
    return std::nullopt;
  }
  std::optional<w2p_json> value(int depth) {
    if (depth > 64 || _i >= _t.size())
      return std::nullopt;
    w2p_json   v;
    const char c = _t[_i];
    if (c == '{') {
      ++_i;
      v.k = w2p_json::kind::object;
      skip();
      if (_i < _t.size() && _t[_i] == '}') {
        ++_i;
        return v;
      }
      while (true) {
        skip();
        auto name = string();
        if (!name)
          return std::nullopt;
        skip();
        if (_i >= _t.size() || _t[_i++] != ':')
          return std::nullopt;
        skip();
        auto member = value(depth + 1);
        if (!member)
          return std::nullopt;
        v.names.push_back(std::move(*name));
        v.items.push_back(std::move(*member));
        skip();
        if (_i >= _t.size())
          return std::nullopt;
        if (_t[_i] == ',') {
          ++_i;
          continue;
        }
        if (_t[_i] == '}') {
          ++_i;
          return v;
        }
        return std::nullopt;
      }
    }
    if (c == '[') {
      ++_i;
      v.k = w2p_json::kind::array;
      skip();
      if (_i < _t.size() && _t[_i] == ']') {
        ++_i;
        return v;
      }
      while (true) {
        skip();
        auto item = value(depth + 1);
        if (!item)
          return std::nullopt;
        v.items.push_back(std::move(*item));
        skip();
        if (_i >= _t.size())
          return std::nullopt;
        if (_t[_i] == ',') {
          ++_i;
          continue;
        }
        if (_t[_i] == ']') {
          ++_i;
          return v;
        }
        return std::nullopt;
      }
    }
    if (c == '"') {
      auto text = string();
      if (!text)
        return std::nullopt;
      v.k = w2p_json::kind::string;
      v.s = std::move(*text);
      return v;
    }
    for (const auto& [word, kind] : {std::pair<std::string_view, w2p_json::kind>{"true", w2p_json::kind::boolean},
                                     {"false", w2p_json::kind::boolean},
                                     {"null", w2p_json::kind::null}}) {
      if (_t.substr(_i).starts_with(word)) {
        _i += word.size();
        v.k = kind;
        v.b = word == "true";
        return v;
      }
    }
    const auto start = _i;
    if (_i < _t.size() && _t[_i] == '-')
      ++_i;
    bool digits = false, plain = true;
    while (_i < _t.size() && _t[_i] >= '0' && _t[_i] <= '9') {
      ++_i;
      digits = true;
    }
    while (_i < _t.size() && (_t[_i] == '.' || _t[_i] == 'e' || _t[_i] == 'E' || _t[_i] == '+' || _t[_i] == '-' ||
                              (_t[_i] >= '0' && _t[_i] <= '9'))) {
      plain = false;
      ++_i;
    }
    if (!digits)
      return std::nullopt;
    v.k       = w2p_json::kind::number;
    v.integer = plain;
    if (plain)
      std::from_chars(_t.data() + start, _t.data() + _i, v.n);
    return v;
  }
};
w2p_json w2p_parse(std::string_view text) {
  auto v = w2p_parser(text).run();
  return v ? std::move(*v) : w2p_json{};
}

// ---- Process and stack plumbing -----------------------------------------------------------------------------------

struct w2p_out {
  int         status = -1;
  std::string out, err;
};
struct w2p_cwd {
  fs::path saved;
  explicit w2p_cwd(const fs::path& to) : saved(fs::current_path()) {
    fs::current_path(to);
    ::unsetenv("PLANAR_WORKSPACE_DIR");
  }
  w2p_cwd(const w2p_cwd&)            = delete;
  w2p_cwd& operator=(const w2p_cwd&) = delete;
  ~w2p_cwd() {
    std::error_code ignored;
    fs::current_path(saved, ignored);
  }
};
w2p_out w2p_stack(const fs::path& cwd, const std::vector<std::string>& args) {
  w2p_cwd                       guard(cwd);
  std::vector<std::string_view> views(args.begin(), args.end());
  const auto                    r = planar::cmd::authoring_stack::invoke(views);
  return {r.status, r.output, r.error};
}
std::string w2p_quoted(std::string_view text) {
  std::string out = "'";
  for (const char c : text) {
    if (c == '\'')
      out += "'\\''";
    else
      out += c;
  }
  return out + "'";
}
std::string w2p_slurp(const fs::path& path) {
  std::ifstream in(path, std::ios::binary);
  return std::string{std::istreambuf_iterator<char>(in), {}};
}
/// The generated authoring stack binary sits beside the production one in the build tree (bin/ and src/cmd/planar/).
fs::path w2p_stack_binary() {
  return fs::path(PLANAR_PRODUCTION_BINARY).parent_path().parent_path() / "src/cmd/planar/planar_authoring_stack";
}
/// One fresh process of the authoring stack; its output files live beside the tree root under `tag`.
w2p_out w2p_spawn(const fs::path& root, const fs::path& cwd, const std::vector<std::string>& args, const std::string& tag) {
  const auto binary = w2p_stack_binary();
  REQUIRE(fs::exists(binary));
  const auto  out_file = root / ("spawn-" + tag + ".out");
  const auto  err_file = root / ("spawn-" + tag + ".err");
  std::string command  = "cd " + w2p_quoted(cwd.string()) + " && env -u PLANAR_WORKSPACE_DIR " + w2p_quoted(binary.string());
  for (const auto& token : args)
    command += " " + w2p_quoted(token);
  command += " > " + w2p_quoted(out_file.string()) + " 2> " + w2p_quoted(err_file.string());
  const int status = std::system(command.c_str());
  REQUIRE(WIFEXITED(status));
  return {WEXITSTATUS(status), w2p_slurp(out_file), w2p_slurp(err_file)};
}

// ---- Prompt documents and text helpers ----------------------------------------------------------------------------

struct w2p_doc {
  w2p_out     run;
  w2p_json    env;
  std::string md;
};
w2p_doc w2p_make_doc(w2p_out r) {
  w2p_doc d;
  d.run = std::move(r);
  d.env = w2p_parse(d.run.out);
  d.md  = d.env.at("prompt").at("markdown").s;
  return d;
}
bool w2p_has(std::string_view hay, std::string_view needle) {
  return hay.find(needle) != std::string_view::npos;
}
std::vector<std::string> w2p_lines(std::string_view text) {
  std::vector<std::string> out;
  std::size_t              at = 0;
  while (at < text.size()) {
    const auto end = text.find('\n', at);
    if (end == std::string_view::npos) {
      out.emplace_back(text.substr(at));
      break;
    }
    out.emplace_back(text.substr(at, end - at));
    at = end + 1;
  }
  return out;
}
std::string w2p_trim(std::string_view text) {
  while (!text.empty() && (text.front() == '\n' || text.front() == ' '))
    text.remove_prefix(1);
  while (!text.empty() && (text.back() == '\n' || text.back() == ' '))
    text.remove_suffix(1);
  return std::string(text);
}
/// Heading names ("Authority", ...) in document order, up to and including "Raw error" (the verbatim block ends the scan).
std::vector<std::string> w2p_headings(const std::string& md) {
  std::vector<std::string> out;
  for (const auto& line : w2p_lines(md))
    if (line.starts_with("## ")) {
      out.push_back(line.substr(3));
      if (out.back() == "Raw error")
        break;
    }
  return out;
}
/// Body of one section: after its heading line, up to the blank line before the next heading or the final LF.
std::optional<std::string> w2p_section(const std::string& md, std::string_view heading) {
  const std::string marker = "\n## " + std::string(heading) + "\n";
  const auto        at     = md.find(marker);
  if (at == std::string::npos)
    return std::nullopt;
  const auto start = at + marker.size();
  const auto next  = md.find("\n\n## ", start);
  const auto end   = next == std::string::npos ? md.size() - 1 : next;
  if (end < start)
    return std::string{};
  return md.substr(start, end - start);
}
std::size_t w2p_indent(std::string_view line) {
  std::size_t n = 0;
  while (n < line.size() && line[n] == ' ')
    ++n;
  return n;
}
/// Entries of inputs.files as sorted (root, path) pairs in document order.
std::vector<std::pair<std::string, std::string>> w2p_input_files(const w2p_doc& d) {
  std::vector<std::pair<std::string, std::string>> out;
  const auto&                                      files = d.env.at("prompt").at("inputs").at("files");
  for (const auto& item : files.items)
    out.emplace_back(item.at("root").s, item.at("path").s);
  return out;
}
bool w2p_raw_controls(std::string_view text) {
  for (std::size_t i = 0; i < text.size(); ++i) {
    const auto c = static_cast<unsigned char>(text[i]);
    if ((c < 32 && c != '\n' && c != '\t' && c != '\r') || c == 127)
      return true;
    if (c == 0xC2 && i + 1 < text.size() && static_cast<unsigned char>(text[i + 1]) >= 0x80 &&
        static_cast<unsigned char>(text[i + 1]) <= 0x9F)
      return true;
  }
  return false;
}
std::string w2p_native_error(std::string_view code, std::string_view message) {
  // io refusals of the path stage carry the capture operation detail; path_denied has none.
  const std::string details = code == "io" ? ",\"details\":{\"operation\":\"capture\"}" : "";
  return "{\"error\":{\"code\":\"" + std::string(code) + "\"" + details + ",\"field\":\"\",\"message\":\"" +
         std::string(message) + "\"},\"kind\":\"error\",\"version\":1}\n";
}
std::string w2p_replace(std::string text, std::string_view from, std::string_view to) {
  const auto at = text.find(from);
  REQUIRE(at != std::string::npos);
  text.replace(at, from.size(), to);
  return text;
}

// ---- Fixture trees -------------------------------------------------------------------------------------------------

struct w2p_tree {
  fs::path root, ws, work, cur, registry;
  w2p_tree() {
    char pattern[] = "/tmp/planar-w2pr-XXXXXX";
    REQUIRE(::mkdtemp(pattern));
    root     = fs::canonical(pattern);
    ws       = root / "ws";
    work     = root / "work";
    cur      = root / "cur";
    registry = root / "registry.json";
    fs::create_directories(ws);
    fs::create_directories(work);
    fs::create_directories(cur);
  }
  w2p_tree(const w2p_tree&)            = delete;
  w2p_tree& operator=(const w2p_tree&) = delete;
  ~w2p_tree() {
    std::error_code ignored;
    fs::remove_all(root, ignored);
  }
  static void put(const fs::path& path, std::string_view bytes) {
    fs::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << bytes;
    REQUIRE(out.good());
  }
  static void edit(const fs::path& path, std::string_view from, std::string_view to) {
    put(path, w2p_replace(w2p_slurp(path), from, to));
  }
  /// The shipped quota fixture (workspace, spec, two jobs, text files, registry), copied into the tree.
  void quota() {
    const auto src = fs::path(PLANAR_B2_WORKSPACE_FIXTURE).parent_path().parent_path();
    fs::copy(src / "workspace", ws, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    fs::copy_file(src / "registry/quota-tools.json", registry, fs::copy_options::overwrite_existing);
  }
  fs::path workspace() const {
    return ws / "workspace.lua";
  }
  fs::path diagnostic() const {
    return root / "diag.json";
  }
  std::vector<std::string> prompt_args(std::string_view verb) const {
    return {std::string(verb), "--workspace", workspace().string(), "--registry", registry.string()};
  }
  std::vector<std::string> orchestrator_args(std::string_view job) const {
    auto args = prompt_args("orchestrator");
    args.push_back("--job");
    args.push_back(std::string(job));
    return args;
  }
  std::vector<std::string> reject_args() const {
    return {"explain-rejection", "--diagnostic", diagnostic().string()};
  }
  w2p_out run(const std::vector<std::string>& args) const {
    return w2p_stack(work, args);
  }
  w2p_doc prompt(std::string_view verb) const {
    return w2p_make_doc(run(prompt_args(verb)));
  }
  w2p_out reject_raw(std::string_view bytes) const {
    put(diagnostic(), bytes);
    return run(reject_args());
  }
  w2p_doc reject(std::string_view bytes) const {
    return w2p_make_doc(reject_raw(bytes));
  }
  /// init plus load in a scratch ledger inside the tree; returns the load response.
  w2p_out load(std::string_view name, std::string_view workspace_id = "quota") const {
    const auto ledger = (root / (std::string(name) + ".db")).string();
    const auto init   = run({"init", "--ledger", ledger, "--workspace-id", std::string(workspace_id)});
    REQUIRE(init.status == 0);
    return run({"load", "--ledger", ledger, "--workspace", workspace().string(), "--registry", registry.string(),
                "--expect-revision", "none", "--request-id", "k", "--actor", "a"});
  }
  w2p_out render(std::string_view job, std::string_view role) const {
    return run({"render", "--workspace", workspace().string(), "--registry", registry.string(), "--root",
                "source=" + cur.string(), "--job", std::string(job), "--role", std::string(role)});
  }
};

// Generated fixtures: registry, workspace, spec and job texts built from small parameter structs.
struct w2p_check {
  std::string id;
  std::string inputs    = R"("expected_contracts","own_output")";
  std::string selection = "nonempty";
  std::string coverage;
};
struct w2p_unit {
  std::string              id;
  std::vector<std::string> checks;
  std::vector<std::string> paths; ///< Empty means one directory named after the unit.
};
std::string w2p_registry_text(const std::vector<w2p_check>& checks) {
  std::string out =
      R"({"version":1,"id":"gen_tools","revision":"v1","toolchains":[{"id":"python","description":"Fixture toolchain.","command":["python3","-B"],"environment":[]}],"checks":[)";
  bool first = true;
  for (const auto& c : checks) {
    if (!first)
      out += ",";
    first = false;
    out += std::format(
        R"({{"id":"{}","toolchain":"python","argv":["-m","unittest"],"coverage":"{}","inputs":[{}],"timeout_ms":10000,"max_output_bytes":65536,"selection":"{}"}})",
        c.id, c.coverage.empty() ? "Covers " + c.id + "." : c.coverage, c.inputs, c.selection);
  }
  return out + "]}\n";
}
std::string w2p_lua_list(const std::vector<std::string>& ids) {
  std::string out = "L { ";
  for (std::size_t i = 0; i < ids.size(); ++i)
    out += std::format("{}\"{}\"", i ? ", " : "", ids[i]);
  return out + " }";
}
/// Two jobs are listed: the named one and a never-written "spare" (a single-job workspace trips a load quirk, see the report).
std::string w2p_workspace_text(const std::vector<std::string>& allow, const std::string& job_id) {
  return "local p = planar\nlocal L = p.list\nreturn p.workspace {\n  version = 1, id = \"gen\",\n"
         "  root = { id = \"source\", base = \"initial\" },\n  spec = \"spec.lua\",\n"
         "  jobs = L { { id = \"" +
         job_id + "\", file = \"" + job_id +
         ".lua\" }, { id = \"spare\", file = \"spare.lua\" } },\n  registry = { id = \"gen_tools\", revision = \"v1\" },\n  "
         "toolchains = L { \"python\" }, checks = " +
         w2p_lua_list(allow) + "\n}\n";
}
std::string w2p_spec_text(const std::vector<w2p_unit>& units, const std::string& extra = "",
                          const std::string& goal = "p.md \"Provide the goal.\"") {
  std::string out =
      "local p = planar\nlocal L = p.list\nreturn p.spec {\n  version = 1, id = \"gen\",\n  goal = " + goal + ",\n  work = L {\n";
  std::vector<std::string> ids;
  for (const auto& u : units) {
    ids.push_back(u.id);
    const auto paths = u.paths.empty() ? std::vector<std::string>{"d_" + u.id + "/"} : u.paths;
    out += "    { id = \"" + u.id + "\", summary = p.md \"Unit " + u.id + ".\", paths = " + w2p_lua_list(paths) +
           ", checks = " + w2p_lua_list(u.checks) + " },\n";
  }
  out += "  },\n  milestones = L { { id = \"m1\", work = " + w2p_lua_list(ids) +
         ", acceptance = L { { id = \"a1\", text = p.md \"Done.\" } } } },\n" + extra + "}\n";
  return out;
}
std::string w2p_job_text(const std::string& id, const std::vector<std::string>& work) {
  return "local p = planar\nlocal L = p.list\nreturn p.job {\n  version = 1, id = \"" + id +
         "\", shape = \"fanout_review\", budget_bytes = 131072,\n  roles = L {\n    { id = \"builder\", kind = \"worker\", work "
         "= " +
         w2p_lua_list(work) + " },\n    { id = \"reviewer\", kind = \"reviewer\", targets = " + w2p_lua_list(work) +
         " }\n  }\n}\n";
}
struct w2p_gen {
  std::vector<w2p_check>   registry;
  std::vector<std::string> allow;
  std::string              spec;
  std::string              job_id = "j1";
  std::string              job_text; ///< Empty: the job file is listed but never written.
};
void w2p_install(w2p_tree& t, const w2p_gen& g) {
  w2p_tree::put(t.workspace(), w2p_workspace_text(g.allow, g.job_id));
  w2p_tree::put(t.ws / "spec.lua", g.spec);
  if (!g.job_text.empty())
    w2p_tree::put(t.ws / (g.job_id + ".lua"), g.job_text);
  w2p_tree::put(t.registry, w2p_registry_text(g.registry));
}
std::vector<std::string> w2p_check_ids(const std::vector<w2p_check>& checks) {
  std::vector<std::string> out;
  for (const auto& c : checks)
    out.push_back(c.id);
  return out;
}

// Critique fixture checks: nonempty without output_vector, not_applicable, and nonempty with output_vector.
std::vector<w2p_check> w2p_critique_registry() {
  return {{"ck_ne", R"("expected_contracts","own_output")", "nonempty", ""},
          {"ck_na", R"("expected_contracts","own_output")", "not_applicable", ""},
          {"ck_ov", R"("own_output","output_vector")", "nonempty", ""}};
}
w2p_gen w2p_critique_gen(const std::vector<w2p_unit>& units, const std::string& extra) {
  w2p_gen g;
  g.registry = w2p_critique_registry();
  g.allow    = w2p_check_ids(g.registry);
  g.spec     = w2p_spec_text(units, extra);
  return g;
}
/// Lines of the Findings section; each non-empty line is one finding or the fixed line.
std::vector<std::string> w2p_findings(const w2p_doc& d) {
  const auto               section = w2p_section(d.md, "Findings");
  std::vector<std::string> out;
  if (section)
    for (auto& line : w2p_lines(*section))
      if (!line.empty())
        out.push_back(line);
  return out;
}
/// "- WORD ID" or "- WORD ID: ..." lines of a section as their ID tokens.
std::vector<std::string> w2p_ids_of(const std::string& section, std::string_view word) {
  std::vector<std::string> out;
  const std::string        prefix = "- " + std::string(word) + " ";
  for (const auto& line : w2p_lines(section))
    if (line.starts_with(prefix)) {
      auto token = line.substr(prefix.size());
      token      = token.substr(0, token.find_first_of(": "));
      out.push_back(token);
    }
  return out;
}
std::set<std::pair<std::string, std::string>> w2p_file_set(const w2p_doc& d) {
  std::set<std::pair<std::string, std::string>> out;
  for (auto [root, path] : w2p_input_files(d))
    out.insert({root, root == "registry" ? "*" : path});
  return out;
}
std::string w2p_diag_error(std::string_view code, std::string_view field, std::string_view message) {
  return "{\"error\":{\"code\":\"" + std::string(code) + "\",\"field\":\"" + std::string(field) + "\",\"message\":\"" +
         std::string(message) + "\"},\"kind\":\"error\",\"version\":1}\n";
}
std::string w2p_diag_base() {
  return "{\"error\":{\"code\":\"schema\",\"field\":\"spec.goal\",\"message\":\"Goal must be "
         "nonempty\"},\"kind\":\"error\",\"version\":1}";
}
/// Raw block of a rejection prompt: fence line and the bytes between the opening and closing fence lines.
struct w2p_raw_block {
  bool        found = false;
  std::string fence, data;
};
w2p_raw_block w2p_raw_error(const std::string& md) {
  w2p_raw_block     block;
  const std::string marker = "\n## Raw error\n";
  const auto        at     = md.find(marker);
  if (at == std::string::npos)
    return block;
  const auto label_end = md.find('\n', at + marker.size());
  if (label_end == std::string::npos)
    return block;
  const auto fence_end = md.find('\n', label_end + 1);
  if (fence_end == std::string::npos)
    return block;
  block.fence        = md.substr(label_end + 1, fence_end - label_end - 1);
  const auto closing = block.fence + "\n";
  if (md.size() < closing.size() || !md.ends_with(closing) || md.size() - closing.size() < fence_end + 1)
    return block;
  block.data  = md.substr(fence_end + 1, md.size() - closing.size() - (fence_end + 1));
  block.found = true;
  return block;
}
} // namespace

// ---- W2-07, W2-08, W2-09, W2-10 --------------------------------------------------------------------------------------

// W2-07. Mutant killed: invocation.cpp execute() raising the prompt markdown ceiling to 16 MiB or deleting the check.
// Positive controls (a small and a half-megabyte goal succeed) keep the refusal from being a general failure.
TEST_CASE("RA01-B2 an oversize prompt is refused as prompt_bytes", "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  const auto small = t.run(t.prompt_args("ingest"));
  REQUIRE(small.status == 0);
  CHECK(small.err.empty());
  const auto goal = std::string_view("goal = p.md \"Provide an atomic in-memory quota and a small command-line client.\"");
  const auto spec = w2p_slurp(t.ws / "spec.lua");
  const auto grow = [&](std::size_t size) {
    std::string text;
    while (text.size() < size)
      text += std::string(99, 'a') + "\n";
    text.resize(size);
    w2p_tree::put(t.ws / "text/big.md", text);
    w2p_tree::put(t.ws / "spec.lua", w2p_replace(spec, goal, "goal = p.file \"text/big.md\""));
  };
  grow(500000);
  const auto half = w2p_make_doc(t.run(t.prompt_args("ingest")));
  REQUIRE(half.run.status == 0);
  CHECK(half.env.at("prompt").at("bytes").n > 500000);
  CHECK(half.env.at("prompt").at("bytes").n <= 1048576);
  // The goal file alone is at most 1048576 bytes; 1048000 bytes plus the fixed sections exceed the markdown ceiling.
  grow(1048000);
  const auto big = t.run(t.prompt_args("ingest"));
  CHECK(big.status == 3);
  CHECK(big.out.empty());
  CHECK(std::ranges::count(big.err, '\n') == 1);
  const auto e = w2p_parse(big.err);
  CHECK(e.at("kind").s == "error");
  CHECK(e.at("error").at("code").s == "resource_limit");
  CHECK(e.at("error").at("message").s == "Capture limit exceeded");
  CHECK(e.at("error").at("details").at("limit").s == "prompt_bytes");
  CHECK(e.at("error").at("details").at("maximum").n == 1048576);
}

// W2-08. Mutant killed: capture_authoring treating include "workspace" like "spec" (spec-prompt would list the spec files).
TEST_CASE("RA01-B2 spec-prompt captures the workspace without the spec while critique and ingest capture the spec",
          "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  w2p_tree::put(t.ws / "text/wnote.md", "Workspace note.\n");
  w2p_tree::edit(t.workspace(), "notes = p.immaterial(p.md \"The driver supplies stable sibling source snapshots.\")",
                 "notes = p.immaterial(p.file \"text/wnote.md\")");
  using entry = std::pair<std::string, std::string>;
  const std::set<entry> workspace_only{{"workspace", "workspace.lua"}, {"workspace", "text/wnote.md"}, {"registry", "*"}};
  std::set<entry>       with_spec = workspace_only;
  with_spec.insert({"workspace", "spec.lua"});
  with_spec.insert({"workspace", "text/shared.md"});
  with_spec.insert({"workspace", "text/quota-api.md"});
  const auto spec_prompt = t.prompt("spec-prompt");
  REQUIRE(spec_prompt.run.status == 0);
  CHECK(w2p_file_set(spec_prompt) == workspace_only);
  CHECK(w2p_input_files(spec_prompt).size() == 3);
  for (const std::string verb : {"critique", "ingest"}) {
    CAPTURE(verb);
    const auto doc = t.prompt(verb);
    REQUIRE(doc.run.status == 0);
    CHECK(w2p_file_set(doc) == with_spec);
    CHECK(w2p_input_files(doc).size() == with_spec.size());
  }
  // No job file is ever listed, whatever the include.
  for (const std::string verb : {"spec-prompt", "critique", "ingest"})
    for (const auto& [root, path] : w2p_input_files(t.prompt(verb))) {
      CHECK(path != "combined.lua");
      CHECK(path != "split.lua");
    }
}

// W2-09. Mutants killed: capture_authoring loading each listed job like capture_render (io on an absent file), or the
// schema mode spec_without_jobs/workspace ignored (Listed job not captured returns). Both job files are absent.
TEST_CASE("RA01-B2 authoring capture succeeds when a listed job file is absent for all three prompts",
          "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  fs::remove(t.ws / "combined.lua");
  fs::remove(t.ws / "split.lua");
  for (const std::string verb : {"spec-prompt", "critique", "ingest"}) {
    CAPTURE(verb);
    const auto doc = t.prompt(verb);
    CAPTURE(doc.run.err);
    CHECK(doc.run.status == 0);
    CHECK(doc.run.err.empty());
    CHECK(doc.env.at("prompt").at("kind").ok());
  }
  CHECK_FALSE(fs::exists(t.ws / "combined.lua"));
  CHECK_FALSE(fs::exists(t.ws / "split.lua"));
}

// W2-10. Mutant killed: capture_authoring reading a job file when it exists (only skipping an absent one). The prompt over a
// tree with a syntactically invalid present job file is byte-equal to the prompt over a tree without job files.
TEST_CASE("RA01-B2 a present job file is neither read nor listed", "[authoring][ra01][prompts]") {
  w2p_tree absent, present;
  absent.quota();
  present.quota();
  fs::remove(absent.ws / "combined.lua");
  fs::remove(absent.ws / "split.lua");
  w2p_tree::put(present.ws / "combined.lua", "return {{{ this is not lua\n");
  w2p_tree::put(present.ws / "split.lua", "return 42\n");
  for (const std::string verb : {"spec-prompt", "critique", "ingest"}) {
    CAPTURE(verb);
    const auto a = absent.prompt(verb);
    const auto b = present.prompt(verb);
    REQUIRE(a.run.status == 0);
    REQUIRE(b.run.status == 0);
    CHECK(a.run.out == b.run.out);
    for (const auto& [root, path] : w2p_input_files(b)) {
      CHECK(path != "combined.lua");
      CHECK(path != "split.lua");
    }
  }
}

// ---- W2-11, W2-12, W2-13 ---------------------------------------------------------------------------------------------

// W2-11. Mutants killed: skipping spec validation, mapping every error to a generic schema error, or the schema mode
// returning the Listed-job diagnostic first. The expected bytes are load's own stderr for the same declaration bytes.
TEST_CASE("RA01-B2 ingest and critique with an invalid spec fail with the typed error load gives", "[authoring][ra01][prompts]") {
  const auto goal = std::string_view("goal = p.md \"Provide an atomic in-memory quota and a small command-line client.\"");
  struct row {
    std::string name, from, to;
  };
  const std::vector<row> rows{
      {"empty-goal", std::string(goal), "goal = p.md \"\""},
      {"unknown-check", "checks = L { \"contract\" } },\n    { id = \"command\"",
       "checks = L { \"nonesuch\" } },\n    { id = \"command\""},
  };
  for (const auto& r : rows) {
    CAPTURE(r.name);
    w2p_tree t;
    t.quota();
    w2p_tree::edit(t.ws / "spec.lua", r.from, r.to);
    const auto loaded = t.load("led-" + r.name);
    REQUIRE(loaded.status == 3);
    REQUIRE(loaded.out.empty());
    const auto typed = w2p_parse(loaded.err);
    if (r.name == "empty-goal") {
      CHECK(typed.at("error").at("code").s == "schema");
      CHECK(typed.at("error").at("field").s == "spec.goal");
      CHECK(typed.at("error").at("file").s == "spec.lua");
    }
    for (const std::string verb : {"ingest", "critique"}) {
      CAPTURE(verb);
      const auto got = t.run(t.prompt_args(verb));
      CHECK(got.status == loaded.status);
      CHECK(got.out.empty());
      CHECK(got.err == loaded.err);
    }
  }
}

// W2-12. Mutants killed: schema mode workspace not honoured (spec-prompt fails on a valid workspace), prompts.lua passing
// mode nil, or the mode dropping every reference error. An invalid spec and a missing spec are not seen by spec-prompt; an
// invalid workspace or registry gives load's own typed error.
TEST_CASE("RA01-B2 spec-prompt validates the workspace and registry only", "[authoring][ra01][prompts]") {
  const auto goal = std::string_view("goal = p.md \"Provide an atomic in-memory quota and a small command-line client.\"");
  {
    w2p_tree t;
    t.quota();
    w2p_tree::edit(t.ws / "spec.lua", goal, "goal = p.md \"\"");
    const auto got = t.run(t.prompt_args("spec-prompt"));
    CHECK(got.status == 0);
    CHECK(got.err.empty());
    CHECK(w2p_parse(got.out).at("prompt").at("kind").s == "spec");
  }
  {
    w2p_tree t;
    t.quota();
    fs::remove(t.ws / "spec.lua");
    const auto got = t.run(t.prompt_args("spec-prompt"));
    CHECK(got.status == 0);
    CHECK(got.err.empty());
  }
  struct row {
    std::string name;
    bool        registry;
    std::string from, to;
  };
  const std::vector<row> rows{
      {"short-toolchains", false, "toolchains = L { \"python\" }", "toolchains = L { }"},
      {"registry-header", true, "\"revision\": \"v1\"", "\"revision\": \"v2\""},
  };
  for (const auto& r : rows) {
    CAPTURE(r.name);
    w2p_tree t;
    t.quota();
    w2p_tree::edit(r.registry ? t.registry : t.workspace(), r.from, r.to);
    const auto loaded = t.load("led-" + r.name);
    REQUIRE(loaded.status == 3);
    const auto got = t.run(t.prompt_args("spec-prompt"));
    CHECK(got.status == 3);
    CHECK(got.out.empty());
    CHECK(got.err == loaded.err);
  }
}

// W2-13. Mutants killed: prompts.lua passing mode nil (full validation) or validating any job content found in the bundle.
// The control shows that load does report the invalid listed job first.
TEST_CASE("RA01-B2 authoring reports no job file errors", "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  w2p_tree::put(t.ws / "combined.lua", "local p = planar\nreturn p.job { version = 1, id = \"combined\" }\n");
  const auto loaded = t.load("led-jobs");
  REQUIRE(loaded.status == 3);
  CHECK(w2p_parse(loaded.err).at("error").at("file").s == "combined.lua");
  for (const std::string verb : {"ingest", "critique", "spec-prompt"}) {
    CAPTURE(verb);
    const auto got = t.run(t.prompt_args(verb));
    CHECK(got.status == 0);
    CHECK(got.err.empty());
  }
}

// ---- W2-14, W2-15, W2-16, W2-17 --------------------------------------------------------------------------------------

// W2-14. Mutant killed: prompts.lua computing the digest with engine.file_digest (file_bytes_v1) instead of
// engine.fragment_digest. Envelope keys, markdown ending and bytes are checked for every prompt-producing verb.
TEST_CASE("RA01-B2 prompt envelope fields digest bytes and kind per verb", "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  w2p_tree::put(t.diagnostic(), w2p_diag_base() + "\n");
  struct row {
    std::string              verb, kind, title;
    std::vector<std::string> args;
  };
  const std::vector<row> rows{
      {"spec-prompt", "spec", "# Planar spec prompt", t.prompt_args("spec-prompt")},
      {"critique", "critique", "# Planar critique prompt", t.prompt_args("critique")},
      {"ingest", "job", "# Planar job prompt", t.prompt_args("ingest")},
      {"explain-rejection", "rejection", "# Planar rejection prompt", t.reject_args()},
      {"orchestrator", "orchestrator", "", t.orchestrator_args("combined")},
  };
  for (const auto& r : rows) {
    CAPTURE(r.verb);
    const auto d = w2p_make_doc(t.run(r.args));
    REQUIRE(d.run.status == 0);
    CHECK(d.run.err.empty());
    CHECK(d.run.out.ends_with("}\n"));
    CHECK(std::ranges::count(d.run.out, '\n') == 1);
    CHECK(d.run.out.starts_with("{\"kind\":\"prompt\",\"prompt\":{\"bytes\":"));
    CHECK(d.env.keys() == std::vector<std::string>{"kind", "prompt", "version"});
    CHECK(d.env.at("version").n == 1);
    CHECK(d.env.at("kind").s == "prompt");
    const auto& prompt = d.env.at("prompt");
    CHECK(prompt.keys() == std::vector<std::string>{"bytes", "digest", "inputs", "kind", "markdown"});
    CHECK(prompt.at("kind").s == r.kind);
    REQUIRE(prompt.at("markdown").k == w2p_json::kind::string);
    CHECK(d.md.ends_with("\n"));
    CHECK_FALSE(d.md.ends_with("\n\n"));
    CHECK_FALSE(w2p_has(d.md, "\r"));
    CHECK(prompt.at("bytes").integer);
    CHECK(prompt.at("bytes").n == static_cast<long long>(d.md.size()));
    if (!r.title.empty())
      CHECK(d.md.starts_with(r.title + "\n"));
    const auto fragment = planar::engine::fragment_digest(d.md);
    const auto file     = planar::engine::file_digest(d.md);
    REQUIRE(fragment.has_value());
    REQUIRE(file.has_value());
    CHECK(prompt.at("digest").s == *fragment);
    CHECK(prompt.at("digest").s != *file);
    CHECK(prompt.at("digest").s.starts_with("xxh3_128:"));
  }
}

// W2-15. Mutants killed: dropping the sort of inputs.files, or adding bytes_hex to an entry. The spec references text files
// whose capture order differs from the (root, path) unsigned byte order (Zeta sorts before alpha).
TEST_CASE("RA01-B2 prompt inputs list sorted file entries and registry header", "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  w2p_tree::put(t.ws / "text/Zeta.md", "Zeta.\n");
  w2p_tree::put(t.ws / "text/alpha.md", "Alpha.\n");
  w2p_tree::edit(t.ws / "spec.lua", "    { id = \"safe\", scope = \"global\", text = p.file \"text/shared.md\" }",
                 "    { id = \"safe\", scope = \"global\", text = p.file \"text/shared.md\" },\n"
                 "    { id = \"zed\", scope = \"global\", text = p.file \"text/Zeta.md\" },\n"
                 "    { id = \"alp\", scope = \"global\", text = p.file \"text/alpha.md\" }");
  for (const std::string verb : {"critique", "ingest"}) {
    CAPTURE(verb);
    const auto d = t.prompt(verb);
    REQUIRE(d.run.status == 0);
    const auto& inputs = d.env.at("prompt").at("inputs");
    CHECK(inputs.keys() == std::vector<std::string>{"files", "registry"});
    CHECK(inputs.at("registry").keys() == std::vector<std::string>{"id", "revision"});
    CHECK(inputs.at("registry").at("id").s == "quota_tools");
    CHECK(inputs.at("registry").at("revision").s == "v1");
    const auto files = w2p_input_files(d);
    REQUIRE(files.size() == 7);
    CHECK(std::ranges::is_sorted(files));
    const std::vector<std::pair<std::string, std::string>> workspace_entries{
        {"workspace", "spec.lua"},          {"workspace", "text/Zeta.md"},   {"workspace", "text/alpha.md"},
        {"workspace", "text/quota-api.md"}, {"workspace", "text/shared.md"}, {"workspace", "workspace.lua"}};
    std::vector<std::pair<std::string, std::string>> seen;
    for (const auto& item : inputs.at("files").items) {
      CHECK(item.keys() == std::vector<std::string>{"digest", "kind", "mode", "path", "root", "size"});
      CHECK(item.at("kind").s == "file");
      CHECK(item.at("mode").k == w2p_json::kind::string);
      if (item.at("root").s == "workspace") {
        seen.emplace_back(item.at("root").s, item.at("path").s);
        const auto bytes = w2p_slurp(t.ws / item.at("path").s);
        CHECK(item.at("size").n == static_cast<long long>(bytes.size()));
        const auto digest = planar::engine::file_digest(bytes);
        REQUIRE(digest.has_value());
        CHECK(item.at("digest").s == *digest);
      } else {
        CHECK(item.at("root").s == "registry");
        const auto bytes = w2p_slurp(t.registry);
        CHECK(item.at("size").n == static_cast<long long>(bytes.size()));
        const auto digest = planar::engine::file_digest(bytes);
        REQUIRE(digest.has_value());
        CHECK(item.at("digest").s == *digest);
      }
    }
    CHECK(seen == workspace_entries);
    CHECK_FALSE(w2p_has(d.run.out, "bytes_hex"));
  }
}

namespace {
/// Fixture of >= 32 checks and 16 work units for the A35 test; `reversed` lists every declaration in the opposite order.
w2p_gen w2p_big(bool reversed) {
  w2p_gen                  g;
  std::vector<w2p_check>   checks;
  std::vector<w2p_unit>    units;
  std::vector<std::string> unit_ids;
  for (int i = 1; i <= 32; ++i)
    checks.push_back({std::format("ck_{:02}", i), R"("own_output","expected_contracts")", "nonempty", ""});
  for (int i = 1; i <= 16; ++i) {
    w2p_unit u{std::format("u_{:02}", i), {std::format("ck_{:02}", 2 * i - 1), std::format("ck_{:02}", 2 * i)}, {}};
    if (reversed)
      std::ranges::reverse(u.checks);
    units.push_back(u);
  }
  if (reversed) {
    std::ranges::reverse(checks);
    std::ranges::reverse(units);
  }
  for (const auto& u : units)
    unit_ids.push_back(u.id);
  g.registry = checks;
  g.allow    = w2p_check_ids(checks);
  g.spec     = w2p_spec_text(units);
  g.job_text = w2p_job_text("j1", unit_ids);
  return g;
}
} // namespace

// W2-16. Mutant killed: prompts.lua embedding a nondeterministic value (clock, address, environment). The full envelope of
// each of the five kinds is compared across two fresh stack processes over the SAME files. Over the reordered fixture (a
// second tree whose declarations are listed in the opposite order, so inputs.files size and digest differ) only
// prompt.markdown, prompt.bytes and prompt.digest of the four capture-based prompts are compared; the rejection prompt reads
// the same bytes and compares in full. Lua hash seed 0 makes pairs-order mutants stable: W2-20 and W2-23 kill those.
TEST_CASE("RA01-B2 A35 five prompt kinds are byte-equal across two fresh processes and reordered declarations",
          "[authoring][ra01][prompts]") {
  w2p_tree a, b;
  w2p_install(a, w2p_big(false));
  w2p_install(b, w2p_big(true));
  w2p_tree::put(a.diagnostic(), w2p_diag_base() + "\n");
  w2p_tree::put(b.diagnostic(), w2p_diag_base() + "\n");
  struct row {
    std::string                                              verb, kind;
    std::function<std::vector<std::string>(const w2p_tree&)> args;
    bool                                                     capture;
  };
  const std::vector<row> rows{
      {"spec-prompt", "spec", [](const w2p_tree& t) { return t.prompt_args("spec-prompt"); }, true},
      {"critique", "critique", [](const w2p_tree& t) { return t.prompt_args("critique"); }, true},
      {"ingest", "job", [](const w2p_tree& t) { return t.prompt_args("ingest"); }, true},
      {"explain-rejection", "rejection", [](const w2p_tree& t) { return t.reject_args(); }, false},
      {"orchestrator", "orchestrator", [](const w2p_tree& t) { return t.orchestrator_args("j1"); }, true},
  };
  for (const auto& r : rows) {
    CAPTURE(r.verb);
    const auto first  = w2p_spawn(a.root, a.work, r.args(a), r.verb + "-1");
    const auto second = w2p_spawn(a.root, a.work, r.args(a), r.verb + "-2");
    REQUIRE(first.status == 0);
    REQUIRE(second.status == 0);
    CHECK(first.err.empty());
    CHECK(first.out == second.out);
    const auto da = w2p_make_doc(first);
    CHECK(da.env.at("prompt").at("kind").s == r.kind);
    const auto other = w2p_spawn(b.root, b.work, r.args(b), r.verb + "-b");
    REQUIRE(other.status == 0);
    const auto db = w2p_make_doc(other);
    if (!r.capture) {
      CHECK(other.out == first.out);
    } else {
      CHECK(da.md == db.md);
      CHECK(da.env.at("prompt").at("bytes").n == db.env.at("prompt").at("bytes").n);
      CHECK(da.env.at("prompt").at("digest").s == db.env.at("prompt").at("digest").s);
      CHECK(w2p_input_files(da).size() == w2p_input_files(db).size());
    }
  }
  const auto spec = w2p_make_doc(w2p_spawn(a.root, a.work, a.prompt_args("spec-prompt"), "scale"));
  REQUIRE(spec.run.status == 0);
  const auto checks = w2p_section(spec.md, "Checks");
  REQUIRE(checks.has_value());
  CHECK(std::ranges::count_if(w2p_lines(*checks), [](const std::string& line) { return line.starts_with("- ck_"); }) == 32);
}

// W2-17. Mutants killed: swapping checks and shapes in the job template, or omitting critique_step from the spec template.
TEST_CASE("RA01-B2 prompts carry their sections in contract order", "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  w2p_tree::put(t.diagnostic(), w2p_diag_base() + "\n");
  struct row {
    std::string              verb, title;
    std::vector<std::string> args, headings;
  };
  const std::vector<row> rows{
      {"spec-prompt",
       "# Planar spec prompt",
       t.prompt_args("spec-prompt"),
       {"Authority", "Task", "Workspace", "Spec schema", "Checks", "Shapes", "Critique step"}},
      {"critique",
       "# Planar critique prompt",
       t.prompt_args("critique"),
       {"Authority", "Task", "Workspace", "Spec schema", "Checks", "Findings"}},
      {"ingest",
       "# Planar job prompt",
       t.prompt_args("ingest"),
       {"Authority", "Task", "Workspace", "Spec summary", "Job schema", "Checks", "Shapes", "Constructors"}},
      {"explain-rejection",
       "# Planar rejection prompt",
       t.reject_args(),
       {"Authority", "Task", "Error", "Schema excerpt", "Raw error"}},
  };
  for (const auto& r : rows) {
    CAPTURE(r.verb);
    const auto d = w2p_make_doc(t.run(r.args));
    REQUIRE(d.run.status == 0);
    CHECK(d.md.starts_with(r.title + "\n"));
    CHECK(w2p_headings(d.md) == r.headings);
    std::size_t titles = 0;
    for (const auto& line : w2p_lines(d.md)) {
      if (line.starts_with("# "))
        ++titles;
      if (line == "## Raw error")
        break;
    }
    CHECK(titles == 1);
    // Sections are separated by exactly one blank line.
    std::size_t pos = 0;
    while ((pos = d.md.find("\n## ", pos)) != std::string::npos) {
      const auto heading = pos + 1;
      REQUIRE(heading >= 3);
      CHECK(d.md[heading - 1] == '\n');
      CHECK(d.md[heading - 2] == '\n');
      CHECK(d.md[heading - 3] != '\n');
      pos = heading + 3;
      if (d.md.compare(heading, 12, "## Raw error") == 0)
        break;
    }
  }
}

// ---- W2-18, W2-19 ---------------------------------------------------------------------------------------------------

// W2-18. Mutants killed (by the prober): schema.lua goal req->opt (the prompt must follow it), and prompts.lua hard-copying
// the schema text (the first mutant then survives, which is the detection). Golden lines for the spec, job and workspace
// tables; the workspace table is reached through the rejection excerpt for the path "workspace".
TEST_CASE("RA01-B2 schema text lists spec job and workspace fields generated from the schema tables",
          "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  const auto spec = t.prompt("spec-prompt");
  const auto crit = t.prompt("critique");
  const auto job  = t.prompt("ingest");
  REQUIRE(spec.run.status == 0);
  REQUIRE(crit.run.status == 0);
  REQUIRE(job.run.status == 0);
  const auto spec_text = w2p_section(spec.md, "Spec schema");
  const auto crit_text = w2p_section(crit.md, "Spec schema");
  const auto job_text  = w2p_section(job.md, "Job schema");
  REQUIRE(spec_text.has_value());
  REQUIRE(crit_text.has_value());
  REQUIRE(job_text.has_value());
  CHECK(*spec_text == *crit_text);
  const auto in = [](const std::string& text, std::string_view line) {
    const auto lines = w2p_lines(text);
    return std::ranges::find(lines, std::string(line)) != lines.end();
  };
  for (const char* line :
       {"- goal (required): text", "- id (required): id", "- version (required): version", "- rationale (optional): note",
        "- milestones (required): list of record", "- work (required): list of record", "  - checks (required): set of id",
        "  - paths (required): set of boundary", "  - summary (required): text", "  - acceptance (required): list of record",
        "    - text (required): text", "  - scope (required): scope",
        "  - blocks (optional, default none): enum none|start|complete", "  - phase (required): enum start|complete",
        "  - quality (required): enum available|done|accepted", "  - source (required): enum contract|facet"}) {
    CAPTURE(line);
    CHECK(in(*spec_text, line));
  }
  for (const auto& prefix : {"- constraints (optional", "- contracts (optional", "- dependencies (optional", "- facets (optional",
                             "- frozen (optional", "- prescriptions (optional", "- questions (optional"}) {
    CAPTURE(prefix);
    CHECK(std::ranges::any_of(w2p_lines(*spec_text), [&](const std::string& line) { return line.starts_with(prefix); }));
  }
  for (const char* line :
       {"- shape (required): enum pipeline|fanout_review", "- max_rounds (optional, default 2): number 0..2147483647",
        "- budget_bytes (optional, default 32768): number 1..1048576", "- roles (required): list of record",
        "  - kind (required): enum worker|reviewer", "  - work (optional): set of id", "  - targets (optional): set of id",
        "  - budget_bytes (optional): number 1..1048576", "  - preferences (optional): record",
        "    - model (optional): string 256", "    - session (optional): string 256", "- id (required): id",
        "- version (required): version", "- notes (optional): note"}) {
    CAPTURE(line);
    CHECK(in(*job_text, line));
  }
  CHECK(
      std::ranges::any_of(w2p_lines(*job_text), [](const std::string& line) { return line.starts_with("- stages (optional"); }));
  // The workspace table through the rejection excerpt for the path "workspace" (a path of only the schema name).
  const auto work = t.reject(w2p_diag_error("schema", "workspace", "Expected workspace"));
  REQUIRE(work.run.status == 0);
  const auto work_text = w2p_section(work.md, "Schema excerpt");
  REQUIRE(work_text.has_value());
  for (const char* line :
       {"- version (required): version", "- id (required): id", "- spec (required): path", "- toolchains (required): set of id",
        "- checks (required): set of id", "- root (required): record", "  - base (required): id", "  - id (required): id",
        "- jobs (required): list of record", "  - file (required): path", "- registry (required): record",
        "  - revision (required): id", "- notes (optional): note"}) {
    CAPTURE(line);
    CHECK(in(*work_text, line));
  }
  CHECK(std::ranges::any_of(w2p_lines(*work_text),
                            [](const std::string& line) { return line.starts_with("  - excluded (optional"); }));
}

// W2-19. Mutants killed: sorting fields descending, indenting four spaces per level, or dropping the default clause. Every
// sibling group is sorted by unsigned byte name, nesting is two spaces per level, and each line states required/optional.
TEST_CASE("RA01-B2 schema text format sorts fields and nests records", "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  const auto spec = t.prompt("spec-prompt");
  const auto job  = t.prompt("ingest");
  REQUIRE(spec.run.status == 0);
  REQUIRE(job.run.status == 0);
  for (const auto& [name, text] :
       {std::pair<std::string, std::optional<std::string>>{"spec", w2p_section(spec.md, "Spec schema")},
        {"job", w2p_section(job.md, "Job schema")}}) {
    CAPTURE(name);
    REQUIRE(text.has_value());
    std::vector<std::optional<std::string>> last(8);
    std::size_t                             fields       = 0;
    int                                     depth_before = -1;
    for (const auto& line : w2p_lines(*text)) {
      if (line.empty())
        continue;
      CAPTURE(line);
      const auto indent = w2p_indent(line);
      REQUIRE(indent % 2 == 0);
      const int depth = static_cast<int>(indent / 2);
      REQUIRE(depth < 8);
      CHECK(depth <= depth_before + 1);
      const auto body = line.substr(indent);
      REQUIRE(body.starts_with("- "));
      const auto open = body.find(" (");
      REQUIRE(open != std::string::npos);
      const auto name = body.substr(2, open - 2);
      CHECK((body.substr(open).starts_with(" (required)") || body.substr(open).starts_with(" (optional")));
      CHECK(w2p_has(body, "): "));
      for (int d = depth + 1; d < 8; ++d)
        last[static_cast<std::size_t>(d)].reset();
      auto& previous = last[static_cast<std::size_t>(depth)];
      if (previous)
        CHECK(*previous < name);
      previous     = name;
      depth_before = depth;
      ++fields;
    }
    CHECK(fields > 8);
  }
  const auto job_text = *w2p_section(job.md, "Job schema");
  const auto lines    = w2p_lines(job_text);
  const auto index    = [&](std::string_view line) {
    const auto it = std::ranges::find(lines, std::string(line));
    return it == lines.end() ? std::ptrdiff_t(-1) : it - lines.begin();
  };
  CHECK(index("  - preferences (optional): record") >= 0);
  CHECK(index("    - model (optional): string 256") == index("  - preferences (optional): record") + 1);
  CHECK(index("    - session (optional): string 256") == index("    - model (optional): string 256") + 1);
  CHECK(index("- max_rounds (optional, default 2): number 0..2147483647") >= 0);
}

// ---- W2-20 to W2-23 --------------------------------------------------------------------------------------------------

// W2-20. Mutants killed: using the union of the allow-list and the registry (or the registry alone), and the pairs-order
// mutant for checks (hash seed 0 makes it deterministic). Thirteen registry checks, eleven allow-listed, ids chosen so that
// the unsigned byte order differs from declaration order.
TEST_CASE("RA01-B2 checks section is the sorted allow-list intersected with the registry", "[authoring][ra01][prompts]") {
  const std::vector<std::string> registry_ids{"zeta", "alpha", "a_b", "ab",  "m1",   "m_1",           "k",
                                              "kk",   "k_k",   "q9",  "q_9", "beta", "extra_reg_only"};
  std::vector<w2p_check>         registry;
  for (const auto& id : registry_ids)
    registry.push_back({id, R"("own_output","expected_contracts")", "nonempty", ""});
  w2p_gen g;
  g.registry = registry;
  g.allow.assign(registry_ids.begin(), registry_ids.begin() + 11);
  g.spec = w2p_spec_text({{"u1", {"zeta", "alpha"}, {}}});
  w2p_tree t;
  w2p_install(t, g);
  auto expected = g.allow;
  std::ranges::sort(expected);
  for (const std::string verb : {"spec-prompt", "critique", "ingest"}) {
    CAPTURE(verb);
    const auto d = t.prompt(verb);
    REQUIRE(d.run.status == 0);
    const auto section = w2p_section(d.md, "Checks");
    REQUIRE(section.has_value());
    std::vector<std::string> ids;
    for (const auto& line : w2p_lines(*section))
      if (line.starts_with("- ")) {
        ids.push_back(line.substr(2, line.find(':') - 2));
        const auto id = ids.back();
        CHECK(line == "- " + id + ": toolchain python; coverage \"Covers " + id +
                          ".\"; inputs expected_contracts, own_output; selection nonempty");
      }
    CHECK(ids == expected);
    CHECK_FALSE(w2p_has(*section, "- beta"));
    CHECK_FALSE(w2p_has(*section, "extra_reg_only"));
  }
}

// W2-21. Mutants killed: swapping the order of the two shapes, or dropping fanout_review. The first block names pipeline.
TEST_CASE("RA01-B2 shape catalog lists pipeline then fanout_review", "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  const auto spec = t.prompt("spec-prompt");
  const auto job  = t.prompt("ingest");
  REQUIRE(spec.run.status == 0);
  REQUIRE(job.run.status == 0);
  const auto a = w2p_section(spec.md, "Shapes");
  const auto b = w2p_section(job.md, "Shapes");
  REQUIRE(a.has_value());
  REQUIRE(b.has_value());
  CHECK(*a == *b);
  const auto lines = w2p_lines(*a);
  const auto first = [&](std::string_view word) {
    for (std::size_t i = 0; i < lines.size(); ++i)
      if (w2p_has(lines[i], word))
        return static_cast<std::ptrdiff_t>(i);
    return std::ptrdiff_t(-1);
  };
  REQUIRE(first("pipeline") >= 0);
  REQUIRE(first("fanout_review") >= 0);
  CHECK(first("pipeline") < first("fanout_review"));
  const auto first_line = std::ranges::find_if(lines, [](const std::string& l) { return !l.empty(); });
  REQUIRE(first_line != lines.end());
  CHECK(w2p_has(*first_line, "pipeline"));
  CHECK_FALSE(w2p_has(*first_line, "fanout_review"));
  CHECK(w2p_has(*a, "stages"));
  CHECK(w2p_has(*a, "reviewer"));
}

// W2-22. Mutant killed: prompts.lua substituting the real workspace path into the critique_step line. No prompt embeds a
// host path (the tree root, workspace and registry paths, or the unique directory name).
TEST_CASE("RA01-B2 spec prompt ends with the critique step line and embeds no host path", "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  w2p_tree::put(t.diagnostic(), w2p_diag_base() + "\n");
  const auto spec = t.prompt("spec-prompt");
  REQUIRE(spec.run.status == 0);
  const std::string step =
      "Run planar critique --workspace <workspace> --registry <registry> and resolve every finding before ingesting the spec.";
  CHECK(spec.md.ends_with(step + "\n"));
  const auto section = w2p_section(spec.md, "Critique step");
  REQUIRE(section.has_value());
  const auto lines = w2p_lines(*section);
  REQUIRE_FALSE(lines.empty());
  CHECK(lines.back() == step);
  CHECK(w2p_headings(spec.md).back() == "Critique step");
  const auto unique = t.root.filename().string();
  for (const auto& d : {spec, t.prompt("critique"), t.prompt("ingest"), t.reject(w2p_diag_base() + "\n")}) {
    REQUIRE(d.run.status == 0);
    CHECK_FALSE(w2p_has(d.md, t.root.string()));
    CHECK_FALSE(w2p_has(d.md, t.workspace().string()));
    CHECK_FALSE(w2p_has(d.md, t.registry.string()));
    CHECK_FALSE(w2p_has(d.md, unique));
    CHECK_FALSE(w2p_has(d.run.out, unique));
  }
}

// W2-23. Mutants killed: iterating work units, prescriptions, facets or dependencies unsorted (pairs order), or embedding
// the whole spec text. Seven units, three prescriptions, two contracts, two facets and three dependencies are declared in
// an order that differs from the unsigned byte order of their ids.
TEST_CASE("RA01-B2 spec summary lists work checks and paths sorted by id", "[authoring][ra01][prompts]") {
  const std::vector<std::string> work_ids{"zz", "u_10", "u_9", "a1", "u_b", "ua", "m"};
  std::vector<w2p_unit>          units;
  for (const auto& id : work_ids)
    units.push_back({id, {"ck_b", "ck_a"}, {"d_" + id + "/zeta/", "d_" + id + "/alpha/"}});
  const std::string extra =
      "  prescriptions = L {\n    { id = \"pr_b\", scope = \"global\", text = p.md \"Rule B.\" },\n"
      "    { id = \"pr_a\", scope = \"global\", text = p.md \"Rule A.\" },\n"
      "    { id = \"pr_c\", scope = \"global\", text = p.md \"Rule C.\" }\n  },\n"
      "  contracts = L {\n"
      "    { id = \"ct_b\", producer = \"zz\", consumers = L { \"u_10\" }, expected = p.md \"B.\", conformance = \"ck_conf\", "
      "review_note = p.md \"Reviewed.\" },\n"
      "    { id = \"ct_a\", producer = \"u_9\", consumers = L { \"u_b\" }, expected = p.md \"A.\", conformance = \"ck_conf\", "
      "review_note = p.md \"Reviewed.\" }\n  },\n"
      "  facets = L {\n    { id = \"f_b\", producer = \"zz\", paths = L { \"d_zz/alpha/x.py\" } },\n"
      "    { id = \"f_a\", producer = \"u_9\", paths = L { \"d_u_9/alpha/y.py\" } }\n  },\n"
      "  dependencies = L {\n"
      "    { id = \"dep_z\", consumer = \"u_10\", source = \"contract\", ref = \"ct_b\", phase = \"start\", quality = "
      "\"available\" },\n"
      "    { id = \"dep_m\", consumer = \"ua\", source = \"facet\", ref = \"f_a\", phase = \"complete\", quality = \"done\" },\n"
      "    { id = \"dep_a\", consumer = \"u_b\", source = \"contract\", ref = \"ct_a\", phase = \"start\", quality = "
      "\"available\" }\n  },\n";
  w2p_gen g;
  g.registry = {{"ck_a", R"("own_output")", "nonempty", ""},
                {"ck_b", R"("own_output")", "nonempty", ""},
                {"ck_conf", R"("expected_contracts","own_output")", "nonempty", ""}};
  g.allow    = w2p_check_ids(g.registry);
  g.spec     = w2p_spec_text(units, extra);
  w2p_tree t;
  w2p_install(t, g);
  const auto d = t.prompt("ingest");
  REQUIRE(d.run.status == 0);
  const auto section = w2p_section(d.md, "Spec summary");
  REQUIRE(section.has_value());
  CHECK(w2p_ids_of(*section, "work") == std::vector<std::string>{"a1", "m", "u_10", "u_9", "u_b", "ua", "zz"});
  CHECK(w2p_ids_of(*section, "prescription") == std::vector<std::string>{"pr_a", "pr_b", "pr_c"});
  CHECK(w2p_ids_of(*section, "contract") == std::vector<std::string>{"ct_a", "ct_b"});
  CHECK(w2p_ids_of(*section, "facet") == std::vector<std::string>{"f_a", "f_b"});
  CHECK(w2p_ids_of(*section, "dependency") == std::vector<std::string>{"dep_a", "dep_m", "dep_z"});
  const auto lines = w2p_lines(*section);
  CHECK(std::ranges::find(lines, std::string("- spec: gen")) != lines.end());
  CHECK(std::ranges::find(lines, std::string("- work a1: checks ck_a, ck_b; paths d_a1/alpha/, d_a1/zeta/")) != lines.end());
  CHECK(std::ranges::find(lines, std::string("- work u_10: checks ck_a, ck_b; paths d_u_10/alpha/, d_u_10/zeta/")) !=
        lines.end());
  // No file bytes beyond the fields: the unit summary and contract texts are not embedded.
  CHECK_FALSE(w2p_has(d.md, "Reviewed."));
  CHECK_FALSE(w2p_has(d.md, "local p = planar"));
}

// ---- W2-24 ---------------------------------------------------------------------------------------------------------

// W2-24. Mutant killed: prompts.lua emitting captured text raw instead of through presentation_data. The goal carries ESC,
// DEL, a C1 character and a run of four backticks; a registry coverage string carries ESC (escaped by the JSON encoding).
TEST_CASE("RA01-B2 captured text is escaped as data in prompts", "[authoring][ra01][prompts]") {
  w2p_gen g;
  g.registry = {{"ck_a", R"("own_output")", "nonempty", "cov \\u001b \\u0086 end"}};
  g.allow    = {"ck_a"};
  g.spec     = w2p_spec_text({{"u1", {"ck_a"}, {}}}, "", R"(p.md "ctl\27[31m del\127 c1\194\133 run\96\96\96\96 two\96\96 end")");
  w2p_tree t;
  w2p_install(t, g);
  const auto ingest = t.prompt("ingest");
  REQUIRE(ingest.run.status == 0);
  CHECK_FALSE(w2p_raw_controls(ingest.md));
  CHECK(w2p_has(ingest.md, "\\u001b"));
  CHECK(w2p_has(ingest.md, "\\u007f"));
  CHECK(w2p_has(ingest.md, "\\u0085"));
  // U+0086 occurs only in a check coverage string, which the Lua escape (not the native text path) renders.
  CHECK(w2p_has(ingest.md, "\\u0086"));
  CHECK(w2p_has(ingest.md, "controls escaped as \\u00xx"));
  bool long_fence = false;
  for (const auto& line : w2p_lines(ingest.md))
    if (line.size() >= 5 && line.find_first_not_of('`') == std::string::npos)
      long_fence = true;
  CHECK(long_fence);
  for (const std::string verb : {"spec-prompt", "critique"}) {
    CAPTURE(verb);
    const auto d = t.prompt(verb);
    REQUIRE(d.run.status == 0);
    CHECK_FALSE(w2p_raw_controls(d.md));
  }
}

// ---- W2-25 to W2-29 --------------------------------------------------------------------------------------------------

// W2-25. Mutant killed: counting a check regardless of its registry selection (dropping selection == nonempty). pr_a is
// scoped to the unit whose only check is not_applicable; pr_b is covered through a conformance check, pr_c is global and
// pr_d scopes a unit with a nonempty own check.
TEST_CASE("RA01-B2 critique rule prescription_unchecked fires only without a nonempty check", "[authoring][ra01][prompts]") {
  const std::string extra = "  prescriptions = L {\n    { id = \"pr_a\", scope = L { \"u1\" }, text = p.md \"A.\" },\n"
                            "    { id = \"pr_b\", scope = L { \"u3\" }, text = p.md \"B.\" },\n"
                            "    { id = \"pr_c\", scope = \"global\", text = p.md \"C.\" },\n"
                            "    { id = \"pr_d\", scope = L { \"u1\", \"u2\" }, text = p.md \"D.\" }\n  },\n"
                            "  contracts = L {\n    { id = \"ct\", producer = \"u2\", consumers = L { \"u3\" }, expected = p.md "
                            "\"E.\", conformance = \"ck_ne\", review_note = p.md \"Reviewed.\" }\n  },\n"
                            "  dependencies = L {\n    { id = \"dep1\", consumer = \"u3\", source = \"contract\", ref = \"ct\", "
                            "phase = \"start\", quality = \"available\" }\n  },\n";
  w2p_tree          t;
  w2p_install(t, w2p_critique_gen({{"u1", {"ck_na"}, {}}, {"u2", {"ck_ne"}, {}}, {"u3", {"ck_na"}, {}}}, extra));
  const auto d = t.prompt("critique");
  REQUIRE(d.run.status == 0);
  const auto findings = w2p_findings(d);
  REQUIRE(findings.size() == 1);
  CHECK(findings[0].starts_with("- prescription_unchecked pr_a: "));
  CHECK(findings[0].size() > std::string("- prescription_unchecked pr_a: ").size());
}

// W2-26. Mutant killed: testing blocks == "start" only (dropping complete). Unblocking or answered questions yield nothing.
TEST_CASE("RA01-B2 critique rule blocking_question_unanswered covers start and complete", "[authoring][ra01][prompts]") {
  const std::string extra =
      "  questions = L {\n"
      "    { id = \"q_start\", scope = \"global\", prompt = p.md \"Start?\", blocks = \"start\" },\n"
      "    { id = \"q_complete\", scope = \"global\", prompt = p.md \"Complete?\", blocks = \"complete\" },\n"
      "    { id = \"q_none\", scope = \"global\", prompt = p.md \"None?\", blocks = \"none\" },\n"
      "    { id = \"q_done\", scope = \"global\", prompt = p.md \"Done?\", blocks = \"start\", answer = p.md \"Yes.\" },\n"
      "    { id = \"q_done2\", scope = \"global\", prompt = p.md \"Done2?\", blocks = \"complete\", answer = p.md \"Yes.\" }\n  "
      "},\n";
  w2p_tree t;
  w2p_install(t, w2p_critique_gen({{"u1", {"ck_ne"}, {}}}, extra));
  const auto d = t.prompt("critique");
  REQUIRE(d.run.status == 0);
  const auto findings = w2p_findings(d);
  REQUIRE(findings.size() == 2);
  CHECK(findings[0].starts_with("- blocking_question_unanswered q_complete: "));
  CHECK(findings[1].starts_with("- blocking_question_unanswered q_start: "));
}

// W2-27. Mutant killed: accepting any check input (dropping the output_vector requirement). Tree A's consumer has only a
// check without output_vector (fires, once: the start-phase facet dependency and the contract-free case stay silent);
// tree B's consumer has an output_vector check (silent).
TEST_CASE("RA01-B2 critique rule facet_dependency_without_output_vector needs an output_vector check",
          "[authoring][ra01][prompts]") {
  const std::string extra =
      "  facets = L {\n    { id = \"f\", producer = \"prod\", paths = L { \"d_prod/x.py\" } }\n  },\n"
      "  dependencies = L {\n"
      "    { id = \"dep_f\", consumer = \"cons\", source = \"facet\", ref = \"f\", phase = \"complete\", quality = \"done\" },\n"
      "    { id = \"dep_s\", consumer = \"cons\", source = \"facet\", ref = \"f\", phase = \"start\", quality = \"available\" "
      "}\n  },\n";
  {
    w2p_tree t;
    w2p_install(t, w2p_critique_gen({{"prod", {"ck_ne"}, {}}, {"cons", {"ck_ne"}, {}}}, extra));
    const auto d = t.prompt("critique");
    REQUIRE(d.run.status == 0);
    const auto findings = w2p_findings(d);
    REQUIRE(findings.size() == 1);
    CHECK(findings[0].starts_with("- facet_dependency_without_output_vector dep_f: "));
  }
  {
    w2p_tree t;
    w2p_install(t, w2p_critique_gen({{"prod", {"ck_ne"}, {}}, {"cons", {"ck_ov"}, {}}}, extra));
    const auto d = t.prompt("critique");
    REQUIRE(d.run.status == 0);
    CHECK(w2p_findings(d) == std::vector<std::string>{"No mechanical findings."});
  }
}

// W2-28. Mutants killed: not sorting the findings, or failing (non-zero exit) when findings exist. All three rules fire,
// two subjects each, declared in reverse order of their (rule, subject) sort.
TEST_CASE("RA01-B2 critique findings are sorted advisory and exit zero", "[authoring][ra01][prompts]") {
  const std::string extra =
      "  prescriptions = L {\n    { id = \"pr_z\", scope = L { \"u1\" }, text = p.md \"Z.\" },\n"
      "    { id = \"pr_a\", scope = L { \"u1\" }, text = p.md \"A.\" }\n  },\n"
      "  questions = L {\n    { id = \"q_b\", scope = \"global\", prompt = p.md \"B?\", blocks = \"start\" },\n"
      "    { id = \"q_a\", scope = \"global\", prompt = p.md \"A?\", blocks = \"complete\" }\n  },\n"
      "  facets = L {\n    { id = \"f\", producer = \"u2\", paths = L { \"d_u2/x.py\" } }\n  },\n"
      "  dependencies = L {\n"
      "    { id = \"dep_y\", consumer = \"cons\", source = \"facet\", ref = \"f\", phase = \"complete\", quality = \"done\" },\n"
      "    { id = \"dep_b\", consumer = \"cons\", source = \"facet\", ref = \"f\", phase = \"complete\", quality = \"done\" }\n  "
      "},\n";
  w2p_tree t;
  w2p_install(t, w2p_critique_gen({{"u1", {"ck_na"}, {}}, {"u2", {"ck_ne"}, {}}, {"cons", {"ck_ne"}, {}}}, extra));
  const auto d = t.prompt("critique");
  CAPTURE(d.run.err);
  REQUIRE(d.run.status == 0);
  CHECK(d.run.err.empty());
  const std::vector<std::string> prefixes{"- blocking_question_unanswered q_a: ",
                                          "- blocking_question_unanswered q_b: ",
                                          "- facet_dependency_without_output_vector dep_b: ",
                                          "- facet_dependency_without_output_vector dep_y: ",
                                          "- prescription_unchecked pr_a: ",
                                          "- prescription_unchecked pr_z: "};
  const auto                     findings = w2p_findings(d);
  REQUIRE(findings.size() == prefixes.size());
  for (std::size_t i = 0; i < prefixes.size(); ++i) {
    CAPTURE(i, findings[i]);
    CHECK(findings[i].starts_with(prefixes[i]));
    CHECK(findings[i].size() > prefixes[i].size());
  }
}

// W2-29. Mutant killed: printing an empty findings section when no rule fires. The shipped quota spec triggers no rule.
TEST_CASE("RA01-B2 critique prints the fixed line when no rule fires", "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  const auto d = t.prompt("critique");
  REQUIRE(d.run.status == 0);
  const auto section = w2p_section(d.md, "Findings");
  REQUIRE(section.has_value());
  CHECK(w2p_trim(*section) == "No mechanical findings.");
  CHECK(w2p_findings(d) == std::vector<std::string>{"No mechanical findings."});
}

// ---- W2-30 ---------------------------------------------------------------------------------------------------------

// W2-30. Mutants killed: listing constructors unsorted, adding workspace or spec to the list, or omitting the statement that
// the job tier has no standard-library globals. No job file is read or created.
TEST_CASE("RA01-B2 job prompt lists sorted constructors and the job tier statement", "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  fs::remove(t.ws / "combined.lua");
  fs::remove(t.ws / "split.lua");
  const auto d = t.prompt("ingest");
  REQUIRE(d.run.status == 0);
  const auto section = w2p_section(d.md, "Constructors");
  REQUIRE(section.has_value());
  std::vector<std::string> bullets;
  for (const auto& line : w2p_lines(*section))
    if (line.starts_with("- "))
      bullets.push_back(line);
  CHECK(bullets == std::vector<std::string>{"- file", "- immaterial", "- job", "- list", "- md"});
  CHECK(w2p_has(*section, "standard-library"));
  CHECK(w2p_has(*section, "globals"));
  const auto job = w2p_section(d.md, "Job schema");
  REQUIRE(job.has_value());
  CHECK(w2p_has(*job, "- shape (required): enum pipeline|fanout_review"));
  CHECK_FALSE(fs::exists(t.ws / "combined.lua"));
  CHECK_FALSE(fs::exists(t.ws / "split.lua"));
}

// ---- W2-31 to W2-34 --------------------------------------------------------------------------------------------------

namespace {
void w2p_expect_schema_refusal(const w2p_tree& t, std::string_view label, const std::string& bytes) {
  INFO(std::string(label));
  const auto r = t.reject_raw(bytes);
  CHECK(r.status == 3);
  CHECK(r.out.empty());
  CHECK(std::ranges::count(r.err, '\n') == 1);
  const auto e = w2p_parse(r.err);
  CHECK(e.at("kind").s == "error");
  CHECK(e.at("error").at("code").s == "schema");
  CHECK_FALSE(e.at("error").has("file"));
  CHECK(e.at("error").at("field").s.starts_with("diagnostic"));
}
/// A syntax refusal carries the shared decoder's message and its details {offset, reason}, with the field exactly
/// `diagnostic` (a schema-stage refusal names a path below it).
void w2p_expect_decoder_refusal(const w2p_tree& t, std::string_view label, const std::string& bytes) {
  INFO(std::string(label));
  const auto r = t.reject_raw(bytes);
  CHECK(r.status == 3);
  const auto  e   = w2p_parse(r.err);
  const auto& err = e.at("error");
  CHECK(err.at("code").s == "schema");
  CHECK(err.at("field").s == "diagnostic");
  CHECK_FALSE(err.at("message").s.empty());
  REQUIRE(err.has("details"));
  CHECK(err.at("details").keys() == std::vector<std::string>{"offset", "reason"});
  CHECK(err.at("details").at("reason").s == err.at("message").s);
  CHECK(err.at("details").at("offset").n >= 1);
}
void w2p_expect_accepted(const w2p_tree& t, std::string_view label, const std::string& bytes) {
  INFO(std::string(label));
  const auto r = t.reject_raw(bytes);
  CHECK(r.status == 0);
  CHECK(r.err.empty());
  CHECK(w2p_parse(r.out).at("prompt").at("kind").s == "rejection");
}
} // namespace

// W2-31. Mutant killed: prompts.lua parsing the diagnostic with the lenient engine.json_decode instead of the strict shared
// parser. Duplicate keys, invalid UTF-8, NUL and trailing data are schema refusals; surrounding whitespace is accepted.
TEST_CASE("RA01-B2 explain-rejection refuses malformed diagnostic JSON as schema", "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  const std::string base = w2p_diag_base();
  w2p_expect_accepted(t, "plain", base + "\n");
  w2p_expect_accepted(t, "surrounding whitespace", "  \n\t" + base + "\r\n  ");
  w2p_expect_schema_refusal(t, "duplicate top-level key",
                            w2p_replace(base, "\"version\":1}", "\"kind\":\"error\",\"version\":1}"));
  w2p_expect_schema_refusal(t, "duplicate error key",
                            w2p_replace(base, "\"field\":\"spec.goal\"", "\"field\":\"spec.goal\",\"field\":\"x\""));
  w2p_expect_schema_refusal(t, "invalid UTF-8", w2p_replace(base, "Goal must", "Goal \xff must"));
  w2p_expect_schema_refusal(t, "overlong UTF-8", w2p_replace(base, "Goal must", "Goal \xc0\x80 must"));
  w2p_expect_schema_refusal(t, "NUL byte", w2p_replace(base, "Goal must", std::string("Goal ") + '\0' + " must"));
  w2p_expect_schema_refusal(t, "trailing data", base + " x");
  w2p_expect_schema_refusal(t, "trailing value", base + "\n{}");
  w2p_expect_schema_refusal(t, "empty file", "");
  w2p_expect_schema_refusal(t, "not json", "not json at all");
  // The refusal keeps the decoder's message and details (the lenient engine decoder returns them unmapped).
  w2p_expect_decoder_refusal(t, "decoder: not json", "not json at all");
  w2p_expect_decoder_refusal(t, "decoder: trailing data", base + " x");
  w2p_expect_decoder_refusal(t, "decoder: empty file", "");
}

// W2-32. Mutant killed: widening the code grammar to {0,64} or to uppercase letters. The envelope is closed: unknown
// fields, nulls, a non-integer line and a wrong version or kind are refused with the field starting diagnostic.
TEST_CASE("RA01-B2 explain-rejection validates the diagnostic envelope and code grammar", "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  const std::string core_fields = R"("field":"spec.goal","message":"m")";
  const auto        wrap        = [&](const std::string& error, const std::string& top = R"("kind":"error","version":1)") {
    return "{\"error\":" + error + (top.empty() ? "" : "," + top) + "}\n";
  };
  const auto with_error = [&](const std::string& members) { return wrap("{" + members + "}"); };
  const auto code       = [&](const std::string& c) { return with_error("\"code\":" + c + "," + core_fields); };
  w2p_expect_accepted(t, "base", code("\"schema\""));
  w2p_expect_accepted(
      t, "all optional members",
      with_error(R"("code":"schema","details":{"paths":["a","b"],"n":1},"field":"x","file":"spec.lua","line":3,"message":"m")"));
  w2p_expect_accepted(t, "empty field", with_error(R"("code":"schema","field":"","message":"m")"));
  w2p_expect_accepted(t, "single character code", code("\"x\""));
  w2p_expect_accepted(t, "digits and underscore", code("\"with_digits_9\""));
  w2p_expect_accepted(t, "64 characters", code("\"a" + std::string(63, 'b') + "\""));
  w2p_expect_schema_refusal(t, "65 characters", code("\"a" + std::string(64, 'b') + "\""));
  w2p_expect_schema_refusal(t, "uppercase", code("\"Schema\""));
  w2p_expect_schema_refusal(t, "uppercase inside", code("\"schEma\""));
  w2p_expect_schema_refusal(t, "leading digit", code("\"1abc\""));
  w2p_expect_schema_refusal(t, "leading underscore", code("\"_abc\""));
  w2p_expect_schema_refusal(t, "hyphen", code("\"a-b\""));
  w2p_expect_schema_refusal(t, "empty code", code("\"\""));
  w2p_expect_schema_refusal(t, "numeric code", code("5"));
  w2p_expect_schema_refusal(t, "version 2",
                            wrap(R"({"code":"schema","field":"f","message":"m"})", R"("kind":"error","version":2)"));
  w2p_expect_schema_refusal(t, "version string",
                            wrap(R"({"code":"schema","field":"f","message":"m"})", R"("kind":"error","version":"1")"));
  w2p_expect_schema_refusal(t, "version missing", wrap(R"({"code":"schema","field":"f","message":"m"})", R"("kind":"error")"));
  w2p_expect_schema_refusal(t, "kind load",
                            wrap(R"({"code":"schema","field":"f","message":"m"})", R"("kind":"load","version":1)"));
  w2p_expect_schema_refusal(t, "kind missing", wrap(R"({"code":"schema","field":"f","message":"m"})", R"("version":1)"));
  w2p_expect_schema_refusal(t, "extra top-level key",
                            wrap(R"({"code":"schema","field":"f","message":"m"})", R"("extra":1,"kind":"error","version":1)"));
  w2p_expect_schema_refusal(t, "error missing", "{\"kind\":\"error\",\"version\":1}\n");
  w2p_expect_schema_refusal(t, "error array", wrap("[]"));
  w2p_expect_schema_refusal(t, "error string", wrap("\"x\""));
  w2p_expect_schema_refusal(t, "error null", wrap("null"));
  w2p_expect_schema_refusal(t, "code missing", with_error(core_fields));
  w2p_expect_schema_refusal(t, "message missing", with_error(R"("code":"schema","field":"f")"));
  w2p_expect_schema_refusal(t, "field missing", with_error(R"("code":"schema","message":"m")"));
  w2p_expect_schema_refusal(t, "message number", with_error(R"("code":"schema","field":"f","message":5)"));
  w2p_expect_schema_refusal(t, "field null", with_error(R"("code":"schema","field":null,"message":"m")"));
  w2p_expect_schema_refusal(t, "file null", with_error(R"("code":"schema","field":"f","file":null,"message":"m")"));
  w2p_expect_schema_refusal(t, "file number", with_error(R"("code":"schema","field":"f","file":3,"message":"m")"));
  w2p_expect_schema_refusal(t, "line zero", with_error(R"("code":"schema","field":"f","line":0,"message":"m")"));
  w2p_expect_schema_refusal(t, "line negative", with_error(R"("code":"schema","field":"f","line":-3,"message":"m")"));
  w2p_expect_schema_refusal(t, "line fraction", with_error(R"("code":"schema","field":"f","line":1.5,"message":"m")"));
  w2p_expect_schema_refusal(t, "line string", with_error(R"("code":"schema","field":"f","line":"3","message":"m")"));
  w2p_expect_schema_refusal(t, "line null", with_error(R"("code":"schema","field":"f","line":null,"message":"m")"));
  w2p_expect_schema_refusal(t, "details array", with_error(R"("code":"schema","details":[],"field":"f","message":"m")"));
  w2p_expect_schema_refusal(t, "details null", with_error(R"("code":"schema","details":null,"field":"f","message":"m")"));
  w2p_expect_schema_refusal(t, "details string", with_error(R"("code":"schema","details":"x","field":"f","message":"m")"));
  w2p_expect_schema_refusal(t, "unknown error key", with_error(R"("code":"schema","extra":"x","field":"f","message":"m")"));
}

// W2-33. Mutants killed: capture_diagnostic comparing with > 65537 (off by one) or reading the whole file before checking its
// size. Exactly 65536 bytes are accepted; 65537 bytes are refused from the descriptor size, even when they are not JSON.
TEST_CASE("RA01-B2 explain-rejection accepts 65536 bytes and refuses 65537 as diagnostic_bytes", "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  std::string exact = w2p_diag_base();
  REQUIRE(exact.size() < 65536);
  exact.append(65536 - exact.size(), ' ');
  REQUIRE(exact.size() == 65536);
  const auto accepted = t.reject(exact);
  CHECK(accepted.run.status == 0);
  CHECK(accepted.run.err.empty());
  CHECK(accepted.env.at("prompt").at("inputs").at("diagnostic").at("bytes").n == 65536);
  const auto refuse = [&](std::string_view label, const std::string& bytes) {
    INFO(std::string(label));
    const auto r = t.reject_raw(bytes);
    CHECK(r.status == 3);
    CHECK(r.out.empty());
    CHECK(std::ranges::count(r.err, '\n') == 1);
    const auto e = w2p_parse(r.err);
    CHECK(e.at("error").at("code").s == "resource_limit");
    CHECK(e.at("error").at("message").s == "Capture limit exceeded");
    CHECK(e.at("error").at("details").at("limit").s == "diagnostic_bytes");
    CHECK(e.at("error").at("details").at("maximum").n == 65536);
  };
  std::string over = exact + " ";
  REQUIRE(over.size() == 65537);
  refuse("valid JSON padded to 65537", over);
  refuse("65537 bytes of garbage", std::string(65537, 'x'));
  refuse("one megabyte", std::string(1 << 20, ' '));
}

// W2-34 (guard). Mutant killed: prepare_inputs skipping the host_file check for the diagnostic path (the symlink would reach
// the capture and be read instead of refused). The messages are the existing ones and stay byte-identical; stdout is empty.
TEST_CASE("RA01-B2 read_diagnostic refuses links and special files with the existing messages", "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  w2p_tree::put(t.root / "real.json", w2p_diag_base() + "\n");
  const auto run = [&](const fs::path& diagnostic) { return t.run({"explain-rejection", "--diagnostic", diagnostic.string()}); };
  const auto expect = [&](std::string_view label, const fs::path& diagnostic, int status, std::string_view code,
                          std::string_view message) {
    INFO(std::string(label));
    const auto r = run(diagnostic);
    CHECK(r.status == status);
    CHECK(r.out.empty());
    CHECK(r.err == w2p_native_error(code, message));
  };
  fs::create_symlink(t.root / "real.json", t.root / "link.json");
  fs::create_symlink(t.root / "absent.json", t.root / "dangling.json");
  fs::create_directory(t.root / "dir.json");
  REQUIRE(::mkfifo((t.root / "fifo.json").c_str(), 0600) == 0);
  expect("symlink leaf", t.root / "link.json", 3, "path_denied", "Explicit input file is a symbolic link");
  expect("dangling symlink leaf", t.root / "dangling.json", 3, "path_denied", "Explicit input file is a symbolic link");
  expect("directory", t.root / "dir.json", 4, "io", "Explicit input must be a regular file");
  expect("fifo", t.root / "fifo.json", 4, "io", "Explicit input must be a regular file");
  expect("missing", t.root / "missing.json", 4, "io", "Cannot resolve explicit input path");
  // Control: a regular file passes the path stage, so none of the path-stage messages is reported for it.
  const auto regular = run(t.root / "real.json");
  CHECK_FALSE(w2p_has(regular.err, "Explicit input"));
  CHECK_FALSE(w2p_has(regular.err, "Cannot resolve explicit input path"));
  CHECK(w2p_slurp(t.root / "real.json") == w2p_diag_base() + "\n");
}

// ---- W2-35 to W2-40 --------------------------------------------------------------------------------------------------

// W2-35. Mutants killed: using fragment_digest for the diagnostic digest, or adding a registry key to the rejection inputs.
TEST_CASE("RA01-B2 rejection inputs carry the diagnostic size and file digest", "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  const std::string bytes = "  " + w2p_diag_base() + "\n\n";
  const auto        d     = t.reject(bytes);
  REQUIRE(d.run.status == 0);
  const auto& inputs = d.env.at("prompt").at("inputs");
  CHECK(inputs.keys() == std::vector<std::string>{"diagnostic", "files"});
  CHECK(inputs.at("files").k == w2p_json::kind::array);
  CHECK(inputs.at("files").items.empty());
  CHECK_FALSE(inputs.has("registry"));
  CHECK(inputs.at("diagnostic").keys() == std::vector<std::string>{"bytes", "digest"});
  CHECK(inputs.at("diagnostic").at("bytes").integer);
  CHECK(inputs.at("diagnostic").at("bytes").n == static_cast<long long>(bytes.size()));
  const auto file     = planar::engine::file_digest(bytes);
  const auto fragment = planar::engine::fragment_digest(bytes);
  REQUIRE(file.has_value());
  REQUIRE(fragment.has_value());
  CHECK(inputs.at("diagnostic").at("digest").s == *file);
  CHECK(inputs.at("diagnostic").at("digest").s != *fragment);
}

// W2-36. Mutant killed: the field-path resolver not stripping [N] suffixes (job.roles[1].work then resolves nothing). The
// excerpt is the resolved field's line with its subtree; the whole schema name gives the complete schema text.
TEST_CASE("RA01-B2 rejection excerpt resolves job.roles[1].work to the work field", "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  const auto excerpt = [&](std::string_view field) {
    const auto d = t.reject(w2p_diag_error("schema", field, "m"));
    CHECK(d.run.status == 0);
    const auto section = w2p_section(d.md, "Schema excerpt");
    CHECK(section.has_value());
    return section.value_or(std::string{});
  };
  CHECK(w2p_trim(excerpt("job.roles[1].work")) == "- work (optional): set of id");
  CHECK(w2p_trim(excerpt("job.roles[12].work")) == "- work (optional): set of id");
  CHECK(w2p_trim(excerpt("job.roles.work")) == "- work (optional): set of id");
  CHECK(w2p_trim(excerpt("workspace.root.id")) == "- id (required): id");
  CHECK(w2p_trim(excerpt("spec.goal")) == "- goal (required): text");
  // A subtree keeps its relative indentation.
  const auto subtree = w2p_lines(w2p_trim(excerpt("job.roles[1].preferences")));
  REQUIRE(subtree.size() == 3);
  CHECK(w2p_trim(subtree[0]) == "- preferences (optional): record");
  CHECK(w2p_trim(subtree[1]) == "- model (optional): string 256");
  CHECK(w2p_trim(subtree[2]) == "- session (optional): string 256");
  CHECK(w2p_indent(subtree[1]) == w2p_indent(subtree[0]) + 2);
  CHECK(w2p_indent(subtree[2]) == w2p_indent(subtree[0]) + 2);
  // A path of only the schema name resolves to the complete schema text of the same prompts.
  const auto job_prompt  = t.prompt("ingest");
  const auto spec_prompt = t.prompt("spec-prompt");
  REQUIRE(job_prompt.run.status == 0);
  REQUIRE(spec_prompt.run.status == 0);
  CHECK(excerpt("job") == w2p_section(job_prompt.md, "Job schema").value_or("<absent>"));
  CHECK(excerpt("spec") == w2p_section(spec_prompt.md, "Spec schema").value_or("<absent>"));
}

// W2-37. Mutant killed: accepting registry as a schema root in the field-path resolver. Every unresolvable path gives the
// fixed line; the control shows that a resolvable path does not.
TEST_CASE("RA01-B2 rejection excerpt for an unresolvable path is the fixed line", "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  const auto excerpt = [&](std::string_view field) {
    const auto d = t.reject(w2p_diag_error("schema", field, "m"));
    CHECK(d.run.status == 0);
    const auto section = w2p_section(d.md, "Schema excerpt");
    CHECK(section.has_value());
    return w2p_trim(section.value_or(std::string{}));
  };
  const std::string fixed = "No schema excerpt applies to this field.";
  CHECK(excerpt("job.roles[1].work") != fixed);
  for (const std::string field : {"registry", "registry.checks[1]", "registry.id", "observations", "observations.x", "bundle",
                                  "bundle.files[1].path", "$", "$.spec", "", "nonesuch", "nonesuch.id", "job.nonesuch",
                                  "workspace.root.nonesuch", "job.roles.work.zzz", "spec.work[1].nonesuch", "spec.goal.x"}) {
    CAPTURE(field);
    CHECK(excerpt(field) == fixed);
  }
}

// W2-38. Mutant killed: building raw_error with presentation_data (escaping) instead of the verbatim block. The diagnostic
// carries a raw DEL and a raw C1 character: the Error section escapes them, the raw block keeps the file bytes.
TEST_CASE("RA01-B2 raw_error block equals the diagnostic bytes with DEL and C1 raw", "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  const std::string bytes = w2p_replace(w2p_diag_base(), "Goal must", "bad \x7f \xc2\x85 end Goal must") + "\n";
  const auto        d     = t.reject(bytes);
  REQUIRE(d.run.status == 0);
  const auto raw = w2p_raw_error(d.md);
  REQUIRE(raw.found);
  CHECK(raw.data == bytes);
  CHECK(w2p_has(raw.data, "\x7f"));
  CHECK(w2p_has(raw.data, "\xc2\x85"));
  const auto error = w2p_section(d.md, "Error");
  REQUIRE(error.has_value());
  CHECK_FALSE(w2p_raw_controls(*error));
  CHECK(w2p_has(*error, "\\u007f"));
  CHECK(w2p_has(*error, "\\u0085"));
  CHECK(w2p_has(*error, "spec.goal"));
  CHECK(d.md.ends_with(raw.fence + "\n"));
}

// W2-39. Mutants killed: a fixed three-backtick fence, or always appending an LF. The fence is max(3, L + 1) backticks for the
// longest backtick run L in the file bytes (runs of 0, 2, 3 and 5 are tested); one LF is added only when the file lacks it.
TEST_CASE("RA01-B2 raw_error fence outgrows backtick runs and adds one LF only when missing", "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  struct row {
    std::string label, message;
    std::size_t fence;
  };
  const std::vector<row> rows{
      {"no backticks", "plain", 3},         {"run of 2", "a``b", 3}, {"run of 3", "a```b", 4}, {"run of 5", "a`````b", 6},
      {"runs of 2 and 5", "x``y`````z", 6},
  };
  for (const auto& r : rows) {
    CAPTURE(r.label);
    const std::string json = w2p_replace(w2p_diag_base(), "Goal must be nonempty", r.message);
    for (const std::string tail : {"", "\n", "\n\n"}) {
      CAPTURE(tail.size());
      const auto d = t.reject(json + tail);
      REQUIRE(d.run.status == 0);
      const auto raw = w2p_raw_error(d.md);
      REQUIRE(raw.found);
      CHECK(raw.fence == std::string(r.fence, '`'));
      CHECK(raw.data == json + (tail.empty() ? "\n" : tail));
    }
  }
}

// W2-40 (A36). Mutants killed: re-encoding the diagnostic canonically before embedding it, or dropping the field line from
// the error section. A rejected load in a scratch ledger is explained from its own stderr; load stays exit 3 and
// byte-identical before and after, and explain-rejection touches neither the ledger nor the tree. The real load stderr
// carries no DEL or C1 byte (no load error echoes user text); W2-38 covers those bytes with a synthetic diagnostic.
TEST_CASE("RA01-B2 A36 rejected load stderr round-trips through explain-rejection", "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  w2p_tree::edit(t.ws / "spec.lua", "goal = p.md \"Provide an atomic in-memory quota and a small command-line client.\"",
                 "goal = p.md \"\"");
  const auto ledger = (t.root / "a36.db").string();
  REQUIRE(t.run({"init", "--ledger", ledger, "--workspace-id", "quota"}).status == 0);
  const std::vector<std::string> load_args{"load",
                                           "--ledger",
                                           ledger,
                                           "--workspace",
                                           t.workspace().string(),
                                           "--registry",
                                           t.registry.string(),
                                           "--expect-revision",
                                           "none",
                                           "--request-id",
                                           "k",
                                           "--actor",
                                           "a"};
  const auto                     first = t.run(load_args);
  REQUIRE(first.status == 3);
  REQUIRE(first.out.empty());
  const auto typed = w2p_parse(first.err);
  REQUIRE(typed.at("error").at("field").s == "spec.goal");
  const auto ledger_before = w2p_slurp(ledger);
  const auto d             = t.reject(first.err);
  REQUIRE(d.run.status == 0);
  const auto error = w2p_section(d.md, "Error");
  REQUIRE(error.has_value());
  CHECK(w2p_has(*error, "spec.goal"));
  CHECK(w2p_has(*error, "schema"));
  CHECK(w2p_has(*error, "Goal must be nonempty"));
  const auto excerpt = w2p_section(d.md, "Schema excerpt");
  REQUIRE(excerpt.has_value());
  CHECK(w2p_trim(*excerpt) == "- goal (required): text");
  const auto raw = w2p_raw_error(d.md);
  REQUIRE(raw.found);
  CHECK(raw.data == first.err);
  CHECK(w2p_slurp(ledger) == ledger_before);
  const auto second = t.run(load_args);
  CHECK(second.status == 3);
  CHECK(second.out == first.out);
  CHECK(second.err == first.err);
  CHECK(w2p_slurp(ledger) == ledger_before);
}

// ---- W2-76 ---------------------------------------------------------------------------------------------------------

// W2-76 (guard). Mutant killed: schema.lua making the authoring mode always on (mode = mode or "spec_without_jobs" at the top
// of normalize_bundle): render and load would then ignore the listed jobs. Render and load behave as before. A listed job
// whose file is absent never reaches schema validation natively: capture refuses it as io, identically for both commands
// (the plan text names the schema diagnostic Listed job not captured, which only a hand-built bundle can reach). A job
// file whose id differs from the listing gives the same typed reference error from both, and a valid render still finds
// its job.
TEST_CASE("RA01-B2 render and load keep Listed job not captured outside the authoring modes", "[authoring][ra01][prompts]") {
  {
    w2p_tree t;
    t.quota();
    fs::remove(t.ws / "split.lua");
    const auto rendered = t.render("combined", "builder");
    const auto loaded   = t.load("led-absent");
    CHECK(rendered.status == 4);
    CHECK(loaded.status == 4);
    CHECK(rendered.out.empty());
    CHECK(loaded.out.empty());
    for (const auto* r : {&rendered, &loaded}) {
      const auto e = w2p_parse(r->err);
      CHECK(e.at("error").at("code").s == "io");
      CHECK(e.at("error").at("message").s == "Cannot inspect confined input");
    }
  }
  {
    w2p_tree t;
    t.quota();
    w2p_tree::edit(t.ws / "split.lua", "id = \"split\"", "id = \"splitx\"");
    const auto rendered = t.render("combined", "builder");
    const auto loaded   = t.load("led-mismatch");
    CHECK(rendered.status == 3);
    CHECK(loaded.status == 3);
    for (const auto* r : {&rendered, &loaded}) {
      const auto e = w2p_parse(r->err);
      CHECK(e.at("error").at("code").s == "reference");
      CHECK(e.at("error").at("field").s == "job.id");
      CHECK(e.at("error").at("file").s == "split.lua");
      CHECK(e.at("error").at("message").s == "Returned job ID does not match workspace");
    }
  }
  {
    w2p_tree t;
    t.quota();
    const auto first  = t.render("combined", "builder");
    const auto second = t.render("combined", "builder");
    CHECK(first.status == 0);
    CHECK(first.err.empty());
    CHECK(first.out.starts_with("{\"kind\":\"render\",\"manifest\":"));
    CHECK(first.out == second.out);
    const auto other = t.render("split", "cli_worker");
    CHECK(other.status == 0);
    const auto loaded = t.load("led-valid");
    CHECK(loaded.status == 0);
    const auto e = w2p_parse(loaded.out);
    CHECK(e.at("kind").s == "load");
    CHECK(e.at("outcome").s == "accepted");
    CHECK(e.at("revision").s == "1");
  }
}

// ---- RA01-B2 W2-C01 corrective (package T-B) -----------------------------------------------------------------------------

// Mutants (C01-20 and C01-21, invocation.cpp prompt_ceiling at line 666): the ceiling check `>` becomes `>=` (an exact
// 1048576 refused: section one), or the ceiling is raised by one (1048577 accepted: section two). The goal size that gives
// exactly 1048576 prompt bytes is found by MEASUREMENT: prompt bytes rise one for each goal byte but may also step up by
// one byte in between (the prompt overhead changes near goal size 1045300), so two-point extrapolation is not exact. The
// search is a secant search of the goal size on the measured prompt bytes; if no goal size gives exactly 1048576 the
// REQUIRE stops the test and the round must report it.
TEST_CASE("RA01-B2 C01 prompt of exactly 1048576 bytes is accepted and one more byte is refused", "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  const auto goal = std::string_view("goal = p.md \"Provide an atomic in-memory quota and a small command-line client.\"");
  const auto spec = w2p_slurp(t.ws / "spec.lua");
  const auto grow = [&](std::size_t size) {
    std::string text;
    while (text.size() < size)
      text += std::string(99, 'a') + "\n";
    text.resize(size);
    w2p_tree::put(t.ws / "text/big.md", text);
    w2p_tree::put(t.ws / "spec.lua", w2p_replace(spec, goal, "goal = p.file \"text/big.md\""));
  };
  // Prompt markdown bytes for a goal of `size` bytes; a refusal as prompt_bytes means the markdown is over the ceiling.
  const auto markdown_bytes = [&](std::size_t size) -> long long {
    grow(size);
    const auto r = t.run(t.prompt_args("ingest"));
    if (r.status == 3) {
      CAPTURE(size, r.err);
      REQUIRE(w2p_has(r.err, "\"limit\":\"prompt_bytes\""));
      return 1048577;
    }
    REQUIRE(r.status == 0);
    return w2p_make_doc(r).env.at("prompt").at("bytes").n;
  };
  // Secant search on measured markdown bytes: prompt bytes rise one per goal byte (with rare single-byte steps), so each
  // measurement moves the guess by the remaining difference; a refused (over-ceiling) guess steps down one byte. This
  // needs a handful of runs instead of a bisection, which exceeded the 30 s ctest limit on a 1 MiB prompt.
  std::size_t low   = 1040000;
  bool        found = false;
  for (int attempt = 0; attempt < 12 && !found; ++attempt) {
    const auto measured = markdown_bytes(low);
    if (measured == 1048576)
      found = true;
    else if (measured > 1048576)
      low -= 1;
    else
      low += static_cast<std::size_t>(1048576 - measured);
  }
  REQUIRE(found);
  const auto exact = low;
  SECTION("exactly 1048576 markdown bytes is accepted") {
    grow(exact);
    const auto d = w2p_make_doc(t.run(t.prompt_args("ingest")));
    CHECK(d.run.status == 0);
    CHECK(d.run.err.empty());
    CHECK(d.env.at("prompt").at("bytes").n == 1048576);
  }
  SECTION("one more byte is refused as prompt_bytes") {
    grow(exact + 1);
    const auto big = t.run(t.prompt_args("ingest"));
    CHECK(big.status == 3);
    CHECK(big.out.empty());
    CHECK(std::ranges::count(big.err, '\n') == 1);
    const auto e = w2p_parse(big.err);
    CHECK(e.at("kind").s == "error");
    CHECK(e.at("error").at("code").s == "resource_limit");
    CHECK(e.at("error").at("message").s == "Capture limit exceeded");
    CHECK(e.at("error").at("details").at("limit").s == "prompt_bytes");
    CHECK(e.at("error").at("details").at("maximum").n == 1048576);
  }
}

// Mutant (C01-22, prompts.lua global prescription branch at line 196): `unit_has(u.id, nonempty)` becomes an always-true
// predicate, so a global prescription whose work units hold only not_applicable checks is never reported. The control
// adds one nonempty check to a unit and must not fire.
TEST_CASE("RA01-B2 C01 critique global prescription needs a nonempty check", "[authoring][ra01][prompts]") {
  const std::string extra = "  prescriptions = L {\n    { id = \"pr_g\", scope = \"global\", text = p.md \"G.\" }\n  },\n";
  const std::string fired = "- prescription_unchecked pr_g: ";
  SECTION("every work unit holds only not_applicable checks") {
    w2p_tree t;
    w2p_install(t, w2p_critique_gen({{"u1", {"ck_na"}, {}}, {"u2", {"ck_na"}, {}}}, extra));
    const auto d = t.prompt("critique");
    REQUIRE(d.run.status == 0);
    const auto findings = w2p_findings(d);
    REQUIRE(findings.size() == 1);
    CHECK(findings[0].starts_with(fired));
    CHECK(findings[0].size() > fired.size());
  }
  SECTION("control: one unit also holds a nonempty check") {
    w2p_tree t;
    w2p_install(t, w2p_critique_gen({{"u1", {"ck_na"}, {}}, {"u2", {"ck_na", "ck_ne"}, {}}}, extra));
    const auto d = t.prompt("critique");
    REQUIRE(d.run.status == 0);
    for (const auto& line : w2p_findings(d))
      CHECK_FALSE(line.starts_with(fired));
  }
}

// Mutant (C01-23, prompts.lua strip_index at line 78): the digit test `if b<48 or b>57 then return nil end` deleted, so a
// non-digit index inside brackets resolves and the fixed line is not printed. The control job.roles[1].work resolves.
TEST_CASE("RA01-B2 C01 rejection excerpt with a non-digit index is the fixed line", "[authoring][ra01][prompts]") {
  w2p_tree t;
  t.quota();
  const auto excerpt = [&](std::string_view field) {
    const auto d = t.reject(w2p_diag_error("schema", field, "m"));
    CHECK(d.run.status == 0);
    const auto section = w2p_section(d.md, "Schema excerpt");
    CHECK(section.has_value());
    return w2p_trim(section.value_or(std::string{}));
  };
  const std::string fixed = "No schema excerpt applies to this field.";
  for (const std::string field :
       {"job.roles[x].work", "job.roles[1x].work", "job.roles[x1].work", "job.roles[].work", "job.roles[1][x].work"}) {
    CAPTURE(field);
    CHECK(excerpt(field) == fixed);
  }
  CHECK(excerpt("job.roles[1].work") == "- work (optional): set of id");
}
