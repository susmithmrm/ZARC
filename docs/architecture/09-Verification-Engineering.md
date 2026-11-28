# Z09 — Verification, traceability and evidence

> **ZARC 2035 | Architecture specification | Design phase.** The single requirements register is [Z12](12-Requirements-Verification.csv) (267 requirements). Implementation and verification are planned for the next phase.

## Verification contract
Z12 is the single source for normative IDs, objective, profile, acceptance, verification method, design allocation, gate, owner and status. Derived trace and development allocation must match these canonical fields. Implementation and test paths are assigned only after review in the implementation phase.
## Applicability
ALL refers to the admitted configuration, release, program or portfolio obligation; mandatory portfolio evidence is not a dependency of every firmware image. SELECTED derives from approved manifest, capability and contract allocation; E/C/Z/I are deployment profiles. No arbitrary per-requirement switches. Unknown predicates, absent implied dependencies and unresolved allocations block affected admission (VER-023). Portfolio obligations cannot be waived by individual image flags.
## Verification ladder
Unit and contract → desktop/native simulation → physical target and HIL → independent integration, safety and security review → customer production acceptance. Test positive, negative and boundary cases as applicable. Record exact source, configuration, toolchain, hardware identity, workload, uncertainty, expected/observed result, reviewer, raw trace and verdict. No fabricated physical evidence, no simulator-to-production inference.
## Architecture evidence
Verify independent component exchange, graph consistency, allocated resource visibility, actual versus intended deployment, trace correlation, authorized access, update/recovery and native control deadlines under observation pressure. Test architectural diagrams and catalogs against the resolved graph (VER-024). AI benchmark evidence reports model, precision, accuracy, latency, memory and energy; AI-assisted work receives accountable review.
## Gates
G0 allocation and architecture; G1 native/portable lab behavior; G2 physical portability, lifecycle and SDK; G3 independent protocol and host integration; G4 allocated safety/security and partner review; G5 portfolio-level adoption-readiness review and release-scope decision. A gate is the planned evidence stage, not acceptance or a mandatory dependency for unrelated configurations. Benchmark equivalent workloads, scope, assurance, licensing and staffing; publish limitations. No blanket certification.

## Applicability dictionary — 2035 baseline selection guidance

All 101 expressions used by Z12 are defined here as interpretation guidance, derived from the resolved register. The exact selection resolver and dependency graph remain to be implemented and reviewed under VER-023. A selected feature entails all applicable obligations; not-applicable needs approved rationale. Missing capability or evidence is never a passing result. Portfolio obligations including PL-006, SUP-015/016/018 and ADOPT-001/022/039/042/044/082 cannot be disabled by an image selection. Where a broad predicate covers independently allocated subcases, each is evaluated separately. No parallel requirements register is created.

