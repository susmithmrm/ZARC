# Z14 Architecture review and disposition

**ZARC 1.1 | Desk-based architecture review | 27 November 2026**

## Executive conclusion

The high-level decision remains sound: native Zephyr, independent allocated physical authority, build-time product composition, standards-based boundaries, trusted host ownership, and scope-specific assurance. The implementation architecture is feasible as a design candidate, not demonstrated on any specific hardware. The v1.0 baseline had ambiguities which could result in incorrect integration or misinterpretation. Version 1.1 resolves the document-level issues identified below and introduces explicit acceptance conditions. It does not approve vehicle use. G0 engineering review can proceed; implementation and qualification gates remain open.

## Review method and coverage

Read the active v1.0 specifications, requirement register, diagrams and source register; compare between system, RTOS, lifecycle, interface, product, safety and supplier documents. Check native Zephyr scheduling, workqueue, zbus, UART, PM and Android power semantics against primary documentation. Evaluate boundary consistency, failure handling, ownership, timing assumptions, applicability, testability and forward compatibility. Record severity, evidence, corrective document, and remaining engineering closure. No implementation source, compiler, board, simulator, OS integration or qualification evidence was independently executed. No independent external assessor participated.

Severity: Critical = incorrect claim or missing control could invalidate architecture/qualification; High = major implementation or ownership ambiguity; Medium = documentation, operational or trace gap. A document correction is not verification of a mechanism. The separate Z15 register records both disposition and evidence gap.

## Principal technical corrections

| Review area | Verdict and correction |
|---|---|
| Native RTOS | Keep native primitives; zbus is not inherently nonblocking. Publisher context, observer type and backpressure must be analyzed. |
| Scheduling | Keep fixed-priority initial target. Deadline setting does not implement global EDF; inventory all kernel/vendor threads, ISR and resource effects. |
| Workqueues | Work item reuse/coalescing is not event delivery; require bounded explicit event queues for occurrence-sensitive work. |
| Timing | Separate lab hypotheses from profile-wide requirements. Restrict response-time method to its assumptions, require measured and analyzed evidence. |
| Power | Separate device/system PM and vehicle electrical states. Balance usage references and account for wake latency. |
| Ownership | Specify source dependency direction and composition root, physical arbiter and owner during startup/reset/update. |
| Lifecycle | Suspend and full stop differ; stop requires reinitialize/start. Quiesce and cancel cannot imply reversal of physical effect. |
| Communication | Separate boot epoch/session epoch; define request ID collision, retention, unknown outcome, freshness and snapshot race. |
| Provider | Truthful per-binding capability, UART exclusive API and hardware support gate; no universal DMA claim. |
| Host | Verify VHAL direction and state-dependent field mapping; separate guest/service/physical ownership. Require actual isolation evidence. |
| Assurance | Correctly keep evidence and risk allocations open; cover boot/reset/update, independent mechanisms and supply chain. |
| Supplier adoption | Explicit hardware support levels, version/ABI policy and configuration authority prevent hidden variants/forks. |
| Verification | Add parent objectives, capability applicability, per-target instances, new tests and consistent gate scope. |

## Objective hierarchy

| ID | Stakeholder objective | Primary allocation |
|---|---|---|
| OBJ-SYS | Physical authority and independence within allocated function | Z02 |
| OBJ-RT | Predictable bounded execution and resource ownership | Z03 |
| OBJ-PL | Vendor-neutral target/product composition | Z04 |
| OBJ-SVC | Explicit service dependencies, readiness and lifecycle | Z05 |
| OBJ-CON | Versioned typed intent and reliable observable contracts | Z06 |
| OBJ-HOST | Central compute integration without physical authority loss | Z07 |
| OBJ-NET | Verified network and integrated-domain adapters | Z06–Z07 |
| OBJ-ASR | Configuration-scoped safety/cyber evidence | Z08 |
| OBJ-VER | Reproducible verification and release control | Z09 |
| OBJ-SUP | Independently owned supplier contribution and maintenance | Z10 |

Objectives are stakeholder-level design intents, not safety goals or finished product requirements. Profile applicability and exact allocations are derived via the selected target manifest. All Z12 entries remain PROPOSED / NOT RUN.

## Residual engineering decisions and blockers

| Gate | Blocking evidence or decision | Owner |
|---|---|---|
| G0 | First reference boards, exact Zephyr branch/toolchain, native driver capabilities, scoped domain and contractual requirements | Architecture, vendor and OEM/Tier 1 |
| G1 | Compile/run minimal native prototype, schedule, driver/callback behavior, UART codec/profile, session semantics, timing and memory measurements | Embedded/Connect/verification |
| G2 | Second vendor and independent smart-edge profile, resource closure, approved boot/update scheme, target HIL and recovery | Supplier/Platform/SDK |
| G3 | Actual host OS/AAOS branch, property mapping/compatibility, independent SOME/IP peer, hypervisor and integrated RT isolation | Host and SoC vendor |
| G4 | Vehicle item, safe states/FTTI, HARA/TARA, allocated requirements, target HIL, environment and independent assessment scope | OEM, Tier 1 and assurance |
| G5 | Exact release evidence, approved residual risks, commercial support, manufacturing/provisioning and regulatory responsibility | OEM, Tier 1, release board |

No universal timing, safety, security or hardware support claim is permissible before evidence. If G1 measurements fail, simplify selected modules or change hardware/transport; if integrated RT cannot demonstrate allocated independence, use external companion. If an OEM mandates a different qualified stack, ZARC may be restricted to the mutually agreed non-safety functions.

## Review exit criteria

Document corrections are accepted as a draft baseline for G0 discussion only. The review board must accept interface and requirement allocations, choose target hardware, establish verification procedures and close outstanding decisions before implementation claims. Z15 and Z12 remain live controlled artifacts. The next review must occur after the G1 reference build and before any supplier support statement.
