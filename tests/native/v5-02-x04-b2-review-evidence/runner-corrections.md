# Reviewer runner corrections

Two reviewer-only runner assumptions failed before any build or candidate test.
Neither failure changed the candidate or author evidence.

1. The first invocation rebuilt the diff in the JSON manifest's sorted key order.
   The author diff uses its owned-file order. The assertion failed at `verify.py`
   line58 with `AssertionError`; the runner now extracts and validates the complete
   eleven-file diff order, then independently reconstructs its bytes.
2. The second invocation assumed the classification regression used `focused.xml`.
   Its actual file is `regression.xml`; the original traceback is retained in
   `preproduct-runner-failure02.log`. The runner now reads that exact file.

The complete corrected run is `verification.log`, with executed commands and
zero exit codes in `commands.json`. Product sources stayed frozen throughout.
