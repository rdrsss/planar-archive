// Draft comparison of a captured declaration closure with the recorded source tree (DD-D6).
module planar.engine.execution.drafts;

import std;

namespace planar::engine::execution {

std::vector<std::string> changed_paths(std::span<const history::tree_file> captured,
                                       std::span<const history::tree_file> recorded) {
  std::map<std::string_view, const history::tree_file*> before;
  for (const auto& file : recorded)
    before.emplace(file.path, &file);
  std::set<std::string> changed;
  for (const auto& file : captured) {
    auto found = before.find(file.path);
    if (found == before.end() || found->second->payload != file.payload || found->second->mode != file.mode)
      changed.insert(file.path);
    if (found != before.end())
      before.erase(found);
  }
  for (const auto& [path, _] : before)
    changed.emplace(path);
  // std::set<std::string> orders by char_traits<char>::compare, which compares as unsigned char.
  return {changed.begin(), changed.end()};
}

} // namespace planar::engine::execution
