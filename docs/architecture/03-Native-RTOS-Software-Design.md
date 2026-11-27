# Z03 Native RTOS and software design

## Native kernel strategy

ZARC targets native Zephyr applications, libraries and modules. The shared behavioral library is portable C17 and is developed first on Windows and Linux. Target-specific execution adapters provide native Zephyr scheduling and device access. The following RTOS requirements apply to the embedded target, while the desktop reducer uses injected ticks and observations. Use native k_thread, k_work, k_msgq, k_fifo, k_sem, k_mutex, k_timer, k_mem_slab, device model, GPIO, UART, CAN, networking, power management and flash APIs as appropriate. Evaluate zbus for explicitly bounded event exchange and SMF for local state logic. zbus publication may run synchronous listeners in the publishing context, lock channels and deliver observer notifications; it does not guarantee nonblocking dispatch or one message per subscriber read [S25](13-References.md#s25). Use K_NO_WAIT where permitted, prove notification/copy and observer bounds, or isolate publication in a worker. Do not use zbus as the mandatory critical control mailbox. A direct `k_msgq` or fixed owned buffer is the initial candidate for loss-sensitive paths. Prefer upstream Zephyr facilities; an adapter is permitted for domain semantics, hardware errata or a third-party provider, not to mirror all kernel APIs. The pinned Zephyr release and supported API subset are decided at G0/G1; current documentation is not a contractual version [S01](13-References.md#s01), [S02](13-References.md#s02), [S03](13-References.md#s03), [S04](13-References.md#s04), [S05](13-References.md#s05), [S06](13-References.md#s06), [S07](13-References.md#s07).

## Execution domains (D03)

| Class | Context | Work | Restrictions |
|---|---|---|---|
| I0 | Interrupt/deferred acquisition | Timestamp and signal bounded samples; minimal acknowledgement | No blocking, flash, network parsing or dynamic allocation |
| R1 | High-priority preemptible threads | Local real-time control and mandatory feedback | No host wait; static resources; bounded driver calls |
| R2 | Supervision/policy threads | Health and lifecycle transition evaluation | Bounded shared state; no synchronous remote callbacks |
| C3 | Dedicated connectivity workers | RX/TX and typed dispatch | Cannot directly manipulate actuators; capped work per activation |
| O4 | Observation/diagnostics workers | Telemetry, fault persistence, log draining | Drop/coalesce according to policy; no critical blocking |
| M5 | Maintenance workers | Updates, configuration staging, noncritical administration | Eligible mode and bounded resource use; separate from R1 |

Actual Zephyr priorities, cooperative/preemptive selection, interrupt priorities, SMP affinity and scheduler choices are target configuration decisions. Treat zero-latency or cooperative code as exceptional and analyze starvation. Never rely on a priority number alone for a deadline. Do not place unbounded application work on the shared system work queue; dedicate queues for isolated task classes. Timer callbacks signal work; driver completion callbacks transfer ownership and return. All tasks declare context, period/trigger, deadline, WCET estimate, stack, queue, maximum blocking, failure handling and dependency.

## Task and service execution specification

Periodic tasks use monotonic deadlines rather than accumulating delay drift; define skipped/late cycle disposition per functional allocation. Push-based acquisition carries a sequence and timestamp; control validates freshness before using samples. One writer per device state. Shared snapshots immutable after publication; copy or transfer ownership via a bounded queue, not raw mutable pointers across domains. Control commands enter policy mailbox and cannot call hardware from communication callback. Mutex only for bounded shared state; document lock order and priority inheritance behavior for selected configuration. No holding locks across I/O, remote requests, logging, callbacks or persistence. Use atomics for well-defined small shared state; memory ordering and SMP reviewed. Use separate task-local scratch memory.

## Timing analysis and budget

For fixed-priority uniprocessor periodic/sporadic work, analyze R_i = C_i + B_i + sum over higher-priority j of ceil(R_i/T_j)*C_j, extended for release jitter and ISRs/shared buses. Require R_i <= D_i under agreed workload and hardware conditions. Model DMA, cache, bus contention, flash operations, interrupt masking, context switches, clock and low-power transitions. SMP and mixed criticality need a separate response-time method. Instrument cycles and external traces; observed maxima alone are not proven WCET.

| Example G1 laboratory item | Provisional target | Measurement |
|---|---|---|
| Benign periodic sensing/control | 10 ms period/deadline | Timestamp release/completion, stress workload |
| Intent acceptance/refusal | <=100 ms nominal UART path | Host/MCU correlation, actual load |
| Observation | 10 Hz, explicit age/status | Snapshot/event cadence |
| Stress | No allocated local misses in 24 h | Hardware trace and missed count |
| RAM/flash margin | >=20% after measured stacks/image | Map/stack watermark and runtime pools |
| Nominal queue occupancy | <80% | High-water metrics, burst test |

Targets are design hypotheses for benign bench usage, not measured, vehicle limits or safety FTTI. System integrator allocates actual task timing, response and hardware conditions. A passing 24 h test cannot establish safety or absence of future misses.

## Bounded resource model

Static pools and queues on critical paths; no unbounded heap after initialization in controlled domains. Use k_mem_slab where fixed blocks are suitable, k_msgq for bounded values, single-owner buffers for DMA. Define max payload, queue depth, enqueue wait and overflow policy per service; high-priority capacity reserved from bulk traffic. Reject excess control intents with BUSY, coalesce replaceable telemetry, retain critical fault indication according to policy, throttle bulk maintenance. Do not silently discard accepted operations. Account for per-context stack, buffers, thread objects, DMA alignment, descriptor rings, logs, flash partitions and reserved headroom. Publish measured vs budget by board and feature set. Allocation failure produces explicit status/degraded behavior; no infinite retry.

## Fault containment and isolation

Where supported, use Zephyr user mode, memory domains and least-privilege object permissions for untrusted application code [S04](13-References.md#s04). Supervisor/kernel threads remain trusted in this model; MPU/MMU permissions do not prove temporal isolation or DMA separation. Restrict privileged interfaces, verify memory map, stack guards, DMA regions, cache/interrupt ownership and resource budgets. If mixed criticality cannot be credibly isolated, separate images/domains/controllers or exclude unsupported configuration. Hardware protection and OEM safe states are independent requirements.

## Health and fault handling

Distinguish alive, deadline and logical supervision. Each mandatory service publishes independently validated progress; a central health supervisor checks required reports and local feedback and uses native watchdog facilities according to approved policy. Do not feed watchdog unconditionally from periodic timer or remotely disable it. Watchdog expiry is not universal safe reaction. Define reset causes, retained diagnostic information, startup consistency checks, bounded recovery attempts and escalation; actual reaction and reset strategy from safety and hardware requirements.

## Native mapping and build controls

| Need | Upstream candidate | ZARC responsibility |
|---|---|---|
| Scheduling and sync | Native kernel primitives | Thread/priority/budget contract |
| State/event model | SMF, zbus | Domain transitions and ownership |
| Peripheral and low power | Device model, devicetree, PM | Logical-to-physical mapping, verified sequence |
| Storage | Settings/NVS/ZMS/flash as applicable | Atomicity, retention and migration policy |
| Network and IPC | Native Zephyr stack/API | Service contract, transport conformance |
| Firmware | MCUboot/sysbuild where supported | Eligibility, compatibility and verified recovery |
| Verification | ztest/Twister/native_sim | Product fixtures, HIL and evidence |

Use Kconfig to exclude unused capabilities at compile time. Scope compiler optimization, debug permissions, asserts, logging, memory and kernel options by profile. Freeze final .config, devicetree output, image manifest, compiler flags and map for supported configurations. Release review rejects hidden target flags, unresolved capability dependencies, unexplained budget changes and unstable upstream APIs without justification.

## Scheduler and context decisions

Zephyr uses priority-based scheduling. Lower numerical priority means higher thread priority in the conventional configuration; cooperative and preemptive ranges are configured. Deadline scheduling, if enabled, refines ordering among equal static priorities, not general EDF across all threads [S26](13-References.md#s26). G1 favors uncomplicated fixed-priority preemptive execution on one core. Select exact priorities only after task allocation and analysis; document all kernel and vendor-driver threads, interrupt priority ceilings and callbacks, not only ZARC-created threads. Timer expiration and driver callbacks may run in interrupt or driver-defined context; allow only API calls permitted in that context. Never assume callback means thread context. System workqueue handlers can block later items; submit is not a counting event mechanism because already queued items may coalesce [S27](13-References.md#s27). When every occurrence matters, use a bounded queue or counter and test overflow. Work items and their data remain valid until cancellation/completion is synchronized; no reuse while pending.

## Timing and multi-core analysis detail

Inventory C (execution bound), B (blocking), T (period/minimum interarrival), D (deadline), J (release jitter), priority, affinity, interrupt load and shared-resource effects as separate values. A usable single-core response-time model for constrained deadlines with fixed priorities has R_i as an iterative bound, e.g. R_next = C_i + B_i + interference(R_current); the interference term must include higher-priority arrival jitter and separately bounded interrupt/device effects. If convergence exceeds D_i, the task configuration is not admitted. Do not apply this model where arbitrary deadlines, nonpreemptive regions, self-suspension or SMP invalidate its assumptions; select an appropriate analysis or conservatively partition and prove bounds. Define measurement clock resolution, wrap handling, missed-period detection and trace overhead. Provisional 10 ms/24 h/20% targets apply to G1 reference companion lab and are not imposed on every profile.

## Runtime power management and shared resources

Use native device runtime PM only where selected driver and dependencies support it [S28](13-References.md#s28). Balance device-use references and manage approved system-PM constraints; account for wake latency, clock transitions, interrupt availability and DMA completion. A Zephyr active/suspended device state does not establish vehicle rail state or safety. Do not allow automatic sleep to invalidate an allocated local deadline or retained protection. Validate idle-to-active and suspend/resume behavior under target conditions. On SMP or heterogeneous platforms, review cache/DMA ownership, noncoherent memory, per-core interrupts, barriers and bus interference with SoC provider. No claim that memory protection alone ensures temporal isolation.
