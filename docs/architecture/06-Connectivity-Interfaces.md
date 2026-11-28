# Z06 Connectivity and interface specification

## Semantic interface first

The proposed ZARC Connect defines one semantic service contract and transport-specific bindings. No protocol or codec is the fundamental product identity. Stable service, capability, method/event and field IDs are controlled in an OEM-coordinated registry. The canonical schema describes types, units, ranges, versions, roles, effects, idempotence, timeout, result, observed outcome and resource bounds. Generated artifacts or reviewed mappings produce compact MCU bindings and host VHAL, SDV, CommonAPI/SOME-IP adapters. No transport callback directly owns an actuator.

| Operation | Inputs | Output |
|---|---|---|
| Hello | Supported versions and identity/session context | Negotiated version and session or refusal |
| GetCapabilities | Scope | IDs, versions, limits, status and build/config digest |
| GetSnapshot | Scope | Full current observed state, quality, time and generation |
| RequestDomainState | Domain ID, desired enum, request ID, validity | Accepted or refused; outcome separate |
| RequestMode | Supported mode, ID, validity | Local admission result |
| GetRequestStatus | Request ID in current or retired same-boot session from the same authenticated principal | Retained current-session outcome; UNKNOWN_OUTCOME for evicted prior-session result |
| Subscribe | Authorized topic and rate | Initial snapshot and bounded events |
| GetHealth | Scope | Readiness, faults and quality |
| GetUpdateStatus | Approved scope | Version/phase, no low-level execution |

Physical capabilities are product-approved. Do not expose arbitrary register/GPIO commands or raw actuator workflows. Identity derives from authenticated peer and local policy, never user-supplied role alone.

## Logical data dictionary

| Field | Candidate type/meaning | Authority |
|---|---|---|
| Capability ID | Stable logical ID; registry uniqueness | OEM/ZARC registry |
| Desired state | Explicit domain enum, finite valid set | Authorized client |
| Observed state | Value + VALID, STALE, UNKNOWN or FAULT | Controller feedback |
| Request ID | Opaque unique in session | Client |
| Boot epoch and session epoch | Boot identity changes on controller reset; session identity changes on reconnect; distinguish them | Producer/session owner |
| Generation | Monotonic state version within epoch | Controller |
| Timestamp | Declared monotonic unit and clock domain | Producer |
| Validity | Separate transit/age allocation and receiver-relative acceptance-to-execution bound; clock assumptions declared | Local policy |
| Result | ACCEPTED, COMPLETED, REJECTED, DENIED, BUSY, CONFLICT, INVALID, EXPIRED, INCOMPATIBLE, UNAVAILABLE, FAULT, UNKNOWN_OUTCOME | Responding authority |
| Build/config digest | Release and capability descriptor identity | Release manifest |

Exact field widths and encoding belong to the future versioned interface control document (ICD). The current simulated reducer has a single caller/session, monotonically increasing IDs and a one-result cache. It is not the external contract. Major mismatch blocks incompatible writes; minor changes additive only under explicit compatibility rules; unknown mandatory data or control enum rejected. No silent security downgrade. Define per-operation max payload, allowed role, deadline, request cache retention and rate bounds in approved schema. Idempotent desired state; duplicate within retention returns same result. On timeout, do not assume operation failed; query retained status/current snapshot. On reset or expiry reject stale work; client must resynchronize. D10 and D11 illustrate request/epoch semantics.

## Provider contract

Transport provider declares maximum message, delivery semantics, latency/resource budget, link/session state, asynchronous send/receive, readiness, statistics, bounded queues, shutdown/quiesce and reset behavior. Separate transport availability, authenticated session, negotiated capability and operational readiness. Per-link state instance; no mutable global session. Use explicit buffer ownership and bounded processing. Reserve capacity for control/health, admission and telemetry; throttle bulk transfer; no indefinite retries or unbounded storage. For streaming links, review frame bounds, corruption detection, recovery, backpressure and delivery. Reliability and safety E2E are separate from authentication and confidentiality; select vetted mechanisms and assess with OEM. This is a design contract, not production wire encoding/security design.

## Transport roadmap (D12)

