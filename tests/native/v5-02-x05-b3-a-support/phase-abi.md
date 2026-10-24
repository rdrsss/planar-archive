# X05-B3-A trusted phase ABI

All exports consume one structural value and return existing `{value:...}` or
`{error:E}`. These exports are product selected, internal trusted operations; they
are not job constructors. Native orchestration must retain and protect the
original values. No function, pointer, hidden mutable session or callable context
crosses a phase. Native accounting, same-owner metadata authenticity, transient
immutability and private root sealing remain B3-B work.

`shape_prepare({validation:{bundle,capture},context:{dialect:
"declarations_shapes_v1",shapes:{ID:{stages:POLICY}}}})` returns
`{base:{bundle,capture,dependencies,jobs},requests:[REQUEST...]}`. Context may be
omitted for embedded preparation. The optional context has native-final catalog
shape descriptors; it needs no callback. Declaration/envelope dialect injection
still refuses. Every mandatory and base-cycle check completes before requests
are returned. Requests are normalized bundle job order, omitting shipped shapes.
Each REQUEST is `{index,id,shape,file,input}`: index is one-based normalized job
index, id is job ID, shape is selected external ID, file is logical job path.
Input is exactly `{version:1,job,spec,assignments,reviewer}`. Shared spec/model
values are referenced in Lua rather than explicitly deep copied by the helper;
native conversions and any duplication must charge existing cumulative budgets.

`shape_check({request:REQUEST,response:RAW,total:INTEGER})` returns
`{index,id,shape,obligations,guidance,total}`. The native caller starts total at
zero and supplies the previous checked total. RAW follows the unchanged B1
success/refusal protocol. Checks and count guards precede material expansion.
The result carries sorted synthetic obligations with assignment-derived roles.
A failure stops orchestration before the next F; callers must not invent results
or skip failed requests. Exact global count arithmetic has controlled prior-total
coverage; a real full-bundle100000/100001 run remains constrained by instruction
limits, as recorded in B1.

`shape_finish({prepared:PREPARED,results:[CHECKED...],definitions:[DEFINITION...]})`
requires one checked result and definition per request, in request order.
DEFINITION is `{index,job,id,revision,definition}` from that same native-owned
selected invocation; index and job identify the request, id identifies its shape.
Wrong job/index/id or missing/extra entries refuses internal. The helper does
not prove native identity or same-call authenticity. It preserves all base edges,
adds unit obligations, and reruns complete cycles in existing diagnostic order.
It returns the base model with custom jobs' `selected_shape` exactly
`{id,revision,definition,obligations,guidance}` and no shape_results sidecar.
It shallow copies base/job records before replacing graphs or adding metadata;
no previously prepared model is mutated.

The old `validate_bundle(input,context)` composes these helpers, keeping its
private callback adapter and old shape_results output. It omits selected_shape
and native definition requirements. Embedded output and old W interpretation
remain unchanged. Shipped shape semantics are the original code. No phase
executes F, selects roles, computes external W or admits public profiles.
