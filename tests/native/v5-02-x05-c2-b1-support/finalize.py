"""Bind frozen candidate/raw evidence; release Debug and all owned edits."""
from pathlib import Path
import datetime,difflib,json,shutil,sys,xml.etree.ElementTree as ET
sys.dont_write_bytecode=True
from freeze import ROOT,EVIDENCE,SUPPORT,OWNED,candidate,protected,sha,verify_candidate,write
verify_candidate();write(EVIDENCE/'protected-final.json',protected())
G=EVIDENCE/'final-gate'
def cases(path):
 rows=ET.parse(path).findall('.//testcase');assert rows and all(c.get('status')=='run' and not any(c.find(t)is not None for t in ['failure','error','skipped'])for c in rows);return len(rows)
assert cases(G/'focused.xml')==432;assert cases(G/'history-codec.xml')==11
codec_cases=cases(G/'codec-targeted.xml');assert codec_cases>=7
assert cases(G/'projection-identity.xml')==9;assert cases(G/'production-render.xml')==46
private=(EVIDENCE/'final-probes/production.log').read_text();assert 'PASS cases=35 assertions=474 actual_fault_positions=45' in private
assert 'PASS controlled_scratch_defect' in (EVIDENCE/'final-probes/scratch-defect.log').read_text()
paths=OWNED+[str(p.relative_to(ROOT))for p in sorted(SUPPORT.iterdir())if p.is_file()];diff=[]
for name in paths:
 old=EVIDENCE/'preedit'/name;before=old.read_text()if old.exists()else'';after=(ROOT/name).read_text()
 diff.extend(difflib.unified_diff(before.splitlines(keepends=True),after.splitlines(keepends=True),fromfile='before/'+name,tofile='after/'+name))
 if name.startswith('src/engine/json_codec/'):
  changes=difflib.SequenceMatcher(a=before.splitlines(keepends=True),b=after.splitlines(keepends=True)).get_opcodes()
  assert all(tag in ['equal','insert']for tag,*_ in changes),(name,changes)
(EVIDENCE/'candidate.diff').write_text(''.join(diff));write(EVIDENCE/'existing-codec-preservation.json',dict(three_product_files_only_insertions=True,all_previous_lines_preserved=True))
manifest=candidate();write(EVIDENCE/'candidate-manifest.json',manifest);archive=EVIDENCE/'candidate-source';archive.mkdir()
for name in manifest:
 dest=archive/name;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/name,dest)
