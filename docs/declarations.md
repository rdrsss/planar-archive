# Declaration reference

The authoritative [declaration contract v1](vernacular/declarations.md) specifies
workspace, spec and job constructors, schemas, IDs, paths, materiality, shapes,
requirements, contracts, checks, grouping and ownership validation.
The [render contract v1](vernacular/render-contract.md) specifies captured inputs,
registry and observation JSON, packets, identities, diagnostics and budgets.

Each text-only Lua declaration returns one immutable constructor value using
the curated `planar` namespace. Lists use `planar.list`, including empty lists.
Declarations have no I/O, process, dynamic loading or standard-library access;
`planar.file` registers confined captured references rather than granting reads.
Trusted embedded Lua validates and projects retained values separately.

Start with the [quota workspace](../fixtures/render/quota/workspace/workspace.lua),
[spec](../fixtures/render/quota/workspace/spec.lua), and
[fixture guide](../fixtures/render/quota/README.md). The combined and split jobs
share one spec while assigning its units to different worker groupings and an
independent reviewer. The fixture distinguishes authored symbolic expectations
from [computed native packets](../fixtures/render/quota/native/README.md).

[V5](vernacular/planar-design-spec-v5.md) supplies broader product direction;
these exact approved schemas continue to govern current behavior until scoped
reviewed amendment. Its abbreviated constructors, registry maps and examples are
illustrative. [Alignment](vernacular/v5-alignment.md) records pending V5-02
workspace/command/trust choices and RA01 authoring prompts; neither adds schema
fields or extension capabilities in this documentation round.
