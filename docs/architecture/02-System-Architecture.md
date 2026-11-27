# Z02 System architecture

## Stakeholders and system boundary

OEM owns vehicle item definition, modes, system interfaces and hazard allocation. Tier 1 owns controller product integration, application policy and delivered ECU. Semiconductor vendor supplies hardware facts, startup, drivers and errata. Tier 2 supplies bounded components and device integration. ZARC maintainers own common mechanisms, contracts, conformance and platform release. Independent protection is a hardware/system concern; software must not be assumed to cover all faults.

The system context (D01) consists of vehicle mode authority, central high-performance services, trusted ZARC Host, one or more physical ZARC control domains, vehicle networks and physical endpoints, update/diagnostic infrastructure. Separate logical control, observation and maintenance planes; allocate physical resources and access per plane. Host downtime is an expected operating condition. No remote arbitrary GPIO/register/execution or watchdog-bypass interface.

## Architecture decomposition (D02)

| Domain | Responsibilities | Public dependencies |
|---|---|---|
| ZARC Core | Composition, local policy gate, lifecycle, resource ownership, supervision | Native Zephyr + domain contracts |
| ZARC Services | Power, edge, health, diagnostics, storage/update orchestration optional | Core and approved interfaces |
| ZARC Connect | Semantic APIs, bounded messaging, link/session providers | Service contracts, native UART/network/IPC |
| ZARC Host | Trusted broker, cache, VHAL/SDV/SOME-IP adapters | Connect contract, host OS APIs |
| ZARC Platform | Target composition, board integration, hardware capability manifest | Native Zephyr board/drivers |
| ZARC Product | OEM/Tier 1 policy, application, calibration and capability selection | Public Core/Services contracts |
| ZARC Assurance | Requirements/evidence, configuration and change impact | All published interfaces |
| ZARC SDK | Native build/test/simulation/integration workflows | Pinned toolchain and fixtures |

Dependencies: product applications import typed public services and Zephyr; service implementations import core contracts and domain device interfaces or native standard drivers; Connect implements transport bindings and depends on semantic contracts, not concrete services. Composition root binds implementations through explicit references; Core must not import service implementations, and a service must not import Connect. Host and target bind through versioned external contracts. Runtime data-flow arrows are not source dependency arrows. D02 illustrates allowed include/build dependencies separately from runtime dispatch. Target adapters map domain IDs to devices and may implement domain-specific interfaces but may not bypass local authority. Host depends only on external contract; no target headers. Pure decision logic may be host-tested with injected time and hardware facts without introducing a universal execution facade. Core must not include a vendor or product header. Dependency cycles and unowned outputs are build/review failures.

## Deployment views (D04–D07)

| Profile | Physical form | Mandatory profile intent | Major gate |
|---|---|---|---|
| Smart edge (E) | Small MCU near device | Local policy, control, health; minimal optional connect | Footprint, upstream loss, local timing |
| Companion (C) | Independently powered MCU beside SoC | Local power/state, supervision, link, host coordination | Startup/shutdown, reset and outage HIL |
| Zonal (Z) | MCU or RT SoC manages physical zone | Endpoint management, local authority, network mapping | Contention and endpoint fault domains |
| Integrated RT core (I) | Dedicated SoC RT domain | RT firmware, IPC, local device ownership and isolation | Reset, clock, shared memory and temporal isolation |

Passive endpoint may have no MCU; the controller manages it via a supported native or supplier device interface. The integrated profile requires evidence of reset domain, memory/clock/power independence, IOMMU/MPU/MMU, cache/coherency, DMA assignment and boot ownership. If the needed independence is unavailable, choose external companion instead. No guest or hardware family is inherently deterministic/safe.

## Authority matrix

| Concern | Decision owner | Observation owner |
|---|---|---|
| Vehicle desired mode | OEM vehicle mode manager | Mode manager |
| Device/rail transition | ZARC product-local policy and designated hardware owner | Measured hardware feedback |
| VM start/stop | Hypervisor/host orchestrator | Hypervisor |
| Application service availability | Service orchestrator | Service instance |
| Update eligibility | Vehicle update policy plus local conditions | Controller/host update agent |
| Safety reaction | Allocated system/product mechanism | Independent monitor as required |

Never collapse these into one global state machine. State across domains can temporarily differ; reconcile through snapshots and bounded coordination. Unique producer per physical capability or a reviewed arbitration mechanism; clients cannot grant themselves ownership.

## Interaction and failure design

Commands express typed, idempotent desired intent, not implementation actions. Policy validates supported capability, role, current state, eligibility and validity; controller publishes feedback-confirmed state. Accepted intent is distinct from completion. On link failure, reject stale requests, retain allocated local behavior, and mark remote observations stale/unavailable. Each boundary has a contract covering owner, version, bounded resource, timing, failure response and verification. The vehicle integrator specifies all actual safe states and temporal allocations.

## Authority conflict and fail-operational allocation

Define a single resource owner and an explicit arbitration policy where multiple authorized requesters exist. OEM policy must resolve conflicting desired modes, precedence and cancellation; no implicit last-writer-wins for consequential physical behavior. Local safety/protection constraints supersede external convenience requests. Independent hardware protection is not a network or software client. Document control ownership during reset, bootloader, shutdown and update, including pin default and feedback availability. If local autonomy is not available for a function, declare that function dependent and allocate its degraded response rather than claiming continued service.

## Architecture interface inventory

| Interface | Producer / consumer | Verification allocation |
|---|---|---|
| Product service | Product / local policy and device owner | Service contract and HIL |
| Target device | Native driver/target adapter / service | Board capability and porting tests |
| External semantics | Connect / Host or approved network peer | Contract conformance |
| Evidence descriptor | Build system / integrator | Identity and applicability audit |
| Independent protection | OEM HW / physical domain | System safety analysis and verification |
