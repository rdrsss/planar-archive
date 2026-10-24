local p = planar
local L = p.list
return p.spec {
  version = 1, id = "quota",
  goal = p.md "Provide an atomic in-memory quota and a small command-line client.",
  work = L {
    { id = "library", summary = p.md "Implement Quota.try_take and library tests.",
      paths = L { "src/core/", "tests/core/" }, checks = L { "contract" } },
    { id = "command", summary = p.md "Implement quota CLI and command tests.",
      paths = L { "src/cli/", "tests/cli/" }, checks = L { "integration" } }
  },
  milestones = L {
    { id = "api", work = L { "library" }, acceptance = L {
      { id = "atomic", text = p.md "A rejected take preserves the balance, including under concurrent calls." }
    } },
    { id = "cli", work = L { "command" }, acceptance = L {
      { id = "exit", text = p.md "Success exits 0; exhaustion exits 2; invalid arguments exit 64." }
    } }
  },
  constraints = L {
    { id = "safe", scope = "global", text = p.file "text/shared.md" }
  },
  prescriptions = L {
    { id = "library_rule", scope = L { "library" }, text = p.md "Serialize balance reads and updates with one instance lock; reject booleans as amounts." },
    { id = "command_rule", scope = L { "command" }, text = p.md "Write one remaining=<integer> line on success or exhaustion; write invalid arguments to stderr only." }
  },
  contracts = L {
    { id = "quota_api", producer = "library", consumers = L { "command" },
      expected = p.file "text/quota-api.md", conformance = "contract",
      review_note = p.md "Fixture author declares the interface reviewed for this example; no authenticated operator approval is asserted." }
  },
  facets = L {
    { id = "library_files", producer = "library", paths = L { "src/core/quota.py" } }
  },
  dependencies = L {
    { id = "develop", consumer = "command", source = "contract", ref = "quota_api", phase = "start", quality = "available" },
    { id = "integrate", consumer = "command", source = "facet", ref = "library_files", phase = "complete", quality = "done" }
  },
  frozen = L { "contracts/" },
  rationale = p.immaterial(p.md "A tiny in-memory quota makes grouping and exact-target review visible without an application framework.")
}
