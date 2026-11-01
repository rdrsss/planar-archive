// RA01-B2 orchestrator command tests over the compile-time selected authoring stack: opaque slots, the native unlisted job
// refusal, selected-job-only validation, the five rendered sections, the graph, prescriptions and the action labels.
// Expected bytes are typed from the authoring contract (RA-2, RA-7) and the pinned line formats of the round decisions.
#include <catch2/catch_test_macros.hpp>
#include <stdlib.h>
#include <unistd.h>
extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}
import std;
import planar.cmd.response;
import planar.cmd.authoring_stack.bootstrap;
import planar.engine.hash;

namespace {
namespace fs = std::filesystem;
using planar::engine::value_kind;
using planar::engine::value_ptr;

/// Exit status plus exact stdout and stderr bytes.
struct or_bytes {
  int         status = -1;
  std::string out, err;
};

/// Scratch copy of the quota fixture; the cwd of every invocation is `work`.
struct or_scratch {
  fs::path root, work, quota, workspace, registry, observations;
  or_scratch() {
    char pattern[] = "/tmp/planar-or-XXXXXX";
    REQUIRE(::mkdtemp(pattern));
    root  = fs::canonical(pattern);
    quota = root / "quota";
    fs::copy(fs::path(PLANAR_B2_WORKSPACE_FIXTURE).parent_path().parent_path(), quota, fs::copy_options::recursive);
    work         = root / "work";
    workspace    = quota / "workspace" / "workspace.lua";
    registry     = quota / "registry" / "quota-tools.json";
    observations = root / "obs";
    fs::create_directories(work);
    fs::create_directories(observations);
  }
  or_scratch(const or_scratch&)            = delete;
  or_scratch& operator=(const or_scratch&) = delete;
  ~or_scratch() {
    std::error_code ignored;
    fs::remove_all(root, ignored);
  }
  /// Overwrite one file under the quota workspace directory.
  void put(std::string_view relative, std::string_view bytes) const {
    std::ofstream out(quota / "workspace" / relative, std::ios::binary | std::ios::trunc);
    out << bytes;
    REQUIRE(out.good());
  }
  std::string read(std::string_view relative) const {
    std::ifstream in(quota / "workspace" / relative, std::ios::binary);
    return std::string{std::istreambuf_iterator<char>(in), {}};
  }
  void remove(std::string_view relative) const {
    REQUIRE(fs::remove(quota / "workspace" / relative));
  }
};

/// Scoped cwd and workspace-environment guard.
struct or_guard {
  fs::path saved;
  explicit or_guard(const fs::path& to) : saved(fs::current_path()) {
    fs::current_path(to);
    ::unsetenv("PLANAR_WORKSPACE_DIR");
  }
  or_guard(const or_guard&)            = delete;
  or_guard& operator=(const or_guard&) = delete;
  ~or_guard() {
    std::error_code ignored;
    fs::current_path(saved, ignored);
  }
};
or_bytes run(const or_scratch& s, const std::vector<std::string>& args) {
  or_guard                      guard(s.work);
  std::vector<std::string_view> views(args.begin(), args.end());
  const auto                    response = planar::cmd::authoring_stack::invoke(views);
  return {response.status, response.output, response.error};
}
std::vector<std::string> orchestrator_args(const or_scratch& s, std::string job) {
  return {"orchestrator", "--workspace", s.workspace.string(), "--registry", s.registry.string(), "--job", std::move(job)};
}
std::vector<std::string> render_args(const or_scratch& s, std::string job) {
  return {"render",
          "--workspace",
          s.workspace.string(),
          "--registry",
          s.registry.string(),
          "--root",
          "source=" + (s.quota / "current").string(),
          "--job",
          std::move(job),
          "--role",
          "builder"};
}
or_bytes orchestrate(const or_scratch& s, std::string job) {
  return run(s, orchestrator_args(s, std::move(job)));
}

/// Decode one JSON document, failing the test when it is not valid.
value_ptr decode(const std::string& bytes) {
  auto decoded = planar::engine::codec_decode(bytes);
  REQUIRE(decoded.has_value());
  return *decoded;
}
value_ptr field(const value_ptr& record, std::string_view name) {
  REQUIRE(record);
  REQUIRE(record->kind == value_kind::record);
  const auto found = record->fields.find(std::string(name));
  REQUIRE(found != record->fields.end());
  return found->second;
}
std::vector<std::string> keys(const value_ptr& record) {
  std::vector<std::string> out;
  for (const auto& [name, value] : record->fields)
    out.push_back(name);
  std::ranges::sort(out);
  return out;
}
/// The decoded error record of a failing invocation; stdout must be empty and stderr one closed envelope.
value_ptr error_record(const or_bytes& r) {
  CHECK(r.out.empty());
  REQUIRE_FALSE(r.err.empty());
  REQUIRE(r.err.back() == '\n');
  auto root = decode(r.err);
  CHECK(keys(root) == std::vector<std::string>{"error", "kind", "version"});
  CHECK(field(root, "kind")->string == "error");
  return field(root, "error");
}

/// A successful prompt: the envelope fields plus the Markdown split into the title and `## ` sections.
struct prompt_doc {
  value_ptr                             envelope, prompt;
  std::string                           markdown;
  int                                   titles = 0;
  std::vector<std::vector<std::string>> sections; ///< Body lines of each `## ` section in order.
  std::vector<std::string>              headings;
};
std::vector<std::string> split_lines(const std::string& text) {
  std::vector<std::string> out;
  std::size_t              begin = 0;
  while (begin < text.size()) {
    const auto end = text.find('\n', begin);
    if (end == std::string::npos) {
      out.push_back(text.substr(begin));
      break;
    }
    out.push_back(text.substr(begin, end - begin));
    begin = end + 1;
  }
  return out;
}
prompt_doc parse_prompt(const or_bytes& r) {
  INFO("stderr: " << r.err);
  REQUIRE(r.status == 0);
  CHECK(r.err.empty());
  REQUIRE_FALSE(r.out.empty());
  REQUIRE(r.out.back() == '\n');
  prompt_doc doc;
  doc.envelope = decode(r.out);
  CHECK(field(doc.envelope, "kind")->string == "prompt");
  doc.prompt = field(doc.envelope, "prompt");
  CHECK(field(doc.prompt, "kind")->string == "orchestrator");
  doc.markdown = field(doc.prompt, "markdown")->string;
  for (const auto& line : split_lines(doc.markdown)) {
    if (line.starts_with("## ")) {
      doc.headings.push_back(line);
      doc.sections.emplace_back();
    } else if (line.starts_with("# "))
      ++doc.titles;
    else if (!doc.sections.empty())
      doc.sections.back().push_back(line);
  }
  return doc;
}
/// Lines of the whole document starting with a prefix, in document order.
std::vector<std::string> lines_with(const prompt_doc& doc, std::string_view prefix) {
  std::vector<std::string> out;
  for (const auto& section : doc.sections)
    for (const auto& line : section)
      if (line.starts_with(prefix))
        out.push_back(line);
  return out;
}
const std::string pipeline_split = R"(local p = planar
local L = p.list
return p.job {
  version = 1, id = "split", shape = "pipeline", max_rounds = 3,
  stages = L { L { "library_worker" }, L { "cli_worker" } },
  roles = L {
    { id = "library_worker", kind = "worker", work = L { "library" } },
    { id = "cli_worker", kind = "worker", work = L { "command" } },
    { id = "reviewer", kind = "reviewer", targets = L { "library", "command" } }
  }
}
)";

