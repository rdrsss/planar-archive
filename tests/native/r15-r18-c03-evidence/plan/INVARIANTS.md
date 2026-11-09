# C03 invariant table

| ID | Invariant | Faithful mutant | Regression |
| --- | --- | --- | --- |
| N5a | Own and boundary evidence vector names its own work | omit membership comparison | two-unit CLI swaps unit vector to other existing vector; status and replay ledger_corrupt |
| N5b | output_vector registry check contains exactly its consuming unit’s complete-phase producers, including grouped internal producers | omit producer set from expected membership | native result vectors omit/add producer; writer refuses atomically; historical corruption reader refuses |
| N5c | Every vector output equals that reservation observation output | skip output comparison | native result uses valid different output digest; refusal and unchanged rows |
| N5d | Observation units are exact sorted own+producer set | bypass observation membership/order check | missing/extra/duplicate unit refused; own-only and producer-inclusive valid cases accepted |
| N6 | Every complete_check mode checks captured ledger identity under lock | guard identity only when completion | evidence-only call with altered snapshot identity returns path_denied unchanged; stale revision/fact same identity succeeds |
| PR3a | Both release reasons unblock reviewer after restored worker becomes done | omit checks.rederive on release | real CLI complete/edit/claim/restore/release; reviewer ready and claim succeeds |
| PR3b | Release target transition remains first and carries matching release reason; dependent transitions use lawful derivation | permit arbitrary target ordering/reason | native malformed release vector/transition cases refused; lawful extra transition round-trips |
| PR3c | Release derived transitions do not bump fact_version | classify release as fact-changing | CLI before/after roles preserve fact versions and request replay exact |

| N5e | Per-work plan vectors retain grouped internal producer and exclude unrelated consumer dependencies | restore shared external producer set | actual-Lua plan fixture with a→b internal and z→c external expects b=[a,b], c=[c,z]; native grouped workflow with integration passes after fix |

Approved render-contract.md:211–213 and observations.lua:40–43 define complete-phase dependency producers; start/facet edges do not enter the output vector. R17 ambiguous slash wording is clarified accordingly, without broadening approved semantics.

| N7 | Facet directory records retain exact tree_edges atomically | replace intern_codec1 with raw intern for publication directories | native nonempty nested facet publication test and grouped/split CLI checks pass, historical verifier validates payload edges |
