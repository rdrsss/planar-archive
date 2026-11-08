// Dispatch projection (execution contract P2-2, P2-3, EX-5) through the fixed exports work_exec and render_dispatch,
// driven by the quota fixture's real interpretation and handcrafted dispatch_context_v1 values.
#include <catch2/catch_test_macros.hpp>
#include <unistd.h>
import std;
import planar.engine.runtime;
import planar.engine.json_codec;
import planar.engine.capture;
import planar.engine.hash;
using namespace planar::engine;
namespace {
value_ptr str(std::string bytes) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = std::move(bytes);
  return v;
}
value_ptr rec(std::map<std::string, value_ptr> fields = {}) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::record;
  v->fields = std::move(fields);
  return v;
}
value_ptr arr(std::vector<value_ptr> items = {}) {
  auto v   = std::make_shared<value>();
  v->kind  = value_kind::list;
  v->items = std::move(items);
  return v;
}
value_ptr num(std::int64_t n) {
  auto v     = std::make_shared<value>();
  v->kind    = value_kind::integer;
  v->integer = n;
  return v;
}
value_ptr with(value_ptr original, std::string key, value_ptr replacement) {
  auto v                    = std::make_shared<value>(*original);
  v->fields[std::move(key)] = std::move(replacement);
  return v;
}
const value_ptr& at(const value_ptr& v, const std::string& key) {
  REQUIRE(v);
  INFO(key);
  REQUIRE(v->fields.contains(key));
  return v->fields.at(key);
}
std::string text(const value_ptr& v, const std::string& key) {
  return at(v, key)->string;
}
std::set<std::string> keys(const value_ptr& v) {
  std::set<std::string> out;
  for (const auto& [key, _] : v->fields)
    out.insert(key);
  return out;
}
std::string read_bytes(const std::filesystem::path& file) {
  std::ifstream stream(file, std::ios::binary);
  REQUIRE(stream.good());
  return {std::istreambuf_iterator<char>(stream), {}};
}
value_ptr success(std::string_view entry, value_ptr input) {
  auto result = run_trusted(entry, input);
  INFO(entry);
  INFO((result ? "native success" : result.error().message));
  REQUIRE(result);
  if ((*result)->fields.contains("error")) {
    auto e = (*result)->fields.at("error");
    INFO(e->fields.at("field")->string);
    INFO(e->fields.at("message")->string);
    FAIL("Unexpected trusted refusal");
  }
  return (*result)->fields.at("value");
}
value_ptr refusal(std::string_view entry, value_ptr input) {
  auto result = run_trusted(entry, input);
  REQUIRE(result);
  REQUIRE((*result)->fields.contains("error"));
  return (*result)->fields.at("error");
}
std::string digest(std::string_view domain, const value_ptr& v) {
  auto d = hash_value(domain, v);
  REQUIRE(d);
  return *d;
}
struct quota {
  std::filesystem::path root;
  value_ptr             interpretation;
  quota() {
    std::string pattern = (std::filesystem::temp_directory_path() / "planar-dispatch-XXXXXX").string();
    REQUIRE(mkdtemp(pattern.data()));
    root = std::filesystem::canonical(pattern);
    std::filesystem::copy(std::filesystem::path(PLANAR_SCHEMA_FIXTURE) / "workspace", root / "workspace",
                          std::filesystem::copy_options::recursive);
    capture_session capture;
    REQUIRE(capture.add_root("workspace", (root / "workspace").string()));
    auto workspace = capture.load("workspace.lua");
    REQUIRE(workspace);
    auto route = success("route_workspace", rec({{"workspace", workspace->root}, {"file", str("workspace.lua")}}));
    auto spec  = capture.load(route->fields.at("spec")->string);
    REQUIRE(spec);
    std::vector<value_ptr> jobs;
    for (auto& j : route->fields.at("jobs")->items) {
      auto file = j->fields.at("file");
      auto job  = capture.load(file->string);
      REQUIRE(job);
      jobs.push_back(rec({{"file", file}, {"value", job->root}}));
    }
    auto snapshot = capture.finish();
    REQUIRE(snapshot);
    std::vector<value_ptr> files;
    for (const auto& f : **snapshot)
      files.push_back(rec({{"path", str(f.path)}, {"bytes", str(f.bytes)}}));
    auto bundle = rec(
        {{"workspace", rec({{"file", str("workspace.lua")}, {"value", workspace->root}})},
         {"spec", rec({{"file", route->fields.at("spec")}, {"value", spec->root}})},
         {"jobs", arr(jobs)},
         {"registry",
          rec({{"file", str("input.json")},
               {"bytes", str(read_bytes(std::filesystem::path(PLANAR_SCHEMA_FIXTURE) / "registry/quota-tools.json"))}})},
         {"files", arr(files)}});
    interpretation = success("interpret_bundle", bundle);
  }
  ~quota() {
    std::filesystem::remove_all(root);
  }
  value_ptr work(const std::string& job, const std::string& role, value_ptr interp = nullptr,
                 value_ptr publications = nullptr) const {
    std::map<std::string, value_ptr> input{
        {"interpretation", interp ? interp : interpretation}, {"job", str(job)}, {"role", str(role)}};
    if (publications)
      input["publications"] = publications;
    return success("work_exec", rec(std::move(input)));
  }
  // dispatch_context_v1 (DD3) for a claim of the role; callers vary run, generation and budget.
  value_ptr context(const std::string& job, const std::string& role, value_ptr run = nullptr,
                    const std::string& generation = "2") const {
    if (!run)
      run = rec({{"request", str("req-1")}, {"kind", str("claim")}});
    return rec({{"revision", rec({{"id", str("3")}, {"source", str("11")}, {"interpretation", str("12")}})},
                {"job", str(job)},
                {"role", str(role)},
                {"generation", str(generation)},
                {"run", run},
                {"snapshot_role", str("none")},
                {"interpretation", interpretation},
                {"publications", arr()},
                {"repair", str("0")}});
  }
};
// Interpretation with an extra start-phase facet dependency of command on library's facet.
value_ptr with_start_facet(const value_ptr& interpretation) {
  auto semantic = at(interpretation, "semantic");
  auto deps     = at(semantic, "dependencies")->items;
  deps.push_back(rec({{"id", str("early")},
                      {"producer", str("library")},
                      {"consumer", str("command")},
                      {"source", str("facet")},
                      {"ref", str("library_files")},
                      {"phase", str("start")},
                      {"quality", str("available")}}));
  return with(interpretation, "semantic", with(semantic, "dependencies", arr(deps)));
}
value_ptr with_prescription_text(const value_ptr& interpretation, const std::string& id, const std::string& replacement) {
  auto bundle = at(interpretation, "bundle");
  auto spec   = at(bundle, "spec");
  auto items  = at(spec, "prescriptions")->items;
  for (auto& item : items)
    if (text(item, "id") == id)
      item = with(item, "text", str(replacement));
  return with(interpretation, "bundle", with(bundle, "spec", with(spec, "prescriptions", arr(items))));
}
std::vector<std::string> section_ids(const value_ptr& manifest) {
  std::vector<std::string> out;
  for (const auto& s : at(manifest, "sections")->items)
    out.push_back(text(s, "id"));
  return out;
}
const std::string worker_run =
    "## run\n\n"
    "run request: req-1\n"
    "- refresh: Re-render this work and acknowledge the current packet.\n"
    "- checkpoint: Record observations of progress on this run.\n"
    "- check: Run the checks of the job against the output of this role.\n"
    "- release: End this run as released or interrupted.\n"
    "Actions not shipped in this build are unavailable.\n";
} // namespace

