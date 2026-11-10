local p = planar
local L = p.list
return p.spec {
  version = 1, id = "engine_cmake",
  goal = p.md "Give every engine directory its own CMakeLists.txt and compose the existing module targets with add_subdirectory.",
  work = L {
    { id = "engine_build", summary = p.md "Move existing target declarations into directory-owned CMake files without changing product behavior.",
      paths = L {
        "CMakeLists.txt", "acceptance/CMakeLists.txt", "budget/CMakeLists.txt",
        "canonical/CMakeLists.txt", "capture/CMakeLists.txt", "execution/CMakeLists.txt",
        "export/CMakeLists.txt", "hash/CMakeLists.txt", "history/CMakeLists.txt",
        "json_codec/CMakeLists.txt", "ledger/CMakeLists.txt", "lua/CMakeLists.txt",
        "process/CMakeLists.txt", "runtime/CMakeLists.txt",
        "runtime/closure_guard/CMakeLists.txt", "value/CMakeLists.txt"
      }, checks = L { "cmake_debug", "cmake_release" } }
  },
  milestones = L {
    { id = "complete", work = L { "engine_build" }, acceptance = L {
      { id = "directory_ownership", text = p.md "Every existing engine directory owns its CMake file and is added exactly once by its immediate parent." },
      { id = "preserved_graph", text = p.md "Preserve target names/types, source/module membership, dependency edges, compile settings and test-only exclusions; allow only exact owner-derived generated/artifact path rebases." },
      { id = "verified", text = p.md "Both host configurations pass the full registered test inventory with zero skips, formatting/Doxygen and a no-work repeat build. No C++ or Lua implementation changes." }
    } }
  },
  constraints = L {}, prescriptions = L {}, contracts = L {},
  facets = L {}, dependencies = L {}, frozen = L {}
}
