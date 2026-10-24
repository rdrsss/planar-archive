local p = planar
local L = p.list
return p.job {
  version = 1, id = "split", shape = "fanout_review", budget_bytes = 131072,
  roles = L {
    { id = "library_worker", kind = "worker", work = L { "library" } },
    { id = "cli_worker", kind = "worker", work = L { "command" } },
    { id = "reviewer", kind = "reviewer", targets = L { "library", "command" } }
  }
}
