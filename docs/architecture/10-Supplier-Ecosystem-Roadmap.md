# Z10 — Global supplier adoption and lifecycle

> **ZARC 2035 | Proposed design baseline | 28 November 2026.** The single normative-candidate register is [Z12](12-Requirements-Verification.csv) (267 requirements). This is not evidence of implementation or production qualification.

## Partner model
OEM owns vehicle allocation, architecture and acceptance; Tier 1 owns integration; Tier 2 supplies independently versioned components; silicon vendor owns BSP and driver evidence; Tier 3 benefits indirectly; AI and tools vendors provide optional bounded integrations. Open MIT core, explicit third-party terms, private customer product inputs.
## Customer integration
Publish pinned SDK, component exchange package, interface and conformance fixtures, exact target support matrix, evidence and known limitations. Demonstrate independent replacement and two customer variants without a mandatory core fork. Customer requirements and quality obligations are controlled product allocations linked to Z12, not extra platform baselines (SUP-017). Assess contracted IATF customer-specific requirements, APQP, PPAP or VDA 2, FMEA, 8D, field quality and change notification. Regional regulatory applicability belongs to the exact product and jurisdiction. No platform certification claim.
## Lifecycle
Version and compatibility policy, reproducible archive, patch/backport and migration route, accountable maintenance owner and explicit support funding. Quarterly public industry roadmap review; annual competitor and standards review; review before major changes. Publish gaps and dispositions (SUP-015).
## Implementation sequencing
1. Native composition, component descriptors and engineering graph.
2. Authorized observation and identity reconciliation.
3. Deployment-unit management, state migration and recovery.
4. Multi-vendor physical targets, legacy and zonal interoperability.
5. Optional AI and heterogeneous providers.
6. Customer pilots, measured economics and scoped qualification.
Portfolio adoption review ADOPT-082 includes both architecture generations. No partnerships or superiority claims without independently substantiated evidence.

## Retained engineering detail and historical context
Existing design detail is retained for trace continuity. The current Z12 and preceding architecture govern where wording differs; historical counts, findings, examples and acceptance claims are scoped to their original revision.

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
