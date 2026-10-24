# Reviewer runner correction

The initial independent audit reconstructed the complete owned diff by iterating
the sorted-key candidate manifest. The author's diff uses its explicit ownership
order, so the whole-patch equality assertion failed before any product command.
`verify-initial.py` and `initial-run.log` preserve the runner and reproduced error.

The corrected runner extracts the path order from the actual diff headers, checks
that the exact nine candidate paths occur once, then independently reconstructs
each hunk from archived originals and current bytes. Candidate or author evidence
was not changed. This is a reviewer ordering assumption, not a product failure.

The first raw-output count also assumed no padding between CTest's `Test` and `#`. Actual
CTest output pads one- and two-digit test numbers in that position. The preserved
`verify-order-corrected.py` and `raw-audit-initial-run.log` reproduce that audit
assertion. The first adjustment allowed padding after `#` and still failed;
`verify-padding-assumption.py` and `raw-audit-padding-attempt.log` retain it.
The final expression permits whitespace on either side of `#`; JUnit and LastTest
name equality checks were already passing. Both runner corrections preceded all
independent product commands.
