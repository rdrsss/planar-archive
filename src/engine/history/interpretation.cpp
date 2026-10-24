module planar.engine.history.interpretation;
import planar.engine.runtime;
namespace planar::engine::history {
namespace {
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
} // namespace
std::expected<prepared_interpretation, interpretation_error> prepare_interpretation(const value_ptr& captured_bundle) {
  auto stdlib = prepare_embedded_stdlib();
  if (!stdlib)
    return std::unexpected(interpretation_error{stdlib.error().code == provenance_error_code::resource_limit
                                                    ? interpretation_error_code::resource_limit
                                                    : interpretation_error_code::runtime});
  auto result = run_trusted("interpret_bundle", captured_bundle);
  if (!result)
    return std::unexpected(interpretation_error{
        result.error().code == "resource_limit" ? interpretation_error_code::resource_limit : interpretation_error_code::runtime,
        result.error()});
  auto failure = at(*result, "error");
  if (failure)
    return std::unexpected(interpretation_error{interpretation_error_code::validation, {}, failure});
  auto interpreted = at(*result, "value");
  if (!keys(interpreted, {"bundle", "semantic"}))
    return std::unexpected(interpretation_error{interpretation_error_code::runtime});
  std::string digest;
  for (auto byte : stdlib->digest)
    digest += std::format("{:02x}", byte);
  auto checked = encode_interpretation(at(interpreted, "bundle"), at(interpreted, "semantic"), digest);
  if (!checked)
    return std::unexpected(checked.error());
  return prepared_interpretation{std::move(*checked), std::move(*stdlib)};
}
} // namespace planar::engine::history
