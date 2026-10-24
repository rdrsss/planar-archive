module planar.engine.history.recorded_interpretation;
import planar.engine.canonical;
namespace planar::engine::history {
namespace {
using code                         = recorded_error_code;
constexpr std::size_t      maximum = 16 * 1024 * 1024;
constexpr canonical_limits limits{256, 4000000, maximum};
struct failure {
  code category;
};
void require(bool condition, code category = code::malformed) {
  if (!condition)
    throw failure{category};
}
bool kind(const value_ptr& v, value_kind k) {
  return v && v->kind == k;
}
value_ptr at(const value_ptr& v, std::string_view key) {
  if (!kind(v, value_kind::record))
    return {};
  auto i = v->fields.find(std::string(key));
  return i == v->fields.end() ? value_ptr{} : i->second;
}
bool keys(const value_ptr& v, std::initializer_list<std::string_view> names) {
  return kind(v, value_kind::record) && v->fields.size() == names.size() &&
         std::ranges::all_of(names, [&](auto name) { return static_cast<bool>(at(v, name)); });
}
value_ptr decode(std::string_view bytes, canonical_limits bound = limits) {
  auto v = canonical_decode(bytes, bound);
  if (!v)
    throw failure{v.error().code == "resource_limit" ? code::resource_limit : code::malformed};
  return *v;
}
recorded_family classify(std::string_view encoded, std::string_view versions_encoded) {
  require(encoded.size() <= maximum && versions_encoded.size() <= maximum, code::resource_limit);
  auto root = decode(encoded), versions = decode(versions_encoded);
  auto marker = [](const value_ptr& v) { return at(v, "provenance") || at(v, "registration"); };
  return at(root, "extensions") || marker(at(root, "versions")) || marker(versions) ? recorded_family::effective
                                                                                    : recorded_family::embedded;
}
void effective_manifest(std::span<const tree_file> files) {
  auto i = std::ranges::find(files, "__planar__/effective.bin", &tree_file::path);
  require(i != files.end());
  auto m = decode(i->payload, {16, 24000, 1024 * 1024});
  require(keys(m, {"version", "scheme", "registration", "embedded", "assembly", "profile", "modules", "catalog"}));
  require(kind(at(m, "version"), value_kind::integer));
  for (auto name : {"scheme", "registration", "embedded", "assembly", "profile"})
    require(kind(at(m, name), value_kind::string));
  auto modules = at(m, "modules"), catalog = at(m, "catalog");
  require(kind(modules, value_kind::list));
  require(modules->items.size() <= 64, code::resource_limit);
  for (const auto& entry : modules->items) {
    require(keys(entry, {"path", "mode"}));
    auto path = at(entry, "path"), mode = at(entry, "mode");
    require(kind(path, value_kind::string) && kind(mode, value_kind::integer));
    require(path->string.starts_with("external/") && valid_path(path->string) && (mode->integer == 420 || mode->integer == 493));
  }
  require(keys(catalog, {"version", "commands", "shapes", "projections"}));
  require(kind(at(catalog, "version"), value_kind::integer));
  for (auto name : {"commands", "shapes", "projections"})
    require(kind(at(catalog, name), value_kind::list));
  require(at(m, "version")->integer == 1 && at(m, "scheme")->string == "effective_stdlib_v1", code::unsupported_manifest);
}
code provenance_code(provenance_error_code c) {
  if (c == provenance_error_code::resource_limit)
    return code::resource_limit;
  if (c == provenance_error_code::unsupported_manifest)
    return code::unsupported_manifest;
  if (c == provenance_error_code::invalid_tree)
    return code::invalid_tree;
  return code::malformed;
}
record_digest provenance(recorded_family family, std::span<const tree_file> files) {
  std::vector<tree_file_view> views;
  // File-count preflight precedes allocating a per-file view array.
  require(files.size() <= (family == recorded_family::effective ? 8192u : 4096u), code::resource_limit);
  views.reserve(files.size());
  for (const auto& f : files)
    views.push_back({f.path, f.payload, f.mode});
  auto digest =
      digest_tree(views, family == recorded_family::effective ? tree_profile::effective_provenance : tree_profile::provenance);
  if (!digest)
    throw failure{digest.error().code == tree_error_code::resource_limit ? code::resource_limit : code::invalid_tree};
  if (family == recorded_family::embedded) {
    auto verified = verify_stdlib(files);
    if (!verified)
      throw failure{provenance_code(verified.error().code)};
    return verified->digest;
  }
  effective_manifest(files);
  auto verified = verify_effective_stdlib(files);
  if (!verified)
    throw failure{verified.error().code == effective_error_code::resource_limit ? code::resource_limit : code::invalid_tree};
  return verified->digest;
}
void correspondence(const value_ptr& root, const value_ptr& versions, const record_digest& digest, std::string_view workspace) {
  // Both fixed decoders check these kinds before either supported or unsupported status.
  auto id = at(at(at(root, "bundle"), "workspace"), "id"), claimed = at(versions, "stdlib_digest");
  require(kind(id, value_kind::string) && kind(claimed, value_kind::string));
  std::string actual;
  for (auto byte : digest)
    actual += std::format("{:02x}", byte);
  require(id->string == workspace && claimed->string == actual, code::invalid_correspondence);
}
code v2_code(interpretation_v2_error_code c) {
  if (c == interpretation_v2_error_code::resource_limit)
    return code::resource_limit;
  if (c == interpretation_v2_error_code::invalid_tree)
    return code::invalid_tree;
  if (c == interpretation_v2_error_code::invalid_correspondence)
    return code::invalid_correspondence;
  return code::malformed;
}
} // namespace
std::expected<recorded_family, recorded_error> classify_recorded_interpretation(std::string_view encoded,
                                                                                std::string_view versions_encoded) {
  try {
    return classify(encoded, versions_encoded);
  } catch (const failure& e) {
    return std::unexpected(recorded_error{e.category});
  } catch (const std::bad_alloc&) {
    return std::unexpected(recorded_error{code::resource_limit});
  }
}
std::expected<verified_recorded_interpretation, recorded_error>
verify_recorded_interpretation(std::string_view encoded, std::string_view versions_encoded, std::span<const tree_file> raw_files,
                               std::string_view workspace) {
  try {
    auto                                           family = classify(encoded, versions_encoded);
    auto                                           digest = provenance(family, raw_files);
    std::optional<decoded_recorded_interpretation> result;
    if (family == recorded_family::embedded) {
      auto decoded = decode_interpretation(encoded, versions_encoded);
      if (decoded)
        result.emplace(std::move(decoded->interpretation), std::move(decoded->versions), std::move(decoded->encoded),
                       std::move(decoded->versions_encoded));
      else if (decoded.error().code != interpretation_error_code::unsupported_version)
        throw failure{decoded.error().code == interpretation_error_code::resource_limit ? code::resource_limit : code::malformed};
    } else {
      auto decoded = decode_interpretation_v2(encoded, versions_encoded, raw_files);
      if (decoded)
        result.emplace(std::move(decoded->interpretation), std::move(decoded->versions), std::move(decoded->encoded),
                       std::move(decoded->versions_encoded));
      else if (decoded.error().code != interpretation_v2_error_code::unsupported_version)
        throw failure{v2_code(decoded.error().code)};
    }
    if (result) {
      correspondence(result->interpretation, result->versions, digest, workspace);
      return verified_recorded_interpretation{family, recorded_status::decoded, digest, std::move(result)};
    }
    // Unknown semantics never leave this scope; only fixed-decoder-checked workspace/digest are inspected.
    correspondence(decode(encoded), decode(versions_encoded), digest, workspace);
    return verified_recorded_interpretation{family, recorded_status::unsupported_interpretation, digest, {}};
  } catch (const failure& e) {
    return std::unexpected(recorded_error{e.category});
  } catch (const std::bad_alloc&) {
    return std::unexpected(recorded_error{code::resource_limit});
  }
}
} // namespace planar::engine::history