TEST_CASE("dispatch work_exec_v1 has the 24 fields, the declared base and repair 0", "[dispatch]") {
  quota q;
  auto  result = q.work("split", "cli_worker");
  auto  W      = at(result, "work");
  CHECK(keys(W) == std::set<std::string>{"versions", "workspace", "spec", "job", "role", "root", "base", "goal",
                                         "units", "milestones", "constraints", "prescriptions", "contracts",
                                         "dependencies", "facets", "pipeline_predecessors", "questions", "ownership",
                                         "effective_checks", "checks", "toolchains", "shape", "start_facets",
                                         "repair"});
  std::vector<std::string> versions;
  for (const auto& v : at(W, "versions")->items)
    versions.push_back(v->string);
  CHECK(versions == std::vector<std::string>{"declarations_v1", "selection_v1", "dependencies_v1", "exact_bytes_v1",
                                             "canonical_v1", "xxh3_128_v1", "work_exec_v1"});
  auto base = at(W, "base");
  CHECK(keys(base) == std::set<std::string>{"status", "root", "label"});
  CHECK(text(base, "status") == "declared");
  CHECK(text(base, "root") == "source");
  CHECK(text(base, "label") == "initial");
  CHECK(text(W, "repair") == "0");
  CHECK(at(W, "start_facets")->items.empty());
  CHECK(text(result, "digest") == digest("work_exec_v1", W));
  // role_structure_v1 of the role and its digest.
  auto structure = at(result, "structure");
  CHECK(keys(structure) == std::set<std::string>{"job", "role", "kind", "work", "targets", "ownership", "shape"});
  CHECK(text(structure, "kind") == "worker");
  REQUIRE(at(structure, "work")->items.size() == 1);
  CHECK(at(structure, "work")->items[0]->string == "command");
  CHECK(at(structure, "targets")->items.empty());
  CHECK(text(structure, "shape") == "fanout_review");
  CHECK(text(result, "structure_digest") == digest("role_structure_v1", structure));
  auto reviewer = at(q.work("split", "reviewer"), "structure");
  CHECK(at(reviewer, "work")->items.empty());
  CHECK(at(reviewer, "targets")->items.size() == 2);
}

