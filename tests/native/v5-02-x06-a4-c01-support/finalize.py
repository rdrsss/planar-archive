from pathlib import Path
import hashlib,json
R=Path(__file__).resolve().parents[3];O=R/'tests/native/v5-02-x06-a4-c01-evidence'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
a=json.loads((O/'baseline.json').read_text());b=json.loads((O/'final.json').read_text());c=json.loads((O/'candidate.json').read_text());owned=set(c['files']);errors=[];dirs=[];changed=[]
for p in sorted(set(a)|set(b)):
 if p in owned:
  if a.get(p)!=b.get(p):changed.append(p)
  continue
 if p not in a or p not in b:errors.append(p);continue
 x,y=a[p],b[p]
 if x['type']=='directory' and y['type']=='directory':
  if any(x[k]!=y[k] for k in ['type','mode','device','inode']):errors.append(p)
  elif x!=y:dirs.append(p)
 elif x!=y:errors.append(p)
assert not errors,errors
for p,h in c['files'].items():assert sha(R/p)==h,p
assert sha(O/'candidate.diff')==c['diff_sha256']
builds={p:dict(members=sum(n==p or n.startswith(p+'/') for n in a),file_bytes=sum(v['size'] for n,v in a.items() if (n==p or n.startswith(p+'/')) and v['type']=='file')) for p,v in a.items() if v['type']=='directory' and p.count('/')==1 and p.startswith('build/')}
result=dict(baseline_members=len(a),final_members=len(b),unexpected_changes=errors,allowed_changed_files=changed,directory_metadata_only_changes=dirs,all_other_entries_exact_attributes=True,all_buildtrees_protected=builds,root_dotfiles_symlinks_and_prior_evidence_protected=True,exclusions=['.git','tests/native/v5-02-x06-a4-c01-evidence','tests/native/v5-02-x06-a4-c01-support','docs/vernacular/rounds/V5-02-X06-A4-C01.md'],baseline_sha256=sha(O/'baseline.json'),final_sha256=sha(O/'final.json'),candidate_sha256=sha(O/'candidate.json'),diff_sha256=sha(O/'candidate.diff'))
(O/'protection-result.json').write_text(json.dumps(result,sort_keys=True,indent=2)+'\n')
links=json.loads((O/'link-audit.json').read_text())
report=f'''# X06-A4-C01 corrective architecture author evidence

Combined A4+C01 documentation-only candidate awaiting independent review of
X06A4-R1; no self-approval or closure claim. No product, test or
build configuration changes; no builds, test execution, Planar tools, installed
binary, commit, installation, publication, merge or operator-data action.
The API sketch was not compiled and is not implementation evidence.

Shared checkout dev/planar-v2-vernacular, HEAD
3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a; intentional pre-existing dirty rewrite.
Only ten owned documentation files changed. The coordinator separately owns the
exact C01 round record. No unrelated edits were reverted.

## Candidate and checks

- Candidate JSON SHA256: `{sha(O/'candidate.json')}`.
- Exact preedit-relative diff SHA256: `{sha(O/'candidate.diff')}`.
- Original baseline SHA256: `{sha(O/'baseline.json')}`.
- Final full protection SHA256: `{sha(O/'final.json')}`.
- Original baseline entries: {len(a)}; final entries: {len(b)}.
- Local Markdown file links checked: {links['local_links']}; unresolved: zero.
- Nine pre-existing historical document bodies remain byte-exact; original
  preedit copies match the baseline. All ten documents existed before C01.
- All non-owned file/link entries preserve content, mode, identity, timestamps
  and size. Directory metadata-only changes reflect authorized membership/write
  activity; directory identity/mode and every non-owned member are preserved.
- Every build tree, root dotfile, symlink and all prior evidence is protected;
  [protection-result.json](protection-result.json) lists build member/byte counts.
  AGENTS.md remains a symlink to CLAUDE.md; compile_commands.json preserves its
  original symlink. No build tree was granted a mutable exemption.

Protection traverses with lstat, hashes every regular file, does not follow
symlinks, and records directory membership plus link targets. The only exact
exclusions are root .git, this C01 support/evidence pair and
`docs/vernacular/rounds/V5-02-X06-A4-C01.md`; no prior-round subtree is excluded.
Original A4 round, review, author/review evidence and support are protected.
One baseline and one final full-content pass were used; later report bookkeeping
uses the frozen manifests and rechecks only candidate/preedit/evidence files.

Executed commands: `python3 tests/native/v5-02-x06-a4-c01-support/protect.py baseline`,
`python3 tests/native/v5-02-x06-a4-c01-support/audit.py`,
`python3 tests/native/v5-02-x06-a4-c01-support/protect.py final`, and
`python3 tests/native/v5-02-x06-a4-c01-support/finalize.py`.
The audit constructs [candidate.diff](candidate.diff) from saved preedit bytes
and current candidates, hashes [candidate.json](candidate.json), checks
URI-decoded local links and preserved historical bodies/symlinks. Finalization
compares both full manifests, validates all candidate hashes and writes a
SHA256/size artifact manifest covering evidence and support. That manifest omits
only itself to avoid self-reference. The coordinator can record its final hash.

## Architectural result and limits

The correction adds the known selected workspace declaration-file metadata slot
alongside its original root before external registration. Bound source capture
requires original root, original declaration-file and registry receipts. Final
parse preserves the same owner; first read performs original-root discovery,
requires same_selection, verifies the actual opened leaf, and retains the original
file token through sealed capture, prepared ownership and native precommit.
Discovery/comparison retains existing cumulative metadata charges; failure poisons
original shared authority and bound operations without unbound retry. No content
read at metadata admission, new quota or product policy is added.

The source citations identify admission admit_root/admit_file(declaration),
retain_file/same_selection, native bound read and profile/capture forwarding.
Existing opaque extension root/file admission is late, after registration; only
its mechanism is reused. Known-load pre-registration timing is a future target.
Future evidence requires disjoint-directory module/declaration hard-link refusal
before any external chunk, replacement between registration and first read, and a
positive distinct-file registry-colocation case. The selected downward storage/
intent/source-capture graph, higher shared transaction consumer, original runtime
registration owner and private active invocation frame remain intact.

Original A4 candidate evidence and request-changes finding remain immutable. The
active architecture links X06A4-R1 and the C01 round; this authored candidate does
not decide whether R1 is closed. Fresh independent combined review is required.

Latest reader approval remains A3, current inventory 918, latest complete approved
macOS gate 876/configuration. Fresh profile acceptance remains refused and public
init creates format1. No new gate, shared budget, RSS, X07/X08, execution/RA01 or
Linux acceptance is claimed. Independent review is required before dependent code.
'''
(O/'REPORT.md').write_text(report)
evidence={p.relative_to(O).as_posix():dict(size=p.stat().st_size,sha256=sha(p)) for p in sorted(O.rglob('*')) if p.is_file() and p.name!='artifact-manifest.json'}
support={p.relative_to(R).as_posix():dict(size=p.stat().st_size,sha256=sha(p)) for p in sorted((R/'tests/native/v5-02-x06-a4-c01-support').rglob('*')) if p.is_file()}
(O/'artifact-manifest.json').write_text(json.dumps(dict(candidate=c,evidence=evidence,support=support),sort_keys=True,indent=2)+'\n')
print(json.dumps(dict(protected=True,baseline_members=len(a),final_members=len(b),candidate=sha(O/'candidate.json'),diff=sha(O/'candidate.diff'),report=sha(O/'REPORT.md'),manifest=sha(O/'artifact-manifest.json'),evidence_files=len(evidence),support_files=len(support)),indent=2))
