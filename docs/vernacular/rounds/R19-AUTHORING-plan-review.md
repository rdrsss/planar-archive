# R19-AUTHORING plan review

Disposition: **approve** the bounded implementation and verification plan.
This approves no implementation, gate result, mutation result or commit candidate.

Reviewed baseline: `d6c989ed9e196bf9b357019ea946e6ff18c47f18`, on
`dev/planar-v2-vernacular`. Fresh ordinary reviewer; the available current model
substitutes for the unavailable prescribed Claude plan-review model. The shared
checkout was read only except for this owned report. No build, candidate CLI,
ledger or operator database was invoked.

## Reviewed inputs and authority

Read the repository instructions, implementation/protocol and roadmap openings,
v5 scope and architecture, the round brief and source map, and the canonical
`tests/native/r19-authoring-evidence/plan/DECISIONS.md` and `INVARIANTS.md`.
Checked the proposed repair against command-contract retained lifetime,
authoring RA-1/RA-9, execution EX-6, D108/D109/D134/D142/D148 and the external
profile pointer. Inspected the current bootstrap, authoring CMake transform,
execution handler interfaces, invocation interface, all four generator families,
X08 target wiring, B2 callable-replacement process test and RA01 source/profile
guards.

The defect is a direct violation of the existing command contract: after catalog
admission bootstrap must bind and execute the retained callable in that same VM
(`command-contract.md:175-189`). The twelve execution branches omit the optional
VM, while load already passes it. Authoring uses a different compiled bundle, so
the fresh production VM reaches the correct EX-6 refusal. Draft DD27 explicitly
deferred extending the generated compositions; it does not override this exact
contract. No new interpretation exemption or product decision is necessary.

The generated stacks own distinct C++ runtime option and VM types. Copying the
twelve thin handler modules through the existing ordered, reversible composition
mechanism is an appropriate typed repair. It preserves family-owned CLI11 option
state and intent construction without adding another semantic dispatcher. The
same common bootstrap change necessarily reaches B2, D7, X07 and X08 copies, so
updating their generation, target dependencies and audits is within the repair.
The runtime-only malformed authoring variants need no handler-copy expansion.

## Acceptance and package assessment

All twelve invariant rows have a concrete failure mechanism and an observation
route. I1/I2 address the actual VM/callable defect; I3/I4 preserve independent
digest refusal and replay precedence; I5/I6 require successful lifecycle and
recovery operations; I7/I8 preserve build/profile exposure; I9/I10 preserve typed
composition and parser/error boundaries; I11/I12 cover build and evidence claims.
The decision table distinguishes contract requirements from the engineering
choice of how to compose types. D10 correctly returns any new product or
authority choice to the operator instead of implementing it implicitly.

Sequential test-first and implementation packages are feasible. CMake and B2
generator ownership transfers only after the test-first package finishes; the
implementation package must preserve its new assertions. The coordinator's
workspace, driver and records ownership is separate from the product repair.
Existing CMake trial source-freeze and graph-equality checks must not be reused
as acceptance policy for this different task.

No blocking plan findings remain. The following are execution requirements of
the approved plan, made explicit for the briefs and final review:

1. Retain a fresh baseline assertion failure at authoring activation and a
   distinct execution-callable replacement control after identical CLI11 parse.
   Compilation errors, an earlier usage failure, prior trial logs and zero test
   matches cannot substitute. Negative controls that already pass at baseline
   should be recorded as controls; they need not be made artificially red.
2. Inventory all twelve embedded branches. Prefer successful semantic operations
   where available; report any structural-only family coverage honestly. A
   substring check alone must not be presented as an end-to-end operation.
3. Preserve and audit ordered module-prefix, namespace and fully qualified name
   rewrites. Include execution option vocabulary where the invocation copy uses
   unqualified names. Keep the protected real-runtime import and exact reverse
   restoration checks; do not use a cast or bind generated options to the original
   runtime owner. Copied handlers require no new authoring macro site.
4. Namespace changes can also redirect with-profile handler calls. Require
   authoring-versus-production byte parity for actual execution-profile paths,
   in addition to existing RA01-tail error precedence and help parity. The
   profile factory remains closed and excludes authoring. No profile admission
   amendment is included in this plan.
5. Cross-bundle negatives must require exact failure and no ledger mutation.
   Exact saved activation/check/verdict requests must replay before binding,
   byte-identically and without check re-execution or extra events; changed
   bodies must conflict. Bound status and preview must retain EX-6, while exempt
   reads retain their existing behavior.
6. Complete gates follow the focused regression and coherent lifecycle, with
   retained nonzero inventories/JUnit, zero unexplained skips, pinned formatting
   and Doxygen, copied-target exclusion and a no-change build with no target
   work. Profile/composition coverage is part of the gate, not optional because
   production previously hid the VM defect. Host results do not establish a new
   Linux gate or close the rest of R19.
7. The initial bootstrap repair necessarily precedes a successful authoring
   activation/claim. Label that sequence explicitly: the broken baseline cannot
   dispatch this repair through its failing path. The repaired candidate's later
   ledger lifecycle proves coherent validation/review, not pre-code dispatch.

Proceed with the authorized test-first package and fresh reproduction. Final
source/evidence review remains required before the signed-off local commit;
pushing, installing, publishing and merging remain unauthorized.
