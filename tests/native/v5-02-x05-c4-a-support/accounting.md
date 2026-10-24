# C4-A ownership and resource boundary

The same `extension_invocation` owns original `profile_capture`, privately prepared
C3-A scalars and its internally registered VM. It exposes one `execute()`; no
caller-constructed invocation, generic handler callback or read provider enters.
All capability-bearing commands refuse before F. The actual retained function
gets exactly invocation and empty context; both are children of one temporary
frozen transport root. Existing strong backing/tag roots survive raw inspection,
owned conversion and codec sizing, then existing cleanup removes temporary roots.

The first external command retains index6 after five shipped entries. Selection
walks original `external_entries` filtering command kind; source owner ordinal and
canonical catalog order never determine the diagnostic. Before selection the
staged fallback is `catalog.commands`. First native failure publishes before
fallible return copies. The owner adopts the VM's original first failure and moves
it with the original receipts. A permitted inner E consumes execution without
becoming a native failure. A second attempt poisons rather than reentering F.

Raw response inspection runs inside `lua_pcall` with trivial borrowed views and
indices only, using approved raw proxy backing lookup. Closed schemas bound
structural nesting independently of caller graph depth. Lists have no new quota:
raw membership is checked before indexed traversal. Exact fields and primitive
strings/numbers are validated before conversion allocates their owned copies.
Only lists explicitly tagged through the existing array constructor are lists.
Unknown metatables, userdata, callables, cycles and unsupported constructors cannot
match this closed schema. Existing `convert` additionally enforces 256 depth,
4M cumulative nodes, signed32 integers and cumulative512MiB native allocation.
Strings/proxies remain rooted throughout every raw and conversion pass.

The new invocation staging reserves before construction: six record/value owners
at `sizeof(value)+512` each (the existing conversion per-node conservative amount
covers the pinned shared-owner block), twelve fixed map nodes/keys at256 each,
two map nodes and key buffers per option at `2*(256+keybytes+1)`, each scalar
node through existing `visit`512, and every copied string byte plus terminator.
Command names separately reserve before assignment. Two options map nodes/keys
cover options and present; present boolean construction gets its own node charge.
The fixed slot allowance covers the four invocation and two outer transport keys;
other fixed reservations intentionally overlap. The existing push/freeze/convert
passes retain their separate cumulative reservations; no refunds or resets occur.

The original owner factory additionally charges64 for its staged command fallback;
execution stages64 for the fallback and128 before formatting the selected index
(maximum64). These cover pinned libc++ string storage and formatted scratch. This
is conservative logical native accounting, not RSS. No custom allocator/extra
quota or public API changes are introduced.

`begin_invocation` remains the existing one-time registration-to-invocation
instruction transition. It is called once after input staging, before F. No new
counter reset is added; native/node/audit counters stay cumulative. Final reserved
Glaze encoding charges its complete measured scratch/output reservation before
allocating output and applies the existing16MiB complete response limit. The
encoding is discarded; native-owned response survives cleanup. Original capture
verification runs before F and again after conversion/encoding for either allowed
branch. Later stdout publication seals and shared capture/VM invocation accounting
remain X06 work.

## Shared owner layout

`state` gains no member. `command_state` gains only `extension_execution`, one
boolean beside the existing invocation/baseline booleans before aligned string
members. The pinned-target private layout variant reconstructs the exact previous
command-state declaration by removing that bool, then statically checks equal
sizeof AND alignof for both command state and the complete command VM owner.
Both embedded and selected-profile positive controls execute that variant. The
new flag occupies previously reserved alignment padding, so neither shared owner
allocation grows; no additional shared-owner byte charge is required. The existing
B4 root-metadata delta reservation is unchanged for both owner kinds. The separate
extension owner does gain `executed`, already covered by its complete sizeof
reservation. This is pinned-target evidence, not a Linux ABI claim.