/// The eleven names of the stack's sealed catalog and the twelve future names, typed from the contract.
const std::vector<std::string> catalog_names = {"critique", "explain-rejection", "export",       "history", "ingest",     "init",
                                                "load",     "observe",           "orchestrator", "render",  "spec-prompt"};
const std::vector<std::string> future_names  = {"activate",   "status",  "preview", "claim", "takeover",      "refresh",
                                                "checkpoint", "release", "recover", "check", "check-recover", "verdict"};
} // namespace

// Mutant (W2-06n): runtime.cpp prepare_authoring_invocation passes command_text(path) instead of the opaque handle for
// diagnostic or from. The Lua type check added in package I-B, wave 3 of the C01 plan (prompts.lua for explain-rejection,
// observe.lua for observe) then refuses internal exit 4, so both verbs would not exit 0. Those checks are unreachable on
// the shipped stack (the slots are always userdata). Only mutant W2-06n fails this test; deleting a check is killed by the source
// guard "RA01-B2 C01 slot type checks are pinned in the Lua handlers" at the end of this file.
TEST_CASE("RA01-B2 diagnostic and from reach Lua only as opaque slots", "[authoring][ra01][orchestrator]") {
  or_scratch s;
  const auto diagnostic = s.root / "diag.json";
  {
    std::ofstream out(diagnostic, std::ios::binary);
    out << "{\"error\":{\"code\":\"schema\",\"field\":\"spec.goal\",\"message\":\"Expected text\"},\"kind\":\"error\","
           "\"version\":1}\n";
  }
  const auto rejected = run(s, {"explain-rejection", "--diagnostic", diagnostic.string()});
  INFO("explain-rejection stderr: " << rejected.err);
  REQUIRE(rejected.status == 0);
  CHECK(rejected.err.empty());
  auto envelope = decode(rejected.out);
  CHECK(field(field(envelope, "prompt"), "kind")->string == "rejection");
  CHECK(field(field(envelope, "prompt"), "markdown")->string.contains("Expected text"));
  CHECK_FALSE(rejected.out.contains(s.root.string()));

  const auto observed = run(s, {"observe", "--from", s.observations.string()});
  INFO("observe stderr: " << observed.err);
  REQUIRE(observed.status == 0);
  CHECK(observed.err.empty());
  CHECK(observed.out == "{\"checks\":[],\"findings\":[],\"version\":1}\n");
  CHECK_FALSE(observed.out.contains(s.root.string()));
}

// Mutant (W2-64): capture_orchestrator skips the listing check (treats the unknown job as io or empty) or words its native
// diagnostic so that any byte differs from render's refusal of an unlisted --job.
TEST_CASE("RA01-B2 orchestrator refuses an unlisted job as reference selection.job", "[authoring][ra01][orchestrator]") {
  or_scratch s;
  const auto render = run(s, render_args(s, "ghost"));
  REQUIRE(render.status == 3);
  {
    auto e = error_record(render);
    REQUIRE(field(e, "code")->string == "reference");
    REQUIRE(field(e, "field")->string == "selection.job");
    REQUIRE(field(e, "message")->string == "Unknown selected job");
  }
  const auto ghost = orchestrate(s, "ghost");
  CHECK(ghost.status == 3);
  CHECK(ghost.out.empty());
  auto e = error_record(ghost);
  CHECK(field(e, "code")->string == "reference");
  CHECK(field(e, "field")->string == "selection.job");
  CHECK(field(e, "message")->string == "Unknown selected job");
  // Byte for byte with render's Lua site, including whether the file key is present.
  CHECK(ghost.err == render.err);
  CHECK_FALSE(ghost.err.contains(s.root.string()));
  // An ID that is not a valid declaration ID is refused with the same bytes.
  for (const std::string bad : {"Bad ID!", "pipeline:library:command", "a.b", "UPPER"}) {
    CAPTURE(bad);
    const auto refused = orchestrate(s, bad);
    CHECK(refused.status == 3);
    CHECK(refused.out.empty());
    CHECK(refused.err == render.err);
  }
  // Listed jobs are not refused as unknown.
  CHECK(orchestrate(s, "split").status == 0);
}

