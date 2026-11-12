local p = planar
local L = p.list
return p.workspace {
  version = 1, id = "fault_mini",
  root = { id = "source", base = "baseline_mini", excluded = L {} },
  spec = "spec.lua",
  jobs = L { { id = "combined", file = "combined.lua" } },
  registry = { id = "fault_tools", revision = "v1" },
  toolchains = L { "python" }, checks = L { "sleep" }
}
