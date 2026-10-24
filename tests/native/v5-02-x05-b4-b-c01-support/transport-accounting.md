# C01 complete ordinary response boundary

The correction adds one local automatic lambda holding references to existing
runtime state and the render argument. It owns no heap state and introduces no
new VM, retained root, phase execution or result graph. `value_ptr` parameters
and the return value share the original converted response pointer. No graph or
string is copied. Existing phase cleanup has completed before the lambda runs;
the semantic scope remains active through encoding and failure adaptation.

Every ordinary normalization, preparation, checked-shape, finalization, selection
and renderer envelope enters this same lambda. Validation-only and selection-only
calls return immediately without encoding. Native unexpected failures continue
through their existing typed adapters and are not encoded as ordinary data.

The unchanged `codec_encode_reserved` supplies its default 16777216 output-byte
ceiling and existing preflight/output/writer/scratch allocation reservations.
The unchanged callback charges `reservation.total_bytes` before writer allocation
against the same checked cumulative native counter. Earlier ordinary errors now
pay the same intended final-response charge that rendered responses already paid.
There is no quota change, second charge of the final writer, new accounting reset,
diagnostic truncation or parallel response representation. The temporary encoded
string is moved and discarded while the original response pointer returns.

Existing resource exception handling makes the first failure terminal and retains
its exact typed diagnostic through retries and moves. The production oversized
unknown-key request reaches an actual schema error whose unmodified field exceeds
the output ceiling; the correction refuses it at this shared boundary. Small
ordinary error controls retain exact diagnostic bytes, recoverability and native
lifetime. Reversible private observations check exact response pointer identity,
one final pass, authentic phase/F counts and untouched validation/selection paths.
Historical full allocation derivations remain in B4-B support. New final-request
allocation samples exercise each genuine early envelope. Controlled failures are
not measurements of process RSS or natural host exhaustion.
