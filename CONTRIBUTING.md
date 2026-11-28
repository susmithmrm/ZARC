# Contributing to ZARC

Contributions are reviewed through GitHub pull requests. A maintainer checks the
change against the authoritative [Z12 requirements register](docs/architecture/12-Requirements-Verification.csv),
the architecture boundaries, and the evidence recorded in
`docs/REQUIREMENT-TRACE.csv`. The register is the only requirements baseline;
do not create extension or duplicate requirements lists.

## Prepare a change

1. Keep the native Zephyr implementation and the portable C17 core boundaries
   intact. Add target-specific behavior in a native BSP or thin adapter.
2. Identify affected requirement IDs and update their existing trace rows when
   implementation or evidence changes. Keep hardware, vendor, and independent
   qualification claims open until their evidence is available and reviewed.
3. Add focused positive, negative, boundary, and integration tests where they
   apply. Run the relevant CTest or documented target test and include the
   exact command and outcome in the pull request.
4. State compatibility, migration, resource, security, and evidence limits.
   Do not describe simulator results as physical qualification or product
   support.

## Review and decisions

The maintainer owns merge decisions and requirement-trace changes. Reviewers
check implementation scope, test evidence, and whether open evidence remains
accurately identified. Changes that alter the architecture or acceptance basis
need an explicit decision in the existing architecture decision records before
the implementation is treated as accepted. Review comments and the merge
decision remain attached to the pull request for traceability.
