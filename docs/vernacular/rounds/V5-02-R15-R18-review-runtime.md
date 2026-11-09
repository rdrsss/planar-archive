# R15–R18 runtime integration review

Disposition: **request-changes**. Reviewed candidate `ae02a12bf77c6b72d3060253dd47800c645796a7` against base `1338637c`. This is a bounded independent review, not full acceptance. Product/source files were read-only; no build, commit, installed binary, legacy tool or real operator database was used.

## Findings

### RT1 — P2: Preserve evidence and findings when adapting recovery records

Location: `src/cmd/planar/invocation.cpp:1135-1137`.

`read_recovery()` reads the native `recovery_records`, but constructs its service response with literal empty `evidence` and `findings` arrays. The ledger reader already fills these members. Thus a worker recovering after a failed check sees `evidence:[]` and the false statement “No check evidence is recorded in this lineage.” The same adapter necessarily drops carried review findings after rejection, hiding the repair instructions from recovery.

Contract: execution-contract-r16.md R16-5 requires these to be the read-recovery values and explicitly assigns population to R17/R18; execution-contract-r17.md R17-7 defines nonempty evidence, and execution-contract-r18.md R18-1 defines carried findings.

Evidence: **CLI reproduced for evidence; findings consequence source-proven, not independently exercised.** An isolated worker check recorded four evidence rows (two failed registry checks and two passing boundary gates). The subsequent successful `recover --role builder` returned `evidence:[]`. Fix the adapter to forward both populated members and add a real check/recover and reject/recover regression.

### RT2 — P2: Resolve open reservations in status request lookup

Location: `src/cmd/planar/invocation.cpp:1218-1225`.

The `status --request-id` service queries only `requests` and immediately returns `request_not_found` if absent. A check reserves its request ID before execution but writes the request row only at completion or explicit check recovery. Therefore status reports a running or interrupted reserved check as nonexistent. Drivers lose the promised request-based recovery discovery, precisely when a check has stopped before recording its result.

Contract: execution-contract-r17.md R17-9 / AM-4 requires `{request_id,operation:"check",outcome:"in_progress",reservation,run:{id,generation}}` for an open reservation, without `result`.

Evidence: **CLI reproduced.** Each registry check slept for two seconds; a status lookup 0.5 seconds after launching the check returned exit 3, code `request_not_found`, message `Recorded history view is unavailable`. The check then finished successfully as a recorded failure with four evidence rows, confirming that this was a real executing reservation, not a setup failure. Query the open-reservation path before returning not-found and cover both active execution and an abandoned reservation.

### RT3 — P2: Restrict the implicit .git exclusion to directories

Location: `src/engine/capture/tree_digest.cpp:75-78` (before the type check at 90-94).

The new digest capture skips every entry named `.git` before `fstatat`. Existing render capture skips only `.git` directories (`src/engine/capture/native.cpp:429`). A regular `.git` file therefore disappears from check identities and boundary comparison; a `.git` symlink or special file bypasses the required refusal. A changed Git-worktree pointer file outside owned paths is invisible to the gate, and check identities differ from render/recovery identities on the same tree.

Contract: execution-contract-r17.md R17-4 requires render capture's mode, kinds and enumeration, lists `.git/` as the implicit exclusion, and reuses its symlink/special-file refusals. render-contract.md capture rejects symlinks and special files.

Evidence: **CLI reproduced for a symlink; regular-file identity consequence source-proven.** Added only `root/.git -> missing-target` to the previously checked root, with no explicit exclusion for it, then ran a fresh check. Exit was 0 with both `planar.boundary_v1` rows `pass`; no `path_denied` refusal occurred. Move the implicit exclusion after no-follow metadata inspection and apply it only to directories; compare digest capture with render for regular `.git` files and reject `.git` links.

## Executed checks and reproducibility

Used existing source development binary `build/rewrite-debug/bin/planar`; no rebuild means its exact artifact-to-commit identity was not independently established. All three observations match the candidate source paths above. Commands executed via Python `subprocess` against a fresh scratch ledger:

1. `init --workspace-id quota`; `load` a scratch copy of the quota workspace/registry; `activate --job combined --expect-revision 1 --expect-job NONE`; `claim --job combined --role builder`.
2. `check --run <returned token> --root <scratch>/root --base-root <scratch>/base --scratch <scratch>/scratch --request-id check --actor review`, concurrently with `status --request-id check`.
3. `recover --role builder` after completion.
4. Add `.git` symlink under the scratch root; repeat check using fresh ID `check-git-link`.

Fixture modifications were scratch-only: root and base both copied from quota `current`; spec contracts and dependencies replaced with empty lists; registry checks used `inputs:["own_output"]`, `selection:"not_applicable"`, toolchain `[/bin/sh,-c]`, empty environment, and argv `[/bin/sleep 2; exit 1]`. This intentionally records failed checks without ending the worker run. Initial setup attempts refused a symlinked `/var` spelling and invalid simplified declarations; these were corrected before the reported successful lifecycle and are not counted as passing checks.

Scratch evidence remains at `<private-temp-root>/planar-runtime-review-gytiemxc/`: `status.json`, `check.json`, `recover.json`, `git-link-check.json`, workspace, registry, roots and isolated ledger. No token was copied into this report. Two actual check invocations each recorded four evidence rows. No Catch2 suite/filter was run; matched formal test count is not claimed, and no skips were used as success.

## Scope limits

Read the repository guidance, implementation/roadmap and v5/execution contracts, and inspected command invocation/services, runtime check-stage admission, recovery capture, check adapter, digest capture and process runner/resolver. This report does not approve every path inspected. Full external-profile adversarial testing, allocator and race injection, Linux execution, exhaustive CLI grammar, process isolation tests and full gates remain unperformed. Ledger invariants and Lua policy were assigned to other reviewers and are not independently approved here.
