module planar.engine.history.interpretation_codec;
import planar.engine.canonical;
namespace planar::engine::history {
namespace {
constexpr std::size_t                    maximum_bytes = 16 * 1024 * 1024;
constexpr canonical_limits               limits{256, 4000000, maximum_bytes};
const std::map<std::string, std::string> supported_versions{{"declarations", "declarations_v1"},
                                                            {"normalization", "normalization_v1"},
                                                            {"semantics", "dependencies_v1"},
                                                            {"selection", "selection_v1"},
                                                            {"normalizer", "exact_bytes_v1"},
                                                            {"canonical", "canonical_v1"},
                                                            {"hash", "xxh3_128_v1"},
                                                            {"interpretation", "interpretation_v1"},
                                                            {"projection", "projection_v2"}};
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
value_ptr version_one() {
  auto v     = std::make_shared<value>();
  v->kind    = value_kind::integer;
  v->integer = 1;
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
  if (!keys(envelope, {"version", "bundle", "semantic", "versions"}) || !kind(at(envelope, "version"), value_kind::integer) ||
      !kind(versions, value_kind::record) || versions->fields.size() != supported_versions.size() + 1)
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
    if (!keys(entry, {"file", "value"}) || !strings(entry, {"file"}) || !strings(job, {"id"}) ||
        !keys(model, {"id", "file", "assignments", "reviewer", "effective_checks", "pipeline_predecessors", "graph"}) ||
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
interpretation_error codec_failure(const diagnostic& error) {
  return {
      error.code == "resource_limit" ? interpretation_error_code::resource_limit : interpretation_error_code::malformed, {}, {}};
}
} // namespace
std::expected<recorded_interpretation, interpretation_error> decode_interpretation(std::string_view encoded,
                                                                                   std::string_view versions_encoded) {
  auto v = canonical_decode(encoded, limits);
  if (!v)
    return std::unexpected(codec_failure(v.error()));
  auto versions = canonical_decode(versions_encoded, limits);
  if (!versions)
    return std::unexpected(codec_failure(versions.error()));
  if (!shape(*v, *versions))
    return std::unexpected(interpretation_error{interpretation_error_code::malformed});
  auto embedded_versions = canonical_encode(at(*v, "versions"), limits);
  if (!embedded_versions)
    return std::unexpected(codec_failure(embedded_versions.error()));
  if (*embedded_versions != versions_encoded)
    return std::unexpected(interpretation_error{interpretation_error_code::malformed});
  bool supported = at(*v, "version")->integer == 1;
  for (const auto& [key, expected] : supported_versions)
    supported &= at(*versions, key)->string == expected;
  if (!supported)
    return std::unexpected(interpretation_error{interpretation_error_code::unsupported_version});
  return recorded_interpretation{*v, *versions, std::string(encoded), std::string(versions_encoded)};
}
std::expected<recorded_interpretation, interpretation_error>
encode_interpretation(const value_ptr& bundle, const value_ptr& semantic, std::string_view stdlib_digest) {
  std::map<std::string, value_ptr> fields;
  for (const auto& [key, val] : supported_versions)
    fields.emplace(key, text(val));
  fields.emplace("stdlib_digest", text(std::string(stdlib_digest)));
  auto versions = object(std::move(fields));
  auto envelope = object({{"version", version_one()}, {"bundle", bundle}, {"semantic", semantic}, {"versions", versions}});
  auto encoded = canonical_encode(envelope, limits), version_bytes = canonical_encode(versions, limits);
  if (!encoded)
    return std::unexpected(codec_failure(encoded.error()));
  if (!version_bytes)
    return std::unexpected(codec_failure(version_bytes.error()));
  auto checked = decode_interpretation(*encoded, *version_bytes);
  if (!checked)
    return std::unexpected(checked.error());
  return checked;
}
} // namespace planar::engine::history
