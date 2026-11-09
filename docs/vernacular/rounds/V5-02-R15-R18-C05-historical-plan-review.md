# C05 historical catalog plan review

Disposition: **approve the bounded plan** in
`tests/native/r15-r18-c05-evidence/plan/CATALOG-DECISIONS.md` and
`CATALOG-INVARIANTS.md`. This is a plan approval, not implementation or gate
approval. No source/test edits or builds were performed.

The current failure is a product boundary regression. Live profile registration
and recorded effective-provenance decoding currently share the same mandatory
seventeen-command validator: `validate_catalog` encodes then calls
`decode_catalog`, while effective provenance also calls `decode_catalog`.
Requiring today's rows/history options in both paths rejects supported recorded
five-command trees. Updating their immutable goldens would conceal the problem.

The approved external-profile contract requires new binaries to read v1/v2
recorded interpretations and effective provenance without old/current Lua or
profile files (reader table at 666–678). Its recorded catalog schema remains
version2 across the execution amendments. Historical validation is structural,
not current-policy re-evaluation (613–655). The explicit valid-before/refused-after
namespace rule at 61 also means an older external command named status or check
cannot be rejected merely because that name became shipped later.

Use strict current admission for `validate_catalog`; give `decode_catalog` an
internal historical policy that selects an exact complete documented shipped
name set. The supported feature-cut sets are 5, 10, 14, 16 and 17, with the exact options
introduced by each cut. These sets follow the original X08 five and the explicit
R15/R16/R17/R18 catalog amendments; they do not include arbitrary six-row or
other partial draft implementations. Selection uses shipped ownership and names,
not total entries (external commands may fill any remaining capacity), current
stdlib size/hash, or ledger format. Five uses the old history signature; ten
adds history event; fourteen adds preview since; sixteen adds checks; seventeen
adds verdict. External collisions are checked against the selected epoch's
reserved names/families/handlers. Every other structural constraint remains.

The current validator must not accidentally inherit this decoder flexibility.
A shared private policy/epoch-aware bounded decode/validation implementation is
sufficient; no runtime option, migration or old Lua execution is needed. Preserve
all existing limits, ownership/correspondence checks, canonical ordering, exact
capability sets and descriptor pairings. Update API prose to state the two
different guarantees. Runtime profile admission can keep its existing call to
`validate_catalog` and thus need not overlap the active profile coder's file.

The tables specify required positive/negative and faithful-mutation controls.
Particularly important are unchanged historical artifacts, live rejection of an
old five-row catalog, and historical acceptance of names that became reserved
later. A merely count-based fallback or allowing each descriptor to choose any
old/current signature independently would not satisfy this plan.

The implementation and every new recognized-schema test still require independent
code review and the integrated host/Linux gates. This plan makes no claim that
previous full-gate failures have been rerun or fixed.

Reviewed plan hashes are generated in `tests/native/r15-r18-c05-evidence/plan/CATALOG-PLAN-IDENTITY.json`.
