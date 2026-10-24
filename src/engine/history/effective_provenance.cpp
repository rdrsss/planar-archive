module planar.engine.history.effective_provenance;
import std;
import planar.engine.capture.profile_codec;
import planar.engine.history.effective_catalog;
import planar.engine.canonical;
namespace planar::engine::history {
namespace {
constexpr std::size_t      mib       = 1024 * 1024;
constexpr std::string_view old_chunk = "__planar__/assembly.lua", old_manifest = "__planar__/assembly.canonical";
constexpr std::string_view chunk = "__planar__/embedded.lua", embedded_manifest = "__planar__/embedded.bin";
constexpr std::string_view effective_manifest = "__planar__/effective.bin", profile_path = "profile/input.json";
auto                       refuse(effective_error_code code = effective_error_code::invalid_provenance) {
  return std::unexpected(effective_error{code});
}
bool add(std::size_t& total, std::size_t n, std::size_t cap) {
  if (n > cap - total)
    return false;
  total += n;
  return true;
}
effective_error_code embedded_code(provenance_error e) {
  if (e.code == provenance_error_code::resource_limit)
    return effective_error_code::resource_limit;
  if (e.code == provenance_error_code::invalid_tree)
    return effective_error_code::invalid_tree;
  return effective_error_code::invalid_provenance;
}
effective_error_code catalog_code(catalog_error e) {
  if (e.code == catalog_error_code::resource_limit)
    return effective_error_code::resource_limit;
  if (e.code == catalog_error_code::identity_conflict)
    return effective_error_code::identity_conflict;
  return effective_error_code::invalid_catalog;
}
value_ptr str(std::string_view s) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = s;
  return v;
}
value_ptr num(std::int64_t n) {
  auto v     = std::make_shared<value>();
  v->kind    = value_kind::integer;
  v->integer = n;
  return v;
}
value_ptr obj(std::map<std::string, value_ptr> fields) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::record;
  v->fields = std::move(fields);
  return v;
}
value_ptr manifest(std::span<const tree_file_view> modules, const value_ptr& catalog) {
  auto list  = std::make_shared<value>();
  list->kind = value_kind::list;
  for (auto& m : modules)
    list->items.push_back(obj({{"path", str("external/" + std::string(m.path))}, {"mode", num(m.mode)}}));
  return obj({{"version", num(1)},
              {"scheme", str("effective_stdlib_v1")},
              {"registration", str("external_registration_v1")},
              {"embedded", str(embedded_manifest)},
              {"assembly", str(chunk)},
              {"profile", str(profile_path)},
              {"modules", list},
              {"catalog", catalog}});
}
// Metadata-only preflight: all component counts/bytes and prefixed path limits precede owned payload copies.
std::expected<void, effective_error> preflight(std::span<const tree_file_view> files) {
  if (files.size() > 8192)
    return refuse(effective_error_code::resource_limit);
  std::size_t total = 0, embedded = 0, external = 0, embedded_count = 0, external_count = 0;
  for (auto& f : files) {
    if (f.path.size() > 1024 || std::ranges::count(f.path, '/') >= 64 || f.payload.size() > 16 * mib ||
        !add(total, f.payload.size(), 32 * mib))
      return refuse(effective_error_code::resource_limit);
    if (!valid_path(f.path) || (f.mode != 420 && f.mode != 493))
      return refuse(effective_error_code::invalid_tree);
    if (f.path.starts_with("shipped/") || f.path == chunk || f.path == embedded_manifest) {
      if (++embedded_count > 4096 || !add(embedded, f.payload.size(), 16 * mib))
        return refuse(effective_error_code::resource_limit);
      if (f.mode != 420)
        return refuse();
    } else if (f.path.starts_with("external/")) {
      if (++external_count > 64 || f.payload.size() > mib || !add(external, f.payload.size(), 8 * mib))
        return refuse(effective_error_code::resource_limit);
    } else if (f.path == profile_path) {
      if (f.payload.size() > 65536)
        return refuse(effective_error_code::resource_limit);
    } else if (f.path == effective_manifest) {
      if (f.payload.size() > mib)
        return refuse(effective_error_code::resource_limit);
      if (f.mode != 420)
        return refuse();
    } else
      return refuse();
  }
  return {};
}
std::vector<tree_file_view> views(std::span<const tree_file> files) {
  std::vector<tree_file_view> result;
  result.reserve(files.size());
  for (auto& f : files)
    result.push_back({f.path, f.payload, f.mode});
  return result;
}
} // namespace
std::expected<prepared_provenance, effective_error> prepare_effective_stdlib(const effective_input& input) {
  if (input.embedded.size() > 4096 || input.modules.size() > 64 || input.profile.size() > 65536 || input.catalog.size() > mib)
    return refuse(effective_error_code::resource_limit);
  std::vector<std::string> paths;
  paths.reserve(input.embedded.size() + input.modules.size());
  std::vector<tree_file_view> files;
  files.reserve(input.embedded.size() + input.modules.size() + 2);
  for (auto& f : input.embedded) {
    // Bound old spelling before constructing a prefixed path.
    if (f.path.size() > 1024)
      return refuse(effective_error_code::resource_limit);
    paths.push_back(f.path == old_chunk      ? std::string(chunk)
                    : f.path == old_manifest ? std::string(embedded_manifest)
                                             : "shipped/" + f.path);
    files.push_back({paths.back(), f.payload, f.mode});
  }
  files.push_back({profile_path, input.profile, input.profile_mode});
  for (auto& f : input.modules) {
    if (f.path.size() > 1024)
      return refuse(effective_error_code::resource_limit);
    paths.push_back("external/" + std::string(f.path));
    files.push_back({paths.back(), f.payload, f.mode});
  }
  auto check = preflight(files);
  if (!check)
    return std::unexpected(check.error());
  auto profile = decode_profile(input.profile);
  if (!profile)
    return refuse(profile.error().detail.code == "resource_limit" ? effective_error_code::resource_limit
                                                                  : effective_error_code::invalid_profile);
  if (profile->size() != input.modules.size())
    return refuse();
  std::vector<std::string_view> external;
  external.reserve(profile->size());
  for (std::size_t i = 0; i < profile->size(); ++i) {
    if ((*profile)[i] != input.modules[i].path)
      return refuse();
    external.push_back((*profile)[i]);
  }
  auto metadata = std::ranges::find(input.embedded, old_manifest, &tree_file::path);
  if (metadata == input.embedded.end())
    return refuse();
  // Decode only bounded order metadata before old verification takes owned payload copies.
  // The unchanged verifier below still proves the complete closed schema and assembly.
  auto decoded = canonical_decode(metadata->payload, {.depth = 4, .nodes = 8200, .bytes = 16 * mib});
  if (!decoded)
    return refuse(decoded.error().code == "resource_limit" ? effective_error_code::resource_limit
                                                           : effective_error_code::invalid_provenance);
  if ((*decoded)->kind != value_kind::record || !(*decoded)->fields.contains("files"))
    return refuse();
  auto originals = (*decoded)->fields.at("files");
  if (originals->kind != value_kind::list || originals->items.size() > 4094)
    return refuse();
  std::vector<std::string_view> shipped;
  for (auto& p : originals->items) {
    if (p->kind != value_kind::string)
      return refuse();
    shipped.push_back(p->string);
  }
  auto catalog = decode_catalog(input.catalog, shipped, external);
  if (!catalog)
    return refuse(catalog_code(catalog.error()));
  auto encoded = canonical_encode(manifest(input.modules, catalog->data()), {.depth = 16, .nodes = 24000, .bytes = mib});
  if (!encoded)
    return refuse(effective_error_code::resource_limit);
  files.push_back({effective_manifest, *encoded, 420});
  check = preflight(files);
  if (!check)
    return std::unexpected(check.error());
  auto embedded = verify_stdlib(input.embedded);
  if (!embedded)
    return refuse(embedded_code(embedded.error()));
  auto digest = digest_tree(files, tree_profile::effective_provenance);
  if (!digest)
    return refuse(digest.error().code == tree_error_code::resource_limit ? effective_error_code::resource_limit
                                                                         : effective_error_code::invalid_tree);
  prepared_provenance result{{}, *digest};
  result.files.reserve(files.size());
  for (auto& f : files)
    result.files.push_back({std::string(f.path), std::string(f.payload), f.mode});
  std::ranges::sort(result.files, {}, &tree_file::path);
  return result;
}
std::expected<prepared_provenance, effective_error> verify_effective_stdlib(std::span<const tree_file> files) {
  if (files.size() > 8192)
    return refuse(effective_error_code::resource_limit);
  auto input_views = views(files);
  auto check       = preflight(input_views);
  if (!check)
    return std::unexpected(check.error());
  auto digest = digest_tree(input_views, tree_profile::effective_provenance);
  if (!digest)
    return refuse(digest.error().code == tree_error_code::resource_limit ? effective_error_code::resource_limit
                                                                         : effective_error_code::invalid_tree);
  auto find = [&](std::string_view p) -> const tree_file* {
    auto i = std::ranges::find(files, p, &tree_file::path);
    return i == files.end() ? nullptr : &*i;
  };
  auto metadata = find(effective_manifest), profile = find(profile_path);
  if (!metadata || !profile)
    return refuse();
  auto decoded = canonical_decode(metadata->payload, {.depth = 16, .nodes = 24000, .bytes = mib});
  if (!decoded)
    return refuse(decoded.error().code == "resource_limit" ? effective_error_code::resource_limit
                                                           : effective_error_code::invalid_provenance);
  if ((*decoded)->kind != value_kind::record)
    return refuse();
  const auto& fields = (*decoded)->fields;
  if (fields.size() != 8)
    return refuse();
  for (auto key : {"version", "scheme", "registration", "embedded", "assembly", "profile", "modules", "catalog"})
    if (!fields.contains(key))
      return refuse();
  auto modules = fields.at("modules");
  if (modules->kind != value_kind::list)
    return refuse();
  if (modules->items.size() > 64)
    return refuse(effective_error_code::resource_limit);
  std::vector<tree_file_view> external;
  for (auto& item : modules->items) {
    if (item->kind != value_kind::record || item->fields.size() != 2 || !item->fields.contains("path") ||
        !item->fields.contains("mode"))
      return refuse();
    auto p = item->fields.at("path"), m = item->fields.at("mode");
    if (p->kind != value_kind::string || m->kind != value_kind::integer || !p->string.starts_with("external/") ||
        (m->integer != 420 && m->integer != 493))
      return refuse();
    auto f = find(p->string);
    if (!f || f->mode != m->integer)
      return refuse();
    external.push_back({std::string_view(f->path).substr(9), f->payload, f->mode});
  }
  auto catalog = canonical_encode(fields.at("catalog"), {.depth = 12, .nodes = 23000, .bytes = mib});
  if (!catalog)
    return refuse(catalog.error().code == "resource_limit" ? effective_error_code::resource_limit
                                                           : effective_error_code::invalid_catalog);
  std::vector<tree_file> embedded;
  for (auto& f : files) {
    if (f.path.starts_with("shipped/"))
      embedded.push_back({f.path.substr(8), f.payload, f.mode});
    else if (f.path == chunk)
      embedded.push_back({std::string(old_chunk), f.payload, f.mode});
    else if (f.path == embedded_manifest)
      embedded.push_back({std::string(old_manifest), f.payload, f.mode});
  }
  auto prepared = prepare_effective_stdlib({embedded, profile->payload, profile->mode, external, *catalog});
  if (!prepared)
    return prepared;
  auto expected = std::ranges::find(prepared->files, effective_manifest, &tree_file::path);
  if (expected->payload != metadata->payload || prepared->files.size() != files.size())
    return refuse();
  // Equality covers every byte and mode; a changed but structurally consistent history gets its own digest.
  for (auto& f : files) {
    auto p = std::ranges::find(prepared->files, f.path, &tree_file::path);
    if (p == prepared->files.end() || p->payload != f.payload || p->mode != f.mode)
      return refuse();
  }
  return prepared;
}
} // namespace planar::engine::history
