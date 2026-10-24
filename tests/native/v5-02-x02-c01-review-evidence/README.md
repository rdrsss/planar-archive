# X02-C01 independent review evidence

Docs-only independent review of the frozen two-document correction candidate.
Run from the repository root:

```sh
python3 tests/native/v5-02-x02-c01-review-evidence/verify.py
```

The checker verifies the supplied manifest/diff identities, current and frozen
candidate bytes, all correction and original X02 author-evidence entries, source
bases, complete correction baseline protection and AGENTS.md symlink. It resolves
candidate local links/anchors and independently extracts the exact documented
omission strings to model UTF-8 assembly/removal arithmetic. Outputs are
verification.json, links.json, byte-vector.json and source-basis.json.

Author evidence paths use the bases described in its README: candidate paths
are repository-relative; author evidence manifest entries are relative to the
author evidence directory. The review evidence manifest uses repository-relative
paths, includes this README/checker/outputs, and excludes itself and the separately
hashed review report. Review source-basis hashes pin the reviewed context at
verification time; later coordinator round/status changes need independent scope
assessment rather than rerunning the author checker against new review files.

No builds, product tests, runtime/schema/parser/Lua execution or Linux verification
were performed. Manual contract review and source inspection accompany these
hash/link/arithmetic checks; passing text checks alone would not approve a design.
P1/P2 remain unselected. External-profile implementation and its runtime gate,
RA01 and execution remain future work. Shared candidate/source/history preserved;
reviewer ownership released and both build directories untouched/idle.