// Mutant (W2-65): capture_orchestrator loads every listed job like capture_render, or schema.lua selected_job mode does not
// skip the listed-job diagnostic for uncaptured other jobs.
TEST_CASE("RA01-B2 orchestrator reads only the selected job", "[authoring][ra01][orchestrator]") {
  or_scratch s;
  const auto baseline = orchestrate(s, "split");
  (void)parse_prompt(baseline);
  const auto original_combined = s.read("combined.lua");
  // A listed job file that is absent is ignored: nothing reports Listed job not captured and no byte changes.
  s.remove("combined.lua");
  const auto absent = orchestrate(s, "split");
  CHECK(absent.status == 0);
  CHECK(absent.err.empty());
  CHECK(absent.out == baseline.out);
  // A listed job file that is malformed is ignored in the same way.
  s.put("combined.lua", "return planar.job {");
  const auto malformed = orchestrate(s, "split");
  CHECK(malformed.status == 0);
  CHECK(malformed.err.empty());
  CHECK(malformed.out == baseline.out);
  s.put("combined.lua", "return planar.job {version=1,id=\"combined\"}\n");
  const auto invalid = orchestrate(s, "split");
  CHECK(invalid.status == 0);
  CHECK(invalid.out == baseline.out);
  // The selected job is validated and its typed error is the one load gives, which render shows for the same bytes
  // once the other job is valid again (mutant: the selected job is skipped or its error genericised).
  s.put("combined.lua", original_combined);
  s.put("split.lua", "return planar.job {version=1,id=\"split\"}\n");
  const auto proxy = run(s, render_args(s, "combined"));
  REQUIRE(proxy.status == 3);
  const auto refused = orchestrate(s, "split");
  CHECK(refused.status == 3);
  CHECK(refused.err == proxy.err);
  CHECK(field(error_record(refused), "file")->string == "split.lua");
  s.put("split.lua", "return planar.job {");
  const auto syntax_proxy = run(s, render_args(s, "combined"));
  REQUIRE(syntax_proxy.status == 3);
  const auto syntax = orchestrate(s, "split");
  CHECK(syntax.status == 3);
  CHECK(syntax.err == syntax_proxy.err);
  CHECK(orchestrate(s, "combined").status == 0);
}

// Mutant (W2-66): orchestrator.lua builds the graph from the base spec graph without the pipeline predecessor edges, leaves
// the roles unsorted, drops the stages or sorts the work set wrongly.
TEST_CASE("RA01-B2 orchestrator sections render shape roles and graph", "[authoring][ra01][orchestrator]") {
  or_scratch s;
  s.put("split.lua", pipeline_split);
  const auto pipeline = parse_prompt(orchestrate(s, "split"));
  CHECK(lines_with(pipeline, "- shape: ") == std::vector<std::string>{"- shape: pipeline max_rounds 3"});
  CHECK(lines_with(pipeline, "- stage ") == std::vector<std::string>{"- stage 1: library_worker", "- stage 2: cli_worker"});
  // Roles are sorted by ID although declared library_worker, cli_worker, reviewer; work and targets are sorted.
  CHECK(lines_with(pipeline, "- cli_worker (") == std::vector<std::string>{"- cli_worker (worker) work: command"});
  std::vector<std::string> roles;
  for (const auto& line : lines_with(pipeline, "- "))
    if (line.contains(" (worker) work: ") || line.contains(" (reviewer) targets: "))
      roles.push_back(line);
  CHECK(roles == std::vector<std::string>{"- cli_worker (worker) work: command", "- library_worker (worker) work: library",
                                          "- reviewer (reviewer) targets: command, library"});
  // Vertices sorted in unsigned byte order.
  std::vector<std::string> vertices = {"role:cli_worker:complete", "role:library_worker:complete", "role:reviewer:complete",
                                       "role:reviewer:start",      "work:command:accepted",        "work:command:complete",
                                       "work:command:start",       "work:library:accepted",        "work:library:complete",
                                       "work:library:start"};
  std::vector<std::string> expected_vertices;
  for (const auto& vertex : vertices)
    expected_vertices.push_back("- vertex " + vertex);
  CHECK(lines_with(pipeline, "- vertex ") == expected_vertices);
  // Edges of the fully expanded phase graph (declarations), including the derived pipeline edge, sorted by (from, to).
  std::vector<std::pair<std::string, std::string>> edges = {
      {"role:reviewer:start", "role:reviewer:complete"},
      {"work:command:start", "work:command:complete"},
      {"work:command:complete", "work:command:accepted"},
      {"work:command:complete", "role:cli_worker:complete"},
      {"role:reviewer:complete", "work:command:accepted"},
      {"work:library:start", "work:library:complete"},
      {"work:library:complete", "work:library:accepted"},
      {"work:library:complete", "role:library_worker:complete"},
      {"role:reviewer:complete", "work:library:accepted"},
      {"role:cli_worker:complete", "role:reviewer:start"},
      {"role:library_worker:complete", "role:reviewer:start"},
      {"work:library:complete", "work:command:complete"}, // integrate: facet, complete, done
      {"work:library:complete", "work:command:start"},    // derived pipeline predecessor edge
  };
  std::ranges::sort(edges);
  std::vector<std::string> expected_edges;
  for (const auto& [from, to] : edges)
    expected_edges.push_back("- edge " + from + " -> " + to);
  CHECK(lines_with(pipeline, "- edge ") == expected_edges);

  // The fanout job has no stage lines, the fanout shape line and no derived edge.
  const auto fanout = parse_prompt(orchestrate(s, "combined"));
  CHECK(lines_with(fanout, "- shape: ") == std::vector<std::string>{"- shape: fanout_review max_rounds 2"});
  CHECK(lines_with(fanout, "- stage ").empty());
  std::vector<std::string> fanout_roles;
  for (const auto& line : lines_with(fanout, "- "))
    if (line.contains(" (worker) work: ") || line.contains(" (reviewer) targets: "))
      fanout_roles.push_back(line);
  CHECK(fanout_roles ==
        std::vector<std::string>{"- builder (worker) work: command, library", "- reviewer (reviewer) targets: command, library"});
  const auto fanout_edges = lines_with(fanout, "- edge ");
  CHECK(std::ranges::find(fanout_edges, "- edge work:library:complete -> work:command:start") == fanout_edges.end());
  CHECK(std::ranges::find(fanout_edges, "- edge work:library:complete -> work:command:complete") != fanout_edges.end());
}

