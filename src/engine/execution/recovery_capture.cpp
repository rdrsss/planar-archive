module planar.engine.execution.recovery_capture;
import std;
import planar.engine.capture.native;
import planar.engine.capture.host_path;
import planar.engine.hash;
namespace planar::engine::execution {
namespace {
value_ptr make(value v) {
  return std::make_shared<const value>(std::move(v));
}
value_ptr text(std::string s) {
  value v{};
  v.kind   = value_kind::string;
  v.string = std::move(s);
  return make(std::move(v));
}
value_ptr integer(std::int64_t n) {
  value v{};
  v.kind    = value_kind::integer;
  v.integer = n;
  return make(std::move(v));
}
value_ptr boolean(bool b) {
  value v{};
  v.kind    = value_kind::boolean;
  v.boolean = b;
  return make(std::move(v));
}
value_ptr array(std::vector<value_ptr> items) {
  value v{};
  v.kind  = value_kind::list;
  v.items = std::move(items);
  return make(std::move(v));
}
value_ptr object(std::map<std::string, value_ptr> fields) {
  value v{};
  v.kind   = value_kind::record;
  v.fields = std::move(fields);
  return make(std::move(v));
}
bool contains(std::string_view boundary, std::string_view path) {
  return boundary == path || (boundary.ends_with('/') && path.starts_with(boundary));
}
std::expected<std::string, diagnostic> mapping_directory(std::string_view mapping, std::string_view root) {
  const auto equal = mapping.find('=');
  if (equal == std::string_view::npos || mapping.substr(0, equal) != root)
    return std::unexpected(diagnostic{"reference", "Root mapping must match workspace source ID"});
  return resolve_explicit_directory(mapping.substr(equal + 1));
}
} // namespace
std::expected<value_ptr, diagnostic> capture_recovery_outputs(const ledger::execution_snapshot& snapshot, std::string_view role,
                                                              std::string_view root_mapping, std::string_view base_mapping) {
  try {
    if (!snapshot.revision)
      return std::unexpected(diagnostic{"no_active_job", "No active job matches the request"});
    const ledger::execution_role* selected = nullptr;
    for (const auto& r : snapshot.roles)
      if (r.job == snapshot.active_job && r.role == role)
        selected = &r;
    if (!selected)
      return std::unexpected(diagnostic{"reference", "Unknown selected role"});
    const auto& bundle       = snapshot.revision->interpretation_value->fields.at("bundle");
    const auto& root         = bundle->fields.at("workspace")->fields.at("root");
    const auto& root_id      = root->fields.at("id")->string;
    auto        current_path = mapping_directory(root_mapping, root_id);
    if (!current_path)
      return std::unexpected(current_path.error());
    auto base_path = mapping_directory(base_mapping, root_id);
    if (!base_path)
      return std::unexpected(base_path.error());
    native_capture_session capture;
    auto                   current_root = capture.add_root("current:" + root_id, *current_path);
    if (!current_root)
      return std::unexpected(current_root.error());
    auto base_root = capture.add_root("base:" + root_id, *base_path);
    if (!base_root)
      return std::unexpected(base_root.error());
    std::vector<std::string> exclusions;
    for (const auto& e : root->fields.at("excluded")->items)
      exclusions.push_back(e->string);
    for (const auto& prefix : {"current:", "base:"}) {
      auto captured = capture.tree(prefix + root_id, exclusions);
      if (!captured)
        return std::unexpected(captured.error());
    }
    auto captured = capture.finish();
    if (!captured)
      return std::unexpected(captured.error());
    std::vector<value_ptr> base_entries, current_entries;
    for (const auto& f : **captured) {
      auto digest = file_digest(f.bytes);
      if (!digest)
        return std::unexpected(digest.error());
      auto entry = object({{"root", text(root_id)},
                           {"path", text(f.path)},
                           {"kind", text("file")},
                           {"mode", text(f.mode)},
                           {"size", integer(f.bytes.size())},
                           {"content", text(*digest)}});
      (f.root == "base:" + root_id ? base_entries : current_entries).push_back(entry);
    }
    auto baseline = hash_value("base_v1", object({{"root", text(root_id)},
                                                  {"label", root->fields.at("base")},
                                                  {"entries", array(std::move(base_entries))},
                                                  {"excluded", root->fields.at("excluded")}}));
    if (!baseline)
      return std::unexpected(baseline.error());
    std::map<std::string, std::vector<std::string>> ownership;
    for (const auto& own : selected->structure.ownership)
      ownership[own.work] = own.paths;
    const auto& ids = selected->structure.kind == "reviewer" ? selected->structure.targets : selected->structure.work;
    // Reviewer targets can be recorded separately from the role's own ownership vector.
    for (const auto& unit : bundle->fields.at("spec")->fields.at("work")->items) {
      const auto& id = unit->fields.at("id")->string;
      if (std::ranges::find(ids, id) == ids.end() || ownership.contains(id))
        continue;
      for (const auto& path : unit->fields.at("paths")->items)
        ownership[id].push_back(path->string);
    }
    std::vector<value_ptr> units;
    for (const auto& id : ids) {
      auto found = ownership.find(id);
      if (found == ownership.end())
        return std::unexpected(diagnostic{"internal", "Invalid embedded command protocol"});
      std::vector<value_ptr> paths, entries;
      for (const auto& path : found->second)
        paths.push_back(text(path));
      for (const auto& entry : current_entries)
        if (std::ranges::any_of(found->second, [&](const auto& p) { return contains(p, entry->fields.at("path")->string); }))
          entries.push_back(entry);
      bool present = !entries.empty();
      auto output  = hash_value("unit_output_v1", object({{"work", text(id)},
                                                          {"root", text(root_id)},
                                                          {"base", text(*baseline)},
                                                          {"paths", array(std::move(paths))},
                                                          {"entries", array(std::move(entries))}}));
      if (!output)
        return std::unexpected(output.error());
      units.push_back(object({{"work", text(id)}, {"present", boolean(present)}, {"output", text(*output)}}));
    }
    std::ranges::sort(units,
                      [](const auto& a, const auto& b) { return a->fields.at("work")->string < b->fields.at("work")->string; });
    return object({{"status", text("captured")}, {"units", array(std::move(units))}});
  } catch (const std::bad_alloc&) {
    return std::unexpected(diagnostic{"resource_limit", "Native conversion allocation exceeded"});
  } catch (const std::out_of_range&) {
    return std::unexpected(diagnostic{"internal", "Invalid embedded command protocol"});
  }
}
} // namespace planar::engine::execution
