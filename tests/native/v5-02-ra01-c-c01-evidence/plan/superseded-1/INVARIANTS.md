# Invariant table, V5-02-RA01-C-C01 (search stage)

New test (all rows below): `RA01-C-C01 orchestrator projection wrapper is versioned and mandatory`
File: src/cmd/planar/authoring_orchestrator.t.cpp (append after the test at :614-637; reuse `c01_squashed_lua` and `c01_occurrences` at :591-603). Source pin limit: it observes orchestrator.lua text, not runtime values; no behavioural path exists (see DECISIONS D10).
Squashed source is whitespace-free. Expected squashed fragments (after the fix):
- helper: `sections[#sections+1]={id=id,text="##"..heading.."\n"..body,mandatory=true}` (written in squashed form with no spaces; `"## "` becomes `"##"`)
- return: `return{version=1,sections=sections}`
- consumer: `localresult=project(c)` then `ipairs(result.sections)`

| id | invariant | mutant (exact product edit) | test | testFile | expectRed | package |
|---|---|---|---|---|---|---|
| I1 | project() returns the wrapper with `version=1` exactly once as the return of project | orchestrator.lua:140 `return sections` -> `return {sections=sections}` (drop version) | RA01-C-C01 orchestrator projection wrapper is versioned and mandatory | src/cmd/planar/authoring_orchestrator.t.cpp | true | T1 (test), I1 (impl) |
| I2 | the version value is 1 | `return {version=1,sections=sections}` -> `version=2` | same | same | true | T1, I1 |
| I3 | every section carries `mandatory=true`: the helper literal contains `mandatory=true`, and `mandatory=` occurs exactly once in code (no `mandatory=false`, no second site) | drop `,mandatory=true` from the helper at orchestrator.lua:114; or set `mandatory=false` in the helper; or set `mandatory=false` on one call site by adding a key | same | same | true | T1, I1 |
| I4 | exactly five `section(` calls with ids authority, graph, roles, prescriptions, actions in that order (positions strictly increasing in the squashed source) | swap two `section(...)` calls (e.g. roles and graph), or delete one | same | same | true (pre-fix passes order but test is red overall on I1-I3; order SECTION asserts independently and is a guard of existing behaviour) | T1 |
| I5 | orchestrator_command_impl iterates `result.sections` of the wrapper, not the bare projection; `ipairs(project(c))` absent | orchestrator.lua:156 `ipairs(result.sections)` -> `ipairs(project(c))` (bypass wrapper) | same (pin) and existing `RA01-B2 orchestrator prompt carries five sections in order` (behavioural: the wrapper table has no array part, the prompt loses all sections) | same | true | T1, I1 |
| I6 | command output bytes unchanged | any change to join, title or section text | all existing orchestrator tests in authoring_orchestrator.t.cpp and authoring_orchestrator_variants.t.cpp, unchanged and passing | same | false | I1 (no test edit) |
| I7 | no existing test name removed or renamed; 1437 names at 93053c42 kept; new name listed | rename or delete any TEST_CASE | inventory comparison (gate), not a TEST_CASE | tests/native/v5-02-ra01-c-c01-evidence/ | false | gate |
| I8 | boundary files runtime.cpp, commands.lua, schema.lua, semantic.lua byte-identical to HEAD (or re-pinned); `X08 C01 role preparation outputs-off boundary pins the request-record conversion charge` still passes | edit any of the four | existing X08 test; identity.py per-file hashes | src/engine/runtime/external_registration.t.cpp | false | gate/record |
| I9 | records: A07 text (acceptance-matrix.md:29), rows C-15r (acceptance-rows.md:48) and RA01-C-RA01-C-F1 (:68) name the new test and gate; RR-A1..A3 already applied (rounds/V5-02-RA01-C.md:266) so recorded as no-op | n/a | n/a (review) | docs | false | D1 |
| I10 | F2-F8 untouched | n/a | n/a | n/a | false | none |

Note on the "mandatory=false on one section" mutant: because the flag is set centrally (DECISIONS D4), a one-section mutant is a call-site or helper edit; the pin requires `mandatory=` to appear exactly once and to equal `mandatory=true`, which kills both forms.
Note on the pin text for the section helper: the test should assert on three separate substrings (`mandatory=true}` within the helper, `c01_occurrences(src,"mandatory=")==1`, `c01_occurrences(src,"mandatory=false")==0`) rather than one long literal, so heading-format edits do not make it brittle.
