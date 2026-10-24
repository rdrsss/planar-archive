# Selected-profile hash reservations

The semantic-active digest path reserves before invoking hash.cpp. Ordinary
embedded digest behavior retains its previous branch. No node quota is added:
these reservations call checked `charge`, not `visit`, and do not reset/refund
native bytes, instructions, converted nodes or prior terminal diagnostics.

Pinned libc++ `string::__get_amortized_growth_capacity` takes max(required,2*old
capacity). `__align_allocation_size` rounds requested characters plus terminator
to A=max(default-new alignment,sizeof(void*)) and handles its inline-boundary
special case. Here A=16, inline capacity=22. Subsequent heap requests at least
double; the final request for N total encoded characters is <=2*N+2*A.
The geometric sum is <=4*N+4*A. This deliberately bounds all cumulative
requests, including a large append followed by framing/suffix appends. Each
encoded-byte term is charged four times; 4*A is charged once per hash. This is a
conservative checked bound, not final live capacity or process RSS.

Assignment into an empty owned domain/raw string can first double inline
capacity; its one heap request is <=2*n+A. Each copy receives that reservation.
The fixed 41-character result in hash.cpp starts with 9 inline characters and
appends one character at a time: one heap request of 48 bytes on the pinned
layout. The runtime derives that request from current inline capacity/alignment.
The result and canonical buffer move; XXH3 reads the buffer directly.

The established 512-byte slots cover encoder scratch and four envelope graph
nodes. `canonical.cpp` active-set insertion allocates one pointer hash node per
value visit; map key visits need sorted-pointer space but no active-set node.
Pinned `__hash_table` defaults to load factor1, first bucket count2, and doubles
power-of-two buckets; cumulative bucket storage through maximum active depth D
is <4*D pointers, hence <=32 bytes per depth/visited value on this 64-bit target.
Node storage is 24 bytes and sorted views use 8 bytes per map member. The four
new value/control blocks and four-entry shared-pointer vector fit their existing
512-byte slots with these scratch terms. All source graph nodes are already
converted and owned, so no second source-graph copy is claimed. Recursion uses
bounded existing input depth and native stack frames, not heap allocations.
Linux remains outside this gate.

The private reversible runtime observes reserved deltas against an independent
C++ value walker and records actual aggregate operator-new request bytes across
real hash.cpp/canonical.cpp calls. It covers normal selection hashes, long text
plus a suffix at inline/growth boundaries through1 MiB, depth250, width8192,
and a no-remaining-budget refusal before hashing. Exact operation counts prove
one F per custom job per selection operation. Separate actual one-shot allocation
sampling covers native failures and first terminal retry diagnostics. These are
controlled probes, not natural host exhaustion.