// Mutant (W2-67): orchestrator.lua lists the prescriptions unsorted or with the wrong scope text. Limit: every valid job
// targets all work units, so a prescription that does not intersect the job cannot exist and the filter clause itself is
// not observable through the CLI; the global scope, the sort by ID and the scope text are.
TEST_CASE("RA01-B2 orchestrator lists only the prescriptions that drive the job", "[authoring][ra01][orchestrator]") {
  or_scratch s;
  auto       spec  = s.read("spec.lua");
  const auto begin = spec.find("  prescriptions = L {");
  const auto end   = spec.find("  contracts = L {");
  REQUIRE(begin != std::string::npos);
  REQUIRE(end != std::string::npos);
  spec.replace(begin, end - begin,
               "  prescriptions = L {\n"
               "    { id = \"zz_global\", scope = \"global\", text = p.md \"Global rule text.\" },\n"
               "    { id = \"library_rule\", scope = L { \"library\" }, text = p.md \"Library rule text.\" },\n"
               "    { id = \"b_both\", scope = L { \"library\", \"command\" }, text = p.md \"Both units rule text.\" },\n"
               "    { id = \"command_rule\", scope = L { \"command\" }, text = p.md \"Command rule text.\" }\n"
               "  },\n");
  s.put("spec.lua", spec);
  const auto               doc = parse_prompt(orchestrate(s, "combined"));
  std::vector<std::string> headings;
  for (const auto& line : lines_with(doc, "- "))
    for (const std::string_view id : {"b_both", "command_rule", "library_rule", "zz_global"})
      if (line.starts_with("- " + std::string(id) + " ("))
        headings.push_back(line);
  REQUIRE(headings.size() == 4);
  CHECK(headings[0].starts_with("- b_both ("));
  CHECK(headings[0] == "- b_both (command, library)");
  CHECK(headings[1].starts_with("- command_rule (command)"));
  CHECK(headings[2].starts_with("- library_rule (library)"));
  CHECK(headings[3] == "- zz_global (global)");
  // Each text follows its own heading, in the same order.
  std::size_t at = 0;
  for (const std::string_view needle : {"- b_both (", "Both units rule text.", "- command_rule (", "Command rule text.",
                                        "- library_rule (", "Library rule text.", "- zz_global (", "Global rule text."}) {
    CAPTURE(needle);
    const auto found = doc.markdown.find(needle, at);
    REQUIRE(found != std::string::npos);
    at = found;
  }
}

// Mutant (W2-68): orchestrator.lua swaps the roles and graph sections or omits the actions section.
TEST_CASE("RA01-B2 orchestrator prompt carries five sections in order", "[authoring][ra01][orchestrator]") {
  or_scratch s;
  s.put("split.lua", pipeline_split);
  const auto doc = parse_prompt(orchestrate(s, "split"));
  CHECK(doc.titles == 1);
  CHECK(doc.markdown.starts_with("# "));
  REQUIRE(doc.sections.size() == 5);
  const auto has = [&](std::size_t index, std::string_view marker) {
    return std::ranges::any_of(doc.sections[index], [&](const std::string& line) { return line.contains(marker); });
  };
  // Authority first: it states no graph, role, prescription or action line.
  CHECK_FALSE(has(0, "- vertex "));
  CHECK_FALSE(has(0, " (worker) work: "));
  CHECK_FALSE(has(0, "(available)"));
  CHECK(std::ranges::any_of(doc.sections[0], [](const std::string& line) { return !line.empty(); }));
  CHECK(has(1, "- shape: "));
  CHECK(has(1, "- vertex "));
  CHECK(has(1, "- edge "));
  CHECK_FALSE(has(1, " (worker) work: "));
  CHECK(has(2, " (worker) work: "));
  CHECK(has(2, " (reviewer) targets: "));
  CHECK_FALSE(has(2, "- vertex "));
  CHECK(has(3, "library_rule"));
  CHECK(has(3, "command_rule"));
  CHECK_FALSE(has(3, "(available)"));
  CHECK(has(4, "- orchestrator (available): "));
  CHECK(has(4, "- verdict (future): "));
  // Envelope shape of RA-3: closed keys, one final LF, byte count, fragment digest form, inputs form.
  CHECK(keys(doc.prompt) == std::vector<std::string>{"bytes", "digest", "inputs", "kind", "markdown"});
  CHECK(field(doc.prompt, "bytes")->integer == static_cast<std::int64_t>(doc.markdown.size()));
  CHECK(doc.markdown.ends_with("\n"));
  CHECK_FALSE(doc.markdown.ends_with("\n\n"));
  CHECK(field(doc.prompt, "digest")->string.starts_with("xxh3_128:"));
  auto inputs = field(doc.prompt, "inputs");
  CHECK(keys(inputs) == std::vector<std::string>{"files", "registry"});
  auto registry = field(inputs, "registry");
  CHECK(keys(registry) == std::vector<std::string>{"id", "revision"});
  CHECK(field(registry, "id")->string == "quota_tools");
  CHECK(field(registry, "revision")->string == "v1");
  auto files = field(inputs, "files");
  REQUIRE_FALSE(files->items.empty());
  std::vector<std::pair<std::string, std::string>> order;
  for (const auto& file : files->items) {
    CHECK(keys(file) == std::vector<std::string>{"digest", "kind", "mode", "path", "root", "size"});
    order.emplace_back(field(file, "root")->string, field(file, "path")->string);
    // Each entry carries the size and file digest of the captured bytes (the digest of the file, not of a fragment).
    const auto root = field(file, "root")->string;
    CAPTURE(root, field(file, "path")->string);
    REQUIRE((root == "workspace" || root == "registry"));
    const auto        disk = root == "workspace" ? s.workspace.parent_path() / field(file, "path")->string : s.registry;
    std::ifstream     in(disk, std::ios::binary);
    const std::string bytes{std::istreambuf_iterator<char>(in), {}};
    const auto        digest = planar::engine::file_digest(bytes);
    REQUIRE(digest.has_value());
    CHECK(field(file, "size")->integer == static_cast<std::int64_t>(bytes.size()));
    CHECK(field(file, "digest")->string == *digest);
  }
  CHECK(std::ranges::is_sorted(order));
  // Only the selected job file is captured: split.lua is listed, combined.lua is not.
  const auto listed = [&](std::string_view path) {
    return std::ranges::any_of(order, [&](const auto& entry) { return entry.second == path; });
  };
  CHECK(listed("split.lua"));
  CHECK_FALSE(listed("combined.lua"));
}

