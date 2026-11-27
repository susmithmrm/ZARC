# ZARC architecture baseline

**Zephyr Automotive Real-Time Control | Version 1.1 | 27 November 2026**

ZARC defines a modular, Zephyr-native software foundation for the physical control layer of software-defined vehicles. The proposed architecture is intended for OEM, Tier 1, Tier 2 and semiconductor partners. It is an engineering design baseline, not a product release or qualification claim.

## Desktop-first realization edition

Start with [the realization specification](realization/16-Desktop-First-Realization.md) and [the development allocation](realization/17-Development-Allocation.csv). These documents supersede the board-first sequencing for development. RT-001 applies to embedded execution adapters. The 1.1 PDF and original requirements register are the reviewed target architecture snapshot.

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

[Diagram atlas](diagrams/README.md) renders the editable Mermaid diagrams directly on GitHub. [Engineering templates](templates/) are also available.

The PDF is the consolidated review copy; Markdown files are editable normative-candidate design sources, CSV is an editable requirements register, Mermaid files are editable diagrams. IDs remain stable across revisions. A requirement marked PROPOSED has not been implemented or verified. All targets are provisional unless stated as public upstream facts. Configuration-specific evidence is required before claiming support.

## Baseline review and scope

First implementation: benign non-safety bench companion controller with a trusted host simulator, then independent smart-edge evaluation. Later profiles are zonal and integrated RT domain. No vehicle-specific safe states, high-voltage or safety-critical actuation control algorithms are specified. Actual requirements come from item analysis, hardware and vehicle allocations.

## Glossary

IOC: I/O or companion controller. SDV: software-defined vehicle. OEM: original equipment manufacturer. BSP: board support package. FTTI: fault tolerant time interval. WCET: worst-case execution time. SIL/HIL: software/hardware in the loop. VHAL: vehicle hardware abstraction layer. AAOS: Android Automotive OS. VM: virtual machine. IPC: interprocessor/interprocess communication. SBOM: software bill of materials. DIA: development interface agreement. TARA: threat analysis and risk assessment. HARA: hazard analysis and risk assessment. QM: quality management classification, not a safety certification.

## Review status

Version 1.1 supersedes v1.0 as a corrected architecture candidate. Z14 and Z15 document source-backed corrections and remaining decisions. No implementation tests were run; no vendor, timing, safety, security or qualification validation has been established. Use the release gate definitions and profiles in Z09 rather than treating all proposed requirements as currently applicable. The separate machine-readable findings register and requirements CSV are controlled review inputs.
