# Platform capability and partner adoption guide

Partner-facing view of the architecture. No new normative requirements, unsupported feature claims, vendor adoption claims or production qualification. [Rendered partner guide](https://susmithmrm.github.io/ZARC/platform-adoption.html).

## Capability landscape

Lab means limited laboratory evidence; Binding means native API code, not physical qualification; Candidate means architecture direction; Planned means not implemented as a complete integration; Opportunity means potential future extension. Desktop, generated service, COVESA, native simulator and Pages workflows passed on commit `2e42477`; check the live CI for later revisions. Simulator results are not physical qualification.

| Area | Capability | Approach | Maturity | Remaining evidence |
|---|---|---|---|---|
| Deployment | [External companion IOC](https://github.com/susmithmrm/ZARC/blob/main/docs/COORDINATION-INCREMENT.md) | Local supervision, wake and guest readiness; independent reset/power allocation | Laboratory evidence | Physical power and reset evidence |
| Deployment | [Zonal controllers](https://github.com/susmithmrm/ZARC/blob/main/docs/architecture/04-Product-Line-Hardware-Integration.md) | Reusable endpoint services and a selected vehicle network | Architecture candidate | OEM zone configuration, electrical and network validation |
| Deployment | [Intelligent edge / standalone MCU](https://github.com/susmithmrm/ZARC/blob/main/docs/AUTONOMY-INCREMENT.md) | Local decisions without host; optional telemetry | Laboratory evidence | Second physical board and product policy |
| Deployment | [Integrated real-time island](https://github.com/susmithmrm/ZARC/blob/main/platform/zephyr/adapters/TRANSPORT.md) | Native IPC to application compute; no competing device owner | Native API binding | SoC memory, reset and interference analysis |
| Deployment | [Passive / remote I/O](https://github.com/susmithmrm/ZARC/blob/main/docs/architecture/02-System-Architecture.md) | Keep local policy at zone, use selected remote endpoint transport | Planned integration | Electrical fallback and transport timing |
| Core | [Native Zephyr execution](https://github.com/susmithmrm/ZARC/blob/main/docs/NATIVE-INCREMENT.md) | Threads, queues, timers, native device APIs; bounded owner model | Laboratory evidence | Physical WCET, stack and timing measurements |
| Core | [Product-line configuration](https://github.com/susmithmrm/ZARC/blob/main/docs/TARGET-ADMISSION-INCREMENT.md) | Service dependencies, declared phase ownership and timing allocations | Laboratory evidence | DTS/Kconfig reconciliation on each approved target |
| Core | [Service lifecycle & autonomy](https://github.com/susmithmrm/ZARC/blob/main/docs/COMPOSITION-INCREMENT.md) | Start, stop, suspend, resume; local admission and cancellation | Laboratory evidence | Product-specific policies and rollback evidence |
| Core | [Deterministic scheduling & load control](https://github.com/susmithmrm/ZARC/blob/main/docs/AUTONOMY-INCREMENT.md) | Absolute periodic releases, deadline accounting, reserved traffic capacity | Laboratory evidence | Selected RT workload, interference and overload HIL |
| Core | [Health & fault diagnostics](https://github.com/susmithmrm/ZARC/blob/main/docs/SUPERVISION-INCREMENT.md) | Alive, completion and logical monitoring; bounded fault journal | Laboratory evidence | Independent supervision and durable diagnostics |
| Core | [Persistent settings](https://github.com/susmithmrm/ZARC/blob/main/platform/zephyr/adapters/README.md) | Two-slot integrity/recovery model and native NVS integration | Native API binding | Flash endurance and physical interruption testing |
| Core | [Watchdog and update control](https://github.com/susmithmrm/ZARC/blob/main/platform/zephyr/adapters/README.md) | Health-gated native watchdog; MCUboot test/confirm integration | Native API binding | Signed images, durable boot journal and hardware recovery |
| Communication | [Typed contracts & discovery](https://github.com/susmithmrm/ZARC/blob/main/docs/OBSERVATION-INCREMENT.md) | Capability enumeration, request retention, snapshot barrier and event gaps | Laboratory evidence | Complete selected service schemas and provider conformance |
| Communication | [UART, CAN and IPC](https://github.com/susmithmrm/ZARC/blob/main/platform/zephyr/adapters/TRANSPORT.md) | Native API bindings, owned buffers and bounded callbacks | Native API binding | Physical bus and cross-core validation |
| Communication | [Ethernet / SOME/IP](https://github.com/susmithmrm/ZARC/blob/main/docs/SOMEIP-RUNTIME-INCREMENT.md) | COVESA local read-only runtime fixture and independent peer | Laboratory evidence | External Ethernet peer, discovery, events and production trust |
| Communication | [CAN FD, LIN, TSN / time sync](https://github.com/susmithmrm/ZARC/blob/main/docs/architecture/06-Connectivity-Interfaces.md) | Selectable OEM network integration candidates | Planned integration | Chosen drivers, timing budgets and network tests |
| Host | [Generated protobuf / gRPC](https://github.com/susmithmrm/ZARC/blob/main/docs/GENERATED-SERVICE-INCREMENT.md) | Actual independent client and host broker reference | Laboratory evidence | Product schemas and deployed peer trust |
| Host | [Android Automotive VHAL](https://github.com/susmithmrm/ZARC/blob/main/docs/HOST-MAPPING-INCREMENT.md) | AP power-property reference mapping | Laboratory evidence | Actual AOSP/OEM Binder or VehicleServer integration, CTS/VTS |
| Host | [AAOS SDV / VSIDL](https://github.com/susmithmrm/ZARC/blob/main/docs/architecture/07-Central-Compute-SDV.md) | Semantic mapping and service-bundle integration direction | Architecture candidate | Generated OEM VSIDL service deployment |
| Host | [Linux, QNX, hypervisor, other OS](https://github.com/susmithmrm/ZARC/blob/main/docs/architecture/07-Central-Compute-SDV.md) | Trusted host process/service VM, selected native IPC | Architecture candidate | Per-OS adapter and isolation evidence |
| Host | [CommonAPI / AUTOSAR coexistence](https://github.com/susmithmrm/ZARC/blob/main/docs/architecture/06-Connectivity-Interfaces.md) | Semantic gateway, independently allocated existing ECUs | Architecture candidate | FIDL/FDEPL, external peer and OEM signal contracts |
| Host | [Host authentication & authorization](https://github.com/susmithmrm/ZARC/blob/main/docs/HOST-SECURITY-INCREMENT.md) | mTLS identity, scoped capabilities and rate limits | Laboratory evidence | Key lifecycle, approved deployment and MCU link trust |
| Future | [Fleet OTA & cloud telemetry](https://github.com/susmithmrm/ZARC/blob/main/docs/architecture/08-Safety-Security-Assurance.md) | Fleet orchestrator via trusted host, no direct actuator path | Architecture candidate | Fleet backend, signing/provisioning, rolling compatibility |
| Future | [SOVD / UDS & service diagnostics](https://github.com/susmithmrm/ZARC/blob/main/docs/architecture/06-Connectivity-Interfaces.md) | Map approved diagnostic operations to local domain evidence | Planned integration | Protocol runtime, diagnostic policy and external conformance |
| Future | [Edge AI and predictive maintenance](https://github.com/susmithmrm/ZARC/blob/main/docs/architecture/01-Executive-Strategy.md) | AI produces advisory intent or analytics; deterministic local policy remains authoritative | Future opportunity | Model resource isolation, data quality and approved use-case validation |
| Future | [Virtual ECU / SIL / digital twin](https://github.com/susmithmrm/ZARC/blob/main/docs/LOOPBACK-INCREMENT.md) | Desktop independent processes and native simulator; wider twin integration candidate | Laboratory evidence | External twin and real HIL correlation |
| Assurance | [Functional safety and cybersecurity](https://github.com/susmithmrm/ZARC/blob/main/docs/architecture/08-Safety-Security-Assurance.md) | Allocated ownership, evidence gate, fault containment design | Architecture candidate | Item HARA/TARA, safety case, independent approval |
| Assurance | [Traceability & build provenance](https://github.com/susmithmrm/ZARC/blob/main/docs/RELEASE-ENGINEERING-INCREMENT.md) | 178 proposed requirements, candidate trace and release evidence tooling | Laboratory evidence | All applicable requirements accepted; SBOM and signing |
| Assurance | [Supplier conformance](https://github.com/susmithmrm/ZARC/blob/main/docs/architecture/10-Supplier-Ecosystem-Roadmap.md) | Structured target descriptor, supplier admission and review records | Laboratory evidence | Partner submissions and configuration-specific approval |

## Partner contribution paths

### Chinese and Korean documentation

The [draft glossary](../localization/GLOSSARY.md), [Chinese quick start](../localization/quickstart.zh-CN.md), [Korean quick start](../localization/quickstart.ko.md), and [translation workflow](../localization/WORKFLOW.md) preserve the English build commands. Native-language technical review and partner acceptance remain open.

| Role | Work | Deliverable | Entry point |
|---|---|---|---|
| OEM / vehicle architect | Select IOC, edge, zone or integrated RT placement; allocate devices, modes, safety and cybersecurity requirements; own vehicle acceptance. | Vehicle allocation, ICD, HARA/TARA, network IDs, acceptance plan | [Start with vehicle reference design](https://github.com/susmithmrm/ZARC/blob/main/docs/architecture/18-Modern-SDV-Reference-Architecture.md) |
| Tier 1 / ECU integrator | Choose board and Zephyr version; create target manifest, service composition, hardware mapping and product policy; run target conformance and HIL. | Pinned build, board/ECU integration, measurements, support scope | [Target admission checklist](https://github.com/susmithmrm/ZARC/blob/main/docs/architecture/templates/Target-Admission-Checklist.md) |
| Tier 2 / component supplier | Implement a bounded service or provider against public contracts; document lifecycle, dependencies, ownership, timing and failure results. | Versioned service contract, tests, licenses, maintenance owner | [Service contract template](https://github.com/susmithmrm/ZARC/blob/main/docs/architecture/templates/Service-Contract-Template.md) |
| Semiconductor / BSP vendor | Contribute Zephyr board/SoC support, drivers, clocks, IRQ/DMA, pinctrl, memory, errata, reset and isolation evidence. Keep generic code independent. | DTS/Kconfig, pinned toolchain, errata, resource and driver evidence | [Vendor porting guide](https://github.com/susmithmrm/ZARC/blob/main/docs/architecture/04-Product-Line-Hardware-Integration.md) |
| Host OS / middleware supplier | Bind vehicle service APIs to trusted host semantics; implement actual platform identity, versioning, discovery, observation and failure contracts. | Independent client tests, VHAL/VSIDL or SOME/IP conformance, security evidence | [Host integration guide](https://github.com/susmithmrm/ZARC/blob/main/docs/architecture/07-Central-Compute-SDV.md) |
| Edge / industrial developer | Start without host dependencies; choose local product policy, sensors and output ownership. Add gateway only when required. | Target and service contracts, measurement, domain-specific assurance | [Autonomous reference](https://github.com/susmithmrm/ZARC/blob/main/docs/AUTONOMY-INCREMENT.md) |
| Verification / safety / security team | Review scope and traceability, inject faults on approved lab hardware, check timing and recovery; accept only demonstrated configurations. | Raw evidence, configuration hashes, review and signed approval | [Verification record template](https://github.com/susmithmrm/ZARC/blob/main/docs/architecture/templates/Verification-Record-Template.md) |

## Public industry alignment

- [BMW Neue Klasse](https://www.bmwgroup.com/en/news/general/2025/superbrains.html): Publicly described high-performance compute and zonal wiring. Possible fit: Evaluate companion/zone profiles and local authority boundaries.
- [Volkswagen Group / Rivian](https://www.volkswagen-group.com/en/press-releases/software-defined-vehicles-joint-venture-rv-tech-successfully-completes-winter-testing-20235): Public next-generation zonal SDV architecture. Possible fit: Evaluate zonal controllers, independent IOC and standard service integration.
- [Hyundai Motor Group / Kia](https://www.hyundaimotorgroup.com/en/news/hyundai-motor-group-hosts-pleos-sdv-standard-forum-to-drive-software-defined-vehicle-era-through-collaboration): Public CODA compute and I/O-domain architecture and Pleos direction. Possible fit: Evaluate the compute-to-I/O semantic boundary and product composition.
- [Toyota / Woven](https://global.toyota/en/newsroom/toyota/43740895.html): Public Arene software-development platform initiative. Possible fit: Evaluate a trusted host adapter and separate hardware-facing contracts.

These are illustrative evaluation directions, not claims of adoption or compatibility. The exact board, SoC stepping, toolchain, enabled services, security profile, ownership and evidence define support. No chip family or named OEM is asserted supported. AAOS VSIDL and Eclipse SDV ecosystems provide integration directions; actual provider interoperability remains configuration-specific.

## First working operation

Follow the [desktop quick start](../../README.md), [native simulator guide](../../platform/zephyr/README.md), [target example](../../config/targets/native-sim-edge.json), and [HIL and evidence gate](../HIL-AND-EVIDENCE.md). Discover capability, authorize/admit typed intent, execute local policy through the native owner, observe independent feedback, and invalidate/reconcile on link or reset. Start with benign indicators. Safety-critical and production hardware work requires approved allocation and evidence.

Release maturity progresses from candidate to buildable, bench validated, supported, and separately assessed scope. Do not infer qualification from tests or an MIT license. See [supplier roadmap](10-Supplier-Ecosystem-Roadmap.md).
