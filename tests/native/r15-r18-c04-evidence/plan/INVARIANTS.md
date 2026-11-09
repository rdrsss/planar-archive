# C04 invariants and faithful faults

| Invariant | Required regression / faithful fault |
| --- | --- |
| Empty profile creates/loads format3 and binds execution | Fresh profile init/load/activate/claim/check/verdict; both grouped and split fixture behavior; reject embedded or changed-profile mutation until fresh load. Existing 3-scenario vectors CLI supplies a strong lifecycle. |
| Saved profile load replay precedes capture | Complete load, remove/alter profile and declaration inputs, repeat exact request: saved bytes; changed parameters same ID: conflict. Fault: move lookup after profile capture. |
| Effective provenance survives | Inspect stored family/effective tree/profile/module bytes and export accepted source; nonempty extension profile works, not just empty profile. Fault: use embedded provenance profile. |
| Profile load reconciles active roles | Active claimed worker, material profile/declaration load, then drift/refresh behavior; exact no-op and immaterial load preserve appropriate state. Fault: omit reconcile or use unrelated snapshot. |
| Identity and CAS remain atomic | Competing acceptance and replaced ledger refuse without partial revision/transitions/request; receipt change before commit aborts. Fault: skip identity, expected revision or original native receipt check. |
| Single owner/frame and bounds | Registration remains once; native/instruction counters retain cumulative values; cleanup succeeds/refuses as existing tests require; no call after preparation. Fault: second VM/factory or reopened frame. |
| History event is actual expansion | Profile init then history --event1 has selector event; after claim, expansion has retained work/manifest/packet and no token/secret. Fault: omit event selector or ID mapping. |
| Format2 compatibility is retained where explicitly contracted | Existing format2 profile acceptance/replay/receipts pass; format1 remains refused. This is supported new-engine ledger formats, not legacy Planar migration. |


| Late replay invariant | Regression and faithful fault |
| --- | --- |
| Concurrent same-ID equal winner is replayed before receipt/CAS checks and has no new completion effects | Prepare two equal requests; winner commits; mutate second prepared inputs; second returns exact saved bytes despite stale head/receipts. Different body conflicts before CAS. Fault: verify receipts before checking staged replay. |
| Load never internally reenters preparation or retries a consumed owner | Force CAS loss; return its typed refusal without another factory/frame or Lua call. Fault: restart preparation on state/revision conflict. |