TEST_CASE("dispatch work_exec_v1 start facets are unavailable without a publication", "[dispatch]") {
  quota q;
  auto  interp = with_start_facet(q.interpretation);
  auto  W      = at(q.work("split", "cli_worker", interp), "work");
  REQUIRE(at(W, "start_facets")->items.size() == 1);
  auto facet = at(W, "start_facets")->items[0];
  CHECK(keys(facet) == std::set<std::string>{"dependency", "unavailable"});
  CHECK(text(facet, "dependency") == "early");
  CHECK(at(facet, "unavailable")->boolean);
  auto published =
      arr({rec({{"work", str("library")}, {"facet", str("library_files")}, {"publication", str("5")},
                {"semantic", str("xxh3_128:00112233445566778899aabbccddeeff")}})});
  auto with_publication = at(q.work("split", "cli_worker", interp, published), "work");
  auto entry            = at(with_publication, "start_facets")->items[0];
  CHECK(text(entry, "semantic") == "xxh3_128:00112233445566778899aabbccddeeff");
  CHECK_FALSE(entry->fields.contains("unavailable"));
  // The library worker produces the facet: its W carries no start facet.
  CHECK(at(at(q.work("split", "library_worker", interp), "work"), "start_facets")->items.empty());
}

TEST_CASE("dispatch work_exec_v1 moves only for applicable inputs", "[dispatch]") {
  quota q;
  auto  changed     = with_prescription_text(q.interpretation, "command_rule", "A different command rule.");
  auto  library     = text(q.work("split", "library_worker"), "digest");
  auto  command     = text(q.work("split", "cli_worker"), "digest");
  CHECK(text(q.work("split", "library_worker", changed), "digest") == library);
  CHECK(text(q.work("split", "cli_worker", changed), "digest") != command);
}

TEST_CASE("dispatch packet has mode dispatch, the authority template and the run section second", "[dispatch]") {
  quota q;
  auto  result   = success("render_dispatch", q.context("split", "cli_worker"));
  auto  packet   = at(result, "packet");
  auto  manifest = at(result, "manifest");
  CHECK(text(result, "kind") == "dispatch");
  CHECK(text(packet, "mode") == "dispatch");
  CHECK(keys(packet) == std::set<std::string>{"version", "mode", "workspace", "spec", "job", "role", "kind", "work",
                                              "markdown"});
  auto markdown = text(packet, "markdown");
  const std::string authority =
      "## authority\n\nThis packet was dispatched by Planar for run request req-1, generation 2 of role cli_worker, "
      "under revision 3 of job split. Checks have not run in this dispatch. Upstream facet content comes from recorded "
      "publications, and prerequisite status comes from ledger qualities, not from a checkout.\n";
  REQUIRE(markdown.starts_with(authority + "\n" + worker_run + "\n## goal\n"));
  CHECK(markdown.ends_with("\n"));
  CHECK_FALSE(markdown.ends_with("\n\n"));
  auto ids = section_ids(manifest);
  REQUIRE(ids.size() > 2);
  CHECK(ids[0] == "authority");
  CHECK(ids[1] == "run");
  auto run = at(manifest, "sections")->items[1];
  CHECK(at(run, "mandatory")->boolean);
  CHECK(at(run, "included")->boolean);
  CHECK(at(run, "bytes")->integer == static_cast<std::int64_t>(worker_run.size() + 1));
  // The packet never carries a token or secret.
  CHECK(markdown.find("token") == std::string::npos);
}

