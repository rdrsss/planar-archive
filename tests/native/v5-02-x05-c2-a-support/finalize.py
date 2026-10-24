"""Bind exact frozen identity candidate and raw verification; release Debug and edits."""
from pathlib import Path
import datetime,difflib,json,shutil,sys,xml.etree.ElementTree as ET
sys.dont_write_bytecode=True
from freeze import ROOT,EVIDENCE,SUPPORT,OWNED,candidate,filesystem,protected,sha,verify_candidate,write
verify_candidate();proof=protected();write(EVIDENCE/'protected-final.json',proof)
assert filesystem()==json.loads((EVIDENCE/'frozen-filesystem.json').read_text()),'filesystem changed after freeze'
write(EVIDENCE/'final-filesystem.json',filesystem())
GATE=EVIDENCE/'final-gate';summary=json.loads((GATE/'summary.json').read_text())
def cases(path):
 c=ET.parse(path).findall('.//testcase');assert c and not any(x.find(t)is not None for x in c for t in ['failure','error','skipped']);return len(c)
assert cases(GATE/'focused.xml')==425;assert cases(GATE/'history-codec.xml')==11
assert cases(GATE/'projection-identity.xml')==9;assert cases(GATE/'production-render.xml')==46
assert cases(EVIDENCE/'final-probes/allocation.xml')==2
for mode in ['remaining','late','nodes']:assert cases(EVIDENCE/'final-probes'/(mode+'.xml'))==1
paths=OWNED+[str(p.relative_to(ROOT))for p in sorted(SUPPORT.iterdir())if p.is_file()]
diff=[]
for name in paths:
 old=EVIDENCE/'preedit'/name;previous=old.read_text()if old.exists()else''
 diff.extend(difflib.unified_diff(previous.splitlines(keepends=True),(ROOT/name).read_text().splitlines(keepends=True),fromfile='before/'+name,tofile='after/'+name))
(EVIDENCE/'candidate.diff').write_text(''.join(diff));manifest=candidate();write(EVIDENCE/'candidate-manifest.json',manifest)
archive=EVIDENCE/'candidate-source';archive.mkdir()
for name in manifest:
 dest=archive/name;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/name,dest)
