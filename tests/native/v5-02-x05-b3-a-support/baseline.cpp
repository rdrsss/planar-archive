// Exact old/current embedded semantic output comparison through production runtime.
#include "../../../src/engine/runtime/shapes.t.cpp"
#include <cstdlib>
TEST_CASE("X05 B3 A baseline byte comparison", "[baseline-compare]") {
  fixture f;
  std::vector<value_ptr> bundles{f.bundle,custom(f.bundle)};
  for(auto id:{"combined","split"}) {
    bundles.push_back(job_field(f.bundle,id,"roles",array_value()));
    bundles.push_back(job_field(f.bundle,id,"shape",string_value("missing")));
    bundles.push_back(role_field(f.bundle,id,"reviewer","checks",json(R"(["contract"] )")));
  }
  bundles.push_back(spec_field(f.bundle,"frozen",json(R"(["src/"])")));
  auto dependencies=f.bundle->fields.at("spec")->fields.at("value")->fields.at("dependencies");
  bundles.push_back(spec_field(f.bundle,"dependencies",array_value({dependencies->items[0],changed(dependencies->items[1],"quality",string_value("accepted"))})));
  std::ofstream stream(std::getenv("B3_BASELINE_OUTPUT"),std::ios::binary);
  REQUIRE(stream.good());
  for(auto bundle:bundles) {
    auto result=invoke("validate_bundle",record_value({{"bundle",bundle},{"capture",json(R"({"status":"not_captured"})")}}));
    stream<<*codec_encode(result)<<'\n';
  }
}
