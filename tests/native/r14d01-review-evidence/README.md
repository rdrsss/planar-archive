# R14-D01 independent review evidence

Run `python3 tests/native/r14d01-review-evidence/verify.py` from repository root
for the read-only candidate checks and regeneration of reviewer-owned results.
The completed run exited 0. No builds or product tests were needed or run.

`verification.json` records independent checks. The four `coder-*` files freeze
exact candidate/evidence/protected manifests and the sentence diff. Candidate
and protected paths are repository-relative; coder evidence paths are relative
to `tests/native/r14d01-evidence/`. The reviewer manifest lists every other file
in this directory with directory-relative paths, excluding itself.

The verdict is recorded in `docs/vernacular/rounds/R14-D01-review.md`. This shared
checkout was treated as read-only outside the report and this evidence folder;
no other work was reverted.
