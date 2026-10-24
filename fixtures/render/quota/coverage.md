# Requirement and role coverage

Each cell describes mandatory context, not completed work. `B` is combined
builder; `L` split library_worker; `C` split cli_worker; `RC` combined reviewer;
`RS` split reviewer; `CR` split cli_worker recovery. Reviewer requirements apply
to targets without granting source ownership. Both jobs have one independent
reviewer with no worker assignment and no role-added checks.

| Declaration / obligation | B | L | C | RC | RS | CR |
| --- | --- | --- | --- | --- | --- | --- |
| goal | full | full | full | full | full | full |
| library summary, api:atomic | yes | yes | absent | yes | yes | absent |
| command summary, cli:exit | yes | absent | yes | yes | yes | yes |
| safe, exact mandatory text/shared.md bytes | yes | yes | yes | yes | yes | yes |
| library_rule | yes | yes | absent | yes | yes | absent |
| command_rule | yes | absent | yes | yes | yes | yes |
| quota_api expected bytes, review assertion, contract check | yes | producer | consumer | yes | yes | consumer |
| develop start/contract/available | internal | absent | cross-role | internal | cross-role | cross-role |
| integrate complete/facet/done | internal | absent | cross-role | internal | cross-role | cross-role |
| library_files policy/bytes | own + subscribed | own bytes, no consumer policy | subscribed | all targets | all targets | subscribed |
| library effective checks: contract | yes | yes | absent as own policy | yes | yes | absent as own policy |
| command effective checks: contract, integration | yes | absent | yes | yes | yes | yes |
| frozen contracts/, excluded generated/, read-only remainder | yes | yes | yes | yes | yes | yes |
| exact source content/base differences | both units | library only | command + quota.py facet | both units | both units | command + quota.py facet |
| exact unit output identities | both | library | command + upstream library | both targets | both targets | command + upstream library |
| assertion context | explicit absence | explicit absence | explicit absence | inconclusive + note | inconclusive + note | explicit absence |
| rationale, workspace note (optional) | yes | yes | yes | yes | yes | yes |
| checkpoint (optional session note) | absent | absent | absent | absent | absent | present |

Per-unit effective policy is independent of grouping: library `[contract]`;
command `[contract, integration]`. Contract conformance is mandatory for both
producer and consumer even though command.work.checks lists only integration.
Integration inputs are `[command=$O_COMMAND, library=$O_LIBRARY]`, sorted by work
ID; it applies only to command, never to the grouped library merely because the
builder's check-definition union contains integration. Contract tests require
expected_contracts and own_output. Their selected examples are partial coverage,
not authenticated review or an exhaustive conformance proof.

Ownership is exclusive even inside builder. Every job assigns library once and
command once, and targets each once. All boundaries (`src/core/`, `tests/core/`,
`src/cli/`, `tests/cli/`, `contracts/`, `generated/`) are pairwise disjoint segment
stems. Facet src/core/quota.py lies inside library's grant. All file references
are workspace-local; root mappings/registry/observations come only from the driver.

Paper phase trace: both units start independently using available contract text;
library.start → library.complete → command.complete → reviewer.start →
reviewer.complete → library.accepted is legal because there is no edge returning
from accepted to either start/complete. Each unit.complete also gates its owning
worker.complete; every worker.complete gates reviewer.start. In the combined job,
library.complete → command.complete stays a local unit obligation; no edge from
builder.complete to its own unit start/complete is added. Render leaves the done
quality unverified in every packet and manifest. No check/observation upgrades it.
