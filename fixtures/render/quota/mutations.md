# Mutation expectations (deferred native tests)

These are requirements for R05–R08, not executed results. Hold base and every
unnamed input fixed. W means the normalized material work object; digest changes
must later be computed natively. A/B below mean the compared before/after states,
not unrelated groupings. Render can change independently of W. Whole-tree
observation identity changes for any non-excluded source edit, even outside the
selected role; selected packet source content still remains scoped.

| Mutation | Required changed context / identity inputs | Required stable inputs / refusal |
| --- | --- | --- |
| Edit text/shared.md | B/L/C/RC/RS/CR requirements and W | Source unit outputs stable; shared text cannot be optional |
| Edit library_rule only | B/L/RC/RS W and requirements | C/CR W and packet unchanged, including check policy |
| Edit command_rule only | B/C/RC/RS/CR W | L W and packet stable |
| Edit quota-api.md exact bytes | All applicable contracts, semantic identity and W (all roles) | Source output identities stable; no authenticated approval appears |
| Wrap safe.text or expected in immaterial | schema refusal | Never silently omit mandatory text |
| Edit rationale or workspace notes | Applicable Markdown/render/optional counts | All W and source outputs stable |
| Edit recovery checkpoint only | CR checkpoint/render and captured observation-file provenance | CR W equals C W; same base/current/outputs; no history invented |
| Edit current/src/core/quota.py only | library output, builder/library_worker role outputs, facet semantic identity, global observation; reviewer targets; C/CR captured upstream content | All W stable: own output is excluded and integrate is completion-only; command output stable |
| Edit current/src/cli/main.py only | command output, builder/cli_worker role outputs, reviewer command target, global observation | All W and library output/facet stable; library packet's selected source content stable |
| Edit library test only | Library output and its role outputs, reviewer target, global observation | Facet bytes, command output and all W stable; C/CR do not display upstream library tests |
| Edit a base byte anywhere | Base identity, all W, observation and both unit outputs (base is identity input), relevant differences | Base label alone cannot preserve identity |
| Add/edit generated/log.txt | No captured tree/output/work change under explicit exclusion | Boundary remains material policy; exclusion must not grant write permission |
| Change integrate facet paths to another library-owned file | B/C/RC/RS/CR W.facets and dependency policy, even if bytes equal or missing | L W stable; do not rely solely on observed values or start_facets |
| Change integrate to start/facet/available | Split C/CR W.start_facets now includes captured library facet semantic value; subsequent upstream bytes move their W | Grouped builder own facets and reviewer target outputs never enter W.start_facets; replace command.work.checks with [contract] or retain another complete facet edge, otherwise schema refusal |
| Add a compatible contract check to library worker only | If adding a distinct allowed own-output check descriptor, library effective set/W and reviewer per-unit effective set change | Command check set stable; allowed registry/workspace updates required; same union cannot erase applicability |
| Add integration as builder role check | refusal: output_vector not applicable to library (no complete facet dependency) | Never spread command integration indiscriminately across grouped units |
| Change split to pipeline stages [library_worker],[cli_worker] | Derive pipeline:library:command start/done predecessor for C/CR and reviewers, producer/consumer role mapping in W/prerequisites | L has no incoming pipeline edge; explicit develop/integrate retained; contract-first command start is now delayed |
| Reverse pipeline stages | dependency_cycle via command.complete → library.start → library.complete → command.complete | Never scope away cycle before checking the full graph |
| Change integrate to complete/accepted | dependency_cycle: library.accepted → command.complete → worker.complete → reviewer.start → reviewer.complete → library.accepted, grouped or split | No reviewer barrier bypass; no accepted state inferred from note |
| Add reverse complete/done dependency | dependency_cycle if command.complete → library.complete closes existing edge | Both local/grouped and cross-role cycles refused |
| Omit command or assign it twice in a job | reference/assignment refusal | No uncovered unit allowed by role filtering |
| Omit a reviewer target, duplicate target, or make reviewer a worker | schema/reference/separation refusal | Every unit independently reviewed exactly once |
| Name unknown check/toolchain/role/work | reference refusal | No zero-test or silently dropped instruction success |
| Overlap work paths, e.g. src/core/ and src/core/quota.py | ownership refusal even grouped | Existing/absent source files cannot legalize overlap |
| Add frozen src/core/ or excluded src/ | ownership refusal across all boundary classes | Exclusion cannot conceal overlap |
| Add boundary x versus x/, x/y or x/y/ | ownership/topology refusal even if absent | x/ versus xy/ remains disjoint |
| Wrong reviewer output or missing integration vector member | observation_target_mismatch | Never attach stale assertions or report verified check success |
| Delete subscribed quota.py | Captured missing facet is unavailable; done quality remains unverified; output identity changes | No successful empty facet; context-only not_captured remains distinct |
| Shrink byte budget | Drop checkpoint, then job_notes if present, workspace_notes, rationale; report removal order | Mandatory source/assertions never truncate; mandatory overflow budget_exceeded |
| Reorder Lua record keys or normalized set entries | Provenance file bytes may change | Semantic W and packet stable; no digest comparisons claimed by R03 |
| Introduce symlink, hardlink alias, case alias or special file | Native capture refusal required | Syntax/manual checks here do not establish filesystem isolation |

Run each future mutation in an isolated copy, not by modifying the reviewed
baseline. A new check descriptor or declaration is a mutation input, not existing
fixture policy. Future integration must recompute exact identities, inspect both
changed and stable sets, and compare selected text/bytes rather than trusting
symbols or claimed manifests.