TEST_CASE("dispatch reviewer run section lists verdict in E1 order", "[dispatch]") {
  quota q;
  auto  markdown = text(at(success("render_dispatch", q.context("split", "reviewer")), "packet"), "markdown");
  const std::string reviewer_run = "## run\n\n"
                                   "run request: req-1\n"
                                   "- refresh: Re-render this work and acknowledge the current packet.\n"
                                   "- checkpoint: Record observations of progress on this run.\n"
                                   "- verdict: Record the verdict of the reviewer for this run.\n"
                                   "- release: End this run as released or interrupted.\n"
                                   "Actions not shipped in this build are unavailable.\n";
  CHECK(markdown.find("\n" + reviewer_run + "\n") != std::string::npos);
}

TEST_CASE("dispatch manifest has exactly the fourteen keys for dispatch, takeover and preview", "[dispatch]") {
  quota                       q;
  const std::set<std::string> fourteen{"version",  "revision", "job",      "role",     "generation",
                                       "run",      "versions", "work",     "render",   "sections",
                                       "omitted",  "prerequisites", "registry", "inputs"};
  auto claim = success("render_dispatch", q.context("split", "cli_worker"));
  auto M     = at(claim, "manifest");
  CHECK(keys(M) == fourteen);
  CHECK(at(M, "version")->integer == 1);
  CHECK(text(M, "revision") == "3");
  CHECK(text(M, "generation") == "2");
  CHECK(keys(at(M, "run")) == std::set<std::string>{"request", "kind"});
  CHECK(text(at(M, "run"), "kind") == "claim");
  std::vector<std::string> versions;
  for (const auto& v : at(M, "versions")->items)
    versions.push_back(v->string);
  REQUIRE(versions.size() == 8);
  CHECK(versions.back() == "dispatch_v1");
  CHECK(versions[6] == "work_exec_v1");
  CHECK(text(at(M, "work"), "digest") == digest("work_exec_v1", at(claim, "work")));
  auto markdown = text(at(claim, "packet"), "markdown");
  auto render   = at(M, "render");
  CHECK(at(render, "bytes")->integer == static_cast<std::int64_t>(markdown.size()));
  auto fragment = fragment_digest(markdown);
  REQUIRE(fragment);
  CHECK(text(render, "digest") == *fragment);
  CHECK(at(render, "budget_bytes")->integer == 131072);
  CHECK(text(at(M, "registry"), "authority") == "driver_selected_trusted_configuration");
  auto inputs = at(M, "inputs");
  CHECK(keys(inputs) == std::set<std::string>{"source", "interpretation", "publications", "repair"});
  CHECK(text(inputs, "source") == "11");
  CHECK(text(inputs, "interpretation") == "12");
  CHECK(text(inputs, "repair") == "0");
  CHECK(at(inputs, "publications")->items.empty());
  // Prerequisite statuses come from ledger qualities: a contract is available, a facet needs a publication.
  for (const auto& p : at(M, "prerequisites")->items) {
    if (text(p, "id") == "develop")
      CHECK(text(p, "status") == "available");
    if (text(p, "id") == "integrate")
      CHECK(text(p, "status") == "unavailable");
    CHECK(text(p, "status") != "not_captured");
  }

  auto takeover = success("render_dispatch",
                          q.context("split", "cli_worker",
                                    rec({{"request", str("req-2")}, {"kind", str("takeover")}, {"previous", str("4")}})));
  auto T        = at(takeover, "manifest");
  CHECK(keys(T) == fourteen);
  CHECK(keys(at(T, "run")) == std::set<std::string>{"request", "kind", "previous"});
  CHECK(text(at(T, "run"), "previous") == "4");

  auto preview = success("render_dispatch", q.context("split", "cli_worker", rec({{"kind", str("preview")}})));
  auto P       = at(preview, "manifest");
  CHECK(keys(P) == fourteen);
  CHECK(keys(at(P, "run")) == std::set<std::string>{"kind"});
  auto preview_markdown = text(at(preview, "packet"), "markdown");
  CHECK(preview_markdown.starts_with(
      "## authority\n\nThis packet is a preview of a dispatch for generation 2 of role cli_worker, under revision 3 of "
      "job split. No run exists and nothing was retained. Checks have not run in this dispatch."));
  CHECK(preview_markdown.find("\n## run\n\nrun request: (preview)\n- refresh: ") != std::string::npos);
  CHECK(text(at(preview, "packet"), "mode") == "dispatch");
  // Same W across claim, takeover and preview; render bytes differ.
  CHECK(text(at(P, "work"), "digest") == text(at(M, "work"), "digest"));
  CHECK(text(at(P, "render"), "digest") != text(at(M, "render"), "digest"));
}

