# Stabilization increment 1

- Host loopback rediscovery is transactional and clears stale capabilities on invalidation. The process test reconnects, verifies a new session and rejects old result correlation.
- Composition reports UNAVAILABLE when any mandatory service is not ready, DEGRADED for optional failures, and preserves admission for independently ready services. Required readiness is queryable separately.
- Guest aggregate readiness expires at the exclusive coordination deadline, even after READY; fallback is a policy notification, never an electrical action.

Regression tests extend existing fixtures. These changes do not close product requirements or replace target lifecycle integration. Relevant requirements: SYS-006, SVC-003/013, HOST-002/008, CON-009. CI must verify the commit.
