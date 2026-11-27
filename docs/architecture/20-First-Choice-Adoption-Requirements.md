# 20 — First-choice OEM and supplier adoption requirements

Status: proposed extension; not an approved baseline or implementation claim. Existing 111 requirements remain unchanged. This supplement identifies additional acceptance obligations; overlapping baseline clauses remain prerequisites. Profile-specific allocation is mandatory; optional features must not inflate minimal edge builds. No production or safety-critical actuation is authorized by this document.

Priorities: G2 bench/product readiness, G3 interoperability, G4 assurance. Existing gate meanings remain authoritative. All requirements are PROPOSED / NOT RUN; owner allocation and acceptance budgets require review. References: [existing baseline](12-Requirements-Verification.csv), [partner adoption](19-Platform-Capabilities-and-Partner-Adoption.md), [Zephyr safety](https://www.zephyrproject.org/how-zephyr-aligns-with-automotive-functional-safety-requirements/), [S-CORE](https://eclipse-score.github.io/score/main/introduction/index.html), [ASAM SOVD](https://www.asam.net/standards/detail/sovd/), [R155/R156](https://www.vehicle-certification-agency.gov.uk/connected-and-automated-vehicles/cyber-security-and-software-updating/).

## Hardware and I/O

| ID | Baseline objective / design | Requirement (shall) | Verification acceptance | Gate | Status |
|---|---|---|---|---|---|
| ZARC-ADOPT-001 | OBJ-PL / Z04 | **Physical target portfolio:** Provide conformance evidence on automotive MCU, general MCU and heterogeneous SoC reference targets. | Common benign application passes on three families; board manifests and limitations published. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-002 | OBJ-PL / Z04 | **Sensor contracts:** Provide typed sensor samples with units, calibration identity, timestamp and quality. | Physical analog, digital, encoder and environmental fixtures pass normal and disconnected cases. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-003 | OBJ-PL / Z04 | **Actuator contracts:** Provide bounded semantic actuator services with independent feedback and allocated fallback. | Benign lamp, fan and servo fixtures demonstrate feedback and approved fallback. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-004 | OBJ-PL / Z04 | **Electrical integration:** Declare pin, voltage, current, transceiver, DMA, interrupt and clock constraints. | Board review and current-limited bench measurements agree with manifest. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-005 | OBJ-PL / Z04 | **Calibration:** Support versioned calibration records with range, provenance and recovery policy. | Golden calibration and interrupted persistence fixtures pass. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-006 | OBJ-PL / Z04 | **Peripheral diversity:** Publish support matrix for GPIO, ADC, PWM, SPI, I2C, UART, capture, SENT and applicable automotive transceivers. | Each advertised interface has independent physical conformance evidence. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-007 | OBJ-PL / Z04 | **Power distribution:** Support selected smart switches, current sensing and power-domain feedback through typed services. | Low-voltage bench load and approved fault-response fixtures pass. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-008 | OBJ-PL / Z04 | **Wake and sleep:** Support allocated wake sources, sleep modes and wake reason reporting. | Physical current and wake latency meet product allocations. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-009 | OBJ-PL / Z04 | **Manufacturing:** Provide factory provisioning, serial identity and end-of-line test interfaces. | Repeatable factory fixture produces traceable unit record. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-010 | OBJ-PL / Z04 | **Hardware lifecycle:** Track silicon errata, board revisions, component substitutions and support windows. | Change-impact review and regression evidence for selected substitution. | G2 | PROPOSED / NOT RUN |

## Network and host

| ID | Baseline objective / design | Requirement (shall) | Verification acceptance | Gate | Status |
|---|---|---|---|---|---|
| ZARC-ADOPT-011 | OBJ-CON / Z06 | **CAN FD:** Qualify physical CAN/CAN FD communication and bus-state reporting. | Independent bench peer passes approved frames, load and recovery cases. | G3 | PROPOSED / NOT RUN |
| ZARC-ADOPT-012 | OBJ-CON / Z06 | **LIN:** Qualify selected LIN controller or responder profile and schedule. | Independent bench peer passes schedule and sleep/wake fixtures. | G3 | PROPOSED / NOT RUN |
| ZARC-ADOPT-013 | OBJ-CON / Z06 | **Ethernet PHY:** Qualify selected standard and automotive Ethernet PHY configurations. | Independent peers demonstrate link, throughput and recovery within allocations. | G3 | PROPOSED / NOT RUN |
| ZARC-ADOPT-014 | OBJ-CON / Z06 | **Time synchronization:** Expose synchronized time with uncertainty and loss-of-sync quality. | Independent clock comparison meets allocated accuracy and degraded policy. | G3 | PROPOSED / NOT RUN |
| ZARC-ADOPT-015 | OBJ-CON / Z06 | **Traffic management:** Support selected traffic classes, bandwidth reservation and TSN where allocated. | Competing traffic fixture meets approved latency and loss budgets. | G3 | PROPOSED / NOT RUN |
| ZARC-ADOPT-016 | OBJ-CON / Z06 | **10BASE-T1S:** Provide optional 10BASE-T1S integration profile where hardware supports it. | Independent physical segment meets declared configuration and timing. | G3 | PROPOSED / NOT RUN |
| ZARC-ADOPT-017 | OBJ-CON / Z06 | **UDS and DoIP:** Provide selected standards-based diagnostic integration through authorized diagnostic services. | Independent approved tester passes scoped diagnostic conformance. | G3 | PROPOSED / NOT RUN |
| ZARC-ADOPT-018 | OBJ-CON / Z06 | **SOVD:** Map approved diagnostic content into selected ASAM SOVD interface. | Independent client confirms discovery, records and availability semantics. | G3 | PROPOSED / NOT RUN |
| ZARC-ADOPT-019 | OBJ-CON / Z06 | **VSS mapping:** Provide versioned COVESA VSS mappings for applicable vehicle signals. | Golden mapping tests preserve units, quality and authority. | G3 | PROPOSED / NOT RUN |
| ZARC-ADOPT-020 | OBJ-CON / Z06 | **S-CORE interoperability:** Provide documented service boundary to selected Eclipse S-CORE release. | Independent integration fixture passes selected service contract. | G3 | PROPOSED / NOT RUN |
| ZARC-ADOPT-021 | OBJ-CON / Z06 | **Legacy coexistence:** Provide migration adapters for existing AUTOSAR and OEM signal contracts without duplicating authority. | Independent legacy peer and ownership review pass. | G3 | PROPOSED / NOT RUN |
| ZARC-ADOPT-022 | OBJ-CON / Z06 | **Multi-ECU operation:** Support a configurable topology of distributed controllers and independent failure domains. | Four physical controllers demonstrate scoped loss and recovery. | G3 | PROPOSED / NOT RUN |
| ZARC-ADOPT-023 | OBJ-CON / Z06 | **High-bandwidth sensing:** Define direct perception data paths and separate ZARC supervision interfaces. | Architecture review confirms raw high-bandwidth data does not overload real-time MCU. | G3 | PROPOSED / NOT RUN |

## Assurance and lifecycle

| ID | Baseline objective / design | Requirement (shall) | Verification acceptance | Gate | Status |
|---|---|---|---|---|---|
| ZARC-ADOPT-024 | OBJ-ASR / Z08 | **Safety package:** Deliver scoped assumptions of use, safety manual, trace and safety analysis for selected product. | Independent assessor accepts allocated evidence or scope is explicitly excluded. | G4 | PROPOSED / NOT RUN |
| ZARC-ADOPT-025 | OBJ-ASR / Z08 | **Safety process:** Define ISO 26262 work products, responsibilities, independence and change review. | Product-specific process audit accepts allocated work products. | G4 | PROPOSED / NOT RUN |
| ZARC-ADOPT-026 | OBJ-ASR / Z08 | **Cybersecurity process:** Define ISO SAE 21434 lifecycle work products and vulnerability response responsibilities. | Responsible integrator approves risk and lifecycle evidence. | G4 | PROPOSED / NOT RUN |
| ZARC-ADOPT-027 | OBJ-ASR / Z08 | **Regulatory support:** Provide configuration and update evidence supporting applicable UN R155 and R156 integrator obligations. | Compliance mapping reviewed by responsible vehicle program; no platform type approval claimed. | G4 | PROPOSED / NOT RUN |
| ZARC-ADOPT-028 | OBJ-ASR / Z08 | **Software update fleet:** Support selected coordinated update dependencies and compatibility reporting. | Benign multi-node update and interruption fixtures demonstrate approved recovery. | G4 | PROPOSED / NOT RUN |
| ZARC-ADOPT-029 | OBJ-ASR / Z08 | **Privacy:** Minimize collected data and support configurable retention and access policy. | Data inventory and deletion/retention tests match approved jurisdictional policy. | G4 | PROPOSED / NOT RUN |
| ZARC-ADOPT-030 | OBJ-ASR / Z08 | **Regional applicability:** Maintain product-specific applicability matrix for China, Korea, EU and other target markets. | Qualified reviewer approves applicable obligations and ownership. | G4 | PROPOSED / NOT RUN |
| ZARC-ADOPT-031 | OBJ-ASR / Z08 | **Operational maintenance:** Publish incident intake, supported release policy and coordinated vulnerability disclosure. | Maintenance drill demonstrates triage and traceable remediation. | G4 | PROPOSED / NOT RUN |
| ZARC-ADOPT-032 | OBJ-ASR / Z08 | **Quality process:** Maintain Automotive SPICE aligned engineering work products and review evidence. | Independent process gap assessment and action register published. | G4 | PROPOSED / NOT RUN |
| ZARC-ADOPT-033 | OBJ-ASR / Z08 | **Tool confidence:** Assess tool confidence and qualification needs for each safety-relevant toolchain. | Tool assessment and selected validation evidence approved. | G4 | PROPOSED / NOT RUN |
| ZARC-ADOPT-034 | OBJ-ASR / Z08 | **Third-party dependencies:** Track dependency licenses, support and approved upgrade policy. | Release audit identifies all dependencies and accepted obligations. | G4 | PROPOSED / NOT RUN |

## Developer and ecosystem

| ID | Baseline objective / design | Requirement (shall) | Verification acceptance | Gate | Status |
|---|---|---|---|---|---|
| ZARC-ADOPT-035 | OBJ-SUP / Z10 | **SDK:** Provide pinned SDK and reproducible quickstart for each supported profile. | Fresh environment builds and runs benign sample following published instructions. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-036 | OBJ-SUP / Z10 | **CLI and generation:** Provide declarative configuration generator and validation with actionable diagnostics. | Valid fixture generates deterministic artifacts; invalid configurations rejected. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-037 | OBJ-SUP / Z10 | **Extension SDK:** Provide documented provider and service extension templates. | Independent developer adds benign service without editing generic core. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-038 | OBJ-SUP / Z10 | **Version policy:** Publish API compatibility, deprecation and LTS policy. | Upgrade fixtures and support window records match release policy. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-039 | OBJ-SUP / Z10 | **Governance:** Publish contribution, review, ownership and decision processes. | Independent contribution can be reviewed through documented workflow. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-040 | OBJ-SUP / Z10 | **Supplier package:** Publish integration checklist, interface control document and evidence template. | Independent integrator can reproduce selected reference deployment. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-041 | OBJ-SUP / Z10 | **Localization:** Provide extensible documentation glossary and translation process for Chinese and Korean partners. | Reviewed glossary and localized quickstart preserve technical meaning. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-042 | OBJ-SUP / Z10 | **Cost evidence:** Publish comparable integration effort and resource cost data. | Reproducible benchmark records hardware, staffing assumptions and limitations. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-043 | OBJ-SUP / Z10 | **Support matrix:** Publish evidence-bound board, profile, transport and OS maturity matrix. | Every supported entry resolves to versioned acceptance evidence. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-044 | OBJ-SUP / Z10 | **Migration guide:** Provide incremental migration from legacy ECU software without forcing full platform replacement. | Reference migration demonstrates coexistence and bounded scope. | G2 | PROPOSED / NOT RUN |

## Validation and performance

| ID | Baseline objective / design | Requirement (shall) | Verification acceptance | Gate | Status |
|---|---|---|---|---|---|
| ZARC-ADOPT-045 | OBJ-VER / Z09 | **Physical HIL:** Publish reproducible low-voltage sensor and actuator bench reference. | Independent reproduction verifies at least 50 representative signals or clearly identifies simulated channels. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-046 | OBJ-VER / Z09 | **Digital twin:** Connect actual controller telemetry to existing vehicle visualization with provenance. | Live, stale and simulated data distinguishable in end-to-end test. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-047 | OBJ-VER / Z09 | **Virtual ECU:** Provide deterministic virtual ECU fixtures and physical correlation procedure. | Common scenario produces comparable observations with documented differences. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-048 | OBJ-VER / Z09 | **Benchmarks:** Publish boot, latency, jitter, RAM, flash, CPU and power measurements per profile. | Raw traces and method reproduce results; no universal threshold claimed. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-049 | OBJ-VER / Z09 | **Stress and endurance:** Measure sustained workload and storage endurance against product allocation. | Approved-duration run and endurance analysis meet documented bounds. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-050 | OBJ-VER / Z09 | **Fault recovery:** Verify approved benign fault cases across startup, normal operation and shutdown. | Independent HIL report records expected and actual outcomes. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-051 | OBJ-VER / Z09 | **Release dashboard:** Publish requirement trace, failed tests, missing evidence and supported configuration status. | Missing evidence cannot be presented as accepted. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-052 | OBJ-VER / Z09 | **Test automation:** Provide repeatable SIL/HIL orchestration and versioned test fixtures. | Independent runner recreates results for selected reference target. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-053 | OBJ-VER / Z09 | **Performance budgets:** Provide profile-specific admission budgets and reference targets. | Measured results compared to approved budgets; deviations explicitly recorded. | G2 | PROPOSED / NOT RUN |
| ZARC-ADOPT-054 | OBJ-VER / Z09 | **Advisory edge AI:** Allow optional bounded advisory analytics without physical control authority. | Resource and authority review plus benign isolation tests demonstrate separation. | G2 | PROPOSED / NOT RUN |

## Adoption sequence

1. Physical benign IOC on two vendors; then distributed bench and third architecture.
2. Qualify selected transports and sensor/actuator providers; connect host and digital twin.
3. Deliver SDK, migration examples, conformance suites and benchmark evidence.
4. Complete product-specific assurance and independent acceptance before any production claim.

OEM evaluation scenarios: XPeng and Leapmotor style central/local control; Geely and Xiaomi style reusable zones; BYD and NIO style proprietary host coexistence; Hyundai/Kia style HPVC and zonal I/O. These are hypothetical evaluation scenarios, not vendor endorsements or verified compatibility. Select applicable requirements by target manifest and link each adopted ID to test evidence before promotion into the controlled baseline. This supplement contains 54 proposed requirements.
