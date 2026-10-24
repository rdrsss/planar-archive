# Prospective projection identity allocation accounting

C2-A reserves before constructing any hex, path, graph, canonical buffer or result
metadata. It borrows immutable final D rather than copying it. No invocation or
Lua action starts, and no native/node/instruction budget resets or refunds.

The five-string descriptor has six value visits and five key visits. The new
input/owner/three strings add five values/five keys; hash.cpp adds four envelope
values: 25 logical visits total. Existing visit() charges 512 bytes per visit.
Five new map members additionally charge the existing 128-byte slots. These
conservative graph slots cover value/control blocks, map nodes/initializer-list
scratch, encoder active-set nodes (24 bytes), cumulative hash buckets (<32 bytes
per visited value), and sorted-pointer views (8 bytes/member), on pinned macOS
ARM64 libc++. There is deliberate overlap: reserve_hash_payload(D) additionally
charges eleven 512-byte scratch slots without incrementing converted-node counts.
No new node or byte quota is introduced.

Borrowed D uses the approved B3-C reserve_hash_payload accounting. The prospective
wrapper has two map headers, five key strings and three value strings. Each
canonical header is nine bytes. reserve_hash_bytes charges each encoded term four
times; reserve_hash_frame reserves the envelope, domain/source copies, one 4*A
padding term and the pinned digest growth request. This uses B3-C's source proof:
libc++ doubles growing string capacity and aligns requests; final request <=2*N+2*A,
so cumulative output requests <=4*N+4*A, A=16 here. The fixed 41-character digest
needs one 48-byte heap request on this layout. XXH3 reads the canonical buffer;
canonical and digest strings move.

Before arithmetic, path+9 and source*2 are explicitly checked against the existing
512MiB maximum. Every further addition/multiplication in the reservation is
expressed through checked charge calls. The path buffer reserves once then appends
prefix and path; no concatenation growth is omitted. Hex likewise reserves once.
Both staging strings and their by-value command_text copies reserve independently
with 2*n+A, covering an empty string assignment's inline-capacity doubling. Fixed
map keys, tier, returned ID and returned revision each receive this copy bound.
Small inline strings are conservatively charged despite requiring no heap request.

allocation.cpp independently computes the flat schema's exact charge, without
walking the graph constructed by the resolver. Its operator-new observer surrounds
the whole public resolver call: staging, graph construction, hash/envelope/encoder
scratch and return metadata. Fixtures cover inline/growth boundaries, suffix bytes,
large source comments through 1,048,207 bytes, and an 816-byte module path. Maximum64-byte ID/revision and4096-byte help also exercise dynamic result metadata. It
compares all observed requests with the planned delta, verifies 25 node visits,
and samples every actual allocation request in a nontrivial successful operation.
All successful one-shot refusal responses are then checked across projection,
shape, validation, selection, render and owner moves for exact first-error fields.

Private source substitutions observe completed reservations and set no remaining
native budget, late native budget (after initial logical visits) or exhausted nodes.
The hook immediately before prospective allocation must remain unvisited on each
refusal. Forward/reverse identity and original/private source hashes are recorded.
Compile drivers expand response files and isolate object, PCM and dep outputs;
shared compiled membership and hashes are verified before/after compilation.
These are controlled failures and cumulative charged-work bounds, not process RSS,
natural host exhaustion, new Linux evidence or callable execution claims.
