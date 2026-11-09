// Test-only read-only snapshot projection probe for real CLI ownership workflows.
import std;
import planar.engine.ledger.execution;
int main(int argc, char** argv) {
  if (argc != 2)
    return 2;
  auto snapshot = planar::engine::ledger::read_execution_snapshot(argv[1]);
  if (!snapshot)
    return 3;
  for (const auto& observed : snapshot->observed)
    std::println("observed {} {} {}", observed.work, observed.output, observed.seq);
  for (const auto& completion : snapshot->completions)
    for (const auto& unit : completion.units)
      std::println("completion {} {} {}", completion.role, unit.work, unit.output);
  return 0;
}
