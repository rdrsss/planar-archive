# C03 bounded mapping and decisions

| Decision | Authority | Implementation location |
| --- | --- | --- |
| Evidence vector includes exact own target, plus the consuming work’s complete-phase dependency producers (including grouped internal producers) only for output_vector checks; boundary remains own singleton | R17-1 output_vector_v1; R17-5 boundary exception; approved render-contract.md:211–213 | execution.cpp verify_check_results |
| Outputs match retained reservation observation, whose units are exact own plus the union of required external producer sets, sorted and unique | R17-1 check_observation_v1 and R17-4 capture | execution.cpp verify_check_results |
| Evidence-only completion checks ledger identity but retains weaker revision/fact preconditions | EX-3 step 6; C02 N6 | execution.cpp complete_check/check_snapshot_locked helper |
| Release rederives dependent states after target becomes inactive, preserving target-first transition and reasons, no fact bumps | R18-1.1 actual done workers; scoped correction to obsolete R16-4 and cross-row wording, requested by coordinator | commands.lua recovery_mutation; execution.cpp release reader/writer guards; execution-contract-r16.md |
| No new public surface or snapshot field | Existing retained W, observation and transition structure suffice | unchanged APIs |

N5 source map (revised after independent plan review): declarations.md applicability is keyed by work ID (296–302); output_vector means exact own plus consuming unit complete-phase producers (352–355), and grouping retains internal obligations. Existing checks.plan incorrectly uses a shared external producer set for each check. Correct its vector construction to inspect dependencies with consumer==e.work, including internal producers; keep the union of external producers for capture/observation. Native verifier derives the same per-consumer membership from retained W, while observation membership is all owned units plus the union of external vector producers. Boundary rows stay own-only by R17-5. Valid absent-completion producers still have captured output and remain in vectors; evidence-only results are verified by the same shape/membership rules without requiring completion applicability.

N6 source map: complete_check has check_snapshot_locked only inside completion branch. Split identity-only comparison from head/fact comparison; call identity unconditionally before token/run mutation. Keep full snapshot check for completion mode.

PR3 source map: recovery_mutation derives target state and target transition only. checks.rederive already projects explicit transitions then derives dependent state without changing original snapshot; invoke for release. Native validate_change and replay_events each require transitions.size()==1; allow target plus ordinary derived transitions while retaining existing apply_transitions state/reason grammar, same active job and unchanged fact versions. Detailed additional target/order checks will be tested.

The R16 amendment explicitly also overrides EX-4’s carrying-event list and M20–M22 event applicability for derived transitions on run_released/run_interrupted, and updates R16 native API “one transition” wording at its implementation section. No state/reason grammar is broadened except allowing these carrying events; target release transition remains first.

Approved render-contract.md:211–213 and observations.lua:40–43 define complete-phase dependency producers; start/facet edges do not enter the output vector. R17 ambiguous slash wording is clarified accordingly, without broadening approved semantics.

Observation external producer union is drawn only from actual effective checks requesting output_vector, not all W dependencies. Every role-owned unit remains observed. A complete-contract dependency without output_vector does not add a captured producer. Parent grants R17 wording clarification consistent with higher-priority approved render/declarations.