TEST_CASE("dispatch W is stable across generations while the packet changes", "[dispatch]") {
  quota q;
  auto  first  = success("render_dispatch", q.context("split", "cli_worker", nullptr, "1"));
  auto  second = success("render_dispatch",
                         q.context("split", "cli_worker",
                                   rec({{"request", str("req-1")}, {"kind", str("takeover")}, {"previous", str("7")}}), "2"));
  CHECK(text(at(at(first, "manifest"), "work"), "digest") == text(at(at(second, "manifest"), "work"), "digest"));
  CHECK(text(at(at(first, "manifest"), "render"), "digest") != text(at(at(second, "manifest"), "render"), "digest"));
}

TEST_CASE("dispatch budget never omits the run section", "[dispatch]") {
  quota q;
  auto  full  = success("render_dispatch", q.context("split", "cli_worker"));
  auto  bytes = at(at(at(full, "manifest"), "render"), "bytes")->integer;
  auto  tight = with(q.context("split", "cli_worker"), "budget_bytes", num(bytes - 1));
  auto  cut   = success("render_dispatch", tight);
  auto  M     = at(cut, "manifest");
  CHECK_FALSE(at(M, "omitted")->items.empty());
  bool run_included = false;
  for (const auto& s : at(M, "sections")->items)
    if (text(s, "id") == "run")
      run_included = at(s, "included")->boolean;
  CHECK(run_included);
  CHECK(at(at(M, "render"), "bytes")->integer <= bytes - 1);
  // A budget below the mandatory bytes refuses rather than dropping run.
  auto refused = refusal("render_dispatch", with(q.context("split", "cli_worker"), "budget_bytes", num(64)));
  CHECK(text(refused, "code") == "budget_exceeded");
}

TEST_CASE("dispatch is byte-identical across fresh VMs", "[dispatch]") {
  quota q;
  auto  first  = success("render_dispatch", q.context("combined", "builder"));
  auto  second = success("render_dispatch", q.context("combined", "builder"));
  auto  a      = codec_encode(first);
  auto  b      = codec_encode(second);
  REQUIRE(a);
  REQUIRE(b);
  CHECK(*a == *b);
}

TEST_CASE("dispatch refuses an unknown job or role and a malformed context", "[dispatch]") {
  quota q;
  auto  job = refusal("render_dispatch", q.context("absent", "cli_worker"));
  CHECK(text(job, "code") == "reference");
  CHECK(text(job, "message") == "Unknown selected job");
  CHECK(text(job, "field") == "selection.job");
  auto role = refusal("render_dispatch", q.context("split", "builder"));
  CHECK(text(role, "message") == "Unknown selected role");
  CHECK(text(role, "field") == "selection.role");
  auto takeover_without_previous =
      refusal("render_dispatch", q.context("split", "cli_worker", rec({{"request", str("r")}, {"kind", str("takeover")}})));
  CHECK(text(takeover_without_previous, "code") == "schema");
  auto work = refusal("work_exec", rec({{"interpretation", q.interpretation}, {"job", str("split")}, {"role", str("nobody")}}));
  CHECK(text(work, "field") == "selection.role");
}
