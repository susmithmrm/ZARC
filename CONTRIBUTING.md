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

## Design and implementation gates

During the design phase, keep documentation aligned with Z12 without changing implementation evidence. Implementation-phase submissions must use exact approved applicability and allocation, with configuration-specific acceptance and no parallel baseline. Keep source-independent component contracts, native configuration, deployment policy and authorized visibility consistent with the system graph. Report unresolved hardware, vendor and reviewer dependencies rather than claiming closure.

## Governance

The maintainer documents the decision, accountable owner, review scope and applicable independence; disclose conflicts of interest and seek independent review when required. Record delegated maintenance and succession responsibilities before a supported release. Confidential customer material belongs to its authorized product workspace, not public issues. Use Z10 and ADOPT-039 for governance and support review.