| Expression | Selection condition |
|---|---|
| `ALL` | Applicable to assigned image, release, program or portfolio scope; record rationale for conditional subcases. |
| `C:UART` | Companion profile with admitted UART link. |
| `G1-C-LAB` | Benign companion lab allocation only. |
| `I` | Integrated real-time island profile. |
| `I:IPC` | Integrated island with selected interprocessor communication. |
| `SELECTED:ACCELERATOR` | Delegated DSP, NPU, GPU or specialized accelerator used. |
| `SELECTED:ACTUATORS` | Allocated physical actuator service present. |
| `SELECTED:ADVANCED_NETWORK` | CAN XL or FlexRay binding admitted. |
| `SELECTED:AI_ASSISTED_DEV` | AI-generated engineering artifacts included in controlled lifecycle. |
| `SELECTED:AI_SAFETY` | Safety-related AI allocated; implies SAFETY_SCOPE and local or external AI. |
| `SELECTED:AUTOMOTIVE_DIAGNOSTICS` | Automotive diagnostic service catalog admitted; implies DIAGNOSTICS. |
| `SELECTED:BROKER` | Trusted host mediation; implies HOST and TRANSPORT. |
| `SELECTED:CALIBRATION` | Versioned calibration or approved measurement/calibration service; allocate each subcase. |
| `SELECTED:CAN` | Physical CAN or CAN FD selected. |
| `SELECTED:CANCELLATION` | Service supports cancellation of admitted requests. |
| `SELECTED:CAN_DIAGNOSTICS` | ISO-TP diagnostic profile; implies CAN, AUTOMOTIVE_DIAGNOSTICS and TRANSPORT. |
| `SELECTED:COMMERCIAL_PILOT` | Comparative supplier development-effort pilot; implies INDUSTRY_PILOT. |
| `SELECTED:COMPONENTS` | Reusable typed application component admitted, including a single component. |
| `SELECTED:COORDINATED_UPDATE` | Coordinated multi-node update; implies UPDATE and versioned dependencies. |
| `SELECTED:CO_SIMULATION` | Versioned FMI or equivalent co-simulation interface selected. |
| `SELECTED:CYCLIC_SIGNALS` | Cyclic signal exchange allocated; implies applicable signal transport. |
| `SELECTED:DATA` | Typed data transformation across representation boundaries. |
| `SELECTED:DECOMMISSION` | Product replacement, retirement or data/identity disposition workflow allocated. |
| `SELECTED:DIAGNOSTICS` | Diagnostic event manager or diagnostic operation/session service; allocate subcases. |
| `SELECTED:DIGITAL_TWIN` | Visualization or digital-twin telemetry interface selected. |
| `SELECTED:DMA` | DMA or shared-memory transfer used. |
| `SELECTED:DOIP` | Diagnostics over IP admitted; implies AUTOMOTIVE_DIAGNOSTICS, applicable Ethernet and TRANSPORT. |
| `SELECTED:DOMAIN_PACK` | Reusable domain reference pack selected. |
| `SELECTED:EDGE_AI` | On-device model inference selected. |
| `SELECTED:EDGE_AI_OR_EXTERNAL_AI` | Derived OR of on-device inference and external AI advisory input; never a separate toggle. |
| `SELECTED:ETHERNET` | Ethernet binding admitted, including edge Ethernet. |
| `SELECTED:EVENTS` | Event notification or subscription interface allocated. |
| `SELECTED:EXTERNAL_OPERATIONS` | External typed operation or request interface exposed. |
| `SELECTED:FLEET` | Fleet-facing host integration selected; implies HOST and allocated transport. |
| `SELECTED:FUNCTION_INHIBITION` | Diagnostic function-availability/inhibition policy allocated. |
| `SELECTED:GROUPED_SIGNALS` | Coherent grouped signal representation or exchange allocated. |
| `SELECTED:HARDWARE_TIMING` | Hardware-triggered acquisition or output timing allocated. |
| `SELECTED:HOST` | Host adapter or host-facing service selected. |
| `SELECTED:INDUSTRIAL` | Industrial or off-highway integration contract selected. |
| `SELECTED:INDUSTRY_PILOT` | External industry partner evaluation agreed. |
| `SELECTED:ISOLATION` | Protection or isolation allocation requires enforcement. |
| `SELECTED:LEGACY` | Legacy ECU/network interoperability selected; portfolio proof remains separate. |
| `SELECTED:LEGACY_IMPORT` | Authorized legacy engineering-description translation selected; portfolio proof remains separate. |
| `SELECTED:LIN` | Physical LIN profile selected. |
| `SELECTED:LOCALIZATION` | Partner-facing localized documentation deliverable selected. |
| `SELECTED:LOCKSTEP` | Lockstep or split-lock configuration allocated. |
| `SELECTED:MANUFACTURING` | Factory provisioning/end-of-line scope; implies SOFTWARE_INVENTORY for traceable units. |
| `SELECTED:MEMORY` | Special target memory placement or initialization constraints apply. |
| `SELECTED:MIXED_CRITICALITY` | Different criticality allocations share resources; implies ISOLATION and applicable SAFETY_SCOPE. |
| `SELECTED:MULTI_APP` | At least two independently allocated consolidated applications; implies COMPONENTS and resource enforcement. |
| `SELECTED:NETWORK_MANAGEMENT` | Network availability, sleep or wake policy allocated. |
| `SELECTED:NETWORK_SECURITY` | Selected network-protection provider/profile allocated. |
| `SELECTED:NUMERIC` | Portable numerical algorithm has precision/overflow constraints. |
| `SELECTED:OBSERVATIONS` | Versioned external observations exposed; derive from interface contract. |
| `SELECTED:PACKED_SIGNALS` | External packed frame/signal mapping selected. |
| `SELECTED:PARTNER` | Supplier customer-variant/IP integration deliverable selected. |
| `SELECTED:PERCEPTION` | Raw perception or related supervision allocated across compute domains. |
| `SELECTED:PERIPHERALS` | Physical peripheral interface selected. |
| `SELECTED:PERSISTENCE` | Persistent configuration, state or block storage allocated. |
| `SELECTED:PERSONAL_DATA_OR_PRIVACY` | Applicable personal-data or privacy obligation derived from assessment. |
| `SELECTED:PM` | Device/system power management including selected frequency transitions. |
| `SELECTED:POWER` | Power-domain control or supervision service selected. |
| `SELECTED:POWER_LOSS_NOTIFICATION` | Allocated notification before loss of supply selected. |
| `SELECTED:POWER_SUPERVISION` | External safety PMIC or voltage/clock/thermal supervision allocated. |
| `SELECTED:REDUNDANT_CONTROL` | Redundant physical-resource control/ownership configuration. |
| `SELECTED:REDUNDANT_NETWORK` | Redundant communication paths; does not imply redundant controller ownership. |
| `SELECTED:REFERENCE_BENCH` | Reference kit/bench deliverable selected; portfolio proof separate. |
| `SELECTED:REGULATED_VEHICLE` | Vehicle regulatory/update evidence allocated by responsible approval holder. |
| `SELECTED:RELOCATABLE_COMPONENTS` | Component relocation approved; implies COMPONENTS. |
| `SELECTED:ROUTING` | Signal/service gateway or routing mapping allocated. |
| `SELECTED:SAFETY_COMMUNICATION` | Safety-related communication allocated; implies SAFETY_SCOPE and transport. |
| `SELECTED:SAFETY_SCOPE` | Safety responsibility allocated by item/system analysis. |
| `SELECTED:SCORE` | Selected S-CORE boundary admitted; host interface scope applies. |
| `SELECTED:SDV` | High-performance SDV host interface; implies HOST. |
| `SELECTED:SENSORS` | Typed sensor samples from allocated device/provider. |
| `SELECTED:SERIALIZATION` | Encoder/decoder for external serialized data present. |
| `SELECTED:SERVICE` | Service/maintenance interface selected; derive inventory if exposed. |
| `SELECTED:SHARED_RESOURCE` | Multiple authorized requesters share an allocated resource. |
| `SELECTED:SILICON_DIAGNOSTICS` | Vendor silicon diagnostics allocated. |
| `SELECTED:SMP` | SMP or AMP selected; core and resource allocation applies. |
| `SELECTED:SOC_IPC` | SoC mailbox/PCIe/vendor IPC; implies applicable TRANSPORT. |
| `SELECTED:SOFTWARE_INVENTORY` | Component/version/config inventory required by manufacture, service or update allocation. |
| `SELECTED:SOME_IP` | SOME/IP binding admitted. |
| `SELECTED:SOME_IP_HOST` | Host SOME/IP binding; implies SOME_IP and HOST. |
| `SELECTED:SOVD` | ASAM SOVD mapping selected; implies applicable diagnostics. |
| `SELECTED:SYSTEM_SLEEP` | System-level wake/sleep; implies PM. |
| `SELECTED:T1S` | Physical 10BASE-T1S segment selected; implies ETHERNET. |
| `SELECTED:TIME_SYNC` | Synchronized time quality required by allocated contracts. |
| `SELECTED:TOOL_INTEROP` | Versioned engineering-data exchange selected. |
| `SELECTED:TRANSPORT` | Managed communication link/provider; derive from resolved bindings. |
| `SELECTED:TSN` | TSN traffic policy selected; implies ETHERNET and applicable TIME_SYNC. |
| `SELECTED:UPDATE` | Firmware or independently activated deployment unit (including selected configuration, calibration or model data) has an allocated update/replacement policy; implies SOFTWARE_INVENTORY. Evaluate every selected artifact class; physical flash/boot and verified-boot evidence applies only to allocated firmware/trust-chain scope. Data-specific verification follows ADOPT-005/061/062 as applicable. |
| `SELECTED:VEHICLE_MODE` | Vehicle-mode availability policy selected; distinct from device and host lifecycle. |
| `SELECTED:VERIFIED_BOOT` | Target verified boot/trust-chain configuration allocated. |
| `SELECTED:VHAL` | AAOS vehicle hardware interface; implies HOST. |
| `SELECTED:VIRTUALIZATION` | Hypervisor or VM deployment selected; allocation and host lifecycle apply. |
| `SELECTED:VIRTUAL_ECU` | Virtual ECU model/provider and deterministic scenarios selected. |
| `SELECTED:VSS` | COVESA VSS mapping selected. |
| `SELECTED:ZBUS` | Native zbus channel/observer path used. |
| `SELECTED:ZONE` | Derived from Z profile or allocated zonal topology; not a separate opt-out. |
| `Z` | Zonal controller profile. |

## Verification allocation and evidence scope
VER-001–012, VER-014/016/017/019/021–024 and ADOPT-045–054/056–059/063–065/067/070 allocate traceability, reproducibility, measurements, coverage, review, virtual engineering, AI evaluation and observability. Only IDs actually present in Z12 apply. Use the resolved configuration to derive applicable, not applicable with reviewed rationale, or unresolved with an owner. Unresolved requirements block affected admission. Distinguish a portfolio record from an image record. Maintain exact candidate, software, simulation, physical and independently qualified verdicts; do not aggregate evidence across different configurations. Validate actual source/test allocations in the implementation phase. The evidence gate must reject missing, failed or unsupported evidence. A documentation audit can check consistency but cannot establish implementation or product acceptance.