| Transport | Target role | Required evaluation |
|---|---|---|
| UART | First point-to-point companion link | Board electrical ICD, native async UART, scheduling, flow, reset and recovery |
| Ethernet/SOME-IP | Zonal and external ECU interoperability | Standard wire binding, service discovery, network design and independent peer testing |
| Native IPC | Integrated RT domain or approved same-chip peer | Endpoint/backend, reset epoch, shared-memory and DMA/cache ownership |
| CAN/LIN | Selected vehicle bus or edge device adapters | OEM signals/diagnostics, bus scheduling, driver/PHY |
| SPI/I2C and vendor remote I/O | Local peripheral integration | Device driver semantics and fault behavior; not general network API |
| 10BASE-T1S | Candidate multidrop Ethernet edge | PHY/PLCA topology and end-to-end timing, link-loss behavior |

UART lab candidate 115200 bit/s, 8N1, maximum application payload 256 bytes; tested values to be established. Raw line capacity 11,520 bytes/s per direction, 100-byte encoded frame ~8.7 ms before other delays. Keep nominal telemetry below 30% initial line capacity and reserve headroom; no safety deadlines derive from these values. Compare compact framing/codec versus serial IP and custom SOME/IP encapsulation; latter is not standard SOME/IP interoperability. Select after measured complexity, footprint and interoperability. Physical UART ICD defines board voltage, pins/flow, clocking, maximum buffers, restart and reset ownership.

Standard Ethernet binding: OEM-assigned service/instance/method/eventgroup IDs, version mapping, standard serialization, TCP/UDP choice and discovery. Service discovery does not establish authorization. Candidate COVESA vsomeip on host [S13](13-References.md#s13), LIBERSOM on RT [S17](13-References.md#s17) subject to upstream fit, license, maturity and measurements. Native IPC uses Zephyr IPC instances/endpoints with backend-specific constraints [S07](13-References.md#s07); verify memory ownership, coherency and reset. Preserve same semantics independently of transport; do not expose private serial envelope as public Ethernet API.

## Transaction and freshness semantics

Define service-specific command class, idempotence, accepted-work retention and terminal results. Deduplication key includes authenticated principal, negotiated session, service/operation and request ID. Reuse of an ID with a different request is refused; no promise of exactly-once delivery or execution across reset. Distinguish transport acknowledgment, semantic acceptance and observed completion. Bound outstanding work and dedup/result storage; reject new requests if capacity cannot guarantee contractual retention. After result eviction provide UNKNOWN_OUTCOME and reconcile snapshot, not a false negative. Accepted local work continues or is cancelled according to approved domain policy; session reset does not imply physical rollback. Record correlation and outcome even when peer disconnects if the product requires it.

For freshness distinguish producer's clock, transport/broker queue age and local policy clock. Monotonic timestamps across devices cannot be subtracted unless synchronized with uncertainty bounded. Receiver-relative timeout alone does not detect age accumulated before receipt. Define validity start, maximum transport age and synchronization/uncertainty requirements; if required freshness cannot be established, refuse intent or report observation UNKNOWN/STALE. Keep boot epoch separate from link/session epoch; generation is scoped to producer boot. Define event sequence, gap detection and full snapshot barrier: subscribe then snapshot/generation reconciliation prevents race between snapshot and events; a gap or unavailable producer invalidates freshness until reconciled.

## UART provider admission

Zephyr offers polling, interrupt-driven and asynchronous driver APIs; support and DMA behavior are target-specific [S29](13-References.md#s29). Do not enable both interrupt-driven and asynchronous callbacks on the same peripheral. Separate production link from console/debug, assign exclusive peripheral ownership, test buffer lifetime/completion, RX/TX reset and flow bounds. If async is unsupported, evaluate bounded interrupt-driven path against the same latency/resource contract or disallow target; do not assume universal DMA support. 115200 raw capacity assumes 8N1 full-duplex per direction; overhead, authentication, framing, flow stalls, half-duplex and retries reduce usable throughput. For each service compute wire bytes and worst burst; do not claim 100 ms acceptance unless measured and analytically allocated.

## Binding mapping and security ownership

Transport providers may differ: byte stream, message, RPC or bus binding. Each publishes a mapping of delivery/ordering, size, backpressure, discovery and failure semantics. Separate transport availability from authenticated peer identity, then capability negotiation and authorization. Where broker terminates a client session and creates an MCU session, the approved delegation model must define how client identity and authority are asserted/limited and audited. Do not imply end-to-end identity preservation simply from two secure channels. Exact codec, cryptographic protocol and credential scheme remain open and require established vetted mechanisms and independent assessment.

## Optional network technologies
CAN XL, FlexRay, J1939, ISOBUS, hardware switch/offload, multi-gigabit Ethernet, PCIe and vendor IPC are selected bindings or external gateway integrations. SENT and PSI5 are native peripheral candidates. No additional mandatory transport stack.
