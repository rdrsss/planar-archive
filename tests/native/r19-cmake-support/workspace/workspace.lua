local p = planar
local L = p.list
return p.workspace {
  version = 1, id = "engine_cmake",
  root = { id = "source", base = "baseline_58128234", excluded = L {} },
  spec = "spec.lua",
  jobs = L { { id = "combined", file = "combined.lua" } },
  registry = { id = "engine_cmake_tools", revision = "v1" },
  toolchains = L { "python" }, checks = L { "cmake_debug", "cmake_release" }
}