report=f'''# X05-C2-B1 production projection count candidate

Implementation and bounded focused Debug verification complete; **fresh independent
review pending**. No self-approval, full/release gate or Linux claim. Shared checkout
`dev/planar-v2-vernacular`, starting HEAD3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a.
Inherited dirty rewrite preserved; `git-before.txt` records it. No Planar tools,
skills, roles, queues, subagents, commits, install, publish, merge or operator data.
Ownership: json_codec codec.cppm/cpp/t.cpp, new B1 support/evidence, exact X-PROJ
amendment and six current prose files. Coordinator owns round records.

## Behavior and accounting

`codec_projection_count` accepts synchronous borrowed native section views,
validates all B0 sizing preconditions before hook/scratch, and reflects exactly
version1/sections through the existing private Glaze options. No generic AST copy,
retained output or parallel JSON grammar is introduced. Validity checks include
<=64 sections, unique declaration IDs, UTF8/noNUL, <=1MiB per text and checked
<=16MiB aggregate. The encoded ceiling may tighten but never relax16MiB.

The exact checked logical precharge is6,325,248 scratch + sizeof fixed workspace
(200 on pinned ABI) =6,325,448 bytes, once before first allocation. This is a source
derived logical measure, not an exact native stack-frame, allocator-overhead or RSS
measurement. Full pinned-source proof, retained B0 limits, nonthrowing sticky
continuation and <=100,669,888 logical byte work bound are in
`../v5-02-x05-c2-b1-support/accounting.md`. Scratch is destroyed before success;
callback context, views and source bytes remain caller-owned/stable synchronously.
There are no refunds/resets or shared allocation routing. Independent nested and
concurrent owners are supported under the documented nonmutation precondition.

Typed enum outcomes distinguish native invalid_input, encoded output_limit,
resource, scratch_defect and unexpected internal failure. All errors own existing
codec vocabulary; callback denial moves the exact diagnostic unchanged. Both
ordinary exception adapters and diagnostic creation are guarded. Final fallback
may allocate; persistent host exhaustion is not promised recoverable. Runtime
external-profile mapping/location remains future. No public code/message is added.

X-PROJ records the operator's settled global18 built-in reserved IDs for both role
and history kinds, including conditional sections, omissions and recorded_history.
The codec deliberately does not enforce this semantic policy. Later raw scanner
owns shared name authority and protocol admission. No F dispatch, Lua rooted
lifetime/longjmp boundary, read services, capabilities, receipts, public CLI,
whole-invocation integration, X06–X08 or execution/RA01 is delivered. Public init
still creates format1.

## Frozen verification

Product/test/support/prose were frozen before the final gate. `frozen-candidate.json`,
`candidate-manifest.json`, `candidate-source/` and `candidate.diff` bind the exact
candidate. All three product files preserve every prior line: only insertions,
verified in `existing-codec-preservation.json`. Existing codec options and behavior
are unchanged. Commands/argv/exits/timings and raw stdout/stderr are in
`final-gate/commands.json` and `final-probes/commands.json` with their named logs.

- Direct Debug all+planar_tests build passes. Inventory **698**, preserving all691
  prior names and adding seven projection count cases.
- Frozen focused **432/432**, supplemental history codec **11/11**; zero failures,
  errors, skips or disabled cases.
- Complete targeted codec **{codec_cases}** cases, projection identity9 and
  production render46 pass with nonzero XML counts.
- Seven current vectors, six source/runtime audits, pinned formatter/Doxygen
  over118 active sources, and no-work all+planar_tests build pass.
- Final whole production helper observer: **35 cases,474 assertions,45 actual
  allocation fault positions**. Separate controlled scratch-defect probe passes.

The seven registered cases cover unchanged-codec parity, controls/UTF8/max IDs/
64 sections/1MiB escaping, exact16MiB−1/equal/+1, every native precondition before
charging, exact/cumulative/refused/throwing hooks, owned diagnostic preservation,
borrowed immutability, nested/concurrent owners and tighter ceilings.

Private fixtures reuse B0 stress only in new owned adapters and compare production
counts with both independent actual Glaze output and unchanged codec_encode.
Nineteen threshold−9..+9 fixtures precede maximum sixfold escaping;64 mixed UTF8/
control/max-ID and maximum raw/alternating cases preserve the admitted source
bound. Entire successful calls make exactly one6,325,248-byte ordinary-new request,
freed before return, with zero retained allocations. The observer includes
validation, hook, scratch, writer/finalize/destruction and error adaptation; oracle
AST/output setup is outside it. Allocation observation is test-only global new,
never product routing. Pinned vector<char> needs no aligned-new observer.

Actual one-shot failures cover scratch, invalid/encoded diagnostic construction,
callback diagnostic copy and exception-adapter diagnostic creation/outer fallback.
Moved long diagnostic storage retains its original pointer with zero requests;
a failed intentional copy preserves original owner storage. A nested inner scratch
failure permits outer success and preserves previous owner fields; nested scratch
and diagnostic allocations clean up before return. These are controlled failures,
not natural host exhaustion. The512-byte private codec adapter changes only actual
physical scratch capacity and returns distinct scratch_defect/internal; it does not
claim the source-proven production capacity failed naturally.

Every private compile copied and rehashed **888** complete shared .o/.pcm/.a files,
expanded response files, redirected all module inputs, object/dependency outputs,
disabled implicit modules/module maps and linked private archives only. Shared and
copied full membership/digests are verified afterward; see nested
`shared-compiled-before.json`, `copied-compiled.json`, `compiled-stability.json` and
exact scratch `transformations.json`.

## Attempts and protection

Development logs preserve an initial compile refusal: pinned Glaze reflection
requires named linkage for its reflected root; the final private module type uses
it. Initial lint refused missing Doxygen on new traits/buffer declarations; these
were documented. One support-generation command stopped before writing its target
because its already-corrected marker was absent; source generation was corrected.
No failed check is represented as success. `development-lint-3.log` preserves the
known upstream Doxygen SIGBUS first attempt and successful existing retry policy.
The final gate has its own raw result and is authoritative.

Before writes, actual owned files were copied to `preedit/`, and complete protected
membership/modes/symlinks/digests captured for src/vendor/cmake/scripts/fixtures/
docs/tests plus root files/symlinks, excluding owned paths and coordinator rounds.
All **38,303** protected entries and **4,238** release entries compare unchanged
at finalization; AGENTS.md remains CLAUDE.md's symlink. Historical evidence and
old B0 sources are immutable. Root cache/generated directories are outside this
protection claim. Debug compiled inputs intentionally rebuild; original888 input
baseline is retained separately. Release was never built or run.

All Debug use and all owned edits are released for coordinator/reviewer use.
**No later edits are planned.** Fresh independent review remains required before
scanner/retained projection integration.
'''
(EVIDENCE/'REPORT.md').write_text(report)
# Private dependency trees are transitively bound by their complete copied manifests.
artifacts={str(p.relative_to(ROOT)):sha(p)for p in EVIDENCE.rglob('*')if p.is_file()and 'private'not in p.relative_to(EVIDENCE).parts and p.name not in ['artifact-manifest.json','finalization.json']}
write(EVIDENCE/'artifact-manifest.json',artifacts)
write(EVIDENCE/'finalization.json',dict(utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),candidate_sha256=sha(EVIDENCE/'candidate-manifest.json'),artifact_manifest_sha256=sha(EVIDENCE/'artifact-manifest.json'),debug_released=True,all_owned_edits_released=True,no_later_edits_planned=True,review_pending=True))
verify_candidate();print('Finalized candidate; Debug and all edits released; no later edits',flush=True)