write(EVIDENCE/'debug-final-compiled.json',{str(p.relative_to(ROOT/'build/rewrite-debug')):sha(p)for p in (ROOT/'build/rewrite-debug').rglob('*')if p.is_file()and p.suffix in {'.o','.pcm','.a'}})
commands=json.loads((GATE/'commands.json').read_text());private=json.loads((EVIDENCE/'final-probes/commands.json').read_text())
report=f'''# X05-C2-A projection definition identity candidate

Status: implementation and focused Debug verification complete; **fresh independent
review pending**. No self-approval, full C milestone, release rerun or Linux claim.
Shared checkout dev/planar-v2-vernacular, starting HEAD
3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a; inherited dirty rewrite preserved.
No Planar tools/skills/roles/queues, subagents, commits, installation, publication,
merge or operator data. Ownership is runtime.cppm/runtime.cpp, colocated external
registration tests, six current prose files and new C2-A support/evidence.
Coordinator owns the round records. AGENTS.md remains the CLAUDE.md symlink.

## Scope and behavior

Internal resolve_projection_definition(id) resolves only retained kind2 entries,
borrows their validated final D (id,abi,revision,input,help), and returns owned
id/revision/digest. Native ordinal locates original privately retained module bytes;
it does not enter the hash. Definition is exact H(projection_definition_v1,
{{descriptor:D,owner:{{tier:external,module:external/<path>}},module_bytes:HEX}}).
HEX is lowercase original selected-source bytes. Unselected modules, profile bytes,
host selector and ordinal do not enter the definition. No F executes, invocation
starts, input-kind dispatch occurs, or budget resets/refunds. The approved existing
shape resolver is byte-identical to its actual pre-edit copy.

Unknown, shape and shipped IDs refuse at catalog.projections without consuming a
healthy owner. Allocation/resource failures move into existing shared shape_failure,
retaining exact first code/message/limit/maximum/field across projection/shape/
validation/selection/render and moves. No new terminal slot or diagnostic vocabulary.
Retained projection execution/native result admission remains C2-B; read services,
capabilities, receipts/whole-invocation orchestration, public profile CLI, X06–X08,
execution/RA01 and Linux remain open. Public init creates format1.

## Reservation derivation and real allocation evidence

[hash-accounting.md](../v5-02-x05-c2-a-support/hash-accounting.md) gives the pinned
source derivation. All prospective graph/map/path/hex/result work is reserved before
its allocations. Existing B3-C helpers bound cumulative canonical string growth by
4*N+4*A and empty-string copies by2*n+A; envelope/encoder scratch/digest storage are
included. The immutable descriptor stays borrowed. Checked size guards precede
path addition/source doubling. Subsequent terms use checked charge calls.
There are25 logical node visits per successful lookup; byte-only borrowed-D scratch
charges deliberately overlap conservative graph slots. No new quota or RSS claim.

The private full-resolver operator-new observer covers staging, graph construction,
hash envelope, canonical encoder scratch and returned metadata. Eleven fixtures span
source207..1,048,207 bytes, inline/growth boundaries and path6..816 bytes. Actual
requests are45..48, total5,880..10,487,448 bytes, against reservations
26,632..16,801,112 bytes. An independently computed flat-schema formula matches every
reservation exactly; all actual totals are below it. All47 actual allocation positions
of a nontrivial lookup are failed one-shot and preserve typed first-terminal errors
across the other APIs and moves. Controlled no-remaining native, late reservation
native and converted-node exhaustion each refuse before the first prospective
allocation hook. These are controlled failures, not natural host exhaustion.

Private source substitutions have exact forward/reverse reconstruction and original/
private SHA256. Drivers recursively expand response files and isolate object, PCM
and dependency outputs; full shared object/PCM/archive membership+SHA256 remain
unchanged before/after each private compile. Final raw evidence is final-probes/.

## Baseline, attempts and identity

preedit/ holds actual owned-file copies/SHA256, actual682-name inventory, git status
and full release-file SHA256 captured before writes. No contemporaneous C2-A
whole-filesystem snapshot was captured. Historical preservation instead uses the
**prior independently approved C1 reviewer checkpoint**
`tests/native/v5-02-x05-c1-review-evidence/source-before.json` (SHA256
{proof['historical_checkpoint_sha256']}), plus its40-entry review artifact manifest.
protected-final.json enumerates every changed/new path from that checkpoint and
verifies all prior historical membership/bytes/modes/symlinks outside explicit
owned/coordinator paths. This provenance is not represented as a fresh pre-edit
whole-tree baseline. The full release baseline remains byte/membership identical
({proof['protected_release_files']} files); no release command was invoked.

Development initially attempted the new filter before rebuilding the edited test
object: zero matches/exit2, not success. Development-probes first failed an
independent expected-charge assertion because the test formula counted external/
as8 bytes instead of9; the8-byte resulting charge discrepancy was corrected only
in support. No product fix was needed. Raw first-failure logs/XML are retained.
The expanded maximum-descriptor support attempt03 had a compile-only local-name typo; attempt04 corrects it and passes all controls, including dynamic returned ID/revision allocation. Product bytes did not change. These attempts do not count as final passes.

## Frozen focused Debug gate

Product/test/support/current prose were frozen before this gate. Candidate SHA256
is checked before/after every command and the final filesystem matches freeze.
Exact argv/cwd/exit/time for all{len(commands)} outer commands are in
final-gate/commands.json; isolated compile/link/run arguments for {len(private)}
private commands are in final-probes/commands.json. Every final command exits zero.

- Direct `cmake --build --preset debug --target all planar_tests` passes.
- Inventory691 preserves all682 pre-edit names and adds9 C2-A names.
- Focused CTest executes425/425 (prior416+nine), plus11/11 history/codec supplement;
  zero failures/errors/skips/disabled, all statuses run. Immediate LastTest.log copies
  and JUnit are preserved.
- Direct projection-definition9/9 and existing render/profile-render46/46 pass.
- Six established common vectors and six source/runtime audits pass. The new
  independent Python canonical/upstream seed-zero XXH3 oracle separately checks
  its pinned806-byte preimage and digest
  `xxh3_128:11f11b5d6a3fd4c65e8b7cb163a73f1f`.
- Two private allocation cases and three controlled refusal cases pass.
- Pinned formatting/Doxygen over118 active C++ sources and private support format pass.
- Final all/planar_tests build reports `ninja: no work to do.`

Production tests cover final descriptor identity, material help/input/revision/id,
selected source/path/comments, module reorder without ordinal leakage, unselected
module/profile metadata independence, actual host profile selector capture, mutation
of caller snapshots and filesystem after capture, repeat ownership/destruction,
unknown/shipped/shape namespace refusal, mixed APIs/moves, cumulative exhaustion
and both projection-first/shape-first terminal diagnostics. Every selected F has
an execution-failure sentinel and never executes during identity lookup.

candidate.diff and candidate-manifest.json bind the scoped exact change, with
candidate-source/ copies. artifact-manifest.json and finalization.json bind raw
evidence/report/diff. Debug and all author edits are released after final hashing;
no subsequent author edits. Coordinator must check this exact candidate before
fresh independent review.
'''
(EVIDENCE/'REPORT.md').write_text(report)
artifacts={str(p.relative_to(EVIDENCE)):sha(p)for p in sorted(EVIDENCE.rglob('*'))if p.is_file()and p.name not in ['artifact-manifest.json','finalization.json']}
write(EVIDENCE/'artifact-manifest.json',artifacts)
write(EVIDENCE/'finalization.json',dict(timestamp_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),candidate_manifest_sha256=sha(EVIDENCE/'candidate-manifest.json'),artifact_manifest_sha256=sha(EVIDENCE/'artifact-manifest.json'),diff_sha256=sha(EVIDENCE/'candidate.diff'),report_sha256=sha(EVIDENCE/'REPORT.md'),filesystem_sha256=sha(EVIDENCE/'final-filesystem.json'),candidate_entries=len(manifest),artifact_entries=len(artifacts),debug_and_all_edits_released=True,no_subsequent_edits=True,review_pending=True))
print('FINALIZED; Debug and all author edits released. No later edits.')
