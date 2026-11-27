# Z05 Service architecture and lifecycle

## Service contract and composition

Each service declares stable ID, semantic version, supported capability/profile, required/provided dependencies, owners, allowed callers, execution class, resource bounds, readiness/health semantics, initialization, start, quiesce, resume and stop behavior, diagnostic codes and conformance tests. Product selects required services; optional unavailable service never masquerades as healthy. A compiler/build-time dependency graph rejects missing mandatory dependencies, duplicate IDs, cycles and impossible budget allocations. Freeze registry before accepting operational work; dynamic discovery reports capability, not dynamic code or permission. Avoid runtime service discovery on time-critical local paths.

| Service | Scope | Initial disposition |
|---|---|---|
| Core composition | Static dependency/ownership, readiness and resource registration | All profiles |
| Local policy | Typed intent admission, state/invariants, role and eligibility | All profiles |
| Health | Alive/deadline/logical monitoring and fault status | All profiles |
| Power | Domain intent and observed lifecycle, approved dependency graph | Companion, optional others |
| Edge | Sensor/endpoint inventory, samples and typed benign device capabilities | Edge/zonal |
| Connect | Session, semantic request/event, transport dispatch | As selected |
| Diagnostics | Fault reporting, trace and dataset access; optional UDS/SOVD mapping | Product-dependent |
| Update coordinator | Eligibility, image/version status, recovery orchestration | Program-dependent |
| Vehicle network | Signal/service mappings over supported buses | Program-dependent |

## Startup and shutdown (D09)

For a selected embedded configuration with verified boot, the boot chain verifies the approved image. The desktop starter has no boot chain or service registry. Zephyr initializes native devices; ZARC registers capabilities only after device readiness, validates config and required dependencies, initializes selected services in dependency order, starts mandatory control, checks feedback and publishes readiness. A service start failure rolls back only initialized resources in reverse dependency order; if mandatory, report degraded/unavailable and apply allocated fault behavior. Host handshake is not a prerequisite for local allocated operation. No unbounded retry or startup wait. Warm boot reconciles actual outputs and retained state; never assume previous intent equals physical fact.

Service lifecycle: REGISTERED → INITIALIZED → STARTING → READY → QUIESCING → SUSPENDED or STOPPED. A suspended service may RESUME through revalidation to READY. A fully stopped service must initialize/start again before READY; it cannot follow the suspend-only resume path. Any transition can report DEGRADED/FAILED with cause. Quiesce blocks new intents except approved exceptions, drains/cancels outstanding work with explicit result, persists permitted state, releases owned resources in reverse order. Resume revalidates dependencies and actual state; increment generation where applicable. Reset invalidates session assumptions. The controller state machine and host/guest/service states remain independent.

## Coordinated power and local control

Use domain-specific intents such as desired operating mode or desired state of an approved lab domain. Domain resource ownership is explicit, with dependency ordering and observed feedback. Do not encode vehicle-specific rail sequencing, electrical levels or safe states in generic platform. An OEM/Tier 1 power policy and HW design own those allocations. Controller eligibility includes local interlocks and confirmed readiness as applicable; it may refuse a host request. All timing/timeout/fallback actions are approved per item, not generic platform defaults. Observed feedback may be UNKNOWN, STALE or FAULT; a successful request acknowledgment is not physical confirmation.

## Observability and diagnostics

Structured record fields: event code/version, originating service, source epoch, monotonic timestamp, severity, occurrence counter, quality, correlation ID, config/build digest and bounded context. A durable fault store has wear, capacity, retention and integrity requirements; reserve fault summary independent of bulk logs. Use native logging with rate limits and deferred output; don't block control on debug transport. Counters include queue high water, deadline miss, link status, dropped/coalesced observation, reset reason, service readiness and update phase. Export diagnostic and health views by approved access policy; don't log secrets or unnecessary personal data. UDS service and DTC allocation optional and OEM-defined; SOVD is a host-side integration candidate [S18](13-References.md#s18). Maintenance/test features are excluded from production images by default and require explicit policy/evidence where applicable.

## Persistent data and update lifecycle

Separate immutable defaults, approved calibrations, runtime retained state and diagnostics. Define schema/version, integrity, migration, atomic update, write endurance, recovery on interruption and authorized mutation. Invalid configuration enters approved unavailable/recovery state, not silent arbitrary defaults. Detect reset cause, reject stale epoch, and reconstruct authoritative snapshots. Boot/update design is detailed in Z08.

## Transition contracts and ownership

| Transition | Preconditions | Completion / failure |
|---|---|---|
| Register | Selected service, valid ID, declared dependencies | Descriptor exists; not operational |
| Initialize/start | Dependencies and resources ready | READY only after local health and feedback; rollback acquired resources or preserve allocated protection |
| Quiesce | Authorized coordination, bounded deadline | Reject new non-exempt work; each accepted item resolved, cancelled or retained as unknown |
| Suspend | Quiesced; allocated retention and wake capability | Resources retained only as declared; report state |
| Resume | Suspended and approved wake, revalidation | READY when dependencies and observed state verified |
| Stop | Quiesced; approved release of resources | STOPPED; next start reinitializes |
| Fault/degraded | Fault detected within allocation | Function-specific response, reason, availability; recovery via approved path |

Define service-specific cancellation semantics: cancel request acknowledgment is not proof an already started physical operation was reversed. Completion must reflect actual outcome or UNKNOWN_OUTCOME; on host restart do not overwrite authoritative local status. Establish service lifecycle completion timeout, readiness and failure reporting. A mandatory fail does not automatically stop unrelated functions with independent allocations.

## Silicon-dependent lifecycle
Reconcile reset cause and retained data validity before service readiness. Use native storage with approved technology-specific endurance and interruption assumptions. Engineering calibration interfaces such as XCP/A2L are optional and subject to allocated authority.
