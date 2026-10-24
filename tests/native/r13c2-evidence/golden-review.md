# Full expected interpretation review

The captured Glaze JSON was expanded in `golden-review.txt` and read in full
before freezing fixtures. The temporary capture branch was removed from the test;
final tests only read goldens and compare them byte for byte.

Reviewed normalized values retain all seven bundle fields: workspace, spec,
registry, jobs and three logical declaration filenames. Both combined/split
jobs retain every role, shape and budget/default. Workspace retains registry
routing, root exclusions, toolchains/checks and notes. Spec retains constraints,
contracts with exact referenced API bytes/review note, dependencies, facets,
frozen paths, goal, milestones/acceptance, prescriptions, empty questions,
rationale and complete work/check/path definitions. Registry retains complete
checks and toolchain argv/environment/coverage/limits. Set normalization sorts
integration check inputs to output_vector, own_output; raw registry provenance
continues to retain the original exact byte order separately.

Reviewed semantic values retain both resolved dependencies and all fields of
both jobs: assignments, effective checks, graph vertices/edges, job ID/file,
empty pipeline predecessors and independent reviewer. The command's effective
checks include contract and integration, while library retains contract. Combined
maps both work units to builder; split maps command/library to their separate
workers. Graphs retain role/work phase edges, cross-work complete obligation and
review acceptance edges. No captured implementation topology or DB IDs appear.

The versions object has exactly the approved ten fields. The digest
283ba8df21db643ea982b7511c7b95bd is the internally prepared portable stdlib tree
digest after deliberate acceptance.lua/bootstrap changes. It is asserted against
prepared embedded provenance, not supplied as a caller credential. JSON is from
Glaze; binary fixtures are the original canonical codec output. Separate tests
compare complete existing normalize/validate results, so a field-loss error in
the wrapper is not masked by simply capturing a new golden.
