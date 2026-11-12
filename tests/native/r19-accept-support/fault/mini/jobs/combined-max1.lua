local p = planar
local L = p.list
return p.job {
  version = 1, id = "combined", shape = "fanout_review", max_rounds = 1, budget_bytes = 131072,
  roles = L {
    { id = "builder", kind = "worker", work = L { "unit" } },
    { id = "reviewer", kind = "reviewer", targets = L { "unit" } }
  }
}
