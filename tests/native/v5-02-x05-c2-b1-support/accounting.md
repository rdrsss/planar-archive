# Production projection count proof

Only `codec_projection_count` uses the new bounded stream traits. Existing codec
options and all previous encode/decode implementations remain unchanged.
`projection_input_valid` runs before the hook and scratch: <=64 sections,
ASCII declaration IDs `[a-z][a-z0-9_]{0,63}`, unique IDs, valid UTF-8/noNUL,
<=1,048,576 text bytes each, checked aggregate <=16,777,216. Version1 and boolean
mandatory are structural native fields. Semantic reserved-name policy belongs to
the future raw scanner; accepting a reserved ID here grants no protocol admission.

Pinned source is the same approved B0 dependency, now protected by the actual
pre-edit filesystem manifest. `vendor/glaze/a4a7/include/glaze/json/write.hpp`
string writing (861–890) reserves `ix+10+6*n`; array paths (1160–1188,
1239,1395–1407) skip preallocation for bounded storage and flush separators.
The fixed reflected object path (2344,2469–2545) does not flush fields. Longest
field `mandatory` has nine bytes; static preflight is round16(9+256)=272,
with padding pinned in `core/opts.hpp:34`. The full maximum section has <=6MiB+101
bytes. After every separator pending data is <32768; the root prefix is smaller.
Closing punctuation, string padding and static field preflight fit <374 extra
bytes beyond6MiB. Four256-byte padding units safely dominate that allowance:
32768+6*1048576+1024 = **6,325,248** physical scratch bytes.

`core/buffer_traits.hpp:108–112` makes flush unconditionally noexcept. The sink
therefore sets sticky overflow using subtraction before addition, ignores later
writes and never throws/allocates/dereferences payload. The writer completes the
already validated input. Six bytes per raw text byte, <=102 framing/separator bytes
per section and <64 root bytes bound logical output/work to **100,669,888** bytes.
All offset additions and stream positions stay below that source-derived bound.
`core/write.hpp:19–39` propagates writer error before finalization; success
finalizes once. A bounded writer error is the distinct internal scratch_defect
classification. The512-byte private variant is an explicit controlled adapter;
it does not claim the production bound naturally fails.

The charge is one checked sum of scratch payload plus
`sizeof(projection_workspace)`. The workspace contains reflected version/span,
sink counters/flags, fixed-buffer vector metadata/output pointer/offset,
`glz::context`, and the three-field reservation. On the pinned ABI its sizeof is
**200**, so the complete logical precharge is **6,325,448**. This is a source-derived
logical workspace measure, including its internal padding, rather than a native
stack-frame, allocator-overhead or RSS measurement. The caller already owns the
borrowed view storage and strings. No graph/string/output allocation or copy is
performed. The empty vector's one resize requests exactly scratch bytes; it never
grows once writing starts and is destroyed before returning a successful size.
The callback runs once after all native validation and before this first request.
There is no refund, reset, allocator routing global, detached plan or retained JSON.

Views, bytes and hook context must remain valid and unchanged throughout the
synchronous call, including callback/nested calls. Independent owners may nest
or execute concurrently. No Lua rooting/lifetime/longjmp or mutable-alias race
safety is inferred. The future raw Lua scanner must establish these preconditions.

Errors use an owning codec vocabulary plus a separate boundary enum. Native
malformation is invalid_input/schema; encoded overflow is output_limit with the
existing resource_limit/output_bytes diagnostic. Callback denial moves its exact
owned diagnostic with no allocation/copy; allocation/length exceptions are
resource, unexpected exceptions internal, and bounded writer errors scratch_defect.
Diagnostic creation inside exception handlers is covered by the outer guarded
adaptation. Final fallback can allocate; persistent host heap exhaustion is not
promised recoverable. Future runtime adapters own external-profile code/message/
location mapping; this prerequisite introduces no public protocol diagnostic.