// Mutant (W2-69): orchestrator.lua marks every action future, drops check-recover from the future list, emits the lines
// unsorted, or lets a purpose be empty or longer than 200 bytes.
TEST_CASE("RA01-B2 orchestrator labels every catalog verb available and every future verb future",
          "[authoring][ra01][orchestrator]") {
  or_scratch s;
  const auto doc     = parse_prompt(orchestrate(s, "combined"));
  const auto actions = lines_with(doc, "- ");
  struct action {
    std::string command, status, purpose;
  };
  std::vector<action> parsed;
  for (const auto& line : actions) {
    const auto open  = line.find(" (");
    const auto close = line.find("): ");
    if (open == std::string::npos || close == std::string::npos || open > close)
      continue;
    const auto status = line.substr(open + 2, close - open - 2);
    if (status != "available" && status != "future")
      continue;
    parsed.push_back({line.substr(2, open - 2), status, line.substr(close + 3)});
  }
  std::vector<std::string> expected_commands = catalog_names;
  expected_commands.insert(expected_commands.end(), future_names.begin(), future_names.end());
  std::ranges::sort(expected_commands);
  REQUIRE(parsed.size() == 23);
  std::vector<std::string> commands;
  for (const auto& entry : parsed) {
    commands.push_back(entry.command);
    CAPTURE(entry.command);
    CHECK_FALSE(entry.purpose.empty());
    CHECK(entry.purpose.size() <= 200);
    const bool in_catalog = std::ranges::find(catalog_names, entry.command) != catalog_names.end();
    CHECK(entry.status == (in_catalog ? "available" : "future"));
  }
  CHECK(commands == expected_commands);
  CHECK(std::ranges::count_if(parsed, [](const action& a) { return a.status == "available"; }) == 11);
  CHECK(std::ranges::count_if(parsed, [](const action& a) { return a.status == "future"; }) == 12);
  // with-profile is never an action.
  CHECK(std::ranges::find(commands, "with-profile") == commands.end());
}

// Mutant (W2-71): orchestrator.lua renders role facets, outputs, observations, preferences, notes or spec facts. Limit: the
// workspace, spec and job IDs of the context are observable only through the rendered sections (D-36).
TEST_CASE("RA01-B2 orchestrator prompt is graph only", "[authoring][ra01][orchestrator]") {
  or_scratch s;
  s.put("combined.lua", R"(local p = planar
local L = p.list
return p.job {
  version = 1, id = "combined", shape = "fanout_review", budget_bytes = 131072,
  notes = p.md "job-note-sentinel",
  roles = L {
    { id = "builder", kind = "worker", work = L { "library", "command" },
      preferences = { model = "model-sentinel-77", session = "session-sentinel-77" } },
    { id = "reviewer", kind = "reviewer", targets = L { "library", "command" } }
  }
}
)");
  const auto doc = parse_prompt(orchestrate(s, "combined"));
  REQUIRE(doc.sections.size() == 5);
  for (const std::string_view absent :
       {"job-note-sentinel", "model-sentinel-77", "session-sentinel-77", "library_files", "quota_api", "output_vector",
        "Implement Quota.try_take", "Implement quota CLI", "A tiny in-memory quota", "atomic in-memory quota",
        "Never permit a negative balance", "A rejected take preserves", "Fixture author declares", "python"}) {
    CAPTURE(absent);
    CHECK_FALSE(doc.markdown.contains(absent));
  }
  // What the graph does need is present: the roles, the work IDs and the prescriptions that drive the job.
  CHECK(doc.markdown.contains("builder"));
  CHECK(doc.markdown.contains("library_rule"));
  CHECK(doc.markdown.contains("command_rule"));
  CHECK_FALSE(doc.markdown.contains(s.root.string()));
}

// Mutant (W2-80): schema.lua selected_job mode skips the whole job loop and the listing checks instead of only the Listed
// job not captured diagnostic for jobs that were not captured. The Unlisted job declaration branch cannot be reached
// through the CLI (native captures the file the workspace lists); it is covered by the provider test in the variants file.
TEST_CASE("RA01-B2 orchestrator selected-job mode still checks the listed job identity", "[authoring][ra01][orchestrator]") {
  or_scratch s;
  const auto baseline = orchestrate(s, "split");
  (void)parse_prompt(baseline);
  const auto original_combined = s.read("combined.lua");
  // Another job whose returned ID disagrees with the workspace is not captured and is ignored.
  s.put("combined.lua",
        "local p = planar\nlocal L = p.list\nreturn p.job { version = 1, id = \"other\", shape = \"fanout_review\", "
        "roles = L { { id = \"builder\", kind = \"worker\", work = L { \"library\", \"command\" } }, "
        "{ id = \"reviewer\", kind = \"reviewer\", targets = L { \"library\", \"command\" } } } }\n");
  const auto ignored = orchestrate(s, "split");
  CHECK(ignored.status == 0);
  CHECK(ignored.out == baseline.out);
  // The selected job whose returned ID disagrees with the workspace listing is refused, exactly as render refuses it.
  s.put("split.lua",
        "local p = planar\nlocal L = p.list\nreturn p.job { version = 1, id = \"other_id\", shape = \"fanout_review\", "
        "roles = L { { id = \"library_worker\", kind = \"worker\", work = L { \"library\" } }, "
        "{ id = \"cli_worker\", kind = \"worker\", work = L { \"command\" } }, "
        "{ id = \"reviewer\", kind = \"reviewer\", targets = L { \"library\", \"command\" } } } }\n");
  s.put("combined.lua", original_combined);
  const auto proxy = run(s, render_args(s, "combined"));
  REQUIRE(proxy.status == 3);
  const auto refused = orchestrate(s, "split");
  CHECK(refused.status == 3);
  CHECK(refused.err == proxy.err);
  auto e = error_record(refused);
  CHECK(field(e, "code")->string == "reference");
  CHECK(field(e, "message")->string == "Returned job ID does not match workspace");
  CHECK(field(e, "field")->string == "job.id");
  CHECK(field(e, "file")->string == "split.lua");
}

