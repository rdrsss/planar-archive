# C10 verification invariants

| ID | Required test / record | Fault or regression distinguished |
|---|---|---|
| I1 | Header checks accepted supported3 before corrupting to4; header-only4 refusal unsupported_version. Existing format3 schema verification preserved. | Remove unsupported-header whitelist / accept4; old false premise3unsupported removed. |
| I2 | All14 exact original names/order/authored bytes, newline separator, complete concatenation and provenance-tree roundtrip. | Omit/reorder/change any original; accepting mere count insufficient. |
| I3 | Historical golden hashes unchanged, canonical/JSON agreement preserved; current full JSON/canonical expectation differs only recomputed14-file stdlib digest; decoder roundtrip unchanged. | Wrong current provenance digest or arbitrary semantic change; no stripping version/bundle/semantic fields. |
| I4 | Full bootstrap_load workflow exact format3 no-job history state, user_version3, unchanged request_v2 marker, history/replay/export after source deletion and alternate retained policy. | Drop active_job/activation/roles/runs, wrong user_version/framing; old R14 behavior protected by explicit format1/2 native tests. |
| I5 | Current exact shipped17 positive and altered writer negative remain. | Wrong current command count or bypass closure audit. |
| I6 | Attachment-only None and existing other packet with unchanged result rejected via status/replay/history, both compositions; original replay still exact. | Disable dispatch_metadata attachment comparison while other record/result guards remain valid. |
| I7 | Current docs state format3, delivered workspace/load and execution features, source approval vs incomplete integrated acceptance; matrix cites actual scoped evidence and pending owners. | Stale missing-API claims or false completion/R19 certification. |
| I8 | Compact records reproduce all31 rows per config, all5 column peaks/margins/exclusions and lower-model claims, from raw JSON with hashes. | Omit refusal other-column peaks or hide exclusions; hand-entered/rounded margins not authority. |
| I9 | Final source identity exact or explicit independently approved test-dependency-only attribution after C10; whole-src mismatch disclosed. | Reuse old whole-src identity as current despite changed tests. |
| I10 | Actual D28 oldpair outcome recorded verbatim; any alternative checker proof labelled differently and scoped amendment/limit approved explicitly. | Claim prescribed unit red without executed failing CHECK. |

Focused test commands must match nonzero counts with zero skips. Full host/Linux gate reruns belong coordinator. Existing gate failures are red evidence; no search build/compile ran. Attachment source mutation is independent prober-owned.
