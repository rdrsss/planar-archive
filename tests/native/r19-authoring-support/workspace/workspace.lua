local p = planar
local L = p.list
return p.workspace {
  version = 1, id = "authoring_execution",
  root = { id = "source", base = "baseline_d6c989ed", excluded = L {} },
  spec = "spec.lua",
  jobs = L { { id = "repair", file = "repair.lua" } },
  registry = { id = "authoring_tools", revision = "v2" },
  toolchains = L { "python" }, checks = L { "host_debug", "host_release" }
}