namespace {
/// Whitespace-free text of a stdlib Lua source beside the authoring stack; empty when it cannot be read.
std::string c01_squashed_lua(const fs::path& file) {
  std::ifstream     in(file, std::ios::binary);
  const std::string text{std::istreambuf_iterator<char>(in), {}};
  std::string       out;
  for (const char c : text)
    if (!std::isspace(static_cast<unsigned char>(c)))
      out += c;
  return out;
}
std::size_t c01_occurrences(std::string_view text, std::string_view needle) {
  std::size_t count = 0;
  for (auto at = text.find(needle); at != std::string_view::npos; at = text.find(needle, at + needle.size()))
    ++count;
  return count;
}
} // namespace

// Mutants (C01-45, D-C01-33 spellings): deleting the Lua type check from prompts_explain_rejection (an equivalent mutant
// of the opaque-slot test above, because the check is unreachable on the shipped stack and so it is killed only here), or
// changing the observe.lua check's code from internal to io. Each check must occur exactly once, before the first read of
// its handler. Stated limit: the checks are defensive and unreachable on the shipped stack (the slots are always userdata),
// so no behavioural test observes them, and the refusal bytes (exit 4, internal / Invalid embedded command protocol) are
// pinned by this source guard only. No seam may be added to reach them.
TEST_CASE("RA01-B2 C01 slot type checks are pinned in the Lua handlers", "[authoring][ra01]") {
  const fs::path stdlib = fs::path(__FILE__).parent_path() / ".." / ".." / "stdlib" / "planar";
  SECTION("explain-rejection rejects a non-userdata diagnostic slot before any read") {
    const auto prompts = c01_squashed_lua(stdlib / "prompts.lua");
    REQUIRE_FALSE(prompts.empty());
    const std::string check = R"lua(iftype(invocation.options.diagnostic)~="userdata")lua"
                              R"lua(thenreturnprompts_kit.fail("internal","Invalidembeddedcommandprotocol")end)lua";
    CHECK(c01_occurrences(prompts, check) == 1);
    const auto handler = prompts.find("localfunctionprompts_explain_rejection(");
    REQUIRE(handler != std::string::npos);
    const auto first_read = prompts.find("context.read_", handler);
    const auto bound      = first_read == std::string::npos ? prompts.size() : first_read;
    const auto at         = prompts.find(check);
    CHECK((at != std::string::npos && at > handler && at < bound));
  }
  SECTION("observe rejects a non-userdata from slot before any read") {
    const auto observe = c01_squashed_lua(stdlib / "observe.lua");
    REQUIRE_FALSE(observe.empty());
    const std::string check = R"lua(iftype(invocation.options.from)~="userdata"thenreturn{ok=false,error={code="internal",)lua"
                              R"lua(message="Invalidembeddedcommandprotocol",field="",details={}}}end)lua";
    CHECK(c01_occurrences(observe, check) == 1);
    const auto first_read = observe.find("context.read_observation_directory");
    const auto bound      = first_read == std::string::npos ? observe.size() : first_read;
    const auto at         = observe.find(check);
    CHECK((at != std::string::npos && at < bound));
  }
}

namespace {
// One test-built Lua state with the base, table and string libraries only; closed when the owning test scope ends.
struct ra01_cc01_vm {
  lua_State* l = luaL_newstate();
  ra01_cc01_vm() {
    if (l == nullptr)
      return;
    luaL_requiref(l, "_G", luaopen_base, 1);
    lua_pop(l, 1);
    luaL_requiref(l, "table", luaopen_table, 1);
    lua_pop(l, 1);
    luaL_requiref(l, "string", luaopen_string, 1);
    lua_pop(l, 1);
  }
  ra01_cc01_vm(const ra01_cc01_vm&)            = delete;
  ra01_cc01_vm& operator=(const ra01_cc01_vm&) = delete;
  ~ra01_cc01_vm() {
    if (l)
      lua_close(l);
  }
};

// Text of the value at idx when it is a string, otherwise empty. Never converts a number in place.
std::string ra01_cc01_text(lua_State* l, int idx) {
  return lua_type(l, idx) == LUA_TSTRING ? std::string(lua_tostring(l, idx)) : std::string{};
}

// Text of field key of the table at the absolute index idx, empty when absent or not a string; the stack is unchanged.
std::string ra01_cc01_field_text(lua_State* l, int idx, const char* key) {
  lua_getfield(l, idx, key);
  std::string text = ra01_cc01_text(l, -1);
  lua_pop(l, 1);
  return text;
}

// Loads and calls a chunk that returns one value, leaving that value on the stack; each failure reports the Lua text.
void ra01_cc01_run(lua_State* l, std::string_view text, const char* name) {
  const int loaded = luaL_loadbufferx(l, text.data(), text.size(), name, "t");
  CAPTURE(ra01_cc01_text(l, -1));
  REQUIRE(loaded == LUA_OK);
  const int called = lua_pcall(l, 0, 1, 0);
  CAPTURE(ra01_cc01_text(l, -1));
  REQUIRE(called == LUA_OK);
}

// Loads the real orchestrator.lua bytes with the consumer return appended, finds the file-local project() as the upvalue
// named project, calls it with the minimal context and leaves its runtime value as the only value on the stack. The
// context has empty graph, roles, prescriptions and actions, so presentation_data, sorted and list_text are never reached.
void ra01_cc01_project(lua_State* l) {
  const fs::path file = fs::path(__FILE__).parent_path() / ".." / ".." / "stdlib" / "planar" / "orchestrator.lua";
  std::ifstream  in(file, std::ios::binary);
  std::string    data{std::istreambuf_iterator<char>(in), {}};
  REQUIRE_FALSE(data.empty());
  data += "\nreturn orchestrator_command_impl";
  ra01_cc01_run(l, data, "orchestrator.lua");
  REQUIRE(lua_isfunction(l, -1));
  bool found = false;
  for (int i = 1;; ++i) {
    const char* name = lua_getupvalue(l, -1, i);
    if (name == nullptr)
      break;
    if (std::string_view(name) == "project") {
      found = true;
      break;
    }
    lua_pop(l, 1);
  }
  REQUIRE(found);
  lua_remove(l, -2);
  REQUIRE(lua_isfunction(l, -1));
  ra01_cc01_run(
      l,
      R"lua(return {workspace={id="ws"},spec={id="sp"},job={id="jb"},shape={kind="loop",max_rounds=3,stages={}},graph={vertices={},edges={}},roles={},prescriptions={},actions={}})lua",
      "context");
  const int called = lua_pcall(l, 1, 1, 0);
  CAPTURE(ra01_cc01_text(l, -1));
  REQUIRE(called == LUA_OK);
}
} // namespace

