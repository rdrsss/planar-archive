# C3-A preparation boundary and accounting

The synchronous member selects the actual native retained kind0 command, whose descriptor
was catalog-validated and sealed during registration. It accepts borrowed records, not
argv and not a caller-created descriptor. Omitted records are absent. Explicit absent
records cannot carry a token; present flags cannot carry a token; present scalars must
carry one. Names omit `--`. The caller may supply records in any order; result options
follow sealed canonical descriptor order, including every absent option. Duplicate and
unknown names refuse. Required absence and malformed supplied tokens are usage.

Every result is native owned. Strings, signed32 integers and booleans use a variant;
host-bearing text under its matching capability instead contains a distinct slot-index
variant. Supplied host slots append in descriptor order. Absent host options contain an
empty optional index and allocate no selector slot. Indices address only that returned
result's native pending list. At future Lua materialization, an absent host option must be
omitted from options while its presence remains false; the native empty index is not
a Lua placeholder. A pending selector is neither an admitted capability nor a
receipt and conveys no authority to any service. The result is intentionally mutable
native data, not an authenticated later dispatch ticket. Future dispatch must bind actual
same-owner identity, capabilities, receipts and unchanged original intent; it cannot trust
an arbitrary caller-supplied prepared result as authority.

Pure same-spelled host text remains data. Preparation does no filesystem normalization,
path lookup, I/O, service call, Lua call or budget transition. No value_ptr or JSON graph is
created. History strings are range-checked through the existing positive INT64 grammar
and from_chars; general decimals remain signed32, with native narrower limit/budget
bounds. History query intent has effective page cursor0/limit50 defaults independent of
absent option values string0/integer0/false presence. Request and revision/state queries
exclude paging as prescribed. Root/base-root parsing only checks current first-equals
nonempty prefix/suffix and matching prefixes; capture flag/base presence must agree.

Before native result allocation, the cumulative charge reserves
`sizeof(extension_prepared_invocation) + N*sizeof(extension_prepared_option) +
N*sizeof(extension_pending_host)`, where N is the sealed descriptor option count (<=32).
Both vectors reserve N exactly once and never grow beyond it. The pending-host reservation
is conservative even for pure commands or absent hosts. Before *each* string copy the
charge includes `sizeof(std::string) + byte_length + 1`, conservatively including its inline
wrapper and NUL even if SSO avoids heap storage. This covers command, option names, scalar
text/history IDs, supplied native selectors, and query selector/ID/request strings. Query
struct storage is already within the inline optional in the result. Result/variant/vector
moves transfer existing storage without copying. There is no canonical/codec/Lua graph,
new node count, instruction work, temporary string growth or VM root allocation.
This is existing conservative cumulative native accounting, not RSS or exact allocator
request measurement. Budget charges are not refunded after success or invalidity.

Protocol/usage refusal does not consume a healthy owner or start invocation; repeated
preparation shares cumulative charges. An exhausted/resource owner retains the same first
registration_error as the existing shape/projection/semantic APIs. Existing failures win
before command/input checks, including across moves. Embedded/moved/consumed owners and
unknown external commands refuse as protocol. The new native-budget test performs real
65536-byte string preparations until the actual shared charge boundary refuses, then
checks first-failure dominance across APIs/moves. It does not inject allocator failure or
claim natural host exhaustion. Existing inherited bad_alloc/length_error adapters map to
the same terminal resource path. No private compilation or instrumentation is used.

Receipt-backed Lua construction, exact Lua string/boolean ABI proof, service order,
once-only/poisoning enforcement, authenticated C2 inputs, handler dispatch and response
correspondence remain future work. Generic profile invoke stays closed.
