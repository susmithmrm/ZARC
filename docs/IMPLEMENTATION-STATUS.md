# ZARC — implementation phase plan

**Current phase: requirements and architecture. Next phase: implementation and verification.**

The [single Z12 baseline](architecture/12-Requirements-Verification.csv) contains 267 specified requirements. The [derived trace](REQUIREMENT-TRACE.csv) assigns each an OPEN state. Source and tests must be assessed against the architecture before assigning implementation coverage. No source presence, simulator result, CI result or design review is counted as requirement acceptance.

| Workstream | Planned result |
|---|---|
| Source assessment | Actual code and test allocations mapped to Z12 |
| Component architecture | Typed composition, deployment identities and system graph |
| Observability | Authorized views, provenance and bounded resource measurement |
| Lifecycle | Version compatibility, state migration and recovery |
| Integration | Selected physical targets, peers, host and AI providers |
| Verification | Positive, negative, boundary and integration evidence |
| Assurance | Independent review for the exact product configuration |

Use [Z16 realization](architecture/realization/16-Implementation-Realization.md), [development allocation](architecture/realization/17-Development-Allocation.csv) and [Z09 verification](architecture/09-Verification-Engineering.md). Hardware support, vendor dependencies, independent qualification and production acceptance remain open.
