// Controlled native allocator and inode-read probes against copied production objects.
#include <cstdlib>
#include <fcntl.h>
#include <new>
#include <sys/stat.h>
namespace {
thread_local bool        watching_new = false;
thread_local std::size_t requests = 0, fault_one = 0, fault_two = 0, fault_three = 0;
} // namespace
void* operator new(std::size_t bytes) {
  if (watching_new) {
    ++requests;
    if (requests == fault_one || requests == fault_two || requests == fault_three)
      throw std::bad_alloc{};
  }
  if (auto result = std::malloc(bytes ? bytes : 1))
    return result;
  throw std::bad_alloc{};
}
void operator delete(void* pointer) noexcept {
  std::free(pointer);
}
void* operator new[](std::size_t bytes) {
  return ::operator new(bytes);
}
void operator delete[](void* pointer) noexcept {
  ::operator delete(pointer);
}
extern "C" void          planar_admission_watch(std::uint64_t, std::uint64_t);
extern "C" std::uint64_t planar_admission_read_attempts();
static_assert(sizeof(std::pair<std::string_view, unsigned char>) == 24);
TEST_CASE("C3 B2 private admission actual one-shot allocator failures remain terminal", "[c3-b2-private]") {
  extension_fixture fixture;
  auto              options = fixture.role(true);
  auto              healthy = fixture.owner();
  requests                  = 0;
  watching_new              = true;
  auto result               = healthy.admit("inspect", options, fixture.cwd);
  watching_new              = false;
  REQUIRE(result);
  const auto count = requests;
  REQUIRE(count > 10);
  for (std::size_t position = 1; position <= count; ++position) {
    auto owner   = fixture.owner();
    requests     = 0;
    fault_one    = position;
    watching_new = true;
    auto refusal = owner.admit("inspect", options, fixture.cwd);
    watching_new = false;
    fault_one    = 0;
    INFO(position);
    REQUIRE_FALSE(refusal);
    CHECK((refusal.error().detail.code == "resource_limit" || refusal.error().detail.code == "capture_changed"));
    auto moved = std::move(owner);
    extension_closed(moved, refusal.error());
  }
  std::println("one-shot admission allocations sampled: {}", count);
}
TEST_CASE("C3 B2 private published preparation error survives a second return-copy fault", "[c3-b2-private]") {
  extension_fixture fixture("history_read_v1");
  std::array        options{extension_parsed_option{"ledger", true, fixture.ledger}};
  auto              original = fixture.owner();
  requests                   = 0;
  fault_one                  = 1;
  watching_new               = true;
  auto first                 = original.admit("inspect", options, fixture.cwd);
  watching_new               = false;
  fault_one                  = 0;
  REQUIRE_FALSE(first);
  REQUIRE(first.error().field == "catalog.commands");
  const auto count = requests;
  REQUIRE(count >= 5);
  for (std::size_t second = 2; second <= count; ++second) {
    auto owner   = fixture.owner();
    requests     = 0;
    fault_one    = 1;
    fault_two    = second;
    watching_new = true;
    auto refusal = owner.admit("inspect", options, fixture.cwd);
    watching_new = false;
    fault_one = fault_two = 0;
    INFO(second);
    REQUIRE_FALSE(refusal);
    // Refusals before diagnostic publication may map to profile; after publication
    // the original catalog.commands diagnostic must win. Known final copy is last.
    if (second + 1 >= count)
      extension_same_error(refusal.error(), first.error());
    extension_closed(owner, refusal.error());
  }
  // The inner return-copy failure at4 is retried by C3-A's outer adapter.
  // Its request5 builds a redundant diagnostic after the first error published;
  // failing there reaches the wrapper's real outer resource adapter.
  {
    auto owner   = fixture.owner();
    requests     = 0;
    fault_one    = 1;
    fault_two    = 4;
    fault_three  = 5;
    watching_new = true;
    auto refusal = owner.admit("inspect", options, fixture.cwd);
    watching_new = false;
    fault_one = fault_two = fault_three = 0;
    REQUIRE_FALSE(refusal);
    extension_same_error(refusal.error(), first.error());
    extension_closed(owner, refusal.error());
  }
  std::println("double-fault first-error sequence requests: {}", count);
}
TEST_CASE("C3 B2 private creation allocator refusal publishes no partial owner", "[c3-b2-private]") {
  extension_fixture fixture("");
  auto              open = [&] {
    auto capture = profile_capture::open((fixture.path / "trusted/profile.json").string());
    REQUIRE(capture);
    return std::move(*capture);
  };
  auto capture = open();
  requests     = 0;
  watching_new = true;
  auto result  = extension_invocation::create(std::move(capture));
  watching_new = false;
  REQUIRE(result);
  const auto count = requests;
  for (auto position : {std::size_t{1}, count / 2, count}) {
    auto input   = open();
    requests     = 0;
    fault_one    = position;
    watching_new = true;
    auto refusal = extension_invocation::create(std::move(input));
    watching_new = false;
    fault_one    = 0;
    INFO(position);
    REQUIRE_FALSE(refusal);
    CHECK(refusal.error().detail.code == "resource_limit");
  }
  std::println("creation allocation requests: {}; first/middle/final sampled", count);
}
TEST_CASE("C3 B2 private late file admission performs no observed data reads", "[c3-b2-private]") {
  extension_fixture fixture;
  for (auto selected : {"workspaces/demo/workspace.lua", "trusted/registry.json", "observations.json"}) {
    auto        owner = fixture.owner();
    struct stat metadata{};
    auto        file = fixture.path / selected;
    REQUIRE(::stat(file.c_str(), &metadata) == 0);
    planar_admission_watch(metadata.st_dev, metadata.st_ino);
    REQUIRE(owner.admit("inspect", fixture.role(true), fixture.cwd));
    REQUIRE(owner.verify());
    CHECK(planar_admission_read_attempts() == 0);
    int descriptor = ::open(file.c_str(), O_RDONLY | O_NOFOLLOW);
    REQUIRE(descriptor >= 0);
    char byte;
    CHECK(::read(descriptor, &byte, 1) == 1);
    ::close(descriptor);
    CHECK(planar_admission_read_attempts() == 1);
    planar_admission_watch(0, 0);
  }
  extension_fixture history("history_read_v1");
  auto              owner = history.owner();
  struct stat       metadata{};
  auto              file = history.path / "ledger.db";
  REQUIRE(::stat(file.c_str(), &metadata) == 0);
  planar_admission_watch(metadata.st_dev, metadata.st_ino);
  std::array options{extension_parsed_option{"ledger", true, history.ledger}};
  REQUIRE(owner.admit("inspect", options, history.cwd));
  REQUIRE(owner.verify());
  CHECK(planar_admission_read_attempts() == 0);
  int descriptor = ::open(file.c_str(), O_RDONLY | O_NOFOLLOW);
  REQUIRE(descriptor >= 0);
  char byte;
  CHECK(::read(descriptor, &byte, 1) == 1);
  ::close(descriptor);
  CHECK(planar_admission_read_attempts() == 1);
  planar_admission_watch(0, 0);
}