// RA01-C-C01 projection wrapper. Mutants, all in orchestrator.lua: the wrapper version dropped, 2, "1" or 1.0 (I1, I2);
// a bare `return sections`, an extra wrapper key, or the wrapper return moved outside project() (I1); the mandatory flag
// dropped from the section helper, set false, stringified as "true", moved onto the wrapper, or reordered with
// table.insert (I3a, I3b, I3d, I3e, I3f); one section given mandatory=false by a dot, bracket or string-built key (I3c);
// the roles and graph calls swapped, one deleted or a sixth added (I4); and the consumer iterating project(c) directly,
// bypassing the wrapper (I5). Kills per SECTION: wrapper shape kills I1 and I2 and the wrapper-level part of I3d;
// sections order and count kills I4 and the I3e reorder; every section mandatory kills I3a, I3b, I3c (all three spellings),
// I3d and I3f. Source pins is a supplement that kills the wrapper literal, helper literal, mandatory count and consumer
// bypass (I5). Limit (D22): the test loads the same orchestrator.lua bytes in a test-built Lua state with base, table and
// string libraries only, not through the embedded authoring stack; the minimal context leaves graph, roles, prescriptions
// and actions empty, so non-empty section bodies are covered by the existing goldens (I6); orchestrator_command_impl, the
// consumer, is pinned by source text in the supplement and by the existing behavioural test RA01-B2 orchestrator prompt
// carries five sections in order (I5b, D20). No seam, hook or new public symbol.
TEST_CASE("RA01-C-C01 orchestrator projection wrapper is versioned and mandatory", "[authoring][ra01][orchestrator]") {
  SECTION("wrapper shape") {
    ra01_cc01_vm vm;
    REQUIRE(vm.l != nullptr);
    lua_State* l = vm.l;
    ra01_cc01_project(l);
    REQUIRE(lua_istable(l, -1));
    const int wrapper = lua_gettop(l);
    // version must be the integer 1: a string "1", a float 1.0, 2 or an absent field all fail.
    lua_getfield(l, wrapper, "version");
    CHECK(lua_type(l, -1) == LUA_TNUMBER);
    CHECK(lua_isinteger(l, -1) != 0);
    CHECK(lua_tointeger(l, -1) == 1);
    lua_settop(l, wrapper);
    // No array part, and exactly the two keys version and sections (a bare array or an extra key fails).
    CHECK(static_cast<std::size_t>(lua_rawlen(l, wrapper)) == 0);
    std::size_t keys = 0;
    lua_pushnil(l);
    while (lua_next(l, wrapper) != 0) {
      CHECK(lua_type(l, -2) == LUA_TSTRING);
      const std::string key = ra01_cc01_text(l, -2);
      CHECK((key == "version" || key == "sections"));
      ++keys;
      lua_pop(l, 1);
    }
    CHECK(keys == 2);
    lua_getfield(l, wrapper, "sections");
    CHECK(lua_istable(l, -1) != 0);
    lua_settop(l, wrapper);
  }
  SECTION("sections order and count") {
    ra01_cc01_vm vm;
    REQUIRE(vm.l != nullptr);
    lua_State* l = vm.l;
    ra01_cc01_project(l);
    REQUIRE(lua_istable(l, -1));
    const int wrapper = lua_gettop(l);
    lua_getfield(l, wrapper, "sections");
    REQUIRE(lua_istable(l, -1));
    const int sections = lua_gettop(l);
    CHECK(static_cast<std::size_t>(lua_rawlen(l, sections)) == 5);
    std::size_t keys = 0;
    lua_pushnil(l);
    while (lua_next(l, sections) != 0) {
      ++keys;
      lua_pop(l, 1);
    }
    CHECK(keys == 5);
    const std::array<std::string_view, 5> expected{"orchestrator_authority", "orchestrator_graph", "orchestrator_roles",
                                                   "orchestrator_prescriptions", "orchestrator_actions"};
    for (int i = 0; i < 5; ++i) {
      CAPTURE(i + 1);
      CAPTURE(expected[static_cast<std::size_t>(i)]);
      lua_rawgeti(l, sections, i + 1);
      const int  entry    = lua_gettop(l);
      const bool is_table = lua_istable(l, entry) != 0;
      CHECK(is_table);
      const std::string id = is_table ? ra01_cc01_field_text(l, entry, "id") : std::string{};
      CHECK(std::string_view(id) == expected[static_cast<std::size_t>(i)]);
      lua_settop(l, sections);
    }
  }
  SECTION("every section mandatory") {
    ra01_cc01_vm vm;
    REQUIRE(vm.l != nullptr);
    lua_State* l = vm.l;
    ra01_cc01_project(l);
    REQUIRE(lua_istable(l, -1));
    const int wrapper = lua_gettop(l);
    lua_getfield(l, wrapper, "sections");
    REQUIRE(lua_istable(l, -1));
    const int sections = lua_gettop(l);
    for (int i = 0; i < 5; ++i) {
      CAPTURE(i + 1);
      lua_rawgeti(l, sections, i + 1);
      const int  entry    = lua_gettop(l);
      const bool is_table = lua_istable(l, entry) != 0;
      CHECK(is_table);
      if (is_table) {
        // mandatory is the Lua boolean true; the string "true", false and a missing key all fail.
        lua_getfield(l, entry, "mandatory");
        CHECK(lua_type(l, -1) == LUA_TBOOLEAN);
        CHECK(lua_toboolean(l, -1) != 0);
        lua_settop(l, entry);
        // text is a string beginning with the heading marker.
        const std::string text = ra01_cc01_field_text(l, entry, "text");
        CHECK(text.starts_with("## "));
        // Exactly three keys, and each key is one of id, text, mandatory; a fourth key or a misspelt key fails.
        std::size_t keys  = 0;
        bool        named = true;
        lua_pushnil(l);
        while (lua_next(l, entry) != 0) {
          const std::string key = ra01_cc01_text(l, -2);
          named                 = named && (key == "id" || key == "text" || key == "mandatory");
          ++keys;
          lua_pop(l, 1);
        }
        CHECK(named);
        CHECK(keys == 3);
      }
      lua_settop(l, sections);
    }
  }
  SECTION("source pins") {
    // Supplement only (D16): the consumer and the literal wrapper and helper shapes, read from the squashed source.
    const auto squashed =
        c01_squashed_lua(fs::path(__FILE__).parent_path() / ".." / ".." / "stdlib" / "planar" / "orchestrator.lua");
    REQUIRE_FALSE(squashed.empty());
    const std::string_view text = squashed;
    CHECK(c01_occurrences(text, "localfunctionproject(c)") == 1);
    const auto project_begin = squashed.find("localfunctionproject(c)");
    REQUIRE(project_begin != std::string::npos);
    CHECK(c01_occurrences(text, "orchestrator_command_impl=function") == 1);
    const auto project_end = squashed.find("orchestrator_command_impl=function");
    REQUIRE(project_end != std::string::npos);
    REQUIRE(project_begin < project_end);
    const std::string_view project_span = text.substr(project_begin, project_end - project_begin);
    const std::string_view tail_span    = text.substr(project_begin);
    // (i) one versioned wrapper return inside project(), and no bare return of the sections array.
    const std::string_view wrapper_return = "return{version=1,sections=sections}";
    CAPTURE(wrapper_return);
    CHECK(c01_occurrences(project_span, wrapper_return) == 1);
    CHECK(c01_occurrences(text, wrapper_return) == 1);
    CHECK(c01_occurrences(project_span, "returnsectionsend") == 0);
    // (ii) the section helper with mandatory=true, inside project().
    const std::string_view helper =
        R"lua(localfunctionsection(id,heading,body)sections[#sections+1]={id=id,text="##"..heading.."\n"..body,mandatory=true}end)lua";
    CAPTURE(helper);
    CHECK(c01_occurrences(text, helper) == 1);
    const auto helper_at = squashed.find(helper);
    CHECK((helper_at != std::string::npos && helper_at >= project_begin && helper_at < project_end));
    // (iii) mandatory appears once in the tail, once as mandatory= in the file, and never as mandatory=false.
    CHECK(c01_occurrences(tail_span, "mandatory") == 1);
    CHECK(c01_occurrences(text, "mandatory=") == 1);
    CHECK(c01_occurrences(text, "mandatory=false") == 0);
    // (iv) five section calls in the contract order, each once.
    const std::array<std::string_view, 5> calls{"section(\"orchestrator_authority\"", "section(\"orchestrator_graph\"",
                                                "section(\"orchestrator_roles\"", "section(\"orchestrator_prescriptions\"",
                                                "section(\"orchestrator_actions\""};
    std::size_t                           previous = 0;
    for (std::size_t k = 0; k < calls.size(); ++k) {
      CAPTURE(calls[k]);
      CHECK(c01_occurrences(text, calls[k]) == 1);
      const auto at = squashed.find(calls[k]);
      CHECK((at != std::string::npos && (k == 0 || at > previous)));
      if (at != std::string::npos)
        previous = at;
    }
    CHECK(c01_occurrences(text, "section(\"orchestrator_") == 5);
    // (v) the consumer iterates the wrapper's sections and binds project(c) once.
    CHECK(c01_occurrences(text, "ipairs(result.sections)") == 1);
    CHECK(c01_occurrences(text, "ipairs(project(c))") == 0);
    CHECK(c01_occurrences(text, "localresult=project(c)") == 1);
  }
}

