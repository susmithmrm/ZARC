# ZARC architecture — 2035 design baseline candidate

**Mission:** the preferred open-source real-time platform for today's distributed ECUs and 2030–2035 intelligent vehicles. Modular, updatable, observable, accessible by authorized roles, deterministic, portable and AI-capable. Native Zephyr first. Software implementation and verification are the next phase.

The single requirements register [Z12](12-Requirements-Verification.csv) contains 267 requirements; no accepted production qualification is claimed. Start with [strategy](01-Executive-Strategy.md), [system architecture](02-System-Architecture.md), [verification](09-Verification-Engineering.md), [diagrams](diagrams/README.md), [implementation realization](realization/16-Implementation-Realization.md), [trace](../REQUIREMENT-TRACE.csv) and [partner guide](19-Platform-Capabilities-and-Partner-Adoption.md).

## Document map

| ID | Document | Primary audience |
|---|---|---|
| Z01 | [Executive strategy](01-Executive-Strategy.md) | Product, procurement, architecture |
| Z02 | [System architecture](02-System-Architecture.md) | Vehicle/system architects |
| Z03 | [Native RTOS and software design](03-Native-RTOS-Software-Design.md) | Embedded engineers |
| Z04 | [Product line and hardware integration](04-Product-Line-Hardware-Integration.md) | BSP, vendor, Tier 1, OEM teams |
| Z05 | [Services and lifecycle](05-Services-Lifecycle.md) | Application and system integrators |
| Z06 | [Connectivity and interface contracts](06-Connectivity-Interfaces.md) | Embedded and middleware teams |
| Z07 | [Central compute and SDV integration](07-Central-Compute-SDV.md) | Host, hypervisor and Android teams |
| Z08 | [Safety and security concept](08-Safety-Security-Assurance.md) | Assurance and product security |
| Z09 | [Verification and engineering infrastructure](09-Verification-Engineering.md) | Verification, release, integrators |
| Z10 | [Supplier ecosystem and roadmap](10-Supplier-Ecosystem-Roadmap.md) | Project and supplier leadership |
| Z11 | [Architecture decision and risk register](11-Decisions-Risks.md) | Architecture review board |
| Z12 | [Requirements and verification matrix](12-Requirements-Verification.csv) | All technical owners |
| Z13 | [Reference register](13-References.md) | Evidence and research |
| Z14 | [Architecture review report](14-Architecture-Review.md) | Architecture and engineering leads |
| Z15 | [Findings register](15-Review-Findings.csv) | Review board and requirement owners |
| Z16–17 | [Realization plan](realization/16-Implementation-Realization.md) and [development allocation](realization/17-Development-Allocation.csv) | Implementation teams |
| Z18–19 | [Vehicle reference](18-Modern-SDV-Reference-Architecture.md) and [partner guide](19-Platform-Capabilities-and-Partner-Adoption.md) | Architects and suppliers |
| Z22–24 | [Physical reference](22-Reference-Prototype.md), [demonstration](23-Reference-Demonstration-Plan.md), [planning BOM](24-Reference-Prototype-BOM.csv) | Bench and verification teams |

[Diagram atlas](diagrams/README.md) renders the editable Mermaid diagrams directly on GitHub. [Engineering templates](templates/) are also available.

Z12 is the sole normative requirements register. Markdown design documents and Mermaid diagrams describe allocated design intent; derived registers and templates do not introduce independent requirements. If wording conflicts, reconcile the design against Z12 before admission. IDs remain stable across revisions. PROPOSED denotes a specified requirement awaiting implementation-phase allocation and accepted verification. All targets are provisional unless stated as public upstream facts. Configuration-specific evidence is required before claiming support.

## Baseline review and scope

First implementation: benign non-safety bench companion controller with a trusted host simulator, then independent smart-edge evaluation. Later profiles are zonal and integrated RT domain. No vehicle-specific safe states, high-voltage or safety-critical actuation control algorithms are specified. Actual requirements come from item analysis, hardware and vehicle allocations.

## Glossary

IOC: I/O or companion controller. SDV: software-defined vehicle. OEM: original equipment manufacturer. BSP: board support package. FTTI: fault tolerant time interval. WCET: worst-case execution time. SIL/HIL: software/hardware in the loop. VHAL: vehicle hardware abstraction layer. AAOS: Android Automotive OS. VM: virtual machine. IPC: interprocessor/interprocess communication. SBOM: software bill of materials. DIA: development interface agreement. TARA: threat analysis and risk assessment. HARA: hazard analysis and risk assessment. QM: quality management classification, not a safety certification.


## Phase control

Z12 is the only requirements baseline. The implementation phase shall assess retained source against the specification, establish implementation and test allocations, and collect configuration-specific evidence. No implementation or qualification acceptance is asserted.
