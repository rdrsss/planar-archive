local p = planar
local L = p.list
return p.spec {
  version = 1, id = "fault_mini",
  goal = p.md "Exercise the interrupted check, takeover, rejection with repair and the max_rounds blocker on one unit.",
  work = L {
    { id = "unit", summary = p.md "Edit the single source file so each recheck produces a new output.",
      paths = L { "src/" }, checks = L { "sleep" } }
  },
  milestones = L {
    { id = "complete", work = L { "unit" }, acceptance = L {
      { id = "verified", text = p.md "The unit passes its registered check and an independent reviewer approves its exact output." }
    } }
  },
  constraints = L {}, prescriptions = L {}, contracts = L {},
  facets = L {}, dependencies = L {}, frozen = L {}
}
