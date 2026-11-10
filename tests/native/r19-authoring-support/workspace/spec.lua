local p = planar
local L = p.list
return p.spec {
  version = 1, id = "authoring_execution",
  goal = p.md "Use one compiled authoring interpretation from load through execution, checks and independent review.",
  work = L {
    { id = "retained_vm", summary = p.md "Repair retained-VM dispatch and its typed generated compositions; preserve digest guards, replay and profile restrictions.",
      paths = L {
        "src/cmd/planar/CMakeLists.txt", "src/cmd/planar/bootstrap.cpp",
        "src/cmd/planar/handlers/activate/command.cppm",
        "src/cmd/planar/execution_options.cppm",
        "src/cmd/planar/invocation.cpp",
        "src/engine/runtime/runtime.cpp",
        "src/engine/runtime/runtime.cppm",
        "src/engine/runtime/commands.t.cpp", "src/engine/runtime/CMakeLists.txt",
        "tests/native/r19-authoring-c01-support/generate.py",
        "src/cmd/planar/handlers/status/command.cppm",
        "src/cmd/planar/handlers/preview/command.cppm",
        "src/cmd/planar/handlers/claim/command.cppm",
        "src/cmd/planar/handlers/takeover/command.cppm",
        "src/cmd/planar/handlers/refresh/command.cppm",
        "src/cmd/planar/handlers/checkpoint/command.cppm",
        "src/cmd/planar/handlers/release/command.cppm",
        "src/cmd/planar/handlers/recover/command.cppm",
        "src/cmd/planar/handlers/check/command.cppm",
        "src/cmd/planar/handlers/check-recover/command.cppm",
        "src/cmd/planar/handlers/verdict/command.cppm",
        "src/cmd/planar/handlers/claim/command.cpp",
        "src/cmd/planar/handlers/takeover/command.cpp",
        "src/cmd/planar/handlers/refresh/command.cpp",
        "src/cmd/planar/handlers/checkpoint/command.cpp",
        "src/cmd/planar/handlers/release/command.cpp",
        "src/cmd/planar/handlers/recover/command.cpp",
        "tests/native/v5-02-b2-support/generate.py", "tests/native/v5-02-b2-support/cli.py",
        "tests/native/v5-02-b2-support/audit-variants.py",
        "tests/native/r14-d7-b-support/generate.py", "tests/native/r14-d7-b-support/audit.py",
        "tests/native/v5-02-x07-support/generate.py",
        "tests/native/v5-02-x08-support/generate.py", "tests/native/v5-02-x08-support/x08.cmake",
        "tests/native/v5-02-x08-support/validate.py", "tests/acceptance/x08_e12.py",
        "tests/native/r19-authoring-support/lifecycle.py", "tests/native/r19-authoring-support/check.py",
        "tests/native/r19-authoring-support/wiring.py"
      }, checks = L { "host_debug", "host_release" } }
  },
  milestones = L {
    { id = "complete", work = L { "retained_vm" }, acceptance = L {
      { id = "coherent", text = p.md "All twelve execution families dispatch through the registered VM; an authoring-loaded revision reaches checked and independently accepted output without production fallback." },
      { id = "guards", text = p.md "Different interpretations still refuse fresh bound requests without writes; exact saved requests replay early. Production help and closed external-profile semantics remain unchanged." },
      { id = "verified", text = p.md "Reserve all cumulative native provenance copies before allocation; demonstrate measured native-budget quantity and terminal exhaustion. Typed source compositions retain reversible audits. Host Debug/Release inventories pass with zero skips, formatting/Doxygen and no target work on repetition." }
    } }
  },
  constraints = L {}, prescriptions = L {}, contracts = L {},
  facets = L {}, dependencies = L {}, frozen = L {}
}
