# C03 gate fixture map

Read-only source audit; no source/test changes and no builds. Only this report is
written. Shared checkout changes by other agents are preserved.

## Product defect found before changing expectations

The current source-built Debug CLI with a fresh, physically resolved temporary
`profile.json` containing `{"version":1,"modules":[]}`:

```
build/rewrite-debug/bin/planar with-profile --stdlib-profile PROFILE --help
```

returns exit **4**, empty stdout, and
`{"error":{"code":"internal","field":"","message":"Invalid external profile protocol"},"kind":"error","version":1}`.
The temporary profile is removed afterward; no ledger or installed binary is used.
An initial unresolved temporary path refused `path_denied`; the reproduced product
failure uses `Path.resolve()` and is distinct from that expected confinement rule.

The static path is `runtime.cpp:validate_profile_catalog` →
`history::validate_catalog`. `commands.lua:424-480` registers all 17 production
commands unconditionally; only the following six authoring rows are conditional.
`effective_catalog.cpp:216-221` recognizes only 14 production commands, and its
shipped branch at line 273 requires `native.contains(name)`. `check`,
`check-recover`, and `verdict` are missing. This is a product regression, not a
fixture-count issue. `history_signature` also lacks shipped `--event`, which was
added to the history descriptor. Keep the extension history capability signature
separate: adding `--event` indiscriminately to external capability signatures
would expand a different contract.

The effective validator needs the exact current shipped schemas, including:

- `check`: `check_v1`; handler `shipped.check.v1`; capabilities `check_run_v1`,
  `execution_commit_v1`, `provenance_read_v1`; eight options including optional
  provenance and required root/base-root/scratch.
- `check-recover`: handler **`shipped.check_recover.v1`**, response **`check_v1`**;
  execution commit capability; seven required options.
- `verdict`: handler `shipped.verdict.v1`, response **`verdict_response_v1`**;
  execution commit and verdict-read capabilities; five required options.
- Existing takeover remains `claim_v1`; do not replace explicit response handling
  with a universal `name + "_v1"` rule.

Authoritative shipped declaration details are in `commands.lua`, native
`runtime.cpp:bundle_table` and the R15/R16/R17/R18 command contracts. Add positive
empty-profile and profile-backed execution coverage before expecting full gates.

## Stale fixture families

| File / locations at audit | Disposition and bounded correction |
| --- | --- |
| `runtime/commands.t.cpp:12,785,968` | All use current generated source, not a frozen five-command snapshot. Expect 17. The R14-D6 generator replaces the load handler but retains the rest of current `commands.lua`; its five-entry comments/status text are stale. Preserve malformed-load and provenance semantics. |
| `runtime/external_registration.t.cpp:217` | Empty captured profile should retain 17 shipped rows. Native validator defect must be fixed first. |
| `history/effective_catalog.t.cpp:catalog()` | Hand-built fixture still supplies five commands. Update independent fixture to exact 17 shipped schemas, including history event and exceptional handler/response names. Preserve independent schema assertions. |
| `cmd/planar/authoring_catalog.t.cpp:339,398-399,424` | Production17, authoring23. Keep descriptor equality for every production row. Variant0 is current source; no frozen eleven-row ABI. Rename stale five/eleven/twelfth wording where appropriate. |
| `authoring_catalog.t.cpp:489` | Profiles intentionally exclude six authoring verbs, but retain all 17 production commands plus inspect. Exact expected names should have 18 entries, not six. Authoring isolation remains valid. |
| `bootstrap.t.cpp:3236 head_help()` | Literal is obsolete five-command help. Update independent expected text to current fixed root help, including history event and execution verbs. Do not replace expected text by calling production helper. Default and authoring bundle must still differ; profile must still exclude authoring commands. |
| `bootstrap.t.cpp:3415,3469` | Five-command comments/test names stale, but authoring tail refusal/profile capture precedence and equality between authoring/default profile composition remain valid tests. |

## Original registration indices and limit tests

`external_registration.t.cpp` embeds the old shipped count throughout original
index expectations. First external command is index18, second19, third20. At
lines333/336/426/430/3470/3768/5244/5259/5269/5299/5300/5307,
old `catalog.commands[6]` becomes `[18]`; second-entry cases at479/5248/5443
become19; preserve whether the assertion checks original or canonical ordering.
The C4 original-index test at3646 must change alpha7/beta8 to19/20.
Mixed kind/module formulas at467/549/572/624 use a shipped offset17, not5.
Do **not** apply the offset to shapes or projections.

Two-digit command-index tests currently target10 and64. Index10 now belongs to
a shipped row, so choose an external two-digit index such as20 and64 for commands;
retain10 and64 for shapes/projections. Preserve both positive registration and
negative original-index assertions.

Catalog maximum remains **64**, not76. The high-catalog fixture at485 currently
adds59 external commands, which would now exceed64; it can add47. Its >1,040,000
and <1,048,576 byte assertions must remain meaningful: redistribute help/option
payloads among valid entries to keep the original near-boundary purpose, rather
than lower/delete the byte assertion. The pure effective-catalog loops at476/509
likewise add only enough entries to reach64 from the new baseline; preserve exact
1 MiB success and plus-one refusal as well as node-shape coverage.

Source-size-sensitive budget tests around `external_registration.t.cpp:4575`
explicitly depend on embedded byte length. Re-measure only after product source
stabilizes, keep positive/negative controls and the approved narrow windows; do
not broaden resource ceilings just to pass fixtures.

## Aggregate initializer scan

The coordinator has already added the seventh optional `event` initializer to
all explicit `command_history_options` aggregates found in
`external_registration.t.cpp`, `profile_acceptance.t.cpp` and `bootstrap.t.cpp`.
The remaining occurrences are default construction, aliases, signatures or
member assignment. No additional missing initializer was identified in this
bounded textual scan. Compilation remains the authoritative check.

No full test result is claimed. The map distinguishes observed CLI failure from
source-predicted stale expectations. Fix product catalog admission first, then run
focused native catalog/profile tests before the integrated gate.

## Minimum corrective regression controls

Keep an independently assembled valid 17-command catalog and check each new row
against wrong handler, wrong response kind, missing/extra capability, missing
required option, extra option and wrong required/type metadata. In particular,
`shipped.check-recover.v1` must fail while `shipped.check_recover.v1` passes;
`check_recover_v1` must fail while `check_v1` passes; `verdict_v1` must fail while
`verdict_response_v1` passes. Check permits exactly its three capabilities, not an
arbitrary up-to-three set. An unmodified empty-profile VM and CLI selected help
must succeed; an external module with a normal extension must still register and
execute. External collisions with all new names/families/handler IDs must refuse,
and six authoring commands must remain excluded/reserved in profile mode.
