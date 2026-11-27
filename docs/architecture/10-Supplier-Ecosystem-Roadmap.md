# Z10 Supplier ecosystem and roadmap

## Supplier contribution model (D20)

| Actor | Owns | Contract and admission evidence |
|---|---|---|
| ZARC maintainers | Public service, module and schema contracts; releases | Compatibility policy, builds, conformance, SBOM and change impact |
| Semiconductor supplier | SoC/BSP/driver/errata, boot/clock/memory/IRQ/DMA and isolation facts | Native Zephyr integration, board support report, tested silicon and limits |
| Tier 2 | Peripheral, diagnostics, specialized service or transport component | Approved interface, bounded resource/timing, licenses and test results |
| Tier 1 | Product app, policy, target composition and ECU integration | Hardware/electrical ICD, integration tests, configuration and evidence pack |
| OEM | Architecture allocations, vehicle modes, identifiers, safety/cyber requirements, validation | Vehicle ICD, DIA, allocated requirements and acceptance |

Use independent repository/module ownership and reviewed API boundaries; no vendor-specific import in generic services. Contract change requires owner and consumer review, semantic version decision and compatibility fixtures. Contribution acceptance requires documented scope, rationale, tests, license and maintenance ownership. Prefer upstream driver/module fixes; maintain minimal justified patch sets.

## Partner onboarding kit

Reference app, architecture and interface contract, target descriptor template, board porting checklist, Kconfig/devicetree examples, golden fixtures, simulator, HIL/test protocol, requirement/evidence template, license/SBOM and security reporting guide. A partner implements a target adapter or service module through public extension interface, runs conformance, submits change impact and evidence. Publish capability matrix and support scope; no silent unsupported feature. Provide onboarding lab with simulated benign endpoint and host; compare time to first telemetry and second vendor port without generic modifications.

## Commercial and governance model

Open generic interoperable foundations subject to legal review. Support paid integration, BSP maintenance, long-term branches/patches, conformance and safety evidence services only to contracted scope. Architectural decision board includes platform and supplier representatives. Require maintainer ownership, contribution policy, SPDX/license review, reproducible and signed releases, vulnerability disclosure, coordinated fix policy and support lifecycle. Avoid proprietary lock-in at protocol and target boundaries; preserve customer-specific confidential policy separately.

## Staged roadmap

| Stage | Relative window | Work and exit |
|---|---|---|
| G0 | 0–2 months | Stakeholder workshops, benign desktop scope, contract and ownership freeze; candidate board discovery in parallel |
| G1 | 2–5 months | Portable core, service composition, two-process simulator, native Zephyr, selected UART and target fixtures |
| G2 | 5–9 months | Smart edge second vendor, target admission, update recovery and measured HIL |
| G3 | 9–15 months | Host VM, VHAL/SDV, CommonAPI/SOME-IP, Ethernet and IPC feasibility |
| G4 | 15–24 months | Zonal subset, OEM/Tier 1 pilot, allocated safety/cyber gap and support plan |
| G5 | After G4 | Configuration-specific candidate release and assessment as appropriate |

Dates are illustrative elapsed windows after funded start, not commitments. Integrated RT path parallel after G2 if SoC partner supplies isolation evidence. Staff system architect, embedded/BSP, native RTOS, host middleware, verification/HIL, cybersecurity, functional safety, release/support and program ownership. First 90 days: scope, portable app and desktop simulator, semantic contract fixtures, measurement harness and architecture review; review target board and UART candidates in parallel. Gate outcomes rather than feature count determine investment.
