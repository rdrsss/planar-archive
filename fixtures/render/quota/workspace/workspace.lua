local p = planar
local L = p.list
return p.workspace {
  version = 1, id = "quota",
  root = { id = "source", base = "initial", excluded = L { "generated/" } },
  spec = "spec.lua",
  jobs = L { { id = "combined", file = "combined.lua" }, { id = "split", file = "split.lua" } },
  registry = { id = "quota_tools", revision = "v1" },
  toolchains = L { "python" }, checks = L { "contract", "integration" },
  notes = p.immaterial(p.md "The driver supplies stable sibling source snapshots.")
}
