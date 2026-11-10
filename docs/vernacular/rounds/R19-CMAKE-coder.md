# R19-CMAKE implementation handoff

Implementation is ready for the coordinator's configured graph comparison and
actual Planar checks. This report grants no code approval. No configure, build,
test or product CLI was run by this coder; the coordinator withheld build lanes
while verification tooling was being authored. The prescribed Claude model is
unavailable, so a fresh ordinary agent implemented the bounded task.

The coder first read the actual retained Planar worker packet, then the round,
source map, approved plan and canonical decisions/invariants. Shared checkout
changes by the coordinator and verification agent were retained. Only the 16
owned engine CMake files, this report and ignored coder evidence were written.

## Result

The former 743-line engine file is a 20-line coordinator: shared embedding helper
include, required Python interpreter discovery, and 14 explicit immediate child
`add_subdirectory()` calls. Runtime adds its `closure_guard` child. The repository
retains its single top-level project; no nested project or helper change was added.

Each existing engine directory owns its target declarations and amendments.
Target names, interfaces, implementation/test membership, dependencies, test
timeouts, compile definitions including injected ceilings, fixture arguments and
existing target-level exclusions are preserved by extraction. Shared setup is
hoisted once so clean configuration does not rely on sibling/cache variables.

| CMake file relative to engine | Lines |
| --- | --- |
| `CMakeLists.txt` | 20 |
| `acceptance/CMakeLists.txt` | 29 |
| `budget/CMakeLists.txt` | 3 |
| `canonical/CMakeLists.txt` | 3 |
| `capture/CMakeLists.txt` | 32 |
| `execution/CMakeLists.txt` | 31 |
| `export/CMakeLists.txt` | 29 |
| `hash/CMakeLists.txt` | 6 |
| `history/CMakeLists.txt` | 95 |
| `json_codec/CMakeLists.txt` | 3 |
| `ledger/CMakeLists.txt` | 200 |
| `lua/CMakeLists.txt` | 14 |
| `process/CMakeLists.txt` | 5 |
| `runtime/CMakeLists.txt` | 282 |
| `runtime/closure_guard/CMakeLists.txt` | 3 |
| `value/CMakeLists.txt` | 2 |

## Extraction and path policy

`coder/EXTRACTION.json` binds the starting root file and explicit original line
ranges to each current owner. Every nonblank original line was assigned once or
was the embedding include/interpreter discovery hoisted into the coordinator.
The two formerly repeated interpreter discoveries become one inherited discovery.
No existing C++ or Lua source/test was edited.

Paths local to an owner lose that directory prefix; explicit
`CMAKE_CURRENT_SOURCE_DIR` generator/dependency paths likewise become owner-local.
Full generator/consume blocks remain together so variables do not cross siblings.
Runtime adds closure_guard before defining the runtime. Left-hand target
amendments remain with their target; existing named right-hand forward links
resolve at CMake generation as before.

Generated files, archives, shared libraries and test executables naturally move
from the engine binary root into their owning child binary directory. No output
root override or shared embedding-helper API was introduced. Catch2 discovery and
its default working directory also move to the owning child. The existing ledger
sections workflow retains its explicit repository working directory. The runtime
load-fixture macro follows its relocated generated directory. These exact
owner-derived rebases require independent configured comparison; no blanket
generated/source/definition waiver is intended. Historical replay scripts and
frozen evidence are unchanged and certify only their earlier layouts.

Owner details for generated/artifact normalization: loader/trusted C library in
`lua`; admission syscall library in `capture`; ledger private publication, record,
atomic and metadata families plus format2 fixture/snapshot probe in `ledger`;
all runtime embeddings, command/load variants, X05 audit and shapes bridge in
`runtime`; history fixture in `history`; export fault/posix family in `export`.
The remaining ordinary module and test targets follow their source directories.

## Executed static checks

`coder/STATIC-CHECKS.json` records actual read-only checks: 16 directory-owned CMake
files, all 15 descendants reached once through their immediate parent, no nested
project, 199 local source/generator references resolving, all 268 baseline
non-CMake source files unchanged, and `git diff --check -- src/engine` passing.
These checks do not replace actual configuration, graph/test inventory comparison
or the required full gates. No proposed invariant-table mutant was executed by
this coder.

The actual authoring trial's earlier interpretation mismatch remains outside this
refactor. No Planar state, driver code, verification tooling, product logic or
operator data was changed by this implementation agent.