// Mutant (I6): orchestrator.lua changes the title, the heading marker or text, the authority sentences, or the section
// join (for example LF to a double LF), so the markdown bytes differ from the RA-3 golden prefix and separators.
TEST_CASE("RA01-C-C01 orchestrator markdown bytes keep the golden title authority and joins", "[authoring][ra01][orchestrator]") {
  or_scratch s;
  s.put("split.lua", pipeline_split);
  const auto        doc = parse_prompt(orchestrate(s, "split"));
  const std::string md  = doc.markdown;
  CHECK(md.starts_with("# Planar orchestrator prompt\n\n## Orchestrator authority\nThis prompt describes the graph of job "));
  CHECK(md.contains("\nThe driver schedules sessions; Planar does not. Planar prints this prompt and states no execution state.\n"
                    "Use only the actions marked as available below; actions marked as future are unavailable.\n\n"
                    "## Orchestrator graph\n- shape: "));
  CHECK(md.contains("\n\n## Orchestrator roles\n- "));
  CHECK(md.contains("\n\n## Orchestrator prescriptions\n- "));
  CHECK(md.contains("\n\n## Orchestrator actions\n- "));
  // Exactly one blank line between blocks: no run of three LFs anywhere.
  CHECK_FALSE(md.contains("\n\n\n"));
  CHECK(md.ends_with("\n"));
}
