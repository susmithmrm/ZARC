# Z03 — Native real-time micro-architecture

> **ZARC 2035 | Architecture specification | Design phase.** The single requirements register is [Z12](12-Requirements-Verification.csv) (267 requirements). Implementation and verification are planned for the next phase. See [implementation plan](../IMPLEMENTATION-STATUS.md).

## Execution model
Portable C17 domain algorithms remain independent of RTOS/vendor-private APIs. Native Zephyr kernel, device model, networking, storage and power management. No new generic scheduler or HAL. Compile optional facilities out.

| Class | Responsibility | Boundaries |
|---|---|---|
| I0 | Interrupt/acquisition | Bounded, nonblocking |
| R1 | Local control | No host, trace or flash wait |
| R2 | Policy/supervision | Bounded progress checks |
| C3 | Communication | Typed dispatch, no direct I/O ownership |
| O4 | Observation | Bounded sampling, queue and drop accounting |
| M5 | Maintenance | Eligible mode and reserved budgets |
| AI | Optional inference | Advisory only, isolated budget |

Every runnable including vendor and kernel contexts has period/trigger, priority, deadline, blocking, resource and affinity allocation. Reconcile actual runnable inventory.
## Isolation and resources
Use native MPU/MMU, user mode and memory domains where supported; analyze DMA, cache, shared bus, interrupt and temporal interference separately. Reject unsupported allocations or isolate in another image/controller. Bound allocation, queues, stacks, critical sections, cancellation and work-item lifetime. zbus may execute synchronous listeners or coalesce notifications. No assumption of lossless or nonblocking critical paths. Health distinguishes alive, deadline and logical progress; watchdog owner follows approved policy.
## Measurement
Measure component/shared-domain overhead, stack watermark, CPU, jitter, misses, trace cost, energy and link pressure. Timing analysis accounts interference and blocking, not observed maximum as WCET. Lab-only thresholds have G1-C-LAB applicability and do not become universal product limits.

## Native scheduling and timing
Assign each runnable its execution context and selected scheduler behavior; deadline scheduling is not assumed to override static priority. Analyze blocking, interrupt load, cache/bus interference, priority inversion and asynchronous completion. Native queues and work items must have explicit ownership, capacity, lifetime, overload disposition and cancellation semantics. Critical paths may not wait on unbounded observation, network, flash or inference operations. G1-C-LAB numerical budgets are defined only in Z12; production allocations are product-specific.
