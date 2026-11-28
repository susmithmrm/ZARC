# Z01 — Strategy: one foundation for two generations

> **ZARC 2035 | Architecture specification | Design phase.** The single requirements register is [Z12](12-Requirements-Verification.csv) (267 requirements). Implementation and verification are planned for the next phase. See [implementation plan](../IMPLEMENTATION-STATUS.md).

## Mission
Make ZARC the preferred open-source real-time platform for OEMs, Tier 1 and Tier 2 suppliers, semiconductor manufacturers and intelligent embedded developers by 2030–2035, while remaining compatible with distributed CAN/CAN FD/LIN ECUs. Preference must be earned through independent comparisons and actual adoption.

| Stakeholder | Outcome | Proof |
|---|---|---|
| OEM | Topology freedom, long-term upgrades and cost control | Legacy and zonal portfolio, lifecycle cost |
| Tier 1 | Reuse across customer programs | Independent component exchange and customer variants |
| Tier 2 | Small intelligent components | Minimal native build and SDK |
| Silicon | Native hardware integration | Two physical MCU vendors, exact support matrix |
| AI and embedded | Optional inference and portable contracts | Bounded reference and benchmarks |
| Quality and assurance | Reviewable evidence | Scoped product acceptance |

## Competitive approach
Compete on necessary engineering outcomes, not copying AUTOSAR layers. Respect licensed specifications; complement S-CORE, AAOS, Linux and QNX. Target 30% reusable platform effort and 25% lifecycle cost reductions as hypotheses (ADOPT-042); report negative results. No vendor endorsement or universal compliance.
## Architecture principles
One codebase, one Z12, native Zephyr first. Independently maintained micro-components are not mandatory dynamically loaded microservices. Every selected capability has resource, authority, lifecycle and evidence allocations. Hardware control stays local; management and AI are bounded. Preserve customer IP and avoid mandatory core forks.

## Decision and acceptance policy
Use independent workload, hardware and assurance comparisons against legally accessible automotive platforms. The shared platform does not own OEM domain algorithms, vehicle safety decisions, cloud fleet services or silicon errata. Public roadmaps are design inputs, not supplier commitments. Support for any company or product requires an agreed integration and independently reviewed evidence.

## Global scope and target outcomes
The shared foundation is intended for passenger, commercial, off-highway and intelligent embedded products; sector adapters and assurance scopes are selected, not universal dependencies. OEM, Tier 1, Tier 2, semiconductor and embedded/AI partner evaluation is a portfolio obligation (ADOPT-082). Tier 3 manufacturing and component vendors participate through allocated manufacturing, materials and lifecycle interfaces where relevant. Regional and customer-specific requirements belong to the product allocation (ADOPT-030, SUP-017). The 30% and 25% economic targets are evaluation hypotheses; they are not per-image release thresholds.
