module planar.engine.history.interpretation_v2_codec;
import planar.engine.history.effective_catalog;
import planar.engine.hash;
import planar.engine.canonical;
namespace planar::engine::history {
namespace {
constexpr std::size_t                    maximum_bytes = 16 * 1024 * 1024;
constexpr canonical_limits               limits{256, 4000000, maximum_bytes};
const std::map<std::string, std::string> supported_versions{{"declarations", "declarations_shapes_v1"},
                                                            {"normalization", "normalization_shapes_v1"},
                                                            {"semantics", "dependencies_shapes_v1"},
                                                            {"selection", "selection_shapes_v1"},
                                                            {"normalizer", "exact_bytes_v1"},
                                                            {"canonical", "canonical_v1"},
                                                            {"hash", "xxh3_128_v1"},
                                                            {"interpretation", "interpretation_v2"},
                                                            {"projection", "projection_v2"},
                                                            {"registration", "external_registration_v1"},
                                                            {"provenance", "effective_stdlib_v1"}};
value_ptr                                object(std::map<std::string, value_ptr> fields) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::record;
  v->fields = std::move(fields);
  return v;
}
value_ptr text(std::string bytes) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = std::move(bytes);
  return v;
}
value_ptr version_two() {
  auto v     = std::make_shared<value>();
  v->kind    = value_kind::integer;
  v->integer = 2;
  return v;
}
bool kind(const value_ptr& v, value_kind expected) {
  return v && v->kind == expected;
}
bool keys(const value_ptr& v, std::initializer_list<std::string_view> expected) {
  return kind(v, value_kind::record) && v->fields.size() == expected.size() &&
         std::ranges::all_of(expected, [&](auto key) { return v->fields.contains(std::string(key)); });
}
value_ptr at(const value_ptr& v, std::string_view key) {
  if (!kind(v, value_kind::record))
    return {};
  auto i = v->fields.find(std::string(key));
  return i == v->fields.end() ? value_ptr{} : i->second;
}
bool strings(const value_ptr& v, std::initializer_list<std::string_view> fields) {
  return std::ranges::all_of(fields, [&](auto key) { return kind(at(v, key), value_kind::string); });
}
bool records(const value_ptr& v) {
  return kind(v, value_kind::list) && std::ranges::all_of(v->items, [](auto& entry) { return kind(entry, value_kind::record); });
}
bool lists(const value_ptr& v, std::initializer_list<std::string_view> fields) {
  return std::ranges::all_of(fields, [&](auto key) { return kind(at(v, key), value_kind::list); });
}
bool shape(const value_ptr& envelope, const value_ptr& versions) {
  if (!keys(envelope, {"version", "bundle", "semantic", "versions", "extensions"}) ||
      !kind(at(envelope, "version"), value_kind::integer) || !kind(versions, value_kind::record) ||
      versions->fields.size() != supported_versions.size() + 1)
    return false;
  for (const auto& [key, unused] : supported_versions)
    if (!kind(at(versions, key), value_kind::string))
      return false;
  auto digest = at(versions, "stdlib_digest");
  if (!kind(digest, value_kind::string) || digest->string.size() != 32 ||
      !std::ranges::all_of(digest->string, [](unsigned char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); }))
    return false;
  auto b = at(envelope, "bundle"), s = at(envelope, "semantic");
  if (!keys(b, {"workspace", "spec", "jobs", "registry", "workspace_file", "spec_file", "registry_file"}) ||
      !strings(b, {"workspace_file", "spec_file", "registry_file"}) || !kind(at(b, "workspace"), value_kind::record) ||
      !kind(at(b, "spec"), value_kind::record) || !kind(at(b, "registry"), value_kind::record) || !records(at(b, "jobs")) ||
      !keys(s, {"dependencies", "jobs"}) || !records(at(s, "dependencies")) || !records(at(s, "jobs")))
    return false;
  // Check recorded structural containers only; do not apply today's schema values,
  // constructor defaults, ownership rules or identifier/reference policy here.
  auto workspace = at(b, "workspace"), spec = at(b, "spec"), registry = at(b, "registry");
  if (!strings(workspace, {"id", "spec"}) || !kind(at(workspace, "version"), value_kind::integer) ||
      !kind(at(workspace, "root"), value_kind::record) || !kind(at(workspace, "registry"), value_kind::record) ||
      !records(at(workspace, "jobs")) || !lists(workspace, {"checks", "toolchains"}) || !strings(spec, {"id"}) ||
      !kind(at(spec, "version"), value_kind::integer) || !kind(at(spec, "goal"), value_kind::record) ||
      (at(spec, "rationale") && !kind(at(spec, "rationale"), value_kind::record)) || !lists(spec, {"frozen"}) ||
      !strings(registry, {"id", "revision"}) || !kind(at(registry, "version"), value_kind::integer) ||
      !records(at(registry, "checks")) || !records(at(registry, "toolchains")))
    return false;
  for (auto key : {"constraints", "contracts", "dependencies", "facets", "milestones", "prescriptions", "questions", "work"})
    if (!records(at(spec, key)))
      return false;
  for (auto key : {"workspace_file", "spec_file", "registry_file"})
    if (!valid_path(at(b, key)->string))
      return false;
  for (const auto& dependency : at(s, "dependencies")->items)
    if (!keys(dependency, {"id", "producer", "consumer", "source", "ref", "phase", "quality"}) ||
        !strings(dependency, {"id", "producer", "consumer", "source", "ref", "phase", "quality"}))
      return false;
  auto bj = at(b, "jobs"), sj = at(s, "jobs");
  if (bj->items.size() != sj->items.size())
    return false;
  std::string previous;
  for (std::size_t i = 0; i < bj->items.size(); ++i) {
    auto entry = bj->items[i], model = sj->items[i], job = at(entry, "value");
    if (!keys(entry, {"file", "value"}) || !strings(entry, {"file"}) || !strings(job, {"id", "shape"}) ||
        !(keys(model, {"id", "file", "assignments", "reviewer", "effective_checks", "pipeline_predecessors", "graph"}) ||
          keys(model, {"id", "file", "assignments", "reviewer", "effective_checks", "pipeline_predecessors", "graph",
                       "selected_shape"})) ||
        !strings(model, {"id", "file", "reviewer"}) ||
        !lists(model, {"assignments", "effective_checks", "pipeline_predecessors"}) ||
        !keys(at(model, "graph"), {"vertices", "edges"}) || !lists(at(model, "graph"), {"vertices", "edges"}))
      return false;
    if (!records(at(job, "roles")) || !kind(at(job, "version"), value_kind::integer))
      return false;
    for (const auto& assignment : at(model, "assignments")->items)
      if (!keys(assignment, {"work", "role"}) || !strings(assignment, {"work", "role"}))
        return false;
    for (const auto& check : at(model, "effective_checks")->items) {
      if (!keys(check, {"work", "checks"}) || !strings(check, {"work"}) || !lists(check, {"checks"}))
        return false;
      for (const auto& name : at(check, "checks")->items)
        if (!kind(name, value_kind::string))
          return false;
    }
    for (const auto& predecessor : at(model, "pipeline_predecessors")->items)
      if (!keys(predecessor, {"id", "source", "producer", "consumer", "producer_role", "consumer_role", "phase", "quality"}) ||
          !strings(predecessor, {"id", "source", "producer", "consumer", "producer_role", "consumer_role", "phase", "quality"}))
        return false;
    for (const auto& vertex : at(at(model, "graph"), "vertices")->items)
      if (!kind(vertex, value_kind::string))
        return false;
    for (const auto& edge : at(at(model, "graph"), "edges")->items)
      if (!keys(edge, {"from", "to"}) || !strings(edge, {"from", "to"}))
        return false;
    auto id = at(job, "id")->string, file = at(entry, "file")->string;
    if (id.empty() || id <= previous || !valid_path(file) || id != at(model, "id")->string || file != at(model, "file")->string)
      return false;
    previous = std::move(id);
  }
  return true;
}
using code = interpretation_v2_error_code;
[[noreturn]] void fail(code c = code::malformed) {
  throw interpretation_v2_error{c};
}
void require(bool yes, code c = code::malformed) {
  if (!yes)
    fail(c);
}
value_ptr array(std::vector<value_ptr> items = {}) {
  auto v   = std::make_shared<value>();
  v->kind  = value_kind::list;
  v->items = std::move(items);
  return v;
}
std::string encode(const value_ptr& v, canonical_limits bounds = limits) {
  auto r = canonical_encode(v, bounds);
  if (!r)
    fail(r.error().code == "resource_limit" ? code::resource_limit : code::malformed);
  return std::move(*r);
}
value_ptr decode(std::string_view s) {
  auto r = canonical_decode(s, limits);
  if (!r)
    fail(r.error().code == "resource_limit" ? code::resource_limit : code::malformed);
  return *r;
}
bool lowerhex(std::string_view s) {
  return std::ranges::all_of(s, [](char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); });
}
bool definition(const value_ptr& v) {
  return kind(v, value_kind::string) && v->string.size() == 41 && v->string.starts_with("xxh3_128:") &&
         lowerhex(std::string_view(v->string).substr(9));
}
bool id(std::string_view s) {
  if (s.empty() || s.size() > 64 || s.front() < 'a' || s.front() > 'z')
    return false;
  return std::ranges::all_of(s, [](char c) { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'; });
}
void selection_syntax(const value_ptr& root) {
  auto e = at(root, "extensions");
  require(keys(e, {"registration", "catalog", "selected_shapes"}) && strings(e, {"registration"}) &&
          kind(at(e, "catalog"), value_kind::record) && records(at(e, "selected_shapes")));
  // Catalog's owner checks require actual ordered manifest paths, and follow tree verification.
  encode(at(e, "catalog"), {.depth = 12, .nodes = 23000, .bytes = 1048576});
  std::string previous;
  for (auto& entry : at(e, "selected_shapes")->items) {
    require(keys(entry, {"job", "id", "revision", "definition"}) && strings(entry, {"job", "id", "revision"}) &&
            definition(at(entry, "definition")));
    auto job = at(entry, "job")->string;
    require(id(job) && id(at(entry, "id")->string) && id(at(entry, "revision")->string) && job > previous);
    previous = job;
  }
  std::size_t total = 0;
  auto        b = at(root, "bundle"), s = at(root, "semantic");
  for (std::size_t i = 0; i < at(s, "jobs")->items.size(); ++i) {
    auto model = at(s, "jobs")->items[i], selected = at(model, "selected_shape");
    if (!selected)
      continue;
    require(keys(selected, {"id", "revision", "definition", "obligations", "guidance"}) &&
            strings(selected, {"id", "revision", "guidance"}) && definition(at(selected, "definition")) &&
            records(at(selected, "obligations")));
    require(id(at(selected, "id")->string) && id(at(selected, "revision")->string));
    auto guidance = at(selected, "guidance");
    require(guidance->string.size() <= 65536, code::resource_limit);
    require(valid_text(guidance->string));
    auto obligations = at(selected, "obligations");
    require(obligations->items.size() <= 10000 && obligations->items.size() <= 100000 - total, code::resource_limit);
    total += obligations->items.size();
    std::map<std::string, std::string> assignments;
    for (auto& a : at(model, "assignments")->items)
      assignments.emplace(at(a, "work")->string, at(a, "role")->string);
    std::set<std::string> work, roles;
    for (auto& w : at(at(b, "spec"), "work")->items)
      if (strings(w, {"id"}))
        work.insert(at(w, "id")->string);
    for (auto& r : at(at(at(b, "jobs")->items[i], "value"), "roles")->items)
      if (strings(r, {"id"}))
        roles.insert(at(r, "id")->string);
    previous.clear();
    auto prefix = "shape:" + at(selected, "id")->string + ":";
    for (auto& o : obligations->items) {
      require(keys(o, {"id", "source", "producer", "consumer", "producer_role", "consumer_role", "phase", "quality"}) &&
              strings(o, {"id", "source", "producer", "consumer", "producer_role", "consumer_role", "phase", "quality"}));
      auto name = at(o, "id")->string, p = at(o, "producer")->string, c = at(o, "consumer")->string,
           pr = at(o, "producer_role")->string, cr = at(o, "consumer_role")->string;
      require(name.starts_with(prefix) && id(std::string_view(name).substr(prefix.size())) && name > previous &&
              at(o, "source")->string == "shape" && p != c && work.contains(p) && work.contains(c) && assignments.contains(p) &&
              assignments.contains(c) && assignments.at(p) == pr && assignments.at(c) == cr && roles.contains(pr) &&
              roles.contains(cr) && (at(o, "phase")->string == "start" || at(o, "phase")->string == "complete") &&
              at(o, "quality")->string == "done");
      previous = name;
    }
  }
}
struct verified_context {
  prepared_provenance           tree;
  value_ptr                     manifest, embedded, catalog;
  std::vector<std::string_view> shipped, external;
  std::string                   catalog_bytes, digest;
  explicit verified_context(std::span<const tree_file> files) {
    auto checked = verify_effective_stdlib(files);
    if (!checked)
      fail(checked.error().code == effective_error_code::resource_limit ? code::resource_limit : code::invalid_tree);
    tree     = std::move(*checked);
    manifest = decode(file("__planar__/effective.bin").payload);
    embedded = decode(file("__planar__/embedded.bin").payload);
    for (auto& p : at(embedded, "files")->items)
      shipped.push_back(p->string);
    for (auto& p : at(manifest, "modules")->items)
      external.push_back(std::string_view(at(p, "path")->string).substr(9));
    catalog       = at(manifest, "catalog");
    catalog_bytes = encode(catalog);
    for (auto byte : tree.digest)
      digest += std::format("{:02x}", byte);
  }
  const tree_file& file(std::string_view path) const {
    auto i = std::ranges::find(tree.files, path, &tree_file::path);
    require(i != tree.files.end(), code::invalid_tree);
    return *i;
  }
  std::string shape_definition(const value_ptr& entry) const {
    auto        owner = at(entry, "owner");
    auto&       bytes = file(at(owner, "module")->string).payload;
    std::string hex;
    hex.reserve(bytes.size() * 2);
    constexpr char digits[] = "0123456789abcdef";
    for (unsigned char c : bytes) {
      hex += digits[c >> 4];
      hex += digits[c & 15];
    }
    auto h = hash_value("shape_definition_v1",
                        object({{"descriptor", at(entry, "descriptor")},
                                {"owner", object({{"tier", at(owner, "tier")}, {"module", at(owner, "module")}})},
                                {"module_bytes", text(std::move(hex))}}));
    if (!h)
      fail(h.error().code == "resource_limit" ? code::resource_limit : code::invalid_correspondence);
    return *h;
  }
};
void verify_correspondence(const value_ptr& root, const value_ptr& versions, const verified_context& context) {
  auto e        = at(root, "extensions");
  auto asserted = decode_catalog(encode(at(e, "catalog")), context.shipped, context.external);
  if (!asserted)
    fail(asserted.error().code == catalog_error_code::resource_limit ? code::resource_limit : code::malformed);
  require(asserted->bytes() == context.catalog_bytes, code::invalid_correspondence);
  require(at(versions, "stdlib_digest")->string == context.digest, code::invalid_correspondence);
  std::size_t index      = 0;
  auto        selections = at(e, "selected_shapes");
  auto        bj = at(at(root, "bundle"), "jobs"), sj = at(at(root, "semantic"), "jobs");
  for (std::size_t i = 0; i < bj->items.size(); ++i) {
    auto      job = at(bj->items[i], "value"), model = sj->items[i], selected = at(model, "selected_shape");
    value_ptr entry;
    for (auto& candidate : at(context.catalog, "shapes")->items)
      if (at(at(candidate, "descriptor"), "id")->string == at(job, "shape")->string)
        entry = candidate;
    require(bool(entry), code::invalid_correspondence);
    bool custom = at(at(entry, "owner"), "tier")->string == "external";
    require(custom == bool(selected), code::invalid_correspondence);
    if (!custom)
      continue;
    require(index < selections->items.size(), code::invalid_correspondence);
    auto record = selections->items[index++], descriptor = at(entry, "descriptor");
    auto h = context.shape_definition(entry);
    require(at(record, "job")->string == at(job, "id")->string, code::invalid_correspondence);
    for (auto key : {"id", "revision"})
      require(at(record, key)->string == at(selected, key)->string && at(selected, key)->string == at(descriptor, key)->string,
              code::invalid_correspondence);
    require(at(record, "definition")->string == h && at(selected, "definition")->string == h, code::invalid_correspondence);
  }
  require(index == selections->items.size(), code::invalid_correspondence);
}
recorded_interpretation_v2 checked(std::string_view encoded, std::string_view versions_encoded, const verified_context* retained,
                                   std::span<const tree_file> files) {
  require(encoded.size() <= maximum_bytes && versions_encoded.size() <= maximum_bytes, code::resource_limit);
  auto root = decode(encoded), versions = decode(versions_encoded);
  require(shape(root, versions));
  selection_syntax(root);
  require(encode(at(root, "versions")) == versions_encoded);
  bool supported =
      at(root, "version")->integer == 2 && at(at(root, "extensions"), "registration")->string == "external_registration_v1";
  for (auto& [key, value] : supported_versions)
    supported &= at(versions, key)->string == value;
  require(supported, code::unsupported_version);
  if (retained)
    verify_correspondence(root, versions, *retained);
  else {
    verified_context context(files);
    verify_correspondence(root, versions, context);
  }
  return {root, versions, std::string(encoded), std::string(versions_encoded)};
}
} // namespace
std::expected<recorded_interpretation_v2, interpretation_v2_error>
decode_interpretation_v2(std::string_view encoded, std::string_view versions_encoded, std::span<const tree_file> raw_files) {
  try {
    return checked(encoded, versions_encoded, nullptr, raw_files);
  } catch (const interpretation_v2_error& e) {
    return std::unexpected(e);
  } catch (const std::bad_alloc&) {
    return std::unexpected(interpretation_v2_error{code::resource_limit});
  }
}
std::expected<recorded_interpretation_v2, interpretation_v2_error>
encode_interpretation_v2(const value_ptr& bundle, const value_ptr& semantic, std::span<const tree_file> raw_files) {
  try {
    // Bound/canonicalize caller graphs before inspecting them, including cycles and shared aliases.
    auto                             b = decode(encode(bundle)), s = decode(encode(semantic));
    verified_context                 context(raw_files);
    std::map<std::string, value_ptr> fields;
    for (auto& [key, value] : supported_versions)
      fields.emplace(key, text(value));
    fields.emplace("stdlib_digest", text(context.digest));
    auto                   versions = object(std::move(fields));
    std::vector<value_ptr> selections;
    if (records(at(s, "jobs")))
      for (auto& model : at(s, "jobs")->items) {
        auto selected = at(model, "selected_shape");
        if (!selected)
          continue;
        require(strings(model, {"id"}) && strings(selected, {"id", "revision", "definition"}));
        selections.push_back(object({{"job", at(model, "id")},
                                     {"id", at(selected, "id")},
                                     {"revision", at(selected, "revision")},
                                     {"definition", at(selected, "definition")}}));
      }
    auto e     = object({{"registration", text("external_registration_v1")},
                         {"catalog", context.catalog},
                         {"selected_shapes", array(std::move(selections))}});
    auto root  = object({{"version", version_two()}, {"bundle", b}, {"semantic", s}, {"versions", versions}, {"extensions", e}});
    auto bytes = encode(root), vbytes = encode(versions);
    return checked(bytes, vbytes, &context, {});
  } catch (const interpretation_v2_error& e) {
    return std::unexpected(e);
  } catch (const std::bad_alloc&) {
    return std::unexpected(interpretation_v2_error{code::resource_limit});
  }
}
} // namespace planar::engine::history
