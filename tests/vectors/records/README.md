# record_v1 independent vectors

`record-v1.tsv` pins ten exact stored encodings, portable digest preimages and
canonical sixteen-byte seed-zero XXH3-128 digests. `generate.py` implements the
ledger contract framing using Python standard-library `struct`, never importing
production Planar codecs. Its pinned upstream xxHash is compiled separately in a
temporary directory. Digests are formatted from numeric high/low words, independently
of the production `XXH128_canonicalFromHash` call. Existing render vectors and their
published upstream hash sanity checks remain in the parent directory.

```sh
python3 tests/vectors/records/generate.py --check
```

Files include arbitrary binary NUL/non-UTF-8 and CRLF bytes. Directory vectors
retain exact local child IDs in storage and omit only those fields in portable
preimages. Values use hand-transcribed canonical_v1 empty map/list, integer and
raw-string literals. Requests preserve UTF-8 actor and literal argument spellings.
These fixtures do not assert contextual graph, acceptance or SQL policy.
