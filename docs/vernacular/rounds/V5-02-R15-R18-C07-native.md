# C07 native ledger and replay correction

Implemented N-C06-1, N-C06-2 and P-C06-3 within assigned ownership. This is an implementation report, not independent approval or final gate acceptance.

## Changes

Saved verdict responses are reconstructed from verified original-event facts and compared as exact codec-encoded bytes. The reconstruction covers all headers, applicability/decision/admission, chronological activation-local count, the original acknowledged dispatch cap, repair generation ordinals/revisions, opened and cleared blocker rows and the event's transition after-states. Approve and historical branches no longer skip count/cap checks. Later loads, activation, repairs and blocker clearing do not substitute current values for original facts. Existing canonical codecs remain the sole product serializer.

Current observed outputs now come solely from the ordered check_recorded event scan. Every completion is independently verified against its linked check_recorded event by load_completions before this projection; the scan includes successful, failed and inconclusive checks, selects original own units and the current owning lineage, and excludes consumed producer units. Removing the completion-unit seed preserves historical completion/vector data while preventing a surviving consumer from introducing a retired owner's observation.

All four embedded/profile early replay returns for run/recovery options retain the already-created stdout_error. No error vocabulary, replay bytes or request ordering changed.

A standalone test-only snapshot reader, snapshot_probe.t.cpp, was authorized by the coordinator and registered outside default all. It uses the public read-only native snapshot API and prints observations/completion units; it does not mutate storage or introduce a product command. The real CLI ownership workflow uses that helper because the existing singleton native fixture cannot reproduce a surviving consumer's consumed-unit contamination.

## Evidence

Raw logs and source/evidence hashes: tests/native/r15-r18-c07-evidence/native/IDENTITY.json.

- Tests-first native red: the assertion-bearing new corruption matrix failed on its **first** count corruption, one case/590 assertions (589 passed, one failed). This does not claim all field cases were independently red before implementation. Existing reviewer red evidence separately demonstrates four public saved-result corruptions. Later independent mutation coverage must establish every newly tested field family.
- Tracked observed CLI red reproduced the exact old producer output inherited through b_worker completion after replacing a_worker. The helper was the reviewer's previously built coherent pre-C07 source reader, recorded in C06 evidence; no product/shared-build rollback occurred.
- Tracked stdout CLI red gathered **10/10 wrong-message failures**, five operations (claim/checkpoint/refresh/takeover/release) in embedded and profile compositions. It also established normal exact replay and unchanged logical ledger. First script setup mistakenly expected the error fields outside the envelope; that excluded attempt is retained as stdout-red-setup.log. Corrected assertions produced stdout-red.log before rebuilding product.
- `cmake --build build/rewrite-debug --target planar planar_ledger_execution_tests planar_execution_snapshot_probe -j3` passed. Final test-only matrix rebuild also passed. Builds were serialized in granted Debug lane2.
- Final complete ledger suite: **164 cases, 125777 assertions, all passed**, zero skips (ledger-final.log). The new test executes **37 independent valid saved-result corruptions across five branches**: approve, admitted rejection, unowned rejection, historical verdict and capped rejection. Fields include count/cap, role identity/state/drift, nonexistent cleared blocker, repaired role/generation ordinal/revision, opened blocker ID/kind. Every changed response first encodes successfully, then verifier and native snapshot read both refuse ledger_corrupt, preventing codec-shape failures from masquerading as cross-row coverage. Existing actual writer tests cover admitted/capped/unowned/approve/historical outcomes, later reactivation/count reset and blocker clearing. The replay test additionally checks identical original bytes after a later repair round and load.
- New stdout CLI: **10 scenarios passed**, embedded/profile with all five operations, exact fallback messages, exit4/io/stdout, unchanged ledger and subsequent byte-identical replay (stdout-green.log).
- New observed CLI: **four controls passed**: obsolete owner observation absent while consumer history survives; fresh owner completion adds correct own output; newer failed own check overrides old completion observation; persistent historical completion rows unchanged across replacement (observed-green.log).
- `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`: pinned formatter and Doxygen passed **250 active source files** (lint.log). `git diff --check` passed.

The first ledger green run (164 cases/117733 assertions) predates the final test expansion and remains separately preserved. Product source did not change for the final test expansion. Full Debug/Release/Linux gates, no-work verification and independent mutations/review remain coordinator obligations. No installed/legacy Planar, operator data, commit, installation, publication or merge was used. All executable ledgers were disposable. Debug lane2 released; no build or test process remains.
