# Exact heap observation correction

The original final gate stopped in semantic-success with SIGABRT. Its raw logs,
XML, commands, compiled copy and support sources are retained in final-probes,
final-gate and failed-support-snapshot. The original binary reproduced the abort
in five of eight fresh processes; diagnostic instrumentation reproduced it in
four of eight. No product or production-test bytes changed during investigation.

The diagnostic in diagnose-capacity/repeat-3.log identifies the exact difference:
operation 3/4 has tags hash capacity 512, backing 1024, sealed 512; operation 7
has tags 1024, backing 1024, sealed 512. Each pinned Lua Node is 24 bytes, so the
tags growth adds (1024-512)*24 = 12288 bytes. Raw live memory rises from 437037
to 449325, exactly 12288. All retained reference identities, tags/backing/sealed
key/value identities and membership remain identical. String capacity/use and
Lua stack size remain fixed. Extra collection was diagnostic only and is not
part of the corrected final observer.

release_shape_temporaries deletes nonbaseline entries with lua_rawset(nil).
It deliberately preserves tables and never rebuilds or shrinks their capacities.
Pinned Lua's hash-table collision/rehash and deleted-key history can therefore
change allocated Node capacity even for identical restored live membership.
The old probe incorrectly equated exact live membership with identical table
capacity. A passing retry alone was not accepted as an explanation.

The corrected observer preserves the original one full collection at each
boundary. It logs raw allocator live bytes and exact Node capacity for all three
retained registry tables. The test still requires exact stable live bytes after
subtracting these observed capacities, alongside exact membership/identity,
monotone cumulative work, closed semantic-active state and zero allocator bytes
on owner destruction. It subtracts actual allocated storage, not an allowance,
and does not remove any object/node leaks from the equality. All three tables
are nonempty hash tables with zero array slots in these fixtures. Any other
allocation growth still violates exact equality. The product Lua allocator still
checks every realloc request against the existing cumulative live 512 MiB limit;
this test-only observation introduces no product limit, quota, refund or reset.
Sixteen fresh corrected runs exercise 25 renders each with collection, covering
both formerly intermittent capacities; their raw logs preserve each observation.
