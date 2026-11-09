# C06 invariants and mutation obligations

Tests named below are proposed C06 tests, not executed evidence. Existing reviewer red scripts are seeds; their collection-only diagnostic command is not a correctness test. All CLI tests must use explicit candidate binary and disposable roots/ledgers, assert actual diagnostic JSON, ledger invariance when refusal is pre-reserve, and be registered in CTest.

| ID/finding | Invariant/test obligation | Exact fault to probe |
|---|---|---|
| I1 F1 | Malformed verdict grammar emits field verdict; nested shape emits exact verdict path; normalize invalid semantic path retained. CLI + existing native verdict tests. | Drop field in parse adapter, separately drop field in normalization adapter. |
| I2 F1 | Reserved PLANAR_ environment emits reference/registry; scratch overlap emits path_denied/scratch; no reserve/events. | Blank each check error field independently. |
| I3 F1 | Scratch mkdir/open failure after reserve emits io/scratch, leaves reservation recoverable. Native deterministic fixture + CLI when reliably injectable. | Omit scratch field on run_checks scratch creation arm. |
| I4 F1 | Missing/different/inline expected contract emits revision_conflict, expected verified active revision, actual working_bytes, before reserve. | Drop delivery marker/context or replace expected with registry revision. |
| I5 F1 | Two mismatching producer outputs name lexicographically first work, no reserve; producer without completion is not prechecked. | Remove work field; reverse producer iteration; precheck absent completion. |
| I6 F2 | Different request during open check gets exact message; same request still check_in_progress; unequal same ID request_conflict; original child finishes. | Remove check_in_progress helper arm. |
| I7 F3 | Same-job activation resets every lineage: consumer blocked, facet unavailable, claim refused; new producer completion restores readiness. | Pass S.publications to activate material. |
| I8 F3 | Switch jobs sharing facet IDs cannot inherit old job publication. | Filter activation only by facet/work without job/reset. |
| I9 F3 | Producer rename/removal/replacement cannot publish through retired owner; preview/status agree, claim refused until new owner completes. | Remove native non-retired/current-owner filter. |
| I10 F3 | Same role structural replacement empties lineage before all consumer material calculations, including consumer-first declaration order. | Retain publications for reset role in hypothetical load view. |
| I11 F3 | Blocker clearing uses post-load filtered view and exact R18 target/policy comparison; obsolete publications cannot incorrectly clear or retain a blocker. | Rebuild clear_blockers snapshot from S with only revision replaced. |
| I12 F3 control | Takeover carries old completion/publication; material-only change preserves definition of current publication; no producer-only fingerprint added. | Filter publications to exact generation rather than lineage; require applicable instead of current completion. |
| I13 history control | Previous persistent completion/publication history and appropriate recovery remain readable; retired completions are absent from current snapshot; published payload integrity still verified even when not current. | Skip verify_publication before current filter; retain retired completion in current snapshot; separately delete/hide persistent historical rows from history reads. |
| I14 adjacent audit | Current observed/evidence use active owner lineage; retired role's later sequence cannot overwrite a newly owning producer's current observations. | Omit active-owner gate in observed/evidence selection (if correction needed), or reintroduce obsolete output through checks.lua completion fallback. |

Carry previous C01–C05 probes only by unchanged source hashes. C06 Lua changes invalidate embedded-byte budget calibration; rerun measurement and relevant budget probes after source freeze. No public fault hooks are authorized for observability. Unreachable diagnostic construction (e.g. scratch creation race) should use native temporary fixture rather than a flaky race.
