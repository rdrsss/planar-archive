# X06-A4-C01 corrective architecture author evidence

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

- Candidate JSON SHA256: `b748b0bfd7b8170c0dcc443931315ed7099bf7f049b544fe5942d096be1fc3a4`.
- Exact preedit-relative diff SHA256: `e8cb96b9f0049eca26d63500542ae874f25ffc0097215a424123c5303887d345`.
- Original baseline SHA256: `cbbedf2c845fd6aa8c3f41ecf3b1320f4b15c5b48dc931ff642d0fbae0767913`.
- Final full protection SHA256: `d158d29f348d36c6e8135029d21648e5286073e1b995686ce3f018114af6fe23`.
- Original baseline entries: 94226; final entries: 94226.
- Local Markdown file links checked: 552; unresolved: zero.
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
