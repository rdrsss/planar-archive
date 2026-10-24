# R14-C3-B1 prepared acceptance candidate

The native-only `acceptance.prepared` module validates explicit immutable request
intent and owns a consumed sealed capture with independently owned prepared data.
Source paths resolve only in its workspace snapshot; registry comes only from
its exact registry/input.json entry. Pure tree and provenance codecs enforce
bounds, topology, modes and aliases. External stdlib files and both exact canonical
interpretation strings are reverified, including digest correspondence and
supported versions. No caller digest or parsed pointer is an authority.
Preparation performs no current file reads or receipt checking. The separate
verify method delegates directly to the retained native capability.

Request intent accepts positive optional signed-64 heads and explicit already
absolute lexically normalized UTF-8 arguments. It preserves exact arguments and
uses the shared record encoder for body bytes, without cwd/content/state inputs.
No callable acceptance writer, CLI, schema change or Lua policy is introduced.

The 12 new native cases construct actual sealed capabilities and small external
stdlib provenance. They test postseal byte changes, exact binary/mode retention,
selection and registry shape, malformed assembly/manifest/aliases/modes, digest
mismatch, supported versions and canonical correspondence, workspace structural
shape, external count/byte limits, source-directory 8192 inclusive root boundary,
moves/lifetimes/final verification and request grammar/normalization/max heads.
Native capture prevents invalid sealed modes and case aliases before this API;
its 25 unchanged cases and pure tree cases retain those negative checks. Semantic
declaration closure/live schema validity remain a future higher service concern.

`acceptance-first.txt` passes the first ten cases. `acceptance-second.txt` retains
two test-fixture mistakes in newly added tests: the synthetic filename was
incorrect and the directory-bound fixture initially excluded the root count.
Both fixture assumptions are corrected in the final 12-case suite. These are
not claimed as product regressions or red-before-fix evidence for existing bugs.

`run.py` executes required focused tests, both vectors, active and helper lint and
no-work rebuild sequentially; `commands.json` records actual command exits.
`audit.py` checks protected bytes, unchanged prior case names, reversible exact
generated variants and the actual prepared-test link excluding SQLite/format/
Lua/runtime/embedded source. `freeze.py` captures the six owned product/test/
reference files and their SHA256 identity separately from evidence. Protection
scope and coordinator planning exclusions are explicit in protection-exclusions.

Limits: macOS debug only. Linux execution and the integrated R14 release gate
remain outstanding. Receipt checks detect cooperative changes but cannot prevent
subsequent writes. This is an implementation candidate awaiting fresh independent
review; its author does not approve it.

The first required run passed all behavioral selections, CLI247 and vectors;
its exit records are in commands-initial.json. Active lint then reported three
missing private implementation/field Doxygen comments, retained in lint-initial.
The sole product change afterward is captured in final-comment-only.diff. A repeat
of the focused verification sequence had already started before the coordinator
clarified that comment-only edits need only rebuild/lint/no-work; it was allowed
to finish. Final suite logs describe that repeat, without claiming extra coverage.
audit-first.txt predates completion of CLI XML and stops at that pending output;
audit-final.txt supersedes it after all verification finishes.
