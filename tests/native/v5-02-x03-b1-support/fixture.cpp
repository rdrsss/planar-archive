// Scratch-only native format2 fixture for genuine pre-B1 executable refusal checks.
import std;
import planar.engine.ledger.format;
int main(int argc, char** argv) {
  if (argc != 2)
    return 2;
  const auto result = planar::engine::ledger::initialize_new_v2(argv[1], "workspace");
  return result ? 0 : 1;
}
