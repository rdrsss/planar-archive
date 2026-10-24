# Native render transport allocation account

This extends the approved B3-B-R01 transport/codec and B3-C hash accounts without
changing their operations, formulas, limits or codecs. Source is frozen before
the final gate. The cumulative native metric is charged allocation work, not RSS.
No new quota, refund, invocation reset or caller-provided model is introduced.

The only added persistent C++ member storage is two inline `int` retained-root
slots. Before `make_unique<implementation>` the fixed array-size difference is
reserved under a compile-time check against the existing native ceiling, and
installed as the owner's initial cumulative charge. Both embedded and profile
owners contain these slots and receive that fixed charge. The private compilation
reconstructs the baseline four-slot command-state layout and asserts that both
command state and complete owner grow by exactly two ints on the pinned target;
raw output records all sizes. It introduces no second runtime owner/state. The
13 root indices are a fixed stack array; `luaL_ref` registry growth and frozen
preflight/render closures use the existing checked Lua allocator and native
closure inspection allocator, all before external module evaluation.

Native inputs and all intermediate outputs use existing `value_ptr` ownership.
Each protected phase first `validate`s its input: `visit` charges 512 per node
before active-set insertion; strings charge their byte lengths and object fields
charge 128 plus key length. Push/memo/proxy allocations use the Lua allocator;
`freeze_visit` and existing native charges remain cumulative. Protected calls do
not carry C++ native RAII across Lua longjmp. Conversion begins only after pcall
returns: `convert` charges the same node/field/string reservations before its
make_shared/map/string/vector allocations. Existing 512-byte slots include
control blocks/container growth and native traversal scratch. Phase cleanup
copies no graph: it drops the input, restores exact native baseline memberships
and clears the Lua stack. A returned native shared pointer survives that cleanup.

Preflight returns separate normalized render and selection requests. Holding
those pointers costs no heap allocation. Building bundle/capture validation adds
one record and two map fields through the existing checked record builder.
Final render staging adds one record with three short inline keys
(request/model/selected), charged at 512 for its node plus 128+key size for each
map entry before constructing initializer-list/map copies. Source/model/selected
subgraphs are borrowed by shared pointer; each ensuing protected validation and
conversion pass is charged separately. No new native strings, vectors, retained
metadata graphs or source copies are introduced by this staging.

While semantic_active remains true, every render JSON hook uses the approved
reserved Glaze encoder/decoder: parser allocation callbacks charge each request,
immutable decode conversion reserves its entire capacity before first allocation,
encode reserves preflight/output/writer/scratch before allocation, and its moved
encoded string receives a separately charged native value/control block wrapper.
Ordinary error graph node/map/string construction uses the approved exact pinned
allocation-type charges. Private node/map/string/wrapper near-budget controls and
actual sampled operator-new refusals exercise these unchanged paths during real
render operations, including observation decode and presentation JSON encode.

Hashing during selection and render uses the approved B3-C payload/frame and
pinned string-growth reservations. The private independent walker and allocation
observer verify actual hash requests against those reservations, now including
the final Markdown fragment hash. See the unchanged historical hash-accounting.md
for the derivation (4*N+4*A cumulative canonical string growth, owned copies,
digest storage, active-set scratch). No duplicate hand-written JSON or hashes.

After authentic rendering, a complete-envelope `codec_encode_reserved` pass
applies the existing default 16 MiB byte ceiling to packet, manifest and wrapper.
Its native reservation callback charges before any writer allocation. The string
is moved into the expected and discarded; the native response graph is returned
without a copy or wrapper allocation. Existing typed codec resource failures
become the first terminal profile diagnostic. A controlled no-remaining-native-
budget probe exercises this callback after the real renderer has succeeded;
a separate adapter substitutes an oversized native result *after* real rendering
to prove this final pass's output boundary. This is not claimed to be a naturally
reachable valid packet. Actual allocation samples include the final writer's last
requests and prove typed first failures survive retries. No natural host exhaustion
is claimed. Linux and a full release/milestone gate remain future work.

The private retained-lifetime observer accounts exact registry hash Node storage
separately from retained object liveness. See heap-observation.md for the preserved
intermittent assertion failure, source derivation and corrected exact invariant.
