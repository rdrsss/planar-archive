# C07 decisions

| ID | Decision | Authority |
|---|---|---|
| D1 | Validate the entire saved verdict response against retained original-event facts, not current head state or the saved response itself. Keep exact replay bytes unchanged. | R18-4 commit and replay; R18 response schema lines286–294; EX-3 reader cross-row invariants; C06 N1 reproduced. |
| D2 | Count is activation-local and computed through the verdict event; cap is the cap effective for that original verdict; approve/historical also carry valid count/cap. | R18-1 review count; R18-5 lines197–216; response schema. |
| D3 | Repair response generations are generation ordinals, not row IDs; opened/cleared blockers and role after-states are exactly those of this event, in canonical order. | R18 response schema; R18-1 repairs/blocker rows and R18-4 atomic commit. |
| D4 | Current observed output belongs to an own unit of its current owning role lineage, including failed/inconclusive observations. Consumed producer units never acquire observation authority through consumers. | R17-7 observed definition lines333–369; N-C06-2 native reproduction. |
| D5 | A successful producer completion replaces its entire current publication set, including absence. Other producers remain untouched; evidence-only check does not replace publications. | R17-1 publications row condition; R17-7 latest current completion; P-C06-1 reproduction. |
| D6 | Role identity in multi-job snapshots is job+role. Hypothetical maps must use composite keys or be explicitly restricted to the active job. | EX-3/EX-5 role/generation identity; activation rules; P-C06-2 switch-back reproduction. |
| D7 | Preserve historical completion vectors/payloads, history verification and replay, current-vs-applicable distinction, takeover lineage and existing W recipe. | R17-7, EX-5, prior reviewed exact contracts. |
| D8 | No schema, public API, product-limit or new product policy change is needed. Canonical serialization stays with existing codecs. | AGENTS and existing native/Lua contract boundaries. |
| D9 | Early successful replay must retain operation-specific stdout failure/retry guidance for run and recovery commands, in embedded and profile invocation. | EX-7 post-commit output failures, R16 retry rules; P-C06-3 closed-pipe reproduction. |
| D10 | A composed-budget allocator refusal is terminal even when Lua garbage collection retries allocation successfully; preserve the first diagnostic across later errors. Do not enlarge the big-Lua fixture to hide it. | profile-preparation-architecture.md397–405 and first-failure table460; unchanged X06 D C01 big-Lua guard, C06 calibration executed red. |

All choices are contract-decided or routine private implementation. No new operator choice identified. Plan review remains required.
