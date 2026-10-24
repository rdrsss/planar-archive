// Exact shipped-bundle compatibility and source-only retained-write positive control.
#include <catch2/catch_test_macros.hpp>
import std;
import planar.engine.x05_a0_0;
import planar.engine.x05_a0_1;
TEST_CASE("X05 A0 audited exact shipped five-handler bundle creates", "[x05-a0]") {
  auto vm = planar::engine::x05_a0_0::command_vm::create();
  REQUIRE(vm);
  REQUIRE(vm->catalog());
  CHECK(vm->catalog()->fields.at("commands")->items.size() == 5);
}
TEST_CASE("X05 A0 altered retained scalar writer bundle refuses", "[x05-a0]") {
  auto vm = planar::engine::x05_a0_1::command_vm::create();
  REQUIRE_FALSE(vm);
  CHECK(vm.error().code == "internal");
}
