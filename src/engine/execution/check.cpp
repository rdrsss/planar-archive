module;
#include <errno.h>
#include <fcntl.h>
#include <glaze/glaze.hpp>
#include <sys/stat.h>
#include <unistd.h>
module planar.engine.execution.check;
import std;
import planar.engine.hash;
import planar.engine.json_codec;
import planar.engine.process.resolve;
namespace planar::engine::execution {
namespace {
value_ptr str(std::string s) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = std::move(s);
  return v;
}
value_ptr integer(std::int64_t n) {
  auto v     = std::make_shared<value>();
  v->kind    = value_kind::integer;
  v->integer = n;
  return v;
}
value_ptr object(std::map<std::string, value_ptr> f) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::record;
  v->fields = std::move(f);
  return v;
}
value_ptr array(std::vector<value_ptr> a = {}) {
  auto v   = std::make_shared<value>();
  v->kind  = value_kind::list;
  v->items = std::move(a);
  return v;
}
value_ptr get(const value_ptr& v, std::string_view k) {
  if (!v)
    return {};
  auto i = v->fields.find(std::string(k));
  return i == v->fields.end() ? value_ptr{} : i->second;
}
std::string string(const value_ptr& v, std::string_view k, std::string fallback = {}) {
  auto p = get(v, k);
  return p && p->kind == value_kind::string ? p->string : fallback;
}
std::uint64_t number(const value_ptr& v, std::string_view k) {
  auto p = get(v, k);
  return p && p->kind == value_kind::integer && p->integer >= 0 ? static_cast<std::uint64_t>(p->integer) : 0;
}
std::vector<value_ptr> list(const value_ptr& v, std::string_view k) {
  auto p = get(v, k);
  return p && p->kind == value_kind::list ? p->items : std::vector<value_ptr>{};
}
std::vector<std::string> strings(const value_ptr& v, std::string_view k) {
  std::vector<std::string> out;
  for (const auto& x : list(v, k))
    out.push_back(x->string);
  return out;
}
bool contains(std::string_view b, std::string_view p) {
  return b == p || (b.ends_with('/') && p.starts_with(b));
}
bool covered(const value_ptr& p, std::string_view k, std::string_view path) {
  for (const auto& b : strings(p, k))
    if (contains(b, path))
      return true;
  return false;
}
auto fail(std::string code, std::string message) {
  return std::unexpected(diagnostic{std::move(code), std::move(message)});
}
std::string hashed(std::string_view domain, const value_ptr& v) {
  auto h = hash_value(domain, v);
  if (!h)
    throw h.error();
  return *h;
}
value_ptr entries(const capture::tree_digest& tree, std::string_view root, const value_ptr& paths = {}) {
  std::vector<value_ptr> out;
  for (const auto& e : tree.entries) {
    if (paths && !std::ranges::any_of(paths->items, [&](const auto& p) { return contains(p->string, e.path); }))
      continue;
    out.push_back(object({{"root", str(std::string(root))},
                          {"path", str(e.path)},
                          {"kind", str("file")},
                          {"mode", str(e.mode)},
                          {"size", integer(e.size)},
                          {"content", str(capture::digest_text(e.digest))}}));
  }
  return array(std::move(out));
}
std::optional<std::string> read_result(int dir) {
  int fd = ::openat(dir, "result.json", O_RDONLY | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK);
  if (fd < 0)
    return {};
  struct stat st{};
  if (::fstat(fd, &st) || !S_ISREG(st.st_mode) || st.st_size < 0 || st.st_size > 65536) {
    ::close(fd);
    return {};
  }
  std::string            out;
  std::array<char, 4096> buf;
  while (true) {
    auto n = ::read(fd, buf.data(), buf.size());
    if (n < 0 && errno == EINTR)
      continue;
    if (n < 0) {
      ::close(fd);
      return {};
    }
    if (n == 0)
      break;
    out.append(buf.data(), n);
    if (out.size() > 65536) {
      ::close(fd);
      return {};
    }
  }
  ::close(fd);
  return out;
}
std::optional<std::uint64_t> result_count(std::string_view bytes, bool successful) {
  if (bytes.size() > 65536 || glz::validate_json(bytes))
    return {};
  std::vector<std::pair<std::string, std::uint64_t>> fields;
  if (glz::read_json(fields, bytes) || fields.size() != 4)
    return {};
  std::map<std::string, std::uint64_t> m;
  for (auto& [k, v] : fields)
    if (v > 9007199254740991ULL || !m.emplace(k, v).second)
      return {};
  if (!m.contains("version") || !m.contains("matched") || !m.contains("passed") || !m.contains("failed") || m["version"] != 1 ||
      m["passed"] + m["failed"] > m["matched"] || (successful && m["failed"] != 0))
    return {};
  return m["matched"];
}
} // namespace
struct prepared_check::impl {
  int                                root = -1, base = -1, scratch = -1;
  std::filesystem::path              scratch_path;
  value_ptr                          plan, observation;
  capture::tree_digest               original, current;
  capture::tree_digest_spec          spec;
  std::map<std::string, std::string> outputs;
  ~impl() {
    if (root >= 0)
      ::close(root);
    if (base >= 0)
      ::close(base);
    if (scratch >= 0)
      ::close(scratch);
  }
};
prepared_check::prepared_check()                                     = default;
prepared_check::~prepared_check()                                    = default;
prepared_check::prepared_check(prepared_check&&) noexcept            = default;
prepared_check& prepared_check::operator=(prepared_check&&) noexcept = default;
check_evidence  classify(const process::run_result& r, std::string_view selection, std::optional<std::string_view> json) {
  check_evidence e;
  e.outcome          = "fail";
  e.output           = r.output;
  e.output_bytes     = r.output_bytes;
  e.output_truncated = r.truncated;
  e.duration_ms      = r.duration_ms;
  if (r.how == process::termination::exited)
    e.exit_status = r.exit_code;
  else if (r.how != process::termination::not_spawned)
    e.exit_status = -r.signal;
  if (selection == "nonempty" && json)
    e.matched = result_count(*json, r.how == process::termination::exited && r.exit_code == 0);
  switch (r.how) {
  case process::termination::not_spawned:
    e.reason = "missing_executable";
    return e;
  case process::termination::timed_out:
    e.reason = "timeout";
    return e;
  case process::termination::output_limit:
    e.reason = "output_limit";
    return e;
  case process::termination::signaled:
    e.reason = "exit_status";
    return e;
  default:
    break;
  }
  if (r.exit_code != 0)
    e.reason = "exit_status";
  else if (selection == "nonempty" && !e.matched)
    e.reason = "malformed_result";
  else if (selection == "nonempty" && *e.matched == 0)
    e.reason = "zero_selected";
  else {
    e.outcome = "pass";
    e.reason  = "passed";
  }
  return e;
}
std::pair<bool, value_ptr> boundary_gate(const value_ptr& plan, const capture::tree_digest& base,
                                         const capture::tree_digest& current) {
  bool                   pass = true;
  std::vector<value_ptr> changes;
  for (const auto& c : capture::diff_trees(base, current)) {
    std::string cls = "outside";
    if (covered(plan, "frozen", c.path))
      cls = "frozen";
    else if (covered(plan, "owned", c.path))
      cls = "own";
    else
      for (const auto& p : list(plan, "producers"))
        if (!string(p, "completion_output").empty() && string(p, "completion_output") == string(p, "output") &&
            covered(p, "paths", c.path)) {
          cls = "producer";
          break;
        }
    if (cls == "outside" || cls == "frozen")
      pass = false;
    const char* kind = c.change == capture::tree_change::kind::added     ? "added"
                       : c.change == capture::tree_change::kind::deleted ? "deleted"
                       : c.change == capture::tree_change::kind::mode    ? "mode"
                                                                         : "modified";
    changes.push_back(object({{"path", str(c.path)}, {"change", str(kind)}, {"class", str(cls)}}));
  }
  return {pass, object({{"version", integer(1)}, {"changes", array(std::move(changes))}})};
}
std::expected<prepared_check, diagnostic> prepare_check(const value_ptr& plan, const std::filesystem::path& root,
                                                        const std::filesystem::path& base, const std::filesystem::path& scratch) {
  try {
    if (!plan || plan->kind != value_kind::record)
      return fail("internal", "Invalid embedded command protocol");
    auto checks = list(plan, "checks"), units = list(plan, "units");
    if (checks.size() + units.size() > 1024)
      return std::unexpected(diagnostic{"resource_limit", "Execution resource limit exceeded", "check_evidence_rows", 1024});
    for (const auto& c : checks) {
      auto env = get(c, "environment");
      if (env)
        for (const auto& [k, v] : env->fields)
          if (k.starts_with("PLANAR_"))
            return fail("reference", "Registry environment uses a reserved name");
      if (strings(c, "command").empty() || number(c, "timeout_ms") == 0 || number(c, "timeout_ms") > 3600000 ||
          number(c, "max_output_bytes") == 0 || number(c, "max_output_bytes") > 16777216)
        return fail("internal", "Invalid embedded command protocol");
    }
    std::error_code ec;
    auto            r = std::filesystem::canonical(root, ec);
    if (ec)
      return fail("io", "Cannot resolve explicit directory path");
    auto b = std::filesystem::canonical(base, ec);
    if (ec)
      return fail("io", "Cannot resolve explicit directory path");
    auto s = std::filesystem::canonical(scratch, ec);
    if (ec)
      return fail("io", "Cannot resolve explicit directory path");
    auto overlap = [](const auto& a, const auto& b) {
      auto x = a.string(), y = b.string();
      return x == y || x.starts_with(y + "/") || y.starts_with(x + "/");
    };
    if (overlap(r, s) || overlap(b, s))
      return fail("path_denied", "Scratch directory overlaps a checked root");
    prepared_check p;
    p.state        = std::make_unique<prepared_check::impl>();
    auto& x        = *p.state;
    x.plan         = plan;
    x.scratch_path = s;
    x.root         = ::open(root.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    x.base         = ::open(base.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    x.scratch      = ::open(scratch.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (x.root < 0 || x.base < 0 || x.scratch < 0)
      return fail("path_denied", "Cannot open explicit directory");
    x.spec.excluded = strings(plan, "excluded");
    for (const auto& f : list(plan, "facets"))
      for (const auto& path : strings(f, "paths"))
        x.spec.retain_paths.push_back(path);
    for (const auto& c : list(plan, "contracts")) {
      auto path = string(c, "path");
      if (path.empty())
        return fail("revision_conflict", "Ledger revision does not match the expected revision");
      x.spec.retain_paths.push_back(path);
    }
    auto bs = x.spec;
    bs.retain_paths.clear();
    auto bt = capture::capture_tree_digest(x.base, bs);
    if (!bt)
      return std::unexpected(bt.error());
    x.original = std::move(*bt);
    auto rt    = capture::capture_tree_digest(x.root, x.spec);
    if (!rt)
      return std::unexpected(rt.error());
    x.current = std::move(*rt);
    for (const auto& f : list(plan, "facets")) {
      std::uint64_t total = 0;
      for (const auto& [path, bytes] : x.current.retained)
        if (covered(f, "paths", path)) {
          total += bytes.size();
          if (total > 4194304)
            return std::unexpected(diagnostic{"resource_limit", "Execution resource limit exceeded", "facet_bytes", 4194304});
        }
    }
    for (const auto& c : list(plan, "contracts")) {
      auto i        = x.current.retained.find(string(c, "path"));
      auto expected = get(c, "bytes");
      if (i == x.current.retained.end() || !expected || i->second != expected->string)
        return fail("revision_conflict", "Ledger revision does not match the expected revision");
    }
    auto root_id  = string(plan, "root");
    auto excluded = get(plan, "excluded");
    if (!excluded)
      excluded = array();
    auto base_id = hashed("base_v1", object({{"root", str(root_id)},
                                             {"label", str(string(plan, "base"))},
                                             {"entries", entries(x.original, root_id)},
                                             {"excluded", excluded}}));
    auto obs     = hashed(
        "observation_v1",
        object(
            {{"root", str(root_id)}, {"base", str(base_id)}, {"entries", entries(x.current, root_id)}, {"excluded", excluded}}));
    auto all_units = units;
    auto producers = list(plan, "producers");
    all_units.insert(all_units.end(), producers.begin(), producers.end());
    std::map<std::string, value_ptr> unit_map;
    for (const auto& u : all_units) {
      auto work  = string(u, "work");
      auto paths = get(u, "paths");
      if (work.empty() || !paths)
        return fail("internal", "Invalid embedded command protocol");
      auto digest     = hashed("unit_output_v1", object({{"work", str(work)},
                                                         {"root", str(root_id)},
                                                         {"base", str(base_id)},
                                                         {"paths", paths},
                                                         {"entries", entries(x.current, root_id, paths)}}));
      x.outputs[work] = digest;
      unit_map[work]  = object({{"work", str(work)}, {"output", str(digest)}});
    }
    for (const auto& u : producers) {
      auto expected = string(u, "completion_output");
      if (!expected.empty() && expected != "none" && x.outputs[string(u, "work")] != expected)
        return fail("observation_target_mismatch", "Wrong output vector");
    }
    std::vector<value_ptr> uv, cv;
    for (const auto& [_, u] : unit_map)
      uv.push_back(u);
    for (const auto& c : list(plan, "contracts"))
      cv.push_back(object({{"id", get(c, "id")},
                           {"path", get(c, "path")},
                           {"digest", str(*file_digest(x.current.retained.at(string(c, "path"))))}}));
    x.observation = object({{"version", integer(1)},
                            {"root", str(root_id)},
                            {"base", str(base_id)},
                            {"observation", str(obs)},
                            {"units", array(std::move(uv))},
                            {"contracts", array(std::move(cv))}});
    return p;
  } catch (const diagnostic& d) {
    return std::unexpected(d);
  }
}
value_ptr check_observation(const prepared_check& p) {
  return p.state ? p.state->observation : value_ptr{};
}
std::expected<check_run_result, diagnostic> run_checks(prepared_check& p, std::uint64_t reservation) {
  if (!p.state || reservation == 0)
    return fail("internal", "Invalid embedded command protocol");
  try {
    auto&            x = *p.state;
    check_run_result result;
    result.observation      = x.observation;
    std::uint64_t remaining = 67108864, ordinal = 0;
    for (const auto& c : list(x.plan, "checks")) {
      auto name = std::format("planar-check.{}.{}", reservation, ++ordinal);
      if (::mkdirat(x.scratch, name.c_str(), 0700))
        return fail("io", "Cannot create check scratch directory");
      int dir = ::openat(x.scratch, name.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
      if (dir < 0)
        return fail("io", "Cannot create check scratch directory");
      process::spawn_spec spec;
      spec.cwd_fd = x.root;
      spec.argv   = strings(c, "command");
      auto args   = strings(c, "argv");
      spec.argv.insert(spec.argv.end(), args.begin(), args.end());
      auto env = get(c, "environment");
      if (env)
        for (const auto& [k, v] : env->fields)
          spec.environment.emplace_back(k, v->string);
      spec.environment.emplace_back("PLANAR_CHECK", string(c, "check"));
      spec.environment.emplace_back("PLANAR_WORK", string(c, "work"));
      spec.environment.emplace_back("PLANAR_CHECK_RESULT", (x.scratch_path / name / "result.json").string());
      std::ranges::sort(spec.environment);
      spec.timeout_ms                = number(c, "timeout_ms");
      spec.max_output_bytes          = number(c, "max_output_bytes");
      spec.retain_bytes              = std::min(remaining, spec.max_output_bytes);
      auto                executable = process::resolve_executable(spec.argv.front(), spec.environment);
      process::run_result run{};
      run.how = process::termination::not_spawned;
      if (executable) {
        spec.executable   = *executable;
        spec.argv.front() = *executable;
        auto r            = process::run(spec);
        if (!r) {
          ::close(dir);
          return fail("io", "Cannot execute check process");
        }
        run = std::move(*r);
      }
      auto bytes = read_result(dir);
      auto e     = classify(run, string(c, "selection"), bytes ? std::optional<std::string_view>(*bytes) : std::nullopt);
      remaining -= e.output.size();
      e.work  = string(c, "work");
      e.check = string(c, "check");
      struct stat st{};
      if (::fstatat(dir, "result.json", &st, AT_SYMLINK_NOFOLLOW) == 0 && S_ISREG(st.st_mode) && st.st_uid == ::geteuid())
        ::unlinkat(dir, "result.json", 0);
      ::close(dir);
      ::unlinkat(x.scratch, name.c_str(), AT_REMOVEDIR);
      std::vector<value_ptr> targets;
      auto                   vector = strings(c, "vector");
      if (vector.empty())
        vector.push_back(e.work);
      std::ranges::sort(vector);
      vector.erase(std::unique(vector.begin(), vector.end()), vector.end());
      for (const auto& work : vector) {
        auto i = x.outputs.find(work);
        if (i == x.outputs.end())
          return fail("internal", "Invalid embedded command protocol");
        targets.push_back(object({{"work", str(work)}, {"output", str(i->second)}}));
      }
      e.vector = object({{"version", integer(1)}, {"targets", array(std::move(targets))}});
      std::vector<value_ptr> argv, env_names;
      for (const auto& a : spec.argv)
        argv.push_back(str(a));
      if (env)
        for (const auto& [k, _] : env->fields)
          env_names.push_back(str(k));
      e.definition = object(
          {{"version", integer(1)},
           {"work", str(e.work)},
           {"check", str(e.check)},
           {"gate", str("registry")},
           {"registry", get(x.plan, "registry") ? get(x.plan, "registry") : object({{"id", str("")}, {"revision", str("")}})},
           {"toolchain", get(c, "toolchain")},
           {"command", array(std::move(argv))},
           {"environment", array(std::move(env_names))},
           {"reserved", array({str("PLANAR_CHECK"), str("PLANAR_CHECK_RESULT"), str("PLANAR_WORK")})},
           {"cwd", object({{"root", get(x.plan, "root")}})},
           {"timeout_ms", get(c, "timeout_ms")},
           {"max_output_bytes", get(c, "max_output_bytes")},
           {"selection", get(c, "selection")},
           {"inputs", get(c, "inputs") ? get(c, "inputs") : array()},
           {"coverage", get(c, "coverage") ? get(c, "coverage") : array()}});
      result.evidence.push_back(std::move(e));
    }
    // Insert observed producer outputs into the native policy copy; Lua cannot authorize a producer merely by naming it.
    auto                   policy = std::make_shared<value>(*x.plan);
    std::vector<value_ptr> producers;
    for (const auto& p : list(x.plan, "producers")) {
      auto row              = std::make_shared<value>(*p);
      row->fields["output"] = str(x.outputs[string(p, "work")]);
      producers.push_back(row);
    }
    policy->fields["producers"] = array(std::move(producers));
    auto [passed, diff]         = boundary_gate(policy, x.original, x.current);
    auto diff_json              = codec_encode(diff);
    if (!diff_json)
      return std::unexpected(diff_json.error().detail);
    for (const auto& u : list(x.plan, "units")) {
      check_evidence e;
      e.work    = string(u, "work");
      e.check   = "planar.boundary_v1";
      e.outcome = passed ? "pass" : "fail";
      e.reason  = passed ? "passed" : "boundary";
      if (!passed) {
        e.output       = *diff_json;
        e.output_bytes = e.output.size();
      }
      e.vector = object(
          {{"version", integer(1)}, {"targets", array({object({{"work", str(e.work)}, {"output", str(x.outputs[e.work])}})})}});
      std::vector<value_ptr> producer_ids;
      for (const auto& p : list(x.plan, "producers"))
        producer_ids.push_back(get(p, "work"));
      e.definition = object({{"version", integer(1)},
                             {"work", str(e.work)},
                             {"check", str(e.check)},
                             {"gate", str("boundary_v1")},
                             {"cwd", object({{"root", get(x.plan, "root")}})},
                             {"inputs", array({str("own_output"), str("output_vector")})},
                             {"owned", get(x.plan, "owned") ? get(x.plan, "owned") : array()},
                             {"frozen", get(x.plan, "frozen") ? get(x.plan, "frozen") : array()},
                             {"excluded", get(x.plan, "excluded") ? get(x.plan, "excluded") : array()},
                             {"producers", array(std::move(producer_ids))}});
      result.evidence.push_back(std::move(e));
    }
    auto after               = capture::capture_tree_digest(x.root, x.spec);
    result.recapture_matches = after && after->entries == x.current.entries;
    if (!result.recapture_matches) {
      for (auto& e : result.evidence) {
        e.outcome = "inconclusive";
        e.reason  = "input_changed";
      }
    } else
      for (const auto& f : list(x.plan, "facets")) {
        auto paths  = get(f, "paths");
        bool exists = paths && std::ranges::all_of(paths->items, [&](const auto& path) {
                        return std::ranges::any_of(after->entries, [&](const auto& e) { return contains(path->string, e.path); });
                      });
        if (!exists)
          continue;
        auto              root = string(x.plan, "root");
        check_publication pub;
        pub.work            = string(f, "work");
        pub.facet           = string(f, "facet");
        pub.semantic        = hashed("facet_v1", object({{"kind", str("files")},
                                                         {"normalizer", str("exact_bytes_v1")},
                                                         {"root", str(root)},
                                                         {"paths", paths},
                                                         {"entries", entries(*after, root, paths)}}));
        std::uint64_t total = 0;
        for (const auto& e : after->entries)
          if (std::ranges::any_of(paths->items, [&](const auto& path) { return contains(path->string, e.path); })) {
            auto it = after->retained.find(e.path);
            if (it == after->retained.end())
              return fail("internal", "Invalid embedded command protocol");
            total += it->second.size();
            if (total > 4194304)
              return std::unexpected(diagnostic{"resource_limit", "Execution resource limit exceeded", "facet_bytes", 4194304});
            pub.files.push_back({root, e.path, e.mode, it->second});
          }
        result.publications.push_back(std::move(pub));
      }
    std::ranges::sort(result.evidence,
                      [](const auto& a, const auto& b) { return std::tie(a.work, a.check) < std::tie(b.work, b.check); });
    return result;
  } catch (const diagnostic& d) {
    return std::unexpected(d);
  }
}
} // namespace planar::engine::execution
