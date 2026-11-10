local p = planar
local L = p.list
return p.job {
  version = 1, id = "combined", shape = "fanout_review", budget_bytes = 131072,
  roles = L {
    { id = "builder", kind = "worker", work = L { "engine_build" } },
    { id = "reviewer", kind = "reviewer", targets = L { "engine_build" } }
  }
}
