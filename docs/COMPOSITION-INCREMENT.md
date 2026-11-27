# Service composition increment

Development branch implementation; not approved product evidence.

The registry is immutable while active, bounded to 16 descriptors and eight dependencies per descriptor. Topological ordering validates dependencies before any callback. Independent services remain available when another service fails startup. A failed start releases its own initialized resources; previously healthy independent services remain active. Shutdown releases acquired resources in reverse order. Callbacks are single-owner, synchronous and must be bounded by the integrator. Release must handle partial failure, including physical quiescence; generic code does not allocate safe electrical behavior.

Quiesce closes all admission before draining in reverse order. Failure prevents suspension and marks unprocessed services unavailable; stop invokes the product release policy. Resume requires suspended state, dependency readiness and callback revalidation. A stopped service must initialize again.

Trace candidates: SVC-001/002 (registry and graph); SVC-003/004/013 (startup failure isolation, release); SVC-005/006/011 (lifecycle transitions). Tests: tests/composition_tests.c. These are partial fixtures: no declared budgets, profile-level readiness, enforced callback timing, durable fault handling or physical evidence. All requirement statuses remain proposed.

No local full-repository build was possible in the authoring environment. GitHub CI is required before merge.
